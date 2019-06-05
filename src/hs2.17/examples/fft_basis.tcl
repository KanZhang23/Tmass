
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init
require_cernlib

set nbins 4
for {set i 0} {$i < $nbins} {incr i} {
    set id [hs::create_1d_hist [next_uid] "Peak at $i" \
	    "Time domain" N F $nbins 0 $nbins]
    set id_f [hs::create_1d_hist [next_uid] "Fourier transform of peak at $i" \
	    "Fourier transforms" N F $nbins 0 $nbins]
    hs::fill_1d_hist $id [expr {$i+0.5}] 1.0
    hs::1d_fft $id $id_f 1
    hs::multiplot fft_test_2_m1 add $id 0,$i -color red add $id_f 1,$i -color blue

    set id [hs::create_1d_hist [next_uid] "Basis function $i" \
	    "Time domain" N F $nbins 0 $nbins]
    set id_f [hs::create_1d_hist [next_uid] "Frequency $i" \
	    "Fourier transforms" N F $nbins 0 $nbins]
    hs::fill_1d_hist $id_f [expr {$i+0.5}] 1.0
    hs::1d_fft $id_f $id 0
    hs::multiplot fft_test_2_m2 add $id 0,$i -color red add $id_f 1,$i -color blue
}
hs::multiplot fft_test_2_m1 show -title "FFT of basis functions -- part 1 of 2"
hs::multiplot fft_test_2_m2 show -title "FFT of basis functions -- part 2 of 2"
