proc histo_init {{init_string "Hs Example"}} {
    # Load the "hs" package
    package require hs
    # Initialize Histo-Scope API. There is no harm
    # in running "hs::initialize" more than once.
    hs::initialize $init_string
    # Check if there is a connected Histo-Scope GUI.
    if {[hs::num_connected_scopes] == 0} {
	# No GUI. (Re)start it.
	hs::histoscope 1
	hs::wait_num_scopes > 0
	hs::periodic_update 100
	hs::config_histoscope -minLineWidth 1
    }
    return
}

proc gauss_random {mean width} {
    expr {$mean + $width * (rand()+rand()+rand()+rand()+rand()+\
	  rand()+rand()+rand()+rand()+rand()+rand()+rand()-6.0)}
}

proc cauchy_random {peak HWHM} {
    expr {$peak + $HWHM * tan(3.1415926 * rand() - 1.57079632679)}
}

proc uniform_random {mean width} {
    expr {(rand() * 2.0 - 1.0) * 1.73205080757 * $width + $mean}
}

proc next_uid {} {
    global hs_examples_next_uid
    if {![info exists hs_examples_next_uid]} {
	set hs_examples_next_uid 0
    }
    incr hs_examples_next_uid
}

proc random_histo {type nbins xmin xmax mean width npoints} {
    set id [hs::create_1d_hist [next_uid] \
	    [string totitle "$type distribution sample"] \
	    "Random Histos" "X" "Y" $nbins $xmin $xmax]
    for {set i 0} {$i < $npoints} {incr i} {
	hs::fill_1d_hist $id [${type}_random $mean $width] 1.0
    }
    return $id
}

proc check_not_loaded {{message ""}} {
    global check_not_loaded_memory
    set file [file tail [info script]]
    if {[info exists check_not_loaded_memory($file)]} {
	puts "Sorry, can't source file $file more than once.\
		Please restart [file tail [info nameofexecutable]]."
	return -code return
    }
    set check_not_loaded_memory($file) 1
    if {![string equal message ""]} {
	puts $message
    }
    return
}

proc require_cernlib {} {
    set file [file tail [info script]]
    if {![::hs::have_cernlib]} {
	puts "Sorry, examples in the file $file can be executed only\
		when the hs extension is compiled with CERNLIB."
	return -code return
    }
    return
}
