#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

# Load server utilities
set source_dir [file dirname [info script]]
source [file join $source_dir server_utils_v8.tcl]

# Some server parameters
set port 49999
set maxclients 1000
set auto_logout_sec 300
set local_clients_only 1

# Allowed hosts. This list is used only when "local_clients_only"
# variable defined above evaluates to boolean "false".
set allowed_hosts {}

# A special command understood by the server
proc id {} {
    return "Unique int server, v1.1"
}

# Compile the "nextint" command
package require hs
set sofile [file join $source_dir nextint.so]
hs::sharedlib_compile [file join $source_dir nextint.c] $sofile
hs::sharedlib open $sofile

# Parser for processing remote commands
set parser [interp create -safe]
foreach command {
    id
    nextint
} {
    $parser alias $command $command
}

# Start the server
set server_id {unique int server}
server::start $server_id $port $maxclients \
    $auto_logout_sec $allowed_hosts $parser $local_clients_only
server::log_message "Server host [info hostname], port [server::cget $server_id -port], pid [pid]"
server::config $server_id -logerrors 1

# Enter the event loop
vwait forever
exit 0
