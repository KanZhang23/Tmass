
# This script needs Tk
package require Tk

# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init

# Generate an ntuple with a small number of random points
global nt_id
set nt_id [hs::create_ntuple [next_uid] "Example ntuple" "Tmp" [list Zero X]]
for {set i 0} {$i < 5} {incr i} {
    hs::fill_ntuple $nt_id [list 0 [gauss_random 0 1]]
}

# Pop a little GUI for modifying kernel and bandwidth
toplevel .kernel
wm title .kernel "Kernel density"
wm protocol .kernel WM_DELETE_WINDOW kernel_dismiss
label .kernel.label -text "Choose kernel:" -anchor w
pack .kernel.label -side top -fill x -padx 4 -pady 2
global kernel_type kernel_bandwidth
set kernel_type "gaussian"
set kernel_bandwidth 1.0
foreach kernel {gaussian uniform epanechnikov biweight triweight quadweight} {
    radiobutton .kernel.$kernel -text $kernel -selectcolor firebrick \
	    -anchor w -variable kernel_type -value $kernel -command update_plot
    pack .kernel.$kernel -side top -fill x -padx 20 -expand 1
}
scale .kernel.bwscale -from 0.0 -to 2.0 -orient horizontal \
	-label "Bandwidth:" -resolution 0.01 -command update_plot \
        -variable kernel_bandwidth -repeatinterval 10
pack .kernel.bwscale -side top -fill x -padx 4 -pady 2
frame .kernel.separator -height 2 -borderwidth 1 -relief sunken
pack .kernel.separator -side top -fill x
frame .kernel.buttons
pack .kernel.buttons -side top -fill x
button .kernel.buttons.new -text "New plot" -command new_plot
button .kernel.buttons.dismiss -text "Dismiss" -command kernel_dismiss
grid .kernel.buttons.new -row 0 -column 1 -pady 4
grid .kernel.buttons.dismiss -row 0 -column 3 -pady 4
foreach column {0 2 4} {
    grid columnconfigure .kernel.buttons $column -minsize 6 -weight 1
}

# Procedure to make a plot
global next_win_num
set next_win_num 0
proc new_plot {} {
    global nt_id kern_id next_win_num kernel_idlist
    set kern_id [hs::create_ntuple [next_uid] "Plot ntuple" "Tmp" [list x y]]
    hs::allow_reset_refresh $kern_id 0
    lappend kernel_idlist $kern_id
    set win wkern_[incr next_win_num]
    hs::overlay okern show clear\
	    -legend off -window $win -title "Kernel density estimator"\
	    add $nt_id xy -x X -y Zero -line 0 -marker star -markersize medium\
	    add $kern_id xy -x x -y y
    global kernel_winlist
    lappend kernel_winlist $win
    update_plot
}

# Procedure to update a plot
proc update_plot {args} {
    global kern_id nt_id kernel_type kernel_bandwidth
    hs::kernel_density_1d -kernel $kernel_type \
	    $nt_id X $kernel_bandwidth $kern_id -5 5 500
    hs::hs_update
}

# Procedure to clean things up
proc kernel_dismiss {} {
    destroy .kernel
    global kernel_idlist kernel_winlist nt_id kern_id \
	    kernel_type kernel_bandwidth next_win_num
    foreach win $kernel_winlist {
	hs::close_window $win 1
    }
    foreach id $kernel_idlist {
	hs::delete $id
    }
    hs::delete $nt_id
    unset kernel_idlist kernel_winlist nt_id kern_id \
	    kernel_type kernel_bandwidth next_win_num
}

# Plot the initial estimate
new_plot

