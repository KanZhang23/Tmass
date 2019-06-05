
proc next_uid {} {
    global hs_tests_next_uid
    if {![info exists hs_tests_next_uid]} {
	set hs_tests_next_uid 0
    }
    incr hs_tests_next_uid
}

proc fractional_difference {a b} {
    if {$a == $b} {return 0.0}
    expr {2.0 * abs($a - $b) / (abs($a) + abs($b))}
}

proc verify_similarity {args} {
    # Check that the items are identical within the specified
    # precision. UID and category are allowed to be different.
    set known_switches {-num_entries}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-num_entries)]} {
	set check_num_entries $options(-num_entries)
    } else {
	set check_num_entries 1
    }
    if {[llength $arglist] != 3} {
	error "wrong # of arguments or invalid option"
    }
    foreach {id1 id2 eps} $arglist {}
    if {![string is double -strict $eps]} {
	error "expected a non-negative double, got \"$eps\""
    } elseif {$eps < 0.0} {
	error "expected a non-negative double, got \"$eps\""
    }
    set type [::hs::type $id1]
    if {![string equal $type [::hs::type $id2]]} {
	error "types differ for items $id1 and $id2"
    }
    if {![string equal [::hs::title $id1] [::hs::title $id2]]} {
	error "titles differ for items $id1 and $id2"
    }
    # if {$id1 == $id2} return
    switch $type {
	HS_1D_HISTOGRAM {
	    if {[hs::1d_hist_num_bins $id1] != [hs::1d_hist_num_bins $id2]} {
		error "numbers of bins differ for 1d histograms $id1 and $id2"
	    }
	    foreach {x_min_1 x_max_1} [hs::1d_hist_range $id1] {}
	    foreach {x_min_2 x_max_2} [hs::1d_hist_range $id2] {}
	    if {[fractional_difference $x_min_1 $x_min_2] > $eps} {
		error "lower limits differ for 1d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $x_max_1 $x_max_2] > $eps} {
		error "upper limits differ for 1d histograms $id1 and $id2"
	    }
	    foreach {x_label_1 y_label_1} [hs::1d_hist_labels $id1] {}
	    foreach {x_label_2 y_label_2} [hs::1d_hist_labels $id2] {}
	    if {![string equal $x_label_1 $x_label_2]} {
		error "x labels differ for 1d histograms $id1 and $id2"
	    }
	    if {![string equal $y_label_1 $y_label_2]} {
		error "y labels differ for 1d histograms $id1 and $id2"
	    }
	    set errstat [hs::hist_error_status $id1]
	    if {$errstat != [hs::hist_error_status $id2]} {
		error "error status differs for 1d histograms $id1 and $id2"
	    }
	    set data1 [hs::1d_hist_bin_contents $id1]
	    set data2 [hs::1d_hist_bin_contents $id2]
	    set badbin [hs::find_data_mismatch $data1 $data2 $eps]
	    if {$badbin >= 0} {
		error "data mismatch in bin $badbin for 1d histograms $id1 and $id2"
	    }
	    foreach polarity {P N} {
		set e1 [hs::1d_hist_errors $id1 $polarity]
		set e2 [hs::1d_hist_errors $id2 $polarity]
		set badbin [hs::find_data_mismatch $e1 $e2 $eps]
		if {$badbin >= 0} {
		    error "$polarity error mismatch in bin\
			    $badbin for 1d histograms $id1 and $id2"
		}
	    }
	    foreach {u1 o1} [hs::1d_hist_overflows $id1] {}
	    foreach {u2 o2} [hs::1d_hist_overflows $id2] {}
	    if {[fractional_difference $u1 $u2] > $eps} {
		error "underflow bins differ for 1d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $o1 $o2] > $eps} {
		error "overflow bins differ for 1d histograms $id1 and $id2"
	    }
	    if {$check_num_entries} {
                if {[hs::num_entries $id1] != [hs::num_entries $id2]} {
                    error "number of entries differ for 1d histograms $id1 and $id2\
			([hs::num_entries $id1] != [hs::num_entries $id2])"
                }
	    }
	}
	HS_2D_HISTOGRAM {
	    foreach {n_x_bins_1 n_y_bins_1} [hs::2d_hist_num_bins $id1] {}
	    foreach {n_x_bins_2 n_y_bins_2} [hs::2d_hist_num_bins $id2] {}
	    if {$n_x_bins_1 != $n_x_bins_2} {
		error "numbers of x bins differ for 2d histograms $id1 and $id2"
	    }
	    if {$n_y_bins_1 != $n_y_bins_2} {
		error "numbers of y bins differ for 2d histograms $id1 and $id2"
	    }
	    foreach {x_min_1 x_max_1 y_min_1 y_max_1} [hs::2d_hist_range $id1] {}
	    foreach {x_min_2 x_max_2 y_min_2 y_max_2} [hs::2d_hist_range $id2] {}
	    if {[fractional_difference $x_min_1 $x_min_2] > $eps} {
		error "lower x axis limits differ for 2d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $x_max_1 $x_max_2] > $eps} {
		error "upper x axis limits differ for 2d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $y_min_1 $y_min_2] > $eps} {
		error "lower y axis limits differ for 2d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $y_max_1 $y_max_2] > $eps} {
		error "upper y axis limits differ for 2d histograms $id1 and $id2"
	    }
	    foreach {x_label_1 y_label_1 z_label_1} [hs::2d_hist_labels $id1] {}
	    foreach {x_label_2 y_label_2 z_label_2} [hs::2d_hist_labels $id2] {}
	    if {![string equal $x_label_1 $x_label_2]} {
		error "x labels differ for 2d histograms $id1 and $id2"
	    }
	    if {![string equal $y_label_1 $y_label_2]} {
		error "y labels differ for 2d histograms $id1 and $id2"
	    }
	    if {![string equal $z_label_1 $z_label_2]} {
		error "z labels differ for 2d histograms $id1 and $id2"
	    }
	    set errstat [hs::hist_error_status $id1]
	    if {$errstat != [hs::hist_error_status $id2]} {
		error "error status differs for 2d histograms $id1 and $id2"
	    }
	    set data1 [hs::2d_hist_bin_contents $id1]
	    set data2 [hs::2d_hist_bin_contents $id2]
	    set badbin [hs::find_data_mismatch $data1 $data2 $eps]
	    if {$badbin >= 0} {
		set xbin [expr {$badbin / $n_y_bins_1}]
		set ybin [expr {$badbin % $n_y_bins_1}]
		error "data mismatch in bin ($xbin,$ybin)\
			for 2d histograms $id1 and $id2"
	    }
	    foreach polarity {P N} {
		set e1 [hs::2d_hist_errors $id1 $polarity]
		set e2 [hs::2d_hist_errors $id2 $polarity]
		set badbin [hs::find_data_mismatch $e1 $e2 $eps]
		if {$badbin >= 0} {
		    set xbin [expr {$badbin / $n_y_bins_1}]
		    set ybin [expr {$badbin % $n_y_bins_1}]
		    error "$polarity error mismatch in bin\
			    ($xbin,$ybin) for 2d histograms $id1 and $id2"
		}
	    }
	    set i -1
	    foreach o1 [hs::2d_hist_overflows $id1] o2 [hs::2d_hist_overflows $id2] {
		if {[incr i] == 4} continue
		if {[fractional_difference $o1 $o2] > $eps} {
		    error "overflow bin $i differs for 2d histograms $id1 and $id2"
		}
	    }
	    if {$check_num_entries} {
                if {[hs::num_entries $id1] != [hs::num_entries $id2]} {
                    error "number of entries differ for 2d histograms $id1 and $id2\
			([hs::num_entries $id1] != [hs::num_entries $id2])"
                }
            }
	}
	HS_3D_HISTOGRAM {
	    foreach {n_x_bins_1 n_y_bins_1 n_z_bins_1} [hs::3d_hist_num_bins $id1] {}
	    foreach {n_x_bins_2 n_y_bins_2 n_z_bins_2} [hs::3d_hist_num_bins $id2] {}
	    if {$n_x_bins_1 != $n_x_bins_2} {
		error "numbers of x bins differ for 3d histograms $id1 and $id2"
	    }
	    if {$n_y_bins_1 != $n_y_bins_2} {
		error "numbers of y bins differ for 3d histograms $id1 and $id2"
	    }
	    if {$n_z_bins_1 != $n_z_bins_2} {
		error "numbers of z bins differ for 3d histograms $id1 and $id2"
	    }
	    foreach {x_min_1 x_max_1 y_min_1 y_max_1 z_min_1 z_max_1} \
		    [hs::3d_hist_range $id1] {}
	    foreach {x_min_2 x_max_2 y_min_2 y_max_2 z_min_2 z_max_2} \
		    [hs::3d_hist_range $id2] {}
	    if {[fractional_difference $x_min_1 $x_min_2] > $eps} {
		error "lower x axis limits differ for 3d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $x_max_1 $x_max_2] > $eps} {
		error "upper x axis limits differ for 3d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $y_min_1 $y_min_2] > $eps} {
		error "lower y axis limits differ for 3d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $y_max_1 $y_max_2] > $eps} {
		error "upper y axis limits differ for 3d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $z_min_1 $z_min_2] > $eps} {
		error "lower z axis limits differ for 3d histograms $id1 and $id2"
	    }
	    if {[fractional_difference $z_max_1 $z_max_2] > $eps} {
		error "upper z axis limits differ for 3d histograms $id1 and $id2"
	    }
	    foreach {x_label_1 y_label_1 z_label_1 v_label_1} [hs::3d_hist_labels $id1] {}
	    foreach {x_label_2 y_label_2 z_label_2 v_label_2} [hs::3d_hist_labels $id2] {}
	    if {![string equal $x_label_1 $x_label_2]} {
		error "x labels differ for 3d histograms $id1 and $id2"
	    }
	    if {![string equal $y_label_1 $y_label_2]} {
		error "y labels differ for 3d histograms $id1 and $id2"
	    }
	    if {![string equal $z_label_1 $z_label_2]} {
		error "z labels differ for 3d histograms $id1 and $id2"
	    }
	    if {![string equal $v_label_1 $v_label_2]} {
		error "v labels differ for 3d histograms $id1 and $id2"
	    }
	    set errstat [hs::hist_error_status $id1]
	    if {$errstat != [hs::hist_error_status $id2]} {
		error "error status differs for 3d histograms $id1 and $id2"
	    }
	    set data1 [hs::3d_hist_bin_contents $id1]
	    set data2 [hs::3d_hist_bin_contents $id2]
	    set badbin [hs::find_data_mismatch $data1 $data2 $eps]
	    if {$badbin >= 0} {
		set xbin [expr {$badbin / ($n_y_bins_1*$n_z_bins_1)}]
		set ybin [expr {($badbin % ($n_y_bins_1*$n_z_bins_1)) / $n_z_bins_1}]
		set zbin [expr {($badbin % ($n_y_bins_1*$n_z_bins_1)) % $n_z_bins_1}]
		error "data mismatch in bin ($xbin,$ybin,$zbin)\
			for 3d histograms $id1 and $id2"
	    }
	    foreach polarity {P N} {
		set e1 [hs::3d_hist_errors $id1 $polarity]
		set e2 [hs::3d_hist_errors $id2 $polarity]
		set badbin [hs::find_data_mismatch $e1 $e2 $eps]
		if {$badbin >= 0} {
		    set xbin [expr {$badbin / ($n_y_bins_1*$n_z_bins_1)}]
		    set ybin [expr {($badbin % ($n_y_bins_1*$n_z_bins_1)) / $n_z_bins_1}]
		    set zbin [expr {($badbin % ($n_y_bins_1*$n_z_bins_1)) % $n_z_bins_1}]
		    error "$polarity error mismatch in bin\
			    ($xbin,$ybin,$zbin) for 3d histograms $id1 and $id2"
		}
	    }
	    hs::3d_hist_overflows $id1 over1
	    hs::3d_hist_overflows $id2 over2
	    for {set i 0} {$i < 3} {incr i} {
		for {set j 0} {$j < 3} {incr j} {
		    for {set k 0} {$k < 3} {incr k} {
			if {[fractional_difference $over1($i,$j,$k) $over2($i,$j,$k)] > $eps} {
			    error "overflow bin ($i,$j,$k) differs for\
				    3d histograms $id1 and $id2"
			}
		    }
		}
	    }
	    if {$check_num_entries} {
                if {[hs::num_entries $id1] != [hs::num_entries $id2]} {
                    error "number of entries differ for 3d histograms $id1 and $id2\
			([hs::num_entries $id1] != [hs::num_entries $id2])"
                }
            }
	}
	HS_NTUPLE {
	    set ncolumns [hs::num_variables $id1]
	    if {$ncolumns != [hs::num_variables $id2]} {
		error "number of columns differ for ntuples $id1 and $id2"
	    }
	    set nrows [hs::num_entries $id1]
	    if {$nrows != [hs::num_entries $id2]} {
		error "number of rows differ for ntuples $id1 and $id2\
			($nrows != [hs::num_entries $id2])"
	    }
	    for {set col 0} {$col < $ncolumns} {incr col} {
		set v1 [hs::variable_name $id1 $col]
		set v2 [hs::variable_name $id2 $col]
		if {![string equal $v1 $v2]} {
		    error "names of column $col differ for ntuples $id1 and $id2"
		}
	    }
	    set data1 [hs::ntuple_contents $id1]
	    set data2 [hs::ntuple_contents $id2]
	    set badbin [hs::find_data_mismatch $data1 $data2 $eps]
	    if {$badbin >= 0} {
		set row [expr {$badbin / $ncolumns}]
		set col [expr {$badbin % $ncolumns}]
		error "data mismatch in row $row, column\
			$col for ntuples $id1 and $id2"
	    }
	}
	default {
	    error "Can't verify similarity for items\
		    $id1 and $id2 of type \"$type\""
	}
    }
    return
}

proc eval_at_precision {n script} {
    global tcl_precision errorInfo
    set old_precision $tcl_precision
    set tcl_precision $n
    if {[catch {uplevel $script} result]} {
        set savedInfo $errorInfo
        set tcl_precision $old_precision
        error $result $savedInfo
    }
    set tcl_precision $old_precision
    set result
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

proc random_histo {type nbins xmin xmax mean width npoints} {
    set id [hs::create_1d_hist [next_uid] \
            [string totitle "$type distribution sample"] \
            "Random Histos" "X" "Y" $nbins $xmin $xmax]
    for {set i 0} {$i < $npoints} {incr i} {
        hs::fill_1d_hist $id [${type}_random $mean $width] 1.0
    }
    return $id
}

proc assert_same_double {x y eps} {
    if {$x == $y} return
    if {$eps <= 0.0} {
	error "values are different (0 tolerance)"
    } else {
	if {[expr {(2.0*abs($x-$y))/(abs($x)+abs($y)) - $eps}] > 0.0} {
	    error "relative difference is larger than tolerance"
	}
    }
    return
}

proc assert_same_list {l1 l2 eps} {
    if {[llength $l1] != [llength $l2]} {
	error "lists have different lengths"
    }
    set index 0
    foreach e1 $l1 e2 $l2 {
	if {[catch {assert_same_double $e1 $e2 $eps} errmess]} {
	    append errmess " for list elements with index $index"
	    error $errmess
	}
	incr index
    }
    return
}

proc assert_same_matrix {x y eps} {
    set maxrel [hs::M $x maxreldiff $y]
    if {$maxrel == 0.0} return
    if {$eps <= 0.0} {
	error "matrices are different (0 tolerance)"
    } else {
	if {[expr {2.0*$maxrel - $eps}] > 0.0} {
	    error "maximum relative difference is larger than tolerance"
	}
    }
    return
}
