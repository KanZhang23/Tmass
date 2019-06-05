
namespace eval ::mw:: {
    variable namespace_loaded_timestamp
    if {![info exists namespace_loaded_timestamp]} {
        set namespace_loaded_timestamp [clock seconds]

        proc default {varname value} {
            uplevel [subst -nobackslashes -nocommands {
                variable $varname
                if {![info exists $varname]} {
                    set $varname $value
                }
            }]
            return
        }

        default unique_int_host localhost
        default unique_int_port 49999

        default log_object_birth 1
        default log_object_death 1
        default log_object_association 1
        default log_object_dissociation 1

        # The following parameter should be set
        # to a typical work chunk processing time
        default worker_wait_interval     300

        # Maximum number of events any worker will process.
        # Non-positive value means there is no limit.
        default worker_max_events       -1

        default max_workloads_per_master 3
        default max_workers_per_master   1000
        default max_workload_tries       2
        default max_chunk_tries          3
        default chunk_timeout_sec        3600

        default log_info_channel    stdout
        default log_warning_channel stdout
        default log_error_channel   stdout
        default log_debug_channel   stdout
        default log_server_channel  stdout
        default log_unknown_channel stdout

        # Signals which can be sent by the job queue
        default job_timeout_signals {[list SIGQUIT SIGINT SIGTERM JOBTIMEOUT MAXEVENTS]}

        rename default {}
    }
}

#######################################################################

proc ::mw::set_parameter {name value} {
    variable $name $value
}

#######################################################################

proc ::mw::parameter {name} {
    variable $name
    set $name
}

#######################################################################

proc ::mw::set_actor {type id} {
    variable actor_type $type
    variable actor_id $id
    return
}

#######################################################################

proc ::mw::actor {} {
    variable actor_type
    variable actor_id
    list $actor_type $actor_id
}

#######################################################################

proc ::mw::actor_id {} {
    variable actor_id
    return $actor_id
}

#######################################################################

proc ::mw::actor_type {} {
    variable actor_type
    return $actor_type
}

#######################################################################

proc ::mw::verify_port {port} {
    if {![string is integer -strict $port]} {
        error "bad port argument \"$port\""
    }
    if {$port < 1024 || $port >= 65536} {
        error "port number is out of range"
    }
    return
}

#######################################################################

proc ::mw::set_director_address {host port} {
    mw::verify_port $port
    variable director_host $host
    variable director_port $port
    return
}

#######################################################################

proc ::mw::director_host {} {
    variable director_host
    set director_host
}

#######################################################################

proc ::mw::director_port {} {
    variable director_port
    set director_port
}

#######################################################################

proc ::mw::unique_random_id {nrand_chars} {
    # Generate the unique part of the id
    variable unique_int_host
    variable unique_int_port
    set newname [remote_eval $unique_int_host $unique_int_port nextint]

    # Generate the random part of the id
    append newname "_"
    set chars "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    set mypid [pid]
    for {set j 0} {$j < $nrand_chars} {incr j} {
        append newname [string index $chars \
                [expr {([clock clicks] ^ $mypid) % 62}]]
    }

    set newname
}

#######################################################################

proc ::mw::verify_index {id} {
    if {[string first {,[]\{\}\"\\} $id] >= 0} {
        error "\"$id\" is not a safe array index"
    }
    return
}

#######################################################################

proc ::mw::verify_id {args} {
    set len [llength $args]
    if {$len == 1} {
        set id [lindex $args 0]
        set object [mw::typeof $id]
    } elseif {$len == 2} {
        foreach {object id} $args break
        mw::verify_index $id
    } else {
        error "wrong \# of arguments"
    }
    variable ${object}_ids
    if {![info exists ${object}_ids($id)]} {
        error "$object with id \"$id\" does not exist"
    }
    return
}

#######################################################################

proc ::mw::default_constructor {} {
    uplevel {
        mw::verify_index $id
        # Constructor name. Variables in this piece of code
        # must have funny names so that we don't clobber
        # the variables defined in the calling procedure.
        set constructor_f3NjWE8jG7Z [namespace tail [lindex [info level 0] 0]]
        if {[string compare -length 13 $constructor_f3NjWE8jG7Z "create_named_"]} {
            error "can't run default constructor inside\
                    [lindex [info level 0] 0] procedure:\
                    unconventional constructor name"
        }
        set object_type_c3fQg0xnG [string range $constructor_f3NjWE8jG7Z 13 end]
        mw::verify_index $object_type_c3fQg0xnG
        variable ${object_type_c3fQg0xnG}_ids
        if {[info exists ${object_type_c3fQg0xnG}_ids($id)]} {
            error "$object_type_c3fQg0xnG with id \"$id\" already exist"
        }
        set varnames_jkT7fg4Kx [info args [lindex [info level 0] 0]]
        if {[lsearch -exact $varnames_jkT7fg4Kx "args"] >= 0} {
            error "can't run default constructor inside\
                    [lindex [info level 0] 0] procedure:\
                    variant constructor parameters"
        }
        variable n_${object_type_c3fQg0xnG}_ids
        variable seqnum_$object_type_c3fQg0xnG
        if {![info exists n_${object_type_c3fQg0xnG}_ids]} {
            set n_${object_type_c3fQg0xnG}_ids 0
            set seqnum_$object_type_c3fQg0xnG -1
        }
        variable ${object_type_c3fQg0xnG}_info
        set ${object_type_c3fQg0xnG}_info($id,created) [clock seconds]
        set ${object_type_c3fQg0xnG}_info($id,state) "created"
        set ${object_type_c3fQg0xnG}_info($id,seqnum) \
                [incr seqnum_$object_type_c3fQg0xnG]
        foreach varname_Cg5sNiWdhJK8x $varnames_jkT7fg4Kx {
            if {[string compare $varname_Cg5sNiWdhJK8x "id"]} {
                set ${object_type_c3fQg0xnG}_info($id,$varname_Cg5sNiWdhJK8x) \
                        [set $varname_Cg5sNiWdhJK8x]
            }
        }
        set ${object_type_c3fQg0xnG}_ids($id) 1
        incr n_${object_type_c3fQg0xnG}_ids
        variable log_object_birth
        if {$log_object_birth} {
            mw::log info [mw::actor] "created $object_type_c3fQg0xnG\
                    #[set seqnum_$object_type_c3fQg0xnG], id $id"
        }
        variable all_object_types
        set all_object_types($object_type_c3fQg0xnG) 1
        variable all_object_ids_t
        set all_object_ids_t($id) $object_type_c3fQg0xnG
        mw::update_load_info $object_type_c3fQg0xnG
    }
    return
}

#######################################################################

proc ::mw::create_named_worker {id hostname user \
        pid script executable jobname jobid} {
    set claimhost $hostname
    # set addr [server::eval_addr]
    # if {[string compare $addr ""]} {
    #	set hostname [server::reverse_ip_lookup $addr]
    # }
    mw::default_constructor
    set worker_info($id,row) -1
    set worker_info($id,workload) ""
    set worker_info($id,claimhost) $claimhost
    return $id
}

#######################################################################

proc ::mw::add_worker_cputime {worker_id workload_id nchunks cputime} {
    mw::verify_id worker $worker_id
    mw::verify_id workload $workload_id
    variable worker_info
    if {![info exists worker_info($worker_id,stats,$workload_id,nchunks)]} {
        set worker_info($worker_id,stats,$workload_id,nchunks) 0
        set worker_info($worker_id,stats,$workload_id,cputime) 0
    }
    incr worker_info($worker_id,stats,$workload_id,nchunks) $nchunks
    incr worker_info($worker_id,stats,$workload_id,cputime) $cputime
    return
}

#######################################################################

proc ::mw::create_named_master {id hostname \
        port user pid script executable jobname jobid} {
    set claimhost $hostname
    # set addr [server::eval_addr]
    # if {[string compare $addr ""]} {
    # 	set hostname [server::reverse_ip_lookup $addr]
    # }
    mw::default_constructor
    set master_info($id,claimhost) $claimhost
    set master_info($id,workloads) [list]
    return $id
}

#######################################################################

proc ::mw::create_named_project {id tclfiles anacommand {cfiles {}} {flags {}}} {
    if {[string equal $id "exit"] || [string equal $id "return"]} {
        error "Reserved project name \"$id\". Please use another project name."
    }
    mw::validate_compile_flags $flags
    foreach {varname listname} {
        ccode   cfiles
        tclcode tclfiles
    } {
        set $varname ""
        foreach fname [set $listname] {
            if {[string compare $fname ""]} {
                if {[file readable $fname]} {
                    set chan [open $fname r]
                    set code [read $chan [file size $fname]]
                    close $chan
                    append $varname $code
                } else {
                    error "File \"$fname\" does not exist (or unreadable)"
                }
            }
        }
    }
    mw::default_constructor
    set project_info($id,tclcode) $tclcode
    set project_info($id,ccode) $ccode
    set project_info($id,starttime) 0
    set project_info($id,stoptime) 0
    return $id
}

#######################################################################

proc ::mw::string_is_almost_alnum {allowed_nonalnum_chars string} {
    set map [list]
    set len [string length $allowed_nonalnum_chars]
    for {set i 0} {$i < $len} {incr i} {
        lappend map [string index $allowed_nonalnum_chars $i] {}
    }
    set mapstring [string map $map $string]
    expr {[string is alnum -strict $mapstring] && \
            [string is ascii -strict $mapstring]}
}

#######################################################################

proc ::mw::validate_file_name {filename} {
    set safechars "#%^_+-=:/."
    if {![mw::string_is_almost_alnum $safechars $filename]} {
        error "Unsafe file name"
    }
    return
}

#######################################################################

proc ::mw::validate_compile_flags {flags} {
    set safechars "#%_+-=:/. \{\}"
    foreach flag $flags {
        if {![mw::string_is_almost_alnum $safechars $flag]} {
            error "Unsafe flags"
        }
    }
    return
}

#######################################################################

proc ::mw::create_named_workload {id host_patterns input_file \
        ntuple_title ntuple_category result_category output_file} {
    if {[string equal [string trim $ntuple_category] \
            [string trim $result_category]]} {
        error "Same category \"$ntuple_category\"\
                is specified for both input and output ntuples"
    }

    # Check that the output file name is safe
    mw::validate_file_name $output_file

    mw::default_constructor
    set workload_info($id,mlist)     [list]
    set workload_info($id,starttime) 0
    set workload_info($id,stoptime)  0
    set workload_info($id,nchunks)   0
    set workload_info($id,failed)    [list]
    set workload_info($id,nfailed)   0
    set workload_info($id,cputime)   0
    return $id
}

#######################################################################

proc ::mw::create {object args} {
    set id [mw::unique_random_id 10]
    eval mw::create_named_$object $id $args
}

#######################################################################

proc ::mw::destroy {id} {
    mw::verify_id $id
    variable all_object_ids_t
    set object $all_object_ids_t($id)
    variable ${object}_ids
    variable n_${object}_ids
    variable ${object}_info

    set number [set ${object}_info($id,seqnum)]

    mw::dissociate_all_branches $object $id
    mw::dissociate_this_branch $object $id
    unset all_object_ids_t($id)
    unset ${object}_ids($id)
    array unset ${object}_info "$id,*"
    incr n_${object}_ids -1

    variable log_object_death
    if {$log_object_death} {
        mw::log info [mw::actor] "destroyed $object\
                #$number, id $id"
    }
    mw::update_load_info $object
    return
}

#######################################################################

proc ::mw::update_load_info {object} {
    variable load_accounting
    if {![info exists load_accounting($object,count)]} {
        variable namespace_loaded_timestamp
        set load_accounting($object,timestamp) $namespace_loaded_timestamp
        set load_accounting($object,accum) 0
        set load_accounting($object,count) 0
    }
    set thistime [clock seconds]
    set dt [expr {$thistime - $load_accounting($object,timestamp)}]
    set load_accounting($object,timestamp) $thistime
    incr load_accounting($object,accum) [expr {$dt * $load_accounting($object,count)}]
    variable n_${object}_ids
    set load_accounting($object,count) [set n_${object}_ids]
    return
}

#######################################################################

proc ::mw::average_load {object} {
    variable load_accounting
    if {![info exists load_accounting($object,count)]} {
        return 0
    }
    update_load_info $object
    variable namespace_loaded_timestamp
    set dt [expr {$load_accounting($object,timestamp) - $namespace_loaded_timestamp}]
    if {$dt == 0} {
        return 0
    }
    expr {$load_accounting($object,accum)*1.0/$dt}
}

#######################################################################

proc ::mw::associate_one_to_many {listtype listid itemtype itemid} {
    # One-to-many association from the listtype object
    # to itemtype objects
    mw::verify_id $itemtype $itemid
    mw::verify_id $listtype $listid
    variable ${itemtype}_info
    if {[info exists ${itemtype}_info($itemid,root,$listtype)]} {
        error "$itemtype \"$itemid\" is already connected to\
                $listtype \"[set ${itemtype}_info($itemid,root,$listtype)]\""
    }
    variable ${listtype}_info
    if {[info exists ${listtype}_info($listid,branch,$itemtype,$itemid)]} {
        error "$listtype \"$listid\" already\
                knows about $itemtype \"$itemid\""
    }
    if {[string equal $listtype $itemtype] && \
            [string equal $listid $itemid]} {
        error "can't connect $listtype \"$listid\" to itself"
    }
    set ${itemtype}_info($itemid,root,$listtype) $listid
    set ${listtype}_info($listid,branch,$itemtype,$itemid) 1
    variable log_object_association
    if {$log_object_association} {
        mw::log info [mw::actor] "$itemtype $itemid\
                is connected to $listtype $listid"
    }
    return
}

#######################################################################

proc ::mw::dissociate_one_to_many {listtype listid itemtype itemid} {
    mw::verify_id $itemtype $itemid
    mw::verify_id $listtype $listid
    variable ${itemtype}_info
    if {![info exists ${itemtype}_info($itemid,root,$listtype)]} {
        error "$itemtype \"$itemid\" is not\
                connected to $listtype \"$listid\""
    }
    variable ${listtype}_info
    if {![info exists ${listtype}_info($listid,branch,$itemtype,$itemid)]} {
        error "$listtype \"$listid\" doesn't\
                know about $itemtype \"$itemid\""
    }
    unset ${itemtype}_info($itemid,root,$listtype)
    unset ${listtype}_info($listid,branch,$itemtype,$itemid)
    variable log_object_dissociation
    if {$log_object_dissociation} {
        mw::log info [mw::actor] "$itemtype $itemid\
                is disconnected from $listtype $listid"
    }
    return
}

#######################################################################

proc ::mw::dissociate_this_branch {itemtype itemid {listtype ""}} {
    mw::verify_id $itemtype $itemid
    variable ${itemtype}_info
    if {[string equal $listtype ""]} {
        foreach index [array names ${itemtype}_info "$itemid,root,*"] {
            set listtype [lindex [split $index ,] end]
            set listid [set ${itemtype}_info($itemid,root,$listtype)]
            mw::dissociate_one_to_many $listtype $listid $itemtype $itemid
        }
    } elseif {[info exists ${itemtype}_info($itemid,root,$listtype)]} {
        set listid [set ${itemtype}_info($itemid,root,$listtype)]
        mw::dissociate_one_to_many $listtype $listid $itemtype $itemid
    }
    return
}

#######################################################################

proc ::mw::root_id {itemtype itemid listtype} {
    mw::verify_id $itemtype $itemid
    variable ${itemtype}_info
    if {[info exists ${itemtype}_info($itemid,root,$listtype)]} {
        set result [set ${itemtype}_info($itemid,root,$listtype)]
    } else {
        set result ""
    }
    set result
}

#######################################################################

proc ::mw::dissociate_all_branches {listtype listid {itemtype ""}} {
    mw::verify_id $listtype $listid
    variable ${listtype}_info
    if {[string equal $itemtype ""]} {
        foreach index [array names ${listtype}_info "$listid,branch,*"] {
            foreach {itemtype itemid} [lrange [split $index ,] end-1 end] break
            mw::dissociate_one_to_many $listtype $listid $itemtype $itemid
        }
    } else {
        foreach index [array names ${listtype}_info "$listid,branch,$itemtype,*"] {
            set itemid [lindex [split $index ,] end]
            mw::dissociate_one_to_many $listtype $listid $itemtype $itemid
        }
    }
    return
}

#######################################################################

proc ::mw::branch_ids {listtype listid {itemtype "*"}} {
    mw::verify_id $listtype $listid
    set idlist [list]
    variable ${listtype}_info
    foreach branch [array names ${listtype}_info "$listid,branch,$itemtype,*"] {
        lappend idlist [lindex [split $branch ,] end]
    }
    set idlist
}

#######################################################################

proc ::mw::count_branches {listtype listid {itemtype "*"}} {
    mw::verify_id $listtype $listid
    variable ${listtype}_info
    llength [array names ${listtype}_info "$listid,branch,$itemtype,*"]
}

#######################################################################

proc ::mw::config_logger {args} {
    # Usage: config_logger type0 channel0 type1 channel1 ....
    #
    # Each type must be one of the known channel types defined below
    # or be equal to "all".
    set known_types [list info warning error debug unknown server]
    set len [llength $args]
    if {[expr {$len % 2}]} {
        error "wrong # of arguments"
    }
    foreach {name channel} $args {
        if {[lsearch -exact $known_types $name] < 0 && \
                ![string equal $name "all"]} {
            error "invalid channel type \"$name\""
        }
    }
    foreach {name channel} $args {
        if {[string equal $name "all"]} {
            foreach type $known_types {
                variable log_${type}_channel $channel
            }
        } else {
            variable log_${name}_channel $channel
        }
    }
    return
}

#######################################################################

proc ::mw::log {type sender args} {
    #
    # Known 'type' arguments are info, warning, error, debug, or server.
    # Any other type will default to unknown.
    #
    # 'sender' is some kind of sender id, for example, procedure
    # name or object id of the sender
    # 
    # 'args' is the message text
    #
    variable log_info_channel
    variable log_warning_channel
    variable log_error_channel
    variable log_debug_channel
    variable log_server_channel
    variable log_unknown_channel
    switch -- $type {
        info -
        Info -
        INFO {
            set channel $log_info_channel
            set prefix "-i-"
        }
        warning -
        Warning -
        WARNING {
            set channel $log_warning_channel
            set prefix "-w-"
        }
        error -
        Error -
        ERROR {
            set channel $log_error_channel
            set prefix "-e-"
        }
        debug -
        Debug -
        DEBUG {
            set channel $log_debug_channel
            set prefix "-d-"
        }
        server -
        Server -
        SERVER {
            set channel $log_server_channel
            set prefix "-s-"
        }
        default {
            set channel $log_unknown_channel
            set prefix "-u-"
        }
    }
    if {[string compare $channel "none"]} {
        set datestring [clock format [clock seconds] -format {%D %T}]
        set fromwhere [server::eval_addr]
        if {[string compare $fromwhere ""]} {
            # This message is caused by a remotely executed command
            # set senderhost [server::reverse_ip_lookup $fromwhere]
            set senderhost [lindex [info level -1] 0]
            if {[string compare $sender ""]} {
                set fullsender "$sender // $senderhost"
            } else {
                set fullsender "\{\} // $senderhost"
            }
            set messagetext "$prefix $datestring   $fullsender :  $args"
        } else {
            # This message is caused by a local command
            if {[string compare $sender ""]} {
                set messagetext "$prefix $datestring   $sender :  $args"
            } else {
                set messagetext "$prefix $datestring   $args"
            }
        }
        puts $channel $messagetext
        flush $channel
    }
    return
}

#######################################################################

proc ::mw::object_list {args} {
    #
    #  Usage: mw::object_list -type? type_patern? -id? id_pattern? \
    #             -property0 value0 -property1 value1 ...
    #
    #  Each propertyN and valueN are patterns for "string match" comparison.
    #  For each N, object matches if there is at least one object property
    #  whose name matches "property0" and whose value matches "value0".
    #  Results of the matches are ANDed for type pattern, id pattern, and
    #  all values of N.
    #
    #  Example: find all objects of type master whose state is "wait",
    #           "waiting", or similar.
    #
    #  mw::object_list -type master -state wait*
    #
    set len [llength $args]
    if {[expr {$len % 2}]} {
        error "wrong # of arguments"
    }
    if {$len == 0} {
        variable all_object_ids_t
        return [array names all_object_ids_t]
    }
    set type "*"
    set id "*"
    set matchlist [list]
    foreach {property value} $args {
        regsub "^-" $property "" property
        if {[string equal $property "type"] || [string equal $property "id"]} {
            set $property $value
        } else {
            lappend matchlist $property $value
        }
    }
    set objlist [list]
    variable all_object_types
    foreach type [array names all_object_types $type] {
        variable ${type}_ids
        variable ${type}_info
        foreach oid [array names ${type}_ids $id] {
            set match_failed 0
            foreach {property_pattern value_pattern} $matchlist {
                set property_match_found 0
                foreach index [array names ${type}_info "$oid,$property_pattern"] {
                    set value [set ${type}_info($index)]
                    if {[string match $value_pattern $value]} {
                        set property_match_found 1
                        break
                    }
                }
                if {!$property_match_found} {
                    set match_failed 1
                    break
                }
            }
            if {!$match_failed} {
                lappend objlist $oid
            }
        }
    }
    set objlist
}

#######################################################################

proc ::mw::objects_created {{object_type ""}} {
    if {[string equal $object_type ""]} {
        set result 0
        foreach type [mw::object_types] {
            incr result [mw::objects_created $type]
        }
    } else {
        variable seqnum_$object_type
        if {[info exists seqnum_$object_type]} {
            set result [set seqnum_$object_type]
            incr result
        } else {
            set result 0
        }
    }
    set result
}

#######################################################################

proc ::mw::object_count {{object_type ""}} {
    if {[string equal $object_type ""]} {
        variable all_object_ids_t
        set result [llength [array names all_object_ids_t]]
    } else {
        variable n_${object_type}_ids
        if {[info exists n_${object_type}_ids]} {
            set result [set n_${object_type}_ids]
        } else {
            set result 0
        }
    }
    set result
}

#######################################################################

proc ::mw::object_info {id} {
    set object [mw::typeof $id]
    variable ${object}_info
    set info "****  $object $id  ****"
    set maxlen 0
    set properties [list]
    foreach name [array names ${object}_info "$id,*"] {
        set property [join [lrange [split $name ,] 1 end] ,]
        lappend properties $property
        set len [string length $property]
        if {$len > $maxlen} {
            set maxlen $len
        }
    }
    foreach property [lsort -dictionary $properties] {
        set value [set ${object}_info($id,$property)]
        switch -exact -- $property {
            tclcode -
            ccode {
                if {[string compare $value ""]} {
                    set value "<$property>"
                }
            }
            created -
            starttime -
            stoptime {
                set value [mw::time_string $value]
            }
        }
        append info "\n" [format "%-${maxlen}s" $property] " : " $value
    }
    set info
}

#######################################################################

proc ::mw::restore {id} {
    variable archive_types
    if {![info exists archive_types($id)]} {
        error "Object with id $id is not in the archive"
    }
    set object $archive_types($id)

    variable ${object}_ids
    if {[info exists ${object}_ids($id)]} {
        set resurrected 0
    } else {
        variable n_${object}_ids
        variable all_object_ids_t

        set ${object}_ids($id) 1
        set all_object_ids_t($id) $object
        incr n_${object}_ids
        set resurrected 1
    }

    variable ${object}_info
    array unset ${object}_info "$id,*"

    variable archive_data
    foreach index [array names archive_data "$object,$id,*"] {
        set property [join [lrange [split $index ,] 1 end] ,]
        set ${object}_info($property) $archive_data($index)
    }

    if {$resurrected} {
        mw::update_load_info $object
    }
    return
}

#######################################################################

proc ::mw::forget {id} {
    variable archive_data
    variable archive_types
    if {[info exists archive_types($id)]} {
        set object $archive_types($id)
        unset archive_types($id)
        array unset archive_data "$object,$id,*"
    }
    return
}

#######################################################################

proc ::mw::recall {id property} {
    variable archive_types
    if {![info exists archive_types($id)]} {
        error "Object with id \"$id\" is not in the archive"
    }
    set object $archive_types($id)
    variable archive_data
    set archive_data($object,$id,$property)
}

#######################################################################

proc ::mw::archive {id} {
    set object [mw::typeof $id]
    mw::forget $id

    variable archive_data
    variable archive_types
    variable ${object}_info
    set archive_types($id) $object
    foreach index [array names ${object}_info "$id,*"] {
        set archive_data($object,$index) [set ${object}_info($index)]
    }
    return
}

#######################################################################

proc ::mw::typeof {id} {
    mw::verify_index $id
    variable all_object_ids_t
    if {![info exists all_object_ids_t($id)]} {
        error "object with id \"$id\" does not exist"
    }
    set all_object_ids_t($id)
}

#######################################################################

proc ::mw::cget {id property} {
    set object [mw::typeof $id]
    variable ${object}_info
    regsub "^-" $property "" property
    if {![info exists ${object}_info($id,$property)]} {
        error "$object with id \"$id\" does not\
                 have \"$property\" property"
    }
    set ${object}_info($id,$property)
}

#######################################################################

proc ::mw::config {id args} {
    set object [mw::typeof $id]
    variable ${object}_info
    if {[expr {[llength $args] % 2}]} {
        error "wrong \# of arguments"
    }
    set valid_data [list]
    foreach {property value} $args {
        regsub "^-" $property "" property
        if {![info exists ${object}_info($id,$property)]} {
            error "$object with id \"$id\" does not\
                     have \"$property\" property"
        }
        lappend valid_data $property $value
    }
    foreach {property value} $valid_data {
        set ${object}_info($id,$property) $value
    }
    return
}

#######################################################################

proc ::mw::object_types {} {
    variable all_object_types
    array names all_object_types
}

#######################################################################

proc ::mw::order_sequentially {type idlist} {
    variable ${type}_info
    set pairlist [list]
    foreach id $idlist {
        mw::verify_id $type $id
        lappend pairlist [list $id [set ${type}_info($id,seqnum)]]
    }
    set sorted [list]
    foreach pair [lsort -integer -index 1 $pairlist] {
        lappend sorted [lindex $pair 0]
    }
    set sorted
}

#######################################################################

proc ::mw::wait {sec} {
    # Wait while processing events -- we don't want to block
    # tcl processing of signals for a long period of time
    if {$sec >= 0} {
        if {$sec} {
            set msec [expr {1000 * $sec}]
        } else {
            set msec idle
        }
        global wait_flag_HB7kWv104KfhtdS3sko0
        set wait_flag_HB7kWv104KfhtdS3sko0 0
        after $msec {
            global wait_flag_HB7kWv104KfhtdS3sko0
            set wait_flag_HB7kWv104KfhtdS3sko0 1
        }
        vwait wait_flag_HB7kWv104KfhtdS3sko0
    }
    return
}

#######################################################################

proc ::mw::myenv {which {default_value {}}} {
    global env
    if {[info exists env($which)]} {
        return $env($which)
    } else {
        return $default_value
    }
}

#######################################################################

proc ::mw::time_string {clock_sec} {
    if {$clock_sec} {
        set value [clock format $clock_sec -format {%D %T}]
    } else {
        set value ""
    }
    set value
}

#######################################################################

proc ::mw::register {host port method args} {
    while {1} {
        mw::set_director_address $host $port
        set reply [eval remote_eval $host $port $method $args]
        if {[string equal -length 9 $reply "redirect "]} {
            if {[llength $reply] != 3} {
                error "Failed to parse server reply \"$reply\""
            }
            foreach {dumm host port} $reply {}
        } else {
            break
        }
    }
    set reply
}

#######################################################################

proc ::mw::sec_to_hrs {dtsec} {
    set dtmin [expr {int($dtsec) / 60}]
    set dtsec [expr {int($dtsec) % 60}]
    set dth   [expr {$dtmin / 60}]
    set dtmin [expr {$dtmin % 60}]
    return "$dth hrs $dtmin min $dtsec sec"
}

#######################################################################

proc ::mw::uptime {start_time} {
    mw::sec_to_hrs [expr {[clock seconds] - $start_time}]
}
