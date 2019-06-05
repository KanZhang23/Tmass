#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Script parameters
set cpp_template_file "integ_template_w4jets.cc"
set cpp_driver_file "integ_w4jets.cc"
set cppfiles [list $cpp_driver_file parameter_parser.cc packStringToHSItem.cc]

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
if {[info hostname] == "fcdflnxgpvm01.fnal.gov"} {
    source [file join $source_dir local_defs_fcdflnx.tcl]
} else {
    source [file join $source_dir local_defs.tcl]
}
source [file join $source_dir w4jets_utils.tcl]
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
set status [generate_scan_code 1 $cpp_template_file $string_to_replace $cpp_driver_file]
if {$status} {
    puts "Generated file $cpp_driver_file from template $cpp_template_file"
} else {
    puts stderr "String \"$string_to_replace\" is not present in file $cpp_template_file"
    exit 1
}

foreach {dll outfile} [eval compile_and_load $cppfiles \
                           -Wno-unused-but-set-variable] break
puts "Compiled library $outfile (and checked that it can be loaded)"

exit 0
