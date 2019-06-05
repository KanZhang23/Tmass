#!/bin/sh
# A simple example fit with an exclusion region
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate a simple histogram with errors
set id [random_histo gauss 50 -5 5 0.0 1.0 1000]
hs::hist_set_gauss_errors $id
hs::hist_set_error_range $id 1.0 {}

# Set up the fit with an exclusion region
set fit [hs::fit $id gauss]
$fit subset 0 config -filter {x < 0.5f || x > 2.5f}
$fit tune

package require rdl
rdl::interact 1
