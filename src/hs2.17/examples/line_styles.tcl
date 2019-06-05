
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init

set pi [expr atan2(0,-1)]
set xmax [expr 2*$pi]
set nbins 15
set binwidth [expr {$xmax / 1.0 / $nbins}]
set vshift 5.0

for {set i 1} {$i < 18} {incr i} {
    set id [hs::create_1d_hist [next_uid] "Dummy histogram"\
	    "Tmp" $i Y $nbins 0 $xmax]
    set data {}
    for {set j 0} {$j < $nbins} {incr j} {
	set x [expr {$j * $binwidth}]
	lappend data [expr {cos(2.0*$x) + $i * $vshift}]
    }
    hs::1d_hist_block_fill $id [hs::list_to_data $data]
    hs::overlay ov add $id
}
hs::overlay ov show clear -style histocolor -geometry 550x750 -xmax 6 -ymin 1 -legend on
