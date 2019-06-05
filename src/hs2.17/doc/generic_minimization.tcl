proc ::fit::Fit_process_gm {name args} {
    if {[llength $args] > 0} {
        set action [lindex $args 0]
        switch -- $action {
            setargs {
                ::fit::Fit_activate $name
                return [eval ::fit::Fit_tcl_fcn_args [lrange $args 1 end]]
            }
            del -
            delete {
                if {[llength $args] != 1} {
                    error "wrong # of arguments"
                }
                ::fit::Fit_rename $name {}
                ::rename [lindex [info level -1] 0] {}
                return
            }
        }
    }
    eval ::fit::Fit_process [list $name] $args
}

proc generic_minimizer_use_dll {dll function_name} {
    # Create and activate the fit
    set fitname [::fit::Fit_next_name]
    set procname ::$fitname
    while {[lsearch -exact [info commands $procname] $procname] >= 0} {
        set fitname [::fit::Fit_next_name]
        set procname ::$fitname
    }
    ::fit::Fit_create $fitname
    ::fit::Fit_activate $fitname

    # Associate the function to minimize with the fit
    global errorInfo
    set status [catch {
        ::fit::Fit_tcl_fcn $fitname $function_name $dll
    } errMess]
    set localError $errorInfo
    if {$status} {
        ::fit::Fit_rename $fitname {}
        error $errMess $localError
    }

    # Associate a command with the fit
    proc $procname {args} "eval ::fit::Fit_process_gm [list $fitname] \$args"
    return $procname
}

proc generic_minimizer {file_name function_name} {
    package require hs

    # Compile the C function to minimize. Make sure to clean up on errors.
    foreach {sharedlib chan} [hs::tempfile "/tmp/" ".so"] {}
    close $chan
    global errorInfo
    set status [catch {
        hs::sharedlib_compile [list $file_name] $sharedlib
        set dll [hs::sharedlib open $sharedlib]
    } errMess]
    set localError $errorInfo
    file delete $sharedlib
    if {$status} {
        error $errMess $localError
    }

    set status [catch {
        generic_minimizer_use_dll $dll $function_name
    } procname]
    set localError $errorInfo
    if {$status} {
        hs::sharedlib close $dll
        error $procname $localError
    }

    # Make sure the dll is unloaded when the fit is deleted
    ::fit::Fit_append_dll $dll

    return $procname
}
