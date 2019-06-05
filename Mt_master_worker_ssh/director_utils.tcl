
proc init_director_globals {} {
    global director_start_time
    set director_start_time [clock seconds]
    global test_job_completion_pending
    set test_job_completion_pending 0
    global current_redirect_state
    set current_redirect_state "late"
    global awol_worker_ids
    array unset awol_worker_ids
    global misconfigured_worker_ids
    array unset misconfigured_worker_ids
    global misconfiguration_patterns
    set misconfiguration_patterns [list]

    # Workers for which we have a pending timeout
    global worker_timeout_afterids
    array unset worker_timeout_afterids

    global redirect_info
    foreach who {master worker} {
        foreach when {early late} {
            set redirect_info($who,$when,host) "none"
            set redirect_info($who,$when,port) 0
        }
    }

    return
}

#######################################################################

proc normalize_redirect_who {who} {
    switch -exact -- $who {
        master -
        masters {
            return [list master]
        }
        worker -
        workers {
            return [list worker]
        }
        all {
            return [list master worker]
        }
        default {
            error "invalid redirect object \"$who\""
        }
    }
}

#######################################################################

proc normalize_redirect_when {when} {
    switch -exact -- $when {
        early {
            return [list early]
        }
        late {
            return [list late]
        }
        always {
            return [list early late]
        }
        default {
            error "invalid redirect time \"$when\""
        }
    }
}

#######################################################################

proc normalize_redirect_address {where} {
    switch [llength $where] {
        1 {
            set host [lindex $where 0]
            switch -exact -- $host {
                none {
                    set port 0
                }
                exit {
                    set port -1
                }
                default {
                    error "wrong redirect address"
                }
            }
        }
        2 {
            foreach {host port} $where break
            if {[string equal $host "none"] && [string equal $port "0"]} {
                # OK
            } elseif {[string equal $host "exit"] && [string equal $port "-1"]} {
                # OK
            } else {
                if {![server::is_safe_hostname $host]} {
                    error "invalid host name \"$host\""
                }
                mw::verify_port $port
            }
        }
        default {
            error "wrong redirect address"
        }
    }
    list $host $port
}

#######################################################################

proc redirect {who when args} {
    set wholist [normalize_redirect_who $who]
    set whenlist [normalize_redirect_when $when]

    global redirect_info
    if {[llength $args] == 0} {
        set result [list]
        foreach who $wholist {
            foreach when $whenlist {
                lappend result [list $who $when \
                        $redirect_info($who,$when,host) \
                        $redirect_info($who,$when,port)]
            }
        }
        return $result
    }

    foreach {host port} [normalize_redirect_address $args] break
    foreach who $wholist {
        foreach when $whenlist {
            set redirect_info($who,$when,host) $host
            set redirect_info($who,$when,port) $port
            mw::log info [mw::actor] "$who $when redirect is set to $host $port"
        }
    }
    global test_job_completion_pending
    if {!$test_job_completion_pending} {
        set test_job_completion_pending 1
        after idle test_job_completion
    }
    list $host $port
}

#######################################################################

proc add_project {args} {
    #
    # Usage: add_project id tclfiles anacommand cfiles? flags?
    #
    # Returns project id
    #
    set id [lindex $args 0]
    if {[string equal $id "auto"]} {
        set result [eval mw::create project [lrange $args 1 end]]
    } else {
        set result [eval mw::create_named_project $args]
    }
    set result
}

#######################################################################

proc add_workload {project_id args} {
    #
    # Usage: add_workload project_id host_patterns input_file ntuple_title \
    #                      ntuple_category result_category output_file
    #
    # Returns workload id
    #
    mw::verify_id project $project_id
    set workload_id [eval mw::create workload $args]
    mw::associate_one_to_many project $project_id workload $workload_id

    global current_redirect_state
    set current_redirect_state "early"

    return $workload_id
}

#######################################################################

proc register_worker {args} {
    #
    # Usage: register_worker hostname user pid script executable \
    #          jobname jobid
    #
    # Returns worker id, or redirects the worker to another director
    #
    global redirect_info current_redirect_state
    if {$redirect_info(worker,$current_redirect_state,port) > 0} {
        return [list redirect \
                $redirect_info(worker,$current_redirect_state,host) \
                $redirect_info(worker,$current_redirect_state,port)]
    }

    eval mw::create worker $args
}

#######################################################################

proc register_master {args} {
    #
    # Usage: register_master hostname port user pid script executable \
    #          jobname jobid
    #
    # Returns master id, or redirects the master to another director
    #
    global redirect_info current_redirect_state
    if {$redirect_info(master,$current_redirect_state,port) > 0} {
        return [list redirect \
                $redirect_info(master,$current_redirect_state,host) \
                $redirect_info(master,$current_redirect_state,port)]
    }

    eval mw::create master $args
}

#######################################################################

proc set_master_port {id port} {
    mw::verify_id master $id
    set oldport [mw::cget $id -port]
    if {$oldport != 0} {
        error "master \"$id\" port is already set to $oldport"
    }
    mw::config $id -port $port
    mw::log info [mw::actor] "master $id server is on\
            [mw::cget $id -hostname] $port"
    return
}

#######################################################################

proc matching_workloads {hostname state} {
    # set address [server::direct_ip_lookup $hostname]
    set idlist [list]
    # foreach id [mw::object_list -type workload] {
    #     if {[string equal $state [mw::cget $id state]]} {
    #         foreach pattern [mw::cget $id host_patterns] {
    #             if {[string match $pattern $hostname] || \
    # 			[string match $pattern $address]} {
    #                 lappend idlist [list $id [mw::cget $id seqnum]]
    #                 break
    #             }
    #         }
    #     }
    # }
    foreach id [mw::object_list -type workload] {
        if {[string equal $state [mw::cget $id state]]} {
            lappend idlist [list $id [mw::cget $id seqnum]]
        }
    }
    set wlist [list]
    foreach pair [lsort -integer -index 1 $idlist] {
        lappend wlist [lindex $pair 0]
    }
    set wlist
}

#######################################################################

proc is_worker_misconfigured {errmess} {
    global misconfiguration_patterns
    foreach pattern $misconfiguration_patterns {
        if {[string match $pattern $errmess]} {
            return 1
        }
    }
    return 0
}

#######################################################################

proc add_misconfig_patterns {args} {
    global misconfiguration_patterns
    foreach pattern $args {
        lappend misconfiguration_patterns $pattern
        lappend misconfiguration_patterns [string map {" " "\\\\ "} $pattern]
    }
    return
}

#######################################################################

proc analyze_log {type sender args} {
    # Log analysis code (and any other log augmentations)
    # must not generate exceptions
    set found_misconfigured_worker 0
    if {[catch {
        switch -- $type {
            error -
            Error -
            ERROR {
                if {[llength $sender] > 1} {
                    # Assume that $sender list was generated
                    # by mw::actor command in the sender
                    foreach {sender_type sender_id} $sender break
                } else {
                    set sender_id [lindex $sender 0]
                }
                if {[string equal [mw::typeof $sender_id] "worker"]} {
                    set message "$args"
                    if {[is_worker_misconfigured $message]} {
                        global misconfigured_worker_ids
                        set misconfigured_worker_ids($sender_id) 1
                        set found_misconfigured_worker 1
                    }
                }
            }
            default {}
        }
    } errmes]} {
        puts stderr "ERROR in analyze_log: $errmes"
    }
    eval mw::log [list $type] [list $sender] $args
    if {$found_misconfigured_worker} {
        mw::log warning [mw::actor] "worker $sender_id is misconfigured"
    }
    return
}

#######################################################################

proc assign_master {worker_id} {
    #
    # Returns a list which can take the following forms:
    #
    #     {work master_hostname master_port}
    #     {wait wait_interval_in_sec}
    #     {exit}
    #
    global awol_worker_ids
    if {[info exists awol_worker_ids($worker_id)]} {
        unset awol_worker_ids($worker_id)
        mw::restore $worker_id
    } else {
        mw::verify_id worker $worker_id
    }

    global worker_timeout_afterids
    if {[info exists worker_timeout_afterids($worker_id)]} {
        catch {after cancel $worker_timeout_afterids($worker_id)}
        unset worker_timeout_afterids($worker_id)
    }

    # Check if the exit redirect is set
    global redirect_info current_redirect_state
    if {$redirect_info(worker,$current_redirect_state,port) == -1} {
        mw::config $worker_id -state "dismissed"
        return [list exit]
    }
    
    # Check if this is one of the misconfigured workers
    global misconfigured_worker_ids
    if {[info exists misconfigured_worker_ids($worker_id)]} {
        mw::config $worker_id -state "dismissed"
        return [list wait forever]
    }
    
    # Figure out appropriate workloads being processed
    set workhost [mw::cget $worker_id hostname]
    set appropriate_workloads [matching_workloads $workhost "scheduled"]

    # Find out the relevant masters and the number of workers
    # they currently employ
    set masters [list]
    foreach load_id $appropriate_workloads {
        set master_id [lindex [mw::cget $load_id mlist] end]
        set n_master_workers [mw::count_branches master $master_id worker]
        lappend masters [list $master_id $n_master_workers]
    }

    # Find out the master with the smallest number of workers
    # and assign this worker to that master
    set masters [lsort -integer -index 1 $masters]
    if {[llength $masters]} {
        foreach {master_id nworkers} [lindex $masters 0] break
        if {$nworkers < [mw::parameter max_workers_per_master]} {
            mw::config $worker_id -state "assigned"
            mw::dissociate_this_branch worker $worker_id master
            mw::associate_one_to_many master $master_id worker $worker_id
            return [list work [mw::cget $master_id -hostname] \
                        [mw::cget $master_id -port]]
        }
    }

    # Figure out appropriate workloads not yet being processed
    set appropriate_workloads [matching_workloads $workhost "created"]
    if {[llength $appropriate_workloads] > 0} {
        mw::config $worker_id -state "waiting"
        return [list wait [mw::parameter worker_wait_interval]]
    }

    # No appropriate workloads left
    mw::config $worker_id -state "dismissed"
    return [list exit]
}

#######################################################################

proc load_master {master_id workload_id} {
    mw::config $workload_id -starttime [clock seconds]
    mw::config $workload_id -state "scheduled"

    variable ::mw::workload_info
    lappend ::mw::workload_info($workload_id,mlist) $master_id

    variable ::mw::master_info
    lappend ::mw::master_info($master_id,workloads) $workload_id

    set project_id [mw::root_id workload $workload_id project]
    after idle [list update_project_status $project_id]
    return
}

#######################################################################

proc unload_master {master_id workload_id new_load_state} {
    variable ::mw::master_info
    set workload_pos [lsearch -exact \
        $::mw::master_info($master_id,workloads) $workload_id]
    if {$workload_pos < 0} {
        error "Master $master_id is not working on workload $workload_id"
    }

    variable ::mw::workload_info
    set master_pos [lsearch -exact \
        $::mw::workload_info($workload_id,mlist) $master_id]
    if {$master_pos < 0} {
        error "Master $master_id has never worked on workload $workload_id"
    }

    set new_workload_list [lreplace $::mw::master_info($master_id,workloads) \
                               $workload_pos $workload_pos]
    set ::mw::master_info($master_id,workloads) $new_workload_list
    if {[llength $new_workload_list] == 0} {
        mw::config $master_id -state "created"
    }

    mw::config $workload_id -stoptime [clock seconds]
    mw::config $workload_id -state $new_load_state
    set project_id [mw::root_id workload $workload_id project]
    after idle [list update_project_status $project_id]
    return
}

#######################################################################

proc master_is_loaded {master_id} {
    variable ::mw::master_info
    if {[llength $::mw::master_info($master_id,workloads)] > 0} {
        return 1
    } else {
        return 0
    }
}

#######################################################################

proc assign_workload {master_id} {
    #
    # Returns a list which can take the following forms:
    #
    #     {work project_id workload_id input_file ntuple_title \
    #           ntuple_category result_category output_file}
    #     {wait wait_interval_in_sec}
    #     {exit}
    #
    mw::verify_id master $master_id

    # Check if the exit redirect is set
    global redirect_info current_redirect_state
    if {$redirect_info(master,$current_redirect_state,port) == -1} {
        if {![master_is_loaded $master_id]} {
            mw::config $master_id -state "dismissed"
        }
        return [list exit]
    }

    # Figure out a good workload. Note that checking the host name
    # for the master is not strictly necessary because the master
    # does not perform any processing itself. However, it still might
    # make sense that the master belongs to the same host group
    # as the workers.
    set masterhost [mw::cget $master_id hostname]
    set appropriate_workloads [matching_workloads $masterhost "created"]

    if {[llength $appropriate_workloads] == 0} {
        # No matching workloads. Tell the master to exit.
        if {![master_is_loaded $master_id]} {
            mw::config $master_id -state "dismissed"
        }
        return [list exit]
    }

    # Just take the first workload on which no masters worked before.
    # If there are no such workloads, take the one on which only
    # one master worked before, etc.
    set maxmasters [mw::parameter max_workload_tries]
    set workload_found 0
    for {set itry 0} {$itry < $maxmasters} {incr itry} {
        foreach workload_id $appropriate_workloads {
            set ntries [llength [mw::cget $workload_id mlist]]
            if {$ntries == $itry} {
                set workload_found 1
                break
            }
        }
        if {$workload_found} break
    }
    if {!$workload_found} {
        # Must have at least one workload, since "matching_workloads"
        # did not return an empty list
        error "Internal error, problem in workload accounting"
    }

    # Associate the master with the work load
    load_master $master_id $workload_id
    mw::config $master_id -state "assigned"

    set project_id [mw::root_id workload $workload_id project]
    list work $project_id $workload_id \
        [mw::cget $workload_id -input_file] \
        [mw::cget $workload_id -ntuple_title] \
        [mw::cget $workload_id -ntuple_category] \
        [mw::cget $workload_id -result_category] \
        [mw::cget $workload_id -output_file]
}

#######################################################################

proc workload_complete {master_id workload_id status nchunks failed cputime} {
    mw::verify_id master $master_id
    mw::verify_id workload $workload_id

    unload_master $master_id $workload_id $status

    mw::config $workload_id -nchunks $nchunks
    mw::config $workload_id -failed  $failed
    mw::config $workload_id -nfailed [llength $failed]
    mw::config $workload_id -cputime $cputime
    return
}

#######################################################################

proc report_worker_return {master_id worker_id} {
    mw::verify_id master $master_id

    global awol_worker_ids
    if {![info exists awol_worker_ids($worker_id)]} {
        error "Worker $worker_id is not awol,\
               wrong report by master $master_id"
    }
    unset awol_worker_ids($worker_id)
    mw::restore $worker_id
    mw::associate_one_to_many master $master_id worker $worker_id
    return
}

#######################################################################

proc report_released_worker {master_id worker_id} {
    mw::verify_id master $master_id
    if {[string equal [mw::root_id worker $worker_id master] $master_id]} {
        mw::dissociate_this_branch worker $worker_id master
        mw::config $worker_id -state "created"
    }
    return
}

#######################################################################

proc report_awol_worker {master_id worker_id} {
    mw::verify_id master $master_id
    if {![string equal [mw::root_id worker $worker_id master] $master_id]} {
        error "Worker $worker_id does not work for master $master_id"
    }

    mw::dissociate_this_branch worker $worker_id master
    mw::archive $worker_id
    mw::destroy $worker_id

    global awol_worker_ids
    set awol_worker_ids($worker_id) 1

    return
}

#######################################################################

proc bgerror {message} {
    global ::errorInfo
    set einf $::errorInfo
    mw::log error [mw::actor] "bgerror $message : $einf"
    return
}

#######################################################################

proc unregister_worker {worker_id worker_stats message} {
    process_worker_stats $worker_id $worker_stats

    if {![string equal $message "ok"]} {
        mw::log error [mw::actor] "worker $worker_id\
              unregistered with message \"$message\""
    }

    mw::destroy $worker_id

    global misconfigured_worker_ids
    if {[info exists misconfigured_worker_ids($worker_id)]} {
        unset misconfigured_worker_ids($worker_id)
    }

    global awol_worker_ids
    if {[info exists awol_worker_ids($worker_id)]} {
        unset awol_worker_ids($worker_id)
        mw::forget $worker_id
    }

    global worker_timeout_afterids
    if {[info exists worker_timeout_afterids($worker_id)]} {
        catch {after cancel $worker_timeout_afterids($worker_id)}
        unset worker_timeout_afterids($worker_id)
    }

    global test_job_completion_pending
    if {!$test_job_completion_pending} {
        set test_job_completion_pending 1
        after idle test_job_completion
    }

    return
}

#######################################################################

proc abort_workload {workload_id} {
    mw::verify_id workload $workload_id
    mw::config $workload_id -state "aborted"
    set message "Workload $workload_id aborted. Its properties were:"
    foreach property {
        host_patterns
        input_file
        ntuple_title
        ntuple_category
        result_category
        output_file
        mlist
        starttime
        stoptime
    } {
        append message "\n  $property  [mw::cget $workload_id $property]"
    }
    mw::log error [mw::actor] $message
    return
}

#######################################################################

proc unregister_master {master_id master_stats message} {
    process_master_stats $master_id $master_stats

    set master_timed_out 0
    if {![string equal $message "ok"]} {
        mw::log error [mw::actor] "master $master_id\
              unregistered with message \"$message\""

        # Check if the master got a signal
        if {[string equal -length 11 $message "got signal "]} {
            set sig [lindex $message 2]
            variable ::mw::job_timeout_signals
            if {[lsearch -exact $::mw::job_timeout_signals $sig] >= 0} {
                set master_timed_out 1
            }
        }
    }

    variable ::mw::master_info
    set workload_idlist $::mw::master_info($master_id,workloads)
    set master_had_a_workload [llength $workload_idlist]

    if {$master_had_a_workload} {
        if {$master_timed_out} {
            set new_workload_state "created"
        } else {
            set new_workload_state "error"
        }
        foreach workload $workload_idlist {
            unload_master $master_id $workload $new_workload_state
        }
    }
    mw::destroy $master_id

    # Check what happens to the master's workloads
    foreach workload_id $workload_idlist {
        set ntries [llength [mw::cget $workload_id mlist]]
        if {$ntries >= [mw::parameter max_workload_tries]} {
            abort_workload $workload_id
        }
    }

    global test_job_completion_pending
    if {!$test_job_completion_pending} {
        set test_job_completion_pending 1
        after idle test_job_completion
    }

    return
}

#######################################################################

proc process_worker_stats {worker_id worker_stats} {
    mw::verify_id worker $worker_id
    if {[llength $worker_stats]} {
        # Process the worker statistics
        if {[llength $worker_stats] != 3} {
            error "Invalid worker statistics format"
            foreach {workload_id nchunks cputime} $worker_stats break
            mw::add_worker_cputime $worker_id $workload_id $nchunks $cputime
        }
    }
    return
}

#######################################################################

proc process_master_stats {master_id master_stats} {
    mw::verify_id master $master_id
    if {[llength $master_stats]} {
        # Concept of master's statistics is not yet implemented
        error "Invalid master statistics format"
    }
    return
}

#######################################################################

proc project_code {worker_id project_id} {
    mw::verify_id worker $worker_id
    mw::verify_id project $project_id
    list [mw::cget $project_id tclcode] \
        [mw::cget $project_id anacommand] \
        [mw::cget $project_id ccode] \
        [mw::cget $project_id flags]
}

#######################################################################

proc tell_master_about_worker_signal {master_id worker_id sig} {
    # This proc is executed "after idle". Worker with id $worker_id 
    # no longer exists. It is conceivable that the master has quit as well.
    if {[catch {mw::verify_id master $master_id}]} return
    global ::errorInfo
    if {[catch {
        set host [mw::cget $master_id -hostname]
        set port [mw::cget $master_id -port]
        remote_eval $host $port worker_got_signal $worker_id $sig
    }]} {
        set savedInfo $::errorInfo
        mw::log error [mw::actor] $savedInfo
    }
    return
}

#######################################################################

proc test_job_completion {} {
    # Test if we can exit. We can do so if
    #  -- all masters and workers have unregistered
    #  -- all workloads are done
    #  -- no "late" redirects are set

    # Awoid being called again from inside "update_project_status".
    # Do the updates before resetting "test_job_completion_pending".
    foreach project_id [mw::object_list -type project] {
        update_project_status $project_id
    }

    global test_job_completion_pending
    set test_job_completion_pending 0

    if {[llength [mw::object_list -type project -state "created"]] > 0} return
    if {[llength [mw::object_list -type project -state "scheduled"]] > 0} return

    global current_redirect_state
    set current_redirect_state "late"

    if {[mw::object_count master] > 0} return
    if {[mw::object_count worker] > 0} {
        # Temporary solution for the problem of persistent director.
        # Needed because a worker job may be killed after we send
        # it to the master but before it gets there.
        set timeout_msec [expr {1000*[mw::parameter chunk_timeout_sec]}]
        global worker_timeout_afterids
        foreach worker_id [mw::object_list -type worker] {
            if {![info exists worker_timeout_afterids($worker_id)]} {
                set worker_timeout_afterids($worker_id) \
                    [after $timeout_msec [list worker_timed_out $worker_id]]
            }
        }
        return
    }

    global redirect_info
    foreach index [array names redirect_info "*,late,port"] {
        if {$redirect_info($index)} return
    }

    mw::log info [mw::actor] [report]
    exit 0
}

#######################################################################

proc worker_timed_out {worker_id} {
    if {[catch {mw::verify_id worker $worker_id}]} return
    unregister_worker $worker_id {} "timed out by director"
    return
}

#######################################################################

proc update_project_status {project_id} {
    #
    # Go over the workloads and check the status. Rules for setting
    # the project status are:
    #
    # If all workloads have status "created" then the project status
    # will be "created".
    #
    # Else if there is at least one workload with status "created"
    # or "scheduled" then the project status will be "scheduled".
    #
    # Else if there is at least one workload with status not
    # equal to "completed" then the project status will be "error".
    #
    set all_workloads [mw::branch_ids project $project_id workload]
    set nworkloads [llength $all_workloads]
    set ncreated   0
    set nscheduled 0
    set ncompleted 0
    set nother     0
    foreach workload_id $all_workloads {
        switch -exact -- [mw::cget $workload_id -state] {
            created {
                incr ncreated
            }
            scheduled {
                incr nscheduled
            }
            completed {
                incr ncompleted
            }
            default {
                incr nother
            }
        }
    }

    set oldstate [mw::cget $project_id -state]
    set starttime [mw::cget $project_id -starttime]
    set stoptime [mw::cget $project_id -stoptime]

    if {$ncreated == $nworkloads} {
        set newstate "created"
    } elseif {[expr {$ncreated + $nscheduled}] > 0} {
        set newstate "scheduled"
        if {!$starttime} {
            mw::config $project_id -starttime [clock seconds]
        }
    } elseif {$ncompleted == $nworkloads} {
        set newstate "completed"
        if {!$stoptime} {
            mw::config $project_id -stoptime [clock seconds]
        }
    } else {
        set newstate "error"
        if {!$stoptime} {
            mw::config $project_id -stoptime [clock seconds]
        }
    }

    if {[string compare $oldstate $newstate]} {
        mw::config $project_id -state $newstate
        if {[string compare $newstate "created"] && \
                [string compare $newstate "scheduled"]} {
            global test_job_completion_pending
            if {!$test_job_completion_pending} {
                set test_job_completion_pending 1
                after idle test_job_completion
            }
        }
    }

    return
}

#######################################################################

proc report {{depth 100}} {
    set report ""
    append report "\n####"
    append report "\n####  [mw::actor] status report"
    append report "\n####"

    global director_start_time awol_worker_ids misconfigured_worker_ids
    append report "\n" "started      : " [mw::time_string $director_start_time]
    append report "\n" "pid          : " [pid]
    append report "\n" "uptime       : " [mw::uptime $director_start_time]
    append report "\n" "server port  : " [server::cget [mw::actor_id] -port]
    append report "\n" "n projects   : " [mw::object_count project]
    append report "\n" "n workloads  : " [mw::object_count workload]
    append report "\n" "n masters    : " [mw::object_count master]
    append report "\n" "ave masters  : " [format {%g} [mw::average_load master]]
    append report "\n" "n workers    : " [mw::object_count worker]
    append report "\n" "ave workers  : " [format {%g} [mw::average_load worker]]
    set n_awol_workers [llength [array names awol_worker_ids]]
    append report "\n" "awol workers : " $n_awol_workers
    append report "\n" "misconfigured: " [array names misconfigured_worker_ids]

    global redirect_info
    append report "\n\nRedirects are set as follows"
    foreach obj [list master worker] {
        foreach when [list early late] {
            set index "$obj,$when"
            append report "\n" [format "%-12s" $index] " : " $redirect_info($index,host)
            if {$redirect_info($index,port)} {
                append report " " $redirect_info($index,port)
            }
        }
    }

    if {$depth > 0} {
        foreach project_id [mw::order_sequentially \
                project [mw::object_list -type project]] {
            append report "\n\n" [mw::object_info $project_id]
            set all_workloads [mw::order_sequentially workload \
                    [mw::branch_ids project $project_id workload]]
            if {$depth > 1} {
                foreach workload_id $all_workloads {
                    append report "\n\n" [mw::object_info $workload_id]
                    set all_masters [mw::order_sequentially master \
                            [mw::branch_ids workload $workload_id master]]
                    if {$depth > 2} {
                        foreach master_id $all_masters {
                            append report "\n\n" [mw::object_info $master_id]
                            set all_workers [mw::order_sequentially worker \
                                    [mw::branch_ids master $master_id worker]]
                            if {$depth > 3} {
                                foreach worker_id $all_workers {
                                    append report "\n\n" [mw::object_info $worker_id]
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if {$depth > 3 && $n_awol_workers > 0} {
        append report "\n\nAWOL worker hosts and job ids are listed below:\n"
        global awol_worker_ids
        foreach id [array names awol_worker_ids] {
            set jobid [mw::recall $id jobid]
            if {[llength $jobid] == 0} {
                set jobid [list [list]]
            }
            append report "\n$id  [mw::recall $id hostname]  $jobid"
        }
    }

    append report "\n"
    set report
}

