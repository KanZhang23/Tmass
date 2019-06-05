#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Script parameters
set c_template_file "data_integ_template_mtm3.c"
set c_driver_file "data_integ_mtm3.c"
set cfiles [list $c_driver_file parameter_parser.c]

# Check the input
if {$argc != 1} {
    puts ""
    puts "Usage: [file tail [info script]] hsfile"
    puts ""
    exit 1
}
foreach {hsfile} $argv break

package require hs
hs::initialize "Compile Driver"

# Load the libraries needed to run the code
set source_dir [file dirname [info script]]
set hostname [info hostname]
if {$hostname == "fcdflnxgpvm01.fnal.gov"} {
    source [file join $source_dir local_defs_fcdflnx.tcl]
} else {
    source [file join $source_dir local_defs.tcl]
}
source [file join $source_dir scan_utils.tcl]
preload_libs

# Generate the driver file from the template
set itemlist [hs::dir $hsfile]
if {[llength $itemlist] != 1} {
    puts stderr "File \"$hsfile\" contains more than one item"
    exit 1
}
foreach {uid category title type} [lindex $itemlist 0] {}
if {![string equal $type "HS_NTUPLE"]} {
    puts stderr "File \"$hsfile\" does not contain an ntuple"
    exit 1
}
hs::read_file $hsfile

set string_to_replace "NTUPLE_UNPACKING_CODE"
set status [generate_scan_code 1 $c_template_file $string_to_replace $c_driver_file]
if {$status} {
    puts "Generated file $c_driver_file from template $c_template_file"
} else {
    puts stderr "String \"$string_to_replace\" is not present in file $c_template_file"
    exit 1
}

foreach {dll outfile} [eval compile_and_load $cfiles \
                           -Wno-unused-but-set-variable] break
puts "Compiled library $outfile (and checked that it can be loaded)"

exit 0
