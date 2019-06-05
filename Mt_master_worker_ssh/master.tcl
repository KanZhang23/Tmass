#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Unfinished code: "exit_master_stats" in "master_utils.tcl".
# Corresponding proc "process_master_stats" in "director_utils.tcl".

# Load the utilities
set source_dir [file dirname [info script]]
source [file join $source_dir remote_eval.tcl]
source [file join $source_dir mw_utils.tcl]
source [file join $source_dir master_utils.tcl]
source [file join $source_dir allowed_hosts.tcl]
source [file join $source_dir server_utils_v8.tcl]
# source [file join $source_dir proctrace.tcl]
# proctrace on

# Check the input arguments
if {$argc != 2} {
    puts ""
    puts "Usage: [file tail [info script]] director_host director_port"
    puts ""
    exit 1
}
foreach {host port} $argv break

# What to do when we get a signal
proc got_signal {sig} {
    if {[catch {remote_eval [mw::director_host] [mw::director_port] \
                    unregister_master [mw::actor_id] \
                    [exit_master_stats] "got signal $sig"}]} {
        puts stderr "Got signal $sig. Exiting."
    }
    exit 1
}

# Initialize master configuration
init_master_globals

# Initialize periodic saving of the workload results
periodic_workload_save 3600

# Wrap the rest of the executable code in a big "catch"
# to report any error to the director
mw::set_actor master nameless_master_at_[info hostname]
global ::errorInfo
set estatus [catch {
    # Register as a master. In the process, we may be
    # rerouted to another director.
    set myjobid [mw::myenv LSB_JOBID]
    if {[string equal $myjobid ""]} {
	set myjobid [mw::myenv FBS_JOB_ID]
    }
    set master_id [mw::register $host $port register_master \
            [info hostname] 0 [mw::myenv USER] \
            [pid] [info script] [info nameofexecutable] \
            [mw::myenv LSB_JOBNAME] $myjobid]
    mw::set_actor master $master_id

    # Load the "hs" package
    package require hs
    hs::initialize "master $master_id"

    # Catch various signals
    package require Signal
    foreach sig {
        SIGUSR1
        SIGUSR2
        SIGQUIT
        SIGINT
        SIGTERM
    } {
        signal add $sig [list got_signal $sig]
    }

    # Some server parameters
    set maxclients [mw::parameter max_workers_per_master]
    set auto_logout_sec 300

    # Create the parser for processing commands
    set parser [interp create -safe]
    foreach command {
        get_work_chunk
        work_chunk_complete
        pack_ntuple_row
        pack_ntuple_header
        worker_got_signal
        save_workload_result
        report
    } {
        $parser alias $command $command
    }
    $parser alias id   ::mw::actor_id    
    $parser alias type ::mw::actor_type

    # Start the server
    server::start $master_id 0 $maxclients \
            $auto_logout_sec $allowed_hosts $parser 1
    set myport [server::cget $master_id -port]
    server::log_message "Master server is on [info hostname] $myport, pid [pid]"
    # server::config $master_id -monitor 1

    # Tell the director our port number
    remote_eval [mw::director_host] [mw::director_port] \
            set_master_port $master_id $myport

    # Process workloads forever, intil the director stops us
    # or until we get a signal
    set exit_request_received 0
    while {1} {
        global current_workload_ids
        set n_workloads [llength $current_workload_ids]

        if {!$exit_request_received} {
            if {$n_workloads < [mw::parameter max_workloads_per_master]} {
                set next_workload [get_next_workload $master_id]
                if {[string equal [lindex $next_workload 0] "exit"]} {
                    set exit_request_received 1
                } else {
                    if {[catch {setup_workload $next_workload}]} {
                        set einfo $::errorInfo
                        report_workload_failure $master_id $next_workload $einfo
                        continue
                    }
                }
            }
        }
        if {$n_workloads == 0 && $exit_request_received} {
            global main_cycle_finished
            set main_cycle_finished 1
            break
        }

        global request_next_workload
        set request_next_workload 0
        vwait request_next_workload
    }

    # Wait until we have no more associated workers
    if {[mw::object_count worker] > 0} {
        # After a short while, declare all workers
        # which are not in the waiting state AWOL
        after 100000 {
            global wait_timed_afterids
            foreach id [mw::object_list -type worker] {
                if {![info exists wait_timed_afterids($id)]} {
                    declare_worker_awol $id
                }
            }
        }

        # Force AWOL state on all remaining workers
        # after the standard chunk timeout
        after [expr {1000*[mw::parameter chunk_timeout_sec]}] {
            foreach id [mw::object_list -type worker] {
                declare_worker_awol $id
            }
        }

        # Initiate periodic job completion checks
        periodic_completion_check 120

        # Wait on a variable which signals that
        # we have no workers left
        global all_workers_returned
        set all_workers_returned 0
        vwait all_workers_returned
    } else {
        global all_workers_returned
        set all_workers_returned 1
    }
} outer_error]
set savedInfo $::errorInfo

# Check work completion status
if {$estatus} {
    # We are out of the work cycle due to some error condition
    set message "error $savedInfo"
    set exitStatus 1
} elseif {$all_workers_returned} {
    # Normal termination
    set message "ok"
    set exitStatus 0
} else {
    # We are out of the work cycle without an error condition,
    # but not all workers have disconnected
    set message "unexpected break out of the work cycle"
    set exitStatus 1
}

# Report work completion to the director
if {[catch {
    remote_eval [mw::director_host] [mw::director_port] \
            unregister_master $master_id [exit_master_stats] $message
} reporting_error]} {
    puts stderr "Failed to report work completion to the director\
            at [mw::director_host] [mw::director_port]:\
            $reporting_error"
    flush stderr
    if {$estatus} {
        error $outer_error $savedInfo
    } elseif {$all_workers_returned} {
        set exitStatus 1
    } else {
        error $message
    }
}

exit $exitStatus
