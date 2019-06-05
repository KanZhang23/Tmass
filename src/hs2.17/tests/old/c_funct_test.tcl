#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

set id [hs::create_1d_hist [next_uid] "Gauss" "Functions" x gauss 100 -5 5]
hs::show_histogram $id

hs::function gauss scan [list $id x] {area 1} {mean 0.5} {sigma 1.5}
hs::function cauchy scan [list $id x] {area 1} {peak 0.5} {hwhm 0.5}

set idn [hs::create_ntuple [next_uid] "Example Ntuple" "Functions" {x f}]
hs::overlay o1 show add $idn xy -x x -y f

hs::function gauss scan [list $idn {x -5 5 1000}] {area 1} {mean 0.5} {sigma 1.5}

set id2 [hs::create_2d_hist [next_uid] "Some 2d histo" "Functions"\
        x y f 100 100 -5 5 -5 5]
hs::show_histogram $id2
hs::function gauss scan [list $id2 x sigma] {area 1} {mean 0}
hs::function bifgauss scan [list $id2 x sigma_r] {area 1} {peak 0} {sigma_l 1}

set id3 [hs::create_2d_hist [next_uid] "Some 2d histo" "Functions"\
        x y f 51 51 -5 5 -5 5]
hs::show_histogram $id3
hs::function gauss scan [list $id3 x sigma] {area 1} {mean 0}

set idn3 [hs::create_ntuple [next_uid] "Example Ntuple 2" "Functions" {x y f}]
hs::overlay o2 show add $idn3 scat3 -x x -y y -z f
hs::function bifgauss scan [list $idn3 {x -5 5 100} {sigma_r -5 5 100}] {area 1} {peak 0} {sigma_l 1}

hs::function landau scan [list $idn {x -5 25 30}] {area 1} {peak 0.5} {width 1.5}
set idn_copy [hs::copy_hist $idn [next_uid] "Copy Ntuple" Functions]
hs::reset $idn_copy
hs::function data_1d config -mode $idn
hs::function data_1d scan [list $idn_copy {x -10 10 2000}] {h_scale 1} {h_shift 0} {v_scale 1} {v_shift 0.01}
hs::overlay o3 show add $idn_copy xy -x x -y f

# Test function combinations
set idn_combo [hs::copy_hist $idn [next_uid] "Copy Ntuple 2" Functions]
hs::overlay o4 show add $idn_combo xy -x x -y f

set combo0 [hs::function_compose gauss 0.5 x 2]
hs::function $combo0 configure -parameters {area mean sigma}
hs::function $combo0 scan [list $idn_combo {x -10 10 2000}] {area 1} {mean 0} {sigma 1}

set combo1 [hs::function_sum 1.0 gauss 1.0 cauchy]
hs::function $combo1 configure -parameters {area1 mean1 sigma1 area2 peak2 sigma2}
hs::function $combo1 scan [list $idn_combo {x -10 10 2000}] \
        {area1 1} {mean1 2} {sigma1 1} {area2 1} {peak2 -2} {sigma2 1}

package require rdl
rdl::interact
