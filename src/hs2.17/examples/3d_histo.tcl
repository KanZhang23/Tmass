
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded
histo_init

# Create a three-dimensional histogram
set id [hs::create_3d_hist [next_uid] "Random filled 3d histogram"\
	 "Example 3d histo" "X" "Y" "Z" "V" 15 15 15 -4 4 -5 5 -6 6]

# Fill the histogram with some random numbers
puts -nonewline "Filling 3d histogram ... "
flush stdout
for {set i 0} {$i < 50000} {incr i} {
    hs::fill_3d_hist $id [gauss_random 0 1] \
	    [gauss_random 0 1.5] [gauss_random 0 2] 1.0
}
puts "Done"

# Create 1d and 2d slicers
hs::slice_slider $id X
hs::slice_slider $id Y Z

