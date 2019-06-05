
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init

# Create an example histogram to fit
set id1 [random_histo gauss 50 -5 5 0.0 1.0 1000]
hs::hist_set_gauss_errors $id1
hs::hist_set_error_range $id1 1.0 {}

# Set up the fit and pop up the tuner
if {[hs::have_cernlib]} {
    set fit [hs::fit $id1 gauss]
    $fit tune
}

# Make a 2d histogram to slice
set id2 [hs::create_2d_hist [next_uid] "Random filled 2d histogram"\
	"Example category" "X" "Y" "Z" 20 20 -5 5 -5 5]
for {set i 0} {$i < 10000} {incr i} {
    hs::fill_2d_hist $id2 [gauss_random 0 1] [gauss_random 0 2] 1.0
}

# Make a slicer for this histogram
foreach {w id3} [hs::slice_slider $id2 x] {}

# Pop up the histogram browser
hs::browse_collection [list $id1 $id2 $id3] -color blue

# Pop up the function browser
hs::function_browser

