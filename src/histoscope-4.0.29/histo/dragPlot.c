/*******************************************************************************
*									       *
* dragPlot.c -- ICCCM Module for the Histo-Scope product set		       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* June 8, 1995								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Color management fixed by igv, 2003		      		       	       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#if XmVersion >= 1002 /* Drag & drop not part of Motif before 1.2 */
#include <Xm/Xm.h>
#include <Xm/PrimitiveP.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>
#include <Xm/MessageB.h>
#include <Xm/Frame.h>
#include <X11/Xatom.h>
#include <Xm/ToggleB.h>
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "configFile.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "ntuplePanel.h"
#include "multPlot.h"
#include "dragPlot.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/H1DP.h" 
#include "../plot_widgets/ScatP.h"
#include "../plot_widgets/XYP.h"
#include "../plot_widgets/XYDialogs.h"
#include "../plot_widgets/CellP.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/3DScat.h"

#define NUM_IMPORT_TARGETS 1
#define NUM_ACTIONS 1

/* Local Function Prototypes */
static Boolean checkTargets(Widget w, Display *display);
static void handleDrop(Widget w, XtPointer call_data);
static void handleHelp(Widget w, XtPointer call_data);
static void handleOK(Widget w, XtPointer client, XtPointer call);
static void dropProcCallback(Widget w, XtPointer client, XtPointer call);
#ifdef NOTDEF
static void dropDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
#endif
static void startDrag(Widget w, XEvent *event, String *params, 
			Cardinal *num_params);
static Boolean convertProc(Widget w, Atom *selection, Atom *target, Atom *type,
		    XtPointer *value, unsigned long *length, int *format);
static void transferProcCallback(Widget w, XtPointer closure, Atom *seltype, 
				 Atom *type, XtPointer value, 
				 unsigned long *length, int *format);
static void cancelDrop(Widget w, XtPointer client, XtPointer call);
static int dragOver(windowInfo *fromWindow, windowInfo *toWindow);
static int copyPlotToMultiPlot(multWindow *multWin, windowInfo *fromWindow,
    	int row, int col, int fromCell);
static void matchMenuToggle(Widget fromWidget, Widget toWidget);
static void matchPlotLimits(Widget fromPlotW, Widget toPlotW);
static void getLogSettings(Widget plotW, Boolean *xLog, Boolean *yLog, 
			   Boolean *zLog);
static Boolean matchStyles(windowInfo *fromWInfo, windowInfo *toWInfo);
#ifdef DEBUG
static void chkWinInfos(windowInfo *wInfo1, windowInfo *wInfo2);
#endif
static int duplicateColor(Display *display, Colormap cmap,
			  Pixel in, Pixel *out);

/* Module global variable declarations */

static Atom ImportList[NUM_IMPORT_TARGETS] = {0};
static char DragTranslations[] = "#override <Btn2Down>: StartDrag()";
static XtActionsRec DragActions[] = { {"StartDrag", (XtActionProc) startDrag} };
static Widget SaveFrameWidget = NULL;	/* saves frame widget in handleOK */

/*
 * This struct is used to pass information from the dropProc 
 * to the transferProc
 */
typedef struct _DropTransferRec {
    Widget widget;		/* frame widget user dropped the plot into */
    multWindow *multWin;	/* its parent Multi-plot window		   */
    int row;			/* row number of frame in multi-plot	   */
    int col;			/* column number of frame in multi-plot	   */
} DropTransferRec, *DropTransfer;

/*
 * This struct is used to pass information from the dragStart proc 
 * to it's associated callback procs (i.e. the convert proc).
 */
typedef struct _DragConvertRec {
    Widget widget;		    /* the plot widget dragged by the user */
    windowInfo *wInfo;		    /* its wInfo pointer	           */
    int pid;			    /* pid of initiating histo process     */
} DragConvertRec, *DragConvertPtr;

/* 
** AddActionsForDraggingPlots - This routine is called once for histo's
**                              application context
**
** Add new actions for this application context for use with translation tables
*/
void AddActionsForDraggingPlots(XtAppContext appContext)
{
    XtAppAddActions(appContext, DragActions, NUM_ACTIONS);
}

/* 
** RegisterWidgetAsPlotReceiver - w: the widget to register as a drop site.
**				     [This will be a frame widget in the
**				          Multi-Plot Window.]
*/
void RegisterWidgetAsPlotReceiver(Widget w)
{
    Atom HISTO_ITEM_ID;
    Display *dpy = XtDisplay(w);
    int n = 0;
    Arg args[25];
    
    HISTO_ITEM_ID = XmInternAtom(dpy, "HISTO_ITEM_ID", False);
    ImportList[0] = HISTO_ITEM_ID;
    XtSetArg(args[n], XmNimportTargets, ImportList); n++;
    XtSetArg(args[n], XmNnumImportTargets, NUM_IMPORT_TARGETS); n++;
    XtSetArg(args[n], XmNdropSiteOperations, XmDROP_COPY); n++;
    XtSetArg(args[n], XmNdropProc, dropProcCallback); n++;
    XmDropSiteRegister(w, args, n);
}


/* 
** UnRegisterWidgetAsPlotReceiver - w: the widget to unregister as a drop site.
**				       [This will be a frame widget in the
**				          Multi-Plot Window.]
*/
void UnRegisterWidgetAsPlotReceiver(Widget w)
{
    XmDropSiteUnregister(w);
}

/* 
** MakePlotADragSource - Add the translations & actions necessary to make a 
** 			 plot widget a drag source.
*/
void MakePlotADragSource(Widget w)
{
    XtTranslations new_table;

    new_table = XtParseTranslationTable(DragTranslations);
    XtOverrideTranslations(w, new_table);
}

/* Static (local) Routines:
**
**                ************** PLEASE NOTE ************
**
** To understand the flow of control between drag initiator and drop receiver
** routines, see O'Reilly Vol 6A, Motif V1.2 Programming Manual, p. 633 
** (section 18.2.5 - The Programming Model).
**
**                        [Initiator Routine]
** startDrag - This action procedure sets up the drag data and begins the
**                 drag operation for a plot widget.
**	       This routine is called by Xt when the user presses mouse 
**                 button 2 over a plot widget registered as a drag source.
**             This routine builds a drag conversion structure (containing
**                a pointer to the plot widget, its window info pointer,
**                and the process' PID) that will be passed as client data
**                to the drag context widget created by XmDragStart.  This
**                structure will be retrieved and used by the drag conversion 
**                procedure: convertProc.  ConvertProc passes this drag 
**                conversion structure to the transfer proc in the pointer
**                "value" so that the pid can be compared and a new mini-
**                plot can be constructed from the wInfo of the dragged plot.
**             While mouse button 2 is pressed and after XmDragStart is
**                called, the toolkit is in control.  It will call the
**                callbacks we specified in the drag context widget at the 
**                appropriate time.
**
**	       w: widget mouse was pointing to when pressed
**
*/
static void startDrag(Widget w, XEvent *event, String *params, 
			Cardinal *num_params)
{
    Atom            targets[1];
    Display         *display = XtDisplay(w);
    DragConvertPtr  convRec;
    Arg             args[16];
    int             n = 0;
    /* Widget          myDC; */
    
#ifdef DEBUG
    printf("------------------------------------------------------------\n");
    printf("in startDrag... plot wid = %p, num_params = %d\n", w, *num_params);
#endif /*DEBUG*/

    /* Setup arglist for the drag context widget that is created at drag 
       start */
    n = 0;

    /*
     * set up the available export targets.  These are targets that we
     * wish to provide data on
     */
    targets[0] = XmInternAtom(display, "HISTO_ITEM_ID", False);
    XtSetArg(args[n], XmNexportTargets, targets); n++;
    XtSetArg(args[n], XmNnumExportTargets, 1); n++;

    /*
     * identify the conversion procedure and the client data passed 
     * to the conversion procedure: plot widget, wInfo ptr, and process ID.
     * The convert proc is used to convert the drag source data to the
     * format requested by the drop site when the drop occurs.
     */
    convRec = (DragConvertPtr) XtMalloc(sizeof(DragConvertRec));
    convRec->widget = w;
    convRec->wInfo = GetWinfoFromWidget(w);
    convRec->pid = getpid();
    if (convRec->wInfo == NULL) {
    	fprintf(stderr, 
    	   "Error: Btn2Down Translation action is not for plot widget.\n");
    	XtFree((char *) convRec);
    	return;
    }
    XtSetArg(args[n], XmNclientData, convRec); n++;
    XtSetArg(args[n], XmNconvertProc, convertProc); n++;

    /* set the drag operations that are supported: only COPY. */
    XtSetArg(args[n], XmNdragOperations, XmDROP_COPY); n++;

    /* start the drag. this creates a drag context widget, which stores
       information during the drag. */
    /* myDC = XmDragStart(w, event, args, n); */
    XmDragStart(w, event, args, n);

#ifdef DEBUG
    printf("called XmDragStart; leaving startDrag...\n");
#endif /*DEBUG*/

}

/*                        [Receiver Routine]
** handleDrop - routine called by dropProcCallback, which is called by the 
**              intrinsics when the user releases mouse button 2 (performs
**              a drop).  That routine is registered with the intrinsics by
**              RegisterWidgetAsPlotReceiver for the frame widget in the
**              Multi-Plot window.
**
**              This procedure is responsible for determining whether the
**              drop is successful based on the targets and operations
**              supported by the drag source and the drop site. It will then
**              call XmDropTransferStart to initialize the drop transfer.
**
**              handleDrop is also called by the handleOK routine when the
**              user clicks on the OK button in the help dialog.
**
**              The call_data parameter is of type XmDropProcCallbackStruct
**              and holds information about the drop to the receiver:  the
**              operation, operations, and dropSiteStatus.  It is the drop
**              action field that indicates whether this is a normal drop or
**              help has been requested (handled by handleHelp before we get
**              here).
**
**              w is the widget receiving the drop - i.e. the widget the
**              mouse was pointing to when the user released MB2 or pressed
**              the HELP key (F1).  For our purposes, w should be a frame 
**              widget in a multi-plot window.
**
**              This drop proc creates a list of DropTransferEntries
**              containing target and client-specific information for each
**              data-target type combination.  Here we handle only one
**              data-target combination: HISTO_ITEM_ID, although this will
**              change once NDraw is finished.  Also only the COPY operation
**              is currently supported (not Move or Link).
**
**              This drop proc creates a Drop Transfer record which
**              it uses to store the widget pointers to the multi-plot and
**              frame widget receiving the drop and the row and column 
**              where the plot was dropped.  The drop transfer record pointer
**              is passed through the transfer entry mechanism to the transfer
**              proc through its "closure" argument.
**
**              This routine cannot take very long in processing the drop,
**              else the drop may time out.  It should only verify that the
**              drop can take place (otherwise it should start a sub-process).
*/
static void handleDrop(Widget w, XtPointer call_data)
{
#ifdef NOTDEF
    static XtCallbackRec dropDestroyCb[] = {
        {dropDestroyCB, NULL},
        {NULL, NULL}
    };
#endif

    XmDropProcCallbackStruct    *cb = (XmDropProcCallbackStruct *)call_data;
    Display                     *display = XtDisplay(w);
    Arg                         args[10];
    int                         n;
    int				invalid = True;
    DropTransfer                transferRec;
    XmDropTransferEntryRec      transferEntries[2];
    XmDropTransferEntryRec      *transferList = NULL;
    Cardinal                    numTransfers = 0;

#ifdef DEBUG
    printf("in handleDrop...    w = %p\n", w);
#endif /*DEBUG*/

    /* Cancel the drop on invalid drop operations */
    if ((cb->operations & XmDROP_COPY)) {
        if (checkTargets(cb->dragContext, display)) {
            cb->operation = XmDROP_COPY;

#ifdef DEBUG
            printf("                  targets checked out OK\n");
#endif /*DEBUG*/

            /* intialize data to send to drop transfer callback */
            transferRec = (DropTransfer) XtMalloc(sizeof(DropTransferRec));
            transferRec->widget = w;
            if (XtClass(w) == xyWidgetClass) {
            	transferRec->multWin = NULL;
            	transferRec->row = 0;
            	transferRec->col = 0;
            	invalid = False;

#ifdef DEBUG
            	printf("                  drop site is XY Widget\n");
#endif /*DEBUG*/

            } else if (XtClass(w) == xmFrameWidgetClass) {
            	transferRec->multWin = GetMPlotInfoFromFrame( w, 
            		&transferRec->row, &transferRec->col);
            	if (transferRec->multWin != NULL) {
            	    invalid = False;
#ifdef DEBUG
                    printf("                frame widget found: multWin = %p\n",
                        transferRec->multWin);
                } else {
                    printf("                  ERROR: frame widget not found\n");
                    printf("                     OR: user cancelled drop\n\n");
#endif /*DEBUG*/
                }
            }
        }
    
#ifdef DEBUG
    	else
            printf("                  targets do not match\n");
#endif /*DEBUG*/
    
    }
    
#ifdef DEBUG
    else
        printf("                  drop operation not XmDROP_COPY\n");
#endif /*DEBUG*/
    
    if (invalid) {
        n = 0;
        cb->operation = XmDROP_NOOP;
        cb->dropSiteStatus = XmINVALID_DROP_SITE;
        XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
        XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
    } else {
        transferEntries[0].target = XmInternAtom(display, 
                                               "HISTO_ITEM_ID", False);
        transferEntries[0].client_data = (XtPointer) transferRec;

        numTransfers = 1;

        transferList = transferEntries;

        /* Setup transfer list */
        n = 0;
        cb->dropSiteStatus = XmVALID_DROP_SITE;
        XtSetArg(args[n], XmNdropTransfers, transferList); n++;
        XtSetArg(args[n], XmNnumDropTransfers, numTransfers); n++;

#ifdef NOTDEF
/* For some reason adding this callback makes a bus error before even
   getting to the callback itself.  Why, I don't know...   
   So instead, I added code to the transfer proc to free the transferRec -JMK */
        /* Setup destroy callback to free transferRec */
        dropDestroyCb[0].closure = (XtPointer) transferRec;
        XtSetArg(args[n], XmNdestroyCallback, dropDestroyCB); n++;
#endif /*NOTDEF*/

        /* Setup transfer proc to accept the drop transfer data. */
        /* transferProcCallback receives the converted data targets
           from the drag source. */
        XtSetArg(args[n], XmNtransferProc, transferProcCallback); n++;
    }
    
    /*
     * XmDropTransferStart initializes & returns a DropTransfer widget,
     * (which is not explicitly used by this program, thus the widget ptr
     * is not saved).  Xt initiates the transfers right after this handleDrop
     * routine returns.  transferProcCallback will be called for each target
     * in the transfer list.  The toolkit on the receiver side is in charge of
     * the transfer process, and information about the transfer is stored in
     * the Drop Transfer widget.
     * 
     * XmDropTransferStart must be called even if the drop is not successful.
     * In that case, XmDropTransferStart cleans up the transfer.
    */
#ifdef DEBUG
    printf("                  Calling XmDropTransferStart...\n");
#endif /*DEBUG*/

    XmDropTransferStart(cb->dragContext, args, n);

#ifdef DEBUG
    printf("Leaving handleDrop...\n");
#endif /*DEBUG*/

}

static Boolean checkTargets(Widget w, Display *display)
{
    Atom        *exportTargets;
    Cardinal    numExportTargets;
    Arg         args[2];

    /* Get list of transfer targets */
    XtSetArg(args[0], XmNexportTargets, &exportTargets);
    XtSetArg(args[1], XmNnumExportTargets, &numExportTargets);
    XtGetValues(w, args, 2);

    /* see if there's a match between export & input targets */
    return XmTargetsAreCompatible(display, exportTargets, numExportTargets,
    		ImportList, NUM_IMPORT_TARGETS);
}


/*                             [Receiver Routine]
 * transferProcCallback - This procedure handles the data that is being 
 *                        transferred.  It is called by the receiver's toolkit
 *                        to process the converted data it received from the
 *                        initiator's convert proc.
 *    
 *                        When the operation is Copy, the value field contains
 *                        a pointer to the data from the initiator.  It is used
 *                        to assign the value to some element in the receiver's
 *                        program.  (When the transfer is finished, both the
 *                        initiator and receiver have the data.)
 *
 *                        In the case of HISTO_ITEM_ID, value points to a 
 *                        DragConvert structure which contains the initiating 
 *                        plot widget, its window info ptr, and the pid of
 *                        the initiating Histo process.  This pid must match
 *                        our pid, since we are supporting only intra-client 
 *                        drops.  (This is the widget being dragged.)
 *
 *                        wid (transferRec->widget) is the widget receiving 
 *                        the drop - i.e. the widget the mouse was pointing  
 *                        to when the user released MB2 (or pressed the HELP   
 *                        key, F1).  For our purposes, w should be a frame    
 *                        widget in a multi-plot window.
 *
 */
static void transferProcCallback(Widget w, XtPointer closure, Atom *seltype, 
				 Atom *type, XtPointer value, 
				 unsigned long *length, int *format)
{
    DropTransfer transferRec = (DropTransfer) closure;
    Widget       wid = transferRec->widget;
    Display      *display = XtDisplay(wid);
    Atom         HISTO_ITEM_ID = XmInternAtom(display, "HISTO_ITEM_ID", False);
    Atom         NULL_ATOM = XmInternAtom(display, "NULL", False);
    Arg          args[10];
    int          n;

    /*
     * The delete target returns a NULL_ATOM type and value equal to NULL 
     * so it isn't a failure.  Otherwise, check for NULL value or targets 
     * that we don't support and set transfer failure.
     */
#ifdef DEBUG
    printf("in transferProcCallback...    w = %p, length = %d, format = %d\n", 
        w, *length, *format);
    printf("                              wid = %p\n", wid);
#endif /*DEBUG*/

    if (*type != NULL_ATOM && (!value || (*type != HISTO_ITEM_ID))) {
        n = 0;
        /*
         * On failures set both transferStatus to XmTRANSFER_FAILURE and
         * numDropTransfers to 0.
         */
#ifdef DEBUG
        printf("in transferProcCallback: TRANSFER_FAILURE due to null atom,\n");
        printf("                          null value, or unsupported target\n");
#endif /*DEBUG*/

        XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
        XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
        XtSetValues(w, args, n);

        /* Free the value if there is one, or we would have a memory leak */
        if (value)
            XtFree(value);

        return;
    }

    /* Handle HISTO_ITEM_ID type */
    if (*type == HISTO_ITEM_ID) {
        /* Can only accept a drop from the same histo process */
        /* This is because we don't provide for xferring the entire item */
        /* and its data.  Also the item could originate from a running proc. */
    	DragConvertPtr conv = (DragConvertPtr) value;
    	int transferFailure = True;
    	if (conv->pid == getpid()) {	/* Is pid same process? */
            hsGeneral *item = GetMPItemByID(conv->wInfo->pInfo[0]->id);
            int fromCell = (XtClass(conv->widget) == cellWidgetClass);
            int fromXYPlot = (XtClass(conv->widget) == xyWidgetClass);
            int toMultiPlot = (transferRec->multWin != NULL);
            /* plotInfo **pInfo = conv->wInfo->pInfo; */
#ifdef DEBUG
printf("        item = %p, pid = %d, multWin = %p, row = %d, col = %d\n",
    	item, conv->pid, transferRec->multWin, transferRec->row, 
    	transferRec->col);
printf("        fromCell = %d, fromXYPlot = %d, toMultiPlot = %d\n", fromCell,
    	fromXYPlot, toMultiPlot);
#endif /*DEBUG*/

            /* Drag a plot to a frame in a Multiple Plot Window */
            if (item != NULL && toMultiPlot) {
    	    	if (copyPlotToMultiPlot(transferRec->multWin, conv->wInfo,
    	    	    	transferRec->row, transferRec->col, fromCell))
#ifdef notdef
                windowInfo *toWInfo = 
                    AddHistToMultiPlot(transferRec->multWin, item, 
                    transferRec->row, transferRec->col, conv->wInfo, fromCell,
                    conv->wInfo->pInfo[0]->errorBars);
            	int i, nCurves;
            	XYCurve *curveStyles = (XYCurve *)conv->wInfo->curveStyles;
            	XYHistogram *histStyles =
            	    	(XYHistogram *)conv->wInfo->histStyles;
            	for  (i = 1; i < conv->wInfo->nPlots; ++i) {
            	    plotInfo **pInfo = conv->wInfo->pInfo;
            	    item = GetMPItemByID(pInfo[i]->id);
            	    if (item != NULL) {
            	    	nCurves = NCurvesInPlot(pInfo[i]->plotType,
            	    	    pInfo[i]->ntVars);
            	    	OverlayPlot(toWInfo, item, pInfo[i]->plotType,
            	    	    pInfo[i]->ntVars, pInfo[i]->sliderVars, 
            	    	    pInfo[i]->nXBins, pInfo[i]->nYBins,
            	    	    pInfo[i]->errorBars, curveStyles, nCurves,
            	    	    histStyles);
             	    	curveStyles += nCurves;
             	    	histStyles +=
             	    	    PlotIsHist(conv->wInfo->pInfo[i]->plotType);
             	    }
            	}
#endif
            	    transferFailure = False;
            
            /* Add a dragged XY plot to another XY plot */
            } else if (item != NULL && fromXYPlot 
            		&& XtClass(transferRec->widget) == xyWidgetClass) {
            	windowInfo *toWInfo = GetWinfoFromWidget(transferRec->widget);
            	/* Disallow dragging into the same plot */
            	if (conv->wInfo != toWInfo) {
            	    if (dragOver(conv->wInfo, toWInfo))
            	    	transferFailure = False;
            	}
            }
            
            /* Otherwise it's a transfer failure */
#ifdef DEBUG
            else
        	printf(
		"in transferProcCB: TRANSFER_FAILURE: item = NULL or not XY\n");
#endif /*DEBUG*/
        
        } 			/* endif: pid is same process */
        
#ifdef DEBUG
	else
            printf(
   "in transferProcCB: TRANSFER_FAILURE: drag source not same histo process\n");
#endif /*DEBUG*/

        if (transferFailure) {
            n = 0;
            XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
            XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
            XtSetValues(w, args, n);
        }
    }			/* endif HISTO_ITEM_ID */

    /* Free the value if there is one, or we would have a memory leak */
    if (value)
        XtFree(value);

#ifndef NOTDEF
/* For some reason adding a dropDestroy callback makes a bus error before even
   getting to the callback itself.  Why, I don't know...   
   So instead, I added this code to free the transferRec (closure arg) -JMK 
*/
    if (closure)
        XtFree(closure);
#endif /*NOTDEF*/

#ifdef DEBUG
    printf("leaving transferProcCB...\n");
#endif /*DEBUG*/

}


/*                        [Receiver Routine]
 * dropDestroyCB - This procedure frees the data used in the data transfer proc
 *                 that was passed from the drop proc.
 */
#ifdef NOTDEF
static void dropDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{

#ifdef DEBUG
    printf("in & out of dropDestroyCB...    w = %p\n", w);
#endif /*DEBUG*/

    XtFree((char *)clientData);
}
#endif

/*                        [Initiator Routine]
 * convertProc - This is a selection conversion function that is used in
 *               converting requests for histo targets for the drop site.
 *               The procedure converts the drag source data to the format
 *               requested by the drop site when the drop occurs.
 *  
 *               The return types follow ICCC standards.
 *
 *               This routine is specified as the XmNconvertProc in the
 *               drag context widget at drag start time.
 *
 *               For target HISTO_ITEM_ID,  a drag conversion structure 
 *               (containing a pointer to the plot widget, its window info
 *               pointer, and the process' PID) was created by the routine 
 *               startDrag.  This convertProc will retrieve this drag 
 *               conversion structure and put the drag source data into 
 *               the format requested by the drop site.
 */
static Boolean convertProc(Widget w, Atom *selection, Atom *target, Atom *type,
		    XtPointer *value, unsigned long *length, int *format)
{

    Display       *display = XtDisplay(w);
    Atom          HISTO_ITEM_ID = XmInternAtom(display, "HISTO_ITEM_ID", False);
    Atom          TARGETS = XmInternAtom(display, "TARGETS", False);
    Atom          MULTIPLE = XmInternAtom(display, "MULTIPLE", False);
    Atom          TIMESTAMP = XmInternAtom(display, "TIMESTAMP", False);
    Atom          *targs;
    int           MAX_TARGS = 4;
    int           target_count;
    DragConvertPtr conv;
    Widget        widget;
    Arg           args[1];

#ifdef DEBUG
    printf("in convertProc...    w = %p\n", w);
#endif /*DEBUG*/

    /* Get the convert structure (client data) from the drag context widget 
     * and the widget that initiated the drag from the convert structure */
    XtSetArg(args[0], XmNclientData, &conv);
    XtGetValues(w, args, 1);
    widget = (Widget) conv->widget;

    /* Make sure we are doing a motif drag by checking if the widget that
     * is passed in is indeed a drag context. Make sure the drag source widget
     * in the client data is not NULL. */
    if (!XmIsDragContext(w) || widget == NULL)
        return False;

    if (*target == HISTO_ITEM_ID) {

        /* value, type, length, and format must be assigned values */
        *value = (XtPointer) conv;
        *type = HISTO_ITEM_ID;
        *length = sizeof(DragConvertRec);
        *format = 32;

    }
    else if (*target == TARGETS) {

        /* This target is required by ICCC */
        targs = (Atom *)XtMalloc((unsigned) (MAX_TARGS * sizeof(Atom)));
        target_count = 0;

        *value = (XtPointer) targs;
        *targs++ = HISTO_ITEM_ID; 
        target_count++;
        *targs++ = TARGETS; 
        target_count++;
        *targs++ = MULTIPLE; 
        target_count++;  /* supported in the Intrinsics */
        *targs++ = TIMESTAMP; 
        target_count++; /* supported in the Intrinsics */
        *type = XA_ATOM;
        *length = (target_count * sizeof(Atom)) >> 2;
        *format = 32;

    }
    else
        return False;

#ifdef DEBUG
    printf("leaving convertProc...\n");
#endif /*DEBUG*/

    return True;

}

/*                           [receiver]
** handleHelp - This procedure is called by the dropProcCallback [receiver]
**              routine.  handleHelp manages the help dialog and allows the
**              user to continue with the drop or cancel it.
**
**              w is the widget receiving the drop - i.e. the widget the
**              mouse was pointing to when the user pressed the HELP
**              key (F1).  For our purposes, w should be a frame widget
**              in a multi-plot window, or else the drop won't succeed.
**
**              call_data points to the XmDropProcCallbackStruct passed from
**              dropProcCallback.
*/
static void handleHelp(Widget w, XtPointer call_data)
{

    static Widget helpDialog = NULL;
    static char help_msg_1[] =       "This drop action can copy a\n\
plot to a Multi-Plot Window.\n\
To accept this drop, press the\n\
OK button, otherwise press\n\
Cancel";
    static char help_msg_2[] =       "This drop action can copy a\n\
plot as an overlay to this one.\n\
To accept this drop, press the\n\
OK button, otherwise press\n\
Cancel";
    static char help_msg_inv[] =    "This drop action is at an Invalid drop\n\
position.  Please cancel this drop \n\
by pressing the Cancel button.";

    XmDropProcCallbackStruct       *cb = (XmDropProcCallbackStruct *)call_data;
    static XmDropProcCallbackStruct client;
    XmString                        helpStr;
    Arg                             args[5];
    int                             n = 0;
    
#ifdef DEBUG
    printf("in handleHelp...    w = %p\n", w);
#endif /*DEBUG*/

    /* the drop is valid until it is determined invalid */
    cb->dropSiteStatus = XmVALID_DROP_SITE;

    /* if we haven't created a help dialog, create one now */
    if (helpDialog != NULL && XtParent(helpDialog) != w) {
    	XtDestroyWidget(helpDialog);
    	helpDialog = NULL;
    }
    if (helpDialog == NULL) {

        XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
        XtSetArg(args[n], XmNtitle, "Plot Drop Help"); n++;
        helpDialog = XmCreateMessageDialog(w, "Help", args, n);

        XtAddCallback(helpDialog, XmNokCallback,
                      (XtCallbackProc) handleOK, (XtPointer) &client);
        XtAddCallback(helpDialog, XmNcancelCallback,
                      (XtCallbackProc) cancelDrop, (XtPointer) &client);

        XtUnmanageChild(XmMessageBoxGetChild(helpDialog, XmDIALOG_HELP_BUTTON));

        XtRealizeWidget(helpDialog);

    }

    /* pass the necessary callback information along */
    client.dragContext = cb->dragContext;
    client.x = cb->x;
    client.y = cb->y;
    client.dropSiteStatus = cb->dropSiteStatus;
    client.operation = cb->operation;
    client.operations = cb->operations;

    /* We need to save the frame (or XY) widget (w) for the drop so we have it
       in handleDrop if the user presses the OK button.  This global will
       be retrieved by handleOK and passed to handleDrop.
    */
    SaveFrameWidget = w;

    /* determine the appropriate help message */
    if (cb->operations == XmDROP_COPY && XtClass(w) == xmFrameWidgetClass) {
            helpStr = XmStringCreateLtoR(help_msg_1, XmFONTLIST_DEFAULT_TAG);
            XtManageChild(XmMessageBoxGetChild(helpDialog, XmDIALOG_OK_BUTTON));
    }
    else if (cb->operations == XmDROP_COPY && XtClass(w) == xyWidgetClass) {
            helpStr = XmStringCreateLtoR(help_msg_2, XmFONTLIST_DEFAULT_TAG);
            XtManageChild(XmMessageBoxGetChild(helpDialog, XmDIALOG_OK_BUTTON));
    }
    else {
        helpStr = XmStringCreateLtoR(help_msg_inv, XmFONTLIST_DEFAULT_TAG);
        XtUnmanageChild(XmMessageBoxGetChild(helpDialog, XmDIALOG_OK_BUTTON));
    }

    /* set the help message in the dialog */
    XtSetArg(args[0], XmNmessageString, helpStr);
    XtSetValues(helpDialog, args, 1);

    /* Free the XmString */
    XmStringFree(helpStr);

    /* map the help dialog */
    XtManageChild(helpDialog);

#ifdef DEBUG
    printf("leaving handleHelp...\n");
#endif /*DEBUG*/

}

/*
 * handleOK - This procedure is called by the drop help dialog [in the 
 *            receiver] to continue with the drop.  
 * 
 *            (Note: SaveFrameWidget may now also be a XY Widget.)
 */

static void handleOK(Widget w, XtPointer client, XtPointer call)
{

    XmDropProcCallbackStruct *cb = (XmDropProcCallbackStruct *)client;

#ifdef DEBUG
    printf("in handleOK...    w = %p, SaveFrameWidget = %p\n", w, 
    	SaveFrameWidget);
#endif /*DEBUG*/

    cb->operation = XmDROP_COPY;
    handleDrop(SaveFrameWidget, (XtPointer) cb);

}

/* 
 * cancelDrop - This procedure is used with the drop help dialog to cancel 
 *              the drop.
 */
static void cancelDrop(Widget w, XtPointer client, XtPointer call)
{

    XmDropProcCallbackStruct    *cb = (XmDropProcCallbackStruct *)client;
    Arg                         args[2];

#ifdef DEBUG
    printf("in cancelDrop...    w = %p\n", w);
#endif /*DEBUG*/

    /* On help, we need to cancel the drop transfer */
    XtSetArg(args[0], XmNtransferStatus, XmTRANSFER_FAILURE);
    XtSetArg(args[1], XmNnumDropTransfers, 0);

    /* we need to start the drop transfer to cancel the transfer */
    XmDropTransferStart(cb->dragContext, args, 2);

}


/*                        [Receiver Routine]
** dropProcCallback - routine called by the intrinsics when the user releases
**              mouse button 2 (performs a drop).  This routine is registered
**              with the intrinsics by RegisterWidgetAsPlotReceiver for the
**              frame widget in the Multi-Plot window.
**
**              This procedure either calls handleDrop to begin the drop 
**              or initiates the help dialog by calling handleHelp.  Which
**              routine is called depends on the dropAction in the callback
**              structure.  handleDrop will then determine whether the
**              drop is successful based on the targets and operations
**              supported by the drag source and the drop site.
**
**              The call data parameter is of type XmDropProcCallbackStruct
**              and holds information about the drop to the receiver:  the
**              operation, operations, and dropSiteStatus.  It is the drop
**              action field that indicates whether this is a normal drop or
**              help has been requested.
**
**              w is the widget receiving the drop - i.e. the widget the
**              mouse was pointing to when the user released MB2 or pressed
**              the HELP key (F1).  For our purposes, w should be a frame 
**              widget in a multi-plot window.
*/
static void dropProcCallback(Widget w, XtPointer client, XtPointer call)
{

    XmDropProcCallbackStruct *cb = (XmDropProcCallbackStruct *)call;

#ifdef DEBUG
    printf("in & out of dropProcCallback...    w = %p\n", w);
#endif /*DEBUG*/

    if (cb->dropAction != XmDROP_HELP)
        handleDrop(w, call);
    else
        handleHelp(w, call);
}

/*
** Overlay a plot on top of another plot
*/
static int dragOver(windowInfo *fromWindow, windowInfo *toWindow)
{
    int i, j, nCurves;
    XYCurve *curveStyles = (XYCurve *)fromWindow->curveStyles;
    XYHistogram *histStyles = (XYHistogram *)fromWindow->histStyles;
    hsGeneral *item;
    plotInfo *fromPInfo, *toPInfo;
    
    /* Individually overlay each plotInfo record in fromWindow on toWindow */
    for  (i = 0; i < fromWindow->nPlots; ++i) {
	fromPInfo = fromWindow->pInfo[i];
	item = GetMPItemByID(fromPInfo->id);
	if (item == NULL)
	    return False;
	nCurves = NCurvesInPlot(fromPInfo->plotType, fromPInfo->ntVars);
	OverlayPlot(toWindow, item, fromPInfo->plotType, 
		    fromPInfo->ntVars, fromPInfo->sliderVars, 
		    fromPInfo->nXBins, fromPInfo->nYBins,
		    fromPInfo->errorBars, curveStyles, nCurves,
		    histStyles, NULL, NULL);
	curveStyles += nCurves;
	histStyles += PlotIsHist(fromPInfo->plotType);
    	/* Copy the attributes not handled by OverlayPlot */
    	toPInfo = toWindow->pInfo[toWindow->nPlots - 1];
	for (j=0; j<N_SLIDERS; j++) {
    	    toPInfo->sliderGTorLT[j] = fromPInfo->sliderGTorLT[j];
    	    toPInfo->sliderThresholds[j] = fromPInfo->sliderThresholds[j];
	}
	toPInfo->aHistBinStrategy = fromPInfo->aHistBinStrategy;
    }

    /* Redraw everything (... this could be avoided) */
    RedisplayPlotWindow(toWindow, UPDATE);
    return True;
}

/*
** Copy a window to into an empty cell of a multi-plot window
*/
static int copyPlotToMultiPlot(multWindow *multWin, windowInfo *fromWindow,
    	                       int row, int col, int fromCell)
{
    hsGeneral *item = GetMPItemByID(fromWindow->pInfo[0]->id);
    windowInfo *toWInfo;
    plotInfo *pInfo     = fromWindow->pInfo[0];
    plotInfo *fromPInfo = fromWindow->pInfo[0];
    plotInfo *toPInfo;
    int i, j; 
    int nCurves = NCurvesInPlot(fromWindow->pInfo[0]->plotType, 
    	fromWindow->pInfo[0]->ntVars);
    Boolean rebin;
    Boolean logX, logY, logZ, showLegend = False;
    XYCurve *curveStyles = (XYCurve *)fromWindow->curveStyles;
    XYHistogram *histStyles = (XYHistogram *)fromWindow->histStyles;
    XmString xLabel = NULL, yLabel = NULL;
    XmString s1;
    char *labelTitle, *errMsg = "";
    colorScaleInfo *csInfo;
    int headingType = 0;
    Pixel black;
    Display *display;
    Colormap cmap;

    if (item == NULL)
    	return False;
    	
    /* Create the first plot (pInfo[0]) in the window */
    if (fromWindow->pInfo[0]->csi.colorScale)
    {
	csInfo = &fromWindow->pInfo[0]->csi;
	if (fromCell) headingType = COLORCELL_HEADING;
    }
    else
    {
	csInfo = NULL;
	if (fromCell) headingType = CELL_HEADING;
    }
    if (item->type == HS_NTUPLE)
    	toWInfo = AddNtupleToMultiPlot(multWin, (hsNTuple *)item, row, col,
    	    fromWindow->pInfo[0]->plotType, fromWindow->pInfo[0]->ntVars,
    	    fromWindow->pInfo[0]->sliderVars, csInfo, NULL, &errMsg);
    else
    	toWInfo = AddHistToMultiPlot(multWin, item, row, col, fromWindow,
    	    headingType, fromWindow->pInfo[0]->errorBars, csInfo, NULL);

    /* Correct the mini-label.  AddHistoToMultiPlot doesn't know much about 
       dragging from one plot window to another... */
    if (fromWindow->multPlot) {
	XtVaGetValues(GetMiniPlotLabel(fromWindow), XmNlabelString, &s1, NULL);
	XtVaSetValues(multWin->plot[row*multWin->numCols+col].label, 
	    XmNlabelString, s1, NULL);
	XmStringFree(s1);
    } else {
	XtVaGetValues(fromWindow->shell, XmNtitle, &labelTitle, NULL);
	XtVaSetValues(multWin->plot[row*multWin->numCols+col].label,
	    XmNlabelString, s1=XmStringCreateSimple(labelTitle), NULL);
	XmStringFree(s1);
    }
    
    /* Correct fields unknown by Add routines above... */
    memcpy(toWInfo->pInfo[0], fromWindow->pInfo[0], sizeof(plotInfo));
#ifdef DEBUG
    printf("toWInfo->nCurves = %d, toWInfo->curveStyles = %X, nCurves = %d\n", 
    	toWInfo->nCurves, toWInfo->curveStyles, nCurves);
    printf("toWInfo->nHists = %d, toWInfo->histStyles = %X, PlotIsHist = %d\n", 
    	toWInfo->nHists, toWInfo->histStyles, PlotIsHist(pInfo->plotType));
#endif /*DEBUG*/

    /* Obtain the color name & pixel tables for the XY Plots */
    /*      GetXYColors(&ColorName, &ColorPixel); */
    display = XtDisplay(multWin->multWidget);
    cmap = DefaultColormapOfScreen(XtScreen(multWin->multWidget));
    black = BlackPixelOfScreen(XtScreen(multWin->multWidget));

    if (curveStyles != NULL && nCurves != 0) {
    	XYCurve *toStyles = &((XYCurve *)toWInfo->curveStyles)[0];
    	XYCurve *fromStyles = &((XYCurve *)fromWindow->curveStyles)[0];
    	XYCurve *toStyle, *fromStyle;
    	memcpy(toWInfo->curveStyles, curveStyles, sizeof(XYCurve) * nCurves);
    	for (j = 0, toStyle = toStyles, fromStyle = fromStyles; 
    	     j < nCurves; ++j, toStyle++, fromStyle++) {
	    toStyle->markerPixel = black;
	    if (fromStyle->markerPixel != black)
		duplicateColor(display, cmap, fromStyle->markerPixel,
			       &toStyle->markerPixel);
	    toStyle->linePixel = black;
	    if (fromStyle->linePixel != black)
		duplicateColor(display, cmap, fromStyle->linePixel,
			       &toStyle->linePixel);
	    toStyle->name = XmStringCopy(fromStyle->name);
    	}
    } else if (histStyles != NULL && PlotIsHist(pInfo->plotType)) {
    	XYHistogram *toStyle = &((XYHistogram *)toWInfo->histStyles)[0];
    	XYHistogram *fromStyle = &((XYHistogram *)fromWindow->histStyles)[0];
    	memcpy(toWInfo->histStyles, histStyles, sizeof(XYHistogram));
	toStyle->name = XmStringCopy(fromStyle->name);
    	/* Allocate color and set pixel value in curves structure */
	toStyle->fillPixel = black;
	if (fromStyle->fillPixel != black)
	    duplicateColor(display, cmap, fromStyle->fillPixel,
			   &toStyle->fillPixel);
	toStyle->linePixel = black;
	if (fromStyle->linePixel != black)
	    duplicateColor(display, cmap, fromStyle->linePixel,
			   &toStyle->linePixel);
    	++histStyles;
    }

    /* If there are overlaid plots, lay them on top of the original */
    for  (i = 1; i < fromWindow->nPlots; ++i) {
        pInfo = fromWindow->pInfo[i];
	if (pInfo->csi.colorScale)
	    csInfo = &pInfo->csi;
	else
	    csInfo = NULL;
        item = GetMPItemByID(pInfo->id);
        if (item == NULL)
            return False;
	nCurves = NCurvesInPlot(pInfo->plotType, pInfo->ntVars);
#ifdef DEBUG
	printf("nCurves = %d, PlotIsHist = %d, id = %d, plotType = %d\n",
	      nCurves, PlotIsHist(pInfo->plotType), pInfo->id, pInfo->plotType);
#endif /*DEBUG*/
	OverlayPlot(toWInfo, item, pInfo->plotType, pInfo->ntVars,
	    	pInfo->sliderVars,  pInfo->nXBins, pInfo->nYBins,
        	pInfo->errorBars, curveStyles, nCurves, histStyles, csInfo, NULL);
	curveStyles += nCurves;
	histStyles += PlotIsHist(pInfo->plotType);
    }

    /* Copy plot information not handled by AddHistToMultiPlot or OverlayPlot */
    /*      and check if we'll need to rebin 				      */
    rebin = (fromWindow->cellNormMin != toWInfo->cellNormMin) || 
		    (fromWindow->cellNormMax != toWInfo->cellNormMax);
    for  (i = 0; i < fromWindow->nPlots; ++i) {
	fromPInfo = fromWindow->pInfo[i];
    	toPInfo = toWInfo->pInfo[i];
    	if (!rebin)
	    rebin = (fromPInfo->nXBins != toPInfo->nXBins) || 
		    (fromPInfo->nYBins != toPInfo->nYBins);
	memcpy(toPInfo, fromPInfo, sizeof(plotInfo));
    }
    toWInfo->update       = fromWindow->update;
    toWInfo->needsUpdate  = fromWindow->needsUpdate;
    toWInfo->needsData    = fromWindow->needsData;
    toWInfo->titleStarred = fromWindow->titleStarred;
    toWInfo->growOnly     = fromWindow->growOnly;
    toWInfo->cellNormMin  = fromWindow->cellNormMin;
    toWInfo->cellNormMax  = fromWindow->cellNormMax;
    for  (i = 0; i < N_SLIDERS; ++i) {
    	toWInfo->sliderMin[i] = fromWindow->sliderMin[i];
    	toWInfo->sliderMax[i] = fromWindow->sliderMax[i];
    }
    matchMenuToggle(fromWindow->thickenMenuItem, toWInfo->thickenMenuItem);
    matchMenuToggle(fromWindow->binEdgeMenuItem, toWInfo->binEdgeMenuItem);
    matchMenuToggle(fromWindow->backplanesMenuItem,toWInfo->backplanesMenuItem);
    matchMenuToggle(fromWindow->cntrOfGravMenuItem,toWInfo->cntrOfGravMenuItem);
    matchMenuToggle(fromWindow->growOnlyMenuItem, toWInfo->growOnlyMenuItem);
    matchMenuToggle(fromWindow->autoUpMenuItem, toWInfo->autoUpMenuItem);
    matchMenuToggle(fromWindow->splitHalfMenuItem, toWInfo->splitHalfMenuItem);
    matchMenuToggle(fromWindow->cntrOfGravMenuItem,toWInfo->cntrOfGravMenuItem);
    matchMenuToggle(fromWindow->gaussErrorMenuItem,toWInfo->gaussErrorMenuItem);
    matchMenuToggle(fromWindow->errorDataMenuItem,toWInfo->errorDataMenuItem);
    matchStyles(fromWindow, toWInfo);
    toWInfo->decor = copyOverlayedObjectList(fromWindow->decor,
                                             multWin->multWidget);
#ifdef DEBUG
    chkWinInfos(toWInfo, fromWindow);
    printf("toWInfo->nCurves = %d, toWInfo->curveStyles = %X\n", 
    	toWInfo->nCurves, toWInfo->curveStyles);
    printf("toWInfo->nHists = %d, toWInfo->histStyles = %X\n", 
    	toWInfo->nHists, toWInfo->histStyles);
    if (toWInfo->nCurves != fromWindow->nCurves || 
    	  toWInfo->nHists != fromWindow->nHists)
    	printf(
    "WARNING: to: #Curves = %d, #Hists = %d; from: #Curves = %d, #Hists = %d\n",
    toWInfo->nCurves, toWInfo->nHists, fromWindow->nCurves, fromWindow->nHists);
#endif /*DEBUG*/

    /* Redraw everything (... this could be avoided) */
    RedisplayPlotWindow(toWInfo, REINIT);
/*
 *     if (rebin)
 *     	   RedisplayPlotWindow(toWInfo, REBIN);
 *     RedisplayPlotWindow(toWInfo, UPDATE);
 */
    /* REINIT'ing does its own thing with the legend, so instead make the
       legend be the same as the dragged plot */
    XtVaGetValues(fromWindow->widget, XmNxAxisLabel, &xLabel,
	    XmNyAxisLabel, &yLabel, XmNshowLegend, &showLegend, NULL);
    matchMenuToggle(fromWindow->legendMenuItem, toWInfo->legendMenuItem);
    XtVaSetValues(toWInfo->widget, XmNxAxisLabel, xLabel,
	    XmNyAxisLabel, yLabel, XmNshowLegend, showLegend, NULL);
    
    /* Do limits last & anything else stored in the widget because REINIT'ing
       nullifies stuff computed/stored in the widget */
    getLogSettings(fromWindow->widget, &logX, &logY, &logZ);
    XtVaSetValues(toWInfo->widget, XmNlogScaling, logY,
    	    XmNxLogScaling, logX,
    	    XmNyLogScaling, logY,
    	    XmNzLogScaling, logZ, NULL);
    matchPlotLimits(fromWindow->widget, toWInfo->widget);
    
    return True;
}

/*
** Version of XmToggleButtonSetState which checks for "w" being NULL
** first before trying to set the button state.  Used for safety so that
** if an improper keyword tries to set a button that doesn't exist, the
** program won't respond with a crash.
*/
static void matchMenuToggle(Widget fromWidget, Widget toWidget)
{
    if (fromWidget != NULL) {
#ifdef DEBUG
    	if (toWidget == NULL) {
    	    printf("ERROR: dragPlot.c:matchMenuToggle, fromWidget = %X.\n",
    	    	fromWidget);
    	    return; 
    	}
#endif /*DEBUG*/
    	XmToggleButtonSetState(toWidget, XmToggleButtonGetState(fromWidget), 
    	    True);
   }
}

/*
** Set the plot limits and rotation for a plot widget.  Values for which
** the keyword was not specified are left to the widget's default setting.
*/
static void matchPlotLimits(Widget fromPlotW, Widget toPlotW)
{
    double origMinXLim, origMaxXLim, origMinYLim, origMaxYLim;
    double origMinZLim, origMaxZLim, origAngle1, origAngle2, origAngle3;
    double newMinXLim, newMaxXLim, newMinYLim, newMaxYLim;
    double newMinZLim, newMaxZLim, newAngle1, newAngle2, newAngle3;
    WidgetClass class = XtClass(toPlotW);

#ifdef DEBUG
    WidgetClass fromClass = XtClass(fromPlotW);    
    if (class != fromClass) {
    	printf("ERROR: dragPlot.c:matchPlotLimits, class = %X.\n", class);
    	return;
    }
#endif /*DEBUG*/
    
    /* Get the default (original) plotting bounds the plot widget */
    /* If a limit had been changed, set limits for the new widget */
    if (class == scatWidgetClass) {
    	ScatGetVisibleRange(fromPlotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    	ScatGetVisibleRange(toPlotW, &newMinXLim, &newMinYLim,
    		&newMaxXLim, &newMaxYLim);
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim)
    	    ScatSetVisibleRange(toPlotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == h1DWidgetClass) {
    	H1DGetVisibleRange(fromPlotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    	H1DGetVisibleRange(toPlotW, &newMinXLim, &newMinYLim,
    		&newMaxXLim, &newMaxYLim);
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim)
    	    H1DSetVisibleRange(toPlotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == xyWidgetClass) {
    	XYGetVisibleRange(fromPlotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    	XYGetVisibleRange(toPlotW, &newMinXLim, &newMinYLim,
    		&newMaxXLim, &newMaxYLim);
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim)
    	    XYSetVisibleRange(toPlotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == cellWidgetClass) {
    	CellGetVisibleRange(fromPlotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    	CellGetVisibleRange(toPlotW, &newMinXLim, &newMinYLim,
    		&newMaxXLim, &newMaxYLim);
#ifdef DEBUG
    	printf("Old Cell Plot Limits: %g, %g, %g, %g\n", origMinXLim, 
    		origMinYLim, origMaxXLim, origMaxYLim);
    	printf("New Cell Plot Limits: %g, %g, %g, %g\n", newMinXLim, 
    		newMinYLim, newMaxXLim, newMaxYLim);  
#endif /*DEBUG*/
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim)
    	    CellSetVisibleRange(toPlotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == hist2DWidgetClass) {
    	hist2DGetVisiblePart(fromPlotW, &origMinXLim, &origMaxXLim,
    		&origMinYLim, &origMaxYLim, &origMinZLim, &origMaxZLim);
    	hist2DGetVisiblePart(toPlotW, &newMinXLim, &newMaxXLim,
    		&newMinYLim, &newMaxYLim, &newMinZLim, &newMaxZLim);
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim ||
    		newMinZLim != origMinZLim || newMaxZLim != origMaxZLim)
    	    hist2DSetVisiblePart(toPlotW, origMinXLim, origMaxXLim,
    		origMinYLim, origMaxYLim, origMinZLim, origMaxZLim);
	hist2DGetViewAngles(fromPlotW, &origAngle1, &origAngle2);
	hist2DGetViewAngles(toPlotW, &newAngle1, &newAngle2);
    	if (newAngle1 != origAngle1 || newAngle2 != origAngle2)
    	    hist2DSetViewAngles(toPlotW, origAngle1, origAngle2);
    } else if (class == scat3DWidgetClass) {
	float origMinXLim, origMaxXLim, origMinYLim, origMaxYLim;
	float origMinZLim, origMaxZLim;
	float newMinXLim, newMaxXLim, newMinYLim, newMaxYLim;
	float newMinZLim, newMaxZLim;
    	Scat3DGetVisiblePart(fromPlotW, &origMinXLim, &origMaxXLim,
    		&origMinYLim, &origMaxYLim, &origMinZLim, &origMaxZLim);
    	Scat3DGetVisiblePart(toPlotW, &newMinXLim, &newMaxXLim,
    		&newMinYLim, &newMaxYLim, &newMinZLim, &newMaxZLim);
    	if (newMinXLim != origMinXLim || newMinYLim != origMinYLim ||
    		newMaxXLim != origMaxXLim || newMaxYLim != origMaxYLim ||
    		newMinZLim != origMinZLim || newMaxZLim != origMaxZLim)
    	    Scat3DSetVisiblePart(toPlotW, origMinXLim, origMaxXLim,
    		origMinYLim, origMaxYLim, origMinZLim, origMaxZLim);
	Scat3DGetViewEulerAngles(fromPlotW, &origAngle1, &origAngle2, 
		&origAngle3);
	Scat3DGetViewEulerAngles(toPlotW, &newAngle1, &newAngle2, 
		&newAngle3);
    	if (newAngle1 != origAngle1 || newAngle2 != origAngle2 || 
    		newAngle3 != origAngle3)
    	    Scat3DSetViewEulerAngles(toPlotW,origAngle1,origAngle2,origAngle3);
    }
}

/*
** Find out if any axis of the plot widget "plotW" is set for log scaling.
*/
static void getLogSettings(Widget plotW, Boolean *xLog, Boolean *yLog, 
	Boolean *zLog)
{
    WidgetClass class = XtClass(plotW);

    *xLog = *yLog = *zLog = False;
    if (class == h1DWidgetClass)
    	XtVaGetValues(plotW, XmNlogScaling, yLog, NULL);
    else if (class == hist2DWidgetClass)
    	XtVaGetValues(plotW, XmNzLogScaling, zLog, NULL);
    else if (class == scat3DWidgetClass)
    	XtVaGetValues(plotW, XmNxLogScaling, xLog, XmNyLogScaling, yLog,
    	        XmNzLogScaling, zLog, NULL);
    else
    	XtVaGetValues(plotW, XmNxLogScaling, xLog, XmNyLogScaling, yLog, NULL);
}
/*
** Set marker, line, and histogram styles and colors for XY plots AFTER
**		the first plot.
*/
static Boolean matchStyles(windowInfo *fromWInfo, windowInfo *toWInfo)
{
    Boolean styleChanged = False;
    plotInfo *fromPInfo = fromWInfo->pInfo[0];
    plotInfo *toPInfo = toWInfo->pInfo[0];
    Pixel black;
    Display *display;
    Colormap cmap;

    /* Obtain the color name & pixel tables for the XY Plots */
    if (XtClass(toWInfo->widget) != xyWidgetClass)
    	return False;

    display = XtDisplay(fromWInfo->widget);
    cmap = DefaultColormapOfScreen(XtScreen(fromWInfo->widget));
    black = BlackPixelOfScreen(XtScreen(fromWInfo->widget));

    if (XtClass(toWInfo->widget)==xyWidgetClass && toWInfo->curveStyles!=NULL) {
    	int i = fromWInfo->nCurves-NCurvesInPlot(fromPInfo->plotType, 
    	    fromPInfo->ntVars);
    	int j = toWInfo->nCurves-NCurvesInPlot(toPInfo->plotType,
    	    toPInfo->ntVars);
    	XYCurve *toStyle; 
    	XYCurve *toStyles = &((XYCurve *)toWInfo->curveStyles)[j];
    	XYCurve *fromStyle; 
    	XYCurve *fromStyles = &((XYCurve *)fromWInfo->curveStyles)[i];
#ifdef DEBUG
    	if (i != j)
    	    printf("\nInternal error in matchStyles\n");
#endif /*DEBUG*/
    	for (toStyle = toStyles, fromStyle = fromStyles; 
    		i < fromWInfo->nCurves && i<toWInfo->nCurves && i<MAX_DISP_VARS;
    		i++, toStyle++, fromStyle++) {
    	    if (toStyle->markerStyle != fromStyle->markerStyle) {
    	    	toStyle->markerStyle = fromStyle->markerStyle;
    	    	styleChanged = True;
    	    }
    	    if (toStyle->lineStyle != fromStyle->lineStyle) {
    	    	toStyle->lineStyle = fromStyle->lineStyle;
    	    	styleChanged = True;
    	    }
    	    if (toStyle->markerSize != fromStyle->markerSize) {
    	    	toStyle->markerSize = fromStyle->markerSize;
    	    	styleChanged = True;
    	    }
    	    /* Allocate color and set pixel value in curves structure */
	    toStyle->markerPixel = black;
	    if (fromStyle->markerPixel != black)
	    {
		duplicateColor(display, cmap, fromStyle->markerPixel,
			       &toStyle->markerPixel);
    	    	styleChanged = True;
	    }
	    toStyle->linePixel = black;
	    if (fromStyle->linePixel != black)
	    {
		duplicateColor(display, cmap, fromStyle->linePixel,
			       &toStyle->linePixel);
		styleChanged = True;
	    }
	}
    }
    if (XtClass(toWInfo->widget)==xyWidgetClass && toWInfo->histStyles!=NULL &&
    	    PlotIsHist(toPInfo->plotType) != 0) {
    	XYHistogram *toStyle = &((XYHistogram *)toWInfo->histStyles)[0];
    	XYHistogram *fromStyle = &((XYHistogram *)fromWInfo->histStyles)[0];
    	if (toStyle->fillStyle != fromStyle->fillStyle) {
    	    toStyle->fillStyle = fromStyle->fillStyle;
    	    styleChanged = True;
    	}
    	if (toStyle->lineStyle != fromStyle->lineStyle) {
    	    toStyle->lineStyle = fromStyle->lineStyle;
    	    styleChanged = True;
    	}
    	/* Allocate color and set pixel value in curves structure */
	toStyle->fillPixel = black;
	if (fromStyle->fillPixel != black)
	{
	    duplicateColor(display, cmap, fromStyle->fillPixel,
			   &toStyle->fillPixel);
	    styleChanged = True;
	}
	toStyle->linePixel = black;
	if (fromStyle->linePixel != black)
	{
	    duplicateColor(display, cmap, fromStyle->linePixel,
			   &toStyle->linePixel);
	    styleChanged = True;
	}
    }
    return styleChanged;
}

#ifdef DEBUG
static void chkWinInfos(windowInfo *wInfo1, windowInfo *wInfo2)
{
    printf("\n    Checking windowInfo's...\n");
    if (wInfo1->nPlots != wInfo2->nPlots)
    	printf("      # Plots:  %d != %d\n", wInfo1->nPlots, wInfo2->nPlots);
    if (wInfo1->update != wInfo2->update)
    	printf("      Update\n");
    if (wInfo1->needsUpdate != wInfo2->needsUpdate)
    	printf("      needsUpdate\n");
    if (wInfo1->needsData != wInfo2->needsData)
    	printf("      needsData\n");
    if (wInfo1->titleStarred != wInfo2->titleStarred)
    	printf("      titleStarred\n");
    if (wInfo1->growOnly != wInfo2->growOnly)
    	printf("      growOnly\n");
    if (wInfo1->nCurves != wInfo2->nCurves)
    	printf("      # Curves:  %d != %d\n", wInfo1->nCurves, wInfo2->nCurves);
    if (wInfo1->nHists != wInfo2->nHists)
    	printf("      # Hists:  %d != %d\n", wInfo1->nHists, wInfo2->nHists);
    if (wInfo1->cellNormMax != wInfo2->cellNormMax)
    	printf("      cellNormMax:  %f != %f\n", wInfo1->cellNormMax, 
    		wInfo2->cellNormMax);
    printf("\n");
}
#endif /* DEBUG */

static int duplicateColor(Display *display, Colormap cmap,
			  Pixel in, Pixel *out)
{
    XColor xcolor;

    xcolor.pixel = in;
    XQueryColor(display, cmap, &xcolor);
    if (!XAllocColor(display, cmap, &xcolor))
    {
	fprintf(stderr, "\nCan't allocate color: all colorcells allocated"
		" and no matching cell found.\n");
	return 1;
    }
    *out = xcolor.pixel;
    return 0;
}

#else /* In Motif versions below 1.2, drag & drop is futile, just make stubs */
void AddActionsForDraggingPlots(XtAppContext appContext) {}
void RegisterWidgetAsPlotReceiver(Widget w) {}
void UnRegisterWidgetAsPlotReceiver(Widget w) {}
void MakePlotADragSource(Widget w) {}
#endif /* XmVersion >= 1002*/
