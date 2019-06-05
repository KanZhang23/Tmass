
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init
hs::config_histoscope -printTitles 0 -bufferGraphics 1

# Create a random histogram
set nbins 50
set id [random_histo gauss $nbins 0 $nbins [expr $nbins/2] 8 1000]
hs::show_histogram $id -window win -geometry 600x500 \
        -font {Helvetica 24} -ipady {50 0} -fill solid -fillcolor firebrick

# Show a comment
hs::comment {EPSF Import Example} {0.5 0.92} -font {Helvetica 32} -anchor c

# Reserve a temporary file name
foreach {epsfile chan} [::hs::tempfile [file join [::hs::tempdir] hs_] .eps] {}
close $chan

# Save/import the plot a few times
for {set i 0} {$i < 8} {incr i} {
    hs::generate_ps win $epsfile 1
    if {$i > 0} hs::undo
    hs::epsf $epsfile {0.45 0.4} -scale 0.45 -window win
}

# Delete the temporary file after giving the program enough time
# to complete the drawing (could be slow if viewed remotely)
after 10000 [list file delete $epsfile]

