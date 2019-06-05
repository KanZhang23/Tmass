#!/bin/sh
# The next line restarts using tclsh \
exec wish "$0" ${1+"$@"}
catch {wm withdraw .}

# Script parameters
set result_scale 1.0e8

# Check the input
if {$argc != 4} {
    puts ""
    puts "Usage: [file tail [info script]] category final_only input_file output_file"
    puts ""
    puts "Run \"hsdir\" on your file to see which categories are available."
    puts ""
    puts "Set \"final_only\" argument to 1 if you don't want to see the full"
    puts "history of integral values."
    puts ""
    exit 1
}
foreach {categ is_1d ifile output_file} $argv break

# Initilaize Histo-Scope
package require hs
hs::initialize "W + 4 jets ME results"

# Load useful utilities
set source_dir [file dirname [info script]]
if {[info hostname] == "fcdflnxgpvm01.fnal.gov"} {
    source [file join $source_dir local_defs_fcdflnx.tcl]
} else {
    source [file join $source_dir local_defs.tcl]
}
preload_libs
source [file join $source_dir w4jets_utils.tcl]
source [file join $source_dir load_results_as_histos.tcl]

proc move_to_subcategory {id subcat} {
    set categ [hs::category $id]
    if {[string length $categ]} {
        hs::change_category $id $categ/$subcat
    } else {
        hs::change_category $id $subcat
    }
}

set prefix ""
foreach useQmcUncert {1 0} uncertMode {Quasi Pseudo} {
    set idlist [load_results_as_histos $ifile $categ \
                    $prefix $is_1d $result_scale $useQmcUncert]
    foreach id $idlist {
        move_to_subcategory $id $uncertMode
        set newtitle "$uncertMode MC [hs::title $id]"
        hs::change_title $id $newtitle
    }
}

if {[hs::save_file $output_file] > 0} {
    puts "Saved file $output_file"
    exit 0
} else {
    puts stderr "Failed to save file $output_file"
    exit 1
}
