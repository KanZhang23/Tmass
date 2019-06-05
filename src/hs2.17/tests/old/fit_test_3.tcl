#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate several datasets each of which is a sum of a wider
# gaussian (background) and a narrower Breight-Wigner (signal).
# The mean and the width of the signal is the same for
# every dataset, but the number of events is different.

set peak 2.0
set hwhm 0.5
set nbins 100

set i 0
set idlist {}
foreach {bgevt bgpeak bgsigma sigevt} {
    2000 -6 4 500
    5000 -1 5 1000
    4000  0 3 1000
    3000  4 6 500
} {
    set bg_id [random_histo gauss $nbins -10 10 $bgpeak $bgsigma $bgevt]
    set sig_id [random_histo cauchy $nbins -10 10 $peak $hwhm $sigevt]
    lappend idlist [hs::sum_histograms [next_uid] \
            "Dataset $i" "Test" $bg_id $sig_id 1 1]
    incr i
}

# Setup the fit
set fit [hs::fit $idlist {} -method chisq -parameters none \
        -title "Fitting several datasets"]
$fit parameter add peak -value 0
$fit parameter add width -value 0.2
set i 0
foreach id $idlist {
    $fit parameter add sigarea$i -value 20
    $fit parameter add area$i -value 100
    $fit parameter add mean$i -value 0
    $fit parameter add sigma$i -value 3
    hs::function gauss copy gauss$i
    $fit function add gauss$i -subset $i -map [subst {
        area = %area$i;
        mean = %mean$i;
        sigma = %sigma$i;
    }]
    hs::function cauchy copy cauchy$i
    $fit function add cauchy$i -subset $i -map [subst {
        area = %sigarea$i;
        peak = %peak;
        hwhm = %width;
    }]
    incr i
}

$fit tune

package require rdl
rdl::interact 1
