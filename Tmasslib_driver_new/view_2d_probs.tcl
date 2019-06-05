#!/bin/sh
# The next line restarts using tclsh \
exec wish "$0" ${1+"$@"}
catch {wm withdraw .}

# Parse the input arguments
if {$argc < 2} {
    puts ""
    puts "Usage: [file tail [info script]] show_gui file0 ..."
    puts ""
    exit 1
}
set show_gui [lindex $argv 0]
set inputs [lrange $argv 1 end]

package require hs
hs::initialize "Prob viewer"

set source_dir [file dirname [info script]]
source [file join $source_dir browse_graphs.tcl]
source [file join $source_dir scan_utils.tcl]

proc upcategory {category} {
    join [lrange [split $category /] 1 end] /
}

proc show_delta {uid dir1 dir2} {
    set id1 [hs::id $uid $dir1]
    if {$id1 < 0} {
        error "Item with uid $uid and category $dir1 not found"
    }
    set id2 [hs::id $uid $dir2]
    if {$id2 < 0} {
        error "Item with uid $uid and category $dir2 not found"
    }
    set newid [hs::sum_histograms [next_uid] [hs::title $id1] \
                   "show_delta" $id1 $id2 1.0 -1.0]
    hs::show_histogram $newid
    return $newid
}

proc histo_bounds {pointset} {
    set n [llength $pointset]
    set min [lindex $pointset 0]
    set max [lindex $pointset end]
    set delta [expr {($max - $min)*0.5/($n - 1)}]
    list $n [expr {$min - $delta}] [expr {$max + $delta}]
}

proc make_proto_histo {id} {
    set mtset [lsort -real -unique [hs::data_to_list \
        [hs::column_contents $id [hs::variable_index $id mt]]]]
    foreach {curve_steps mtmin mtmax} [histo_bounds $mtset] {}

    set jesset [lsort -real -unique [hs::data_to_list \
        [hs::column_contents $id [hs::variable_index $id jes]]]]
    foreach {jes_sigma_steps jes_sigma_min jes_sigma_max} \
        [histo_bounds $jesset] {}

    hs::create_2d_hist 1 "Proto histo" \
        "Tmp" "Delta JES" Mt Weight $jes_sigma_steps $curve_steps \
        $jes_sigma_min $jes_sigma_max $mtmin $mtmax
}

set id_proto -1
foreach infile $inputs {
    hs::read_file $infile "Curves"
    foreach id [hs::list_items "" "Curves/..." 1] {
        if {$id_proto <= 0} {
            set id_proto [make_proto_histo $id]
        }
        set categ "[file tail $infile]/[upcategory [hs::category $id]]"
        set id_proj [hs::duplicate_axes $id_proto [hs::uid $id] \
                         [hs::title $id] $categ]
        hs::ntuple_c_project $id $id_proj 1 prob jes mt
        if {[hs::variable_index $id "errprob"] >= 0} {
            hs::reset $id_proto
            hs::ntuple_c_project $id $id_proto 1 errprob jes mt
            hs::set_2d_errors $id_proj [hs::2d_hist_bin_contents $id_proto]
        }
    }
    hs::delete_category "Curves/..."
}
hs::delete $id_proto

if {$show_gui} {
    start_gui
    set dirlist [dirlist]
    if {[llength $dirlist] == 1} {
        catch {browse_graphs [lindex $dirlist 0]}
    }
} else {
    hs::periodic_update 100
    puts "Histo-Scope server is running on [info hostname], port [hs::server_port]."
    puts "Forward the port via ssh if this script is not running on your local machine."
    puts "Use \"histo -rhost localhost -port local_port_number\" to connect."
}

prompt
