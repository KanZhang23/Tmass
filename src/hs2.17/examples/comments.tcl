
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded
histo_init

# Buffer graphics to a pixmap so that labels
# do not flicker when the plot is zoomed.
hs::config_histoscope -bufferGraphics true

# Make a histogram
set w win1
set npoints 800
set id [random_histo gauss 30 -3 3 0 1 $npoints]
hs::show_histogram $id -window $w -geometry 500x500

# Example line anchored to the window top and bottom
# and to a certain horizontal position in the plot
foreach {mean std_dev} [hs::1d_hist_stats $id] {}
hs::draw line [list $mean plot o 0 winrel o] [list $mean plot o 1 winrel o] \
        -line 3 -fg blue

# Example line which starts at a certain position on the plot
# and ends at a certain distance from the top of the window
set refpoint [list $mean plot o 1 winrel o]
set maxvalue [lindex [hs::1d_hist_maximum $id] 2]
hs::draw line [list $mean plot o [expr {0.7 * $maxvalue}] plot o] \
        {40 winabs r -120 winabs r} {130 winabs l 0 winabs l} \
        -refpoint $refpoint -arrow back -fg blue

# The following comment is offset by the certain number of pixels
# from the top of the window and from the mean of the histogram.
# As a result, it moves horizontally when the histogram is zoomed
# but does not move vertically.
hs::comment [format "Mean is %.3f \xb1 %.3f" $mean \
        [expr {$std_dev/sqrt($npoints)}]] -font {times -12} \
        {45 winabs r -115 winabs r} -refpoint $refpoint -fg blue

# Show a bunch of labels using different fonts
set i 0
foreach {font bgcolor fgcolor} {
    {"Zapf Chancery" 18}          black white
    {"Zapf Dingbats" 20}          red2  cyan
    {Symbol 18}                   green magenta
    {Helvetica 24 bold}           blue  yellow
    {Courier 18 italic}           white blue
    {"New Century Schoolbook" 14} white green4
    {Times 20}                    white red
} {
    hs::comment $font [list 0.08 [expr 0.1+0.08*[incr i]]] -font $font \
            -bg $bgcolor -fg $fgcolor -border on -immediate 0
}
hs::comment "Examples of fonts:" -font {Times 20 bold} \
        [list 0.08 [expr 0.1+0.08*[incr i]]] -bg gray80

