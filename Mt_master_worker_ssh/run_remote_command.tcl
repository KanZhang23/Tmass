#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Load the utilities
set source_dir [file dirname [info script]]
source [file join $source_dir remote_eval.tcl]

# Check the input arguments
if {$argc < 3} {
    puts ""
    puts "Usage: [file tail [info script]] host port command arg0 arg1 ..."
    puts ""
    exit 1
}
puts [eval remote_eval $argv]
exit 0
