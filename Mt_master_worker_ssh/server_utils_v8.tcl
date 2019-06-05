
namespace eval server {
    #
    # Useful user-callable procedures are:
    #
    # server::start
    # server::stop
    # server::config
    # server::cget
    # server::idlist
    # server::eval_addr
    # server::log_message
    # server::get_my_ip_addr
    # server::direct_ip_lookup
    # server::reverse_ip_lookup
    # server::is_safe_hostname
    # server::is_valid_ipv4_addr
    #
    # Also, it may be useful for the user to overwrite the server::log_message
    # procedure. Current server::log_message simply prints the messages to
    # the standard output with timestamps.
    #
    # This server is capable of multiplexing text and binary data on the same
    # socket. Passing of binary information is done with the help of a special
    # keyword "bintransmit". This keyword serves as an indicator that the last
    # element of the command executed or result returned is a binary object.
    # The keyword is dropped by the server when the command is passed to
    # the parser, but given to the client when the result is returned.

    # Figure out the type of the system we are running on
    proc systype {} {
        variable cached_systype
        if {![info exists cached_systype]} {
            set known_system_types [list "linux" "irix"]
            set mytype [exec uname]
            set match_found 0
            foreach type $known_system_types {
                if {[string match -nocase "*$type*" $mytype]} {
                    set match_found 1
                    set cached_systype $type
                    break
                }
            }
            if {!$match_found} {
                error "Unknown system type \"$mytype\",\
                       can't figure out how to do DNS lookup"
            }
        }
        set cached_systype
    }

    # A few system-dependent package parameters
    variable direct_ip_lookup_code
    variable reverse_ip_lookup_code
    switch [systype] {
        irix {
            # Direct ip lookup code uses variable "name" as input,
            # and should set variable "result" when done.
            set direct_ip_lookup_code {
                set lookup [exec /usr/sbin/nslookup -timeout=10 $name]
                if {[string first "can't find" $lookup] >= 0} {
                    error "host \"$name\" not found"
                }
                set lookup [string map {{ } {}} $lookup]
                set result [lindex [split $lookup ":\n"] end-1]
            }
            # Reverse ip lookup code uses variable "addr" as input,
            # and should set variable "result" when done.
            set reverse_ip_lookup_code {
                set lookup [exec /usr/sbin/nslookup -timeout=10 $addr]
                if {[string first "can't find" $lookup] >= 0} {
                    error "address \"$name\" not found"
                }
                set lookup [string map {{ } {}} $lookup]
                set result [lindex [split $lookup ":\n"] end-3]
            }
        }
        linux {
            set direct_ip_lookup_code {
                set lookup [exec /usr/bin/host -W 60 $name]
                if {[string first "not found" $lookup] >= 0} {
                    error "host \"$name\" not found"
                }
                set result ""
                foreach line [split $lookup "\n"] {
                    if {[string first "has address" $line] >= 0} {
                        set result [lindex [split $line] end]
                        break
                    }
                }
                if {[string equal $result ""]} {
                    error "failed to parse \"$lookup\""
                }
            }
            set reverse_ip_lookup_code {
                set lookup [exec /usr/bin/host -W 60 $addr]
                if {[string first "not found" $lookup] >= 0} {
                    error "address \"$addr\" not found"
                }
                set result [string range [lindex [split $lookup] end] 0 end-1]
            }
        }
        default {
            error "Missing switch item in [lindex [info level 0] 0]"
        }
    }

    # Initialize DNS lookup caches
    variable ipv4_address_to_host_lookup
    set ipv4_address_to_host_lookup(127.0.0.1) localhost
    variable ipv4_host_to_address_lookup
    set ipv4_host_to_address_lookup(localhost) 127.0.0.1

    # Address of the client whose command is currently being evaluated
    variable current_client_address
    set current_client_address ""

    proc start {id port maxclients timeout allowed parser {localonly 0}} {
        #
	# This is the main proc to start a server. It may be called once,
        # and then it may be called again with the same id only after
        # "server::stop". Several different servers using different ids
        # may be started at the same time. Note that the application MUST
        # enter the event loop some time after calling this proc.
        #
        # Arguments:
        #
        # id            A unique id string which identifies the server.
        #
	# port          The port number for the server. If 0 then
        #                  the port will be selected automatically.
        #                  In this case the port number may be retrieved
        #                  using the server::port procedure.
        #
        # maxclients    Maximum number of simultaneous connections allowed.
        #
        # timeout       Maximum allowed session time in seconds. Set to 0
        #                  to disable automatic logouts.
        #
        # allowed       The list of allowed hosts and/or ip addresses.
        #                  This argument can have the special value "all".
        #                  "localhost" can be an element of the list.
        #                  Wild cards can be used in host names or
        #                  addresses.
        #
        # parser        Interpreter to use for command evaluation.
        #                  It is allowed to be unsafe only when the
        #                  "allowed" argument is set to "localhost".
        #                  An empty string can be used to represent
        #                  the current interpreter.
        #
        # localonly     If "true", the server socket will be bound to
        #                  the loopback address, and only connections
        #                  from the local host will be allowed. In this
        #                  case, the list of allowed hosts is ignored.

        if {[string first {,[]\{\}\\} $id] >= 0} {
            error "\"$id\" is not a safe server id"
        }

	# Check if a server with this id is already running
        variable server_ids
        variable server_data
        if {[info exists server_ids($id)]} {
	    error "server \"$id\" is already running on port\
		    $server_data($id,port), handle $server_data($id,socket)"
	}

	# Check the arguments
	foreach name {port maxclients timeout} {
	    set value [set $name]
	    if {![string is integer -strict $value]} {
		error "expected an integer for $name argument, got \"$value\""
	    }
	    if {$value < 0} {
		error "$name argument can not be negative"
	    }
	}

        # Check that the command interpreter is safe
	if {![interp issafe $parser] && \
                [string compare $allowed "localhost"]} {
	    error "command interpreter is not safe"
	}

	# Start the server
        if {$localonly} {
            set server_data($id,socket) [socket -server \
                 [list server::accept $id] -myaddr "127.0.0.1" $port]
        } else {
            set server_data($id,socket) [socket -server \
                 [list server::accept $id] $port]
        }

	# Initialize various other internal variables
        set server_data($id,port) [lindex [fconfigure \
                $server_data($id,socket) -sockname] 2]
        set server_data($id,nclients)   0
        set server_data($id,maxclients) $maxclients
        set server_data($id,parser)     $parser
        set server_data($id,monitor)    0
        set server_data($id,logerrors)  1
        set server_data($id,logconnect) 0
        set server_data($id,localonly)  $localonly
        set server_data($id,auto_logout_msec) [expr {1000 * $timeout}]
        set server_ids($id) 1
        if {[catch {server::parse_allowed_hosts $id $allowed}]} {
            # This should never happen
            server::log_message "Failed to set allowed hosts from\
                    list \"$allowed\". Not safe to continue."
            exit 1
        }
	return
    }

    proc eval_addr {} {
        # Returns the address of the client whose command is currently
        # being evaluated, or an empty string if no command evaluation
        # is currently in progress.
        variable current_client_address
        set current_client_address
    }

    proc config {id args} {
        #
	# This procedure can be used to configure parameters of a running
        # server with given id on the fly. The following options are
        # acceptable: -timeout, -maxclient, -parser, -allowed.
        # The option meanings is the same as for the ::server::start
        # arguments. Additional options are:
        #
        #   -monitor     takes a boolean argument and can be used to turn on
        #                and off the detailed logging of the socket traffic.
        #
        #   -logerrors   takes a boolean argument. Can be used to turn on
        #                and off logging of all errors raised by the parser.
        #
        #   -logconnect  takes a boolean argument which tells whether to
        #                log a mesage for _all_ connections and disconnections.
        #                Unsuccessful connection attempts are always logged.
        #
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        set arglen [llength $args]
        if {[expr {$arglen % 2}]} {
            error "Wrong number of arguments"
        }

        foreach index {maxclients logerrors logconnect auto_logout_msec monitor} {
            set $index $server_data($id,$index)
        }

        set have_new_allowed_hosts 0
        set have_new_parser 0
        foreach {option value} $args {
            regsub "^-" $option "" option
            switch -exact -- $option {
                timeout {
                    if {[string is integer -strict $value]} {
                        set auto_logout_msec [expr {$value * 1000}]
                    } else {
                        error "bad value \"$value\" for option $option"
                    }
                }
                maxclients {
                    if {[string is integer -strict $value]} {
                        set maxclients $value
                    } else {
                        error "bad value \"$value\" for option $option"
                    }
                }
                logconnect -
                monitor -
                logerrors {
                    if {[string is boolean -strict $value]} {
                        set $option $value
                    } else {
                        error "bad value \"$value\" for option $option"
                    }
                }
                parser {
                    set have_new_parser 1
                    set parser $value
                }
                allowed {
                    set have_new_allowed_hosts 1
                    set new_allowed_hosts $value
                }
                default {
                    error "Invalid option name \"$option\""
                }
            }
        }

        if {$have_new_parser} {
            if {[interp issafe $parser]} {
                set server_data($id,parser) $parser
            } else {
                # Allow an unsafe interpreter for localhost only,
                # and only when no clients are connected already.
                set parser_changed 0
                if {$have_new_allowed_hosts} {
                    if {[string equal $new_allowed_hosts "localhost"]} {
                        if {$server_data($id,nclients)} {
                            error "Can't change to an unsafe interpreter\
                                      while clients are connected"
                        } else {
                            set server_data($id,parser) $parser
                            set parser_changed 1
                        }
                    }
                }
                if {!$parser_changed} {
                    error "\"$parser\" is not a safe interpreter"
                }
            }
        }
        foreach index {maxclients logerrors logconnect auto_logout_msec monitor} {
            set server_data($id,$index) [set $index]
        }
        if {$have_new_allowed_hosts} {
            parse_allowed_hosts $id $new_allowed_hosts
        }
        return
    }

    proc cget {id option} {
        #
        # Returns values of various configuration options.
        # Can also return:
        #
        # the number of currently connected clients using
        # the "-nclients" option
        #
        # port number using the "-port" option
        #
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        regsub "^-" $option "" option
        switch -exact -- $option {
            timeout {
                set value [expr {$server_data($id,auto_logout_msec)/1000}]
            }
            port -
            nclients -
            maxclients -
            monitor -
            logerrors -
            logconnect -
            parser {
                set value $server_data($id,$option)
            }
            allowed {
                set value {}
                foreach name [array names server_data "$id,allowed_ip_address,*"] {
                    set host [lindex [split $name ,] 2]
                    lappend value $host
                }
                set value [concat $value $server_data($id,allowed_hostnames)]
            }
            default {
                error "Invalid option name \"$option\""
            }
        }
        return $value
    }

    proc idlist {} {
        variable server_ids
        array names server_ids
    }

    proc stop {id {message_to_all_clients ""}} {
        #
	# This proc closes all connections and stops listening
        # on the server socket. $message_to_all_clients is sent
        # to all clients if it is not an empty string.
        #
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        set cidlist {}
        foreach index [array names server_data "$id,cid_to_addr,*"] {
            lappend cidlist [lindex [split $index ,] end]
        }
        if {[string compare $message_to_all_clients ""]} {
            if {$server_data($id,monitor)} {
                server::log_message "$id sends\
                   \"$message_to_all_clients\" to $cidlist"
            }
            foreach cid $cidlist {
                puts $cid $message_to_all_clients
            }
        }
        foreach cid $cidlist {
            server::disconnect $id $cid
        }
        catch {close $server_data($id,socket)}
        array unset server_data "$id,*"
        unset server_ids($id)
	return
    }

    proc get_my_ip_addr {} {
	variable my_ip_address
	if {![info exists my_ip_address]} {
	    set sockid [socket -server server::get_my_ip_and_close 0]
	    set port [lindex [fconfigure $sockid -sockname] 2]
	    set tmpsock [socket [info hostname] $port]
	    vwait server::my_ip_address
	    catch {close $tmpsock}
	    catch {close $sockid}
	}
	return $my_ip_address
    }

    proc get_my_ip_and_close {cid addr port} {
	variable my_ip_address $addr
	catch {close $cid}
        return
    }

    proc accept {id cid addr port} {
	# Check the ip address of the connecting host
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
	if {![server::is_host_allowed $id $addr]} {
            catch {puts $cid [list error "Connections\
                    from your host are not allowed"]}
            catch {close $cid}
            server::log_message "$id refused $addr $port: host is not allowed"
            return
	}
	# Check the number of connected clients
	if {$server_data($id,nclients) >= $server_data($id,maxclients)} {
	    catch {puts $cid [list error "Too many\
                    clients. Please try again later."]}
	    catch {close $cid}
	    server::log_message "$id refused $addr $port: too many clients"
	    return
	}
	# Set up the command handler
	fileevent $cid readable [list server::handle $id $cid]
	fconfigure $cid -buffering line -blocking 0
        if {$server_data($id,logconnect)} {
            server::log_message "$id connected $addr $port, handle $cid"
        }
	incr server_data($id,nclients)
        set server_data($id,buffer,$cid) ""
	if {$server_data($id,auto_logout_msec) > 0} {
	    set server_data($id,afterid,$cid) [after \
                    $server_data($id,auto_logout_msec) \
		    [list server::autologout $id $cid]]
	}
	set server_data($id,cid_to_addr,$cid) $addr
        return
    }

    # The following proc works only on line-buffered channels
    proc gets_with_timeout {cid varname timeout} {
        upvar $varname x
        set maxtime [expr {[clock seconds] + $timeout}]
        set count [gets $cid x]
        while {$count < 0 && [clock seconds] < $maxtime} {
            after 50
            set count [gets $cid x]
        }
        return $count
    }

    proc handle {id cid} {
        variable server_ids
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        if {[eof $cid]} {
	    server::disconnect $id $cid
	} elseif {[gets $cid request] >= 0} {
            # Channel is not blocked
            variable server_data
	    append server_data($id,buffer,$cid) $request "\n"
	    set tmp $server_data($id,buffer,$cid)
	    if {[info complete $tmp]} {
                set monitor $server_data($id,monitor)
                set logerrors [expr {$server_data($id,logerrors) || $monitor}]

                # Evaluate the command
                set local $server_data($id,buffer,$cid)
		set server_data($id,buffer,$cid) ""                
                if {[string equal [lindex $local 0] "bintransmit"]} {
                    if {[llength $local] == 1} {
                        set errmess "no command to bintransmit"
                        if {$logerrors} {
                            server::log_message "$id error \{$errmess\} with $cid"
                        }
                        puts $cid [list error $errmess]
                        return
                    }
                    # The next line should represent the length
                    # of the subsequent binary object
                    if {[server::gets_with_timeout $cid length 30] < 0} {
                        server::log_message "$id premature bintransmit termination"
                        server::disconnect $id $cid
                        return
                    }
                    if {![string is integer -strict $length]} {
                        set errmess "invalid bintransmit length specifier"
                        if {$logerrors} {
                            server::log_message "$id error \{$errmess\} with $cid"
                        }
                        puts $cid [list error $errmess]
                        return
                    }
                    if {$length < 0} {
                        set errmess "negative bintransmit length specifier"
                        if {$logerrors} {
                            server::log_message "$id error \{$errmess\} with $cid"
                        }
                        puts $cid [list error $errmess]
                        return
                    }
                    if {$length > 0} {
                        fconfigure $cid -buffering full -translation binary -blocking 1
                        catch {read $cid $length} packed
                        fconfigure $cid -buffering line -translation auto -blocking 0
                        if {[eof $cid]} {
                            server::log_message "$id premature binary data termination"
                            server::disconnect $id $cid
                            return
                        }
                    } else {
                        set packed [binary format ""]
                    }
                    set command [lrange $local 1 end]
                    if {$monitor} {
                        server::log_message "$id evaluates\
                            \"$command <binary_string>\" for $cid"
                    }
                    variable current_client_address
                    set current_client_address $server_data($id,cid_to_addr,$cid)
                    set status [catch {interp eval $server_data($id,parser) \
                                           $command [list $packed]} result]
                    set current_client_address ""
                } else {
                    if {$monitor} {
                        server::log_message "$id evaluates\
                            \"[string trimright $tmp]\" for $cid"
                    }
                    variable current_client_address
                    set current_client_address $server_data($id,cid_to_addr,$cid)
                    set status [catch {interp eval $server_data($id,parser) $tmp} result]
                    set current_client_address ""
                }
                if {$status} {
                    if {$logerrors} {
                        server::log_message "$id error \{$result\} with $cid"
                    }
		    puts $cid [list error $result]
                    return
		}

                # Return the result
                set tmp $result
                if {[string equal [lindex $tmp 0] "bintransmit"]} {
                    # The last element of the list should be a binary string
                    if {[llength $tmp] < 2} {
                        set errmess "bad bintransmit format"
                        if {$logerrors} {
                            server::log_message "$id error \{$errmess\} with $cid"
                        }
                        puts $cid [list error $errmess]
                        return
                    }
                    set packed [lindex $tmp end]
                    if {$monitor} {
                        server::log_message "$id sends \"[lrange $tmp 0 end-1]\" to $cid"
                    }
                    puts $cid [lrange $tmp 0 end-1]
                    set len [string length $packed]
                    if {$monitor} {
                        server::log_message "$id sends \"$len\" to $cid"
                    }
                    puts $cid $len
                    if {$len > 0} {
                        if {$monitor} {
                            server::log_message "$id sends <binary string> to $cid"
                        }
                        fconfigure $cid -buffering full -translation binary -blocking 1
                        catch {
                            puts -nonewline $cid $packed
                            flush $cid
                        }
                        fconfigure $cid -buffering line -translation auto -blocking 0
                    }
                } else {
                    if {$monitor} {
                        server::log_message "$id sends \"[list ok $result]\" to $cid"
                    }
                    puts $cid [list ok $result]
                }
	    }
	}
        return
    }

    proc autologout {id cid} {
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        unset server_data($id,afterid,$cid)
        if {$server_data($id,monitor)} {
            server::log_message "$id connection with $cid timed out"
        }
	# puts $cid [list error "Connection timed out"]
	server::disconnect $id $cid
        return
   }

    proc log_message {message} {
	puts "[clock format [clock seconds] -format {%D %T}] : $message"
	flush stdout
        return
    }

    proc disconnect {id cid} {
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
	catch {close $cid}
        if {$server_data($id,logconnect)} {
            server::log_message "$id disconnected\
                    $server_data($id,cid_to_addr,$cid),\
                    handle $cid"
        }
        unset server_data($id,cid_to_addr,$cid)
        unset server_data($id,buffer,$cid)
	if {[info exists server_data($id,afterid,$cid)]} {
	    after cancel $server_data($id,afterid,$cid)
	    unset server_data($id,afterid,$cid)
	}
        incr server_data($id,nclients) -1
        return
    }

    proc parse_allowed_hosts {id allowed_hosts} {
        variable server_data
        if {$server_data($id,localonly)} {
            # Only connections from the local host will be allowed anyway
            parse_allowed_hosts_real $id "all"
        } else {
            parse_allowed_hosts_real $id $allowed_hosts
        }
    }

    proc parse_allowed_hosts_real {id allowed_hosts} {
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
        set server_data($id,check_ip_addr) 1
        array unset server_data "$id,allowed_ip_address,*"
        set server_data($id,allowed_hostnames) {}
	if {[string equal $allowed_hosts "all"]} {
	    set server_data($id,check_ip_addr) 0
	} else {
            foreach addr $allowed_hosts {
                if {[server::is_valid_ipv4_addr $addr]} {
                    set server_data($id,allowed_ip_address,$addr) 1
                } elseif {[string equal $addr "localhost"]} {
                    set server_data($id,allowed_ip_address,127.0.0.1) 1
                    set my_ip [server::get_my_ip_addr]
                    set server_data($id,allowed_ip_address,$my_ip) 1
                } else {
                    lappend server_data($id,allowed_hostnames) $addr
                }
            }
	}
        return
    }

    proc is_host_allowed {id addr} {
        variable server_ids
        variable server_data
        if {![info exists server_ids($id)]} {
            error "Invalid server id \"$id\""
        }
	if {$server_data($id,check_ip_addr)} {
	    if {[info exists server_data($id,allowed_ip_address,$addr)]} {
		return 1
	    }
            if {[llength $server_data($id,allowed_hostnames)] > 0} {
		# Pattern matching will be performed in parts.
		# Parts are separated by dots. Match the host
		# name only if we can perform both direct and
		# reverse ip lookup.
		set reverse_lookup_failed [catch \
			{server::reverse_ip_lookup $addr} clienthost]
		if {$reverse_lookup_failed} {
		    set address_components [list [split $addr .]]
		} else {
		    # If reverse lookup succeeded then the direct lookup
		    # must succeed as well, and must return the same address
		    if {[catch {server::direct_ip_lookup $clienthost} clientaddr]} {
			return 0
		    }
		    if {![string equal $addr $clientaddr]} {
			return 0
		    }
		    set address_components [list [split $addr .] [split $clienthost .]]
		}

		# Match either host name or host address
		# to the provided patterns
		foreach hostlist $address_components {
		    foreach pattern $server_data($id,allowed_hostnames) {
			set patlist [split $pattern .]
			if {[llength $hostlist] == [llength $patlist]} {
			    set matches 1
			    foreach hostpart $hostlist patpart $patlist {
				if {![string match -nocase $patpart $hostpart]} {
				    set matches 0
				    break
				}
			    }
			    if {$matches} {
				set server_data($id,allowed_ip_address,$addr) 1
				if {$reverse_lookup_failed} {
				    # Come up with a surrogate name for this host
				    set surrogate "host-at-$addr"
				    variable ipv4_host_to_address_lookup
				    set ipv4_host_to_address_lookup($surrogate) $addr
				    variable ipv4_address_to_host_lookup
				    set ipv4_address_to_host_lookup($addr) $surrogate
				}
				return 1
			    }
			}
		    }
		}
            }
            return 0
	} else {
            return 1
        }
    }

    proc direct_ip_lookup {name} {
        # IPv4 lookup only
        variable ipv4_host_to_address_lookup
        if {![info exists ipv4_host_to_address_lookup($name)]} {
            if {![server::is_safe_hostname $name]} {
                error "\"$name\" is not a safe host name for DNS lookup"
            }
            variable direct_ip_lookup_code
            eval $direct_ip_lookup_code
            if {![server::is_valid_ipv4_addr $result]} {
                error "DNS lookup failed for host $name"
            }
            set ipv4_host_to_address_lookup($name) $result
        }
        set ipv4_host_to_address_lookup($name)
    }

    proc reverse_ip_lookup {addr} {
        # IPv4 lookup only
        variable ipv4_address_to_host_lookup
        if {![info exists ipv4_address_to_host_lookup($addr)]} {
            if {![server::is_valid_ipv4_addr $addr]} {
                error "\"$addr\" is not a valid IPv4 address"
            }
            variable reverse_ip_lookup_code
            eval $reverse_ip_lookup_code
            if {![server::is_safe_hostname $result]} {
                error "DNS lookup failed for address $addr"
            }
            set ipv4_address_to_host_lookup($addr) $result
        }
        set ipv4_address_to_host_lookup($addr)
    }

    proc is_valid_ipv4_addr {addr} {
        set parts [split $addr .]
        if {[llength $parts] != 4} {
            return 0
        }
        foreach part $parts {
            if {![string is integer -strict $part]} {
                return 0
            }
            if {$part < 0 || $part >= 256} {
                return 0
            }
        }
        return 1
    }

    proc is_safe_hostname {name} {
        set parts [split $name .]
        if {[llength $parts] == 0} {
            return 0
        }
        foreach part $parts {
            # Allow dash inside the name
            set part [string map {- {}} $part]
            if {![string is alnum -strict $part]} {
                return 0
            }
            if {![string is ascii -strict $part]} {
                return 0
            }
        }
        return 1
    }
}
