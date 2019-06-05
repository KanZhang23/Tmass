
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init
require_cernlib

# Create an ntuple and fill it with random numbers
set id [hs::create_ntuple [next_uid] "Random ntuple" "Random Histos" {x}]
for {set i 0} {$i < 100} {incr i} {
    hs::fill_ntuple $id [list [gauss_random 3.0 2.0]]
}

# Set up the fit
set fit [hs::fit [list $id -x x] gauss -method ml]
$fit parameter area config -state fixed
$fit tune
