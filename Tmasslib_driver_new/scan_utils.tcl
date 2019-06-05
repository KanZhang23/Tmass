proc load_transfer_functions {tf_dir minExceedance partonMassSplit \
                                  drStretchFactor jetPtCutoff interpolate} {
    # Delta R efficiency file
    set deltaR_eff_file [file join $tf_dir delta_r_efficiency.hs]

    # All other TF and efficiency functions
    set tfs_gssa_file [file join $tf_dir ttbar_jet_tfs.gssa]

    # Now, load the transfer functions and the efficiencies
    load_tfs_2015 $tfs_gssa_file $minExceedance [get_eta_boundaries] \
        $partonMassSplit $drStretchFactor $jetPtCutoff $interpolate

    # Load the delta-R efficiency data
    set deltar_eff [load_efficiency_interpolator "deltaR" $deltaR_eff_file]
    set_qqbar_deltaR_interpolator $deltar_eff

    return
}

proc load_efficiency_interpolator {tag filename} {
    hs::read_file $filename "Myeff"
    set id [hs::id_from_title "Fitted $tag efficiency" "Myeff"]
    set interp [create_linear_interpolator_nd $id]
    hs::delete_category "Myeff/..."
    return $interp
}

# The following procedure can be used to set
# and to extract parameter values in a convenient manner
proc parameter {name {value "ImpoSsiBle_ParAmeTer_VaLue"}} {
    global scan_parameters
    if {[string equal $value "ImpoSsiBle_ParAmeTer_VaLue"]} {
        foreach pair $scan_parameters {
            foreach {n v} $pair break
            if {[string equal $n $name]} {
                return $v
            }
        }
        error "Parameter \"$name\" is undefined"
    } else {
        set oldlist $scan_parameters
        set scan_parameters [list [list $name $value]]
        foreach pair $oldlist {
            foreach {n v} $pair break
            if {![string equal $n $name]} {
                lappend scan_parameters $pair
            }
        }
        return $value
    }
}

proc init_random {} {
    global random_generator_init_seed
    if {![info exists random_generator_init_seed]} {
        set random_generator_init_seed [uniform_random_setseed 0]
    }
    return $random_generator_init_seed
}

proc c {} {
    set ::rdl::interactive 0
    return
}

proc start_gui {{histo_ids_to_show {}}} {
    hs::histoscope 1
    hs::wait_num_scopes > 0
    hs::periodic_update 100
    hs::config_histoscope -minLineWidth 1
    foreach id $histo_ids_to_show {
        hs::show_histogram $id
    }
    return
}

proc prompt {} {
    package require rdl
    catch {rdl::read_history}
    rename ::exit ::_history_write_exit
    proc ::exit {{status 0}} {
        catch {rdl::write_history}
        ::_history_write_exit $status
    }
    uplevel {rdl::interact 1}
}

proc next_uid {} {
    global hswish_next_uid_holder_variable
    if {![info exists hswish_next_uid_holder_variable]} {
        set hswish_next_uid_holder_variable 0
    }
    incr hswish_next_uid_holder_variable
}

proc file_contents {filename} {
    set chan [open $filename "r"]
    set contents [read $chan [file size $filename]]
    close $chan
    set contents
}

proc filemap {infile outfile args} {
    set replaced_something 0
    set in_data [file_contents $infile]
    set map [list]
    foreach {string filename} $args {
        if {[string first $string $in_data] >= 0} {
            set replaced_something 1
        }
        lappend map $string [file_contents $filename]
    }
    if {$replaced_something} {
        set chan [open $outfile "w"]
        puts $chan "/* Auto-generated from $infile. Do not edit by hand. */"
        puts -nonewline $chan [string map $map $in_data]
        close $chan
    }
    return $replaced_something
}

proc generate_scan_code {id template_scan_file to_replace outfile} {
    foreach {ufile chan} [hs::tempfile "/tmp/" ".inc"] {}
    hs::Ntuple_pack_code $id $chan row_data 0
    close $chan
    set status [filemap $template_scan_file $outfile $to_replace $ufile]
    file delete $ufile
    return $status
}

# Search between [min, max], including both boundaries,
# in case min >= 0 and max >= min. If defaults are kept,
# take both min and max from data.
proc missing_uids {category {min -1} {max -1}} {
    set idlist [hs::list_items "" $category 1]
    set uids [list]
    foreach id $idlist {
        lappend uids [hs::uid $id]
    }
    set uids [lsort -integer -increasing $uids]
    set nfound [llength $uids]
    set missing [list]
    if {$nfound == 0} {
        if {$min >= 0 && $max >= $min} {
            for {set i $min} {$i <= $max} {incr i} {
                lappend missing $i
            }
        }
    } else {
        if {$min == -1} {
            set min [lindex $uids 0]
        }
        if {$max == -1} {
            set max [lindex $uids end]
        }
        if {$min >= 0 && $max >= $min} {
            for {set i $min} {$i <= $max} {incr i} {
                if {[lsearch -integer -sorted -increasing $uids $i] < 0} {
                    lappend missing $i
                }
            }
        }
    }
    return $missing
}
