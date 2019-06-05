proc job_queue_gotline {iscript chan print_lines} {
    if {[gets $chan nextline] < 0} {
        # This command is completed
        global job_queue_results job_queue_status job_queue_errors
        if {[catch {close $chan} err]} {
            # This script has failed
            set job_queue_status($iscript) 1
            set job_queue_errors($iscript) $err
        } else {
            set job_queue_status($iscript) 0
            set job_queue_errors($iscript) ""
        }
        global job_queue_scripts job_queue_iscript
        set nscripts [llength $job_queue_scripts]
        if {$job_queue_iscript < $nscripts} {
            # Submit the next script
            set script [lindex $job_queue_scripts $job_queue_iscript]
            set chan [open "|$script" r]
            fconfigure $chan -buffering line
            fileevent $chan readable [list job_queue_gotline \
                $job_queue_iscript $chan $print_lines]
            set job_queue_results($job_queue_iscript) ""
            set job_queue_status($job_queue_iscript) -1
            incr job_queue_iscript
        } else {
            # Check for completion
            set complete 1
            for {set iscript 0} {$iscript < $nscripts} {incr iscript} {
                if {$job_queue_status($iscript) < 0} {
                    set complete 0
                    break
                }
            }
            if {$complete} {
                global job_queue_waitvar
                set job_queue_waitvar 1
            }
        }
    } else {
        global job_queue_results
        append job_queue_results($iscript) $nextline
        if {$print_lines} {
            puts "job $iscript : $nextline"
        }
    }
    return
}

proc job_queue {queue_scripts n_simultaneous {print_lines 0}} {
    global job_queue_scripts
    set job_queue_scripts $queue_scripts
    set nscripts [llength $queue_scripts]
    if {$nscripts < $n_simultaneous} {
        set maxlen $nscripts
    } else {
        set maxlen $n_simultaneous
    }
    global job_queue_results job_queue_status job_queue_errors
    array unset job_queue_results
    array unset job_queue_status
    array unset job_queue_errors
    for {set iscript 0} {$iscript < $maxlen} {incr iscript} {
        set script [lindex $queue_scripts $iscript]
        set chan [open "|$script" r]
        fconfigure $chan -buffering line
        fileevent $chan readable [list job_queue_gotline\
            $iscript $chan $print_lines]
        set job_queue_results($iscript) ""
        set job_queue_status($iscript) -1
    }
    global job_queue_iscript
    set job_queue_iscript $iscript
    global job_queue_waitvar
    set job_queue_waitvar 0
    vwait job_queue_waitvar
    set result [list]
    for {set iscript 0} {$iscript < $nscripts} {incr iscript} {
        lappend result $job_queue_status($iscript) \
            $job_queue_results($iscript) \
            $job_queue_errors($iscript)
    }
    return $result
}

proc n_cpus_linux {} {
    global n_cpus_linux_found
    if {![info exists n_cpus_linux_found]} {
        set n_cpus_linux_found 0
        set chan [open "/proc/cpuinfo" r]
        while {[gets $chan l] >= 0} {
            if {[string equal -length 9 $l "processor"]} {
                incr n_cpus_linux_found
            }
        }
        close $chan
    }
    return $n_cpus_linux_found
}

proc parallel_by_last_arg {script lastarg_list {print_lines 0}} {
    set scriptlist [list]
    foreach lastarg $lastarg_list {
        lappend scriptlist [concat $script $lastarg]
    }
    job_queue $scriptlist [max_parallel_jobs] $print_lines
}

proc max_parallel_jobs {{value 0}} {
    global job_queue_max_parallel_jobs
    if {$value > 0} {
        set job_queue_max_parallel_jobs $value
    } elseif {![info exists job_queue_max_parallel_jobs]} {
        set job_queue_max_parallel_jobs [n_cpus_linux]
    }
    set job_queue_max_parallel_jobs
}

proc check_status {job_queue_results} {
    foreach {status result err} $job_queue_results {
        if {$status} {
            error $err
        }
    }
    return
}
