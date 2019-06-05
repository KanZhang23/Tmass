hs::sharedlib open -export_globals ./libJetUser.so
source ./jet_corrections_api.tcl

set level 5
set nvertices 1
set coneSize 0
set version 5
set syscode 0
set run 0
set imode 0

init_generic_corrections 0 $level $nvertices $coneSize \
    $version $syscode $run $imode

proc calculate_syserr {icorr pt detectorEta} {
    set emf 0.3
    set_sys_total_correction $icorr 0
    set s1 [generic_correction_scale $icorr $pt $emf $detectorEta]
    set_sys_total_correction $icorr 1
    set s2 [generic_correction_scale $icorr $pt $emf $detectorEta]
    expr {($s2 - $s1)/$s1}
}

set ptmin 25.0
set ptmax 28.0
set eta 1.5

set id [hs::create_1d_hist [next_uid] "Systematic Uncertainty" "" \
       pt0 Uncertainty 1000 $ptmin $ptmax]
set id1 [hs::create_1d_hist [next_uid] "Standard Correction" "" \
       pt0 Correction 1000 $ptmin $ptmax]
set id2 [hs::create_1d_hist [next_uid] "Shifted Correction" "" \
       pt0 Correction 1000 $ptmin $ptmax]

set c1 [list]
set c2 [list]
set uncert [list]
foreach pt [hs::data_to_list [hs::1d_hist_bin_coords $id]] {
    set_sys_total_correction 0 0
    lappend c1 [generic_correction_scale 0 $pt 0.3 $eta]
    set_sys_total_correction 0 1
    lappend c2 [generic_correction_scale 0 $pt 0.3 $eta]
    lappend uncert [generic_correction_uncertainty 0 $pt $eta 0]
}
hs::1d_hist_block_fill $id $uncert
hs::1d_hist_block_fill $id1 $c1
hs::1d_hist_block_fill $id2 $c2
hs::show_histogram $id

return

generic_correction_uncertainty 0 26.31 $eta
generic_correction_uncertainty 0 26.33 $eta

corrector_level 0

generic_correction_scale 0 30 0.1 1
set_sys_total_correction 0 1
generic_correction_scale 0 30 0.1 1
set_sys_total_correction 0 0
generic_correction_scale 0 30 0.1 1
set_sys_total_correction 0 -1
generic_correction_scale 0 30 0.1 1
set_sys_total_correction 0 0
generic_correction_scale 0 30 0.1 1

hs::function l0pt eval {x 12} {icorr 0} {detEta 1} {emf 0.3}
