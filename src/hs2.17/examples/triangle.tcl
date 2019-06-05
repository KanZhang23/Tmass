
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded
histo_init

# Buffer graphics to a pixmap so that primitives
# do not flicker when the plot is zoomed.
hs::config_histoscope -bufferGraphics true

# Make a histogram
set id [hs::create_1d_hist [next_uid] "Linear ramp"\
	"Example plots" t V 50 0 50]
set data {}
for {set i 0} {$i < 50} {incr i} {
    lappend data $i
}
hs::1d_hist_block_fill $id $data
hs::show_histogram $id -window triangle_win -geometry 500x400

# Draw a triangle somewhere in the middle of the plot
set vertexes {{300 winabs o 200 winabs o} {0.4 0.5} {23 plot o 35 plot o}}
eval hs::draw polygon $vertexes -bg gray80
foreach vertex $vertexes color {red green blue} {
    hs::draw ellipse -refpoint $vertex -bg $color \
            {-5 winabs r -5 winabs r} {5 winabs r 5 winabs r}
}

# Write a message
set comment " Each vertex of the triangle is fixed\n\
	in one of the three coordinate systems:\n\
	absolute window system, relative window\n\
	system, and plot system."
hs::comment $comment -refpoint {100 winabs o 1 winrel o} -font {times -12} \
	[list 0 winabs r -25 winabs r]

set comment " Guess which coordinate system is used\n\
	for each vertex. (Hint: zoom the histogram\n\
	and resize the window to see how\n\
	the triangle transforms.)"
hs::comment $comment -refpoint {1 winrel o 0 winrel o} -font {times -12} \
	[list -290 winabs r 130 winabs r]
