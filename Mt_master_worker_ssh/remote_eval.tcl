proc user_name_from_klist {} {
    set uname ""
    if {![catch {exec klist} out]} {
        set look_for "Default principal: "
        foreach line [split $out "\n"] {
            set found [string first $look_for $line]
            if {$found >= 0} {
                set start [expr {$found + [string length $look_for]}]
                set principal [string range $line $start end]
                set pr_name [lindex [split $principal "@"] 0]
                set uname [lindex [split $pr_name "/"] 0]
                break
            }
        }
    }
    return $uname
}

proc ssh_user_name {} {
    global ssh_channel_user_name
    if {![info exists ssh_channel_user_name]} {
        set ssh_channel_user_name [user_name_from_klist]
        if {![string length $ssh_channel_user_name]} {
            global env
            set ssh_channel_user_name $env(USER)
        }
    }
    return $ssh_channel_user_name
}

proc unused_local_port {} {
    set chan [socket -server {} -myaddr "127.0.0.1" 0]
    set port [lindex [fconfigure $chan -sockname] 2]
    close $chan
    return $port
}

proc ssh_tunnel_errormessages {} {
    global ssh_tunnel_errormessages_list
    if {![info exists ssh_tunnel_errormessages_list]} {
        set errlist [list]
        #
        # Potential error messages which could be generated by ssh.
        # The first member in each pair indicates whether the problem
        # is "fatal" in a sense that it is no longer useful to retry
        # the connection.
        #
        lappend errlist 1 "Permission denied"
        lappend errlist 0 "bind: Address already in use"
        lappend errlist 0 "cannot listen to port"
        lappend errlist 0 "Could not request local forwarding"
        #
        set ssh_tunnel_errormessages_list $errlist
    }
    return $ssh_tunnel_errormessages_list
}

proc ssh_tunnel_gotline {handle chan} {
    if {[gets $chan nextline] >= 0} {
        foreach {status msg} [ssh_tunnel_errormessages] {
            if {[string first $msg $nextline] >= 0} {
                global make_ssh_tunnel_status
                set make_ssh_tunnel_status($handle) [list $status $msg]
                break
            }
        }
        global make_ssh_tunnel_readback
        append make_ssh_tunnel_readback($handle) $nextline "\n"
        global make_ssh_tunnel_waitvariable_$handle
        set make_ssh_tunnel_waitvariable_$handle 1
    }
    return
}

proc make_ssh_tunnel {handle host port} {
    set usename [ssh_user_name]
    set localport [unused_local_port]
    set cmd "ssh -L ${localport}:localhost:$port ${usename}@$host"
    set chan [open "|$cmd" "r+"]
    fconfigure $chan -buffering line
    fileevent $chan readable [list ssh_tunnel_gotline $handle $chan]

    global make_ssh_tunnel_status
    set make_ssh_tunnel_status($handle) [list]

    global make_ssh_tunnel_waitvariable_$handle
    set make_ssh_tunnel_waitvariable_$handle 0
    vwait make_ssh_tunnel_waitvariable_$handle

    global make_ssh_tunnel_status
    if {[llength $make_ssh_tunnel_status($handle)]} {
        set msg [lindex $make_ssh_tunnel_status($handle) 1]
        error "Failed to open ssh tunnel: $msg"
    }

    return [list $chan $localport]
}

proc establish_remote_channel {host port} {
    if {[string equal $host "localhost"] || \
            [string equal $host "127.0.0.1"] || \
            [string equal $host [info hostname]]} {
        return [list 0 "localhost" $port]
    }

    global remote_channel_handle_number
    if {![info exists remote_channel_handle_number]} {
        set remote_channel_handle_number 0
    }
    set h [incr remote_channel_handle_number]

    set success 0
    for {set itry 0} {$itry < 10} {incr itry} {
        if {![catch {make_ssh_tunnel $h $host $port} result]} {
            set success 1
            break
        }
        global make_ssh_tunnel_status
        foreach {fatal msg} $make_ssh_tunnel_status($h) break
        if {$fatal} break
        after [expr {int(50*pow(2.0,$itry))}]
    }

    if {$success} {
        foreach {chan localport} $result break
        global remote_channel_config
        set remote_channel_config($h,chan) $chan
        return [list $h "localhost" $localport]
    } else {
        remote_channel_cleanup $h
        error "Failed to establish channel to host $host, port $port : $result"
    }
}

proc remote_channel_cleanup {h} {
    global make_ssh_tunnel_readback
    catch {unset make_ssh_tunnel_readback($h)}
    global make_ssh_tunnel_status
    catch {unset make_ssh_tunnel_status($h)}
    global make_ssh_tunnel_waitvariable_$h
    catch {unset make_ssh_tunnel_waitvariable_$h}
    global remote_channel_config
    catch {unset remote_channel_config($h,chan)}
    return
}

proc close_remote_channel {h} {
    if {$h > 0} {
        global remote_channel_config
        if {[info exists remote_channel_config($h,chan)]} {
            set chan $remote_channel_config($h,chan)
            catch {puts $chan "exit"}
            catch {close $chan}
            remote_channel_cleanup $h
        }
    }
    return
}

proc remote_eval {host port args} {
    if {[llength $args] == 0} {
        error "No command specified"
    }
    set command [lindex $args 0]
    if {[string equal $command "bintransmit"]} {
        set binmode 1
        if {[llength $args] < 3} {
            error "No command or data to bintransmit"
        }
    } else {
        set binmode 0
    }
    foreach {rhandle host port} [establish_remote_channel $host $port] break
    if {[catch {socket $host $port} cid]} {
        close_remote_channel $rhandle
        error $cid
    }
    fconfigure $cid -buffering line
    if {$binmode} {
        set packed [lindex $args end]
        set length [string length $packed]
        puts $cid [lrange $args 0 end-1]
        puts $cid $length
        if {$length > 0} {
            fconfigure $cid -buffering full -translation binary
            catch {
                puts -nonewline $cid $packed
                flush $cid
            }
            fconfigure $cid -buffering line -translation auto
        }
    } else {
        puts $cid $args
    }
    set answer ""
    while {[gets $cid result_buffer] >= 0} {
        append answer $result_buffer
        if {[info complete $answer]} {
            break
        }
        append answer "\n"
    }
    set return_type [lindex $answer 0]
    if {[string equal $return_type "bintransmit"]} {
        if {[gets $cid length] < 0} {
            close $cid
            close_remote_channel $rhandle
            error "Premature bintransmit termination"
        }
        if {![string is integer -strict $length]} {
            close $cid
            close_remote_channel $rhandle
            error "Invalid bintransmit length specifier"
        }
        if {$length < 0} {
            close $cid
            close_remote_channel $rhandle
            error "Negative bintransmit length specifier"
        }
        if {$length > 0} {
            fconfigure $cid -buffering full -translation binary
            catch {read $cid $length} packed
            fconfigure $cid -buffering line -translation auto
            if {[eof $cid]} {
                close $cid
                close_remote_channel $rhandle
                error "Premature binary data termination"
            }
        } else {
            set packed [binary format ""]
        }
        close $cid
        close_remote_channel $rhandle
        # Just uncomment the following two lines if you want
        # the "bintransmit" keyword to be passed to the client
        #  lappend answer $packed
        #  return $answer
        if {[llength $answer] == 1} {
            return $packed
        } else {
            return [concat [lrange $answer 1 end] [list $packed]]
        }
    } else {
        close $cid
        close_remote_channel $rhandle
        if {[string equal $return_type "ok"]} {
            return [lindex $answer 1]
        } else {
            error $answer
        }
    }
}

# The following functions will work if the server enables
# commands hs::pack_item, hs::unpack_item, and serve_item
proc copy_item_to {local_id host port remote_prefix} {
    remote_eval $host $port bintransmit hs::unpack_item \
        $remote_prefix [hs::pack_item $local_id]
}

proc serve_item {id} {
    list bintransmit [hs::pack_item $id]
}

proc copy_item_from {host port remote_id local_prefix} {
    set packed [remote_eval $host $port serve_item $remote_id]
    hs::unpack_item $local_prefix $packed
}