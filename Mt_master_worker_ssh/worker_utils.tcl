
proc get_remote_row {host port remote_id row} {
    remote_eval $host $port pack_ntuple_row $remote_id $row
}

#######################################################################

proc get_remote_header {host port remote_id} {
    global current_workload_id
    set local_prefix "Workload $current_workload_id"
    set packed [remote_eval $host $port pack_ntuple_header $remote_id]
    hs::unpack_item $local_prefix $packed
}

#######################################################################
#
# It looks like, due to some reason, in the recent versions of tcl
# async signal handling happens in the interpreter which was running
# when the signal was received. Because of this, we need to forward
# the relevant command to a proper interpreter.
#
proc got_signal {sig} {
    global signal_handling_interp
    $signal_handling_interp eval got_signal $sig
}

#######################################################################

proc handle_signals {} {
    global signal_handling_interp

    # Setup minimal amount of code in the signal handling
    # interpreter so that it can respond to signals without
    # invoking anything in the parent interpreter
    global source_dir
    $signal_handling_interp eval [subst {
        source [list [file join $source_dir remote_eval.tcl]]
        global i_director_host
        set i_director_host [list [mw::director_host]]
        global i_director_port
        set i_director_port [list [mw::director_port]]
        global i_actor_id
        set i_actor_id [list [mw::actor_id]]
        global i_workload_stats
        set i_workload_stats {}
    }]

    foreach command {
        master_host
        master_port
    } {
        $signal_handling_interp alias $command $command
    }

    $signal_handling_interp eval {
        proc got_signal {sig} {
            global i_director_host i_director_port \
                    i_actor_id i_workload_stats
            set report_failure 0
            if {[catch {remote_eval [master_host] [master_port] \
                    worker_got_signal $i_actor_id $sig}]} {
                set report_failure 1
            }
            if {[catch {remote_eval $i_director_host $i_director_port \
                    unregister_worker $i_actor_id $i_workload_stats \
                    "got signal $sig"}]} {
                set report_failure 1
            }
            if {$report_failure} {
                puts stderr "Got signal $sig. Exiting."
            }
            exit 1
        }
        package require Signal
        foreach sig {
            SIGUSR1
            SIGUSR2
            SIGQUIT
            SIGINT
            SIGTERM
            SIGHUP
        } {
            signal add $sig [list got_signal $sig] -async
        }
    }

    return
}

#######################################################################

proc init_worker_globals {} {
    # We will maintain CPU statistics for current workload
    global workload_stats
    array unset workload_stats

    global current_project_id
    set current_project_id ""
    global current_workload_id
    set current_workload_id ""
    global current_remote_ntuple_id
    set current_remote_ntuple_id 0
    global local_ntuple_id
    set local_ntuple_id 0

    # Interpreter for handling signals
    global signal_handling_interp
    set signal_handling_interp [interp create]

    # Try to figure out the job deadline.
    # Give it a small offset hopefully sufficient for timeout reporting.
    global caf_deadline
    set caf_deadline [mw::myenv CAF_DEADLINE 0]
    if {$caf_deadline} {
        incr caf_deadline -60
    }

    global worker_last_time_check
    set worker_last_time_check [clock seconds]

    global worker_event_count
    set worker_event_count 0

    return
}

#######################################################################

proc worker_time_check {sec} {
    global caf_deadline
    if {$caf_deadline} {
        global worker_last_time_check
        set thistime [clock seconds]
        set deltat [expr {$thistime - $worker_last_time_check}]
        set worker_last_time_check $thistime
        if {$deltat <= 0} {
            set deltat 1
        }
        if {[expr {$thistime + 3*$deltat}] >= $caf_deadline} {
            set report_failure 0
            if {[catch {remote_eval [master_host] [master_port] \
                    worker_got_signal [mw::actor_id] "JOBTIMEOUT"}]} {
                set report_failure 1
            }
            if {[catch {
                remote_eval [mw::director_host] [mw::director_port] \
                    unregister_worker [mw::actor_id] {} \
                    "got signal JOBTIMEOUT"}]} {
                set report_failure 1
            }
            if {$report_failure} {
                puts stderr "Failed to report job timeout"
                exit 1
            } else {
                # Job time out is considered normal termination
                # if it is successfully reported
                exit 0
            }
        }
    }

    set max_events [mw::parameter worker_max_events]
    if {$max_events > 0} {
        global worker_event_count
        if {$worker_event_count >= $max_events} {
            set report_failure 0
            if {[catch {remote_eval [master_host] [master_port] \
                    worker_got_signal [mw::actor_id] "MAXEVENTS"}]} {
                set report_failure 1
            }
            if {[catch {
                remote_eval [mw::director_host] [mw::director_port] \
                    unregister_worker [mw::actor_id] {} \
                    "got signal MAXEVENTS"}]} {
                set report_failure 1
            }
            if {$report_failure} {
                puts stderr "Failed to report reaching max events limit"
                exit 1
            } else {
                # Reaching max events limit is considered normal termination
                # if it is successfully reported
                exit 0
            }
        }
    }

    after [expr {1000*$sec}] [list worker_time_check $sec]
    return
}

#######################################################################

proc set_master_address {host port} {
    mw::verify_port $port
    global master_address_KHGgcfYT
    set master_address_KHGgcfYT [list $host $port]
    return
}

#######################################################################

proc master_host {} {
    global master_address_KHGgcfYT
    lindex $master_address_KHGgcfYT 0
}

#######################################################################

proc master_port {} {
    global master_address_KHGgcfYT
    lindex $master_address_KHGgcfYT 1
}

#######################################################################

proc visit_director {worker_id} {
    while {1} {
        set director_reply [remote_eval [mw::director_host] \
                [mw::director_port] assign_master $worker_id]
        set command [lindex $director_reply 0]
        switch -exact -- $command {
            exit {
                remote_eval [mw::director_host] [mw::director_port] \
                    unregister_worker $worker_id [current_workload_stats] ok
                exit 0
            }
            wait {
                set wtime [lindex $director_reply 1]
                if {[string equal $wtime "forever"]} {
                    # Director requested us to block forever
                    # (until a job control signal comes)
                    remote_eval [mw::director_host] [mw::director_port] \
                        unregister_worker $worker_id [current_workload_stats] blocking
                    global signal_handling_interp
                    $signal_handling_interp eval {
                        proc got_signal {sig} {
                            puts stderr "Got signal $sig while blocked. Exiting."
                            exit 1
                        }
                    }
                    while {1} {
                        mw::wait 3600
                    }
                } else {
                    mw::wait $wtime
                }
            }
            work {
                return [lrange $director_reply 1 end]
            }
            default {
                error "Can't recognize director's command \"$director_reply\""
            }
        }
    }
}

#######################################################################

proc parse_directors_reply {directors_reply} {
    if {[llength $directors_reply] != 2} {
        error "Failed to parse director's reply \"$directors_reply\""
    }
    foreach {host port} $directors_reply break
    set_master_address $host $port
    return
}

#######################################################################

proc visit_master {worker_id} {
    while {1} {
        set master_reply [remote_eval [master_host] \
                [master_port] get_work_chunk $worker_id]
        set command [lindex $master_reply 0]
        switch -exact -- $command {
            return {
                return "return"
            }
            work {
                return [lrange $master_reply 1 end]
            }
            wait {
                mw::wait [lindex $master_reply 1]
            }
            default {
                error "Can't recognize masters's command \"$master_reply\""
            }
        }
    }
}

#######################################################################

proc cleanup_current_project {} {
    global current_project_id
    if {![string equal $current_project_id ""]} {
        # Project cleanup code goes here
        global current_project_interp current_project_dll
        if {[info exists current_project_dll]} {
            if {$current_project_dll >= 0} {
                interp eval $current_project_interp \
                    hs::sharedlib close $current_project_dll
            }
            unset current_project_dll
        }
        if {[info exists current_project_interp]} {
            interp delete $current_project_interp
            unset current_project_interp
        }
        global current_project_anacommand
        if {[info exists current_project_anacommand]} {
            unset current_project_anacommand
        }
        set current_project_id ""
    }
    return
}

#######################################################################

proc setup_project {worker_id project_id} {
    # This command should return a pair {interp analysis_command}
    # The analysis command should take ntuple id as an argument,
    # and return ntuple id as a result.
    global current_project_id
    if {![string equal $project_id $current_project_id]} {
        cleanup_current_project
        set current_project_id $project_id

        # Project initialization code goes here.
        # Go to the director and get the tcl and C
        # codes for the analysis. It would be easier
        # to go to the master, but then a malicious
        # master would be too much of a security
        # problem.
        global ::errorInfo
        set init_status [catch {
            global current_project_anacommand
            foreach {tcl_code current_project_anacommand c_code compile_flags} \
                [remote_eval [mw::director_host] [mw::director_port] \
                     project_code $worker_id $project_id] break

            # Create the interpreter which will run the job
            set parser [interp create]
            global current_project_interp
            set current_project_interp $parser

            # Forward the signal handling command to the
            # project interpreter
            $parser alias got_signal got_signal

            # Load the hs package into the parser
            $parser eval package require hs

            # Evaluate the tcl code
            $parser eval $tcl_code
            
            # Compile and load the C code
            global current_project_dll
            set current_project_dll -1
            if {![string is space $c_code]} {
                mw::validate_compile_flags $compile_flags
                foreach {cname chan} [hs::tempfile "/tmp/" ".c"] {}
                puts $chan $c_code
                close $chan
                foreach {soname chan} [hs::tempfile "/tmp/" \
                                      [info sharedlibextension]] {}
                close $chan
                set status [catch {
                    eval hs::sharedlib_compile $compile_flags \
                        [list $cname] [list $soname]
                    set current_project_dll [$parser eval \
                        hs::sharedlib open [list $soname]]
                } errmes]
                set savedInfo $::errorInfo
                catch {file delete $cname}
                catch {file delete $soname}
                if {$status} {
                    error $errmes $savedInfo
                }

                # Check if we got a signal
                mw::wait 0
            }
        } erra]
        set localInfo $::errorInfo
        if {$init_status} {
            cleanup_current_project
            error $erra $localInfo
        }

        # Check if we got a signal
        mw::wait 0
    }
    global current_project_interp current_project_anacommand
    list $current_project_interp $current_project_anacommand
}

#######################################################################

proc setup_local_items {remote_ntuple_id} {
    global local_ntuple_id current_remote_ntuple_id
    if {$remote_ntuple_id == $current_remote_ntuple_id && \
            $local_ntuple_id > 0} {
        return $local_ntuple_id
    }
    if {$local_ntuple_id > 0} {
        catch {hs::delete $local_ntuple_id}
        set local_ntuple_id 0
    }
    set local_ntuple_id [get_remote_header [master_host] \
                             [master_port] $remote_ntuple_id]
    set current_remote_ntuple_id $remote_ntuple_id
    return $local_ntuple_id
}

#######################################################################

proc cleanup_current_workload {} {
    global workload_stats
    array unset workload_stats
    global current_workload_id
    set current_workload_id ""
    global current_remote_ntuple_id
    set current_remote_ntuple_id 0
    global local_ntuple_id
    if {$local_ntuple_id > 0} {
        catch {hs::delete $local_ntuple_id}
        set local_ntuple_id 0
    }
    return
}

#######################################################################

proc current_workload_stats {} {
    global current_workload_id
    set result [list]
    if {[string compare $current_workload_id ""]} {
        global workload_stats
        lappend result $current_workload_id
        lappend result $workload_stats(nchunks)
        lappend result $workload_stats(cputime)
    }
    set result
}

#######################################################################

proc report_stats_and_cleanup {worker_id} {
    set stats [current_workload_stats]
    if {[llength $stats] > 0} {
        remote_eval [mw::director_host] [mw::director_port] \
            process_worker_stats $worker_id $stats
        cleanup_current_workload
    }
    return
}

#######################################################################

proc add_chunk_stats {cputime} {
    global workload_stats
    incr workload_stats(nchunks)
    incr workload_stats(cputime) $cputime
    return
}

#######################################################################

proc setup_workload {worker_id workload_id} {
    global current_workload_id
    if {![string equal $current_workload_id $workload_id]} {
        report_stats_and_cleanup $worker_id

        set current_workload_id $workload_id
        global workload_stats
        set workload_stats(nchunks) 0
        set workload_stats(cputime) 0
    }
    return
}

#######################################################################

proc work_with_master {worker_id} {
    # Work forever until master tells us to return
    while {1} {
        # Get a work chunk
        set chunk_description [visit_master $worker_id]
        # The chunk description should contain
        #   project_id          used for identification of the analysis code
        #   workload_id         used for accounting
        #   remote_ntuple_id    ntuple under analysis
        #   remote_row_number   row number to analyze
        #   uid title category  remote parameters for the result ntuple
        if {[string equal [lindex $chunk_description 0] "return"]} break

        # Parse the chunk description
        if {[llength $chunk_description] != 7} {
            error "Bad work chunk description \"$chunk_description\""
        }
        foreach {project_id workload_id \
                remote_ntuple_id remote_row_number \
                uid title category} $chunk_description break

        set starttime [clock seconds]

        global ::errorInfo
        set status [catch {
            # Setup the project and workload
            foreach {interp analysis_command} [setup_project $worker_id $project_id] {}
            setup_workload $worker_id $workload_id

            # Make sure we have an ntuple similar to the workload ntuple
            set nt_id [setup_local_items $remote_ntuple_id]
            hs::reset $nt_id

            # Get the data
            set packed_row_contents [get_remote_row [master_host] \
                    [master_port] $remote_ntuple_id $remote_row_number]
            hs::unpack_ntuple_row $nt_id $packed_row_contents

            # Evaluate the project code
            $interp eval $analysis_command $nt_id
        } result_id]
        set savedInfo $::errorInfo

        set cputime [expr {[clock seconds] - $starttime}]

        # Report the error to the master if necessary
        if {$status} {
            catch {
                remote_eval [master_host] [master_port] \
                        work_chunk_complete $worker_id \
                        $workload_id $remote_ntuple_id \
                        $remote_row_number $cputime error $result_id
            }
            error $result_id $savedInfo
        }

        # Return the evaluation result to the master
        set status [catch {
            hs::change_title $result_id $title
            hs::change_uid_and_category $result_id $uid $category
            remote_eval [master_host] [master_port] \
                    bintransmit work_chunk_complete $worker_id \
                    $workload_id $remote_ntuple_id $remote_row_number \
                    $cputime ok [hs::pack_item $result_id]
        } errmes]
        set savedInfo $::errorInfo
        catch {hs::delete $result_id}
        if {$status} {
            error $errmes $savedInfo
        }

        # Accumulate CPU statistics for this chunk
        add_chunk_stats $cputime

        # Update statistics in case the signal handler will need it
        global signal_handling_interp
        $signal_handling_interp eval [subst {
            global i_workload_stats
            set i_workload_stats [list [current_workload_stats]]
        }]

        # Increment the event count
        global worker_event_count
        incr worker_event_count

        # Check for signals
        mw::wait 0
    }
    return
}
