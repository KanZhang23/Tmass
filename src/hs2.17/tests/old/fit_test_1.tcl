#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}
wm withdraw .

# Initialize Histo-Scope
set testdir [file dirname [info script]]
source [file join $testdir .. .. examples example_utils.tcl]
histo_init

# Generate a simple histogram
set id [random_histo gauss 50 -5 5 0.0 1.0 1000]
hs::hist_set_gauss_errors $id
hs::hist_set_error_range $id 1.0 {}

# Setup the fit using low-level functions 
# (not recommended for user programs)
fit::Fit_create f1 $id -title "Fitting a Gaussian"
fit::Fit_activate f1
fit::Fit_parameter add area -value 900 -step 50
fit::Fit_parameter add mean -value 0.2 -step 0.1
fit::Fit_parameter add sigma -value 0.9 -step 0.1
fit::Fit_function add gauss

# Example data filter
fit::Fit_subset 0 config -filter {x < 1 || x > 2}

# Perform the fit
fit::Fit_fit

# Draw the fit result -- the hard way, just to test that things work
set idn0 [hs::create_ntuple [next_uid] "Fit Result" "Test3" {x y}]
fit::Fit_subset 0 plotfit [list $idn0 {-5 5 1000}]
hs::overlay o1 show clear \
        add $id -errors on \
        add $idn0 xy -x x -y y -color red -line 11

# Draw the error contour
set idn0 [hs::create_ntuple [next_uid] "Contour 1" "Test3" {mean {Fit result}}]
set idn1 [hs::create_ntuple [next_uid] "Contour 1" "Test3" {mean {1 sigma}}]
set idn2 [hs::create_ntuple [next_uid] "Contour 2" "Test3" {mean {2 sigma}}]
set idn3 [hs::create_ntuple [next_uid] "Contour 3" "Test3" {mean {3 sigma}}]

# The fit result
hs::fill_ntuple $idn0 [list [fit::Fit_parameter mean cget -value] \
	[fit::Fit_parameter sigma cget -value]]

# Make Minuit silent
mn::comd set pri -1

# Get the error contours
mn::comd set err 1.0
mn::cont 2 3 400 $idn1
mn::comd set err 4.0
mn::cont 2 3 400 $idn2
mn::comd set err 9.0
mn::cont 2 3 400 $idn3

# Return to default verbosity level
mn::comd set pri 1
hs::overlay o1 show clear -legend on \
	-title "Fit error contours (width vs. mean)"\
        add $idn0 xy -x mean -y {Fit result} \
	    -color red -marker solidcircle -markersize medium -line 0 \
        add $idn1 xy -x mean -y {1 sigma} -color blue \
        add $idn2 xy -x mean -y {2 sigma} -color magenta \
        add $idn3 xy -x mean -y {3 sigma} -color green2

package require rdl
rdl::interact 1
