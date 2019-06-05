#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}

# Get rid of the main window
wm withdraw .

# Check the input arguments
set testdir [file dirname [info script]]
source [file join $testdir strd_utils.tcl]
if {$argc != 3} {
    puts stderr ""
    puts stderr "Usage: [file tail [info script]] dataset_name start_num is_tune"
    puts stderr ""
    puts stderr "dataset_name should be one of \"[join [lsort\
	    [strd_dataset_list]] {", "}]\"."
    puts stderr ""
    puts stderr "start_num should be 1, 2, or 3."
    puts stderr "  1 -- starting point is very far away from the solution"
    puts stderr "  2 -- starting point is reasonably close to the solution"
    puts stderr "  3 -- solution is the starting point"
    puts stderr ""
    puts stderr "is_tune should be \"on\" (to start the tuner)\
	    or \"off\" (to try an automatic fit)"
    puts stderr ""
    exit 1
}
foreach {dataset_name start_num is_tune} $argv break

# Check the arguments
if {[lsearch -exact [strd_dataset_list] $dataset_name] < 0} {
    puts stderr "Bad dataset \"$dataset_name\". Must be one of\
	    [join [lsort [strd_dataset_list]] {, }]."
    exit 1
}
if {![string is integer -strict $start_num]} {
    puts stderr "Bad start number \"$start_num\", should be 1, 2, or 3"
    exit 1
}
if {$start_num < 1 || $start_num > 3} {
    puts stderr "Bad start number \"$start_num\", should be 1, 2, or 3"
    exit 1
}
if {![string is boolean -strict $is_tune]} {
    puts stderr "Expected a boolean value, got \"$is_tune\""
    exit 1
}

# Check that the dataset file exists
set strd_data_dir [file join $testdir Data]
set filename [file join $strd_data_dir $dataset_name.dat]
if {![file readable $filename]} {
    retrieve_strd_dataset $dataset_name $filename
    if {![file readable $filename]} {
	puts stderr "Dataset \"$dataset_name\" data not found (or unreadable)"
	exit 1
    }
}

# Initialize Histo-Scope
package require hs
hs::initialize "StRD test"

# Make sure we can fit things...
if {![hs::have_cernlib]} {
    puts stderr "The Hs extension has been compiled without CERNLIB."
    puts stderr "In this mode nonlinear regression is not supported."
    exit 1
}

# Start the Histo-Scope GUI
hs::histoscope 1
hs::wait_num_scopes > 0
hs::periodic_update 100

# Read in the file
set chan [open $filename r]
set lines [split [read $chan [file size $filename]] "\n"]
close $chan

# Parse the header
foreach {start_param end_param start_data end_data} \
	[parse_strd_header [lrange $lines 0 6]] {}

# Parse the parameter definitions
set paramnames {}
foreach paramline [lrange $lines $start_param $end_param] {
    foreach {pname = start1 start2 value stdev} $paramline break
    lappend paramnames $pname
    foreach q {start1 start2 value stdev} {
	set param_data($pname,_,$q) [set $q]
    }
}

# A proc to print the best parameter values
proc expected_solution {} {
    uplevel \#0 {
	puts ""
	puts "**** Expected solution *****"
	foreach param $paramnames {
	    puts "$param = [expr {1.0 * $param_data($param,_,value)}]\
		    +- [expr {1.0 * $param_data($param,_,stdev)}]"
	}
    }
}

# Parse the definition of variables
set dataspec [lindex $lines [expr {$start_data - 1}]]
if {![string equal -length 5 $dataspec "Data:"]} {
    puts stderr "Data statement not found on line\
	    $start_data of file $filename"
}
set varnames [lrange $dataspec 1 end]

# Build an ntuple out of the given points
set id [hs::create_ntuple [next_uid] "$dataset_name data" "StRD data" $varnames]
if {[string equal $dataset_name "Nelson"]} {
    # This guy has a log in the response variable
    foreach data_buffer [lrange $lines $start_data $end_data] {
	set data_buffer [concat [expr log([lindex $data_buffer 0])]\
		[lrange $data_buffer 1 end]]
	# Force internal list representation before filling the ntuple
	llength $data_buffer
	hs::fill_ntuple $id $data_buffer
    }
} else {
    foreach data_buffer [lrange $lines $start_data $end_data] {
	# Force internal list representation before filling the ntuple
	llength $data_buffer
	hs::fill_ntuple $id $data_buffer
    }
}

# Basic fit creation
set dataspec [list $id -v [lindex $varnames 0] -x [lindex $varnames 1]]
if {[llength $varnames] > 2} {
    lappend dataspec -y [lindex $varnames 2]
}
set fit [hs::fit $dataspec {} -title "Fit of $dataset_name data" -method L2]

# Add parameters
switch $start_num {
    1 {set index start1}
    2 {set index start2}
    3 {set index value}
    default {error "Internal error"}
}
foreach param $paramnames {
    $fit parameter add $param -value $param_data($param,_,$index)
}

# Add the function
add_strd_model $fit $dataset_name

# Check whether we should start the tuner
if {$is_tune} {
    $fit tune
} else {
    $fit fit
    $fit plot
    expected_solution
}

package require rdl
rdl::interact
