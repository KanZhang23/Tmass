#
# Proctrace arranges for the proc execution sequence
# to be printed on the standard output with time stamps.
# Not as powerful as Tcl_CreateTrace but still useful
# for taking a quick look at the sequence.
#
# Usage: proctrace on ?-mute? ?pattern_list?
#        proctrace off
#
# By default, all procs are traced except the proctrace
# itself. If a pattern list is specified then only those
# procedures whose names match one of the patterns are traced.
# The matching is performed with "string match" command.
# If -mute switch is used then only the procedures which
# do not match any of the patterns are traced.
# 
# Limitation: if proctrace is run not from the command
# prompt but from a script, the application must use
# the event loop.
# 
# Author: Igor Volobouev, ivolobouev@lbl.gov, 02/15/01
#

proc ::proctrace {args} {
    global proctrace_script_UnlIkELY_VARiAbLE_NaME
    if {[info exists proctrace_script_UnlIkELY_VARiAbLE_NaME]} {
	error "previous call is not complete"
    }
    # Check the first argument. Convert to boolean.
    set onoff [lindex $args 0]
    if {[string equal $onoff "on"]} {
	set onoff 1
    } elseif {[string equal $onoff "off"]} {
	set onoff 0
    }
    if {$onoff != 1 && $onoff != 0} {
	error "Invalid option \"$onoff\". Must be \"on\" or \"off\"."
    }
    set body_debug {
        # This is the script which will be executed before
        # each matching proc runs. It should be a single line.
	set debug_line {puts -nonewline "Proctrace @ [clock format\
		[clock seconds] -format {%T}]: ";\
		catch {puts -nonewline "[lindex [info level -1] 0] --> "};\
		puts "[info level 0]"}
    }
    global proctrace_guard_UnlIkELY_VARiAbLE_NaME
    if {$onoff} {
	if {[llength $args] > 3} {
	    error "wrong # of arguments"
	}
	if {[info exists proctrace_guard_UnlIkELY_VARiAbLE_NaME]} {
	    error "proc trace is already on"
	}
	# Parse subsequent arguments
	if {[llength $args] > 1} {
	    set arg1 [lindex $args 1]
	    if {[string equal $arg1 "-mute"]} {
		set mute 1
		if {[llength $args] != 3} {
		    error "wrong # of arguments"
		} else {
		    set name_patterns [lindex $args 2]
		}
	    } else {
		set mute 0
		set name_patterns $arg1
	    }
	} else {
	    set mute 1
	    set name_patterns {}
	}
	proc ::proctrace_existing_procs {namsp} {
	    namespace eval $namsp {
		set prefix "[namespace current]::"
		if {$prefix == "::::"} {
		    set prefix "::"
		}
		foreach shortname [info procs] {
		    set name ${prefix}$shortname
		    if {[string compare $name ::proctrace_existing_procs] && \
			    [string compare $name ::proctrace_proc] && \
			    [string compare $name ::proctrace]} {
			set arglist {}
			foreach argname [info args $name] {
			    if {[info default $name $argname value]} {
				lappend arglist [list $argname $value]
			    } else {
				lappend arglist $argname
			    }
			}
			::proctrace_proc $name $arglist [info body $name]
		    }
		}
		foreach child [namespace children] {
		    ::proctrace_existing_procs $child
		}
	    }
	    return
	}
	set body2 "foreach pattern \[list ${name_patterns}\] \\"
	if {$mute} {
	    set body3 {{
		    if {[string match $pattern $name]} {
			::proctrace_real_proc $name $arg $body
			return
		    }   
		}
		set newbody "${debug_line}\n$body"
		::proctrace_real_proc $name $arg $newbody
		return
	    }
	} else {
	    set body3 {{
		    if {[string match $pattern $name]} {
			set newbody "${debug_line}\n$body"
			::proctrace_real_proc $name $arg $newbody
			return
		    }   
		}
		::proctrace_real_proc $name $arg $body
		return
	    }
	}
	set fullbody [join [list $body_debug $body2 $body3] "\n"]
	proc ::proctrace_proc {name arg body} $fullbody
	rename ::proc ::proctrace_real_proc
	::proctrace_real_proc ::proc {name arg body} $fullbody
	# If we are not at level 1 then we have to replace the
        # bodies of various procedures from an "after" script.
        # Otherwise we probably have to preserve the procedures
        # from which this procedure was called up to the top level
        # of the stack.
	set script {
	    ::proctrace_existing_procs ::
	    rename ::proctrace_existing_procs {}
	    rename ::proctrace_proc {}
	    puts "proc trace is now on"
	    global proctrace_script_UnlIkELY_VARiAbLE_NaME
	    unset proctrace_script_UnlIkELY_VARiAbLE_NaME
	}
	set proctrace_script_UnlIkELY_VARiAbLE_NaME 1
	if {[info level] > 1} {
	    after idle $script
	} else {
	    eval $script
	}
	set proctrace_guard_UnlIkELY_VARiAbLE_NaME 1
    } else {
	if {[llength $args] > 1} {
	    error "wrong # of arguments"
	}
	if {[info exists proctrace_guard_UnlIkELY_VARiAbLE_NaME] == 0} {
	    error "proc trace is already off"
	}
	rename ::proc {}
	rename ::proctrace_real_proc ::proc
	proc ::proctrace_debug_line {} $body_debug
	proc ::proctrace_undo {namsp} {
	    namespace eval $namsp {
		set prefix "[namespace current]::"
		if {$prefix == "::::"} {
		    set prefix "::"
		}
		foreach shortname [info procs] {
		    set name ${prefix}$shortname
		    set bodylines [split [info body $name] "\n"]
		    if {[string equal [lindex $bodylines 0] \
			    [::proctrace_debug_line]]} {
			set newlines [lreplace $bodylines 0 0]
			set newbody [join $newlines "\n"]
			set arglist {}
			foreach argname [info args $name] {
			    if {[info default $name $argname value]} {
				lappend arglist [list $argname $value]
			    } else {
				lappend arglist $argname
			    }
			}
			proc $name $arglist $newbody
		    }
		}
		foreach child [namespace children] {
		    ::proctrace_undo $child
		}
	    }
	    return
	}
	set script {
	    ::proctrace_undo ::
	    rename ::proctrace_undo {}
	    rename ::proctrace_debug_line {}
	    puts "proc trace is now off"
	    global proctrace_script_UnlIkELY_VARiAbLE_NaME
	    unset proctrace_script_UnlIkELY_VARiAbLE_NaME
	}
	set proctrace_script_UnlIkELY_VARiAbLE_NaME 1
	if {[info level] > 1} {
	    after idle $script
	} else {
	    eval $script
	}
	unset proctrace_guard_UnlIkELY_VARiAbLE_NaME
    }
    return
}
