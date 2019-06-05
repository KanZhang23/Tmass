
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded

# Start Histo-Scope
histo_init
set test_category [file tail [info script]]

# Ntuple creation
set nt_id1 [hs::create_ntuple [next_uid] "Example ntuple 1"\
        $test_category [list A B C D E]]

# Simple time series plot
foreach {varname color marker} {
    A red 1
    B green 2
    C blue 3
    D magenta 7
} {
    hs::overlay o1 add $nt_id1 ts -y $varname -color $color -marker $marker
}
hs::overlay o1 show

# Time series plot with errors
hs::overlay o2 show \
	add $nt_id1 tse -y B -ey A -color green \
	add $nt_id1 tse -y D -ey E -color magenta \
	add $nt_id1 tse -y E -color cyan -marker 6

# Fill the ntuple
for {set i 0} {$i < 100} {incr i} {
    set A [expr {rand() - 0.5}]
    set B [gauss_random 2.0 1.0]
    set C [gauss_random 4.0 0.5]
    set D [expr {6.0 + sin(0.251328 * $i)}]
    set E [gauss_random 0.0 1.0]
    hs::fill_ntuple $nt_id1 [list $A $B $C $D $E]
    hs::hs_update
    # after 50
}

# The easiest way to test simple xy plot is to use hs::list_plot
hs::list_plot {2 0 30 0 30 19 2 19 2 0}\
        -color red -line 12 -overlay o3
hs::list_plot {2 19 2 19 2 19 2 19 5 20.5 33 20.5 30 19 33 20.5\
        33 1.5 30 0} -color red -overlay o3
hs::list_plot {10 16 10 5 10 10.5 11.5 12 13.5 12 15 10.5 15 5}\
        -color blue -line 11 -overlay o3
hs::list_plot {17 6 18 5 21 5 22 6 22 8 21 9 18 9 17 10 17 11\
        18 12 21 12 22 11} -color blue -line 11 -overlay o3
hs::list_plot {11 11.5 11 16.5 10 16 10 5 11 5.5 11 11 10 10.5\
        11 11 12 12 11.5 12 12.5 12.5 14.5 12.5 13.5 12 14.5 12.5 16\
        11 16 5.5 15 5 15 10.5 16 11} -color blue -overlay o3
hs::list_plot {17 6 18 6.5 19 5.5 18 5 19 5.5 21.5 5.5 21 5 22\
        5.5 23 6.5 22 6 23 6.5 23 8.5 22 8 23 8.5 22 9.5 21 9 22\
        9.5 19 9.5 18 9 19 9.5 18 10.5 17 10 18 10.5 18 11.5 17\
        11 18 11.5 18.5 12 18 12 19 12.5 22 12.5 23 11.5 22 11 21\
        12 22 12.5} -color blue -overlay o3
hs::overlay o3 show -legend off

# Try the ntuple fitting algorithm
if {[::hs::have_cernlib]} {
    set nt_id2 [hs::create_ntuple [next_uid] "Example ntuple 2"\
	    $test_category [list X Data EY]]
    set EY 5.0
    for {set i 0} {$i <= 10} {incr i} {
	set y [gauss_random [expr {$i * $i}] $EY]
	hs::fill_ntuple $nt_id2 [list $i $y $EY]
    }
    foreach {coeffs sigma} [hs::ntuple_poly_fit $nt_id2 0 1 2] {}
    foreach {c b a} $coeffs {}
    hs::expr_plot {expr {$a*$x*$x + $b*$x + $c}} x -0.5 10.5 \
	    -color red -overlay o3a -line 11
    hs::overlay o3a add $nt_id2 xye -x X -y Data -ey EY \
	    -color blue -line 0 -marker solidcircle -markersize medium
    hs::overlay o3a show
}

# Ntuple for the xy plot with errors
set nt_id3 [hs::create_ntuple [next_uid] "Example ntuple 3"\
        $test_category [list A B C]]

hs::overlay o4 show clear\
        add $nt_id3 xye -x A -y A -ex- A -ey- A -ex+ B -ey+ B\
        -marker 7 -markersize 3\
        add $nt_id3 xye -x A -y A -ex- A -ey- A -ex+ B -ey+ B\
        -marker 7 -markersize 3\
        add $nt_id3 xye -x A -y C -ex B -ey B\
        -color red -marker 6 -markersize 3

for {set A 0} {$A <= 10} {incr A} {
    set B [expr {0.5 + 2.0*rand()}]
    set C [expr {0.5 * $A}]
    hs::fill_ntuple $nt_id3 [list $A $B $C]
}

set nt_id5 [hs::create_ntuple [next_uid] "Example ntuple 5"\
        $test_category [list A B C D]]

for {set A 0} {$A < 500} {incr A} {
    set r0 [gauss_random 0.0 1.0]
    set r1 [gauss_random 0.0 2.0]
    set D  [gauss_random 0.0 3.0]
    hs::fill_ntuple $nt_id5 [list $A [expr {$r0 + $r1}] [expr {$r0 - $r1}] $D]
}

# Overlay which mixes ntuples
hs::overlay o4 show \
        add $nt_id5 h1 -x B -nbins 50 -color green2 \
        add $nt_id1 h1 -x B -nbins 50 -color red

# Adaptive histograms
hs::overlay o5 show \
        add $nt_id5 h1a -x B -color green2 \
        add $nt_id5 h1a -x D -color red -binlimit 5
hs::hs_update

hs::overlay o6 show add $nt_id5 h2 -x B -y D
hs::overlay o7 show add $nt_id5 scat2 -x B -y D -geometry 600x500
hs::overlay o8 show add $nt_id5 scat3 -x B -y C -z D
hs::overlay o9 show add $nt_id5 cell -x B -y D
hs::overlay o10 show add $nt_id5 h2a -x B -y D
hs::overlay o11 show add $nt_id5 cscat2 -x B -y D -z A
hs::overlay o12 show add $nt_id5 tartan -x B -y D -nxbins 10 -nybins 10

hs::multiplot m1 show clear \
        add o1 0,0 -xlabel Time -ylabel Value \
        add o2 0,1 \
        add o3 1,0 -legend off \
        add o4 1,1 -legend off -xlabel X -ylabel Y
