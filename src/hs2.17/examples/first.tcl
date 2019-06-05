# A simple example which shows how to load the "hs" extension
# and start the Histo-Scope GUI. Source into a running wish.
#
# The first five commands in this file make up a rather common
# sequence. You might want to combine them into a single procedure
# for your future scripts. There is an example proc like that
# called "histo_init" in file "example_utils.tcl".

# Load the package
package require hs

# Initialize the Histo-Scope API
hs::initialize "Simple example"

# Start the Histo-Scope GUI
hs::histoscope 1

# Wait until the GUI connects
hs::wait_num_scopes > 0

# Start periodic updates. Do an update every 100 msec.
hs::periodic_update 100

# Create a 1d histogram
set id [hs::create_1d_hist 1 "Random histogram" \
	"Example category" "X label" "Y label" 60 -0.5 1.5]

# Fill the histogram with some random numbers
for {set i 0} {$i < 1000} {incr i} {
    hs::fill_1d_hist $id [expr {rand()}] 1
}

# Tell the GUI to pop up the plot
hs::show_histogram $id
