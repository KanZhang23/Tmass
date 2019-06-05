
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init
require_cernlib

# Check that the extension has been compiled with CERNLIB
if {![hs::have_cernlib]} {
    puts "Sorry, can't run FFT examples:\
	    the hs extension has been compiled without CERNLIB."
    return
}

# Create couple histograms and do a convolution
set nbins 256
set events 10000
set id1 [random_histo gauss $nbins 0 $nbins [expr $nbins/2] 10 $events]
hs::change_category $id1 "Time domain"
set id2 [hs::create_1d_hist [next_uid] "Two pulses" \
	"Time domain" X Y $nbins 0 $nbins]
hs::fill_1d_hist $id2 42.5 1
hs::fill_1d_hist $id2 212.5 1

# Fourier transforms
set id3 [hs::copy_hist $id1 [next_uid] "Fourier transform of\
	\"[hs::title $id1]\"" "Frequency domain"]
set id4 [hs::copy_hist $id2 [next_uid] "Fourier transform of\
	\"[hs::title $id2]\"" "Frequency domain"]
hs::1d_fft $id1 $id3 1
hs::1d_fft $id2 $id4 1

# Convolution and reverse transform
set id_conv [hs::1d_fourier_multiply [next_uid] \
	"Convolution in the frequency domain" "Frequency domain" $id3 $id4]
set id_time [hs::copy_hist $id_conv [next_uid] \
	"Convolution in the time domain" "Time domain"]
hs::1d_fft $id_conv $id_time 0

# Now, try deconvolution
set id_decon [hs::1d_fourier_divide [next_uid] \
	"Deconvolution in the frequency domain" "Frequency domain" \
	$id_conv $id4 1]
set id_diff [hs::sum_histograms [next_uid] "Deconvolution error (frequency)"\
	"Frequency domain" $id_decon $id3 1 -1]

set id_dect [hs::copy_hist $id2 [next_uid] \
	"Deconvolution in the time domain" "Time domain"]
hs::1d_fft $id_decon $id_dect 0
set id_difft [hs::sum_histograms [next_uid] "Deconvolution error (time)"\
	"Time domain" $id_dect $id1 1 -1]

# Plot everything
hs::multiplot fft_m1 -geometry 600x800 -title "FFT examples" show \
	add $id1 0,0 \
	add $id3 1,0 -color red \
	add $id2 0,1 \
	add $id4 1,1 -color green \
	add $id_time 0,2 \
	add $id_conv 1,2 -color blue \
	add $id_dect 0,3 \
	add $id_decon 1,3 -color magenta \
	add $id_difft 0,4 \
	add $id_diff 1,4 -color cyan
