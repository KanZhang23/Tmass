
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init

set styles {none square circle star x triangle\
	solidsquare solidcircle thicksquare thickcircle}
set varnames {}
set istyle 0
foreach style $styles {
    lappend varnames "$style ($istyle)"
    incr istyle
}
set id [hs::create_ntuple [next_uid] "Marker Styles" "Tmp" $varnames]

set slope_step 0.01
set x0 -20
set y0 1
for {set j 0} {$j < 10} {incr j} {
    set data {}
    set istyle 1
    foreach sname $styles {
	set slope [expr {$slope_step * $istyle}]
	lappend data [expr {($j - $x0) * $slope + $y0}]
	incr istyle
    }
    hs::fill_ntuple $id $data
}
foreach style $varnames {
    hs::overlay ov add $id ts -y $style \
	    -marker [lindex $style 0] -markersize large
}
hs::overlay ov show clear -geometry 440x485 -title "Histo-Scope Marker Styles"
