
# Source this file into a running "hswish". Then run a command like this:
#
# browse_graphs category xvariable yvariable

proc browse_graphs {category {xvariable mt} {yvariable prob}} {
    package require Tk
    set uidlist {}
    set have_2d_histos 0
    set have_ntuples 0
    foreach id [hs::list_items "" $category 1] {
        if {[string equal [hs::type $id] "HS_2D_HISTOGRAM"]} {
            lappend uidlist [hs::uid $id]
            set have_2d_histos 1
        } elseif {[string equal [hs::type $id] "HS_NTUPLE"]} {
            set varlist [hs::ntuple_variable_list $id]
            if {[lsearch -exact $varlist $xvariable] < 0} {
                break
            }
            if {[lsearch -exact $varlist $yvariable] < 0} {
                break
            }
            lappend uidlist [hs::uid $id]
            set have_ntuples 1
        }
    }
    if {[llength $uidlist] == 0} {
        error "No appropriate items in category \"$category\""
    }
    if {$have_2d_histos && $have_ntuples} {
        error "Can't mix 2d histos and ntuples"
    }
    set uidlist [lsort -integer $uidlist]
    set lolim [lindex $uidlist 0]
    set hilim [lindex $uidlist end]
    set nbins [expr {$hilim - $lolim + 1}]
    set new_label "UID"
    set wingeometry +100+490
    set plotgeometry 400x300+100+150

    if {$have_ntuples} {
        set slice_id [hs::create_ntuple [next_uid] "$category slice" \
                          "Slices" [list $xvariable $yvariable]]
        hs::overlay ov1 show clear -window "gslide_$slice_id" \
            -title "Category \"$category\"" -geometry $plotgeometry \
            add $slice_id xy -x $xvariable -y $yvariable
        set snapshot_command browse_graphs_snapshot
    } else {
        set id0 [hs::id [lindex $uidlist 0] $category]
        set slice_id [hs::duplicate_axes $id0 [next_uid] "$category slice" "Slices"]
        hs::show_histogram $slice_id -window "gslide_$slice_id" \
            -title "Category \"$category\"" -geometry $plotgeometry
        set snapshot_command browse_2d_snapshot
    }

    set w .gslide_$slice_id
    toplevel $w
    wm withdraw $w
    wm title $w "UID in category \"$category\""

    scale $w.scale -orient horizontal -length 384 \
        -tickinterval [expr {$hilim-$lolim}] \
        -from $lolim -to $hilim \
        -resolution 1 -repeatinterval 300 -command \
        [list browse_graphs_callback $slice_id \
             $category $xvariable $yvariable]
    bind $w.scale <ButtonPress-1> +[list focus %W]
    bind $w.scale <KeyPress-Up> break
    bind $w.scale <KeyPress-Down> break

    frame $w.controls
    button $w.controls.dismiss -text "Dismiss" -width 8 \
        -command "destroy $w;\
                  hs::close_window gslide_$slice_id 1;\
                  hs::delete $slice_id"
    button $w.controls.snapshot -text "Snapshot" -width 8 \
        -command [list $snapshot_command \
                      $w.scale $category $xvariable $yvariable]
    ::hs::Int_entry_widget $w.controls.speed "Frame delay (ms): " \
        red2 1 9999 300 ::hs::Int_entry_incr ::hs::Int_entry_decr \
        [list ::hs::Config_scale_repeat [list $w.scale]]

    pack $w.controls.speed -side left -padx 2 -fill x
    pack $w.controls.dismiss -side right -padx 2 -fill x
    pack $w.controls.snapshot -side right -padx 2 -fill x
    pack $w.scale -side top -padx 5 -fill x -expand 1
    pack $w.controls -side top -padx 5 -fill x -expand 1
    frame $w.spacer -height 8
    pack $w.spacer -side top -fill x -expand 1

    update idletasks
    wm geom $w $wingeometry
    wm deiconify $w
    wm resizable $w 1 0
    wm protocol $w WM_DELETE_WINDOW "$w.controls.dismiss invoke"
    after idle [list browse_graphs_callback $slice_id $category \
                    $xvariable $yvariable $lolim]
    return $w
}

proc browse_graphs_callback {slice_id category xvariable yvariable uid} {
    hs::reset $slice_id
    set uid [expr {round($uid)}]
    set id [hs::id $uid $category]
    if {$id > 0} {
        if {[string equal [hs::type $id] "HS_NTUPLE"]} {
            hs::ntuple_project $id $slice_id {expr 1} \
                [subst {expr \$$xvariable}] [subst {expr \$$yvariable}]
        } else {
            if {[hs::hist_error_status $id] > 0} {
                hs::2d_hist_block_fill $slice_id [hs::2d_hist_bin_contents $id] \
                    [hs::2d_hist_errors $id]
            } else {
                hs::2d_hist_block_fill $slice_id [hs::2d_hist_bin_contents $id]
            }
        }
    } else {
        # hs::reset does not reset the errors!
        # Looks like a Histo-Scope bug to me...
        if {[hs::hist_error_status $slice_id] > 0} {
            hs::hist_set_gauss_errors $slice_id
        }
    }
    hs::hs_update
    hs::hs_update
    return
}

proc browse_2d_snapshot {scale category xvariable yvariable} {
    set uid [$scale get]
    set uid [expr {round($uid)}]
    set id [hs::id $uid $category]
    if {$id > 0} {
        set newid [hs::copy_hist $id [next_uid] \
                       "UID $uid in category \"$category\"" "Snapshots"]
        hs::show_histogram $newid
    }
    return
}

proc browse_graphs_snapshot {scale category xvariable yvariable} {
    set snapshot_id [hs::create_ntuple [next_uid] "$category slice" \
                         "Snapshots" [list $xvariable $yvariable]]
    set uid [$scale get]
    set uid [expr {round($uid)}]
    set id [hs::id $uid $category]
    if {$id > 0} {
        hs::ntuple_project $id $snapshot_id {expr 1} \
            [subst {expr \$$xvariable}] [subst {expr \$$yvariable}]
    }
    hs::overlay ov1 show clear -window "snapshot_$snapshot_id" \
        -title "UID $uid in category \"$category\"" \
        add $snapshot_id xy -x $xvariable -y $yvariable
    return
}

proc assert_class {win class} {
    if {[string compare [winfo class $win] $class]} {
        error "Window $win os not of class $class"
    }
    return
}

proc tie_scales {args} {
    foreach win $args {
        assert_class $win.scale Scale
    }
    set len [llength $args]
    if {$len > 1} {
        for {set i 0} {$i < $len} {incr i} {
            set win [lindex $args $i]
            set other_wins [lreplace $args $i $i]
            set callback [list]
            foreach o $other_wins {
                lappend callback [subst -nocommands {$o.scale set [$win.scale get]}]
            }
            lappend callback [$win.scale cget -command]
            $win.scale configure -command [join $callback "\n"]

            set callback [list [concat untie_scales $args]]
            lappend callback [$win.controls.dismiss cget -command]
            $win.controls.dismiss configure -command [join $callback "\n"]
        }
    }
    return
}

proc untie_scales {args} {
    foreach win $args {
        if {[winfo exists $win]} {
            assert_class $win.scale Scale

            set callback [$win.scale cget -command]
            $win.scale configure -command [lindex [split $callback "\n"] end]

            set callback [$win.controls.dismiss cget -command]
            $win.controls.dismiss configure -command [lindex [split $callback "\n"] end]
        }
    }
    return
}

proc next_uid {} {
    global browse_graphs_next_uid_holder_variable
    if {![info exists browse_graphs_next_uid_holder_variable]} {
        set browse_graphs_next_uid_holder_variable 0
    }
    incr browse_graphs_next_uid_holder_variable
}

proc dirlist {} {
    set dirs {}
    foreach id [hs::list_items "" "..." 1] {
	lappend dirs [hs::category $id]
    }
    lsort -unique $dirs
}
