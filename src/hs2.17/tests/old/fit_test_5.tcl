#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate a simple histogram
set id [hs::create_2d_hist [next_uid] "Example 2d histo"\
	"Tmp" x y f 40 40 -5 5 -5 5]
set npoints 10000
for {set i 0} {$i < $npoints} {incr i} {
    set a [gauss_random 0 1.0]
    set b [gauss_random 0 0.5]
    set x [expr {$a + $b}]
    set y [expr {$a - $b}]
    hs::fill_2d_hist $id $x $y 1
}

# Set up the fit
set fit [hs::fit $id bivar_gauss -method chisq]
$fit parameter volume config -value 600
$fit parameter rho set {0.0 variable 0.05 {-0.99 0.99}}
$fit fit
$fit plot

# Get the interactive prompt
package require rdl
rdl::interact 1
