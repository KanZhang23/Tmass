#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Check the input
if {$argc < 4} {
    puts ""
    puts "Usage: [file tail [info script]] thisRow run event inputfile0 inputfile1 ..."
    puts ""
    exit 1
}
foreach {thisRow run event} $argv break
set filelist [lrange $argv 3 end]

package require hs
hs::initialize "Row finder"

proc column_values {id name} {
    set col [hs::variable_index $id $name]
    if {$col < 0} {
        error "bad column name \"$name\""
    }
    hs::data_to_list [hs::column_contents $id $col]
}

foreach inputfile $filelist {
    if {[hs::read_file $inputfile "InputData"] <= 0} {
        puts stderr "Failed to read file $inputfile"
        exit 1
    }
    set id [lindex [hs::list_items "" "InputData/..." 1] 0]

    set rows [column_values $id "thisRow"]
    set runs [column_values $id "runNumber"]
    set ev0 [column_values $id "eventNum0"]
    set ev1 [column_values $id "eventNum1"]

    hs::delete_category "..."

    set rcount 0
    set matches [list]
    foreach r $rows e0 $ev0 e1 $ev1 rn $runs {
        if {$event == [expr {int($e0)*65536 + int($e1)}] && \
                $run == [expr {int($rn)}] && $thisRow == [expr {int($r)}]} {
            lappend matches $rcount
        }
        incr rcount
    }

    if {[llength $matches] > 0} {
        puts "$inputfile : matching rows are $matches"
    } else {
        puts "$inputfile : no matching rows"
    }
}

exit 0
