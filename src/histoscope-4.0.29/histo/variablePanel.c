/*******************************************************************************
*									       *
* variablePanel.c -- Histoscope create/modify variable window		       *
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
* May 12, 1995								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <float.h>
#include <stdio.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/CascadeB.h>
#include <Xm/Label.h>
#include <Xm/SelectioB.h>
#include <Xm/List.h>
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/help.h"
#include "histoP.h"
#include "help.h"
#include "interpret.h"
#include "variablePanel.h"
#include "configFile.h"
#include "parse.h"
#include "plotWindows.h"
#include "ntuplePanel.h"
#include "mainPanel.h"
#include "multPlot.h"
#include "auxWindows.h"

#define N_FUNCTIONS 13
#define N_CONSTANTS 4
#define PULLDOWN_LIMIT 30	/* Max entries that will fit in a pulldown */

typedef struct varPanelRec {
    struct varPanelRec *next;
    char name[HS_MAX_NAME_LENGTH+1];
    int id;
    Widget shell, form, createBtn, modifyBtn, deleteBtn, nameText, exprText;
    Widget pasteVarBtn, pasteVarPane, pasteVarMenu, pasteVarDialog;
} varPanel;

typedef struct {
    char *menuName;
    char mnemonic;
    char *fnName;
} fnInfo;

typedef struct {
    char *menuName;
    char mnemonic;
    char *constName;
} constInfo;

static int evaluateDependentVariables(Program *program, hsNTuple *ntuple,
	ntupleExtension *ntExt, int index, char **errMsg);
static int checkRefsTo(ntupleExtension *ntExt, int searchFor, int searchIn,
	char *checked);
static void updateVariablePanel(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt);
static Widget createPasteVarPane(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt);
static Widget createPasteVarDialog(varPanel *panel);
static Widget updatePasteVarDialog(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt);
static void pasteVarCB(Widget w, varPanel *panel, caddr_t callData);
static void createCB(Widget w, varPanel *panel, caddr_t callData);
static void modifyCB(Widget w, varPanel *panel, caddr_t callData);
static void deleteCB(Widget w, varPanel *panel, caddr_t callData);
static void dismissCB(Widget w, varPanel *panel, caddr_t callData);
static void destroyCB(Widget w, varPanel *panel, caddr_t callData);
static void pasteFnCB(Widget w, varPanel *panel, caddr_t callData);
static void pasteConstCB(Widget w, varPanel *panel, caddr_t callData);
static void pasteFnString(varPanel *panel, char *fnName);
static void pasteVarString(varPanel *panel, char *varName);
static void varMenuBtnCB(Widget w, varPanel *panel, caddr_t callData);
static void varOkCB(Widget w, varPanel *panel, caddr_t callData);
static void varDismissCB(Widget w, varPanel *panel, caddr_t callData);
static void removeFromVarPanelList(varPanel *panel);
static void reinitPlotsWithVar(int id, int varIndex);
static void cleanName(char *string);

/* List of all ntuple extensions */
static ntupleExtension *ExtList = NULL;

/* List of all variable panels currently displayed */
static varPanel *VarPanelList;

/* Variables for maintaining statistics on interpreter errors occuring on
   extended ntuple variable references (see CheckExtNTRefErrors) */
static int ExtNTRefErrors = False;
static char *LastExtNTRefErrMsg = NULL;

/* Table of function information for the Paste Function menu */
static fnInfo FnData[N_FUNCTIONS] = {
    {"Sin", 'S', "sin"},
    {"Cos", 'C', "cos"},
    {"Tangent", 'T', "tan"},
    {"Arc Sin", 0, "asin"},
    {"Arc Cos", 0, "acos"},
    {"Arc Tangent", 0, "atan"},
    {"Log Base e", 'L', "log"},
    {"Log Base 10", '1', "log10"},
    {"Exponential", 'E', "exp"},
    {"Square Root", 'q', "sqrt"},
    {"Round to Int", 'I', "int"},
    {"Random", 'R', "rand"},
    {"Absolute Value", 'A', "abs"}};

/* Table of constant information for the Paste Function menu */
static constInfo ConstData[N_CONSTANTS] = {
    {"Pi", 'P', "pi"},
    {"e", 'e', "e"},
    {"Euler Gamma", 'G', "gamma"},
    {"Degree/Radian", 'D', "deg"}};
    
/*
** Add a derived variable to an ntuple, given the name for the new variable,
** the expression it represents, and the program compiled from the expression.
** If the name matches an existing variable name, just replace it.
** This does NOT resolve the variable references implied in the expression
** and program, ResolveVariableReferences should be called before the new
** variable can be used.
** 
** Note that the name, expression, and program are NOT COPIED.  The calling
** routine must allocate these and pass ownership to this routine.  The memory
** will be freed when ClearNTupleExtensions is called.
*/
int AddUserVariable(hsNTuple *ntuple, char *name, char *expression,
	Program *program)
{
    int i, var = -1;
    ntupleExtension *ntExt;
    userDefNTVar *newVars;
    
    /* Find the extension list corresponding to the requested ntuple */
    for (ntExt=ExtList; ntExt!=NULL; ntExt=ntExt->next)
    	if (ntExt->ntupleID == ntuple->id)
    	    break;
    
    /* If no list was found, begin a new one */
    if (ntExt == NULL) {
	ntExt = (ntupleExtension *)XtMalloc(sizeof(ntupleExtension));
	ntExt->vars = NULL;
	ntExt->nVars = 0;
	ntExt->ntupleID = ntuple->id;
	ntExt->next = ExtList;
	ExtList = ntExt;
    }
    
    /* Search the extension for the named variable */
    for (i=0; i<ntExt->nVars; i++) {
    	if (!strcmp(name, ntExt->vars[i].name))
    	    var = i;
    }
    
    /* If the name wasn't found, make room for a new variable by
       re-allocating the variable list to extend it.  If it was
       found, free the previous entries */
    if (var == -1) {
	newVars = (userDefNTVar *)XtMalloc(sizeof(userDefNTVar) *
		(ntExt->nVars+1));
	for (i=0; i<ntExt->nVars; i++)
    	    newVars[i] = ntExt->vars[i];
	if (ntExt->vars != NULL)
    	    XtFree((char *)ntExt->vars);
	ntExt->vars = newVars;
	var = ntExt->nVars++;
    } else {
        XtFree(ntExt->vars[var].name);
	XtFree(ntExt->vars[var].expression);
	FreeProgram(ntExt->vars[var].program);
    }
    
    /* Write the new information into the list */
    ntExt->vars[var].name = name;
    ntExt->vars[var].expression = expression;
    ntExt->vars[var].program = program;
    return 0;
}

/*
** Delete a user defined variable from an ntuple extension, update
** any displayed variable or ntuple panels whose variable lists may be
** affected (this includes fixing up references by ntuple index in any
** displayed plot windows)
*/
void DeleteUserVariable(hsNTuple *ntuple, ntupleExtension *ntExt, char *name)
{
    windowInfo *w;
    plotInfo *pInfo;
    int i, p, deleteExtVarIndex = -1, deleteNTVarIndex, var;
    
    if (ntExt == NULL)
    	return;
    
    /* Search the extension for the named variable */
    for (i=0; i<ntExt->nVars; i++) {
    	if (!strcmp(name, ntExt->vars[i].name))
    	    deleteExtVarIndex = i;
    }
    if (deleteExtVarIndex == -1)
    	return;
    
    /* Move the entries after the variable down to fill the void */
    for (i=deleteExtVarIndex; i<ntExt->nVars-1; i++)
    	ntExt->vars[i] = ntExt->vars[i+1];
    ntExt->nVars--;
    
    /* In plot windows, variable references are by ntuple index.  Renumber
       variable index arrays in displayed plots (ntVars and sliderVars) so
       the variable references remain correct */
    deleteNTVarIndex = deleteExtVarIndex + ntuple->nVariables;
    for (w=WindowList; w!=NULL; w=w->next) {
    	for (p=0; p<w->nPlots; p++) {
    	    pInfo = w->pInfo[p];
    	    for (var=0; var<MAX_DISP_VARS; var++) {
    		if (pInfo->ntVars[var] >= deleteNTVarIndex)
    	    	    pInfo->ntVars[var]--;
    	    }
    	    for (var=0; var<N_SLIDERS; var++) {
    		if (pInfo->sliderVars[var] >= deleteNTVarIndex)
    	    	    pInfo->sliderVars[var]--;
    	    }
    	}
    }
    
    /* Fix up references to it in other variables */
    for (i=0; i<ntExt->nVars; i++)
    	ResolveVariableReferences(ntExt->vars[i].program, ntuple, ntExt);
    
    /* Update displayed panels with the new variable list */
    UpdateVariablePanels(ntuple, ntExt);
    UpdateNTuplePanelList(ntuple->id);
    
    /* If the ntuple extension is now empty, remove it */
    if (ntExt->nVars == 0)
    	RemoveNTupleExtension(ntExt->ntupleID);
}

/*
** Check an ntuple extension for circular references in user variables.
** if one is found, return True and store pointer to name in errVar.  This
** uses user variable indicies, so be sure that ResolveVariableReferences
** has been called on all of the variables in the extension before calling this
*/
int CheckForCircularReferences(ntupleExtension *ntExt, char **errVar)
{
    int i, var;
    char *checked;
    
    /* Create an array of markers to prevent infinite recursion in searches
       in case there's a circularity */
    checked = XtMalloc(sizeof(char) * ntExt->nVars);
    
    /* Loop over each variable, recursively searching variables referenced
       in the variable's program for references to the variable */
    for (var=0; var<ntExt->nVars; var++) {
    	for (i=0; i<ntExt->nVars; i++)
	    checked[i] = False;
    	if (checkRefsTo(ntExt, var, var, checked)) {
    	    *errVar = ntExt->vars[var].name;
    	    XtFree(checked);
    	    return True;
    	}
    }
    XtFree(checked);
    return False;
}

/*
** Recursively check variables referenced by the expression in the variable
** "searchIn" for references to variables with the index "searchFor".
** Keep track of variables which have already been examined (to prevent
** infinite recursion if there are circularities) in the array "checked".
** 
*/
static int checkRefsTo(ntupleExtension *ntExt, int searchFor, int searchIn,
	char *checked)
{
    Symbol *s;
    
    /* Make sure we haven't examined this variable before */
    if (checked[searchIn])
        return False;
    checked[searchIn] = True;
    
    /* Look for searchFor in the variables referenced in searchIn, and
       recursively in any that they in turn reference */
    for (s=ntExt->vars[searchIn].program->externSymList; s!=NULL; s=s->next) {
	if (s->type == USER_DEF_VAR) {
	    if (s->index == searchFor)
        	return True;
	    if (checkRefsTo(ntExt, searchFor, s->index, checked))
    	        return True;
	}
    }
    return False;
}

/*
** Fetch the ntuple extension corresponding to ntuple with id "id".  This
** searches a list of extensions, so for good performance with large numbers
** of extended ntuple references, call this once, and save the result, rather
** than doing: ExtNtRef(ntuple, GetNTupleExtension(ntuple->id), index).
** The result will be valid until the next call to SetNtupleExtension.
*/
ntupleExtension *GetNTupleExtension(int id)
{
    ntupleExtension *ntExt;
    
    for (ntExt=ExtList; ntExt!=NULL; ntExt=ntExt->next)
    	if (ntExt->ntupleID == id)
    	    return ntExt;
    return NULL;
}

/*
** Read the value of an ntuple variable OR a derived variable in the
** extended ntuple data structure.  (Extended ntuple reference)
*/
int ExtNTRef(hsNTuple *ntuple, ntupleExtension *ntExt, int var, int index,
	float *value)
{
    /* int extIndex; */
    double result;
    char *errMsg;
    Program *program;
    
    /* If it's a plain ntuple reference, just return the ntuple value */
    if (var < ntuple->nVariables) {
    	*value = NTRef(ntuple, var, index);
    	return True;
    }
    
    /* If it's a reference to a derived variable, execute the program to
       derive the value.  Ignore all errors. */
    program = ntExt->vars[var - ntuple->nVariables].program;
    evaluateDependentVariables(program, ntuple, ntExt, index, &errMsg);
    if (!ExecuteProgram(program, &result, &errMsg)) {
    	ExtNTRefErrors = True;
    	LastExtNTRefErrMsg = errMsg;
    	*value = result;
    	return False;
    }
    *value = result;
    return True;
}

/*
** Clear the error counter for extended ntuple variable references
** (see CheckExtNTRefErrors).
*/
int ResetExtNTRefErrors(void)
{
    ExtNTRefErrors = False;
    LastExtNTRefErrMsg = NULL;
    return 0;
}

/*
** Report on interpreter errors in extended ntuple variable references since
** the last call to ResetExtNTRefErrors.  Return False if no errors have
** occured.  If there were errors, return True and return a copy of the last
** reported error in the parameter "lastMessage"
*/
int CheckExtNTRefErrors(char **lastMessage)
{
    if (ExtNTRefErrors)
    	*lastMessage = LastExtNTRefErrMsg;
    return ExtNTRefErrors;
}

/*
** Remove and dealocate an ntuple extension without checking for consequences. 
*/
void RemoveNTupleExtension(int id)
{
    ntupleExtension *temp, *extToDelete = NULL;
    int var;

    /* Find the extension and remove it from the list */
    if (ExtList == NULL)
        return;
    if (ExtList->ntupleID == id) {
	extToDelete = ExtList;
	ExtList = ExtList->next;
    } else {
	for (temp = ExtList; temp->next != NULL; temp = temp->next) {
	    if (temp->next->ntupleID == id) {
		extToDelete = temp->next;
		temp->next = temp->next->next;
		break;
	    }
	}
    }
    if (extToDelete == NULL)
    	return;
    	
    /* Deallocate all of the memory associated with the extension */
    for (var=0; var<extToDelete->nVars; var++) {
    	XtFree(extToDelete->vars[var].name);
    	XtFree(extToDelete->vars[var].expression);
    	FreeProgram(extToDelete->vars[var].program);
    }
    XtFree((char *)extToDelete->vars);
    XtFree((char *)extToDelete);
}

char *ExtNTVarName(hsNTuple *ntuple, int var)
{
    ntupleExtension *ntExt;
    
    if (var < ntuple->nVariables)
    	return ntuple->names[var];
    
    ntExt = GetNTupleExtension(ntuple->id);
    return ntExt->vars[var - ntuple->nVariables].name;
}

void ExtCalcNTVarRange(hsNTuple *ntuple, ntupleExtension *ntExt, int var,
	float *minReturn, float *maxReturn)
{
    int i;
    float value, min = FLT_MAX, max = -FLT_MAX;

    /* If it's a plain ntuple reference, just return the ntuple value */
    if (var < ntuple->nVariables) {
    	CalcNTVarRange(ntuple, var, minReturn, maxReturn);
    	return;
    }
    
    /* If it's a user defined variable, CalcNTVarRange is duplicated
       here using ExtNTRef instaed of NTRef */
    for (i=0; i<ntuple->n; i++) {
    	if (ExtNTRef(ntuple, ntExt, var, i, &value)) {
    	    if (value < min)
    		min = value;
    	    if (value > max)
    		max = value;
    	}
    }
    *minReturn = min; *maxReturn = max;
}

/*
** Scan through the current data in ntuple and evaluate the user defined
** variable indicated by "varIndex" for all values, stopping and returning
** False with an error message in "errMsg" if an error occurs along the way
*/
int TestUserVariable(hsNTuple *ntuple, ntupleExtension *ntExt, int varIndex,
	char **errMsg)
{
    double result;
    Program *program;
    int i;
    
    program = ntExt->vars[varIndex].program;
    
    for (i=0; i<ntuple->n; i++) {
    	if (!evaluateDependentVariables(program, ntuple, ntExt, i, errMsg)) {
    	    *errMsg = "Unable to evaluate dependent variable";
    	    return False;
    	}
        if (!ExecuteProgram(program, &result, errMsg))
            return False;
    }
    return True;
}

/*
** Put up a dialog for creating or modifying a user defined variable.  
** The dialog has two modes, create and modify.  If 'name' is NULL, the
** panel appears initially in "create" mode.
*/
void CreateVariablePanel(Widget parent, hsNTuple *ntuple, char *name)
{
    Widget form, btn, lbl, fnPane, pasteFnMenu, constPane, pasteConstMenu;
    Widget dismissBtn;
    XmString s1;
    int ac, i;
    Arg args[20];
    char *title;
    varPanel *panel;
    ntupleExtension *ntExt = GetNTupleExtension(ntuple->id);

    /* If the panel already exists, just pop it to the top */
    if (name != '\0') {
	for (panel=VarPanelList; panel!=NULL; panel=panel->next) {
    	    if (ntuple->id == panel->id && !strcmp(panel->name, name)) {
    		XMapRaised(XtDisplay(panel->shell), XtWindow(panel->shell));
    		XSetInputFocus(XtDisplay(panel->shell), XtWindow(panel->shell),
    	    		RevertToParent, CurrentTime);
    		XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
    		return;
    	    }
	}
    }
    
    /* Create a data structure to hold the information about the panel,
       and add it to the list of variable panels */
    panel = (varPanel *)XtMalloc(sizeof(varPanel));
    if (name == NULL)
    	panel->name[0] = '\0';
    else
    	strcpy(panel->name, name);
    panel->id = ntuple->id;
    panel->next = VarPanelList;
    VarPanelList = panel;
    
    /* Create a popup dialog shell with a form for laying out the window */
    ac = 0;
    XtSetArg(args[ac], XmNautoUnmanage, False); ac++;
    form = XmCreateFormDialog(parent, "variablePanel", args, ac);
    panel->form = form;
    panel->shell = XtParent(form);
    title = name == NULL ? "Create Variable" : name;
    XtVaSetValues(panel->shell, XmNtitle, title, NULL);
    XtAddCallback(form, XmNdestroyCallback, (XtCallbackProc)destroyCB, panel);
    AddMotifCloseCallback(panel->shell, (XtCallbackProc)dismissCB, panel);

    /* Create the field for entering the variable name */
    lbl = XtVaCreateManagedWidget("nameLbl", xmLabelWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Variable Name"),
    	    XmNmnemonic, 'N',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, 2, NULL);
    XmStringFree(s1);
    panel->nameText = XtVaCreateManagedWidget("nameText",
    	    xmTextWidgetClass, form,
    	    XmNmaxLength, HS_MAX_NAME_LENGTH,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 99,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, lbl, NULL);
    XtVaSetValues(lbl, XmNuserData, panel->nameText, NULL);
    	    
    /* Create bottom row of buttons */
    panel->createBtn = XtVaCreateManagedWidget("create",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Create"),
    	    XmNmnemonic, 'C',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 0,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(panel->createBtn, XmNactivateCallback,
    	    (XtCallbackProc)createCB, panel);
    XmStringFree(s1);
    panel->modifyBtn = XtVaCreateManagedWidget("modify",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Modify"),
    	    XmNmnemonic, 'M',
    	    XmNsensitive, name != NULL,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 20,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 40,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(panel->modifyBtn, XmNactivateCallback,
    	    (XtCallbackProc)modifyCB, panel);
    XmStringFree(s1);
    panel->deleteBtn = XtVaCreateManagedWidget("delete",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Delete"),
    	    XmNmnemonic, 'D',
    	    XmNsensitive, name != NULL,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 40,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 60,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(panel->deleteBtn, XmNactivateCallback,
    	    (XtCallbackProc)deleteCB, panel);
    XmStringFree(s1);
    dismissBtn = XtVaCreateManagedWidget("dismiss",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 60,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 80,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, (XtCallbackProc)dismissCB,
    	    panel);
    XmStringFree(s1);
    btn = XtVaCreateManagedWidget("help",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Help"),
    	    XmNmnemonic, 'H',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 80,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 100,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(btn, XmNactivateCallback,
    	    (XtCallbackProc)VariablePanelHelpCB, panel);
    XmStringFree(s1);

    /* Create the menu/dialog button for pasting variables */
    ac = 0;
    XtSetArg(args[ac], XmNmarginHeight, 0); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, 3); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, 33); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, panel->createBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    panel->pasteVarMenu = XmCreateMenuBar(form, "fnMenuBar", args, ac);
    panel->pasteVarPane = NULL;
    panel->pasteVarDialog = NULL;
    panel->pasteVarBtn = XtVaCreateManagedWidget("varMenuBtn",
    	    xmCascadeButtonWidgetClass, panel->pasteVarMenu, 
    	    XmNlabelString, s1=XmStringCreateSimple("Paste Variable..."),
    	    XmNmnemonic, 'V', NULL);
    XtAddCallback(panel->pasteVarBtn, XmNactivateCallback,
    	    (XtCallbackProc)varMenuBtnCB, panel);
    XmStringFree(s1);
    XtManageChild(panel->pasteVarMenu);
    
    /* Fill in the variable menu (or dialog) */
    updateVariablePanel(panel, ntuple, ntExt);
    
    /* Create the menu for pasting functions */
    ac = 0;
    XtSetArg(args[ac], XmNmarginHeight, 0); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, 35); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, 65); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, panel->createBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    pasteFnMenu = XmCreateMenuBar(form, "fnMenuBar", args, ac);
    fnPane = AddSubMenu(pasteFnMenu, "pasteFn", "Paste Function...", 'F');
    for (i=0; i<N_FUNCTIONS; i++) {
    	btn = AddMenuItem(fnPane, "fnBtn",FnData[i].menuName,FnData[i].mnemonic,
    		NULL, NULL, (XtCallbackProc)pasteFnCB, panel);
    	XtVaSetValues(btn, XmNuserData, (XtPointer)((long)i), NULL);
    }
    XtManageChild(pasteFnMenu);
    
    /* Create the menu for pasting constants */
    ac = 0;
    XtSetArg(args[ac], XmNmarginHeight, 0); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, 67); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, 97); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, panel->createBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    pasteConstMenu = XmCreateMenuBar(form, "constMenuBar", args, ac);
    constPane =
    	    AddSubMenu(pasteConstMenu, "pasteConst", "Paste Constant...", 'C');
    for (i=0; i<N_CONSTANTS; i++) {
    	btn = AddMenuItem(constPane, "constBtn", ConstData[i].menuName,
    		ConstData[i].mnemonic, NULL, NULL,
    		(XtCallbackProc)pasteConstCB, panel);
    	XtVaSetValues(btn, XmNuserData, (XtPointer)((long)i), NULL);
    }
    XtManageChild(pasteConstMenu);
    
    /* Create the text area for entering expressions */
    lbl = XtVaCreateManagedWidget("exprLbl", xmLabelWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Expression"),
    	    XmNmnemonic, 'E',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, panel->nameText, NULL);
    XmStringFree(s1);
    ac = 0;
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNrows, 6); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNtopWidget, lbl); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, 1); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, 99); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, pasteFnMenu); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 8); ac++;
    panel->exprText = XmCreateScrolledText(form, "exprText", args, ac);
    XtManageChild(panel->exprText);
    XtVaSetValues(lbl, XmNuserData, panel->exprText, NULL);

    /* Fill in the expression and name fields if this is a modify */
    if (name != NULL) {
	for (i=0; i<ntExt->nVars; i++) {
    	    if (!strcmp(name, ntExt->vars[i].name)) {
    		XmTextSetString(panel->nameText, name);
    		XmTextSetString(panel->exprText, ntExt->vars[i].expression);
    	    }
	}
    }
    
    /* Set the default and cancel buttons */
    XtVaSetValues(form, XmNcancelButton, dismissBtn, XmNdefaultButton,
    	name==NULL?panel->createBtn:panel->modifyBtn, NULL);

    /* Set the initial focus */
#if XmVersion >= 1002
    XtVaSetValues(form, XmNinitialFocus,
    	    name==NULL ? panel->nameText : panel->exprText, NULL);
#endif
    
    /* Add a handler for the button mnemonics */
    AddDialogMnemonicHandler(form);
    
    XtManageChild(form);
}

/*
** Update any displayed variable panels related to a particular ntuple
** to reflect changes in the variable list for the ntuple extension.
*/
void UpdateVariablePanels(hsNTuple *ntuple, ntupleExtension *ntExt)
{
    varPanel *p;
    
    /* Scan all of the displayed variable panels for ones with matching id */
    for (p=VarPanelList; p!=NULL; p=p->next) {
    	if (ntuple->id == p->id)
    	    updateVariablePanel(p, ntuple, ntExt);
    }
}

/*
** Write a configuration file entry for each existing ntuple extension
*/
void WriteNTupleExtensions(FILE *fp)
{
    ntupleExtension *ntExt;
    hsNTuple *ntuple;
    char *string;
    int i;
    
    for (ntExt=ExtList; ntExt!=NULL; ntExt=ntExt->next) {
	ntuple = (hsNTuple *)GetMPItemByID(ntExt->ntupleID);
	fprintf(fp, "NTupleExtension\n");
	if (ntuple->category != NULL && ntuple->category[0] != '\0')
    	    fprintf(fp, " Category %s\n", ntuple->category);
	if (ntuple->uid != 0)
    	    fprintf(fp, " UID %d\n", ntuple->uid);
	fprintf(fp, " Title %s\n", ntuple->title);
    	for (i=0; i<ntExt->nVars; i++) {
    	    fprintf(fp, " Name%d %s\n", i+1, ntExt->vars[i].name);
    	    string = XtMalloc(10 + strlen(ntExt->vars[i].expression));
    	    sprintf(string, " Expr%d %s", i+1, ntExt->vars[i].expression);
    	    WriteMultilineString(fp, string);
    	    XtFree(string);
    	    fprintf(fp, "\n");
    	}
    }
}

/*
** Change the id of an ntuple extension (for re-numbering ntuple ids)
*/
void ChangeNTupleExtensionItemID(int oldID, int newID)
{
    ntupleExtension *ntExt = GetNTupleExtension(oldID);
    
    if (ntExt != NULL)
    	ntExt->ntupleID = newID;
}

/*
** Change the id of all of the variable panels for the ntuple with id "id"
*/
void ChangeVariablePanelItemIDs(int oldID, int newID)
{
    varPanel *panel;
    
    for (panel=VarPanelList; panel!=NULL; panel=panel->next) {
	if (panel->id == oldID)
    	    panel->id = newID;
    }
}

/*
** Close all of the variable panels related to the ntuple with id "id"
*/
void CloseVariablePanelsWithItem(int id)
{
    varPanel *panel;
    
    for (panel=VarPanelList; panel!=NULL; panel=panel->next) {
	if (panel->id == id)
    	    XtDestroyWidget(panel->shell);
    }
}
 
static void updateVariablePanel(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt)
{
    Widget newMenu = NULL;
    int nVars;
    
    /* Choose menu or dialog based on how many variables there are */
    nVars = ntuple->nVariables + (ntExt == NULL ? 0 : ntExt->nVars);
    if (nVars > PULLDOWN_LIMIT) {
        if (panel->pasteVarDialog == NULL)
            panel->pasteVarDialog = createPasteVarDialog(panel);
        updatePasteVarDialog(panel, ntuple, ntExt);
        XtVaSetValues(panel->pasteVarBtn, XmNsubMenuId, NULL, NULL);
    } else {
        newMenu = createPasteVarPane(panel, ntuple, ntExt);
        XtVaSetValues(panel->pasteVarBtn, XmNsubMenuId, newMenu, NULL);
    }
     
    /* Get rid of the old menu if it existed */
    if (panel->pasteVarPane != NULL)
    	XtDestroyWidget(panel->pasteVarPane);
    panel->pasteVarPane = newMenu;
}
    
static Widget createPasteVarPane(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt)
{
    Widget menu, item;
    int var, nVars;

    /* Construct a new variable menu and replace the old one */
    menu = XmCreatePulldownMenu(panel->pasteVarMenu, "pasteVarPane",
    	    NULL, 0);
    nVars = ntuple->nVariables + (ntExt == NULL ? 0 : ntExt->nVars);
    XtVaSetValues(panel->pasteVarBtn, XmNsubMenuId, menu , NULL);
    for (var=0; var<ntuple->nVariables; var++) {
    	item = AddMenuItem(menu, "ntItem", ntuple->names[var], 0, NULL,
    	        NULL, (XtCallbackProc)pasteVarCB, panel);
    	XtVaSetValues(item, XmNuserData, (XtPointer)((long)var), NULL);
    }
    for (var=ntuple->nVariables; var<nVars; var ++) {
    	item = AddMenuItem(menu, "extNTItem",
    		ntExt->vars[var-ntuple->nVariables].name, 0,
    	    	NULL, NULL, (XtCallbackProc)pasteVarCB, panel);
        XtVaSetValues(item, XmNuserData, (XtPointer)((long)var), NULL);
    }
    return menu;
}

/*
** Create an alternate to the popup menu when the number of items gets
** too large for a menu: a dialog with a list.  Note that because this code
** is only invoked when the number of variables gets large, it will tend
** not to be exercised unless you specifically make a point to test it.  So
** if you change something that might affect it, TEST IT.
*/ 
static Widget createPasteVarDialog(varPanel *panel)
{
    Widget dialog;
    int ac;
    Arg args[20];
    XmString s1;
    
    ac = 0;
    XtSetArg(args[ac], XmNtitle, "Paste Variable"); ac++;
    XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); ac++;
    XtSetArg(args[ac], XmNlistLabelString,
    	    s1=XmStringCreateSimple("Variables")); ac++;
    dialog = XmCreateSelectionDialog(panel->shell, "pasteVar", args, ac);
    XmStringFree(s1);
    XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)varOkCB, panel);
    XtAddCallback(dialog, XmNcancelCallback,
    	    (XtCallbackProc)varDismissCB, panel);
    AddMotifCloseCallback(panel->shell, (XtCallbackProc)varDismissCB, panel);
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_SELECTION_LABEL));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    return dialog;
}

static Widget updatePasteVarDialog(varPanel *panel, hsNTuple *ntuple,
	ntupleExtension *ntExt)
{
    int i, nVars;
    XmString *stringTable;
    Widget list;
    
    /* Find the list widget in the dialog */
    if (panel->pasteVarDialog == NULL)
    	return NULL;
    list = XmSelectionBoxGetChild(panel->pasteVarDialog, XmDIALOG_LIST);
    
    /* Build a string table to supply to the list widget */
    nVars = ntuple->nVariables + (ntExt != NULL ? ntExt->nVars : 0);
    stringTable = (XmString *)XtMalloc((nVars+1) * sizeof(XmString));
    for (i=0; i<nVars; i++)
    	stringTable[i] = XmStringCreateSimple(ExtNTVarName(ntuple, i));
    stringTable[i] = (XmString)0; /* mark end of table with 0 */
    
    /* Display the items in the variable list */
    XtVaSetValues(list, XmNitems, stringTable, XmNitemCount, nVars, NULL);
    FreeStringTable(stringTable);
    XmListDeselectAllItems(list);
    return list;
}

/*
** Fill in the ntuple variable values for all of the ntuple variables in
** "program" from "ntuple" and "ntExt".  If "errMsg" is NULL, blow past
** errors and just keep going.  Note that this routine is critical to
** the speed of processing ExtNTRefs.
**
** This will wastefully re-evaluate user-defined variables which
** a user-defined variable depends upon.  If users start using these
** features heavily, it should be improved. 
*/
static int evaluateDependentVariables(Program *program, hsNTuple *ntuple,
	ntupleExtension *ntExt, int index, char **errMsg)
{
    Symbol *s;

    /* Look thru all of the symbols in the symbol table, and fill in the
       values for all of the  */
    for (s=program->externSymList; s!=NULL; s=s->next) {
	if (s->type == NT_VAR)
    	    s->u.val = NTRef(ntuple, s->index, index);
    	else if (s->type == USER_DEF_VAR) {
    	    program = ntExt->vars[s->index].program;
    	    if (!evaluateDependentVariables(program, ntuple, ntExt,
    	    	    index, errMsg) && errMsg != NULL)
    	    	return False;
    	    if (!ExecuteProgram(program, &s->u.val, errMsg) && errMsg != NULL)
    	    	return False;
    	}
    }
    return True;
}

static void pasteVarCB(Widget w, varPanel *panel, caddr_t callData)
{
    int var;
    XtPointer userData;

    /* Get the variable name from the variable index stored in
       the userData field of the menu item pushButton widget */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    var = (long)userData;
    
    /* Quote the string if necessary and add it to the expression text field */
    pasteVarString(panel,
    	    ExtNTVarName((hsNTuple *)GetMPItemByID(panel->id), var));
    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
}

static void createCB(Widget w, varPanel *panel, caddr_t callData)
{
    char *nameString, *exprString, *errMsg, *errVar;
    int i, varIndex, failedAt, choice;
    Program *program;
    ntupleExtension *ntExt;
    hsNTuple *ntuple;
    
    /* Get the name and expression from the text widgets in the panel */
    nameString = XmTextGetString(panel->nameText);
    exprString = XmTextGetString(panel->exprText);
    cleanName(nameString);
    if (nameString[0] == '\0' || exprString[0] == '\0') {
    	if (nameString[0] == '\0' && exprString[0] != '\0')
    	    errMsg = "Please supply a name\nfor the new variable";
    	else if (exprString[0] == '\0' && nameString[0] != '\0')
    	    errMsg =
   	 "Please enter an expression to calculate\nvalues for the new variable";
    	else
    	    errMsg ="Please supply a name and\nexpression for the new variable";
    	DialogF(DF_ERR, w, 1, errMsg, "Dismiss");
    	XmProcessTraversal(nameString[0] != '\0' ? panel->exprText :
    		panel->nameText, XmTRAVERSE_CURRENT);
    	XtFree(nameString);
    	XtFree(exprString);
    	return;
    }
    
    /* Check for existing names and warn */
    ntExt = GetNTupleExtension(panel->id);
    if (ntExt != NULL) {
	for (i=0; i<ntExt->nVars; i++) {
    	    if (!strcmp(nameString, ntExt->vars[i].name)) {
    		choice = DialogF(DF_WARN, w, 2, "Replace existing variable %s?",
    	    		"No", "Yes", nameString);
    		if (choice == 1) {
    		    XmProcessTraversal(panel->nameText, XmTRAVERSE_CURRENT);
    	    	    XtFree(nameString);
    		    XtFree(exprString);
    		    return;
		}
	    }
	}
    }
    
    /* Parse the expression entered by the user and warn of parsing errors */
    program = ParseExpr(exprString, &errMsg, &failedAt);
    if (program == NULL) {
        XmTextSetInsertionPosition(panel->exprText, failedAt);
        DialogF(DF_ERR, w, 1,
    	       "Error parsing expression: %s",
    	       "Dismiss", errMsg);
    	XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
        XtFree(nameString);
    	XtFree(exprString);
    	return;
    }
    
    /* Resolve variable references and warn about undefined variables */
    ntuple = (hsNTuple *)GetMPItemByID(panel->id);
    ResolveVariableReferences(program, ntuple, ntExt);
    WarnUndefined(program, w);
    
    /* Create the new variable.  Note that here, ownership of the memory
       allocated by XmTextGetString is transferred to the ntuple extension */
    AddUserVariable(ntuple, nameString, exprString, program);
    
    /* Fetch the ntuple extension again, because AddUserVariable may have
       re-allocated it */
    ntExt = GetNTupleExtension(panel->id);
    
    /* Resolve variable references in other user defined variables to this
       variable */
    for (i=0; i<ntExt->nVars; i++)
    	ResolveVariableReferences(ntExt->vars[i].program, ntuple, ntExt);
       
    /* Check for deadly circular references */
    if (CheckForCircularReferences(ntExt, &errVar)) {
    	DialogF(DF_ERR, w, 1, "Variable %s is circularly referenced",
    	 	"Dismiss", errVar);
        DeleteUserVariable(ntuple, ntExt, nameString);
    	XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
   	return;
    }

    /* Test the expression on the current data and report arithmetic errors */
    for (i=0; i<ntExt->nVars; i++) {
    	if (!strcmp(ntExt->vars[i].name, nameString)) {
    	    varIndex = i;
    	    break;
    	}
    }
    if (!TestUserVariable(ntuple, ntExt, varIndex, &errMsg)) {
    	choice = DialogF(DF_ERR, w, 2,
	"Expression fails with error:\n\n  %s\n\n(using currently loaded data)",
		"Don't Create", "Create Anyhow", errMsg);
	if (choice == 1) {
	    DeleteUserVariable(ntuple, ntExt, nameString);
    	    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
    	    return;
        }
    }
       
    /* reflect new variable in this and other variable panels for this ntuple,
       and in displayed plots and the ntuple panel if it is displayed */
    UpdateVariablePanels(ntuple, ntExt);
    UpdateNTuplePanelList(ntuple->id);
    RefreshItem(ntuple->id);
    
    /* Change the panel to "modify" mode */
    XtVaSetValues(panel->shell, XmNtitle, nameString, NULL);
    strcpy(panel->name, nameString);
    XtSetSensitive(panel->modifyBtn, True);
    XtSetSensitive(panel->deleteBtn, True);
    XtVaSetValues(panel->form, XmNdefaultButton, panel->modifyBtn, NULL);
    XmProcessTraversal(panel->modifyBtn, XmTRAVERSE_CURRENT);
}

static void modifyCB(Widget w, varPanel *panel, caddr_t callData)
{
    char *nameString, *exprString, *errMsg, *errVar, *oldExpr, *oldName;
    int i, varIndex = -1, failedAt, choice;
    Program *program, *oldProgram;
    ntupleExtension *ntExt;
    hsNTuple *ntuple;
    userDefNTVar *varPtr;
    
    if (panel->name[0] == '\0')
    	return;
    
    /* Get a pointer to the variable that we'll be changing */
    ntExt = GetNTupleExtension(panel->id);
    for (i=0; i<ntExt->nVars; i++) {
    	if (!strcmp(panel->name, ntExt->vars[i].name))
    	    varIndex = i;
    }
    if (varIndex == -1) {
    	fprintf(stderr, "Internal error: variable to modify has disappeared\n");
    	return;
    }
    varPtr = &ntExt->vars[i];
    
    /* Get the name and expression from the text widgets in the panel.
       If either are blank, use whatever they were originally */
    nameString = XmTextGetString(panel->nameText);
    exprString = XmTextGetString(panel->exprText);
    cleanName(nameString);
    if (nameString[0] == '\0') {
    	XtFree(nameString);
    	nameString = XtMalloc(strlen(panel->name) + 1);
    	strcpy(nameString, panel->name);
    }
    if (exprString[0] == '\0') {
    	XtFree(exprString);
    	exprString = XtMalloc(strlen(varPtr->expression) + 1);
    	strcpy(nameString, varPtr->expression);
    }
    
    /* Parse the expression entered by the user and warn of parsing errors */
    program = ParseExpr(exprString, &errMsg, &failedAt);
    if (program == NULL) {
        XmTextSetInsertionPosition(panel->exprText, failedAt);
        DialogF(DF_ERR, w, 1,
    	       "Error parsing expression: %s",
    	       "Dismiss", errMsg);
        XtFree(nameString);
    	XtFree(exprString);
    	XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
    	return;
    }
    
    /* Resolve variable references and warn about undefined variables */
    ntuple = (hsNTuple *)GetMPItemByID(panel->id);
    ResolveVariableReferences(program, ntuple, ntExt);
    WarnUndefined(program, w);
    
    /* Modify the new variable.  Note that here, ownership of the memory
       allocated by XmTextGetString is transferred to the ntuple extension */
    oldName = ntExt->vars[varIndex].name;
    oldExpr = ntExt->vars[varIndex].expression;
    oldProgram = ntExt->vars[varIndex].program;
    ntExt->vars[varIndex].name = nameString;
    ntExt->vars[varIndex].expression = exprString;
    ntExt->vars[varIndex].program = program;
    
    /* Resolve variable references in other user defined variables to this
       variable */
    for (i=0; i<ntExt->nVars; i++)
    	ResolveVariableReferences(ntExt->vars[i].program, ntuple, ntExt);
       
    /* Check for deadly circular references */
    if (CheckForCircularReferences(ntExt, &errVar)) {
    	DialogF(DF_ERR, w, 1, "Variable %s is circularly referenced",
    	 	"Dismiss", errVar);
	XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
	ntExt->vars[varIndex].name = oldName;
	ntExt->vars[varIndex].expression = oldExpr;
	ntExt->vars[varIndex].program = oldProgram;
	XtFree(nameString);
	XtFree(exprString);
	FreeProgram(program);
   	return;
    }

    /* Test the expression on the current data and report arithmetic errors */
    if (!TestUserVariable(ntuple, ntExt, varIndex, &errMsg)) {
    	choice = DialogF(DF_ERR, w, 2,
	"Expression fails with error:\n\n  %s\n\n(using currently loaded data)",
		"Cancel Modify", "Modify Anyhow", errMsg);
	if (choice == 1) {
	    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
	    ntExt->vars[varIndex].name = oldName;
	    ntExt->vars[varIndex].expression = oldExpr;
	    ntExt->vars[varIndex].program = oldProgram;
	    XtFree(nameString);
	    XtFree(exprString);
	    FreeProgram(program);
    	    return;
        }
    }
       
    /* reflect new variable name in this and other variable panels for this
       ntuple, and the ntuple panel if it is displayed, and refresh plots
       for this NTuple in case they use the expression */
    if (strcmp(nameString, panel->name)) {
	XtVaSetValues(panel->shell, XmNtitle, nameString, NULL);
	strcpy(panel->name, nameString);
	UpdateVariablePanels(ntuple, ntExt);
	UpdateNTuplePanelList(ntuple->id);
	reinitPlotsWithVar(ntuple->id, ntuple->nVariables + varIndex);
    }
    RefreshItem(ntuple->id);
    XtFree(oldName);
    XtFree(oldExpr);
    FreeProgram(oldProgram);
}

static void deleteCB(Widget w, varPanel *panel, caddr_t callData)
{
    windowInfo *win;
    plotInfo *pInfo;
    int i, p, var, deleteVarIndex = -1, foundVar, choice, warned = False;
    ntupleExtension *ntExt = GetNTupleExtension(panel->id);
    hsNTuple *ntuple = (hsNTuple *)GetMPItemByID(panel->id);
    
    /* Check if the variable is used in any plots.  If so, warn the user,
       and pop down the plots */
    for (i=0; i<ntExt->nVars; i++) {
    	if (!strcmp(panel->name, ntExt->vars[i].name))
    	    deleteVarIndex = i + ntuple->nVariables;
    }
    if (deleteVarIndex == -1)
    	return;
    do {
	foundVar = False;
	for (win=WindowList; win!=NULL; win=win->next) {   
            foundVar = False;
            for (p=0; p<win->nPlots; p++) {
    		pInfo = win->pInfo[p];
    		for (var=0; var<MAX_DISP_VARS; var++) {
    		    if (pInfo->ntVars[var] == deleteVarIndex)
    	    		foundVar = True;
    		}
    		for (var=0; var<N_SLIDERS; var++) {
    		    if (pInfo->sliderVars[var] == deleteVarIndex)
    	    		foundVar = True;
    		}
    		if (foundVar) {
    		    if (!warned) {
    			choice = DialogF(DF_ERR, w, 2,
    	    			"Variable: %s\nis used in displayed plots",
    	    			"Cancel", "Close Plots", panel->name);
			warned = True;
			if (choice == 1)
	    		    return;
		    }
		    if (win->multPlot)
	    		CloseMiniPlot(win);
		    else
	    		ClosePlotWindow(win);
		    break;
		}
	    }
	}
    } while (foundVar);

    /* Remove the variable and pop down the variable panel */
    DeleteUserVariable(ntuple, ntExt, panel->name);
    XtDestroyWidget(panel->shell);
}

static void dismissCB(Widget w, varPanel *panel, caddr_t callData)
{
    XtDestroyWidget(panel->shell);
}

static void destroyCB(Widget w, varPanel *panel, caddr_t callData)
{
    removeFromVarPanelList(panel);
    XtFree((char *)panel);
}

static void pasteFnCB(Widget w, varPanel *panel, caddr_t callData)
{
    int index;
    XtPointer userData;

    /* Get the function name from the index stored in the userData field
       of the menu item pushButton widget, and paste it into the expression */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    index = (long)userData;
    pasteFnString(panel, FnData[index].fnName);
    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
}

static void pasteConstCB(Widget w, varPanel *panel, caddr_t callData)
{
    int index;
    XtPointer userData;

    /* Get the constant name from the index stored in the userData field
       of the menu item pushButton widget, and paste it into the expression */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    index = (long)userData;
    pasteVarString(panel, ConstData[index].constName);
    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
}

static void pasteFnString(varPanel *panel, char *fnName)
{
    XmTextPosition left, right;
    char *expr;
    
    /* Get the position of the insertion cursor or the selection */
    expr = XmTextGetString(panel->exprText);
    if (!XmTextGetSelectionPosition(panel->exprText, &left, &right) ||
    	    left == right)
       left = right = XmTextGetInsertionPosition(panel->exprText);
    if (left > (long int)(strlen(expr)))
    	left = right = strlen(expr);
    
    /* If the function name needs to be separated from the text before it,
       insert an extra space */
    if (left > 0 && isalnum(expr[left-1])) {
    	XmTextInsert(panel->exprText, left, " ");
    	left++; right++;
    }
    XtFree(expr);
    
    /* Insert the function name and parenthesis */
    XmTextInsert(panel->exprText, left, fnName);
    left += strlen(fnName); right += strlen(fnName);
    XmTextInsert(panel->exprText, left, "(");
    right++;
    XmTextInsert(panel->exprText, right, ")");
}

static void pasteVarString(varPanel *panel, char *varName) 
{
    XmTextPosition left, right;
    char *expr, *c;
    int addQuotes = False;
    char quotedName[HS_MAX_NAME_LENGTH+3];    
    
    /* If the name can't be used in an expression by itself, because it has
       characters that make it unsuited to be a variable by itself, (like
       punctuation, or begining with a number), surround it with quotes */
    if (isdigit(varName[0]))
    	addQuotes = True;
    for (c=varName; *c != '\0'; c++) {
    	if (!isalnum(*c)) {
    	    addQuotes = True;
    	    break;
    	}
    }
    if (addQuotes) {
    	sprintf(quotedName, "\"%s\"", varName);
    	varName = quotedName;
    }

    /* Get the position of the insertion cursor or the selection */
    expr = XmTextGetString(panel->exprText);
    if (!XmTextGetSelectionPosition(panel->exprText, &left, &right) ||
    	    left == right)
       left = right = XmTextGetInsertionPosition(panel->exprText);
    if (left > (long int)(strlen(expr)))
    	left = right = strlen(expr);
    
    /* If the variable or constant name needs to be separated from
       the text before it or after it, insert an extra space */
    if (isalnum(expr[right]))
    	XmTextInsert(panel->exprText, right, " ");
    if (left > 0 && isalnum(expr[left-1])) {
    	XmTextInsert(panel->exprText, left, " ");
    	left++; right++;
    }
    XtFree(expr);
    
    /* Insert the name, replacing the selection if it exists */
    XmTextReplace(panel->exprText, left, right, varName);
}

static void varMenuBtnCB(Widget w, varPanel *panel, caddr_t callData)
{
    if (panel->pasteVarDialog != NULL)
    	XtManageChild(panel->pasteVarDialog);
}

static void varOkCB(Widget w, varPanel *panel, caddr_t callData)
{
    char *string;
    
    string = XmTextGetString(XmSelectionBoxGetChild(panel->pasteVarDialog,
    	    XmDIALOG_TEXT));
    if (string[0] == '\0') {
        XtFree(string);
        return;
    }
    pasteVarString(panel, string);
    XtFree(string);
    XmProcessTraversal(panel->exprText, XmTRAVERSE_CURRENT);
}

static void  varDismissCB(Widget w, varPanel *panel, caddr_t callData)
{
    /* let autoUnmanage pop down dialog (for now) */
}

static void removeFromVarPanelList(varPanel *panel)
{
    varPanel *temp;

    if (VarPanelList == panel)
	VarPanelList = panel->next;
    else {
	for (temp = VarPanelList; temp != NULL; temp = temp->next) {
	    if (temp->next == panel) {
		temp->next = panel->next;
		break;
	    }
	}
    }
}

/*
** Reinitialize all plots from ntuple of id "id" which use the variable
** with index "varIndex".  id MUST be the id of an ntuple
*/
static void reinitPlotsWithVar(int id, int varIndex)
{
    windowInfo *w;
    plotInfo *pInfo;
    int p, var, foundVar = False;
    
    for (w=WindowList; w!=NULL; w=w->next) {
    	for (p=0; p<w->nPlots; p++) {
    	    pInfo = w->pInfo[p];
    	    if (pInfo->id == id) {
    		for (var=0; var<MAX_DISP_VARS; var++) {
    		    if (pInfo->ntVars[var] == varIndex)
    	    		foundVar = True;
    		}
    		for (var=0; var<N_SLIDERS; var++) {
    		    if (pInfo->sliderVars[var] == varIndex)
    	    		foundVar = True;
    		}
    		if (foundVar) {
    		    RedisplayPlotWindow(w, REINIT);
    		    UpdateSliderRange(w);
    		}
    	    }
	}
    }	    
}

/*
** Clean leading and trailing space off of name
*/
static void cleanName(char *string)
{
    char *c, *s, *whiteEnd;
    
    /* Find the first non-white character */
    for (whiteEnd=string; isspace(*whiteEnd); whiteEnd++);
     
    /* move non-white chars left */
    for (c=whiteEnd, s=string; *s != '\0'; c++, s++)
    	*s = *c;
    *s = '\0';
    
    /* Remove trailing whitespace by converting to nulls */
    for (c = &string[strlen(string)-1]; c>=string && isspace(*c); c--)
    	*c = '\0';
}

        
