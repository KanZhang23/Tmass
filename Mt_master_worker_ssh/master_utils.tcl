
proc init_master_globals {} {
    # This procedure initializes all global 
    # variables pertaining to master
    global master_start_time
    set master_start_time [clock seconds]

    # The list of current workload ids
    global current_workload_ids
    set current_workload_ids [list]

    # Variable to weight upon in the main cycle
    global request_next_workload
    set request_next_workload 1

    # Variable to wait upon if the director tells
    # us to wait
    global get_next_workload_waitvariable
    set get_next_workload_waitvariable 0

    # Variable to wait upon when we are waiting for
    # all workers to go back to the director, after
    # the director told us to exit
    global all_workers_returned
    set all_workers_returned 0

    # Variable which will be set to 1 after
    # the director told us to exit
    global main_cycle_finished
    set main_cycle_finished 0

    # Workers which went AWOL
    global awol_worker_ids
    array unset awol_worker_ids

    # Workers for which we have a pending wait timer
    global wait_timed_afterids
    array unset wait_timed_afterids

    global workload_completion_check_pending
    array unset workload_completion_check_pending

    global n_workloads_completed
    set n_workloads_completed 0

    global n_workloads_success
    set n_workloads_success 0

    # Variables which describe the workloads
    global workload_attributes
    set workload_attributes {project_id workload_id input_file \
            ntuple_title ntuple_category result_category output_file}

    global rows_currently_processed
    array unset rows_currently_processed

    return
}

#######################################################################

proc pack_ntuple_row {id row} {
    list bintransmit [hs::pack_ntuple_row $id $row]
}

#######################################################################

proc pack_ntuple_header {id} {
    set newcateg ""
    set new_id [hs::duplicate_ntuple_header $id \
                    [hs::next_category_uid $newcateg] \
                    [hs::title $id] $newcateg]
    set bstring [hs::pack_item $new_id]
    hs::delete $new_id
    list bintransmit $bstring
}

#######################################################################

proc get_next_workload {master_id} {
    while {1} {
        set director_reply [remote_eval [mw::director_host] \
                [mw::director_port] assign_workload $master_id]
        set command [lindex $director_reply 0]
        switch -exact -- $command {
            exit {
                return "exit"
            }
            wait {
                set sec [lindex $director_reply 1]
                if {$sec > 0} {
                    global get_next_workload_waitvariable
                    set get_next_workload_waitvariable \
                            [expr {[clock seconds] + $sec}]
                    after [expr {$sec * 1000}] {
                        global get_next_workload_waitvariable
                        set get_next_workload_waitvariable 0
                    }
                    vwait get_next_workload_waitvariable
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

proc fetch_file {url} {
    set splitlist [split $url :]
    set protocol [lindex $splitlist 0]
    set remainder [join [lrange $splitlist 1 end] :]
    switch -exact -- $protocol {
        ftp -
        http {
            foreach {filename chan} [hs::tempfile "/tmp/" ".hs"] break
            close $chan
            exec wget -q -O $filename $url
            return [list $filename 1]
        }
        rcp -
        scp {
            foreach {filename chan} [hs::tempfile "/tmp/" ".hs"] break
            close $chan
            exec $protocol $remainder $filename
            return [list $filename 1]
        }
        file {
            # This should be a file name on the local machine
            return [list $remainder 0]
        }
        default {
            # This should be a file name on the local machine
            return [list $url 0]
        }
    }
    error "Missing return in [lindex [info level 0] 0]"
}

#######################################################################

proc setup_workload {parlist} {
    global workload_attributes
    if {[llength $workload_attributes] != [llength $parlist]} {
        error "Failed to parse workload specification"
    }
    foreach name $workload_attributes value $parlist {
        set $name $value
    }

    # Create the workload object
    mw::create_named_workload $workload_id {} $input_file \
        $ntuple_title $ntuple_category $result_category $output_file
    variable ::mw::workload_info
    set ::mw::workload_info($workload_id,project_id) $project_id

    # Check the validity of the input file
    foreach {infile istemp} [fetch_file $input_file] break
    if {![file readable $infile]} {
        error "File \"$infile\" does not exist (or unreadable)"
    }

    # Make sure to remove the temporary file
    # after reading in the workload ntuple
    global ::errorInfo
    set status [catch {
        set uids [list]
        foreach item [hs::Dir_uncached $infile] {
            foreach {uid category title type} $item break
            if {[string equal $title $ntuple_title] && \
                    [string equal $category $ntuple_category] && \
                    [string equal $type "HS_NTUPLE"]} {
                lappend uids $uid
                break
            }
        }
        if {[llength $uids] == 0} {
            error "Ntuple with title \"$ntuple_title\" and category\
                    \"$ntuple_category\" is not present in file\
                    \"$input_file\""
        }
        set prefix "WL Input $workload_id"
        if {[hs::read_file_items $infile \
                $prefix $ntuple_category $uids] != 1} {
            error "Failed to read ntuple with title \"$ntuple_title\"\
                    and category \"$ntuple_category\" from file\
                    \"$input_file\""
        }
    } erra]
    set savedInfo $::errorInfo
    if {$istemp} {
        file delete $infile
    }
    if {$status} {
        error $erra $savedInfo
    }

    set newcateg [file join $prefix $ntuple_category]
    set workload_ntuple_id [hs::id_from_title $ntuple_title $newcateg]
    if {$workload_ntuple_id <= 0} {
        error "Internal error: can't find loaded ntuple"
    }
    set ::mw::workload_info($workload_id,ntuple_id) $workload_ntuple_id
    set workload_ntuple_nrows [hs::num_entries $workload_ntuple_id]
    set ::mw::workload_info($workload_id,ntuple_nrows) $workload_ntuple_nrows
    set ::mw::workload_info($workload_id,first_worker_arrived) 0
    set ::mw::workload_info($workload_id,n_saved_items) 0

    set maxpass [mw::parameter max_chunk_tries]
    for {set ipass 0} {$ipass < $maxpass} {incr ipass} {
        set ::mw::workload_info($workload_id,passlist_index,$ipass) 0
        set ::mw::workload_info($workload_id,rows_in_pass,$ipass) [list]
    }
    for {set row 0} {$row < $workload_ntuple_nrows} {incr row} {
        foreach {property value} {
            state     "init"
            ipass     0
            worker    ""
            started   0
            finished  0
            afterid   ""
            resultid  0
        } {
            set ::mw::workload_info($workload_id,row_info,$row,$property) $value
        }
        lappend ::mw::workload_info($workload_id,rows_in_pass,0) $row
    }
    set ::mw::workload_info($workload_id,starttime) [clock seconds]

    global workload_completion_check_pending
    set workload_completion_check_pending($workload_id) 0

    global current_workload_ids
    lappend current_workload_ids $workload_id
    return
}

#######################################################################

proc bgerror {message} {
    global ::errorInfo
    set einf $::errorInfo
    remote_eval [mw::director_host] [mw::director_port] \
        log error [mw::actor] "bgerror $message : $einf"
    return
}

#######################################################################

proc cleanup_workload {workload_id} {
    global current_workload_ids
    set pos [lsearch -exact $current_workload_ids $workload_id]
    if {$pos >= 0} {
        hs::delete_category "WL Input $workload_id/..."
        hs::delete_category "WL Output $workload_id/..."
        mw::destroy $workload_id
        global rows_currently_processed
        array unset rows_currently_processed "$workload_id,*"
        global workload_completion_check_pending
        unset workload_completion_check_pending($workload_id)
        set current_workload_ids [lreplace $current_workload_ids $pos $pos]
    }
    return
}

#######################################################################

proc get_work_chunk {worker_id} {
    global wait_timed_afterids
    if {[info exists wait_timed_afterids($worker_id)]} {
        catch {after cancel $wait_timed_afterids($worker_id)}
        unset wait_timed_afterids($worker_id)
    }
    if {[catch {mw::verify_id worker $worker_id}]} {
        # This is a new worker id
        mw::create_named_worker $worker_id {} {} 0 {} {} {} {}
        global awol_worker_ids
        if {[info exists awol_worker_ids($worker_id)]} {
            unset awol_worker_ids($worker_id)
            remote_eval [mw::director_host] [mw::director_port] \
                    report_worker_return [mw::actor_id] $worker_id
        }
    } else {
        # This is an old worker
        if {[mw::cget $worker_id -row] >= 0} {
            # In the current system, a worker can not work
            # on more than one row. Most probably, we are
            # in a situation where the worker was unable
            # to return us the result of his previous
            # calculation because of some network-related
            # problem.
            work_chunk_failure $worker_id "problem"
            mw::config $worker_id -row -1
            mw::config $worker_id -workload ""
        }
    }

    global main_cycle_finished
    if {$main_cycle_finished} {
        mw::destroy $worker_id
        set result [list return]
    } else {
        set maxpass [mw::parameter max_chunk_tries]
        set chunk_found 0
        variable ::mw::workload_info
        global current_workload_ids
        foreach workload_id $current_workload_ids {
            for {set ipass 0} {$ipass < $maxpass} {incr ipass} {
                set n_pass_rows [llength $::mw::workload_info($workload_id,rows_in_pass,$ipass)]
                for {} {$::mw::workload_info($workload_id,passlist_index,$ipass) < $n_pass_rows} \
                    {incr ::mw::workload_info($workload_id,passlist_index,$ipass)} {
                    set row [lindex $::mw::workload_info($workload_id,rows_in_pass,$ipass) \
                                 $::mw::workload_info($workload_id,passlist_index,$ipass)]
                    set state [get_chunk_property $workload_id $row state]
                    if {[string compare $state "completed"]} {
                        if {[string equal $state "assigned"]} {
                            error "Bug in the chunk processing sequence"
                        }
                        set chunk_found 1
                        break
                    }
                }
                if {$chunk_found} break
            }
            if {$chunk_found} break
        }

        if {$chunk_found} {
            incr ::mw::workload_info($workload_id,passlist_index,$ipass)
            global rows_currently_processed
            set rows_currently_processed($workload_id,$row) 1
            set thistime [clock seconds]
            set timeout_msec [expr {1000*[mw::parameter chunk_timeout_sec]}]
            set afterid [after $timeout_msec [list work_chunk_expired $worker_id]]
            set_chunk_property $workload_id $row state "assigned"
            set_chunk_property $workload_id $row ipass $ipass
            set_chunk_property $workload_id $row worker $worker_id
            set_chunk_property $workload_id $row started $thistime
            set_chunk_property $workload_id $row finished 0
            set_chunk_property $workload_id $row afterid $afterid
            mw::config $worker_id -row $row
            mw::config $worker_id -state "assigned"
            mw::config $worker_id -workload $workload_id
            set first_worker_arrived [mw::cget $workload_id -first_worker_arrived]
            if {$first_worker_arrived == 0} {
                mw::config $workload_id -first_worker_arrived $thistime
            }
            set current_ntuple_title [mw::cget $workload_id -ntuple_title]
            set title "Result for row $row of $current_ntuple_title"
            set result [list work \
                [mw::cget $workload_id -project_id] \
                $workload_id \
                [mw::cget $workload_id -ntuple_id] \
                $row $row $title \
                [mw::cget $workload_id -result_category]]
        } else {
            # Is master itself waiting for a workload
            # from the director? If so, schedule the worker
            # to return a little bit after master's next
            # workload request will be complete.
            global get_next_workload_waitvariable
            if {$get_next_workload_waitvariable} {
                # Waiting for a workload from the director
                set thistime [clock seconds]
                set waittime [expr {$get_next_workload_waitvariable - $thistime + 2}]
            } else {
                # Waiting for completion of the current workload.
                # Set the waiting time in proportion to the number
                # of connected workers, so that we do not drown
                # in their requests.
                set waittime [mw::object_count worker]
            }
            if {$waittime <= 0} {
                set waittime 1
            }
            mw::config $worker_id -state "waiting"
            set result [list wait $waittime]
            set timeout_msec [expr {1000*(2*$waittime + 300)}]
            global wait_timed_afterids
            set wait_timed_afterids($worker_id) \
                    [after $timeout_msec [list worker_wait_expired $worker_id]]

            # Are we waiting for the workload request
            # in the main cycle? If so, raise the request.
            global request_next_workload
            if {$request_next_workload == 0} {
                after idle {
                    global request_next_workload
                    set request_next_workload 1
                }
            }
        }
    }

    if {$main_cycle_finished} {
        if {[mw::object_count worker] == 0} {
            after idle {
                global all_workers_returned
                set all_workers_returned 1
            }
        }
    }

    set result
}

#######################################################################

proc get_chunk_property {workload_id row property} {
    variable ::mw::workload_info
    return $::mw::workload_info($workload_id,row_info,$row,$property)
}

#######################################################################

proc remove_from_future_processing {workload_id row} {
    set maxpass [mw::parameter max_chunk_tries]
    for {set ipass 1} {$ipass < $maxpass} {incr ipass} {
        set inpass [lsearch -exact \
                        $::mw::workload_info($workload_id,rows_in_pass,$ipass) $row]
        if {$inpass >= 0} {
            if {$inpass >= $::mw::workload_info($workload_id,passlist_index,$ipass)} {
                set ::mw::workload_info($workload_id,rows_in_pass,$ipass) \
                    [lreplace $::mw::workload_info($workload_id,rows_in_pass,$ipass) \
                         $inpass $inpass]
            }
        }
    }
    return
}

#######################################################################

proc set_chunk_property {workload_id row property value} {
    variable ::mw::workload_info
    if {[info exists ::mw::workload_info($workload_id,row_info,$row,$property)]} {
        set ::mw::workload_info($workload_id,row_info,$row,$property) $value
    } else {
        error "set_chunk_property: invalid args $workload_id $row $property"
    }
    return
}

#######################################################################

proc work_chunk_complete {worker_id workload_id \
                          ntuple_id row cputime status packed} {

    set status_is_ok [string equal $status "ok"]

    if {[catch {get_chunk_property $workload_id $row state} chunk_state]} {
        # Looks like this workload no longer exists.
        # This can happen to some slow worker, AWOL or normal.
        set complete 1
    } else {
        set complete [string equal $chunk_state "completed"]
    }

    # Check whether this is a returning AWOL worker
    global awol_worker_ids
    set awol [info exists awol_worker_ids($worker_id)]

    if {$awol} {
        # This _is_ a returning AWOL worker
        if {$complete || !$status_is_ok} {
            # Just forget about it. It is unlikely to do us any good.
            mw::log warning [mw::actor] "Ignored result from AWOL\
                 worker $worker_id (data status $status)"
            return
        }
    } else {
        set expected_id [mw::cget $worker_id -workload]
        if {![string equal $workload_id $expected_id]} {
            error "Wrong worker $worker_id workload $workload_id,\
                 expected workload $expected_id"
        }
        set oldrow [mw::cget $worker_id -row]
        if {$oldrow != $row} {
            error "Wrong worker $worker_id row number"
        }
        if {$status_is_ok} {
            mw::config $worker_id -state "created"
            mw::config $worker_id -row   -1
            mw::config $worker_id -workload ""
        } else {
            # This worker will leave us and go to the director
            declare_worker_released $worker_id
        }
    }

    if {!$complete} {
        if {$status_is_ok} {
            set id_result [hs::unpack_item "WL Output $workload_id" $packed]
            set expect_category [file join "WL Output $workload_id" \
                                     [mw::cget $workload_id -result_category]]
            if {[string compare [hs::category $id_result] $expect_category]} {
                error "wrong result category"
            }
            if {[hs::uid $id_result] != $row} {
                error "wrong user id"
            }
            set_chunk_property $workload_id $row state "completed"
            set_chunk_property $workload_id $row resultid $id_result
        } else {
            set_chunk_property $workload_id $row state "error"
            set_chunk_property $workload_id $row resultid 0
        }
        set thistime [clock seconds]
        set_chunk_property $workload_id $row finished $thistime
        if {$awol} {
            set_chunk_property $workload_id $row started \
                [expr {$thistime - $cputime}]
            # We need to remove this chunk from the next pass list.
            # Otherwise the code which recognizes workload completion
            # will not work correctly.
            remove_from_future_processing $workload_id $row
        }
        global rows_currently_processed
        catch {unset rows_currently_processed($workload_id,$row)}
        set afterid [get_chunk_property $workload_id $row afterid]
        if {[string compare $afterid ""]} {
            after cancel $afterid
            set_chunk_property $workload_id $row afterid ""
        }
        global workload_completion_check_pending
        if {!$workload_completion_check_pending($workload_id)} {
            set workload_completion_check_pending($workload_id) 1
            after idle [list check_workload_completion $workload_id]
        }
    }

    return
}

#######################################################################

proc use_on_next_pass {workload_id row} {
    set ipass [get_chunk_property $workload_id $row ipass]
    set nextpass [expr {$ipass + 1}]
    set maxpass [mw::parameter max_chunk_tries]
    if {$nextpass < $maxpass} {
        variable ::mw::workload_info
        lappend ::mw::workload_info($workload_id,rows_in_pass,$nextpass) $row
    }
    return
}

#######################################################################

proc work_chunk_failure {worker_id newstate {canceltimeout 1}} {
    if {[catch {mw::cget $worker_id -row} row]} return
    if {$row < 0} return

    set workload_id [mw::cget $worker_id -workload]
    if {[string equal $workload_id ""]} {
        error "Can't get workload id for worker $worker_id"
    }

    global rows_currently_processed
    if {![info exists rows_currently_processed($workload_id,$row)]} return
    unset rows_currently_processed($workload_id,$row)

    set_chunk_property $workload_id $row state $newstate
    set_chunk_property $workload_id $row finished [clock seconds]
    set_chunk_property $workload_id $row resultid 0
    if {$canceltimeout} {
        set afterid [get_chunk_property $workload_id $row afterid]
        if {[string compare $afterid ""]} {
            after cancel $afterid
        }
    }
    set_chunk_property $workload_id $row afterid ""
    if {[string compare $newstate "error"]} {
        use_on_next_pass $workload_id $row
    }
    global workload_completion_check_pending
    if {!$workload_completion_check_pending($workload_id)} {
        set workload_completion_check_pending($workload_id) 1
        after idle [list check_workload_completion $workload_id]
    }
    return
}

#######################################################################

proc periodic_completion_check {period_sec} {
    global main_cycle_finished
    if {$main_cycle_finished && [mw::object_count worker] == 0} {
        global all_workers_returned
        set all_workers_returned 1
    } else {
        after [expr {$period_sec * 1000}] \
            [list periodic_completion_check $period_sec]
    }
    return
}

#######################################################################

proc declare_worker_released {worker_id} {
    if {![catch {mw::verify_id worker $worker_id}]} {
        mw::destroy $worker_id
        remote_eval [mw::director_host] [mw::director_port] \
            report_released_worker [mw::actor_id] $worker_id
    }
    return
}

#######################################################################

proc declare_worker_awol {worker_id} {
    if {![catch {mw::verify_id worker $worker_id}]} {
        mw::destroy $worker_id
        global awol_worker_ids
        set awol_worker_ids($worker_id) 1
        remote_eval [mw::director_host] [mw::director_port] \
            report_awol_worker [mw::actor_id] $worker_id
    }
    if {[mw::object_count worker] == 0} {
        global main_cycle_finished
        if {$main_cycle_finished} {
            after idle {
                global all_workers_returned
                set all_workers_returned 1
            }
        } else {
            remote_eval [mw::director_host] [mw::director_port] \
                log warning [mw::actor] "No workers left"
        }
    }
    return
}

#######################################################################

proc worker_wait_expired {worker_id} {
    global wait_timed_afterids
    if {[info exists wait_timed_afterids($worker_id)]} {
        unset wait_timed_afterids($worker_id)
    }
    declare_worker_awol $worker_id
    return
}

#######################################################################

proc work_chunk_expired {worker_id} {
    if {[catch {mw::verify_id worker $worker_id}]} return
    work_chunk_failure $worker_id "timeout" 0
    declare_worker_awol $worker_id
    return
}

#######################################################################

proc check_workload_completion {workload_id} {
    global workload_completion_check_pending
    set workload_completion_check_pending($workload_id) 0

    # Workload is completed when there are no
    # chunks in the "assigned" state and all
    # pass levels are over.
    global rows_currently_processed
    if {[llength [array names rows_currently_processed "$workload_id,*"]] > 0} {
        return
    }

    variable ::mw::workload_info
    set maxpass [mw::parameter max_chunk_tries]
    for {set ipass 0} {$ipass < $maxpass} {incr ipass} {
        if {$::mw::workload_info($workload_id,passlist_index,$ipass) < \
                [llength $::mw::workload_info($workload_id,rows_in_pass,$ipass)]} {
            return
        }
    }

    # Looks like we are done indeed. Report to the director.
    global ::errorInfo
    if {[catch {
        save_workload_byid $workload_id
        set newstatus [report_workload_completion $workload_id]
        if {[string equal $newstatus "completed"]} {
            global n_workloads_success
            incr n_workloads_success
        }
    }]} {
        set einfo $::errorInfo
        remote_eval [mw::director_host] [mw::director_port] \
            log error [mw::actor] $einfo
        remote_eval [mw::director_host] [mw::director_port] \
                workload_complete [mw::actor_id] $workload_id \
                "error" 0 {} 0
    }
    if {[catch {cleanup_workload $workload_id}]} {
        set einfo $::errorInfo
        remote_eval [mw::director_host] [mw::director_port] \
            log error [mw::actor] $einfo
    }

    global n_workloads_completed
    incr n_workloads_completed

    # Do we have any more workloads to process?
    # If not, raise the request.
    global current_workload_ids
    if {[llength $current_workload_ids] == 0} {
        global request_next_workload
        if {$request_next_workload == 0} {
            after idle {
                global request_next_workload
                set request_next_workload 1
            }
        }
    }

    return
}

#######################################################################

proc periodic_workload_save {period_sec} {
    catch {save_workload_result}
    after [expr {1000*$period_sec}] [list periodic_workload_save $period_sec]
    return
}

#######################################################################

proc save_workload_result {} {
    set nsaved 0
    global current_workload_ids
    foreach id $current_workload_ids {
        incr nsaved [save_workload_byid $id]
    }
    set nsaved
}

#######################################################################

proc upcategory {id} {
    set category [hs::category $id]
    set newcat [join [lrange [split $category /] 1 end] /]
    hs::change_category $id $newcat
    return
}

#######################################################################

proc downcategory {id prefix} {
    set category [hs::category $id]
    set newcat "$prefix/$category"
    hs::change_category $id $newcat
    return
}

#######################################################################

proc save_workload_items {wid filename} {
    set prefix "WL Output $wid"
    set result_ids [hs::list_items "" "$prefix/..." 1]
    if {[llength $result_ids] > 0} {
        foreach id $result_ids {
            upcategory $id
        }
        set n_saved [hs::save_file_byids $filename $result_ids]
        foreach id $result_ids {
            downcategory $id $prefix
        }
        if {$n_saved < 1} {
            error "Failed to save file $filename"
        }
    } else {
        set n_saved 0
    }
    return $n_saved
}

#######################################################################

proc save_workload_byid {wid} {
    mw::verify_id workload $wid

    set current_output_file [mw::cget $wid -output_file]

    set splitlist [split $current_output_file :]
    set protocol [lindex $splitlist 0]
    set remainder [join [lrange $splitlist 1 end] :]

    set remote 0
    switch -exact -- $protocol {
        ftp -
        http {
            error "Protocol \"$protocol\" is not supported for output files"
        }
        rcp -
        scp {
            set remote 1
            set location $remainder
        }
        file {
            set location $remainder
        }
        default {
            set location $current_output_file
        }
    }

    if {$remote} {
        foreach {filename chan} [hs::tempfile "/tmp/" ".hs"] break
        close $chan
        global ::errorInfo
        set status [catch {
            set n_saved [save_workload_items $wid $filename]
            if {$n_saved > 0} {
                exec $protocol $filename $location
            }
        } errmes]
        set saveInfo $::errorInfo
        file delete $filename
        if {$status} {
            mw::config $wid -n_saved_items 0
            error $errmes $saveInfo
        }
    } else {
        file mkdir [file dirname $location]
        set n_saved [save_workload_items $wid $location]
    }
    mw::config $wid -n_saved_items $n_saved

    catch {
        remote_eval [mw::director_host] [mw::director_port] \
            log info [mw::actor] "Saved $n_saved chunks\
            from workload $wid in file $location"
    }
    return $n_saved
}

#######################################################################

proc report_workload_failure {master_id next_workload einfo} {
    remote_eval [mw::director_host] [mw::director_port] \
            log error [mw::actor] $einfo
    if {[llength $next_workload] >= 2} {
        foreach {project_id workload_id} $next_workload break
        remote_eval [mw::director_host] [mw::director_port] \
                workload_complete $master_id $workload_id \
                "error" 0 {} 0
    }
    return
}

#######################################################################

proc report_workload_completion {workload_id} {
    set workload_ntuple_nrows [mw::cget $workload_id -ntuple_nrows]
    set failed [list]
    set cputime 0
    for {set row 0} {$row < $workload_ntuple_nrows} {incr row} {
        set state [get_chunk_property $workload_id $row state]
        if {[string compare $state "completed"]} {
            lappend failed $row
        }
        set started [get_chunk_property $workload_id $row started]
        set finished [get_chunk_property $workload_id $row finished]
        if {$started && $finished} {
            set dt [expr {$finished - $started}]
            incr cputime $dt
        } else {
            error "Missing timing info for row $row"
        }
    }
    if {[llength $failed] > 0} {
        set newstatus "error"
    } else {
        set newstatus "completed"
    }
    remote_eval [mw::director_host] [mw::director_port] \
        workload_complete [mw::actor_id] $workload_id \
        $newstatus $workload_ntuple_nrows $failed $cputime
    return $newstatus
}

#######################################################################

proc worker_got_signal {worker_id signal} {
    if {[catch {mw::verify_id worker $worker_id}]} return

    variable ::mw::job_timeout_signals
    if {[lsearch -exact $::mw::job_timeout_signals $signal] >= 0} {
        set newstate "signal"
    } else {
        set newstate "error"
    }

    work_chunk_failure $worker_id $newstate
    mw::destroy $worker_id
    return
}

#######################################################################

proc exit_master_stats {} {
    return
}

#######################################################################

proc current_workload_info {} {
    global current_workload_ids
    if {[llength $current_workload_ids] == 0} {
        return "No current workload"
    }
    set result [list]
    foreach id $current_workload_ids {
        lappend result [workload_info_string $id]
    }
    join $result "\n\n"
}

#######################################################################

proc workload_info_string {id} {
    set current_workload_id $id

    global workload_attributes
    foreach name $workload_attributes {
        catch {set current_$name [mw::cget $id -$name]}
    }
    set workload_start_time [mw::cget $id -starttime]
    set workload_first_worker_arrived [mw::cget $id -first_worker_arrived]
    set workload_stop_time [mw::cget $id -stoptime]
    set workload_ntuple_id [mw::cget $id -ntuple_id]
    set workload_ntuple_nrows [mw::cget $id -ntuple_nrows]
    set workload_n_saved_items [mw::cget $id -n_saved_items]

    # Figure out the number of rows in each state
    array unset counts_by_state
    set cputime 0
    set n_cpu_time 0
    for {set row 0} {$row < $workload_ntuple_nrows} {incr row} {
        set state [get_chunk_property $id $row state]
        if {[catch {incr counts_by_state($state)}]} {
            set counts_by_state($state) 1
        }
        set started [get_chunk_property $id $row started]
        set finished [get_chunk_property $id $row finished]
        if {$started && $finished} {
            set dt [expr {$finished - $started}]
            incr cputime $dt
            incr n_cpu_time
        }
    }

    set ettc -1
    if {$workload_stop_time == 0} {
        if {$n_cpu_time >= 5} {
            # Estimate time to completion
            if {$cputime > 0} {
                set remaining_rows [expr {$workload_ntuple_nrows - $n_cpu_time}]
                set completion_rate [expr {$n_cpu_time*1.0/$cputime}]
                set ettc1worker [expr {$remaining_rows/$completion_rate}]
                set n_workers_now [mw::object_count worker]
                if {$n_workers_now > 0} {
                    set ettc [expr {round($ettc1worker/$n_workers_now)}]
                } else {
                    set ettc -1
                }
            } else {
                set ettc 0
            }
        }
    }

    set result "****  current workload id $current_workload_id  ****"
    append result "\n" "project_id      : " $current_project_id
    append result "\n" "input_file      : " $current_input_file
    append result "\n" "ntuple_title    : " $current_ntuple_title
    append result "\n" "ntuple_category : " $current_ntuple_category
    append result "\n" "result_category : " $current_result_category
    append result "\n" "output_file     : " $current_output_file
    append result "\n" "assigned        : " [mw::time_string $workload_start_time]
    append result "\n" "work started    : " [mw::time_string $workload_first_worker_arrived]
    if {$workload_stop_time} {
        append result "\n" "work finished   : " [mw::time_string $workload_stop_time]
    } elseif {$ettc >= 0} {
        append result "\n" "estimated TTC   : " [mw::sec_to_hrs $ettc]
    } else {
        append result "\n" "estimated TTC   : " "unknown"
    }
    append result "\n" "ntuple_id       : " $workload_ntuple_id
    append result "\n" "n_ntuple_rows   : " $workload_ntuple_nrows
    append result "\n" "n_saved_items   : " $workload_n_saved_items

    variable ::mw::workload_info
    set maxpass [mw::parameter max_chunk_tries]
    for {set ipass 0} {$ipass < $maxpass} {incr ipass} {
        set rows_in_pass $::mw::workload_info($id,rows_in_pass,$ipass)
        set n_pass_rows [llength $rows_in_pass]
        append result "\n" "pass " [format "%-11d" $ipass] \
                ": $::mw::workload_info($id,passlist_index,$ipass) / $n_pass_rows"
        if {$workload_ntuple_nrows == $n_pass_rows} {
            append result ", all rows"
        } elseif {$n_pass_rows > 0} {
            append result ", rows [lsort -integer $rows_in_pass]"
        }
    }    

    append result "\n" "assigned rows   : " [lsort -integer \
            [array names rows_currently_processed "$id,*"]]
    append result "\n" "row cputime     : " [mw::sec_to_hrs $cputime]
    append result "\n" "Count of rows by state"
    foreach state [array names counts_by_state] {
        append result "\n" [format "%-16s" $state] ": " $counts_by_state($state)
    }
    set result
}

#######################################################################

proc report {} {
    set report ""
    append report "\n####"
    append report "\n####  [mw::actor] status report"
    append report "\n####"

    global awol_worker_ids master_start_time n_workloads_completed n_workloads_success

    append report "\n" "director host        : " [mw::director_host]
    append report "\n" "director port        : " [mw::director_port]
    append report "\n" "host                 : " [info hostname]
    append report "\n" "pid                  : " [pid]
    append report "\n" "started              : " [mw::time_string $master_start_time]
    append report "\n" "uptime               : " [mw::uptime $master_start_time]
    append report "\n" "workloads processed  : " $n_workloads_completed
    append report "\n" "no bug workloads     : " $n_workloads_success
    append report "\n" "n workers average    : " [format {%g} [mw::average_load worker]]
    append report "\n" "n current workers    : " [mw::object_count worker]
    append report "\n" "current worker ids   : " [lsort -dictionary [mw::object_list -type worker]]
    append report "\n" "n workers awol       : " [llength [array names awol_worker_ids]]
    append report "\n" "awol workers ids     : " [lsort -dictionary [array names awol_worker_ids]]
    append report "\n" "main cycle completed : " 
    global main_cycle_finished
    if {$main_cycle_finished} {
        append report "yes"
    } else {
        append report "no"
    }
    append report "\n" "waiting for workload : " 
    global get_next_workload_waitvariable
    if {$get_next_workload_waitvariable} {
        append report " until [mw::time_string $get_next_workload_waitvariable]"
    } else {
        append report "no"
    }

    append report "\n\n" [current_workload_info]
    append report "\n"
    set report
}
