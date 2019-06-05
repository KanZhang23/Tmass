#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Some script parameters
set max_value_deviation 0.1
set max_error_deviation 0.2
set Minuit_strategy 1

# Other settings which can be changed
set testdir [file dirname [info script]]
set strd_data_dir [file join $testdir Data]
set use_minos 0
set save_file "strd_results.hs"
set table_file "strd_results.txt"

# Initialize Histo-Scope
package require hs
hs::initialize "StRD fits"

# Make sure we can fit things...
if {![hs::have_cernlib]} {
    puts stderr "The Hs extension has been compiled without CERNLIB."
    puts stderr "In this mode nonlinear regression is not supported."
    exit 1
}

# Source various utilities
source [file join $testdir strd_utils.tcl]

# Create the ntuple of results
set varnames {test_num start_num errmat_status \
	param_num fit_value fit_error cert_value cert_error}
set result_id [hs::create_ntuple [next_uid] "StRD results" "StRD data" $varnames]

# Cycle over the datasets
set test_num 0
foreach dataset_name [lsort [strd_dataset_list]] {
    # Check that the dataset file exists
    set filename [file join $strd_data_dir $dataset_name.dat]
    if {![file readable $filename]} {
	retrieve_strd_dataset $dataset_name $filename
	if {![file readable $filename]} {
	    puts stderr "Dataset \"$dataset_name\" data not found (or unreadable)"
	    exit 1
	}
    }

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
    set fit [hs::fit $dataspec {} -title "Fit of $dataset_name data" \
	    -method L2 -minos $use_minos -strategy $Minuit_strategy]

    # Add the fitting function
    add_strd_model $fit $dataset_name

    # Cycle over the starting points
    foreach start_num {1 2 3} {
	switch $start_num {
	    1 {set index start1}
	    2 {set index start2}
	    3 {set index value}
	    default {error "Internal error"}
	}

	# Add the parameters
	$fit parameter clear
	foreach param $paramnames {
	    $fit parameter add $param -value $param_data($param,_,$index)
	}

	# A hack to avoid an infinite loop in Minos which
	# happens for Misra1a dataset at starting point 3
	if {[string equal $dataset_name "Misra1a"] && $start_num == 3} {
	    $fit config -minos 0
	}

	# Perform the fit
	set is_fit_failed [catch {$fit fit}]
	if {[$fit cget -complete]} {
	    foreach {fmin fedm errdef npari nparx errmat_status} \
		    [$fit cget -ministat] {}
	} else {
	    set errmat_status 0
	}

	# Get the parameter values and fill the ntuple of results
	set pass_fail($dataset_name,$start_num,value) 1
	set pass_fail($dataset_name,$start_num,error) 1
	set param_num 0
	foreach param $paramnames {
	    set cert_value $param_data($param,_,value)
	    set cert_error $param_data($param,_,stdev)
	    set fit_value [$fit parameter $param cget -value]
	    set fit_error [$fit parameter $param cget -error]

	    hs::fill_ntuple $result_id [list $test_num $start_num \
		    $errmat_status $param_num $fit_value $fit_error \
		    $cert_value $cert_error]

	    set value_deviation [expr {abs(($fit_value-$cert_value)/$cert_error)}]
	    if {$value_deviation > $max_value_deviation} {
		set pass_fail($dataset_name,$start_num,value) 0
	    }
	    set error_deviation [expr {abs(($fit_error-$cert_error)/$cert_error)}]
	    if {$error_deviation > $max_error_deviation} {
		set pass_fail($dataset_name,$start_num,error) 0
	    }

	    incr param_num
	}
    }

    $fit delete
    catch {hs::function model delete}
    catch {hs::function gauss1 delete}
    incr test_num
}

# Preapare and print out the pass/fail table
set table_string ""
append table_string "Dataset name     Start 1      Start 2      Start 3 " "\n"
append table_string "------------    ---------    ---------    ---------" "\n"
foreach dataset_name [lsort [strd_dataset_list]] {
    set shortname [string range $dataset_name 0 11]
    append table_string [format {%12s} $shortname]
    foreach start_num {1 2 3} {
	if {$pass_fail($dataset_name,$start_num,value)} {
	    set result "pass/"
	} else {
	    set result "FAIL/"
	}
	if {$pass_fail($dataset_name,$start_num,error)} {
	    append result "pass"
	} else {
	    append result "FAIL"
	}
	append table_string "    $result"
    }
    append table_string "\n"
}
puts ""
puts $table_string
if {![string equal $table_file ""]} {
    set table_file [file join $testdir $table_file]
    set ch [open $table_file w]
    puts -nonewline $ch $table_string
    close $ch
    puts "Saved the pass/fail table in file $table_file"
}

# Save the ntuple of results and exit
if {![string equal $save_file ""]} {
    set save_file [file join $testdir $save_file]
    if {[hs::save_file_byids $save_file $result_id] != 1} {
	error "Failed to save the ntuple of results!"
    } else {
	puts "Saved the ntuple of results in file $save_file"
    }
}
exit 0
