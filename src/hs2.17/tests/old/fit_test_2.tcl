#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

set h1 [random_histo gauss 100 -10 10 0.0 1.0 1000]
set h2 [random_histo gauss 100 -10 10 1.0 2.0 2000]
set id [hs::sum_histograms [next_uid] "Sum" "Test3" $h1 $h2 1.0 1.0]
hs::hist_set_gauss_errors $id
hs::hist_set_error_range $id 1.0 {}

# Setup the fit
hs::function gauss copy gauss1
set fit [hs::fit $id {gauss gauss1} -title "Fitting a sum of two Gaussians"]
$fit tune

package require rdl
rdl::interact 1

