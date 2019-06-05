/*******************************************************************************
*									       *
* MarginSlider.h - Margin Slider Widget, Public Header File		       *
*									       *
* Copyright (c) 1994 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* FermmarginSliderWidgetClassilab Nirvana GUI Library						       *
* December 30, 1993							       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
*******************************************************************************/

#ifndef  MARGINSLIDER_H
#define  MARGINSLIDER_H

/* Resource strings */
#define  XmNleftBarStopMin           "leftBarStopMin"
#define  XmCLeftBarStopMin           "LeftBarStopMin"
#define  XmNrightBarStopMax          "rightBarStopMax"
#define  XmCRightBarStopMax          "RightBarStopMax"
#define  XmNleftBarStop              "leftBarStop"
#define  XmCLeftBarStop              "LeftBarStop"
#define  XmNrightBarStop             "rightBarStop"
#define  XmCRightBarStop             "RightBarStop"
#define  XmNleftBar                  "leftBar"
#define  XmCLeftBar                  "LeftBar"
#define  XmNrightBar                 "rightBar"
#define  XmCRightBar                 "RightBar"
#define  XmNmarginDragCallback       "dragCallback"
#define  XmCmarginDragCallback       "DragCallback"
#define  XmNmarginResizeCallback     "resizeCallback"
#define  XmCmarginResizeCallback     "ResizeCallback"
#define  XmNmarginChangedCallback    "marginChangedCallback"
#define  XmCmarginChangedCallback    "MarginChangedCallback"
#define  XmNredisplayCallback        "redisplayCallback"
#define  XmCRedisplayCallback        "RedisplayCallback"

extern WidgetClass marginSliderWidgetClass;

typedef struct _MarginSliderClassRec *MarginSliderWidgetClass;
typedef struct _MarginSliderRec      *MarginSliderWidget;

typedef struct {
    int     reason;
    XEvent *event;
} MarginSliderCallbackStruct;
/*
*/
#endif  /* MARGINSLIDER_H */
