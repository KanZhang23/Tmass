#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

set interactive_debug 0

set scanlib "integ_w4jets.so"
# set rng_dump_file "qmc_sequence.txt"
# set fcn_dump_file "raw_function_values.txt"
set rng_dump_file ""
set fcn_dump_file ""

# Check the input
if {$argc != 6 && $argc != 7} {
    puts ""
    puts "Usage: [file tail [info script]] test_mode param_file tf_file infile outfile nev \\"
    puts "       \[nskip\]"
    puts ""
    puts " test_mode    0 or 1. Use 1 to run in \"test mode\". In this mode, the"
    puts "              integration code will run much faster but the results"
    puts "              will be much less precise."
    puts ""
    puts " param_file   File which contains W + 4 jets integration parameters."
    puts ""
    puts " tf_file      File which contains the W + jets transfer functions"
    puts "              (normally, with extension .gssa or .gssaz)."
    puts ""
    puts " infile       Input file (normally, with extension .hs)."
    puts ""
    puts " outfile      Output file (normally, with extension .hs)."
    puts ""
    puts " nev          Number of events to process. Use 0 to process all events"
    puts "              in the input file."
    puts ""
    puts " nskip        (Optional) number of events to skip at the beginning of"
    puts "              the input file. Default value of this argument is 0."
    puts ""
    exit 1
}
global test_mode
foreach {test_mode parameter_file tf_file inputfile outputfile maxevents} $argv break

set nskip 0
if {$argc > 6} {
    set nskip [lindex $argv 6]
}

if {$maxevents == 0} {
    set maxevents 2147483647
}

package require hs
hs::initialize "Scan"

# Load all necessary libraries
set source_dir [file dirname [info script]]
if {[info hostname] == "fcdflnxgpvm01.fnal.gov"} {
    source [file join $source_dir local_defs_fcdflnx.tcl]
} else {
    source [file join $source_dir local_defs.tcl]
}
preload_libs

global scan_code_loaded
if {![info exists scan_code_loaded]} {
    set scan_code_loaded [hs::sharedlib open \
        [file join $source_dir $scanlib]]
}

# Define the scan parameters
source $parameter_file
parameter "maxevents" $maxevents
parameter "tf_config_file" $tf_file
parameter "rng_dump_file" $rng_dump_file
parameter "fcn_dump_file" $fcn_dump_file
parameter "skipevents" $nskip

# Procedure to run on each event
proc scanit {id} {
    global scan_code_loaded integ_params
    hs::ntuple_dll_scan $id $scan_code_loaded $integ_params
}

hs::read_file $inputfile "MyInput"
set input_id [lindex [hs::list_items "" "MyInput/..." 1] 0]

if {$interactive_debug} {
    package require rdl
    rdl::interact 1    
}

scanit $input_id
hs::delete $input_id
hs::sharedlib close $scan_code_loaded

file mkdir [file dirname $outputfile]
set nItems [hs::num_items]
if {[hs::save_file $outputfile] == $nItems && $nItems > 0} {
    puts "Wrote file $outputfile"
    exit 0
} else {
    puts "Failed to write file $outputfile"
    exit 1
}
