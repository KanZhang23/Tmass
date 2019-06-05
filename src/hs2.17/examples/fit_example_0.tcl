
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init
require_cernlib

# Generate a simple histogram with errors
set id [random_histo gauss 50 -5 5 0.0 1.0 1000]
hs::hist_set_gauss_errors $id
hs::hist_set_error_range $id 1.0 {}

# Set up the fit
set fit [hs::fit $id gauss]
$fit tune
