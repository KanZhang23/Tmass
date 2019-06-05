#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Check the input arguments
if {$argc != 3 && $argc != 2} {
    puts ""
    puts "Usage: [file tail [info script]] director_host director_port \[n_workers\]"
    puts ""
    exit 1
}
foreach {host port} $argv break
if {$argc > 2} {
    set nworkers [lindex $argv 2]
} else {
    set nworkers 0
}

global source_dir
set source_dir [file dirname [info script]]
source [file join $source_dir job_queue.tcl]

if {$nworkers <= 0} {
    set nworkers [n_cpus_linux]
}

set worker_exe [file join $source_dir worker.tcl]
set scripts [list]
for {set i 0} {$i < $nworkers} {incr i} {
    lappend scripts "$worker_exe $host $port"
}

set job_results [job_queue $scripts $nworkers 1]

if {[catch {check_status $job_results} errmess]} {
    puts stderr $errmess
    exit 1
} else {
    exit 0
}
