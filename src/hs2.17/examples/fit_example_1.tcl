
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded
histo_init "Simultaneous fit of several datasets"
require_cernlib

# First, let's simulate the background
set xmin 0.0
set xmax 5.0
set nbins 50
set binwidth [expr {($xmax - $xmin)/$nbins}]
foreach {num a b c} {
    0  1  1  40
    1  1 -1  40
    2 -1  1  40
    3 -1 -1  40
} {
    set idn($num) [hs::create_1d_hist [expr {$num+10}] \
        "Dataset $num" "Tmp" "X" "Y" $nbins $xmin $xmax]
    set data {}
    for {set bin 0} {$bin < $nbins} {incr bin} {
	set x [expr {$bin * $binwidth}]
	set mean [expr {$a*$x*$x + $b*$x + $c}]
	set value [gauss_random $mean [expr {sqrt($mean)}]]
	if {$value < 0.0} {set value 0.0}
	lappend data $value
    }
    hs::1d_hist_block_fill $idn($num) [hs::list_to_data $data]
}

# The following code simulates the signal
set peak 2.0
set width 0.3
foreach {num nevents} {0 400  1 800  2 600  3 900} {
    for {set event 0} {$event < $nevents} {incr event} {
	hs::fill_1d_hist $idn($num) [cauchy_random $peak $width] 1.0
    }
}

# Create the fit. We will use chi-square minimization this time.
set idlist [list $idn(0) $idn(1) $idn(2) $idn(3)]
set fit [hs::fit $idlist {} -title "Fitting several datasets" -method chisq]

# Add the parameters for the fitted peak. Pretend that
# we don't know the real parameter values.
$fit parameter add peak -value 3
$fit parameter add width -value 0.2

# Go over each dataset and add fitting functions and parameters
set i 0
foreach id $idlist {
    foreach {name init_value} {a 1 b 1 c 20 signal 10} {
        $fit parameter add ${name}_$i -value $init_value
    }
    hs::function poly_1d copy poly_1d$i
    hs::function poly_1d$i configure -mode 4
    $fit function add poly_1d$i -subset $i -map [subst {
	x0 = 0.0;
	a0 = %c_$i;
	a1 = %b_$i;
	a2 = %a_$i;
    }]
    hs::function cauchy copy cauchy$i
    $fit function add cauchy$i -subset $i -map [subst {
        area = %signal_$i;
        peak = %peak;
        hwhm = %width;
    }]
    incr i
}

$fit tune
