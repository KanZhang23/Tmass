
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded
histo_init

# Create a two-dimensional histogram
set id [hs::create_2d_hist [next_uid] "Random filled 2d histogram"\
	 "Example 2d histo" "X" "Y" "Z" 20 20 -5 5 -5 5]

# Create a multiplot which shows three different 
# representations of this histogram
set rows_and_modes {0 lego 1 tartan 2 cell}
foreach {row mode} $rows_and_modes {
    hs::multiplot 2d_histos add $id 0,$row -mode $mode
}
hs::multiplot 2d_histos show -geometry 400x800 \
	-title "2d Histogram Plotting Modes" -window 2d_histos

# Label the mode in the upper right corner of each plot
foreach {row mode} $rows_and_modes {
    hs::comment $mode {0.97 0.95} -column 0 -row $row \
	    -border on -anchor ne -font {Helvetica 20} -bg white
}

# Fill the histogram with some random numbers
for {set i 0} {$i < 50000} {incr i} {
    hs::fill_2d_hist $id [gauss_random 0 1] [gauss_random 0 2] 1.0
    if {$i%100 == 0} hs::hs_update
}
