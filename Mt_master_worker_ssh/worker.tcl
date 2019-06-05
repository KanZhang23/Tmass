#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

#  Remote commands used, and server
#
#  register_worker       director
#  unregister_worker     director
#  assign_master         director
#  project_code          director
#  process_worker_stats  director
#  log                   director
#
#  pack_ntuple_header    master
#  pack_ntuple_row       master
#  get_work_chunk        master
#  work_chunk_complete   master
#
#  Commands executable remotely: none

# Load the utilities
global source_dir
set source_dir [file dirname [info script]]
source [file join $source_dir mw_utils.tcl]
source [file join $source_dir worker_utils.tcl]
source [file join $source_dir remote_eval.tcl]
# source [file join $source_dir proctrace.tcl]
# proctrace on

# Check the input arguments
if {!($argc == 2 || $argc == 3)} {
    puts ""
    puts "Usage: [file tail [info script]] director_host director_port maxevents?"
    puts ""
    exit 1
}
foreach {host port} $argv break
if {$argc > 2} {
    mw::set_parameter worker_max_events [expr {round([lindex $argv 2])}]
}

# Check that the compiler exists.
# Block this host if it doesn't, otherwise
# this host will quickly generate many errors.
# set cc_name "gcc"
# set cc_path [::auto_execok $cc_name]
# while {[string equal $cc_path ""]} {
#    after 1000
#    set cc_path [::auto_execok $cc_name]
# }

# Initialize the overall work accounting
init_worker_globals

# Wrap the rest of the executable code in a big "catch"
# to report any error to the director
mw::set_actor worker nameless_worker_at_[info hostname]
global ::errorInfo
set estatus [catch {
    # Register as a worker. In the process, we may be
    # rerouted to another director.
    set myjobid [mw::myenv LSB_JOBID]
    if {[string equal $myjobid ""]} {
	set myjobid [mw::myenv FBS_JOB_ID]
    }
    set worker_id [mw::register $host $port register_worker \
            [info hostname] [mw::myenv USER] \
            [pid] [info script] [info nameofexecutable] \
            [mw::myenv LSB_JOBNAME] $myjobid]
    mw::set_actor worker $worker_id

    # Load the "hs" package
    package require hs
    hs::initialize "worker $worker_id"

    # Catch various signals
    handle_signals

    # Check for job timeout every 15 seconds
    worker_time_check 15

    # Work forever, intil the director stops us or until we get a signal
    while {1} {
        parse_directors_reply [visit_director $worker_id]
        if {[catch {work_with_master $worker_id}]} {
            set einfo $::errorInfo
            remote_eval [mw::director_host] [mw::director_port] \
                log error [mw::actor] $einfo
        }

        # Check for signals
        mw::wait 0
    }
} outer_error]
set savedInfo $::errorInfo

# We are out of the work cycle. This should not normally happen.
if {$estatus} {
    set message "error $savedInfo"
} else {
    set message "unexpected break out of the work cycle"
}

# Report the problem to the director
if {[catch {
    remote_eval [mw::director_host] [mw::director_port] \
        unregister_worker $worker_id \
        [current_workload_stats] $message
} reporting_error]} {
    puts stderr "Failed to report an error to the director\
                 at [mw::director_host] [mw::director_port]:\
                 $reporting_error"
    flush stderr
    if {$estatus} {
        error $outer_error $savedInfo
    } else {
        error $message
    }
}

exit 1
