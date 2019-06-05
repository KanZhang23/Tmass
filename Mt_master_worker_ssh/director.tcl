#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Load the utilities
set source_dir [file dirname [info script]]
source [file join $source_dir mw_utils.tcl]
source [file join $source_dir director_utils.tcl]
source [file join $source_dir allowed_hosts.tcl]
source [file join $source_dir server_utils_v8.tcl]
source [file join $source_dir remote_eval.tcl]
# source [file join $source_dir proctrace.tcl]
# proctrace on

# Check the input arguments
if {$argc < 3 || $argc > 5} {
    puts ""
    puts "Usage: [file tail [info script]] id project_config_file\
            logfile director_port? monitor_port?"
    puts ""
    exit 1
}
global director_id project_config_file logfile
foreach {director_id project_config_file logfile} $argv break
set dirport 0
if {$argc > 3} {
    set dirport [lindex $argv 3]
}
set monport 0
if {$argc > 4} {
    set monport [lindex $argv 4]
}

# Check that the configuration file exists. We will
# source it later using the monitoring parser.
if {[string compare $project_config_file "none"]} {
    if {![file readable $project_config_file]} {
        puts stderr "File \"$project_config_file\"\
                does not exists (or unreadable)"
        exit 1
    }
}

# Recognize myself
mw::set_actor director $director_id

# Configure the logger
switch -exact -- $logfile {
    stdout -
    stderr {
        set logchan $logfile
    }
    default {
        set logchan [open $logfile w]
    }
}
mw::config_logger all $logchan

# Redirect all server messages to the log file
proc server::log_message {message} {
    mw::log server {} $message
    return
}

# Initialize director's configuration
init_director_globals

# Some server parameters
set maxclients 1000000
set auto_logout_sec 300

# Parser for processing remote commands
set parser [interp create -safe]
foreach command {
    register_worker
    unregister_worker
    assign_master
    process_worker_stats
    project_code

    register_master
    unregister_master
    set_master_port
    assign_workload
    workload_complete
    report_released_worker
    report_awol_worker
    report_worker_return
} {
    $parser alias $command $command
}
$parser alias log  analyze_log
$parser alias id   ::mw::actor_id
$parser alias type ::mw::actor_type

# Start the server
server::start $director_id $dirport $maxclients \
    $auto_logout_sec $allowed_hosts $parser 1
server::log_message "Director server is on [info hostname]\
        [server::cget $director_id -port], pid [pid]"
# server::config $director_id -monitor 1

# Parser for the monitoring socket. On the monitoring socket
# we will enable all commands which allow us to examine
# the state of the program without changing this state,
# as well as the "log" command. "add_project" and "add_workload"
# commands are added temporarily so that we can evaluate
# the configuration file. These two commands will be removed
# after sourcing the configuration file -- they are too much
# of a security risk.
set parser [interp create -safe]
foreach command {
    add_project
    add_workload
    report
    redirect
    matching_workloads
    add_misconfig_patterns
    mw::parameter
    mw::actor
    mw::actor_id
    mw::actor_type
    mw::verify_port
    mw::director_host
    mw::director_port
    mw::verify_index
    mw::verify_id
    mw::root_id
    mw::branch_ids
    mw::count_branches
    mw::object_types
    mw::object_list
    mw::object_count
    mw::object_info
    mw::objects_created
    mw::typeof
    mw::cget
    mw::order_sequentially
    mw::average_load
    mw::uptime
} {
    $parser alias $command $command
}
$parser alias log    ::mw::log
$parser alias getenv ::mw::myenv

set monitor_id "$director_id monitor"
proc monitor_id {}   [list return $monitor_id]
proc monitor_type {} [list return "monitor"]
$parser alias id   monitor_id
$parser alias type monitor_type

# Turn on the monitoring socket
server::start $monitor_id $monport 5 0 $allowed_hosts $parser 1
server::log_message "Monitor port is [server::cget $monitor_id -port]"
server::config $monitor_id -logconnect 1

# Source the configuration file using the monitoring parser
if {[string compare $project_config_file "none"]} {
    $parser invokehidden source [list $project_config_file]
}

# Remove unsafe commands from the monitoring parser
$parser eval {
    rename add_project  {}
    rename add_workload {}
    rename getenv {}
}

# Enter the event loop
vwait forever
exit 0
