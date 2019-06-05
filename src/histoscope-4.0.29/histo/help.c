/*******************************************************************************
*									       *
* help.c -- help text for Histo-Scope tool				       *
*									       *
* Copyright (c) 1992, 1993 Universities Research Association, Inc.	       *
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
* Fermilab Nirvana GUI Library						       *
* December 28, 1992							       *
*									       *
* Written by Joy Kyriakopulos and Mark Edel				       *
*									       *
*******************************************************************************/
#include <Xm/Xm.h>
#include "../util/help.h"
#include "2DHelp.xbm"
#include "2DHistHelp.xbm"
#include "3DHelp.xbm"
#include "help.h"
#include "preferences.h"

static void helpButtonDialog(Widget parent, helpMenuInfo *help);

/*
** Help item titles, menu information, and text
*/
#define STATS "\nTo get statistics on the data for the plot, select Show \
Statistics from the Plot Settings menu.  This will pop-up a window that \
displays the following information:\n\n\
    o  the Name of the data item\n\
    o  the Number of Entries (ntuple) or Fills (histogram)\n\
    o  for histograms only: the Sum of all the Fills\n\
         (= the number of entries if weight was always 1)\n\
    o  for each variable in the plot:\n\
         - Mean\n\
         - Standard Deviation\n\
         - Minimum\n\
         - Maximum\n\
    o  for histograms: the Overflow Sums\n\n\
To dismiss the statistics window, select Show Statistics once again from \
the Plot Settings menu."

#define SLIDE "\nSlider variables can be defined for any plot \
from NTUPLE data except time series and XY plots.  To use animation \
sliders, \
assign the variables to be used in the Ntuple Window as S1, S2, etc.\n\n\
If any ntuple variables are assigned to sliders, selecting \"Show Sliders\" \
in the plot menu will display a window containing interactive controls \
associated with each variable.  The controls \
eliminate points (for scatter plots) or weighted values (for histograms) \
associated in the ntuple with the values of the slider variable less than \
the minimum (<) or greater than the maximum (>) value set on the slider.  \
For example, \
when you press Plot, the plot will be displayed with each slider variable \
scale set to All.  This means that all points are displayed.  Select \"Show \
Sliders\" from the Plot Settings menu to display the Animation Sliders window.\
  As you move \
an Animation Slider scale to the left, a numeric value is displayed.  This \
means that all ntuple entries with the slider variable greater than this value \
are eliminated from the plot.  As you move the animation slider, you can see \
the plot change accordingly:  Scatter Plots lose/add points and histograms \
lose/add height.\n\n\
Use the tab key to select the slider you want to change in a window with \
multiple sliders in it.  \n\n\
In addition to dragging the slider with the mouse, you can click on the space \
to the left or right of the slider to move it in pre-defined increments.  \
Each slider can also be adjusted for smaller increments by using the left \
and right arrow keys."

static helpMenuInfo

zoom = {"Scaling the Plot", 'S', False, HELP_TEXT,
"\nThere are three different ways to scale a plot after it is displayed:\n\n\
    1)  using the mouse to grab the axis ticks of the plot\n\
    2)  using the Axis Settings window\n\
    3)  using Zoom In, Zoom Out, or Reset View functions\n\
         from the Plot Settings menu\n\n\
1) Using the mouse to grab the axis ticks of the plot:  point the mouse just \
to the \
outside of an axis, press the leftmost button, and while holding it down move \
the mouse up or down the axis.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
  Once you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)\n\n\
2) Using the Axis Settings window:  Selecting Axis Settings from the Plot \
Settings menu will pop up the Axis Settings window.  This window allows you \
to enter values for the minimum and maximum value of each axis.  Press OK \
or Apply to see the new settings in the plot.  \
In addition, you can change the scaling of an \
axis from linear to logarithmic (and back) using the diamond-shaped radio \
buttons. \n\n\
3)  Using Zoom In, Zoom Out, or Reset View functions from the Plot Settings \
menu:  Selecting Zoom In or Zoom Out will immediately change the plot in the \
direction you specified.  They can be used repetitively \
to approximate a desired scaled view of the plot.  For finer tuning, use one \
of the other methods described above.  Reset View is used to scale out the \
plot to display the entire range of the data.  For 2-dimensional histograms, \
Reset View also resets the orientation of the plot to its initial state.", 0, 0},

/*        1         2         3         4         5         6 */
/* 3456789012345678901234567890123456789012345678901234567890 */

showStats = {"Getting Statistics on the Data", 'G', True, HELP_TEXT,
STATS, 0, 0},

showStatsS = {"Getting Statistics on the Data", 'G', False, HELP_TEXT,
STATS, 0, 0},

showSlid = {"Adjusting the Plot with Slider Variables", 'A', True, HELP_TEXT,
SLIDE, 0, 0},

showSlidH = {"Adjusting the Plot with Slider Variables", 'A', False, HELP_TEXT,
SLIDE, 0, 0},

rebin = {"Rebinning the Data", 'R', True, HELP_TEXT,
"\nTo re-bin histograms displayed from ntuple data, or to find out the \
number of bins \
Histo-Scope used to display the ntuple, select Show Rebin Sliders from the \
Plot Settings menu.\n\n\
To adjust the slider to the exact number of bins you desire \
use the left and right arrow keys.  (Use \
the tab key to select the slider you want to change for two-dimensional \
histograms if the slider you want to change isn't selected.)\n\n\
You can also drag the slider with the mouse or click on the space \
to the left or right of the slider.", 0, 0},

histStyle = {"Setting Histogram Styles and Color", 't', True, HELP_TEXT,
"\nHistograms are by default outlined in black and\n\
contain no fill pattern.\n\
\n\
After a histogram plot is created, the following\n\
attributes can be set to alter the style of the\n\
histogram:\n\
\n\
      o Line Style (also used to set markers)\n\
      o Line (or marker) Color\n\
      o Fill Style\n\
      o Fill Color\n\
\n\
When markers are selected as the line style, only\n\
markers are displayed (without connecting lines or\n\
fill pattern).\n", 0, 0},

normSlid = {"Adjusting Cell Plot with Normalization Slider", 'R', True, 
HELP_TEXT,
"\nThe Normalization Slider is used to alter the cell size and density \
in a Cell Plot.  The size of a rectangle in a cell plot is proportional \
to the contents of the bin it represents.  The maximum cell size (filled \
in black) represents a value greater than or equal to the right (maximum) \
value on the normalization slider window.  An empty cell represents \
a bin value less than or equal to the left (minimum) value on the \
normalization slider window.  \n\
\n\
Initially, the normalization slider shows the entire range \
of the bin data.  The number displayed in the right side of the slider \
instructs Histo-Scope to display every bin value from that value up to the \
maximum bin \
value at the maximum cell size in the plot.  (Correspondingly, the number \
displayed in the left side of the slider instructs Histo-Scope to eliminate \
displaying every bin value from that left value to the minimum bin value in \
the plot.)  The \
data between the two sliders is displayed with cell size proportional to \
the bin contents within the range of the normalization minimum and maximum.\n\
\n\
To adjust the slider, drag the arrow slide you want to change with the mouse \
in the direction you desire.\n\
", 0, 0},

otherV = {"Other Views", 't', True, HELP_TEXT,
"The Other Views plot setting is available from 2D histogram \
and cell plots from two-dimensional histograms.  When Other Views is \
selected from a 2D histogram plot, a 2D cell plot is popped up.  When Other \
Views is selected from a 2D Cell Plot, a 2D Histogram plot is popped up.\n\
\n\
A 2D cell plot is a flat representation of the 2D histogram with bin \
contents represented by the size of a shaded rectangle drawn on the \
center of the bin.  The normalization slider is used to adjust the size \
and density of the cells.\n\
", 0, 0},

binLimit = {"Changing Bin Limits", 'm', True, HELP_TEXT,
"\nTo set the bin splitting threshold for adaptive histograms displayed \
from ntuple data, or to find out its current value, \
select Show Bin Limit Sliders from the \
Plot Settings menu.\n\n\
To adjust the slider to a new threshold value, \
use the left and right arrow keys.  (Use \
the tab key to select the slider you want to change for two-dimensional \
histograms if the slider you want to change isn't selected.)\n\n\
You can also drag the slider with the mouse or click on the space \
to the left or right of the slider.", 0, 0},

showBackP = {"Show/Hide Backplanes", 'B', True, HELP_TEXT,
"\nBy default, the backplanes of 2D Histograms are displayed.\n\n\
To make the backplanes disappear, select Show Backplanes from the Plot \
Settings menu.  To make them reappear, select Show Backplanes once again.", 0, 0},

multShowTit = {"Show/Hide Titles", 'T', True, HELP_TEXT,
"\nBy default, the titles of mini-plots in a Multiple Plot Window are \
displayed.\n\n\
To make the titles disappear, select Show Titles from the Plot \
Settings menu.  To make them reappear, select Show Titles once again.", 0, 0},

markLine = {"Setting Marker & Line Styles", 'M', False, HELP_TEXT,
"\nTime Series and XY plots allow you to place markers on the data points of \
the plot, and to set line styles for each variable.\n\
\n\
To set the mark or line style, select Set Mark & Line Style... from the Plot \
Settings menu.  A window will pop up with three columns of option menus, \
an example line segment and/or marker, and the name of the variable.  \
The option menus set the line style, marker style, and marker size for \
the variable corresponding to that row (the variable name is listed at the \
far right).\n\
\n\
To change the line style or assign a marker type to a variable, select the \
new marker or line style from the menus in the row with the variable name \
at the right.  \
As you make assignments, the marker and line styles will change in the sample \
area to the left of the variable name.  Click Apply or OK to make the changes \
to the plot (The OK button will dismiss the dialog as well as making the \
changes).", 0, 0},

showLeg = {"Show/Hide Variables Legend", 'L', True, HELP_TEXT,
"\nBy default, the Variables Legend is displayed.\n\n\
To make the the legend disappear, select Show Variables Legend from the Plot \
Settings menu.  To make it reappear, select Show Variables Legend once again.",
0, 0},

thickPts = {"Thicken/Lighten Scatter Points", 'T', True, HELP_TEXT,
"\nBy default, the points in a scatter plot are drawn as thick points.\n\n\
To make the the plot lighter, select Thicken Points from the Plot \
Settings menu to lighten the points.  To make the points thick again, select \
Thicken Points once again.", 0, 0},

pan = {"Panning the Plot", 'n', False, HELP_TEXT,
"\nOnce you have scaled the plot (so that part of the plot is out of view), it \
is possible to grab a point inside of the plot \
and move it around inside the axes.", 0, 0},

pan2D = {"Panning the Plot", 'n', False, HELP_TEXT,
"\nOnce you have scaled the plot (so that part of the plot is out of view), it \
is possible to grab a point just inside an axis \
and pan the plot along that axis.", 0, 0},

loG = {"Log Scaling", 'o', True, HELP_TEXT,
"\nSelecting Axis Settings from the Plot \
Settings menu will pop up the Axis Settings window.  This window allows you \
to change the scaling of an \
axis from linear to logarithmic (and back) using the diamond-shaped radio \
buttons.", 0, 0},

updOpt = {"Update Options", 'U', True, HELP_TEXT,
"\n\"Automatic Update\", \"Update\", and \"Grow Only\" are useful only \
when connected to a running process.  These three settings affect \
how the data is re-displayed as it changes in the client program.  (The \
main window's \
Preferences menu is used to set the Update Frequency, which affects how \
often data is requested from the running process.)\n\n\
Initially Automatic Update mode is on.  This is indicated by the presence of \
a square \
toggle button to the left of the words \"Automatic Update\".  Selecting the \
Automatic Update item in the plot settings menu turns on or off \
Automatic Update mode.\n\n\
When Histo-Scope is connected to a running process, each displayed plot with \
Automatic Update mode on will be updated according to the frequency set in \
the Preferences Menu in the main window (initially every 3 seconds).\n\n\
With Automatic Update off, a plot can still be individually updated \
with data Histo-Scope has received from the client by selecting \"Update\" \
from the plot settings menu.\n\n\
When the new data arrives, the plot may be re-scaled to include the new \
data.  Setting Grow Only \
mode will prevent Histo-Scope from reducing the scale of the plot if \
the new data covers less area than the old data.  Grow Only mode is useful \
for maintaing comparable scales between several plots, particularly in \
conjunction with using the Axis Settings dialog to set the plot limits \
beyond the limits of the data.  When using sliders, the plot is temporarily \
set in grow-only mode while the sliders are affecting the display of \
the data.", 0, 0},

printP = {"Printing the Plot", 'P', False, HELP_TEXT,
"\nSelect Print... to print the plot to a PostScript printer.\n\nHisto-Scope \
will pop up a Printer Options window for specifying the printer queue name \
and the number of copies to print.\n\
\n\
The last field in this dialog is the Unix (or VMS) command that will be \
used to queue the plot for printing.  Since this command is generated from \
the other options in the dialog, there is normally no reason to change it, \
but you can use the field to add options and change the command \
in ways not otherwise supported by the print dialog. \
\n\n\
If the statistics window for a plot is displayed while printing a plot, \
the statistics will also be printed to the side of the plot.\
\n\n\
Please note that 2D Histogram plots take some time to print because of \
PostScript processing time in the printer.  As the number of bins goes up, \
print processing time increases so that histograms of 100 x 100 bins can \
take over nine minutes for the printer to process and print the file \
(depending on the speed of the printer).", 0, 0},

printPMult = {"Printing a Multi-Plot", 'P', False, HELP_TEXT,
"\nSelect Print... to print all of the plots in a Multi-Plot Window to a \
PostScript printer.  All plots will appear on a single page similar in \
appearance to how they appear on the screen.  If titles are displayed in the \
Multi-Plot Window, they will also be printed.\n\nHisto-Scope \
will pop up a Printer Options window for specifying the printer queue name \
and the number of copies to print.\n\
\n\
The last field in this dialog is the Unix (or VMS) command that will be \
used to queue the plot for printing.  Since this command is generated from \
the other options in the dialog, there is normally no reason to change it, \
but you can use the field to add options and change the command \
in ways not otherwise supported by the print dialog. \
\n\n\
Please note that 2D Histogram plots take some time to print because of \
PostScript processing time in the printer.  As the number of bins goes up, \
print processing time increases so that histograms of 100 x 100 bins can \
take over nine minutes for the printer to process and print the file \
(depending on the speed of the printer).", 0, 0},

genPOST = {"Generating a PostScript file of the Plot", 'f', True, HELP_TEXT,
"\nSelect Generate PostScript... if you would like to save a PostScript \
representation of your plot to a file.\n\n\
Histo-Scope will pop-up a file selection dialog for you to enter a \
filename for the PostScript output.\n\n\
If the statistics window for a plot is displayed while generating a \
PostScript file of a plot, \
the statistics will also be output to the side of the plot in the file.\
", 0, 0},

genPOSTMult = {"Generating a PostScript file of the Plot", 'f', True, HELP_TEXT,
"\nSelect Generate PostScript... if you would like to save a PostScript \
representation of all the plots in your multiple-plot window to a file.\n\n\
Histo-Scope will pop-up a file selection dialog for you to enter a \
filename for the PostScript output.", 0, 0},

gettingStarted = {"Getting Started", 'G', True,  HELP_TEXT,"\
Histo-Scope is a tool to select and display histograms, n-tuples, and scalar \
values from a program as data is being created or from HBOOK or HistoScope \
files.\n\
\n\
After invoking Histo-Scope, you'll want to do one of three \
things:\n\n\
     Connect to a running process\n\
     Open a Histoscope file\n\
     Open an HBOOK file\n\n\
Use the File/Process pull-down menu to perform any of these functions.  \n\n\
(--> Use scroll bar at right to see rest of this message.)\n\n\
After specifying how Histo-Scope should access your data, you need to \
specify the data items to view.  You can use the Category \
pull-down menu and the Sub Categories list box to navigate through a \
hierarchy of data items.\n\n\
Choose the data items \
to display from the Histogram/Ntuple/Indicator/Control list box by clicking on \
an item and pressing the View button.  \
Each plot allows you to adjust its plot settings.  To bring up \
a Plot Settings menu, point to the plot with the mouse and press the right \
mouse button.\n\n\
The Preferences pull-down menu allows you to specify how often Histo-Scope \
should request new data from a connected process, to set/unset buffer \
graphics mode, and to set/unset automatic plot help.  These preferences \
can be saved for future Histo-Scope sessions.", 0, 0},

/*        1         2         3         4         5         6 */
/* 3456789012345678901234567890123456789012345678901234567890 */

preferences = {"Preferences", 'P', False, HELP_TEXT,
"\nThe preferences pull-down menu allows you set/unset various options \
affecting the Histoscope session.  The options can also be saved in \
a file which is automatically be re-loaded every time Histoscope is started. \
Selecting \"Save Preferences\" generates this file in your home directory \
(Unix) or SYS$LOGIN (VMS) with the name .histo.\n\n\
Update Frequency:\n\
     If you select Update Frequency, a window will pop-up \n\
     which allows you to use a slider bar to set the update\n\
     frequency.  This update frequency is used only when you\n\
     are connected to a running process.  By default, the \n\
     update frequency is set to 3 seconds.  That means that\n\
     every three seconds, Histo-Scope will request any new \n\
     data from the running process and update displayed\n\
     plots (unless update mode for that plot is set to off).\n\n\
Buffer Graphics:\n\
     Graphics buffering changes the way Histo-Scope draws\n\
     moving graphics in X-Windows.  Depending on your X-Win-\n\
     dows server or X-Terminal, turning on buffered graphics\n\
     may either improve or degrade animation quality - it\n\
     eliminates flicker at the expense of drawing speed and\n\
     X-server memory.  Initially Buffer Graphics mode is off.\n\n\
Automatic Plot Help\n\
     Automatic plot help pops up a help window explaining\n\
     Histo-Scope's interactive controls for manipulating plots\n\
     the first time a plot of a particular type is displayed.\n\
     Most users will want to turn this item off after a few\n\
     sessions.  To view the automatic help panels without\n\
     automatic help turned on, select \"Interactive Controls\"\n\
     from the help sub-menu of the plot's menu. (to access the\n\
     menu for a plot window, press the right mouse button\n\
     anywhere in the window)", 0, 0},

nplotPref = {"Preferences", 'P', False, HELP_TEXT,
"\nThe preferences pull-down menu allows you set/unset various options \
affecting the Histoscope session.  The options can also be saved in \
a file which is automatically be re-loaded every time Histoscope is started. \
Selecting \"Save Preferences\" generates this file in your home directory \
(Unix) or SYS$LOGIN (VMS) with the name .nplot.\n\n\
Buffer Graphics:\n\
     Graphics buffering changes the way Histo-Scope draws\n\
     moving graphics in X-Windows.  Depending on your X-Win-\n\
     dows server or X-Terminal, turning on buffered graphics\n\
     may either improve or degrade animation quality - it\n\
     eliminates flicker at the expense of drawing speed and\n\
     X-server memory.  Initially Buffer Graphics mode is off.\n\n\
Automatic Plot Help\n\
     Automatic plot help pops up a help window explaining\n\
     Histo-Scope's interactive controls for manipulating plots\n\
     the first time a plot of a particular type is displayed.\n\
     Most users will want to turn this item off after a few\n\
     sessions.  To view the automatic help panels without\n\
     automatic help turned on, select \"Interactive Controls\"\n\
     from the help sub-menu of the plot's menu. (to access the\n\
     menu for a plot window, press the right mouse button\n\
     anywhere in the window)", 0, 0},

fileProcess = {"File/Process", 'F', False, HELP_TEXT,
"\nThe File/Process menu is used to tell Histo-Scope how to access your \
data and for exitting the program.  You can:\n\n\
     Connect to a running process\n\
     Open an HBOOK file\n\
     Open a Histoscope format file\n\
     Re-read Same File\n\
     Close File/Connection\n\
     Read a configuration file\n\
     Save a configuration file\n\
     Exit\n\n\
After opening a file or connecting to a process, data items that can be \
viewed are listed.  (If you have organized your data items in a hierarchical \
directory or category structure, the top-level category will be identified \
and only items in that top-level category will be initially listed.) \n\
\n\
Only one process connection or one file can be open at a time. \
If you'd like to open another file or connect to another process, Histo-Scope \
will ask you if you'd like to close the current file or connection before it \
will open the new one.  Even after a process connection or file is closed, all \
plot windows continue to be displayed until you explicitly close them \
(individually via the plot's Plot Settings Menu or collectively by selecting \
Close All Windows from the Windows menu).  All plot windows are closed when \
you exit Histo-Scope.\n\
\n\
If you currently have a file open, you can select Re-Read Same File to update \
Histo-Scope with new data that may have been written to the file since \
Histo-Scope opened it.  To ensure that all your plot windows reflect new \
data, Histo-Scope will close all plot windows before re-reading the file.\n\
\n\
Configuration files are files of Histo-Scope commands for opening windows, \
scaling plots, adjusting plot appearance, setting slider positions, etc..  \
Save Configuration... saves the current appearance of your Histo-Scope \
session (excluding opening the file or making the connection).  Load \
Configuration... re-loads these files.  More than one configuration file \
can be loaded in a single session.  Loading additional files simply displays \
the additional plots specified in the new files.  Note that configuration \
files do not specify the file or connection to be opened, nor do they \
specify Preferences settings.  Configuration files are an editable \
but undocumented ascii text file format.", 0, 0},

nplotFile = {"File Menu", 'F', False, HELP_TEXT,
"\nThe File menu is used to open and close files and for exitting NPlot. \n\n\
After opening a file the variables that you have defined in your data file \
are listed and you can choose variables to plot.", 0, 0},

windows = {"Windows", 'W', True, HELP_TEXT,
"\nThe Window menu lists all of the currently displayed plot windows.  \
Selecting one will bring that plot to the front of all other windows.  \
In addition to the list of windows, the Windows menu also contains the \
item \"Close All Windows\", which closes all of the open plot windows without \
closing the currently open file or connection to an attached process.\n\n\
To create a plot window containing more than one plot, select Multiple Plot \
Window....  This will pop up a dialog, The Create Multiple Plot Window, \
which allows you to specify to Histo-Scope the parameters for \
creating a plot window with more than one plot.  There are two ways \
these multiple-plot windows can be populated with plots:\n\n\
1)  All the 1-D and 2-D Histograms in the current category (as listed \
in the Histo-Scope Main Window) can be displayed in a single multiple \
plot. \n\n\
2) For more flexibility, don\'t press the \"Use Current Category\" button.  \
Instead just type the window title and number of rows \
and columns for the Multiple Plot window, and after pressing \"Create Window\" \
use the mouse to drag the plots one-by-one into the initially empty \
multi-plot window.  \
This method gives you the most flexibility for choosing and placing plots \
for display. \n\n\
To change the size of the individual plots in Multiple Plot windows, use \
Motif window controls such as the window sides or corners to make the entire \
window larger or smaller.\n\n\
Each mini-plot in the Multiple Plot window allows you to adjust its plot \
settings in a manner similar to the individual plot windows.  To bring up a \
Plot Settings menu, point to the plot with the mouse and press the right \
mouse button.\n\
", 0, 0},

nplotWindows = {"Windows", 'W', True, HELP_TEXT,
"\nThe Window menu lists all of the currently displayed plot windows.  \
Selecting one will bring that plot to the front of all other windows.  \
In addition to the list of windows, the Windows menu also contains the \
item \"Close All Plot Windows\", which closes all of the open plot \
windows without closing the currently open file.", 0, 0},

categoryList = {"Category List - navigation" , 'C', False, HELP_TEXT,
"\nHisto-Scope supports the notion of categories, equivalent to HBOOK's \
directories, for arranging large numbers of histograms and n-tuples in \
a hierarchy rather than in a single long list.  If the connected process \
or open file has \
its data items organized using categories, you will initially see the \
top-level \
category displayed on the pull-down menu and all of its sub-categories listed \
below it.  All data items in the current category are listed in the \
Histogram/Ntuple/Indicator/Control list.\n\n\
To list data items from another category, use the mouse to select the \
category and then \
click on \
the Open button.  Double-clicking on a (sub)category will also open it.\n\n\
To traverse back up the hierarchy, select and open the item \
\"<-- Up One Level\" \
or use the Category pull-down menu to select the current category's parent.",
0, 0},

histNtIndList = {"Histogram/Ntuple/Indicator/Control List", 'H', True, HELP_TEXT,
"\nAfter using the File/Process menu to connect to a process or open a file, \
data items in the top-level category will be listed in the \
Histogram/Ntuple/Indicator/Control list.\n\n\
To display data items from another category, select that category in the Sub \
Category list.\n\
\n\
To plot a data item from the Histogram/Ntuple/Indicator/Control list \
click on the item with \
the mouse and then click the View button.  Double-clicking on an item \
in this \
list also plots that item.\n\n\
", 0, 0},

openP = {"Open - a category", 'O', False, HELP_TEXT,
"\nTo select a Sub Category, and display all data items in that category, \
select the category with the mouse and then \
click on \
the Open button, or just double-click on the category.", 0, 0},

viewP = {"View - plotting your data", 'V', True, HELP_TEXT,
"\nFirst, use the File/Process menu to connect to a process or open a file. \
Data items that can be viewed will be listed in the Histogram/Ntuple/\
Indicator/Control list.  \n\n\
If necessary, select and open the category for the data item you want to plot \
so that it will be listed in the Histogram/Ntuple/Indicator/Control list.\n\n\
To view a data item, select the item from the Histogram/Ntuple/Indicator/Control \
list by clicking on it with \
the mouse and then click on the View button.  Double-clicking on an item also \
plots that item.\n\n\
One- and two-dimensional histogram plots will immediately pop up.  If you \
select an n-tuple, Histo-Scope displays an Ntuple Window from which you \
can specify the variables to plot and the type of plot you desire. \n\n\
Multiple items can be selected at one time for \
plotting.  (See \"Selecting Plots to View.\")  If the  \
items selected are all 1D Histograms or groups,  you \
will notice that two additional View buttons will become \
dark enough to read and sensitive to mouse-clicks:  the \
\"View Overlaid\"  button and the \"View Multiple\" button.\
\n\n\
The \"View Overlaid\" button will display the selected \
plots overlaid on each other.  Only 1D Histograms, or \
groups containing 1D Histograms, selected from the Main \
Window can be overlaid.  (Other plot types displayed \
from Ntuples, such as Time-Series, XY, and Adaptive \
Histograms can also be overlayed with each other or with \
1D Histograms, but must be dragged into the other plot \
window with the second mouse button.)\
\n\n\
The \"View Multiple\" button will display all the selected \
1D or 2D Histograms from the Main Window into mini-plots \
of a Multiple Plot Window.  (Empty mini-plots in a \
Multiple Plot Window can receive other types of plots to \
display by using the 2nd mouse button to drag the plot \
into the empty mini-plot.)  In addition to the \"View \
Multiple\" button, Multiple Plot Windows can be created \
via the Windows Menu. \
\n\n\
When a group is selected and the \"View\" button is pressed, \
the items in the group are displayed according to the \
group type specified by the user program that created \
the group (see HS_CREATE_GROUP in the Histo-Scope User's  \
Manual).  Thus plots will be displayed individually, in \
a multiple-plot window, or in an overlaid plot window, \
according to this default group type.  The \"View \
Multiple\" and \"View Overlaid\" buttons override the \
default behavior.\
\n\n\
Each plot window allows you to adjust its plot settings.  To bring up \
a Plot Settings menu, point to the plot with the mouse and press the right \
mouse button.\n\n\
To display another data item, go back to the main window.  You may need to \
move the plot window out of the way to do this.  Using the mouse, point \
to the plot window title and press the select button while dragging the \
window out of the way.\n\n\
Many plots can be displayed at one time.  Plots will be displayed until \
explicitly closed, individually using the Window Menu button at the top left \
of the plot window or the Plot Settings menu, or collectively via the Windows \
menu on the Main Histo-Scope Window.", 0, 0},

capViewP = {"View - plotting your data", 'V', True, HELP_TEXT,
"\n\
Data items that can be viewed will be listed in the Histogram/Ntuple/\
Indicator/Control list.  \n\n\
If necessary, select the category for the data item you want to plot.\n\n\
To view a data item, select the item from the Histogram/Ntuple/Indicator/Control \
list by clicking on it with \
the mouse and then click on the View button.  Double-clicking on an item also \
selects that item for viewing.\n\n\
One- and two-dimensional histogram plots will immediately pop up.  If you \
select an n-tuple, Histo-Scope displays an Ntuple Window from which you \
can specify the variables to plot and the type of plot you desire. \n\n\
To display another data item, go back to the main window.  You may need to \
move the plot window out of the way to do this.  Using the mouse, point \
to the plot window title and press the select button while dragging the \
window out of the way.\n\n\
Multiple items can be selected at one time for \
plotting.  (See \"Selecting Plots to View.\")  If the  \
items selected are all 1D Histograms or groups,  you \
will notice that two additional View buttons will become \
dark enough to read and sensitive to mouse-clicks:  the \
\"View Overlaid\"  button and the \"View Multiple\" button.\
\n\n\
The \"View Overlaid\" button will display the selected \
plots overlaid on each other.  Only 1D Histograms, or \
groups containing 1D Histograms, selected from the Main \
Window can be overlaid.  (Other plot types displayed \
from Ntuples, such as Time-Series, XY, and Adaptive \
Histograms can also be overlayed with each other or with \
1D Histograms, but must be dragged into the other plot \
window with the second mouse button.)\
\n\n\
The \"View Multiple\" button will display all the selected \
1D or 2D Histograms from the Main Window into mini-plots \
of a Multiple Plot Window.  (Empty mini-plots in a \
Multiple Plot Window can receive other types of plots to \
display by using the 2nd mouse button to drag the plot \
into the empty mini-plot.)  In addition to the \"View \
Multiple\" button, Multiple Plot Windows can be created \
via the Windows Menu. \
\n\n\
When a group is selected and the \"View\" button is pressed, \
the items in the group are displayed according to the \
group type specified by the user program that created \
the group (see HS_CREATE_GROUP in the Histo-Scope User's  \
Manual).  Thus plots will be displayed individually, in \
a multiple-plot window, or in an overlaid plot window, \
according to this default group type.  The \"View \
Multiple\" and \"View Overlaid\" buttons override the \
default behavior.\
\n\n\
Many plots can be displayed at one time.  Plots will be displayed until \
explicitly closed using the Window Menu Button at the top left of the \
plot window or the Plot Settings menu.\n\n\
Each plot window allows for adjusting its plot settings.  To bring up \
a Plot Settings menu, point to the plot with the mouse and press the right \
mouse button.", 0, 0},

selectP = {"Selecting Plots to View", 'l', False, HELP_TEXT,
"\n\
After opening a file or connecting to a process, \
Histo-Scope will list the data items belonging to the \
top-level category.  If necessary, select and open the \
category for the data items you want to plot so they \
will be shown in the Histogram/Ntuple/Indicator/Control \
list. \
\n\n\
If you want to plot only one item, simply point to it \
with the mouse and either click on it once and then \
press the View button, or just double-click on the item \
in the list box.\
\n\n\
If you want to plot more than one item (for overlaying \
plots,  viewing multiple plots in a single window, or \
just conveniently bringing up several plots at \
once), there are a few different ways to do this:\
\n\n\
To select contiguous items in the list, you can press \
the left mouse button and drag the mouse over the items \
you wish to select, then release the button.  A second \
way to select contiguous items is to point and click on \
the first item, then move the mouse to the last item you \
wish to select, and point and click on that item while \
holding down the SHIFT key.\
\n\n\
To select non-contiguous items, hold the CTRL key down \
while selecting the new item(s) with the mouse.  \
Whenever the CTRL key is held while clicking with the \
mouse, previously selected items will remain selected \
along with the newly selected item(s). \
\n\n\
Selecting a group causes all of the data items in the \
group to be selected. \
\n\n\
Once you have selected the items you wish to plot, \
select one of the View buttons (View, View Multiple, or \
View Overlaid) at the bottom of the Main Window (see \
the help section titled \"View - plotting your data\").\n\
", 0, 0},

capSelectP = {"Selecting Plots to View", 'l', False, HELP_TEXT,
"\n\
Histo-Scope initially lists the data items belonging to the \
top-level category.  If necessary, select and open the \
category for the data items you want to plot so they \
will be shown in the Histogram/Ntuple/Indicator/Control \
list. \
\n\n\
If you want to plot only one item , simply point to it \
with the mouse and either click on it once and then \
press the View button, or just double-click on the item \
in the list box.\
\n\n\
If you want to plot more than one item (for overlaying \
plots,  viewing multiple plots in a single window, or \
just conveniently bringing up several plots at \
once), there are a few different ways to do this:\
\n\n\
To select contiguous items in the list, you can press \
the left mouse button and drag the mouse over the items \
you wish to select, then release the button.  A second \
way to select contiguous items is to point and click on \
the first item, then move the mouse to the last item you \
wish to select, and point and click on that item while \
holding down the SHIFT key.\
\n\n\
To select non-contiguous items, hold the CTRL key down \
while selecting the new item(s) with the mouse.  \
Whenever the CTRL key is held while clicking with the \
mouse, previously selected items will remain selected \
along with the newly selected item(s). \
\n\n\
Selecting a group causes all of the data items in the \
group to be selected. \
\n\n\
Once you have selected the items you wish to plot, \
select one of the View buttons (View, View Multiple, or \
View Overlaid) at the bottom of the Main Window (see \
the help section titled \"View - plotting your data\").\n\
", 0, 0},

/*        1         2         3         4         5        5 */
/* 345678901234567890123456789012345678901234567890123456789 */

showHBOOK = {"Show User Id or HBOOK ID Numbers", 'S', False, HELP_TEXT,
"\nClicking on the \"Show User Id or HBOOK ID Numbers\" toggle button will \
alternately display/hide the User I.D. histogram numbers of data items listed \
in the Histogram/Ntuple/Indicator/Control list. \n\n\
HBOOK data items have their User I.D. identical to the HBOOK I.D.", 0, 0},

showHsIds = {"Show Histo-Scope Ids", 'i', False, HELP_TEXT,
"\nClicking on the \"Show Histo-Scope Ids\" toggle button will \
alternately display/hide Histo-Scope ids of data items listed \
in the Histogram/Ntuple/Indicator/Control list. \n", 0, 0},

showData = {"Show Data Type", 'D', True, HELP_TEXT,
"\nClicking on the \"Show Data Type\" toggle button will \
show the data type for items listed \
in the Histogram/Ntuple/Indicator/Control list. \n", 0, 0},

terms = {"Terms", 'T', True, HELP_TEXT,
"Accelerator Key \n\
      A key or key combination used to perform a function\n\
      inside of a pull-down menu without actually pulling\n\
      that menu down with the mouse.  This allows faster\n\
      execution of desired program functions.  This feature\n\
      is available when a key or key combination is dis-\n\
      played in the menu alongside the menu function.\n\n\
Adaptive Histogram \n\
      A histogram whose bins, rather than being spaced\n\
      evenly, are partitioned according to the density of\n\
      the data.  A threshold value determines the number of\n\
      fills at which a single bin is divided into additional\n\
      bins.  Thus, the resolution of an adaptive histogram\n\
      is highest where the data is densest, and lowest where\n\
      the data is sparse.\n\n\
Category \n\
      Histo-Scope supports the notion of categories,\n\
      equivalent to HBOOK's directories, for arranging large\n\
      numbers of histograms and n-tuples in a hierarchy\n\
      rather than in a single long list.\n\n\
Control \n\
      A special type of item that appears in the Histogram/\n\
      Ntuple/Indicator/Control list, which can set the value\n\
      of a variable in a connected process.  Controls are\n\
      defined in the client process using hs_create_control.\n\n\
Dragging/Grabbing \n\
      A mouse action that allows you to move objects on the\n\
      screen.  Press and hold down the first (or select) \n\
      mouse button while moving the mouse on the desktop \n\
      (and the pointer on the screen).\n\n\
Histogram \n\
      1-dimensional: A plot of one variable over a pre-\n\
      specified range divided into a number of bins.\n\n\
      2-dimensional: A plot of two variables each with its\n\
      own range and bin size.\n\n\
Indicator \n\
      A special type of item that appears in the Histogram/\n\
      Ntuple/Indicator/Control list, which represents a\n\
      scalar value in a connected process or open file.\n\
      Indicators are defined in a client process using\n\
      hs_create_indicator.\n\n\
Menu Bar \n\
      The Histo-Scope Main Window has a menu bar across the\n\
      top of the window, just below the title bar.  Each\n\
      word in the menu bar holds a pull-down menu of\n\
      functions.\n\n\
Mnemonic \n\
      A mnemonic allows you to select a menu function which\n\
      has one of its letters underlined.  To select that\n\
      menu, press the Alt key while typing the letter that\n\
      is underlined.  For example, type Alt P to pull down\n\
      the Preferences menu.\n\n\
      After pulling down the Preferences menu, you can\n\
      select one of the items in it by typing the letter\n\
      that is underlined in that item.  For example, to\n\
      choose Update Frequency, type: U.\n\n\
Ntuple \n\
      A two-dimensional array composed of a fixed number of\n\
      named variables times a growing number of occurrences.\n\
      Histo-Scope allows you to specify which variables in\n\
      the n-tuple to plot and the type of plot to display.\n\n\
      Histo-Scope currently supports only in-memory n-tuples\n\
      of real numbers; i.e. n-tuples must fit in virtual\n\
      memory in order to be displayed by Histo-Scope.\n\n\
Pull-Down Menu \n\
      A menu which is displayed by dragging or clicking with\n\
      the mouse.  Some menus are found in the main window\n\
      menu bar (e.g. Preferences).  Another type of pull-\n\
      down menu is the Category pull-down menu which appears\n\
      with a small, brick-shaped menu bar symbol.\n\n\
Push Button \n\
      Push buttons appear as text or a meaningful graphic\n\
      outlined with a box.  Clicking a push-button with the\n\
      mouse causes an immediate action by the program.  A\n\
      push-button with a dark frame around it can be acti-\n\
      vated with the Return key.  The Histo-Scope Main Panel\n\
      has two pushbuttons: Open and View.\n\n\
Scatter Plot \n\
      An XY Scatter Plot plots two variables against each\n\
      other using a dot for each plotted point.  This kind \n\
      of plot is useful when there are enough data points to\n\
      be seen and their values form some visual pattern.\n\
      An XYZ Scatter Plot plots three variables.\n\n\
Sub Category \n\
      See Category above.\n\n\
Time Series Plot\n\
      A line or point plot of values from a list against their\n\
      index in the list.  A time series plot of an Ntuple\n\
      variable shows all of the variable's values in the order\n\
      in which they were collected.\n\n\
Title Bar \n\
      The top-most bar of a window, just inside the window\n\
      frame that contains title text for the window.  Drag-\n\
      ging on the title bar of a window will move the entire\n\
      window.\n\n\
Toggle Button\n\
      A toggle button appears as a small, square box and is\n\
      used to set and indicate a condition that can be on or\n\
      off.  On the Main Window, a toggle button is used to\n\
      indicate whether to show HBOOK histogram numbers. When\n\
      a toggle button appears \"in\" (or filled with shadow)\n\
      the button is \"on\".  When the toggle button appears\n\
      \"out\" (or full of light) the button is \"off.\"\n\n\
Trigger\n\
      A special type of item that appears in the Histogram/\n\
      Ntuple/Indicator/Control list, which can cause an\n\
      action to be triggered in a connected process.\n\
      Triggers are defined in the client process using\n\
      hs_create_trigger.\n\n\
Window Menu Button \n\
      The button just to the left of the title bar.  Used to\n\
      display or \"pull-down\" the Motif window menu.\n\n\
XY Plot \n\
      A line or point plot of values from ntuple variables\n\
      plotted against the corresponding values in another\n\
      variable.  In a sorted xy plot, points are sorted\n\
      according to the values of the X axis variable.", 0, 0},

relNts = {"Release Notes", 'R', False, HELP_TEXT,
"\nIt is possible for Histo-Scope to fail with a segmentation violation \
or a bus error when closing plot windows or creating overlay plots from \
configuration files.  Which conditions produce such errors differ depending \
on the operating system.  We have spent much time looking into these \
problems and believe them to be caused by internal problems in Motif.  \
Some of the problems have been worked around, but it was impossible to \
work around all problems on all operating systems.  If you encounter such \
problems, you may want to re-run Histo-Scope and try to accomplish what you \
were trying to do another way.  Or try running Histo-Scope on another kind of \
Unix system (utilizing the remote connections feature) if such problems \
persist.\n\n\
Occasionally while printing a plot using flpr, you may \
experience a delay for up to a minute.  This is usually because the queue \
you've specified does not exist.  After the delay, Histo-Scope will display \
the error that occurred.\n\n\
Some, possibly older, X-Terminals and servers do not properly destroy dialogs \
when the parent window is destroyed.  On these servers doing anything with \
these orphan windows can cause Histo-Scope to terminate with an error such \
as Segmentation Violation.  To avoid this on X servers with this problem, be \
sure to \
close slider, statistics, and axis settings windows before closing their \
corresponding plot window.\n\n\
Sometimes HBOOK n-tuple names listed in the \
Histogram/Ntuple/Indicator/Control list \
box of the Main Window are incomplete or completely garbled.  To our knowledge \
this is a problem in the HBOOK routine that returns the name of the n-tuple to \
Histo-Scope and no work-around has yet been found for supplying the \
correct name of the n-tuple.\n", 0, 0},

nplotRelNts = {"Release Notes", 'R', False, HELP_TEXT,
"\nOccasionally while printing a plot using flpr, you may \
experience a delay for up to a minute.  This is usually because the queue \
you've specified does not exist.  After the delay, NPlot will display \
the error that occurred.\n\n\
Some, possibly older, X-Terminals and servers do not properly destroy dialogs \
when the parent window is destroyed.  On these servers doing anything with \
these orphan windows can cause NPlot to terminate with an error such \
as Segmentation Violation.  To avoid this on X servers with this problem, be \
sure to \
close slider, statistics, and axis settings windows before closing their \
corresponding plot window.", 0, 0},

cmdLine = {"Command Line and Options", 'm', False, HELP_TEXT,
"\nTo run Histo-Scope, use the command: \n\
\n\
  % histo\n\
\n\
Optionally, you can specify a Histo-Scope format data \
file to open on the command line:\n\
\n\
  % histo histoFile.hs\n\
\n\
This parameter should be used last on the command line \
when using any options that can be used with a Histo-Scope \
data file.\n\
\n\
To specify an HBOOK file to open, use the option -hbook:\n\
\n\
  % histo -hbook hbookfile.hst\n\
\n\
Many HBOOK files require the block size to be specified:\n\
\n\
  % histo -hbook hbookfile.hst -blocksize 1024\n\
\n\
To specify a client process on your local machine for \
Histo-Scope to connect to:\n\
\n\
  % histo -clientid clientname\n\
\n\
where clientname is the string which was passed to \
HS_INITIALIZE in your client program.\n\
\n\
If the client process is on a remote machine, use the \
-rhost option and, if necessary, the -ruser option to \
specify a different username on that machine.  For example:\n\
\n\
  % histo -clientid HsExample -rhost fnalu -ruser E831\n\
\n\
You should specify only one file or process on the histo \
command line since Histo-Scope only allows one file to be \
open or one process to connect to at a time.\n\
\n\
If you have specified a file to open or a client process to \
connect to, you can also specify a configuration file to \
automatically load with the -config option.  For example:\n\
\n\
  % histo -config file.cfg histoFile.hs\n\
\n\
Histo-Scope will first obtain the data from the file or \
process and then read the configuration file to display \
your plots.\n", 0, 0},

newF = {"New Features", 'N', False, HELP_TEXT,
"\nThe most significant new features in Histo-Scope V4.0 are:\n\
\n\
    - 2D plots can be displayed overlaid\n\
    - Color support for most 2D plots\n\
    - New fill patterns for Histograms\n\
    - Histograms can now be plotted with markers\n\
    - Re-titling of plot windows\n\
    - Multiple selection of data items in main window\n\
    - New \"View Overlaid\" button in main window\n\
    - New \"View Multiple\" button in main window\n\
    - Item data types can be displayed in main window\n\
    - New command line options\n\
    - New routines in the API library\n\
    - C++ Binding for API contributed by Univ. of Penn.\n\
\n\
Most kinds of 2D plots can now be displayed on top of each other in an \
Overlay plot.  There are three methods for creating overlaid plots: \
1) using the \"View Overlaid\" button in the main panel, 2) viewing a plot \
group whose group type is HS_OVERLAY, or 3) dragging one plot over \
another.\n\
\n\
Histo-Scope now allows multiple selection of data items in the main window \
list box.  To do this, use the Ctrl and Shift keys in addition to the \
first mouse button.  You can also hold the mouse button down and select \
contiguous items in the list by dragging while holding the button down. \
Multiple selections are useful for creating overlaid plots, creating multiple \
plot windows, or just saving time when plotting individual plots.\n\
\n\
The following 2D plots now allow you to specify colors if running Histo-Scope \
from a color workstation or X-Terminal:  1D Histograms, \
Time Series Plots, XY Plots, and Adaptive 1D Histograms.  To set colors \
for lines, markers, or fill patterns, \
select \"Set Histogram Style\" or \"Set Mark & Line Style\" from the plot \
settings menu.\n\
\n\
Histograms can now have various fill patterns.  To specify these, \
select \"Set Histogram Style\" from the plot settings menu.\n\
\n\
Histograms can now be plotted as a marker with/without error bars. \
To do this, choose the \"Set Histogram Style\" option from the plot \
settings menu and click on the linestyle option menu.  At the bottom \
of the list of linestyles you will find marker types you can select.\n\
\n\
You can now specify in the histo command that you want to connect \
to a client.  Use the -clientid option followed by the string you \
put in the HS_INITIALIZE call, and if necessary use the -rhost and \
-ruser options to specify the remote host and remote username (just \
as you would in the Connect to Client dialog).  These options can \
be used with the -config option which specifies the configuration file \
to read.\n\
\n\
There are new callable routines in the API library:\n\
\n\
      hs_histo_with_config\n\
      hs_num_connected_scopes\n\
      hs_load_config_string\n\
      hs_load_config_file\n\
      hs_create_group\n\
      hs_delete_items\n\
      hs_kill_histoscope\n\
\n\
A C++ Binding for the Histo-Scope API was contributed by Roque D. Oliveira at \
the University of Pennsylvania.  These files can be found in Histo-Scope's \
contrib directory ($HISTO_DIR/contrib).\n\
\n\
The Histo-Scope User's Manual has been updated for V4.0.  A hardcopy version \
is (still) available from the Fermilab Computing Division Library.  The manual \
is also available via the web at:\n\n\
    http://www-pat.fnal.gov/histo_doc/histo_ug.html\n\n\
A Postscript file of the manual is in Histo-Scope's \
doc directory ($HISTO_DIR/doc). \n\
", 0, 0},

vR = {"Version", 'V', False, HELP_TEXT,
"\n\
Histo-Scope Version 4.0\n\
Copyright (c) 1992, 1993, 1994, 1995, 1996 Universities Research Association, Inc.\n\
All rights reserved.\n\
\n\
Histo-Scope was written by Mark Edel, Konstantine Iourha, Joy Kyriakopulos, \
Jeff Kallenbach, Paul Lebrun, Nina Denisenko and Baolin Ren at Fermi National \
Accelerator Laboratory.\n", 0, 0},

nplotVer = {"Version", 'V', False, HELP_TEXT,
"\n\
NPlot Version 4.0\n\
Copyright (c) 1992, 1993, 1994, 1995, 1996 Universities Research Association, Inc.\n\
All rights reserved.\n\
\n\
NPlot was written by Mark Edel, Konstantine Iourha, Jeff Kallenbach, \
Joy Kyriakopulos, Nina Denisenko and Baolin Ren at Fermi National \
Accelerator Laboratory.\n", 0, 0},

capGettingStarted = {"Getting Started", 'G', True, HELP_TEXT,
"\nHisto-Scope is a tool to select and display histograms, n-tuples, and \
scalar values from the program as data is being created.\n\
\n\
Initially, data items from the top-level category are listed in the \
Histogram/Ntuple/Indicator/Control list.  Use the Category \
pull-down menu and the Sub Categories list box to navigate through a \
hierarchy of data items.\n\n\
To plot a data item \
click on \
it and press the View button.\n\n\
The Preferences pull-down menu allows you to specify how often Histo-Scope \
should request new data from the connected process and to set/unset buffer \
graphics mode.\n\n\
To exit Histo-Scope, use the Window menu.", 0, 0},

nplotGetStd = {"Getting Started", 'G', True, HELP_TEXT,
"\nNPlot is a tool for quickly plotting columnar data from text files. \n\n\
To plot data from a file, first open the file using Open... in the File \
pull-down menu.  \
NPlot will list each column from the data file as a variable in the \
variables list.  If you include a column heading line in the file \
(a line beginning with \">\", see File Format), you can control \
the variable names used to \
represent each column of data.  After opening the file, choose the type \
of plot you want to see from the Plot Type pull-down \
menu, assign the variables to plot, and press the \"Plot\" button.\n\n\
There are two ways to assign variables for the plot.  Double-clicking on \
a variable name from the Variables list box will assign that variable to \
the first vacant plot position.  Or, click once on a variable to highlight \
it, then click on the push-button to assign the variable to the desired plot \
position.  To de-assign a variable, click on \"<Clear Entry>\" and then \
click on the plot position you'd like to clear.\n\n\
Up to fourteen variables can be assigned for certain plot types.  To expose \
additional plot assignments, drag the bottom frame of the NPlot window \
downward with the mouse.  The window will grow downward as you pull.\n\n\
The Preferences pull-down menu allows you to set/unset buffer \
graphics mode.\n\n\
To exit NPlot, use the File menu.", 0, 0},

/*        1         2         3         4         5        5 */
/* 345678901234567890123456789012345678901234567890123456789 */

nplotFileFmt = {"File Format", 'o', True, HELP_TEXT,
"NPlot reads simple tab or space delimited data with each column representing \
a variable.  Comments can be included in the file if they begin with a \"#\" \
in column 1.  If a line beginning with \">\" appears in the file, its contents \
will be used as column headings for the data.  The column headings should be \
separated by spaces and or tabs.  To include spaces in a column heading, \
enclose it in quotes.  For example:\n\n\
   # A Comment\n\
   >X		Y	\"A Name with Spaces\"\n\
   2.34		5.2	6.3\n\
   24352345.2	3453	.2\n\
   34.2		-25	1.4e-20\n\
   .\n\
   .\n\
   .", 0, 0},

nplotTypes = {"Plot Types", 'T', False, HELP_TEXT,
"\nA Time Series Plot will plot all of the values of a variable (all of the \
numbers in that column) in the order that they occur in the file.  \
Thus, the x-axis values roughly correspond to the line number in the file.  \
The value of the variable is plotted along the y-axis of the plot. \
If only one variable \
is plotted, its name appears at the top of the y-axis.  If more \
than one variable is plotted, a legend of variable names appears at the \
bottom of the plot.\n\
\n\
A sorted xy plot requires at least two variables.  The x-axis variable is \
sorted in order of increasing value and each succeeding variable is plotted \
along the y-axis \"against\" the first variable.  \
Up to thirteen Y-variables can be plotted in one window.  If only one \
Y-variable is plotted, its name appears at the top of the y-axis.  If more \
than one is plotted, a legend of variable names appears at the \
bottom of the plot.\n\
\n\
Scatter Plots plot two variables (XY), or three variables (XYZ), against \
each other using a dot for each plotted point.  In addition to the \
variables plotted \
along the plot axes, additional variables can be selected for animation \
sliders. \
This kind of plot is useful when there \
are enough data points to be seen and their values form some visual pattern. \
Plotted dots can be thickened or drawn lighter by selecting Thicken Points \
from the plot settings menu. \n\
\n\
A 1-dimensional histogram is a plot of one variable \
over a pre-specified range divided into a number of bins.\n\n\
A 2-dimensional \
histogram is a plot of two variables \
each with its own range and bin size.\n\
\n\
Adaptive histograms are histograms whose bins, rather than being spaced \
evenly, are partitioned according to the density of \
the data.  A threshold value determines the number of \
fills at which a single bin is divided into additional \
bins.  The threshold can be adjusted using the \"Bin Limit\" sliders \
available from the menu in the plot window.\n\
\n\
Many settings used in each of the plots can be changed using the Plot \
Settings menu.  To pop up the menu, press the rightmost mouse button from a \
selected plot window.", 0, 0},

closeW = {"Close", 'C', False, HELP_TEXT,
"\nSelecting Close from the plot settings window will close the plot window \
and make it disappear.  If the window is a Multiple Plot Window, the \
entire multi-plot and all of its mini-plots will be closed. \
\n\nTyping Ctrl W on a selected plot window will \
also achieve the same effect. \
\n\nIt is a good idea to close plots that are no longer \
needed when connected to a process to avoid the \
CPU overhead of transferring and displaying data.  It also saves you screen \
space.", 0, 0},

delMultW = {"Delete Mini Plot", 'D', False, HELP_TEXT,
"\nSelecting Delete Mini Plot from the plot settings window will \
delete the mini-plot from a multiple plot window.  The empty cell will \
be able to accept another plot if one is dragged into the formerly \
occupied space. \
\n\nIt is a good idea to close plots that are no longer \
needed when connected to a process to avoid the \
CPU overhead of transferring and displaying data.  ", 0, 0},

showRange = {"Show Range", 'S', True, HELP_TEXT,
"\nShow Range mode means that the minimum and \
maximum values are displayed and a sliding scale display shows \
the current value relative to the minimum and maximum.  Turning off \
show range mode displayes the indicator or control in a smaller window.  \
Selecting the \"Show Range\" item in the plot settings menu toggles \
show range mode on and off.", 0, 0},

binStrat = {"Binning Strategies", 't', True, HELP_TEXT,
"\nHisto-Scope provides two adaptive histogram binning strategies: \
\"Split in Half,\" and \"Center of Gravity.\"  Split in half is simply \
dividing the bins into two equal parts when the threshold number of fills \
is reached.  Center of gravity \
starts with the whole data set and begins splitting the bins at their median \
value until the bins all contain less than threshold number of fills.  \
(Because there is also a limit on the number of splits (20), bins can actually \
have more than the threshold number of fills, but these bins will already be \
imperceptibly narrow except at very high magnifications).\n\
\n\
To change the binning strategy, select the desired strategy in the plot \
menu.", 0, 0},

errorData = {"Showing Error Data", 'E', False, HELP_TEXT,
"\nIf error data is available for the plot, selecting \"Error Data\" will \
hide or show error bars using the error data from the file or process.", 0, 0},

gaussErrors = {"Showing Gaussian Errors", 'G', False, HELP_TEXT,
"\nSelecting \"Gaussian Errors\" will create error bars for the histogram \
showing the square root of the bin contents.", 0, 0},

labeling = {"Axis Numbering", 'G', True, HELP_TEXT,
"\nHistogram axes can optionally be numbered using the exact values of the \
bin edges rather than an ordinary plot scale.  To change the labeling \
style, select \"Label at Bin Edges.\"  When the bins are very narrow, \
the plot reverts to the ordinary axis labeling scheme regardless of the \
setting of this option.", 0, 0},

indW = {"Indicators", 'I', True, HELP_TEXT,
"\nThe Indicator window by default shows the minimum and maximum values \
of the indicator (at the far left and right), and the value of the indicator \
is displayed in the middle of \
the window in a rectangular box.  \n\nIf the indicator has not yet been given \
a value, the words \"* not set *\" will be displayed in the box.", 0, 0},

controlW = {"Controls", 'C', True, HELP_TEXT,
"\nControls allow a variable in a process to be set interactively \
using Histo-Scope.  To set the value of the variable, first either type a \
value in the text field in the lower left corner of the control window, or \
use the slider to enter the value.  Then, press the \"Set\" button. \
When the value is read and accepted by the connected process, the current \
value (in the top right corner) and the indicator above the slider will \
change to reflect the requested value.\n\
In the connected process, controls can be read by calling \
hs_read_control.", 0, 0},

triggerW = {"Triggers", 'T', True, HELP_TEXT,
"\nTriggers are buttons which can cause an action in a connected process.  \
Each time the button in the trigger window is pressed, a message is sent \
to the process and, if the code in the process properly checks the trigger, \
an associated routine or code fragment will be executed.  Until the trigger \
is read by the connected process, the button will read \"Pending.\"  If \
the button is pressed several times before it is read, it will show a count \
of the number of unprocessed presses.\n\
In the connected process, triggers can be detected by calling \
hs_check_trigger." , 0, 0},

windowM = {"Window menu", 'W', False, HELP_TEXT,
"\nTo close all windows (and exit the Histo-Scope), select Close All from \
the Window menu (or type Ctrl W).", 0, 0},

timeS = {"Time Series Plots", 'T', False, HELP_TEXT,
"\nA Time Series Plot will plot the value of a variable from an ntuple in \
the sequence in which that variable was collected.  Thus, the x-axis values \
start at zero and increase to the number of occurrences in the ntuple.  The \
value of the ntuple variable is plotted along the y-axis of the plot.\n\n\
Up to eighteen variables can be plotted in one window.  If only one variable \
is plotted, its name appears at the top of the y-axis.  If more \
than one variable is plotted, a legend of variable names appears at the \
bottom of the plot.\n\n\
To change the size of the plot drag on the window frame.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

XYplot = {"XY Plots", 'X', False, HELP_TEXT,
"\nAn XY plot plots pairs of points in two dimensions.  The points are taken \
in sequence from the ntuple, and are initially drawn with lines connecting \
the points and no markers.\n\nThe XY plot can also be used as an alternative \
to the scatter plot.  Use the mark and line style dialog to turn on markers \
and turn off connecting lines.  When the number of points is small, \
the marks drawn by the XY plot are much more visible than the pixel-sized \
dots drawn by the scatter \
plot.  The XY plot can also plot multiple sets of variable pairs from the \
ntuple, and distinguish them with different marker styles, as well as \
draw both horizontal and vertical error bars.\n\n\
Since the points are not sorted, the initial appearance of the plot \
may include lines which loop back on themselves.  The Sorted XY Plot \
is often a better way to observe how y varies with x.\n\n\
The plot legend name or Y axis label is taken from the name of the Y \
variable.\n\n\
You can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

XYsort = {"Sorted XY Plots", 'X', False, HELP_TEXT,
"\nAn XY Line plot requires at least two variables.  The x-axis variable is \
sorted in order of increasing value and each succeeding variable is plotted \
along the y-axis \"against\" the first variable.\n\n\
Up to seventeen Y-variables can be plotted in one window.  If only one \
Y-variable is plotted, its name appears at the top of the y-axis.  If more \
than one is plotted, a legend of variable names appears at the \
bottom of the plot.\n\n\
You can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

scatXY = {"XY Scatter Plots", 'X', False, HELP_TEXT,
"\nAn XY Scatter Plot plots two variables against each other \
using a dot for each plotted point.  This kind of plot is useful when there \
are enough data points to be seen and their values form some visual pattern. \
\n\nPlotted dots can be thickened or lightened by selecting Thicken Points \
from the plot settings menu. \n\n\
In addition to the variables plotted along the x- and y-axis, \
additional variables can be selected for animation sliders. \n\n\
Initially, the entire range of the Ntuple data is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

scatXYZ = {"XYZ Scatter Plots", 'X', False, HELP_TEXT,
"\nAn XYZ Scatter Plot plots three variables against each other \
using a dot for each plotted point.  This kind of plot is useful when there \
are enough data points to be seen and their values form some visual pattern. \
\n\nPlotted dots can be thickened or lightened by selecting Thicken Points \
from the plot settings menu. \n\n\
In addition to the variables plotted along the x- and y-axis, \
additional variables can be selected for animation sliders. \n\n\
Initially, the entire range of the Ntuple data is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
To rotate the plot, use the mouse to drag inside of the boundaries of the \
cube frame.  Dragging outside of the cube frame rotates the plot about an \
axis perpendicular to the plane of the window.   \
You can change the scale of the plot by dragging on the axis \
tics (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.  \
Once you have scaled the plot, it is possible to grab points inside the \
axis lines and pan the plot, moving it around inside the axes. (See \
Interactive Controls)", 0, 0},

hist1D = {"1-Dimensional Histogram Plots", 'H', False, HELP_TEXT,
"\nA 1-dimensional histogram is a plot of one variable \
over a pre-specified range divided into a number of bins.\n\n\
Initially, the entire range (excluding overflows) of a histogram is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

overlay = {"Overlay Plots", 'H', False, HELP_TEXT,
"\nAn overlay plot is the result of using the \"View Overlaid\" button in the \
main panel, viewing a plot group whose group type is HS_OVERLAY, or dragging \
one plot over another.  Some of the features and options offered for the \
original plots are not available in the overlaid state.  To access these \
features, you must remove the plot from the overlay (use \"Remove Overlayed \
Plot\"), re-create it individually, and merge it back in to the overlay.  \
Sliders, including rebin sliders, in an overlayed plot, refer to the original \
plot, to which other plots have been dragged.\n\
\n\
Interactive sliders and rebinning can still be used on the original (bottom) \
plot in the overlay.  The menu items: Show Statistics, Show Sliders, and \
Show Rebin Slider, all refer to this original plot.", 0, 0},

hist1DA = {"1-Dimensional Adaptive Histogram", 'H', False, HELP_TEXT,
"\nA 1-dimensional histogram is a plot of one variable \
over a pre-specified range divided into a number of bins.  An adaptive \
histogram is a histogram whose bins, rather than being spaced evenly, \
are partitioned according to the density of the data.  A threshold value \
determines the number of fills at which a single bin is divided into \
additional bins.  Thus, the resolution of an adaptive histogram is \
highest where the data is densest, and lowest where the data is sparse.\n\
\n\
Histo-Scope provides two adaptive histogram binning strategies: \
\"Split in Half,\" and \"Center of Gravity.\"  Split in half is simply \
dividing the bins into two equal parts when the threshold number of fills \
is reached.  Center of gravity \
starts with the whole data set and begins splitting the bins at their median \
value until the bins all contain less than threshold number of fills.  \
(Because there is also a limit on the number of splits (20), bins can actually \
have more than the threshold number of fills, but these bins will already be \
imperceptibly narrow except at very high magnifications)\n\
\n\
Initially, the entire range (excluding overflows) of a histogram is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

hist2D = {"2-Dimensional Histogram Plots", 'H', False, HELP_TEXT,
"\nA 2-dimensional \
histogram is a plot of two variables, where each axis is divided into a number \
of bins and has its own range and bin size.\n\n\
Initially, the entire range (excluding overflows) of a histogram is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot on an axis, it is possible to grab a point \
just inside of the axis and pan the plot so that it moves along the axis.\n\n\
The orientation of the histogram can be changed by grabbing a part of the \
display (away from an axis) with the mouse and moving \
the display in space to a different orientation. (See Interactive \
Controls)", 0, 0},

hist2DA = {"2-Dimensional Adaptive Histogram", 'H', False, HELP_TEXT,
"\nA 2-dimensional \
histogram is a plot of two variables, where each axis is divided into a number \
of bins.  An adaptive \
histogram is a histogram whose bins, rather than being spaced evenly, \
are partitioned according to the density of the data.  A threshold value \
determines the number of fills at which a single bin is divided into \
additional bins.  Thus, the resolution of an adaptive histogram is \
highest where the data is densest, and lowest where the data is sparse.\n\
\n\
Histo-Scope provides two adaptive histogram binning strategies: \
\"Split in Half,\" and \"Center of Gravity.\"  Split in half is simply \
dividing the bins into two equal parts when the threshold number of fills \
is reached.  Center of gravity \
starts with the whole data set and begins splitting the bins at their median \
value until the bins all contain less than threshold number of fills.  \
(Because there is also a limit on the number of splits (20), bins can actually \
have more than the threshold number of fills, but these bins will already be \
imperceptibly narrow except at very high magnifications)\n\
\n\
Initially, the entire range of the data is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\
\nOnce you have scaled the plot on an axis, it is possible to grab a point \
just inside of the axis and pan the plot so that it moves along the axis.\n\n\
The orientation of the histogram can be changed by grabbing a part of the \
display (away from an axis) with the mouse and moving \
the display in space to a different orientation. (See Interactive \
Controls)", 0, 0},

cell = {"Cell Plot", 'C', False, HELP_TEXT,
"\nA cell plot is a flat representation of a 2-dimensional \
histogram, with bin contents represented by the size of a shaded rectangle \
drawn on the center of the bin.\n\n\
If the source of the data is an n-tuple, \
additional variables can be selected for animation sliders. \n\n\
Initially, the entire range of the data is displayed.\
\n\nYou can change the size of the plot by changing the size of its window.  \
Just drag on the window frame to make the window the size you desire.\n\n\
You can change the scale of the plot by using the mouse to drag a part of \
an axis (just outside of the plot) toward one direction or another.  Use a \
point near the upper half of the axis to change the maximum axis value.  Use \
a point near the lower half of the axis to change the minimum axis value.  \
This will zoom in or out, depending on the direction you drag that axis point.\
\n\nOnce you have scaled the plot, it is possible to grab a point inside the \
plot and pan the plot, moving it around inside the axes. (See Interactive \
Controls)", 0, 0},

controls2D = {"Interactive Controls", 'I', True, HELP_BITMAP, Help2D_bits,
	Help2D_width, Help2D_height},
	
controls3D = {"Interactive Controls", 'I', True, HELP_BITMAP, Help3D_bits,
	Help3D_width, Help3D_height},
	
controls2DHist = {"Interactive Controls", 'I', True, HELP_BITMAP,
	Help2DHist_bits, Help2DHist_width, Help2DHist_height};

/*
** Menu lists
**
** Some of the menu list arrays have the number of items specified because
** the IBM C compiler complains: "Number of initializers cannot be greater
** than the number of aggregate members"
*/
helpMenuInfo

*TSPlotHelp[] = {&timeS, &controls2D, &showStats, &markLine, &showLeg,
	&zoom, &pan, &loG, &updOpt, &printP, &genPOST, &closeW, NULL},

*TSPlotHelpM[15] = {&timeS, &controls2D, &showStats, &markLine, &showLeg,
	&multShowTit, &zoom, &pan, &loG, &updOpt, &printPMult, &genPOSTMult,
	&delMultW, &closeW, NULL},

*MainMenuHelp[18] = {&gettingStarted, &fileProcess, &preferences, &windows,
	&categoryList, &histNtIndList, &openP, &selectP, &viewP, &showHBOOK,
	&showHsIds, &showData, &terms, &cmdLine, &newF, &relNts, &vR, NULL},

*CapMainMenuHelp[] = {&capGettingStarted, &windowM, &preferences, &categoryList,
	&openP, &capViewP, &capSelectP, &showHBOOK, &showHsIds, &showData, &terms,
	&relNts, &vR, NULL},

*XYPlotHelp[] = {&XYplot, &controls2D, &showStats, &markLine, &showLeg, &zoom,  
	&pan, &loG, &updOpt, &printP, &genPOST, &closeW, NULL},

*XYPlotHelpM[15] = {&XYplot, &controls2D, &showStats, &markLine, &showLeg, 
	&multShowTit, &zoom,  &pan, &loG, &updOpt, &printPMult, &genPOSTMult,
	&delMultW,  &closeW, NULL},

*XYSortHelp[] = {&XYsort, &controls2D, &showStats, &markLine, &showLeg, &zoom,  
	&pan, &loG, &updOpt, &printP, &genPOST, &closeW, NULL},

*XYSortHelpM[15] = {&XYsort, &controls2D, &showStats, &markLine, &showLeg,  
	&multShowTit, &zoom,  &pan, &loG, &updOpt, &printPMult, &genPOSTMult,
	&delMultW,  &closeW, NULL},

*Scat2DHelp[] = {&scatXY, &controls2D, &showStatsS, &showSlid, &zoom, &pan,
	&loG, &thickPts, &updOpt, &printP, &genPOST, &closeW, NULL},

*Scat2DHelpM[15] = {&scatXY, &controls2D, &showStatsS, &showSlid, &multShowTit,
	&zoom, &pan, &loG, &thickPts, &updOpt, &printPMult, &genPOSTMult,
	&delMultW,  &closeW, NULL},

*Scat3DHelp[] = {&scatXYZ, &controls3D, &showStatsS, &showSlid, &zoom, &pan,
	&loG, &updOpt, &printP, &genPOST, &closeW, NULL},

*Scat3DHelpM[] = {&scatXYZ, &controls3D, &showStatsS, &showSlid, &multShowTit,
	&zoom, &pan, &loG, &updOpt, &printPMult, &genPOSTMult, &delMultW,
	&closeW, NULL},

*Hist1DHelp[17] = {&hist1D, &controls2D, &showStatsS, &showSlidH, &rebin, 
	&histStyle, &zoom,
	&pan, &loG, &errorData, &gaussErrors, &labeling, &updOpt, &printP,
	&genPOST, &closeW, NULL},

*Hist1DHelpM[19] = {&hist1D, &controls2D, &showStatsS, &showSlidH,   
	&rebin, &histStyle, 
	&multShowTit, &zoom, &pan, &loG, &errorData, &gaussErrors, &labeling, 
	&updOpt, &printPMult, &genPOSTMult, &delMultW, &closeW, NULL},

*OverlayHelp[14] = {&overlay, &controls2D, &showStatsS, &showSlidH,   
	&rebin, &histStyle, &zoom, &pan, &loG, &updOpt,
	&printP, &genPOST, &closeW, NULL},

*OverlayHelpM[16] = {&overlay, &controls2D, &showStatsS, &showSlidH, &rebin, 
	&histStyle, &multShowTit, &zoom, &pan, &loG, 
	&updOpt, &printPMult, &genPOSTMult, &delMultW, &closeW, NULL},

*Hist2DHelp[18] = {&hist2D, &controls2DHist, &showStatsS, &showSlidH, &rebin,
	&otherV, &zoom, &pan2D, &loG, &errorData, &gaussErrors, &labeling, 
	&showBackP, &updOpt, &printP, &genPOST, &closeW, NULL},

*Hist2DHelpM[20] = {&hist2D, &controls2DHist, &showStatsS, &showSlidH, &rebin,
	&otherV, &multShowTit, &zoom, &pan2D, &loG, &errorData, &gaussErrors, 
	&labeling, &showBackP, &updOpt, &printPMult, &genPOSTMult, &delMultW, 
	&closeW, NULL},

*Hist1DAHelp[] = {&hist1DA, &controls2D, &showStatsS, &showSlidH, &binLimit,
	&zoom, &pan, &loG, &binStrat, &updOpt, &printP, &genPOST, &closeW,NULL},

*Hist1DAHelpM[] = {&hist1DA, &controls2D, &showStatsS, &showSlidH, &binLimit,
	&multShowTit, &zoom, &pan, &loG, &binStrat, &updOpt, &printPMult, 
	&genPOSTMult, &delMultW, &closeW, NULL},

*Hist2DAHelp[15] = {&hist2DA, &controls2DHist, &showStatsS, &showSlidH,
	&binLimit, &zoom, &pan2D, &loG, &binStrat, &showBackP, &updOpt,
	&printP, &genPOST, &closeW, NULL},

*Hist2DAHelpM[17] = {&hist2DA, &controls2DHist, &showStatsS, &showSlidH,
	&binLimit, &multShowTit, &zoom, &pan2D, &loG, &binStrat, &showBackP, 
	&updOpt, &printPMult, &genPOSTMult, &delMultW, &closeW, NULL},

*CellHelp[] = {&cell, &controls2D, &showStatsS, &otherV, &normSlid, &showSlid, 
	&zoom, &pan, &updOpt, &printP, &genPOST, &closeW, NULL},

*CellHelpM[] = {&cell, &controls2D, &showStatsS, &otherV, &normSlid, &showSlid, 
	&multShowTit, &zoom, &pan, &updOpt, &printPMult, &genPOSTMult, 
	&delMultW, &closeW, NULL},

*IndicatorHelp[] = {&indW, &showRange, &updOpt, &closeW, NULL},

*ControlHelp[] = {&controlW, &showRange, &closeW, NULL},

*TriggerHelp[] = {&triggerW, NULL},

*NPlotHelp[] = {&nplotGetStd, &nplotFile, &nplotPref, &nplotWindows,
	&nplotTypes, &nplotFileFmt, &nplotRelNts, &nplotVer, NULL};
    

void AutoHelp2D(Widget parent)
{
    static char helpShown = False;
    
    if (helpShown || !PrefData.plotAutoHelp)
        return;
        
    CreateBitmapHelpDialog(parent, "Interacting with 2D Plots", Help2D_bits,
    	    Help2D_width, Help2D_height);
    helpShown = True;
}
        
void AutoHelp3D(Widget parent)
{
    static char helpShown = False;
    
    if (helpShown || !PrefData.plotAutoHelp)
        return;
        
    CreateBitmapHelpDialog(parent, "Interacting with 3D Plots", Help3D_bits,
    	    Help3D_width, Help3D_height);
    helpShown = True;
}
 
void AutoHelp2DHist(Widget parent)
{
    static char helpShown = False;
    
    if (helpShown || !PrefData.plotAutoHelp)
        return;
        
    CreateBitmapHelpDialog(parent, "Interacting with 2D Histograms",
    	    Help2DHist_bits, Help2DHist_width, Help2DHist_height);
    helpShown = True;
}

void TimeSeriesHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &timeS);
}

void XYHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &XYplot);
}

void XYSortHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &XYsort);
}

void Scat2DHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &scatXY);
}

void Scat3DHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &scatXYZ);
}

void Hist1DHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &hist1D);
}

void Hist2DHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &hist2D);
}

void Hist1DAHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &hist1DA);
}

void Hist2DAHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &hist2DA);
}

void CellHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &cell);
}

void IndicatorHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &indW);
}

void TriggerHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &triggerW);
}

void ControlHelpCB(Widget widget, caddr_t client_data, caddr_t call_data)
{
    helpButtonDialog(widget, &controlW);
}

void NtuplePanelHelpCB(Widget w, caddr_t clientData, caddr_t callData)
{
    CreateHelpDialog(w, "Plotting N-Tuple Data", 
"\nTo plot data from an n-tuple, choose the desired plot type, \
assign the variables to plot and then press the \"Plot\" button.\n\
\n\
Choose the plot type by clicking on the Plot Type pull down menu at the \
top right of the window and then selecting one of the listed plot types by \
clicking on the desired plot type.  \
(Or, using the mouse, point to the Plot Type pull-down menu, press and hold \
the select button, drag the mouse pointer to the desired \
plot type and release the mouse button.)\n\
\n\
Each plot type allows you to assign ntuple variables to be used in the plot.  \
Variables assigned to positions beginning with X or Y will be used along the \
X or Y axis.  Variables assigned to positions beginning with \"S\" are used \
as slider variables to make \
cuts or slices of the data.  Variable place-holders beginning with \"E\" are \
used as error information for the plot variable listed above it (H and V are \
also used in the XY Plot w/Errors to represent horizontal and vertical \
error bars).\n\n\
There are two ways to assign variables for the plot.  Double-clicking on \
a variable name from the Variables list box will assign that variable to \
the first vacant plot position.  Or, click once on a variable to highlight \
it, then click on the push-button to assign the variable to the desired plot \
position.  To de-assign a variable, click on \"<Clear Entry>\" and then \
click on the plot position you'd like to clear.\n\
\n\
In addition to the variables that are part of the n-tuple, you can use the \
Create Variable button to derive additional variables and constants for \
plotting (press Help in the Create Variable panel for more information).  \
Once you have created a variable, selecting it in the Variables list and \
pressing Modify Variable will pop up a panel for making changes.\n\
\n\
Up to eighteen variables can be assigned for certain plot types.  To expose \
additional plot assignments, drag the bottom frame of the Ntuple window \
downward with the mouse.  The window will grow downward as you pull.\n\
\n\
A Time Series Plot will plot the value of a variable from an Ntuple in \
the sequence in which that variable was collected.  The x-axis values \
start at zero and increase to the number of occurrences in the Ntuple.  The \
value of the Ntuple variable is plotted along the y-axis of the plot. \
If only one variable \
is plotted, its name appears at the top of the y-axis.  If more \
than one variable is plotted, a legend of variable names appears at the \
bottom of the plot.\n\
\n\
An XY plot plots pairs of points in two dimensions, showing the points as \
connected line segments, and/or marker symbols.  In a sorted xy plot, \
the x-axis variable is \
sorted in order of increasing value and each succeeding variable is plotted \
along the y-axis against the first (sorted) variable.  A sorted xy plot \
can show up to seventeen Y-variables in one window.  If only one \
Y-variable is plotted, its name appears at the top of the y-axis.  If more \
than one is plotted, a legend of variable names appears at the \
bottom of the plot.\n\
\n\
Scatter Plots plot two variables (XY), or three variables (XYZ), against \
each other using a dot for each plotted point.  In addition to the \
variables plotted \
along the plot axes, additional variables can be selected for animation \
sliders.  \
This kind of plot is useful when there \
are enough data points to be seen and their values form some visual pattern.  \
Plotted dots can be thickened or drawn lighter by selecting Thicken Points \
from the plot settings menu.  XY Plots can be used as an alternative to the \
two dimensional scatter plot, by using the \"Set Mark & Line Style\" menu \
item in the plot background menu to turn off lines and turn on markers.  For \
small data sets, xy plot markers are more visible than scatter plot \
points.  The xy plot can also present error bars and plot multiple sets \
of points.\n\
\n\
A 1-dimensional histogram is a plot of one variable \
over a pre-specified range divided into a number of bins.\n\n\
A 2-dimensional \
histogram is a plot of two variables \
each with its own range and bin size.\n\
\n\
Adaptive histograms are histograms whose bins, rather than being spaced \
evenly, are partitioned according to the density of \
the data.  A threshold value determines the number of \
fills at which a single bin is divided into additional \
bins.  The threshold can be adjusted using the \"Bin Limit\" sliders \
available from the menu in the plot window.\n\
\n\
Many settings used in each of the plots can be changed using the Plot \
Settings menu.  To pop up the menu, press the rightmost mouse button from a \
selected plot window.");
}

void VariablePanelHelpCB(Widget w, caddr_t clientData, caddr_t callData)
{
    CreateHelpDialog(w, "Creating Derived Variables", 
"To create a derived variable, fill in the Variable Name and Expression \
fields in this dialog and press Create.\n\
\n\
The variable name can be any arbitrary character \
string, though strings without spaces or punctuation are easier to use in \
expressions.\n\
\n\
The expression should be a fortran or C style arithmetic expression.  Valid \
operators are: +, -, /, *, ^ (or **).  There are also a number of built-in \
single-argument functions and constants.  These are listed in the \
Paste Function and Paste Constant menus.  Variable names containing spaces \
or punctuation, or starting with a number should be surrounded in double \
quotes (\").  Multi-line expressions with \
temporary variable assignments are also allowed, for example:\n\
\n\
   a = sqrt(X) + 2*Y\n\
   b = Z**2 / (4*X)\n\
   a**2 + b*a**3\n\
\n\
The value of the last line of the expression in is taken as the value of the \
variable.\n\
\n\
Once the variable is created, it can be modified or deleted using the same \
panel.  If you dismiss the panel or lose it under other windows on your \
screen, you can pop it up again by selecting the \
variable in the ntuple browser and pressing Modify Variable.");
}

/*
** Add information about the right mouse button to the help normally
** available using the help menu.  The purpose is to create help dialogs
** to be activated by the Help button which is available on some workstations.
** This help will tell the really dumb users who ignored the automatic help
** that they should press the right mouse button to get a plot menu.
*/
static void helpButtonDialog(Widget parent, helpMenuInfo *help)
{
    int len = strlen(help->text);
    static char psString[] = "\n\nPress the right mouse button to pop up \
a menu of settings for this window.";
    char *newText;
    
    newText = (char *)XtMalloc(sizeof(char) * (len + strlen(psString) + 1));
    strcpy(newText, help->text);
    strcpy(&newText[len], psString);
    CreateHelpDialog(parent, help->topic, newText);
    XtFree(newText);
}
