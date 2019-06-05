#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <X11/Intrinsic.h>

#include "gsComm.h"

#include "histoP.h"
#include "../histo_util/hsTypes.h"
#include "plotWindows.h"
#include "globalgc.h"

#define MAX_GSCOMM_CHANNELS 10
#define TIMEOUT_SEC 120
#define TIMEOUT_CHECK_PERIOD_SEC 2
#define DELETABLE_FILE_PREFIX "hs_latex_"

typedef struct _GsCommChannel {
    Widget w;
    time_t reserved;
    Pixmap pixmap;       /* pixmap to use              */
    windowInfo *wInfo;   /* window to refresh          */
    pid_t pid;           /* pid to kill                */
    char *filename;      /* file to unlink             */
    Pixel background;    /* use this to replace white  */
    unsigned int width;  /* pixmap width               */
    unsigned int height; /* pixmap height              */
    int *statusPtr;      /* change value when complete to -10 */
} GsCommChannel;

typedef struct _winPointer WinPointer;
struct _winPointer {
    WinPointer *next;
    windowInfo *wInfo;
};

static void resetGsChannel(GsCommChannel *ch);
static void HandleGsMessages(Widget w, XtPointer clientData,
			     XEvent *event, Boolean *continue_to_dispatch);
static int numWinReferences(windowInfo *wInfo);
static void channelFinished(int channel);
static windowInfo *popCommStack(void);
static int redisplayIfUnique(windowInfo *wInfo);
static void timer_proc(XtPointer ptr, XtIntervalId *id);

static XtAppContext appContext = NULL;
static int mainLoopStarted = 0;
static GsCommChannel commChannels[MAX_GSCOMM_CHANNELS];
static int nCommChan = 0;
static WinPointer* commQueue = NULL;
static int timeoutCheckSet = 0;
static Pixel white = 0;

void mainLoopStartsNow(XtAppContext context)
{
    appContext = context;
    mainLoopStarted = 1;
}

void addToCommWidgets(Widget w)
{
    int i;

    if (nCommChan == 0)
    {
	memset(commChannels, 0, sizeof(commChannels));
	white = WhitePixelOfScreen(XtScreen(w));
    }
    if (mainLoopStarted || w == 0 || nCommChan >= MAX_GSCOMM_CHANNELS)
	return;
    for (i=0; i<nCommChan; ++i)
	if (commChannels[i].w == w)
	    return;
    resetGsChannel(commChannels + nCommChan);
    commChannels[nCommChan++].w = w;
    XtAddEventHandler(w, 0, True, HandleGsMessages, NULL);
}

static void resetGsChannel(GsCommChannel *ch)
{
    ch->reserved    = 0;
    ch->pixmap      = XmUNSPECIFIED_PIXMAP;
    ch->wInfo       = NULL;
    ch->pid         = 0;
    if (ch->filename)
    {
	free(ch->filename);
	ch->filename = NULL;
    }
    ch->background = white;
    ch->width      = 0;
    ch->height     = 0;
    ch->statusPtr  = NULL;
}

Widget getGsChannelWidget(int channel)
{
    if (channel >= 0 && channel < nCommChan)
	return commChannels[channel].w;
    else
	return NULL;
}

void fillGsChannelInfo(int channel, pid_t pid, Pixmap pixmap)
{
    if (channel >= 0 && channel < nCommChan)
    {
	commChannels[channel].pid = pid;
	commChannels[channel].pixmap = pixmap;
    }
}

void   updateStatusPointer(int channel, int *statusPointer)
{
    if (channel >= 0 && channel < nCommChan)
	commChannels[channel].statusPtr = statusPointer;
}

int reserveGsChannel(void *ptr, char *filename, Pixel bg,
		     unsigned int width, unsigned int height,
		     int *statusPointer)
{
    int channel;
    WinPointer *wp;
    windowInfo *wInfo = (windowInfo *)ptr;

    for (channel=0; channel<nCommChan; ++channel)
	if (commChannels[channel].reserved == 0)
	{
	    if (filename) {
		commChannels[channel].filename = strdup(filename);
		if (commChannels[channel].filename == NULL)
		{
		    fprintf(stderr, "Error in reserveGsChannel: out of memory\n");
		    return -1;
		}
	    } else {
		commChannels[channel].filename = NULL;
	    }
	    commChannels[channel].reserved = time(0);
	    commChannels[channel].pixmap = XmUNSPECIFIED_PIXMAP;
	    commChannels[channel].wInfo = wInfo;
	    commChannels[channel].background = bg;
	    commChannels[channel].width = width;
	    commChannels[channel].height = height;
	    commChannels[channel].pid = 0;
	    commChannels[channel].statusPtr = statusPointer;
	    if (!timeoutCheckSet && appContext)
	    {
		XtAppAddTimeOut(
		    appContext, 1000*TIMEOUT_CHECK_PERIOD_SEC,
		    timer_proc, NULL);
		timeoutCheckSet = 1;
	    }
	    return channel;
	}
    /* No free channels. Add to the queue if necessary. */
    if (numWinReferences(wInfo) == 0)
    {
	wp = (WinPointer *)malloc(sizeof(WinPointer));
	if (wp == NULL)
	{
	    fprintf(stderr, "Error in reserveGsChannel: out of memory\n");
	    return -1;
	}
	wp->wInfo = wInfo;
	wp->next = commQueue;
	commQueue = wp;
    }
    return -1;
}

static int numWinReferences(windowInfo *wInfo)
{
    int i, count = 0;
    WinPointer *wp;

    for (i=0; i<nCommChan; ++i)
	if (commChannels[i].wInfo == wInfo)
	    ++count;
    wp = commQueue;
    while (wp)
    {
	if (wp->wInfo == wInfo)
	    ++count;
	wp = wp->next;
    }
    return count;
}

static void HandleGsMessages(Widget w, XtPointer clientData,
			     XEvent *event, Boolean *continue_to_dispatch)
{
    int channel;

    if (event->type != ClientMessage)
	return;
    for (channel=0; channel<nCommChan; ++channel)
	if ((long)commChannels[channel].pixmap == event->xclient.data.l[1])
	    break;
    if (channel < nCommChan)
	channelFinished(channel);
}

static windowInfo *popCommStack(void)
{
    WinPointer *wp;
    windowInfo *win;

    if (commQueue == NULL)
	return NULL;
    win = commQueue->wInfo;
    wp = commQueue;
    commQueue = commQueue->next;
    free(wp);
    return win;
}

static int redisplayIfUnique(windowInfo *winfo)
{
    if (winfo)
	if (numWinReferences(winfo) == 0)
	    if (existsInWindowList(winfo))
	    {
		RedisplayPlotWindow(winfo, REFRESH);
		return 1;
	    }
    return 0;
}

static void channelFinished(int channel)
{
    windowInfo *winfo;

    assert(commChannels[channel].reserved);

    /* Stop the ghostscript process */
    if (commChannels[channel].pid > 0)
    {
	kill(commChannels[channel].pid, SIGTERM);
	waitpid(commChannels[channel].pid, NULL, 0);
    }

    /* Delete the PS file if necessary. Make sure 
     * we do not delete some strange file.
     */
    if (commChannels[channel].filename)
    {
	char *checkname = strrchr(commChannels[channel].filename, '/');
	if (checkname)
	    ++checkname;
	else
	    checkname = commChannels[channel].filename;
	if (strncmp(checkname, DELETABLE_FILE_PREFIX,
		    strlen(DELETABLE_FILE_PREFIX)) == 0)
	    unlink(commChannels[channel].filename);
    }

    /* Change the background pixel of the image */
    if (commChannels[channel].background != white)
    {
	int width  = commChannels[channel].width;
	int height = commChannels[channel].height;
	Pixel bg   = commChannels[channel].background;
	XImage *xi = XGetImage(XtDisplay(commChannels[channel].w),
			       commChannels[channel].pixmap,
			       0, 0, width, height, AllPlanes, ZPixmap);
	if (xi)
	{
	    int i, j;
	    for (i=0; i<width; ++i)
		for (j=0; j<height; ++j)
		    if (XGetPixel(xi, i, j) == white)
			XPutPixel(xi, i, j, bg);
	    XPutImage(XtDisplay(commChannels[channel].w),
		      commChannels[channel].pixmap,
		      globalGC, xi, 0, 0, 0, 0, width, height);
	    XDestroyImage(xi);
	}
	else
	    fprintf(stderr, "Error fetching pixmap data. Low on memory?\n");
    }

    /* Set the status */
    if (commChannels[channel].statusPtr)
	*commChannels[channel].statusPtr = -10;

    /* Remember the window pointer and reset the channel */
    winfo = commChannels[channel].wInfo;
    resetGsChannel(commChannels + channel);

    /* Redraw the window if there are
       no more connections pending for it */
    redisplayIfUnique(winfo);

    /* Try to take an item from the window stack */
    while (commQueue)
    {
	winfo = popCommStack();
	if (redisplayIfUnique(winfo))
	    break;
    }
}

static void timer_proc(XtPointer ptr, XtIntervalId *id)
{
    int channel, nreserved;
    time_t thistime = time(0);

    nreserved = 0;
    for (channel=0; channel<nCommChan; ++channel)
	if (commChannels[channel].reserved)
	{
	    if (thistime - commChannels[channel].reserved > TIMEOUT_SEC)
		channelFinished(channel);
	    else
		++nreserved;
	}

    /* Reinstall the timeout check if necessary */
    if (nreserved > 0 && appContext)
    {
	XtAppAddTimeOut(
	    appContext, 1000*TIMEOUT_CHECK_PERIOD_SEC,
	    timer_proc, NULL);
	timeoutCheckSet = 1;
    }
    else
	timeoutCheckSet = 0;
}
