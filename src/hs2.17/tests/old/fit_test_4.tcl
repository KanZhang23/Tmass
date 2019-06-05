#!/bin/sh
# Test 2d unbinned extended max. likelihood fit
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate some random points
set id1 [hs::create_ntuple [next_uid] "Random points" "Tmp" [list x y z]]
set npoints 1000
for {set i 0} {$i < $npoints} {incr i} {
    set x [gauss_random 2 1]
    set a [gauss_random 0 1.5]
    set b [gauss_random 0 0.5]
    set y [expr {$a + $b}]
    set z [expr {$a - $b}]
    hs::fill_ntuple $id1 [list $x $y $z]
}

set fit [hs::fit [list $id1 -x y -y z -method eml] bivar_gauss]
$fit subset 0 config -normreg {-5 5 100 -5 5 100}
$fit parameter rho set {0.0 variable 0.1 {-0.999 0.999}}
$fit tune

package require rdl
rdl::interact
