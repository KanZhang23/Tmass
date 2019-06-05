
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded

# Initialize Histo-Scope
histo_init
require_cernlib

# Some script parameters
set nbins 1024
set halfbins [expr {$nbins / 2}]
set twopi [expr 2*atan2(0,-1)]

set m 0
foreach function {
    {exp(-$i/100.0)}
    {cos(257.5*$twopi*$i/1024.0)}
} {
    # Generate some histograms
    set id1 [hs::create_1d_hist [next_uid]\
	    $function "Originals" N F $nbins 0 $nbins]
    set id2 [hs::create_1d_hist [next_uid] "Fourier transform of\
	    \"[hs::title $id1]\"" "Fourier transforms" N F $nbins 0 $nbins]
    set id3 [hs::create_1d_hist [next_uid] "Fourier power spectrum"\
	    "Fourier transforms" N P $halfbins 1 [expr {$halfbins+1}]]
    set id4 [hs::create_1d_hist [next_uid] "Fourier phase spectrum"\
	    "Fourier transforms" N P $halfbins 1 [expr {$halfbins+1}]]
    for {set i 0} {$i < $nbins} {incr i} {
	set y [expr $function]
	hs::fill_1d_hist $id1 $i $y
    }
    hs::1d_fft $id1 $id2 1
    hs::1d_fourier_power $id2 $id3
    hs::1d_fourier_phase $id2 $id4
    hs::multiplot fft_test_3_[incr m] show -title "FFT of $function" \
	    add $id1 0,0 \
	    add $id2 1,0 -color blue \
	    add $id3 0,1 -color red \
	    add $id4 1,1 -color green
}

