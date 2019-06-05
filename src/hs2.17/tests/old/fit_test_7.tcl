#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate a set of points which will be fitted
set xmin -1.0
set xmax 1.0
set ymin -1.0
set ymax 1.0
set npoints 500

# Coefficients of the polynomial:
foreach {a b c d e f} {6.0 5.0 4.0 3.0 2.0 1.0} {}

set id [hs::create_ntuple [next_uid] "Set of Points" "Tmp" {x y v e}]
for {set i 0} {$i < $npoints} {incr i} {
    set x [expr {$xmin + rand()*($xmax - $xmin)}]
    set y [expr {$ymin + rand()*($ymax - $ymin)}]
    set err [expr {0.2 * (0.1 + rand())}]
    set shift [gauss_random 0 $err]
    set v [expr {$a*$x*$x + $b*$x*$y + $c*$y*$y + $d*$x + $e*$y + $f + $shift}]
    hs::fill_ntuple $id [list $x $y $v $err]
}

# Set up the fit
set fit [hs::fit [list $id -x x -y y -v v -e e] quadratic_2d]
$fit tune

# Get the interactive prompt
package require rdl
rdl::interact 1
