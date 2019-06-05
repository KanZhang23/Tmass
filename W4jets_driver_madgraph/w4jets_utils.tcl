proc init_random {} {
    global random_generator_init_seed
    if {![info exists random_generator_init_seed]} {
        if {[catch {
            set chan [open "/dev/urandom" "r"]
            binary scan [read $chan 4] "i1" seed
            close $chan
            set seed [expr {abs($seed)}]
            if {!$seed} {error "0 seed"}
        }]} {
            set seed "1"
            set mypid [pid]
            for {set i 0} {$i < 9} {incr i} {
                append seed [expr {([clock clicks] ^ $mypid) % 10}]
            }
        }
        hs::random_init $seed
        set random_generator_init_seed $seed
    }
    return $random_generator_init_seed
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
