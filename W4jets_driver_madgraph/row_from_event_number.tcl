#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}
catch {wm withdraw .}

# Check the input
if {$argc != 3} {
    puts ""
    puts "Usage: [file tail [info script]] filename run event"
    puts ""
    exit 1
}
foreach {filename inrun event} $argv break
if {![string is integer -strict $inrun]} {
    error "Run number must be an integer"
}
if {![string is integer -strict $event]} {
    error "Event number must be an integer"
}

# Initilaize Histo-Scope
package require hs
hs::initialize "row from event"

# Load the input file
set nread [hs::read_file $filename "Input"]
if {$nread != 1} {
    error "Expected an input file containing a single ntuple"
}
set id [hs::id_from_title "minintuple" "Input"]
if {$id <= 0} {
    error "Expected an input file containing an item with title \"minintuple\""
}

set runCol [hs::variable_index $id "runNumber"]
set ev0Col [hs::variable_index $id "eventNum0"]
set ev1Col [hs::variable_index $id "eventNum1"]
if {$runCol < 0 || $ev0Col < 0 || $ev1Col < 0} {
    error "The ntuple does not contain run number/event number variables"
}

set found 0
set count 0
foreach run [hs::data_to_list [hs::column_contents $id $runCol]] \
    ev0 [hs::data_to_list [hs::column_contents $id $ev0Col]] \
    ev1 [hs::data_to_list [hs::column_contents $id $ev1Col]] {
        set irun [expr {round($run)}]
        set iev [expr {round($ev0*65536) + round($ev1)}]
        if {$irun == $inrun && $iev == $event} {
            puts "Match found for row $count"
            incr found
        }
        incr count
}
if {$found > 1} {
    puts "\nWarning: $found matches found"
}
if {$found == 0} {
    puts "No matches found"
}
   
exit 0
