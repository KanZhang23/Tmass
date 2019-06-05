#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/ScrolledW.h>
#include <Xm/XmP.h>

#include <X11/Xcms.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "colorScaleDialog.h"
#include "../util/help.h"
#include "../plot_widgets/drawAxes.h"
#include "../util/psUtils.h"
#include "globalgc.h"
#include "defaultColorScale.h"

extern void RedisplayAllColoredWindows(const ColorScale *scale);

/*  #define CALL_SEQ_DEBUG */

#define ACTION_AREA_HEIGHT 27
#define BORDER_OFFSET 7
#define COLOR_SLIDER_MAX 1000
#define MAX_COLORNAME_LEN 256
#define COLOR_SCALE_HEIGHT 30
#define DATA_AXIS_PAD_BOTTOM 11 /* The amount of space needed for padding
				   and for drawing vertical ticks */
#define DATA_AXIS_PAD_SIDE 20
#define MARKER_HEIGHT 21
#define MARKER_WIDTH 10

#define WIDGET_NAME "ColorScale"

typedef struct {
    char *label;
    void (*callback)(Widget, XtPointer, XtPointer);
    XtPointer data;
} ActionAreaItem;

typedef struct {
    int numer;
    int denom;
} Ratio;

typedef enum {
    RED = 0,
    GREEN,
    BLUE
} PrimeColor;

typedef enum {
    START_COLOR = 0,
    END_COLOR,
    UNDER_COLOR,
    OVER_COLOR,
    N_COLOR_ROLES
} ColorRole;

typedef struct {
    ColorScaleArgs orig;
    ColorScaleArgs work;
    const ColorScale *saved_default;
    Widget crole[N_COLOR_ROLES];
    Widget activerole[N_COLOR_ROLES];
    Widget dialog;
    Widget scale_area;
    Widget rgb_scales[3];
    Widget colorwin;
    Widget colorentry;
    Widget ncolorsentry;
    Widget cellentry;
    Widget setbutton;
    Widget minentry;
    Widget maxentry;
    Widget use_data_b;
    XmFontList font;
    XcmsColor choicecolor; /* The color of the color chooser rectangle */
    Pixel background;
    Pixel foreground;
    Pixel prime_colors[3];
    int apply_count;
    int color_role;
    int active_cell;
    int setbutton_enabled;
    int gravity_adjusted;
    int typing_cb_disabled;
    int locked_value[3];
    int ratios_locked;
    int new_colors_tmp;
    char ctmp[MAX_COLORNAME_LEN];
} DialogData;

#ifdef CALL_SEQ_DEBUG
static char *event_names[] = {
    "",
    "",
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "EnterNotify",
    "LeaveNotify",
    "FocusIn",
    "FocusOut",
    "KeymapNotify",
    "Expose",
    "GraphicsExpose",
    "NoExpose",
    "VisibilityNotify",
    "CreateNotify",
    "DestroyNotify",
    "UnmapNotify",
    "MapNotify",
    "MapRequest",
    "ReparentNotify",
    "ConfigureNotify",
    "ConfigureRequest",
    "GravityNotify",
    "ResizeRequest",
    "CirculateNotify",
    "CirculateRequest",
    "PropertyNotify",
    "SelectionClear",
    "SelectionRequest",
    "SelectionNotify",
    "ColormapNotify",
    "ClientMessage",
    "MappingNotify"
};
#endif

static char help_text[] = "The Color Editor GUI allows you "
"to modify the color scale of your plot and to adjust the mapping "
"from data into colors. By default, the color scale is a straight "
"line in the coordinates of the color space whose name is chosen "
"from the menu in the top left corner of the GUI. All color "
"spaces supported by the X Color Management System may be used, "
"together with two additional popular spaces "
"HSV (Hue, Saturation, and Value) and "
"HLS (Hue, Luminance, and Saturation). A detailed description "
"of various color spaces is beyond the scope of this simple "
"note. If you need more information, please consult the relevant "
"literature about colorimetry or search the Web for \"color space FAQ\". "
"In general, you need to have some idea how your favorite "
"color space is related to the RGB space so that the whole color scale "
"stays within the limits imposed by RGB and can be displayed on "
"the computer screen."
"\n\n"
"To change the starting or the ending point in the chosen color space, "
"you need to click on the corresponding button just below the color space "
"menu. A selection mark will appear to the right of the button "
"(an asterisk). Double-click will also transfer the color of the chosen "
"button to the color chooser on the top right side of the GUI. Adjust "
"the color using the RGB color composition scales or simply type the color name "
"into the \"Name\" entry and then push the \"Set\" button. The image of "
"the color scale will be repainted to reflect the new scale limit. Click "
"on the \"Apply\" button at the bottom of the GUI to see how your data "
"look like with the new scale."
"\n\n"
"Individual color cells may be edited by clicking on the color scale "
"image. Click with the left mouse button will select a cell, "
"middle click will select a cell "
"and transfer its color to the color chooser, and right click will move "
"the selected cell one step in the direction of the click. The keyboard "
"may also be used for navigation when the color scale image is activated. "
"Spacebar will transfer the color of the selected cell to the chooser, "
"while right and left arrows will move the selected cell one step in their "
"respective directions and transfer the color of the new cell. "
"The selected cell number will be shown in the \"Cell number\" entry. "
"This entry can also be used for cell selection. Just type the "
"desired cell number and press \"Enter\". The color of the selected "
"cell can be changed by pressing the \"Set\" button which copies the "
"color from the color chooser into the cell. Note that any change in the "
"starting color, ending color, number of colors in the scale, or color space "
"will linearize the scale and discard all individual cell edits. Therefore, "
"if you plan to edit individual cells, make sure to pick the color space "
"and the number of colors first."
"\n\n"
"The part of the GUI inside the \"Data mapping to the color scale\" frame "
"can be used to change the data range mapped to color. By default, the full "
"data range is mapped when the plot is first displayed on the "
"computer screen, and the mapping is linear. The default mapping is static -- "
"it is not changed when new data points are added to the plot. "
"You can adjust the upper and "
"lower limits of the data range or choose dynamic mapping. "
"With static limits, the points outside the mapped range will be shown "
"on the plot using the underflow and overflow colors. "
"Current minimum and maximum data values can be chosen as limits "
"by clicking on the \"Use data range\" button. "
"When the range is dynamic, "
"the full data range will be mapped to the color scale every time the "
"plot is redrawn (unless the "
"mapping is logarithmic and the data contains points which are not positive -- "
"such points will always be shown using the underflow color). "
"In this case, the colors of some individual data points (on scatter plots) "
"or histogram bins "
"(on 2d histograms) may change -- even if their data values stay the same -- "
"when the scale is adjusted to fit the minimum and the maximum data values."
"\n\n"
"The action buttons at the bottom of the GUI have usual meanings. The "
"\"Make Default\" button replaces the default Histo-Scope color scale "
"with the scale currently edited. Only the default colors are changed, "
"not the data mapping method."
"\n";

static Widget CreateScaleControls(Widget parent, char *name,
				  DialogData *dialogData);
static Widget CreateColorChooser(Widget parent, char *name,
				 DialogData *dialogData);
static Widget ShowScaleColors(Widget parent, char *name,
			      DialogData *dialogData);
static Widget CreateDataMappingControls(Widget parent, char *name,
					DialogData *dialogData);
static Widget CreateActionArea(Widget parent, char *name,
			       const ActionAreaItem *actions,
			       int num_actions, int default_button,
			       int height, Ratio r);
static void ok_pushed(Widget, XtPointer, XtPointer);
static void apply_pushed(Widget, XtPointer, XtPointer);
static void make_default_pushed(Widget, XtPointer, XtPointer);
static void cancel_pushed(Widget, XtPointer, XtPointer);
static void help_pushed(Widget, XtPointer, XtPointer);
static void delete_pushed(Widget, XtPointer, XtPointer);
static void perform_apply_action(DialogData *, DialogStatus reason);
static void perform_cancel_action(DialogData *);
static void perform_exit_action(DialogData *, DialogStatus);
static void set_trough_color(Widget w, Pixel p);
static DialogData *create_dialog_data(Widget dialog, ColorScaleArgs *args);
static void destroy_dialog_data(DialogData *dialogData);
static void set_colorspace(Widget, XtPointer, XtPointer, ColorSpace);
static void set_static_range(Widget, XtPointer, XtPointer);
static void set_dynamic_range(Widget, XtPointer, XtPointer);
static void set_log_scale(Widget, XtPointer, XtPointer);
static void set_linear_scale(Widget, XtPointer, XtPointer);
static void scale_expose_cb(Widget, XtPointer, XtPointer);
static void scale_input_cb(Widget, XtPointer, XtPointer);
static void rgbi_scale_movement_cb(Widget, XtPointer, XtPointer);
static void redraw_color_scale(DialogData *dialogData);
static void redraw_data_axis(DialogData *dialogData);
static void refresh_colorwin(DialogData *dialogData);
static void refresh_color_name(DialogData *dialogData);
static void refresh_rgbscales(DialogData *dialogData);
static void set_color_role(Widget, XtPointer, XtPointer, ColorRole);
static void color_set_button_cb(Widget, XtPointer, XtPointer);
static void color_clear_button_cb(Widget, XtPointer, XtPointer);
static void update_color_role_backgrounds(DialogData *dialogData);
static void highlight_color_role(DialogData *dialogData, ColorRole role);
static void unhighlight_color_role(DialogData *dialogData);
const char *colorname_from_role(DialogData *dialogData, ColorRole);
static int set_rgbscales_byname(DialogData *dialogData, const char *colorname);
static void set_rgbscales_bycell(DialogData *dialogData, int cell);
static void color_text_entry_cb(Widget, XtPointer, XtPointer);
static void show_error_message(Widget parent, char *message);
static void generic_typing_cb(Widget, XtPointer, XtPointer);
static void ncolors_entry_cb(Widget, XtPointer, XtPointer);
static void lock_ratio_cb(Widget, XtPointer, XtPointer);
static void cellnum_entry_cb(Widget, XtPointer, XtPointer);
static void set_chooser_colorname(DialogData *dialogData, const char *name);
static void typing_cb_enable(DialogData *dialogData, int enable);
static void highlight_active_cell(DialogData *dialogData, int cell);
static void unhighlight_active_cell(DialogData *dialogData);
static void get_data_range_cb(Widget, XtPointer, XtPointer);
static void set_limit_cb(Widget, XtPointer, XtPointer);
static unsigned get_widget_width(Widget);
static unsigned get_widget_height(Widget);
static XFontStruct *getFontStruct(XmFontList font);

static void set_color_role_start(Widget w, XtPointer p1, XtPointer p2)
{
    set_color_role(w, p1, p2, START_COLOR);
}
static void set_color_role_end(Widget w, XtPointer p1, XtPointer p2)
{
    set_color_role(w, p1, p2, END_COLOR);
}
static void set_color_role_under(Widget w, XtPointer p1, XtPointer p2)
{
    set_color_role(w, p1, p2, UNDER_COLOR);
}
static void set_color_role_over(Widget w, XtPointer p1, XtPointer p2)
{
    set_color_role(w, p1, p2, OVER_COLOR);
}

static void set_cspace_rgb(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, RGB);
}
static void set_cspace_rgbi(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, RGBI);
}
static void set_cspace_ciexyz(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, CIEXYZ);
}
static void set_cspace_cieuvy(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, CIEUVY);
}
static void set_cspace_ciexyy(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, CIEXYY);
}
static void set_cspace_cielab(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, CIELAB);
}
static void set_cspace_cieluv(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, CIELUV);
}
static void set_cspace_tekhvc(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, TEKHVC);
}
static void set_cspace_hsv(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, HSV);
}
static void set_cspace_hls(Widget w, XtPointer p1, XtPointer p2)
{
    set_colorspace(w, p1, p2, HLS);
}

/*  static Widget get_top_shell(Widget w) */
/*  { */
/*      while (w && !XtIsWMShell(w)) */
/*  	w = XtParent(w); */
/*      return w; */
/*  } */

void * CreateColorScale(Widget w, char *title, ColorScaleArgs *parentData)
{
    int n;
    Widget dialog, topform, control_area, color_scale, action_area;
    Widget controls, chooser, data_mapping, control_frame, control_form;
    ActionAreaItem action_items[] = {
	{"OK", ok_pushed, NULL},
	{"Apply", apply_pushed, NULL},
	{"Make Default", make_default_pushed, NULL},
	{"Cancel", cancel_pushed, NULL},
	{"Help", help_pushed, NULL}
    };
    const Ratio button_width_ratio = {15, 1};
    Arg args[10];
    DialogData *dialogData;
    Atom wm_delete_window;

#ifdef CALL_SEQ_DEBUG
    printf("CreateColorScale called\n");
#endif
    /* Set up a DialogShell as a popup window. Set the delete
     * window protocol response to XmDO_NOTHING and install
     * our own deletion callback to ensure that the window cleans
     * up after itself.
     *
     * There is no variable argument version of the XmCreateDialogShell
     * function.
     */
    n = 0; 
    XtSetArg(args[n], XmNtitle, title); n++;
    XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
    dialog = XmCreateDialogShell(w, WIDGET_NAME, args, n);

    /* Create a new DialogData object for this dialog.
     * We will pass this object to various callbacks.
     */
    dialogData = create_dialog_data(dialog, parentData);
    if (dialogData == NULL)
    {
	XtDestroyWidget(dialog);
	return NULL;
    }
    for (n=0; (unsigned)n<XtNumber(action_items); ++n)
	action_items[n].data = dialogData;

    /* Add window deletion callback */
    wm_delete_window = XInternAtom(XtDisplay(dialog), "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(dialog, wm_delete_window,
			    delete_pushed, (XtPointer)dialogData);

    /* Create a top level form child of the dialog */
    topform = XmCreateForm(dialog, "topform", NULL, 0);

    /* Create the container which will hold the main set of buttons */
    action_area = CreateActionArea(
	topform, "action_area", action_items, 
	XtNumber(action_items), -1, ACTION_AREA_HEIGHT, button_width_ratio);

    /* Attach this container to the dialog */
    XtVaSetValues(action_area,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, BORDER_OFFSET,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, BORDER_OFFSET,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, BORDER_OFFSET+2,
		  NULL);

    /* Create the container which will hold the "controls" */
    control_area = XmCreateForm(topform, "control_area", NULL, 0);

    /* Attach this container */
    XtVaSetValues(control_area,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, action_area,
		  XmNbottomOffset, 2,
		  NULL);

    /* The bottom part of the control area will be taken by
     * the frame which sets up the data mapping.
     */
    data_mapping = CreateDataMappingControls(
	control_area, "data_mapping", dialogData);

    /* Attach the data mapping area to the parent */
    XtVaSetValues(data_mapping,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, BORDER_OFFSET,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, BORDER_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, BORDER_OFFSET,
		  NULL);  

    /* The color scale display is just above the data mapping frame */
    color_scale = ShowScaleColors(
	control_area, "color_scale", dialogData);

    /* Attach color scale to the parent */
    XtVaSetValues(color_scale,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, data_mapping,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, 1,
		  NULL);

    /* Create a frame for the scale editing GUIs */
    control_frame = XtVaCreateManagedWidget(
	"control_frame", xmFrameWidgetClass, control_area,
	XmNleftAttachment, XmATTACH_FORM,
	XmNleftOffset, BORDER_OFFSET,
	XmNrightAttachment, XmATTACH_FORM,
	XmNrightOffset, BORDER_OFFSET,
	XmNtopAttachment, XmATTACH_FORM,
	XmNtopOffset, BORDER_OFFSET,
	XmNbottomAttachment, XmATTACH_WIDGET,
	XmNbottomWidget, color_scale,
	XmNbottomOffset, 4,
	NULL);

    control_form = XmCreateForm(control_frame, "control_form", NULL, 0);
    XtVaSetValues(control_form,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  NULL);

    /* The top left part will be used to hold various buttons and labels */
    controls = CreateScaleControls(control_form, "color_buttons", dialogData);

    /* Attach controls to the parent */
    XtVaSetValues(controls,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, BORDER_OFFSET+1,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNtopOffset, BORDER_OFFSET,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, BORDER_OFFSET+1,
		  NULL);

    /* The top right part is used to show the color chooser */
    chooser = CreateColorChooser(control_form, "color_chooser", dialogData);

    /* Attach the chooser */
    XtVaSetValues(chooser,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNtopOffset, BORDER_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, BORDER_OFFSET,
		  XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		  XmNbottomWidget, controls,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, controls,
		  XmNleftOffset, BORDER_OFFSET,
		  NULL);

    /* Manage the control area form since it is complete */
    XtManageChild(control_form);
    XtManageChild(control_area);

    /* Now, manage the paned window since it is complete */
    XtManageChild(topform);

    refresh_colorwin(dialogData);
    /* No point in drawing the color scale until
     * the window gets the expose event.
     *
     * redraw_color_scale(dialogData);
     */
    return dialogData;
}

void DestroyColorScale(void *scale)
{
    perform_exit_action((DialogData *)scale, DIALOG_DELETE);
}

Widget CreateActionArea(Widget parent, char *name,
			const ActionAreaItem *actions,
			int num_actions, int default_button,
			int height, Ratio r)
{
    int i;
    Widget action_area, widget;

#ifdef CALL_SEQ_DEBUG
    printf("CreateActionArea called\n");
#endif
    action_area = XmCreateForm(parent, name, NULL, 0);
    XtVaSetValues(action_area,
		  XmNfractionBase, r.numer*num_actions + r.denom*(num_actions+1),
		  NULL);
    if (height > 0)
	XtVaSetValues(action_area,
		      XmNheight, height,
		      NULL);

    for (i=0; i<num_actions; ++i)
    {
	widget = XmCreatePushButton(action_area, actions[i].label, NULL, 0);
	XtVaSetValues(widget,
		      XmNleftAttachment, XmATTACH_POSITION,
		      XmNrightAttachment, XmATTACH_POSITION,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_FORM,
		      XmNleftPosition, r.denom*(i+1) + r.numer*i,
		      XmNrightPosition, (r.denom + r.numer)*(i+1),
		      NULL);
	if (i == default_button)
	    XtVaSetValues(widget,
			  XmNshowAsDefault, True,
			  XmNdefaultButtonShadowThickness, 1,
			  NULL);
	if (actions[i].callback)
	    XtAddCallback(widget, XmNactivateCallback,
			  actions[i].callback, (XtPointer)actions[i].data);
	XtManageChild(widget);
    }

    /* Manage the form since it is complete */
    XtManageChild(action_area);
    return action_area;
}

static void ok_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("ok_pushed called\n");
#endif
    perform_apply_action(dialogData, DIALOG_OK);
    perform_exit_action(dialogData, DIALOG_OK);
}

static void apply_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("apply_pushed called\n");
#endif
    perform_apply_action(dialogData, DIALOG_APPLY);
}

static void make_default_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    char errmess[] = "Failed to replace the default color scale";
    const ColorScale *oldDefault, *newdefault;

#ifdef CALL_SEQ_DEBUG
    printf("make_default_pushed called\n");
#endif
    oldDefault = defaultColorScale();
    if (oldDefault != NULL && dialogData->saved_default == NULL)
    {
	dialogData->saved_default = backupColorScale(oldDefault);
	incrColorScaleRefCount(dialogData->saved_default);
    }
    newdefault = cloneColorScale(DEFAULT_COLOR_SCALE_NAME,
				 dialogData->work.scale);
    if (newdefault == NULL)
	show_error_message(dialogData->dialog, errmess);
    else if (oldDefault != NULL)
	assert(newdefault == oldDefault);
    RedisplayAllColoredWindows(newdefault);
}

static void cancel_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("cancel_pushed called\n");
#endif
    perform_cancel_action(dialogData);
    perform_exit_action(dialogData, DIALOG_CANCEL);
}

static void delete_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("delete_pushed called\n");
#endif
    perform_exit_action(dialogData, DIALOG_DELETE);
}

static void perform_apply_action(DialogData *dialogData, DialogStatus reason)
{
#ifdef CALL_SEQ_DEBUG
    printf("perform_apply_action called\n");
#endif
    /* Apply the parent callback */
    if (dialogData->orig.apply_cb)
    {
	dialogData->work.reason = reason;
	dialogData->orig.apply_cb(XtParent(dialogData->dialog),
				  dialogData->orig.apply_data,
				  &dialogData->work);
	++dialogData->apply_count;
    }
}

static void perform_cancel_action(DialogData *dialogData)
{
    const ColorScale *cscale = NULL;

#ifdef CALL_SEQ_DEBUG
    printf("perform_cancel_action called\n");
#endif
    if (dialogData->saved_default)
    {
	/* Restore the original default color scale */
	cscale = cloneColorScale(DEFAULT_COLOR_SCALE_NAME,
				 dialogData->saved_default);
	RedisplayAllColoredWindows(cscale);
    }
    if (dialogData->apply_count > 0)
    {
	/* Restore the condition which existed 
	 * before the dialog has been applied
	 */
	dialogData->orig.reason = DIALOG_CANCEL;
	dialogData->orig.apply_cb(XtParent(dialogData->dialog),
				  dialogData->orig.apply_data,
				  &dialogData->orig);
    }
}

static void perform_exit_action(DialogData *dialogData, DialogStatus status)
{
    Widget parent;
    XtPointer ptr;
    void (*close_cb)(Widget, XtPointer, DialogStatus);

#ifdef CALL_SEQ_DEBUG
    printf("perform_exit_action called\n");
#endif
    XtDestroyWidget(dialogData->dialog);
    parent = XtParent(dialogData->dialog);
    ptr = (XtPointer)dialogData->orig.close_data;
    close_cb = dialogData->orig.close_cb;
    destroy_dialog_data(dialogData);
    if (close_cb)
	close_cb(parent, ptr, status);
}

#include <unistd.h>
static void help_pushed(Widget w, XtPointer client_data,
			XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("help_pushed called\n");
#endif
    CreateHelpDialog(dialogData->dialog,
  		     "Color Editor Help", help_text);
}

static Widget ShowScaleColors(Widget parent, char *name,
			      DialogData *dialogData)
{
    Widget color_scale, scale_picture;
    XFontStruct *fs = getFontStruct(dialogData->font);

#ifdef CALL_SEQ_DEBUG
    printf("ShowScaleColors called\n");
#endif
    color_scale = XmCreateForm(parent, name, NULL, 0);
    XtVaSetValues(color_scale,
		  XmNverticalSpacing, BORDER_OFFSET-1,
		  XmNhorizontalSpacing, BORDER_OFFSET+1,
		  NULL);
    scale_picture = XtVaCreateManagedWidget(
	name, xmDrawingAreaWidgetClass, color_scale,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
        XmNbackground, WhitePixelOfScreen(XtScreen(dialogData->dialog)),
	XmNheight, COLOR_SCALE_HEIGHT + fs->ascent+DATA_AXIS_PAD_BOTTOM,
	NULL);
    dialogData->scale_area = scale_picture;
    XtAddCallback(scale_picture, XmNexposeCallback,
		  scale_expose_cb, dialogData);
    XtAddCallback(scale_picture, XmNinputCallback,
		  scale_input_cb, dialogData);

    /* Manage the scale when it is complete */
    XtManageChild(color_scale);
    return color_scale;
}

static Widget CreateScaleControls(Widget parent, char *name,
				  DialogData *dialogData)
{
    int i, n, label_end, button_end;
    Widget controls, align, label, button, space_menu, stretch;
    Widget wtmp, ncolors_form, ncolors_label, ncolors_entry;
    Widget cellnum_label, cellnum_entry;
    XmString str, menulabel, menu_titles[N_COLOR_SPACES];
    char *color_titles[N_COLOR_ROLES] = {
	"Starting color:", "Ending color:",
	"Underflow color:", "Overflow color:"
    };
    char *button_titles[N_COLOR_ROLES] = {
	"start_b", "end_b", "under_b", "over_b"
    };
    void (*cspace_callbacks[N_COLOR_SPACES])(Widget, XtPointer, XtPointer) = {
	set_cspace_rgb,
	set_cspace_rgbi,
	set_cspace_ciexyz,
	set_cspace_cieuvy,
	set_cspace_ciexyy,
	set_cspace_cielab,
	set_cspace_cieluv,
	set_cspace_tekhvc,
	set_cspace_hsv,
	set_cspace_hls
    };
    void (*crole_callbacks[N_COLOR_ROLES])(Widget, XtPointer, XtPointer) = {
	set_color_role_start,
	set_color_role_end,
	set_color_role_under,
	set_color_role_over
    };
    char buf[64];
    const int fracbase = 36;

#ifdef CALL_SEQ_DEBUG
    printf("CreateScaleControls called\n");
#endif

    controls = XmCreateForm(parent, name, NULL, 0);

    /* Create the color space menu */
    assert(RGB == 0 && HLS == N_COLOR_SPACES - 1);
    for (n=0; n<N_COLOR_SPACES; ++n)
	menu_titles[n] = XmStringCreateLocalized((char *)colorSpaceName(n));
    menulabel = XmStringCreateLocalized("Color space:");
    space_menu = XmVaCreateSimpleOptionMenu(
	controls, "color_space_menu", menulabel,
	'\0',                               /* mnemonic               */
	dialogData->work.scale->colorspace, /* initial menu selection */
	NULL,                               /* callback function      */
	/* If the option name is XmVaPUSHBUTTON then 
	 * the option values follow in the following order:
	 * label, mnemonic, accelerator, accelerator text.
	 */
	XmVaPUSHBUTTON, menu_titles[RGB],    '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[RGBI],   '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[CIEXYZ], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[CIEUVY], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[CIEXYY], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[CIELAB], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[CIELUV], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[TEKHVC], '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[HSV],    '\0', NULL, NULL,
	XmVaPUSHBUTTON, menu_titles[HLS],    '\0', NULL, NULL,
	XmNleftAttachment,  XmATTACH_FORM,
	XmNtopAttachment,   XmATTACH_FORM,
	NULL);
    XmStringFree(menulabel);
    for (n=0; n<N_COLOR_SPACES; ++n)
	XmStringFree(menu_titles[n]);
    XtManageChild(space_menu);

    /* Add callbacks which reference dialogData */
    for (n=0; n<N_COLOR_SPACES; ++n)
    {
	sprintf(buf, "*color_space_menu.button_%d", n);
	wtmp = XtNameToWidget(controls, buf);
	XtAddCallback(wtmp, XmNactivateCallback,
		      cspace_callbacks[n], dialogData);
    }

    /* The form for the colors. Can't really use RowColumn because
     * there is no way in it to nicely align objects which have
     * dissimilar size.
     */
    align = XmCreateForm(controls, "color_components", NULL, 0);
    XtVaSetValues(align,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, 4, 
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, space_menu,
		  XmNfractionBase, fracbase,
		  NULL);
    str = XmStringCreateLocalized(" ");
    label_end = 24;
    button_end = 32;
    for (i=0; i<N_COLOR_ROLES; ++i)
    {
	label = XmCreateLabelGadget(align, color_titles[i], NULL, 0);
	XtVaSetValues(label,
		      XmNtopAttachment, XmATTACH_POSITION,
		      XmNtopPosition, i*(fracbase/4),
		      XmNbottomAttachment, XmATTACH_POSITION,
		      XmNbottomPosition, (i+1)*(fracbase/4),
		      XmNrightAttachment, XmATTACH_POSITION,
		      XmNrightPosition, label_end,
		      NULL);
	XtManageChild(label);
	button = XmCreatePushButton(align, button_titles[i], NULL, 0);
	dialogData->crole[i] = button;
	XtVaSetValues(button,
		      XmNlabelString, str,
		      XmNtopAttachment, XmATTACH_POSITION,
		      XmNtopPosition, i*(fracbase/4),
		      XmNbottomAttachment, XmATTACH_POSITION,
		      XmNbottomPosition, (i+1)*(fracbase/4),
		      XmNleftAttachment, XmATTACH_POSITION,
		      XmNleftPosition, label_end,
		      XmNrightAttachment, XmATTACH_POSITION,
		      XmNrightPosition, button_end,
		      XmNleftOffset, 5,
		      NULL);
	XtManageChild(button);
	XtAddCallback(button, XmNactivateCallback,
		      crole_callbacks[i], dialogData);
	sprintf(buf, "%s_active", button_titles[i]);
	label = XtVaCreateManagedWidget(
	    buf, xmLabelWidgetClass, align,
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, i*(fracbase/4),
	    XmNbottomAttachment, XmATTACH_POSITION,
	    XmNbottomPosition, (i+1)*(fracbase/4),
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, button_end,
	    XmNlabelString, str,
	    NULL);
	dialogData->activerole[i] = label;
    }
    XmStringFree(str);
    update_color_role_backgrounds(dialogData);
    XtManageChild(align);

    /* The form for the number of colors and cell number */
    ncolors_form = XmCreateForm(controls, "n_colors", NULL, 0);
    XtVaSetValues(ncolors_form,
		  XmNfractionBase, 2,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, align,
		  XmNtopOffset, 2,
		  XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		  XmNrightWidget, space_menu,
		  XmNrightOffset, 4,
		  NULL);

    /* Number of colors */
    ncolors_label = XmCreateLabel(ncolors_form, "Number of colors:", NULL, 0);
    XtVaSetValues(ncolors_label,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 1,
		  NULL);
    XtManageChild(ncolors_label);

    sprintf(buf, "%d", dialogData->orig.scale->ncolors);
    ncolors_entry = XtVaCreateManagedWidget(
	"ncolors_entry", xmTextWidgetClass, ncolors_form,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, ncolors_label,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, 1,
	XmNvalue, buf,
	XmNwidth, 10,
	NULL);
    dialogData->ncolorsentry = ncolors_entry;
    XtAddCallback(ncolors_entry, XmNmodifyVerifyCallback,
		  generic_typing_cb, dialogData);
    XtAddCallback(ncolors_entry, XmNactivateCallback,
		  ncolors_entry_cb, dialogData);

     /* Active cell number */
    cellnum_entry =  XtVaCreateManagedWidget(
	"cellnum_entry", xmTextWidgetClass, ncolors_form,
 	XmNtopAttachment, XmATTACH_FORM,
 	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, 1,
	XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNleftWidget, ncolors_entry,
 	NULL);
    dialogData->cellentry = cellnum_entry;
    XtAddCallback(cellnum_entry, XmNmodifyVerifyCallback,
 		  generic_typing_cb, dialogData);
    XtAddCallback(cellnum_entry, XmNactivateCallback,
		  cellnum_entry_cb, dialogData);
    
    cellnum_label = XmCreateLabel(ncolors_form, "Cell number:", NULL, 0);
    XtVaSetValues(cellnum_label,
		  XmNbottomAttachment, XmATTACH_POSITION,
		  XmNbottomPosition, 1,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_WIDGET,
		  XmNrightWidget, cellnum_entry,
		  NULL);
    XtManageChild(cellnum_label);
    
    XtManageChild(ncolors_form);

    /* Dummy form used only to fill the vertical space 
       when the dialog size is increased vertically */
    stretch = XmCreateForm(controls, "stretch", NULL, 0);
    XtVaSetValues(stretch,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, ncolors_form,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);
    XtManageChild(stretch);

    /* Manage the controls when the form is complete */
    XtManageChild(controls);
    return controls;
}

static Widget CreateColorChooser(Widget parent, char *name,
				 DialogData *dialogData)
{
    int i, n;
    Widget form, nameframe, namelabel, nameentry, oldframe;
    Widget setbutton, clearbutton, clabel, colorwin, lock_ratio;
    Widget stretch, buttonholder, clearset, cframe, dumm;
    Widget scaleframe = NULL;
    char *scale_titles[3] = {"Red", "Green", "Blue"};
    Widget scales[3];
    char buf[64];
    Arg args[20];
    Pixel bg;
    XmString str;

#ifdef CALL_SEQ_DEBUG
    printf("CreateColorChooser called\n");
#endif
    /* Frame doesn't do complicated geometry management, so it can
     * be created managed without any noticeable performance loss.
     */
    form = XmCreateForm(parent, name, NULL, 0);

    /* The frame which contains the color name and the "Set" button */
    nameframe = XmCreateForm(form, "color_form", NULL, 0);
    XtVaSetValues(nameframe,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, BORDER_OFFSET-1,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNverticalSpacing, 1,
		  NULL);

    /* Name label */
    namelabel = XmCreateLabel(nameframe, "Name:", NULL, 0);
    XtVaSetValues(namelabel,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftOffset, 5,
		  NULL);
    XtManageChild(namelabel);

    /* Name entry */
    nameentry = XtVaCreateManagedWidget(
	"color_name", xmTextWidgetClass, nameframe,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, namelabel,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNwidth, 10,
	XmNvalue, "RGBi:0.5/0.5/0.5",
	NULL);
    XtAddCallback(nameentry, XmNactivateCallback,
		  color_text_entry_cb, dialogData);
    XtAddCallback(nameentry, XmNmodifyVerifyCallback,
		  generic_typing_cb, dialogData);
    dialogData->colorentry = nameentry;

    /* Manage the name frame -- it is now complete */
    XtManageChild(nameframe);

    /* The color ratio lock button */
    lock_ratio = XtVaCreateManagedWidget(
	"Lock RGB ratios", xmToggleButtonWidgetClass, form,
	XmNbottomAttachment, XmATTACH_WIDGET,
	XmNbottomWidget, nameframe,
	XmNbottomOffset, BORDER_OFFSET/2,
	XmNleftAttachment, XmATTACH_FORM,
	XmNleftOffset, 35,
	NULL);
    XtAddCallback(lock_ratio, XmNvalueChangedCallback,
		  lock_ratio_cb, (XtPointer)dialogData);

    /* Color scales */
    for (i=0; i<3; ++i)
    {
	/* Frame which holds the scale and the title */
	oldframe = scaleframe;
	sprintf(buf, "color_frame_%s", scale_titles[i]);
	scaleframe = XmCreateForm(form, buf, NULL, 0);
	XtVaSetValues(scaleframe,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNtopOffset, 3,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, lock_ratio,
		      XmNbottomOffset, BORDER_OFFSET/2-1,
		      XmNleftAttachment, i ? XmATTACH_WIDGET : XmATTACH_FORM,
		      NULL);
	if (i > 0)
	    XtVaSetValues(scaleframe,
			  XmNleftWidget, oldframe,
			  NULL);
	else
	    XtVaSetValues(scaleframe,
			  XmNleftOffset, 5,
			  NULL);

	/* Scale title */
	clabel = XmCreateLabel(scaleframe, scale_titles[i], NULL, 0);
	XtVaSetValues(clabel,
		      XmNbottomAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      NULL);
	XtManageChild(clabel);

	/* Color scale */
	n = 0;
	XtSetArg(args[n], XmNminimum, 0); n++;
	XtSetArg(args[n], XmNmaximum, COLOR_SLIDER_MAX); n++;
	XtSetArg(args[n], XmNvalue, COLOR_SLIDER_MAX/2); n++;
	XtSetArg(args[n], XmNdecimalPoints, 3); n++;
	XtSetArg(args[n], XmNshowValue, True); n++;
	XtSetArg(args[n], XmNeditable, True); n++;
	XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, clabel); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
#ifdef linux
        /* The XmTHERMOMETER mode works only in Motif 2.x. 
	   We only have Motif 1.2 on our SGIs and SUNs */
  	XtSetArg(args[n], XmNslidingMode, XmTHERMOMETER); n++;
#endif
	XtSetArg(args[n], XmNscaleMultiple, 1); n++;
	/* Disable the scale highlighting */
	XtVaGetValues(scaleframe, XmNbackground, &bg, NULL);
	XtSetArg(args[n], XmNhighlightColor, bg); n++;
	XtSetArg(args[n], XmNhighlightPixmap, NULL); n++;
	assert(n <= 20);
	scales[i] = XmCreateScale(scaleframe, scale_titles[i], args, n);
	dialogData->rgb_scales[i] = scales[i];
#ifdef linux
	set_trough_color(scales[i], dialogData->prime_colors[i]);
#endif
	XtAddCallback(scales[i], XmNvalueChangedCallback,
		      rgbi_scale_movement_cb, dialogData);
	XtAddCallback(scales[i], XmNdragCallback,
		      rgbi_scale_movement_cb, dialogData);
	XtManageChild(scales[i]);

	XtManageChild(scaleframe);
    }

    /* Frame for "Set" and "Clear" buttons */
    clearset = XmCreateForm(form, "clearset", NULL, 0);
    XtVaSetValues(clearset,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, BORDER_OFFSET,
		  XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		  XmNbottomWidget, lock_ratio,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, scaleframe,
		  XmNleftOffset, 9,
		  NULL);

    /* Clear color name button */
    buttonholder = XmCreateForm(clearset, "buttonholder", NULL, 0);
    XtVaSetValues(buttonholder,
		  XmNfractionBase, 2,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  NULL);
    clearbutton = XmCreatePushButton(buttonholder, " Clear ", NULL, 0);
    XtVaSetValues(clearbutton,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 1,
		  XmNrightOffset, 1,
		  XmNheight, ACTION_AREA_HEIGHT,
		  NULL);
    XtAddCallback(clearbutton, XmNactivateCallback,
		  color_clear_button_cb, dialogData);
    XtManageChild(clearbutton);

    /* Color set button */
    setbutton = XmCreatePushButton(buttonholder, "Set", NULL, 0);
    XtVaSetValues(setbutton,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 1,
		  XmNleftOffset, 1,
		  XmNsensitive, False,
		  XmNheight, ACTION_AREA_HEIGHT,
		  NULL);
    XtAddCallback(setbutton, XmNactivateCallback,
		  color_set_button_cb, dialogData);
    XtManageChild(setbutton);
    dialogData->setbutton = setbutton;
    XtManageChild(buttonholder);

    /* Stretching space */
    stretch = XmCreateForm(clearset, "stretch", NULL, 0);
    XtVaSetValues(stretch,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_WIDGET,
		  XmNrightWidget, buttonholder,
		  NULL);
    XtManageChild(stretch);

    XtManageChild(clearset);

    /* Frame for the color drawing area */
    cframe = XtVaCreateManagedWidget(
	"color_frame", xmFrameWidgetClass, form,
	XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNtopWidget, scaleframe,
	XmNtopOffset, 5,
	XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNrightWidget, clearset,
	XmNrightOffset, 1,
	XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNleftWidget, clearset,
	XmNleftOffset, 2,
	XmNbottomAttachment, XmATTACH_WIDGET,
	XmNbottomWidget, clearset,
	XmNbottomOffset, BORDER_OFFSET/2+get_widget_height(clabel)-3,
	NULL);
    dumm = XmCreateForm(cframe, "dumm_form", NULL, 0);
    XtVaSetValues(dumm,
		  XmNfractionBase, 6,
		  NULL);

    /* Color drawing area */
    str = XmStringCreateLocalized(" ");
    XtVaCreateManagedWidget(
	"white_window", xmLabelGadgetClass, dumm,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, 2,
	XmNbackground, WhitePixelOfScreen(XtScreen(dialogData->dialog)),
	XmNlabelString, str,
	NULL);
    XtVaCreateManagedWidget(
	"bg_window", xmLabelGadgetClass, dumm,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, 2,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, 4,
	XmNlabelString, str,
	NULL);
    XtVaCreateManagedWidget(
	"black_window", xmLabelGadgetClass, dumm,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, 4,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNbackground, BlackPixelOfScreen(XtScreen(dialogData->dialog)),
	XmNlabelString, str,
	NULL);
    XmStringFree(str);
    colorwin = XtVaCreateManagedWidget(
	"color_window", xmDrawingAreaWidgetClass, dumm,
	XmNleftAttachment, XmATTACH_POSITION,
	XmNleftPosition, 1,
	XmNrightAttachment, XmATTACH_POSITION,
	XmNrightPosition, 5,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, 1,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, 5,
	NULL);
    dialogData->colorwin = colorwin;

    XtManageChild(dumm);

    /* Manage the form when it is complete */
    XtManageChild(form);
    return form;
}

static void set_trough_color(Widget w, Pixel selectColor)
{
    WidgetList *kids;
    int nkids;
    Arg argList[1], tmpargs[2];
    int i, s, t;

#ifdef CALL_SEQ_DEBUG
    printf("set_trough_color called\n");
#endif
    i = 0;
    XtSetArg(argList[i], XmNtroughColor, selectColor); i++;
    
    /* Unfortunately, scale does not have a direct way
     * to get its scrollbar widget, so use Composite resources.
     */
    s = 0;
    XtSetArg(tmpargs[s], XmNnumChildren, &nkids); s++;
    XtSetArg(tmpargs[s], XmNchildren, &kids); s++;
    XtGetValues(w, tmpargs, s);
    for (t = 0; t < nkids; t++)
	if (XmIsScrollBar((Widget)kids[t]))
	    XtSetValues((Widget)kids[t], argList, i);
}

static Widget CreateDataMappingControls(Widget parent, char *name,
					DialogData *dialogData)
{
    int n, init_menu;
    Widget frame, form, framelabel, menu_form, range_type, scale_type;
    Widget limit_form, minlabel, maxlabel, minentry, maxentry, use_data_b;
    XmString str, linear_title, log_title;
    const int entrywidth = 80;
    Arg args[10];
    char buf[32];

#ifdef CALL_SEQ_DEBUG
    printf("CreateDataMappingControls called\n");
#endif

    frame = XtVaCreateManagedWidget(name, xmFrameWidgetClass, parent,
				    XmNmarginHeight, BORDER_OFFSET-3,
				    XmNmarginWidth, BORDER_OFFSET-2,
				    NULL);
    n = 0;
    XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
    XtSetArg(args[n], XmNchildVerticalAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNchildHorizontalAlignment, XmALIGNMENT_CENTER); n++;
    framelabel = XmCreateLabelGadget(
	frame, "Data mapping to the color scale", args, n);
    XtManageChild(framelabel);
    form = XmCreateForm(frame, "mapping_controls", NULL, 0);

    /* Form to chose range and mapping types */
    menu_form = XmCreateForm(form, "mapping_menus", NULL, 0);
    XtVaSetValues(menu_form,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, 1,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_FORM,
		  NULL);

    str = XmStringCreateLocalized("Mapped range is");
    linear_title = XmStringCreateLocalized("static");
    log_title = XmStringCreateLocalized("dynamic");
    if (dialogData->orig.rangeIsDynamic)
	init_menu = 1;
    else
	init_menu = 0;
    range_type = XmVaCreateSimpleOptionMenu(
	menu_form, "range_type_menu", str,
	'\0',              /* mnemonic               */
	init_menu,         /* initial menu selection */
	NULL,              /* callback function      */
	XmVaPUSHBUTTON, linear_title, '\0', NULL, NULL,
	XmVaPUSHBUTTON, log_title, '\0', NULL, NULL,
	XmNleftAttachment,  XmATTACH_FORM,
	XmNtopAttachment,   XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    XmStringFree(str);
    XmStringFree(linear_title);
    XmStringFree(log_title);
    XtManageChild(range_type);

    /* Set up menu callbacks */
    XtAddCallback(XtNameToWidget(menu_form, "*range_type_menu.button_0"),
		  XmNactivateCallback, set_static_range, dialogData);
    XtAddCallback(XtNameToWidget(menu_form, "*range_type_menu.button_1"),
		  XmNactivateCallback, set_dynamic_range, dialogData);

    str = XmStringCreateLocalized("Scale is");
    linear_title = XmStringCreateLocalized("linear");
    log_title = XmStringCreateLocalized("logarithmic");
    if (dialogData->orig.mappingIsLog)
	init_menu = 1;
    else
	init_menu = 0;
    scale_type = XmVaCreateSimpleOptionMenu(
	menu_form, "scale_type_menu", str,
	'\0',              /* mnemonic               */
	init_menu,         /* initial menu selection */
	NULL,              /* callback function      */
	XmVaPUSHBUTTON, linear_title, '\0', NULL, NULL,
	XmVaPUSHBUTTON, log_title, '\0', NULL, NULL,
	XmNleftAttachment,  XmATTACH_WIDGET,
	XmNleftWidget, range_type,
	XmNleftOffset, 35,
	XmNtopAttachment,   XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    XmStringFree(str);
    XmStringFree(linear_title);
    XmStringFree(log_title);
    XtManageChild(scale_type);

    /* Set up menu callbacks */
    XtAddCallback(XtNameToWidget(menu_form, "*scale_type_menu.button_0"),
		  XmNactivateCallback, set_linear_scale, dialogData);
    XtAddCallback(XtNameToWidget(menu_form, "*scale_type_menu.button_1"),
		  XmNactivateCallback, set_log_scale, dialogData);

    XtManageChild(menu_form);

    /* Form for the scale range limits */
    limit_form = XmCreateForm(form, "mapping_limits", NULL, 0);
    XtVaSetValues(limit_form,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, 2,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, menu_form,
		  NULL);
    
    str = XmStringCreateLocalized("Range begins at:");
    minlabel = XtVaCreateManagedWidget(
	"z_min_label", xmLabelWidgetClass, limit_form,
	XmNleftAttachment, XmATTACH_FORM,
	XmNleftOffset, 2,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNlabelString, str,
	NULL);
    XmStringFree(str);

    minentry = XtVaCreateManagedWidget(
	"z_min_entry", xmTextWidgetClass, limit_form,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, minlabel,
	XmNtopAttachment, XmATTACH_FORM,
  	XmNbottomAttachment, XmATTACH_FORM,
	XmNwidth, entrywidth,
	NULL);
    dialogData->minentry = minentry;
    sprintf(buf, "%g", dialogData->work.colorMin);
    XmTextSetString(minentry, buf);
    XtAddCallback(minentry, XmNmodifyVerifyCallback,
		  generic_typing_cb, dialogData);
    XtAddCallback(minentry, XmNactivateCallback,
		  set_limit_cb, (XtPointer)dialogData);

    str = XmStringCreateLocalized("ends at:");
    maxlabel = XtVaCreateManagedWidget(
	"z_max_label", xmLabelWidgetClass, limit_form,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, minentry,
	XmNleftOffset, 5,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNlabelString, str,
	NULL);
    XmStringFree(str);

    maxentry = XtVaCreateManagedWidget(
	"z_max_entry", xmTextWidgetClass, limit_form,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, maxlabel,
	XmNtopAttachment, XmATTACH_FORM,
  	XmNbottomAttachment, XmATTACH_FORM,
	XmNwidth, entrywidth,
	NULL);
    dialogData->maxentry = maxentry;
    sprintf(buf, "%g", dialogData->work.colorMax);
    XmTextSetString(maxentry, buf);
    XtAddCallback(maxentry, XmNmodifyVerifyCallback,
		  generic_typing_cb, (XtPointer)dialogData);
    XtAddCallback(maxentry, XmNactivateCallback,
		  set_limit_cb, (XtPointer)dialogData);

    str = XmStringCreateLocalized(" Use data range ");
    use_data_b = XtVaCreateManagedWidget(
	"use_data_b", xmPushButtonWidgetClass, limit_form,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_WIDGET,
	XmNleftWidget, maxentry,
	XmNleftOffset, 10,
	XmNlabelString, str,
	NULL);
    XmStringFree(str);
    dialogData->use_data_b = use_data_b;
    if (dialogData->orig.limits_cb)
	XtAddCallback(use_data_b, XmNactivateCallback,
		      get_data_range_cb, (XtPointer)dialogData);
    else
	XtVaSetValues(use_data_b,
		      XmNsensitive, False,
		      NULL);

    /* Deactivate limit entries for dynamic mapping */
    if (dialogData->orig.rangeIsDynamic)
	set_dynamic_range(range_type, (XtPointer)dialogData, NULL);

    XtManageChild(limit_form);
    XtManageChild(form);
    return frame;
}

static DialogData *create_dialog_data(Widget dialog,
				      ColorScaleArgs *parentData)
{
    DialogData *dialogData;
    char buf[64];
    XcmsColor color;

#ifdef CALL_SEQ_DEBUG
    printf("create_dialog_data called\n");
#endif
    dialogData = (DialogData *)calloc(1, sizeof(DialogData));
    if (dialogData == NULL)
    {
	fprintf(stderr, "Error in create_dialog_data: out of memory\n");
	return NULL;
    }

    /* Copy the arguments two times: one as the backup 
       and one as the working area for the dialog */
    dialogData->orig = *parentData;
    dialogData->orig.scale = backupColorScale(parentData->scale);
    if (dialogData->orig.scale == NULL)
    {
	fprintf(stderr, "Error in create_dialog_data: out of memory\n");
	return NULL;
    }
    incrColorScaleRefCount(dialogData->orig.scale);

    dialogData->work = *parentData;
    unusedColorScaleName(buf);
    dialogData->work.scale = cloneColorScale(buf, parentData->scale);
    if (dialogData->work.scale == NULL)
    {
	fprintf(stderr, "Error in create_dialog_data: out of memory\n");
	return NULL;
    }
    incrColorScaleRefCount(dialogData->work.scale);

    dialogData->dialog = dialog;
    dialogData->choicecolor.format = XcmsRGBiFormat;
    dialogData->choicecolor.spec.RGBi.red = 0.5;
    dialogData->choicecolor.spec.RGBi.green = 0.5;
    dialogData->choicecolor.spec.RGBi.blue = 0.5;
    AllocSharedColor(XtDisplay(dialog),
	DefaultColormapOfScreen(XtScreen(dialog)),
	&dialogData->choicecolor, XcmsRGBiFormat);
    dialogData->color_role = -1;
    dialogData->active_cell = -1;
    dialogData->locked_value[RED] = 1;
    dialogData->locked_value[GREEN] = 1;
    dialogData->locked_value[BLUE] = 1;
    XtVaGetValues(dialog,
		  XmNbackground, &dialogData->background,
		  XmNforeground, &dialogData->foreground,
		  NULL);

    /* Allocate prime colors */
    color.format = XcmsRGBiFormat;

    color.spec.RGBi.red = 1.0;
    color.spec.RGBi.green = 0.0;
    color.spec.RGBi.blue = 0.0;
    AllocSharedColor(XtDisplay(dialog),
	DefaultColormapOfScreen(XtScreen(dialog)),
	&color, XcmsRGBiFormat);
    dialogData->prime_colors[RED] = color.pixel;

    color.spec.RGBi.red = 0.0;
    color.spec.RGBi.green = 1.0;
    color.spec.RGBi.blue = 0.0;
    AllocSharedColor(XtDisplay(dialog),
	DefaultColormapOfScreen(XtScreen(dialog)),
	&color, XcmsRGBiFormat);
    dialogData->prime_colors[GREEN] = color.pixel;

    color.spec.RGBi.red = 0.0;
    color.spec.RGBi.green = 0.0;
    color.spec.RGBi.blue = 1.0;
    AllocSharedColor(XtDisplay(dialog),
	DefaultColormapOfScreen(XtScreen(dialog)),
	&color, XcmsRGBiFormat);
    dialogData->prime_colors[BLUE] = color.pixel;

    /* Font list for drawing the data scale labels */
    dialogData->font = XmFontListCopy(_XmGetDefaultFontList(
	dialog, XmLABEL_FONTLIST));

    return dialogData;
}

static void destroy_dialog_data(DialogData *dialogData)
{
    Display *dpy = XtDisplay(dialogData->dialog);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));

#ifdef CALL_SEQ_DEBUG
    printf("destroy_dialog_data called\n");
#endif
    decrColorScaleRefCount(dialogData->orig.scale);
    decrColorScaleRefCount(dialogData->work.scale);
    decrColorScaleRefCount(dialogData->saved_default);
    FreeSharedColors(dpy, cmap, &dialogData->choicecolor.pixel, 1, 0);
    FreeSharedColors(dpy, cmap, dialogData->prime_colors, 3, 0);
    XmFontListFree(dialogData->font);
    free(dialogData);
}

static void set_colorspace(Widget w, XtPointer client_data,
			   XtPointer call_data, ColorSpace s)
{
    DialogData *dialogData = (DialogData *)client_data;

#ifdef CALL_SEQ_DEBUG
    printf("set_colorspace called\n");
#endif
    if (dialogData->work.scale->colorspace != s)
    {
	unhighlight_active_cell(dialogData);
	setColorSpace(dialogData->work.scale, s);
	redraw_color_scale(dialogData);
    }
}

static void set_static_range(Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
#ifdef CALL_SEQ_DEBUG
    printf("set_static_range called\n");
#endif
    dialogData->work.rangeIsDynamic = 0;
    XtVaSetValues(dialogData->minentry,
		  XmNsensitive, True,
		  NULL);
    XtVaSetValues(dialogData->maxentry,
		  XmNsensitive, True,
		  NULL);
    XtVaSetValues(dialogData->use_data_b,
		  XmNsensitive, True,
		  NULL);
    redraw_data_axis(dialogData);
}

static void set_dynamic_range(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
#ifdef CALL_SEQ_DEBUG
    printf("set_dynamic_range called\n");
#endif
    dialogData->work.rangeIsDynamic = 1;
    XtVaSetValues(dialogData->minentry,
		  XmNsensitive, False,
		  NULL);
    XtVaSetValues(dialogData->maxentry,
		  XmNsensitive, False,
		  NULL);
    XtVaSetValues(dialogData->use_data_b,
		  XmNsensitive, False,
		  NULL);
    redraw_data_axis(dialogData);
}

static void set_log_scale(Widget w, XtPointer client_data,
			  XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
#ifdef CALL_SEQ_DEBUG
    printf("set_log_scale called\n");
#endif
    dialogData->work.mappingIsLog = 1;
    redraw_data_axis(dialogData);
}

static void set_linear_scale(Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
#ifdef CALL_SEQ_DEBUG
    printf("set_linear_scale called\n");
#endif
    dialogData->work.mappingIsLog = 0;
    redraw_data_axis(dialogData);
}

static void scale_input_cb(Widget w, XtPointer client_data,
			   XtPointer call_data)
{
    Position x;
    DialogData *dialogData = (DialogData *)client_data;
    Widget drawing_a = dialogData->scale_area;
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)call_data;
    XEvent *event = cbs->event;
    int i, width, ncolors = dialogData->work.scale->ncolors;
    KeySym key;
    XComposeStatus compose;
    int update_chooser = 1;

#ifdef CALL_SEQ_DEBUG
    printf("scale_input_cb called with %s event\n", event_names[event->xany.type]);
#endif
    /* Respond to mouse clicks and arrow presses. Set up variable i
     * which will be the new color cell number to use.
     */
    switch (event->xany.type)
    {
    case KeyPress:
	if (dialogData->active_cell < 0)
	    return;
	XLookupString((XKeyEvent *)event, NULL, 0, &key, &compose);
	if (key == XK_Left && dialogData->active_cell > 0)
	    i = dialogData->active_cell - 1;
	else if (key == XK_Right && dialogData->active_cell < ncolors-1)
	    i = dialogData->active_cell + 1;
	else if (key == XK_space)
	    i = dialogData->active_cell;
	else
	    return;
	break;
    case ButtonPress:
	unhighlight_color_role(dialogData);
	x = event->xbutton.x;
	width = get_widget_width(drawing_a) - 2*DATA_AXIS_PAD_SIDE;
	if (width <= 0)
	    return;
	i = (int)(((double)(x-DATA_AXIS_PAD_SIDE)/width)*ncolors);
	if (i >= ncolors)
	    i = ncolors-1;
	else if (i < 0)
	    i = 0;
	/* For the left button click, go to that cell.
	 *
         * For the middle button click, go to that cell
         * and change the color of the chooser to the color
         * of the cell.
         *
	 * For the right button click, move one position in the
	 * direction of the click.
	 */
	if (event->xbutton.button == 1)
	    update_chooser = 0;
	else if (event->xbutton.button == 2)
	    ;
	else if (event->xbutton.button == 3 && dialogData->active_cell >= 0)
	{
	    update_chooser = 0;
	    if (i > dialogData->active_cell &&
		dialogData->active_cell < ncolors-1)
		i = dialogData->active_cell+1;
	    else if (i < dialogData->active_cell &&
		     dialogData->active_cell > 0)
		i = dialogData->active_cell-1;
	}
	else
	    return;
	break;
    default:
	return;
    }
    highlight_active_cell(dialogData, i);
    if (update_chooser)
	set_rgbscales_bycell(dialogData, i);
}

static void set_rgbscales_bycell(DialogData *dialogData, int cell)
{
    XcmsColor cmsColor;
    Pixel oldpixel;
    Display *dpy;
    Colormap cmap;
    char buf[64];

    assert(cell >= 0 && cell < dialogData->work.scale->ncolors);
    dpy = XtDisplay(dialogData->dialog);
    cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));
    cmsColor.pixel = dialogData->work.scale->pixels[cell];
    XcmsQueryColor(dpy, cmap, &cmsColor, XcmsRGBiFormat);
    oldpixel = dialogData->choicecolor.pixel;
    dialogData->choicecolor = cmsColor;
    dialogData->choicecolor.pixel = oldpixel;
    refresh_rgbscales(dialogData);
    refresh_colorwin(dialogData);
    printColorName(dpy, cmap, buf, dialogData->work.scale->colorspace,
		   dialogData->choicecolor.spec.RGBi.red,
		   dialogData->choicecolor.spec.RGBi.green,
		   dialogData->choicecolor.spec.RGBi.blue);
    set_chooser_colorname(dialogData, buf);    
}

static void redraw_data_axis(DialogData *dialogData)
{
    Dimension width, height;
    int right_border, left_border;
    Widget drawing_a = dialogData->scale_area;
    Display *dpy;
    Window win;
    XFontStruct *fs;
    Pixel bgpix;

#ifdef CALL_SEQ_DEBUG
    printf("redraw_data_axis called\n"); fflush(stdout);
#endif
    if (!dialogData->gravity_adjusted)
	/* There was no expose event yet */
	return;

    /* First, clear the drawing area */
    XtVaGetValues(drawing_a,
		  XmNwidth, &width,
		  XmNheight, &height,
		  XmNbackground, &bgpix,
		  NULL);
    dpy = XtDisplay(drawing_a);
    win = XtWindow(drawing_a);
    XSetForeground(dpy, globalGC, bgpix);
    XFillRectangle(dpy, win, globalGC, 0, COLOR_SCALE_HEIGHT, width,
		   height-COLOR_SCALE_HEIGHT);

    /* Now, redraw the axis */
    left_border = DATA_AXIS_PAD_SIDE;
    right_border = width - DATA_AXIS_PAD_SIDE;
    if (right_border <= left_border)
	return;
    fs = getFontStruct(dialogData->font);
    XSetForeground(dpy, globalGC, BlackPixelOfScreen(XtScreen(drawing_a)));
    if (dialogData->work.rangeIsDynamic)
    {
	/* Use some dummy axis limits */
	if (dialogData->work.mappingIsLog)
	    DrawHorizontalAxis(dpy, win, globalGC, fs, X_SCREEN, COLOR_SCALE_HEIGHT,
			       left_border, right_border,
			       1.0, 10.0, 1.0, 10.0, True, 0, NULL, 0);
	else
	    DrawHorizontalAxis(dpy, win, globalGC, fs, X_SCREEN, COLOR_SCALE_HEIGHT,
			       left_border, right_border,
			       0.0, 1.0, 0.0, 1.0, False, 0, NULL, 0);
    }
    else
    {
	if (!dialogData->work.mappingIsLog ||
	    (dialogData->work.colorMin > 0.f && dialogData->work.colorMax > 0.f))
	    DrawHorizontalAxis(dpy, win, globalGC, fs, X_SCREEN, COLOR_SCALE_HEIGHT,
			       left_border, right_border,
			       dialogData->work.colorMin,
			       dialogData->work.colorMax,
			       dialogData->work.colorMin,
			       dialogData->work.colorMax,
			       dialogData->work.mappingIsLog, 0, NULL, 0);
    }
}

static void redraw_color_scale(DialogData *dialogData)
{
    int i, ncolors, x1, x2, center, width, height, right_border;
    double dx;
    XPoint marker[4];
    Widget drawing_a = dialogData->scale_area;
    Display *dpy = XtDisplay(drawing_a);
    Window win = XtWindow(drawing_a);
    XRectangle sides[2];
    Pixel bgpix;
    XcmsColor colordef;
    Colormap cmap;

#ifdef CALL_SEQ_DEBUG
    printf("redraw_color_scale called\n"); fflush(stdout);
#endif
    width = get_widget_width(drawing_a);
    right_border = width + 1 - DATA_AXIS_PAD_SIDE;
    if (right_border <= DATA_AXIS_PAD_SIDE)
	return;
    height = COLOR_SCALE_HEIGHT;
    ncolors = dialogData->work.scale->ncolors;
    dx = (right_border - DATA_AXIS_PAD_SIDE)/(double)ncolors;
    for (i=0; i<ncolors; ++i)
    {
	x1 = (int)(i*dx) + DATA_AXIS_PAD_SIDE;
	x2 = (int)((i+1)*dx) + DATA_AXIS_PAD_SIDE;
	if (x2 > right_border || i == ncolors-1)
	    x2 = right_border;
	if (x1 < x2)
	{
	    XSetForeground(dpy, globalGC, dialogData->work.scale->pixels[i]);
	    XFillRectangle(dpy, win, globalGC, x1, 0, x2-x1, height);
	}
    }

    if (dialogData->active_cell >= 0)
    {
	cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));
	i = dialogData->active_cell;
	x1 = (int)(i*dx) + DATA_AXIS_PAD_SIDE;
	x2 = (int)((i+1)*dx) + DATA_AXIS_PAD_SIDE;
	if (x2 > right_border)
	    x2 = right_border;
	center = (x1+x2)/2;

	/* Figure out a contrast color for the marker (black or white) */
	colordef.pixel = dialogData->work.scale->pixels[i];
	XcmsQueryColor(dpy, cmap, &colordef, XcmsRGBiFormat);
	if (colordef.spec.RGBi.red + colordef.spec.RGBi.green + 
	    0.5*colordef.spec.RGBi.blue >= 1.0)
	    XSetForeground(dpy, globalGC, BlackPixelOfScreen(XtScreen(drawing_a)));
	else
	    XSetForeground(dpy, globalGC, WhitePixelOfScreen(XtScreen(drawing_a)));

	/* Draw a diamond-shaped marker */
	marker[0].x = center + MARKER_WIDTH/2;
	marker[0].y = height - MARKER_HEIGHT/2;
	marker[1].x = center;
	marker[1].y = height;
	marker[2].x = center - MARKER_WIDTH/2;
	marker[2].y = height - MARKER_HEIGHT/2;
	marker[3].x = center;
	marker[3].y = height - MARKER_HEIGHT + 1;
	XFillPolygon(dpy, win, globalGC, marker, XtNumber(marker),
		     Convex, CoordModeOrigin);
    }

    sides[0].x = 0;
    sides[0].y = 0;
    sides[0].width = DATA_AXIS_PAD_SIDE;
    sides[0].height = height+1;
    sides[1].x = right_border;
    sides[1].y = 0;
    sides[1].width = DATA_AXIS_PAD_SIDE;
    sides[1].height = height+1;
    XtVaGetValues(drawing_a,
		  XmNbackground, &bgpix,
		  NULL);
    XSetForeground(dpy, globalGC, bgpix);
    XFillRectangles(dpy, win, globalGC, sides, 2);
}

static void scale_expose_cb(Widget w, XtPointer client_data,
			    XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    XSetWindowAttributes attrs;
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)call_data;
    XEvent *event = cbs->event;

#ifdef CALL_SEQ_DEBUG
    printf("scale_expose_cb called\n");
#endif
    if (!dialogData->gravity_adjusted)
    {
	/* Set the bit gravity to "ForgetGravity"
	 * to ensure that we get expose events even when
	 * the window size shrinks. This can not be done
	 * in the window creation routine because at that
	 * time the correct window id is not known yet.
	 */
	attrs.bit_gravity = ForgetGravity;
	XChangeWindowAttributes(XtDisplay(w), XtWindow(w), CWBitGravity, &attrs);
	dialogData->gravity_adjusted = 1;
    }
    if (event->type == Expose)
	if (event->xexpose.count > 0)
	    return;
    redraw_color_scale(dialogData);
    redraw_data_axis(dialogData);
}

static void rgbi_scale_movement_cb(Widget w, XtPointer client_data,
				   XtPointer call_data)
{
    PrimeColor i, cl, cmax = RED;
    DialogData *dialogData = (DialogData *)client_data;
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
    int oldtotal = 0, oldmax = -1, newcolors[3], out_of_range = 0;
    XcmsFloat frac[3], newtotal;

#ifdef CALL_SEQ_DEBUG
    printf("rgbi_scale_movement_cb called\n");
#endif
    for (cl = RED; cl <= BLUE; ++cl)
	if (w == dialogData->rgb_scales[cl])
	    break;
    assert(cl <= BLUE);
    if (dialogData->ratios_locked)
    {
	if (dialogData->locked_value[cl] == 0)
	{
	    XmScaleSetValue(dialogData->rgb_scales[cl], 0);
	    return;
	}
	for (i = RED; i <= BLUE; ++i)
	{
	    oldtotal += dialogData->locked_value[i];
	    if (dialogData->locked_value[i] > oldmax)
	    {
		oldmax = dialogData->locked_value[i];
		cmax = i;
	    }
	}
	for (i = RED; i <= BLUE; ++i)
	    frac[i] = dialogData->locked_value[i]/(XcmsFloat)oldtotal;
	newtotal = cbs->value/(XcmsFloat)COLOR_SLIDER_MAX/frac[cl];
	for (i = RED; i <= BLUE; ++i)
	{
	    newcolors[i] = (int)(1.0e-10+newtotal*frac[i]*COLOR_SLIDER_MAX);
	    if (newcolors[i] > COLOR_SLIDER_MAX)
		out_of_range = 1;
	}
	if (out_of_range)
	{
	    newtotal = 1.0/frac[cmax];
	    for (i = RED; i <= BLUE; ++i)
		newcolors[i] = (int)(1.0e-10+newtotal*frac[i]*COLOR_SLIDER_MAX);
	}
	dialogData->choicecolor.spec.RGBi.red = 
	    newcolors[RED]/(XcmsFloat)COLOR_SLIDER_MAX;
	dialogData->choicecolor.spec.RGBi.green = 
	    newcolors[GREEN]/(XcmsFloat)COLOR_SLIDER_MAX;
	dialogData->choicecolor.spec.RGBi.blue = 
	    newcolors[BLUE]/(XcmsFloat)COLOR_SLIDER_MAX;
	for (i = RED; i <= BLUE; ++i)
	    XmScaleSetValue(dialogData->rgb_scales[i], newcolors[i]);
    }
    else
    {
	switch (cl)
	{
	case RED:
	    dialogData->choicecolor.spec.RGBi.red = 
		cbs->value/(XcmsFloat)COLOR_SLIDER_MAX;
	    break;
	case GREEN:
	    dialogData->choicecolor.spec.RGBi.green = 
		cbs->value/(XcmsFloat)COLOR_SLIDER_MAX;
	    break;
	case BLUE:
	    dialogData->choicecolor.spec.RGBi.blue = 
		cbs->value/(XcmsFloat)COLOR_SLIDER_MAX;
	    break;
	default:
	    assert(0);
	}
    }
    refresh_colorwin(dialogData);
    refresh_color_name(dialogData);
}

static void refresh_color_name(DialogData *dialogData)
{
    Display *dpy  = XtDisplay(dialogData->dialog);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));
    char buf[64];

#ifdef CALL_SEQ_DEBUG
    printf("refresh_color_name called\n");
#endif
    printColorName(dpy, cmap, buf, dialogData->work.scale->colorspace,
		   dialogData->choicecolor.spec.RGBi.red,
		   dialogData->choicecolor.spec.RGBi.green,
		   dialogData->choicecolor.spec.RGBi.blue);
    set_chooser_colorname(dialogData, buf);
}

static void refresh_colorwin(DialogData *dialogData)
{
    Display *dpy  = XtDisplay(dialogData->dialog);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));

#ifdef CALL_SEQ_DEBUG
    printf("refresh_colorwin called\n");
#endif
    FreeSharedColors(dpy, cmap, &dialogData->choicecolor.pixel, 1, 0);
    AllocSharedColor(dpy, cmap, &dialogData->choicecolor, XcmsRGBiFormat);
    XtVaSetValues(dialogData->colorwin,
		  XmNbackground, dialogData->choicecolor.pixel,
		  NULL);
}

static void set_color_role(Widget w, XtPointer client_data,
			   XtPointer call_data, ColorRole crole)
{
    DialogData *dialogData = (DialogData *)client_data;
    XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *)call_data;
    const char *colorname;

#ifdef CALL_SEQ_DEBUG
    printf("set_color_role called\n");
#endif
    unhighlight_active_cell(dialogData);
    if (dialogData->color_role != (int)crole)
    {
	unhighlight_color_role(dialogData);
	highlight_color_role(dialogData, crole);
    }
    if (cbs->click_count > 1)
    {
	/* Transfer the color settings to the color chooser */
	colorname = colorname_from_role(dialogData, crole);
	set_rgbscales_byname(dialogData, colorname);
	set_chooser_colorname(dialogData, colorname);
    }
}

static void color_text_entry_cb(Widget w, XtPointer client_data,
				XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    char *textcolor;
    const char *color;
    char buf[64];

#ifdef CALL_SEQ_DEBUG
    printf("color_text_entry_cb called\n");
#endif
    textcolor = XmTextGetString(dialogData->colorentry);
    color = normalizeColorName(textcolor, buf);
    if (set_rgbscales_byname(dialogData, color) == 0)
	XtVaSetValues(dialogData->colorentry,
		      XmNforeground, dialogData->foreground,
		      NULL);
    XtFree(textcolor);
}

static void color_clear_button_cb(Widget w, XtPointer client_data,
				  XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
#ifdef CALL_SEQ_DEBUG
    printf("color_clear_button_cb called\n");
#endif
    set_chooser_colorname(dialogData, "");
}

static void color_set_button_cb(Widget w, XtPointer client_data,
				XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    Display *dpy = XtDisplay(dialogData->dialog);
    Colormap cmap = dialogData->work.scale->colormap;
    Status status;
    char *textcolor, *tmp;
    const char *color;
    char buf[64];
    XcmsColor need_color;

    if (dialogData->color_role < 0 && dialogData->active_cell < 0)
	return;
    assert(dialogData->color_role < 0 || dialogData->active_cell < 0);

#ifdef CALL_SEQ_DEBUG
    printf("color_set_button_cb called\n");
#endif
    textcolor = XmTextGetString(dialogData->colorentry);
    color = normalizeColorName(textcolor, buf);
    if (set_rgbscales_byname(dialogData, color))
    {
	XtFree(textcolor);
	return;
    }

    /* We have a good color name. Get the corresponding pixel.
     * We are relying on the "set_rgbscales_byname" function
     * to set dialogData->choicecolor. Pixel needs to be allocated
     * again because setStartPixel, setEndPixel, and setScalePixel
     * assume the pixel ownership.
     */
    need_color = dialogData->choicecolor;
    status = AllocSharedColor(dpy, cmap, &need_color, XcmsRGBiFormat);
    if (status == XcmsFailure)
    {
	tmp = (char *)malloc(strlen(textcolor) + 64);
	if (tmp == NULL)
	{
	    fprintf(stderr, "Out of memory!\n");
	    return;
	}
	sprintf(tmp, "Failed to allocate color \"%s\"", textcolor);
	show_error_message(dialogData->dialog, tmp);
	free(tmp);
    }
    XtFree(textcolor);
    if (status == XcmsFailure)
	return;

    if (dialogData->color_role >= 0)
    {
	switch (dialogData->color_role)
	{
	case START_COLOR:
	    setStartPixel(dialogData->work.scale, need_color.pixel);
	    redraw_color_scale(dialogData);
	    break;
	case END_COLOR:
	    setEndPixel(dialogData->work.scale, need_color.pixel);
	    redraw_color_scale(dialogData);
	    break;
	case UNDER_COLOR:
	    setScalePixel(dialogData->work.scale, -1, need_color.pixel);
	    break;
	case OVER_COLOR:
	    setScalePixel(dialogData->work.scale,
			  dialogData->work.scale->ncolors,
			  need_color.pixel);
	    break;
	default:
	    assert(0);
	}
	update_color_role_backgrounds(dialogData);
    }

    else if (dialogData->active_cell >= 0)
    {
	/* Modify the contents of this particular color cell */
	setScalePixel(dialogData->work.scale,
		      dialogData->active_cell,
		      need_color.pixel);
	redraw_color_scale(dialogData);
    }

    /* Change the foreround of the color name entry */
    XtVaSetValues(dialogData->colorentry,
		  XmNforeground, dialogData->foreground,
		  NULL);
}

static void update_color_role_backgrounds(DialogData *dialogData)
{
#ifdef CALL_SEQ_DEBUG
    printf("update_color_role_backgrounds called\n");
#endif
    XtVaSetValues(dialogData->crole[START_COLOR],
		  XmNbackground, dialogData->work.scale->startPixel, NULL);
    XtVaSetValues(dialogData->crole[END_COLOR],
		  XmNbackground, dialogData->work.scale->endPixel, NULL); 
    XtVaSetValues(dialogData->crole[UNDER_COLOR],
		  XmNbackground, dialogData->work.scale->underflowPixel, NULL); 
    XtVaSetValues(dialogData->crole[OVER_COLOR],
		  XmNbackground, dialogData->work.scale->overflowPixel, NULL);
}

static void highlight_active_cell(DialogData *dialogData, int cell)
{
    char buf[32];

#ifdef CALL_SEQ_DEBUG
    printf("highlight_active_cell called \n");
#endif
    assert(cell >= 0 && cell < dialogData->work.scale->ncolors);
    sprintf(buf, "%d", cell);
    XmTextSetString(dialogData->cellentry, buf);
    XtVaSetValues(dialogData->cellentry,
		  XmNforeground, dialogData->foreground,
		  NULL);
    dialogData->active_cell = cell;
    if (!dialogData->setbutton_enabled)
    {
	XtVaSetValues(dialogData->setbutton,
		      XmNsensitive, True,
		      NULL);
	dialogData->setbutton_enabled = 1;
    }
    redraw_color_scale(dialogData);
}

static void unhighlight_active_cell(DialogData *dialogData)
{
    if (dialogData->active_cell >= 0)
    {
#ifdef CALL_SEQ_DEBUG
	printf("unhighlight_active_cell called \n");
#endif
	XmTextSetString(dialogData->cellentry, "");
	XtVaSetValues(dialogData->cellentry,
		      XmNforeground, dialogData->foreground,
		      NULL);
        dialogData->active_cell = -1;
	if (dialogData->color_role < 0)
	{
	    XtVaSetValues(dialogData->setbutton,
			  XmNsensitive, False,
			  NULL);
	    dialogData->setbutton_enabled = 0;
	}
	redraw_color_scale(dialogData);
    }
}

static void highlight_color_role(DialogData *dialogData, ColorRole crole)
{
    XmString str;
#ifdef CALL_SEQ_DEBUG
    printf("highlight_color_role called\n");
#endif
    assert(crole < N_COLOR_ROLES);
    str = XmStringCreateLocalized("*");
    XtVaSetValues(dialogData->activerole[crole],
		  XmNlabelString, str,
		  NULL);
    XmStringFree(str);
    dialogData->color_role = crole;
    if (!dialogData->setbutton_enabled)
    {
	XtVaSetValues(dialogData->setbutton,
		      XmNsensitive, True,
		      NULL);
	dialogData->setbutton_enabled = 1;
    }
}

static void unhighlight_color_role(DialogData *dialogData)
{
    XmString str;
    if (dialogData->color_role >= 0)
    {
#ifdef CALL_SEQ_DEBUG
	printf("unhighlight_color_role called\n");
#endif
	str = XmStringCreateLocalized(" ");
	XtVaSetValues(dialogData->activerole[dialogData->color_role],
		      XmNlabelString, str,
		      NULL);
	XmStringFree(str);
	dialogData->color_role = -1;
	if (dialogData->active_cell < 0)
	{
	    XtVaSetValues(dialogData->setbutton,
			  XmNsensitive, False,
			  NULL);
	    dialogData->setbutton_enabled = 0;
	}
    }
}

const char *colorname_from_role(DialogData *dialogData, ColorRole crole)
{
    char *clr = dialogData->ctmp;
    Pixel result;
    XcmsColor def;

#ifdef CALL_SEQ_DEBUG
    printf("colorname_from_role called\n");
#endif
    switch (crole)
    {
    case START_COLOR:
	result = dialogData->work.scale->startPixel;
	break;
    case END_COLOR:
	result = dialogData->work.scale->endPixel;
	break;
    case UNDER_COLOR:
	result = dialogData->work.scale->underflowPixel;
	break;
    case OVER_COLOR:
	result = dialogData->work.scale->overflowPixel;
	break;
    default:
	assert(0);
    }
    def.pixel = result;
    XcmsQueryColor(dialogData->work.scale->display,
		   dialogData->work.scale->colormap,
		   &def, XcmsRGBiFormat);
    printColorName(dialogData->work.scale->display,
		   dialogData->work.scale->colormap,
		   clr, dialogData->work.scale->colorspace,
		   def.spec.RGBi.red, def.spec.RGBi.green,
		   def.spec.RGBi.blue);
    return clr;
}

static int set_rgbscales_byname(DialogData *dialogData,
				const char *textcolorname)
{
    char buf[64];
    char *errmess;
    const char *colorname;
    XcmsColor exact_def, screen_def;
    Display *dpy  = XtDisplay(dialogData->dialog);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(dialogData->dialog));
    Pixel oldpixel;

#ifdef CALL_SEQ_DEBUG
    printf("set_rgbscales_byname called\n");
#endif
    colorname = normalizeColorName(textcolorname, buf);
    if (colorname == NULL)
	return 1;
    if (XcmsLookupColor(dpy, cmap, colorname, &screen_def,
			&exact_def, XcmsRGBiFormat) == XcmsFailure)
    {
	errmess = (char *)malloc(strlen(colorname)+20);
	if (errmess == NULL)
	{
	    fprintf(stderr, "Out of memory!\n");
	    return 1;
	}
	sprintf(errmess, "Bad color name \"%s\"", colorname);
	show_error_message(dialogData->dialog, errmess);
	free(errmess);
	return 1;
    }
    oldpixel = dialogData->choicecolor.pixel;
    dialogData->choicecolor = exact_def;
    dialogData->choicecolor.pixel = oldpixel;
    refresh_rgbscales(dialogData);
    refresh_colorwin(dialogData);
    return 0;
}

static void refresh_rgbscales(DialogData *dialogData)
{
    int newcolors[3], total = 0;
    PrimeColor i;

#ifdef CALL_SEQ_DEBUG
    printf("refresh_rgbscales called\n");
#endif
    newcolors[RED] = (int)(1.0e-10+COLOR_SLIDER_MAX*dialogData->choicecolor.spec.RGBi.red);
    newcolors[GREEN] = (int)(1.0e-10+COLOR_SLIDER_MAX*dialogData->choicecolor.spec.RGBi.green);
    newcolors[BLUE] = (int)(1.0e-10+COLOR_SLIDER_MAX*dialogData->choicecolor.spec.RGBi.blue);
    for (i = RED; i <= BLUE; ++i)
    {
	total += newcolors[i];
	XmScaleSetValue(dialogData->rgb_scales[i], newcolors[i]);
    }
    if (dialogData->ratios_locked)
	for (i = RED; i <= BLUE; ++i)
	{
	    if (total > 0)
		dialogData->locked_value[i] = newcolors[i];
	    else
		dialogData->locked_value[i] = 1;
	}
}

static void show_error_message(Widget parent, char *message)
{
    int n;
    Arg arg[5];
    XmString str;
    Widget win;

#ifdef CALL_SEQ_DEBUG
    printf("show_error_message called\n");
#endif
    str = XmStringCreateLocalized(message);
    n = 0;
    XtSetArg(arg[n], XmNmessageString, str); n++;
    XtSetArg(arg[n], XmNtitle, "Error"); n++;
    XtSetArg(arg[n], XmNdeleteResponse, XmDESTROY); n++;
    win = XmCreateErrorDialog(parent, "error_message", arg, n);
    XmStringFree(str);
    XtUnmanageChild(XtNameToWidget(win, "Cancel"));
    XtUnmanageChild(XtNameToWidget(win, "Help"));
    str = XmStringCreateLocalized("Acknowledged");
    XtVaSetValues(XtNameToWidget(win, "OK"),
		  XmNlabelString, str,
		  NULL);
    XmStringFree(str);
    XtManageChild(win);
}

static void generic_typing_cb(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
    Pixel pix, bluepix;
    DialogData *dialogData = (DialogData *)client_data;
    if (dialogData->typing_cb_disabled)
	return;
#ifdef CALL_SEQ_DEBUG
    printf("generic_typing_cb called\n");
#endif
    XtVaGetValues(w, XmNforeground, &pix, NULL);
    bluepix = dialogData->prime_colors[BLUE];
    if (pix != bluepix)
	XtVaSetValues(w, XmNforeground, bluepix, NULL);
}

static void cellnum_entry_cb(Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    long cellnum_new;
    char *eptr, buf[128];
    DialogData *dialogData = (DialogData *)client_data;
    char *data = XmTextGetString(dialogData->cellentry);

#ifdef CALL_SEQ_DEBUG
    printf("cellnum_entry_cb called\n");
#endif
    cellnum_new = strtol(data, &eptr, 0);
    if (cellnum_new < 0 || cellnum_new >= dialogData->work.scale->ncolors ||
	data[0] == '\0' || *eptr != '\0')
    {
	sprintf(buf, "Bad color cell number. Please enter "
		"an integer between 0 and %d.",
		dialogData->work.scale->ncolors - 1);
	show_error_message(dialogData->dialog, buf);
	return;
    }
    XtFree(data);
    if (!dialogData->setbutton_enabled)
    {
	XtVaSetValues(dialogData->setbutton, XmNsensitive, True, NULL);
	dialogData->setbutton_enabled = 1;
    }
    if (cellnum_new != dialogData->active_cell)
    {
	highlight_active_cell(dialogData, cellnum_new);
	set_rgbscales_bycell(dialogData, cellnum_new);
    }
}

static void ncolors_entry_cb(Widget w, XtPointer client_data,
			     XtPointer call_data)
{
    long n_colors_new;
    char *eptr, buf[128];
    DialogData *dialogData = (DialogData *)client_data;
    char *data = XmTextGetString(dialogData->ncolorsentry);

#ifdef CALL_SEQ_DEBUG
    printf("ncolors_entry_cb called\n"); fflush(stdout);
#endif
    n_colors_new = strtol(data, &eptr, 0);
    if (n_colors_new < 2 || n_colors_new > maxColorScaleColors() ||
	data[0] == '\0' || *eptr != '\0')
    {
	sprintf(buf, "Bad number of colors. Please enter "
		"an integer between 2 and %d.",
		maxColorScaleColors());
	show_error_message(dialogData->dialog, buf);
	return;
    }
    XtFree(data);
    XtVaSetValues(dialogData->ncolorsentry,
 		  XmNforeground, dialogData->foreground,
 		  NULL);
    if (n_colors_new != dialogData->work.scale->ncolors)
    {
	unhighlight_active_cell(dialogData);
 	setNumColors(dialogData->work.scale, n_colors_new);
	redraw_color_scale(dialogData);
    }
}

static void set_chooser_colorname(DialogData *dialogData,
				  const char *name)
{
#ifdef CALL_SEQ_DEBUG
    printf("set_chooser_colorname called\n");
#endif
    typing_cb_enable(dialogData, 0);
    XmTextSetString(dialogData->colorentry, (char *)name);
    XtVaSetValues(dialogData->colorentry,
		  XmNforeground, dialogData->foreground,
		  NULL);
    typing_cb_enable(dialogData, 1);
}

static void typing_cb_enable(DialogData *dialogData, int enable)
{
    dialogData->typing_cb_disabled = !enable;
}

static void get_data_range_cb(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    LimitsCbResult limits;
    char buf[32];

#ifdef CALL_SEQ_DEBUG
    printf("get_data_range_cb called\n");
#endif
    assert(dialogData->orig.limits_cb);
    dialogData->orig.limits_cb(XtParent(dialogData->dialog),
			       dialogData->orig.limits_data,
			       &limits);
    dialogData->work.colorMin = limits.min;
    dialogData->work.colorMax = limits.max;
    sprintf(buf, "%g", limits.min);
    XmTextSetString(dialogData->minentry, buf);
    XtVaSetValues(dialogData->minentry,
		  XmNforeground, dialogData->foreground,
		  NULL);
    sprintf(buf, "%g", limits.max);
    XmTextSetString(dialogData->maxentry, buf);
    XtVaSetValues(dialogData->maxentry,
		  XmNforeground, dialogData->foreground,
		  NULL);
    redraw_data_axis(dialogData);
}

static void set_limit_cb(Widget w, XtPointer client_data,
			 XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    double dvalue;
    char *text;
    char *endptr;

#ifdef CALL_SEQ_DEBUG
    printf("set_limit_cb called\n");
#endif
    text = XmTextGetString(w);
    if (text[0] == '\0')
    {
	XtFree(text);
	return;
    }
    dvalue = strtod(text, &endptr);
    if (endptr != text)
	while (*endptr && isspace(*endptr))
	    ++endptr;
    if (*endptr != '\0' || endptr == text)
    {
	show_error_message(dialogData->dialog,
			   "Please enter a floating point number");
	XtFree(text);
	return;
    }
    XtFree(text);
    if (w == dialogData->minentry)
	dialogData->work.colorMin = dvalue;
    else if (w == dialogData->maxentry)
	dialogData->work.colorMax = dvalue;
    else
	assert(0);
    XtVaSetValues(w, XmNforeground, dialogData->foreground, NULL);
    redraw_data_axis(dialogData);
}

static unsigned get_widget_width(Widget w)
{
    Dimension x;
    XtVaGetValues(w, XmNwidth, &x, NULL);
    return x;
}

static unsigned get_widget_height(Widget w)
{
    Dimension x;
    XtVaGetValues(w, XmNheight, &x, NULL);
    return x;
}

static void lock_ratio_cb(Widget w, XtPointer client_data,
			  XtPointer call_data)
{
    DialogData *dialogData = (DialogData *)client_data;
    XmToggleButtonCallbackStruct *str = (XmToggleButtonCallbackStruct *)call_data;
    PrimeColor c;
    int total;

#ifdef CALL_SEQ_DEBUG
    printf("lock_ratio_cb called\n");
#endif
    if (str->reason == XmCR_VALUE_CHANGED)
    {
	dialogData->ratios_locked = str->set;
	if (str->set)
	{
	    total = 0;
	    for (c = RED; c <= BLUE; ++c)
	    {
		XmScaleGetValue(dialogData->rgb_scales[c], &dialogData->locked_value[c]);
		total += dialogData->locked_value[c];
	    }
	    if (total == 0)
		for (c = RED; c <= BLUE; ++c)
		    dialogData->locked_value[c] = 1;
	}
    }
}

static XFontStruct *getFontStruct(XmFontList font)
{
    XFontStruct *fs;
    XmFontContext context;
    XmStringCharSet charset;

    XmFontListInitFontContext(&context, font);
    XmFontListGetNextFont(context, &charset, &fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
    return fs;
}
