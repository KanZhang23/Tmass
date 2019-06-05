#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

set scanlib "data_integ_mtm3.so"
set nskip 0

# Check the input
if {$argc != 6} {
    puts ""
    puts "Usage: [file tail [info script]] param_file tf_dir infile outfile maxpoints nevents"
    puts ""
    puts " param_file   File which contains top mass ME integration parameters."
    puts ""
    puts " tf_dir       Directory which contains the transfer function data."
    puts ""
    puts " infile       Input file (normally, with extension .hs)."
    puts ""
    puts " outfile      Output file (normally, with extension .hs)."
    puts ""
    puts " maxpoints    Number of quasi-MC points to use for integration. Set this"
    puts "              argument to 0 (or negative) in order to use the default."
    puts ""
    puts " nevents      Number of events to process. Use 0 to process all events"
    puts "              in the input file."
    puts ""
    exit 1
}
foreach {parameter_file tf_dir inputfile outputfile maxpoints nrun} $argv break
if {$nrun == 0} {
    set maxevents 2147483647
} else {
    set maxevents [expr {$nskip + $nrun}]
}

# Are we profiling the top mass library?
set profile_mode [llength [info commands mtm3scan]]
if {$profile_mode} {
    puts "Running in the profile mode"
    flush stdout
}

package require hs
hs::initialize "Scan"

# Load all necessary libraries
set source_dir [file dirname [info script]]
set hostname [info hostname]
if {$hostname == "fcdflnxgpvm01.fnal.gov"} {
    source [file join $source_dir local_defs_fcdflnx.tcl]
} else {
    source [file join $source_dir local_defs.tcl]
}
if {!$profile_mode} {
    preload_libs
}
source [file join $source_dir scan_utils.tcl]
source [file join $source_dir eta_bin_boundaries.tcl]

# Load the top mass scan code
if {!$profile_mode} {
    global scan_code_loaded
    if {![info exists scan_code_loaded]} {
        set scan_code_loaded [hs::sharedlib open \
             [file join $source_dir $scanlib]]
    }
}

# Define the scan parameters
source $parameter_file
parameter "skipevents" $nskip
parameter "maxevents" $maxevents
if {$maxpoints > 0} {
    parameter "mc_max_points" $maxpoints
}

# Load transfer functions and efficiencies
global transfer_functions_loaded
if {![info exists transfer_functions_loaded]} {
    puts -nonewline "Loading transfer functions and efficiencies... "
    flush stdout
    load_transfer_functions $tf_dir [parameter minExceedance] \
        [parameter partonMassSplit] [parameter drStretchFactor] \
        [parameter jetPtCutoff] [parameter interpolate_tfs_over_params]
    puts "Done"
    flush stdout
    set transfer_functions_loaded 1
}

# Initialize LHAPDF
set pdf_to_use "cteq6l1/0"
set renorm_scale_factor 1.0   
init_lhapdf $pdf_to_use $renorm_scale_factor

# Initialize the random number generator (used for QMC scrambling)
uniform_random_setseed 1898055953

# Procedure to run on each ntuple
proc scanit {id} {
    global profile_mode scan_code_loaded scan_parameters
    if {$profile_mode} {
        set r [mtm3scan $id $scan_parameters]
    } else {
        set r [hs::ntuple_dll_scan $id $scan_code_loaded $scan_parameters]
    }
    return $r
}

hs::read_file $inputfile "MyInput"
set input_id [lindex [hs::list_items "" "MyInput/..." 1] 0]

# package require rdl
# rdl::interact 1

scanit $input_id
hs::delete $input_id

file mkdir [file dirname $outputfile]
if {[hs::save_file $outputfile] == [hs::num_items]} {
    puts "Wrote file $outputfile"
    exit 0
} else {
    puts "Failed to write file $outputfile"
    exit 1
}
