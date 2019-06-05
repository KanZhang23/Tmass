
# Check that we have the shared library loaded
if {[catch ::rdl::tcl_api_version]} {
    error "Can not load readline Tcl interface.\
	    Please check your installation."
    return
} else {
    package provide rdl [lindex [::rdl::tcl_api_version] 0]
}

namespace eval ::rdl:: {
    variable olddir [pwd]
    variable host [lindex [split [info hostname] .] 0]
    variable rdl_prompt1 "[file tail [info nameofexecutable]]% "
    variable rdl_prompt2 "> "
    variable got_partial 0
    variable interactive 0
    variable first_interaction 1
    variable cmdline ""

    variable term
    if [info exists env(TERM)] {
	set term $env(TERM)
    } else {
	set term unknown
    }

    namespace export {[a-z]*}
}

###########################################################################

proc ::rdl::add_namespace {nsp} {
    foreach cmd [info commands ${nsp}::*] {
	rdl::add_completion [string trimleft $cmd :]
    }
    foreach child [namespace children $nsp] {
	rdl::add_namespace $child
    }
    return
}

###########################################################################

proc ::rdl::add_path {args} {
    global env
    set gooddirs [split $env(PATH) :]
    foreach path [concat $args] {
	if [file isdirectory $path] {
	    if {[lsearch -exact $gooddirs $path] >= 0} {
		foreach filename [glob -nocomplain -directory $path *] {
		    if [file executable $filename] {
			rdl::add_completion [file tail $filename]
		    }
		}
	    }
	} elseif [file executable $path] {
	    if {[lsearch -exact $gooddirs [file dirname $path]] >= 0} {
		rdl::add_completion [file tail $path]
	    }
	}
    }
    return
}

###########################################################################

proc ::rdl::prompt1 {} {
    variable rdl_prompt1
    return $rdl_prompt1
}

###########################################################################

proc ::rdl::prompt2 {} {
    variable rdl_prompt2
    return $rdl_prompt2
}

###########################################################################

proc ::rdl::interact {{force_event_loop {}}} {
    variable interactive
    if {!$interactive} {
	variable first_interaction
	if {$first_interaction} {
	    set first_interaction 0
	    uplevel #0 {namespace import rdl::ls rdl::rm}
	    rdl::use_xtermcd
	    rdl::add_namespace ::
	    global env
	    rdl::add_path /bin /usr/bin $env(HOME)/bin
	}
	global tcl_interactive
	set old_tcl_interactive $tcl_interactive
	set tcl_interactive 1
	# The next command will never return unless the user explicitly
	# sets the rdl::interactive variable to 0 
	rdl::input_loop $force_event_loop
        set tcl_interactive $old_tcl_interactive
    }
    return
}

###########################################################################

proc ::rdl::input_loop {{force_event_loop {}}} {

    if {$force_event_loop == {}} {
        # Automatic mode. Enter event loop if we are running
        # wish, use classic readline if we are in tclsh.
	set do_event_loop [llength [info globals tk_version]]
    } else {
	set do_event_loop $force_event_loop
    }

    if {[string compare [info script] ""]} {
	# We are running from a script. The problem here is that the
        # standard "unknown" procedure will not go through all its
        # motions if it detects that it is run from a script. Try
        # to patch it on the fly -- I am too lazy at the moment to
        # write a better handler.
	set goodcondition {([string equal [info script] ""] ||\
		[string equal [info script] }
	append goodcondition [info script]
	append goodcondition {])}
	regsub -all {\[string equal \[info script\] ""\]}\
		[info body unknown] $goodcondition newbody
	proc ::unknown {args} $newbody
    }

    variable interactive
    set interactive 1
    if {$do_event_loop} {
	# Use callback mechanism
	fconfigure stdin -buffering none
	rdl::callback_handler_install [rdl::prompt1]
	fileevent stdin readable rdl::callback_read_char
	while {$interactive} {
	    vwait rdl::interactive
	}
	fconfigure stdin -buffering line
        rdl::callback_handler_remove
	fileevent stdin readable {}
    } else {
	# Use classic readline API
	set have_partial 0
	set comline ""
	while {$interactive} {
	    if {$have_partial} {
		set prompt [rdl::prompt2]
	    } else {
		set prompt [rdl::prompt1]
	    }
	    if {[catch {rdl::readline $prompt} line]} {
		exit
	    }
	    append comline $line "\n"
	    if {[info complete $comline]} {
		catch {uplevel #0 $comline} result
		if {[string compare "" $result]} {
		    puts $result
		    flush stdout
		}
		set have_partial 0
		set comline ""
	    } else {
		set have_partial 1
	    }
	}
    }
    return
}

###########################################################################

proc ::rdl::cd {{newdir {}}} {
    set tmpdir [pwd]
    variable oldcd
    if {[llength $newdir] > 0} {
	eval $oldcd $newdir
    } else {
	eval $oldcd
    }
    variable olddir $tmpdir
    variable term
    if [string equal $term xterm] {
	variable host
	set cwd [pwd]
	set bname [file tail $cwd]
	puts -nonewline "\]1;${host}::${bname}\]2;${host} ${cwd}"
	flush stdout
    }
    return
}

###########################################################################

proc ::rdl::back {} {
    variable olddir
    rdl::cd $olddir
    return
}

###########################################################################

proc ::rdl::use_xtermcd {} {
    if {[info exists ::rdl::oldcd] == 0} {
	variable oldcd oldcd_kJGkjhljyUYkbgDK
	namespace eval :: {
	    rename cd oldcd_kJGkjhljyUYkbgDK
	    namespace import rdl::cd rdl::back
	}
    }
    return
}

###########################################################################

proc ::rdl::ls {args} {
    set input [eval ::rdl::globber $args]
    catch {uplevel #0 [eval concat exec /bin/ls -C $input]} result
    puts $result
    flush stdout
    return
}

###########################################################################

proc ::rdl::rm {args} {
    set redir ">&@stdout <@stdin"
    set input [eval ::rdl::globber $args]
    catch {uplevel #0 [eval concat exec $redir /bin/rm -i $input]} result
    if {[string compare "" $result]} {
	puts $result
	flush stdout
    }
    return
}

###########################################################################

proc ::rdl::globber {args} {
    set result ""
    set parse_flags 1
    foreach element [concat $args] {
	if {[string compare "" $element]} {
	    if {$parse_flags && [string equal "-" [string index $element 0]]} {
		lappend result $element
	    } else {
		set parse_flags 0
		set subst [glob -nocomplain -- $element]
		if {[string equal "" $subst]} {
		    lappend result $element
		} else {
		    lappend result $subst
		}
	    }
	}
    }
    return $result
}

###########################################################################

proc ::rdl::line_callback {line} {
    variable got_partial
    variable cmdline
    # Remove the readline callback before processing the command 
    # line because the command itself may try to read stdin
    rdl::callback_handler_remove
    append cmdline $line "\n"
    if {[info complete $cmdline]} {
	catch {uplevel #0 $cmdline} result
	if {[string compare "" $result]} {
	    puts $result
	    flush stdout
	}
	set got_partial 0
	set cmdline ""
	set prompt [rdl::prompt1]
    } else {
	set got_partial 1
	set prompt [rdl::prompt2]
    }
    rdl::callback_handler_install $prompt
    return
}

###########################################################################

proc ::rdl::eof_callback {} {
    exit
}

###########################################################################

proc ::rdl::callback_error_handler {who message} {
    puts stderr "ERROR in ${who}: $message"
    flush stderr
    return
}

