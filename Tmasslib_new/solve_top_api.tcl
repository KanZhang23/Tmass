proc import_functions {dlltoken} {
    hs::function_import taupt $dlltoken taupt \
        "Tau pt spectrum " 1 0 2 2 \
        {A mtop} {} tau_pt_spectrum_fcn {} {}
}

proc named_list_member {refnames name list} {
    if {[llength $refnames] != [llength $list]} {
        error "Bad list length: expected [llength $refnames],\
               got [llength $list]"
    }
    set index [lsearch -exact $refnames $name]
    if {$index < 0} {
        error "bad member \"$name\""
    }
    lindex $list $index
}

proc solve_leptonic_member {list name} {
    set refnames [list pblep bPx bPy bPz mb nux nuy nuz \
                      mwsq tlepz mt quality]
    named_list_member $refnames $name $list
}

proc solve_hadronic_member {list name} {
    set refnames [list qP qPx qPy qPz qbarP qbarPx \
                      qbarPy qbarPz bP bPx bPy bPz mq mqbar mb]
    named_list_member $refnames $name $list
}

proc minmax_member {list name} {
    set refnames [list minmax pblep bPx bPy bPz mb nux nuy nuz \
                      mwsq tlepz mt quality]
    named_list_member $refnames $name $list
}

proc create_1d_interpolator {id functname} {
    set nxbins [hs::1d_hist_num_bins $id]
    if {$nxbins < 2} {
	error "Need at least 2 bins"
    }
    foreach {xmin xmax} [hs::1d_hist_range $id] break
    set x_label [lindex [hs::1d_hist_labels $id] 0]
    set dx [expr {($xmax - $xmin)*0.5/$nxbins}]
    set xmin [expr {$xmin + $dx}]
    set xmax [expr {$xmax - $dx}]
    set data [hs::data_to_list [hs::1d_hist_bin_contents $id]]
    set hfile $functname.h
    set cfile $functname.c
    set header_def "[string toupper $functname]_H_"
    set chan [open $hfile "w"]
    puts -nonewline $chan \
	    [string map [list header_def $header_def \
	                      xvariable $x_label \
	                      functname $functname] \
{#ifndef header_def
#define header_def

#ifdef __cplusplus
extern "C" {
#endif
	    
double functname(double xvariable);
	    
#ifdef __cplusplus
}
#endif
	
#endif /* header_def */
}]
    close $chan

    set chan [open $cfile "w"]
    puts -nonewline $chan \
"#include \"$hfile\"
#include \"linear_interpolator_1d.h\"

#ifdef __cplusplus
extern \"C\" {
#endif

static const float ${functname}_data_data\[$nxbins\] = {
    "
    puts $chan [join $data ",\n    "]
    puts -nonewline $chan \
"};

static const Interpolator_data_1d ${functname}_data = {
    $nxbins,
    $xmin,
    $xmax,
    ${functname}_data_data
};

double ${functname}\(double xvariable\)
{
    return linear_interpolate_1d(&${functname}_data, (float)xvariable);
}

#ifdef __cplusplus
}
#endif
"
    close $chan
    return
}

proc create_2d_interpolator {id functname} {
    foreach {nxbins nybins} [hs::2d_hist_num_bins $id] break
    if {$nxbins < 2 || $nybins < 2} {
	error "Need at least 2 bins in each axis"
    }
    set allbins [expr {$nxbins*$nybins}]
    foreach {xmin xmax ymin ymax} [hs::2d_hist_range $id] break
    foreach {x_label y_label} [hs::2d_hist_labels $id] break
    set dx [expr {($xmax - $xmin)*0.5/$nxbins}]
    set dy [expr {($ymax - $ymin)*0.5/$nybins}]
    set xmin [expr {$xmin + $dx}]
    set xmax [expr {$xmax - $dx}]
    set ymin [expr {$ymin + $dy}]
    set ymax [expr {$ymax - $dy}]
    set data [hs::data_to_list [hs::2d_hist_bin_contents $id]]
    set hfile $functname.h
    set cfile $functname.c
    set header_def "[string toupper $functname]_H_"
    set chan [open $hfile "w"]
    puts -nonewline $chan \
	    [string map [list header_def $header_def \
	                      xvariable $x_label \
                              yvariable $y_label \
	                      functname $functname] \
{#ifndef header_def
#define header_def

#ifdef __cplusplus
extern "C" {
#endif
	    
double functname(double xvariable, double yvariable);
	    
#ifdef __cplusplus
}
#endif
	
#endif /* header_def */
}]
    close $chan

    set chan [open $cfile "w"]
    puts -nonewline $chan \
"#include \"$hfile\"
#include \"linear_interpolator_2d.h\"

#ifdef __cplusplus
extern \"C\" {
#endif

static const float ${functname}_data_data\[$allbins\] = {
    "
    puts $chan [join $data ",\n    "]
    puts -nonewline $chan \
"};

static const Interpolator_data_2d ${functname}_data = {
    $nxbins,
    $nybins,
    $xmin,
    $xmax,
    $ymin,
    $ymax,
    ${functname}_data_data
};

double ${functname}\(double xvariable, double yvariable\)
{
    return linear_interpolate_2d(&${functname}_data,
                                 (float)xvariable,
                                 (float)yvariable);
}

#ifdef __cplusplus
}
#endif
"
    close $chan
    return
}

proc p_tfs_to_file {filename n_pt_bins n_mjet_bins n_ratio_bins pt_max mjet_max} {
    set uid -2
    set category "p over E TFs"
    set id_3d [hs::create_3d_hist [incr uid] "Reference Histo" $category \
                   "Pt" "M parton" "p/E ratio" "TF value" \
                   $n_pt_bins $n_mjet_bins $n_ratio_bins \
                   0.0 $pt_max 0.0 $mjet_max 0.0 2.0]
    set etalist {0.0 0.5 1.0 1.5}
    set histo_list [list]
    foreach isB {0 1} qname {q b} {
        foreach eta $etalist {
            set id [hs::copy_hist $id_3d [incr uid] \
                        "$qname quark eta $eta" $category]
            transfer_function_scan $id 0.0 0.0 $eta $isB
            lappend histo_list $id
        }
    }
    set nsaved [hs::save_file_byids $filename $histo_list]
    hs::delete_category $category
    if {$nsaved == 8} {
        puts "Wrote file \"$filename\""
    } else {
        error "Failed to write file \"$filename\""
    }
    return
}

proc p_effs_to_file {filename n_pt_bins n_mjet_bins n_ratio_bins pt_max mjet_max} {
    set uid -2
    set category "p over E TF efficiencies"
    set id_3d [hs::create_3d_hist [incr uid] "Reference Histo" $category \
                   "Pt" "M parton" "p/E ratio" "Efficiency" \
                   $n_pt_bins $n_mjet_bins $n_ratio_bins \
                   0.0 $pt_max 0.0 $mjet_max 0.0 2.0]
    set etalist {0.0 0.5 1.0 1.5}
    set histo_list [list]
    foreach isB {0 1} qname {q b} {
        foreach eta $etalist {
            set id [hs::copy_hist $id_3d [incr uid] \
                        "$qname quark eta $eta" $category]
            transfer_function_efficiency_scan $id 0.0 0.0 $eta $isB
            lappend histo_list $id
        }
    }
    set nsaved [hs::save_file_byids $filename $histo_list]
    hs::delete_category $category
    if {$nsaved == 8} {
        puts "Wrote file \"$filename\""
    } else {
        error "Failed to write file \"$filename\""
    }
    return
}

proc p_invcdfs_to_file {filename n_pt_bins n_mjet_bins n_rnd_bins pt_max mjet_max} {
    # Make sure that the center of the first bin is at 0
    # and the center of the last bin is at 1.
    set bwz [expr {1.0/($n_rnd_bins - 1)}]
    set minz [expr {-0.5*$bwz}]
    set maxz [expr {1.0 + 0.5*$bwz}]
    set uid -2
    set category "p over E inverse CDFs"
    set id_3d [hs::create_3d_hist [incr uid] "Reference Histo" $category \
                   "Pt" "M parton" "CDF" "p/E ratio" \
                   $n_pt_bins $n_mjet_bins $n_rnd_bins \
                   0.0 $pt_max 0.0 $mjet_max $minz $maxz]
    set etalist {0.0 0.5 1.0 1.5}
    set histo_list [list]
    foreach isB {0 1} qname {q b} {
        foreach eta $etalist {
            set id [hs::copy_hist $id_3d [incr uid] \
                        "$qname quark eta $eta" $category]
            transfer_function_invcdf_scan $id 0.0 0.0 $eta $isB
            lappend histo_list $id
        }
    }
    set nsaved [hs::save_file_byids $filename $histo_list]
    hs::delete_category $category
    if {$nsaved == 8} {
        puts "Wrote file \"$filename\""
    } else {
        error "Failed to write file \"$filename\""
    }
    return
}

proc angle_tfs_to_file {filename n_pt_bins n_mjet_bins n_angle_bins pt_max mjet_max} {
    # The following number must be the same as MAX_DEVIATION
    # in file Configurables/angular_transfer_function.c
    set delta_max 0.7
    #
    set maxz [expr {$delta_max*($n_angle_bins - 0.001)/($n_angle_bins - 1)}]
    set minz [expr {-1.0 * $maxz}]
    set uid -2
    set category "Angle TFs"
    set id_3d [hs::create_3d_hist [incr uid] "Reference Histo" $category \
                   "Pt" "M parton" "Delta" "TF value" \
                   $n_pt_bins $n_mjet_bins $n_angle_bins \
                   0.0 $pt_max 0.0 $mjet_max $minz $maxz]
    set histo_list [list]
    foreach atype {eta phi} {
        foreach isB {0 1} qname {q b} {
            set id [hs::copy_hist $id_3d [incr uid] \
                        "$qname quark $atype" $category]
            angular_tf_scan $id $atype $isB
            lappend histo_list $id
        }
    }
    set nsaved [hs::save_file_byids $filename $histo_list]
    hs::delete_category $category
    if {$nsaved == 4} {
        puts "Wrote file \"$filename\""
    } else {
        error "Failed to write file \"$filename\""
    }
    return
}

proc angle_invcdfs_to_file {filename n_pt_bins n_mjet_bins n_rnd_bins pt_max mjet_max} {
    # Make sure that the center of the first bin is at 0
    # and the center of the last bin is at 1.
    set bwz [expr {1.0/($n_rnd_bins - 1)}]
    set minz [expr {-0.5*$bwz}]
    set maxz [expr {1.0 + 0.5*$bwz}]
    set uid -2
    set category "Angle inverse CDFs"
    set id_3d [hs::create_3d_hist [incr uid] "Reference Histo" $category \
                   "Pt" "M parton" "CDF" "Delta" \
                   $n_pt_bins $n_mjet_bins $n_rnd_bins \
                   0.0 $pt_max 0.0 $mjet_max $minz $maxz]
    set histo_list [list]
    foreach atype {eta phi} {
        foreach isB {0 1} qname {q b} {
            set id [hs::copy_hist $id_3d [incr uid] \
                        "$qname quark $atype" $category]
            angular_tf_invcdf_scan $id $atype $isB
            lappend histo_list $id
        }
    }
    set nsaved [hs::save_file_byids $filename $histo_list]
    hs::delete_category $category
    if {$nsaved == 4} {
        puts "Wrote file \"$filename\""
    } else {
        error "Failed to write file \"$filename\""
    }
    return
}
