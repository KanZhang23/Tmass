# Procedure for waiting while processing events -- we don't want
# to block tcl processing of signals for a long period of time
proc wait_processing_events {sec} {
    if {$sec >= 0} {
        if {$sec} {
            set msec [expr {1000 * $sec}]
        } else {
            set msec idle
        }
        global wait_flag_Ghefl07GFyrq3DN
        set wait_flag_Ghefl07GFyrq3DN 0
        after $msec {
            global wait_flag_Ghefl07GFyrq3DN
            set wait_flag_Ghefl07GFyrq3DN 1
        }
        vwait wait_flag_Ghefl07GFyrq3DN
    }
    return
}

proc next_uid {} {
    global global_counter_next_uid
    if {![info exists global_counter_next_uid]} {
        set global_counter_next_uid 0
    }
    incr global_counter_next_uid
}

proc sum_columns {wait_sec nt_id} {
    wait_processing_events $wait_sec
    set newid [hs::create_ntuple [next_uid] "Untitled" "Tmp" {sum}]
    set nrows [hs::num_entries $nt_id]
    for {set i 0} {$i < $nrows} {incr i} {
        set row [hs::data_to_list [hs::row_contents $nt_id $i]]
        hs::fill_ntuple $newid [expr [join $row { + }]]
    }
    return $newid
}
