#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate a set of points which will be fitted
set xmin 1.0
set xmax 3.0
set npoints 100
set id [hs::create_ntuple [next_uid] "Set of Points" "Tmp" {x y e}]
for {set i 0} {$i < $npoints} {incr i} {
    set x [expr {$xmin + rand()*($xmax - $xmin)}]
    set e [expr {0.5*(1.0 + rand()*$x)}]
    set shift [gauss_random 0 $e]
    set y [expr {2.0*$x*$x + $shift}]
    hs::fill_ntuple $id [list $x $y $e]
}

# Set up the fit
set fit [hs::fit [list $id -x x -v y -e e] poly_1d]
$fit parameter x0 config -state fixed
$fit tune

# Get the interactive prompt
package require rdl
rdl::interact 1
