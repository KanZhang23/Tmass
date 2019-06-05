
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init

# Create a dummy histogram which will be used as a drawing board
set w win1
set id [hs::create_1d_hist [next_uid] "Ellipses" "Tmp" X Y 10 0 1]
hs::fill_1d_hist $id 0.5 0
hs::show_histogram $id -window $w -geometry 600x500

# Generate a bunch of random ellipses. This may take a while.
set colors {red green blue yellow magenta cyan black white}
set ncolors [llength $colors]
for {set i 0} {$i < 50} {incr i} {
    set x [expr rand()]
    set y [expr rand()]
    set width [expr {rand() * 0.5}]
    set height [expr {rand() * 0.5}]
    set rho [expr {(rand() * 2.0 - 1.0)*0.999}]
    set color [lindex $colors [expr {int(rand() * $ncolors)}]]
    # Do not draw the ellipse immediately (note the option -immediate 0):
    # this will result in too many window updates
    hs::draw ellipse -window $w -coord plot -immediate 0 -bg $color \
            [list [expr {$x - $width/2}] [expr {$y - $height/2}]] \
            [list [expr {$x + $width/2}] [expr {$y + $height/2}]] \
            [list $rho 0]
}

# Now, force the window update
hs::redraw
