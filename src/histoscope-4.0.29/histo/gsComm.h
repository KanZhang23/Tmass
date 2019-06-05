#ifndef GSCOMM_H_
#define GSCOMM_H_

#include <sys/types.h>
#include <Xm/Xm.h>

/* All "addToCommWidgets" calls must be made before histoMainLoop
 * is called. After that all such calls are simply ignored.
 */
void   addToCommWidgets(Widget w);
void   mainLoopStartsNow(XtAppContext context);
int    reserveGsChannel(void *wInfo, char *filename, Pixel bg,
			unsigned int width, unsigned int height,
			int *statusPointer);
Widget getGsChannelWidget(int channel);
void   fillGsChannelInfo(int channel, pid_t pid, Pixmap pixmap);
void   updateStatusPointer(int channel, int *statusPointer);

#endif /* GSCOMM_H_ */
