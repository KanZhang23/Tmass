#!/bin/sh
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}

wm withdraw .

# Load some utilities
set testdir [file dirname [info script]]
source [file join $testdir strd_utils.tcl]

# Initialize Histo-Scope
package require hs
hs::initialize "StRD test"
hs::histoscope 1
hs::wait_num_scopes > 0
hs::periodic_update 100

# Read in the file of results
set file_name "strd_results.hs"
if {[hs::read_file $file_name] <= 0} {
    exit 1
}

set nt_id [hs::id_from_title "StRD results" "StRD data"]
if {$nt_id < 0} {
    puts stderr "Ntuple of test results not found"
    exit 1
}

set id [hs::create_1d_hist [next_uid] "Deviation of the fitted value" \
	"Plots" "sigma" "N" 10000 -1 1]
hs::ntuple_project $nt_id $id {expr 1} {expr 1} \
	{expr {($fit_value - $cert_value)/$cert_error}}
hs::show_histogram $id

set id [hs::create_1d_hist [next_uid] "Deviation of the fitted error" \
	"Plots" "sigma" "N" 300 -3 3]
hs::ntuple_project $nt_id $id {expr 1} {expr 1} \
	{expr {($fit_error - $cert_error)/$cert_error}}
hs::show_histogram $id

package require rdl
rdl::interact
