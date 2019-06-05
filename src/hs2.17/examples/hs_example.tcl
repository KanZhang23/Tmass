
package require hs

# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded

set test_category [file tail [info script]]

# Initialize Histo-Scope
hs::initialize "Histo-Scope Tcl API v[hs::tcl_api_version] "

######################################################
#                                                    #
#   Test the Histo-Scope GUI start/stop sequence     #
#                                                    #
######################################################
puts "Testing Histo-Scope GUI start/stop sequence"

# Kill all GUIs which may have been opened 
# before this file is sourced
hs::kill_histoscope
hs::wait_num_scopes == 0

# Start the Histo-Scope GUI
hs::histoscope 1

# Wait until the GUI window pops up
hs::wait_num_scopes > 0

# Kill the GUI
hs::kill_histoscope
hs::wait_num_scopes == 0

# Start another GUI
hs::histoscope 1
hs::wait_num_scopes > 0

# Set up general Histo-Scope options
hs::config_histoscope -bufferGraphics off \
        -updatePeriod 100 -autoHelp on

# (Re)start periodic updates
hs::periodic_update 100

######################################################
#                                                    #
#          Ntuple creation and filling               #
#                                                    #
######################################################
puts "Testing ntuple creation and filling"

# Ntuple creation
if {[catch {
    hs::create_ntuple [next_uid] "Example ntuple" $test_category [list A B C D]
} ntuple_id]} {
    puts stderr "ERROR: ntuple creation failed!! Further tests aborted."
    return
}
hs::hs_update

# Ntuple filling
for {set i 0} {$i < 20} {incr i} {
    set B [expr rand()]
    set C [gauss_random 2 1]
    set D [cauchy_random 0 1]
    hs::fill_ntuple $ntuple_id [list $i $B $C $D]
}

# Ntuple ASCII dump
hs::ascii_dump stdout $ntuple_id

# Ntuple filtering
set newid [hs::ntuple_filter $ntuple_id [next_uid]\
        "Tcl filtered ntuple" $test_category {expr $A < 10}]
if {[hs::num_entries $newid] != 10} {
    puts stderr "ERROR: tcl ntuple filtering failed!!"
}
set newid [hs::ntuple_c_filter $ntuple_id [next_uid]\
        "C filtered ntuple" $test_category {A < 10}]
if {[hs::num_entries $newid] != 10} {
    puts stderr "ERROR: C ntuple filtering failed!!"
}
hs::hs_update

# Ntuple projection
set newid [hs::create_ntuple [next_uid] "Tcl projected ntuple" \
        $test_category [list CminusD CplusD]]
hs::ntuple_project $ntuple_id $newid {expr $A < 10} \
        {expr $C - $D} {expr $C + $D}
if {[hs::num_entries $newid] != 10} {
    puts stderr "ERROR: tcl ntuple projection failed!!"
}
set newid [hs::create_ntuple [next_uid] "C projected ntuple" \
        $test_category [list CminusD CplusD]]
hs::ntuple_c_project $ntuple_id $newid {A < 10} {C - D} {C + D}
if {[hs::num_entries $newid] != 10} {
    puts stderr "ERROR: C ntuple projection failed!!"
}
hs::hs_update

# Fill ntuple with a reasonably large number of entries to make
# subsequent projections
puts -nonewline "Filling ntuple with 5000 random entries. Please wait ... "
flush stdout
for {set i 0} {$i < 4980} {incr i} {
    set B [expr rand()]
    set C [gauss_random 2 1]
    set D [cauchy_random 0 1]
    hs::fill_ntuple $ntuple_id [list $i $B $C $D]
}
puts "Done"
hs::hs_update

######################################################
#                                                    #
#        1d histogram creation and filling           #
#                                                    #
######################################################
puts "Testing creation and filling of 1d histograms"

# Create several 1d histograms 
if {[catch {
    hs::create_1d_hist [next_uid] "Example 1d histogram"\
            $test_category "X1" "Y" 10 0 10
} id1]} {
    puts stderr "ERROR: 1d histogram creation failed!! Further tests aborted."
    return
}
set id10 [hs::create_1d_hist [next_uid] "Another 1d histogram"\
        $test_category "X2" "Y" 10 0 10]
set id4 [hs::create_1d_hist [next_uid] "Gaussian distribution"\
        $test_category "X" "Y" 40 0 10]
set id5 [hs::create_1d_hist [next_uid] "Uniform distribution"\
        $test_category "X" "Y" 40 0 1]
hs::hs_update

# Ntuple projection onto a 1d histogram
set id6 [hs::create_1d_hist [next_uid] "Ntuple projection 1d"\
        $test_category "X" "D" 50 -10 10]
hs::ntuple_project $ntuple_id $id6 {expr 1} {expr 1} {expr $D}
hs::hs_update

# Put them in one Histo-Scope window, together with 1d ntuple projection
hs::multiplot hs_example_m1 show -title 1d histograms \
        -window first -geometry 700x600+60+50 \
        add $id1 0,0 -color red \
        add $id10 0,0 -line 17 -color blue \
        add $id6 1,0 \
        add $id5 0,1 -color green3 -fill lFineX \
        add $id4 1,1 -color magenta -fill solid

# Block-fill the example histogram
set data1 [hs::list_to_data {1 2 3 4 5 6 7 8 9 10}]
hs::1d_hist_block_fill $id1 $data1
hs::hs_update

# Block-fill the histogram with errors
set data2 [hs::list_to_data {11 12 13 14 15 16 17 18 19 20}]
set pos_err [hs::list_to_data {0.5 1 1.5 2 2.5 3 3.5 4 4.5 5}]
set neg_err [hs::list_to_data {1 1 1 1 1 2 2 2 2 2}]
hs::1d_hist_block_fill $id1 $data2 $pos_err $neg_err
hs::hs_update

# 1d histogram ASCII dump
hs::ascii_dump stdout $id1

# Prepare the example histogram for animation
hs::reset $id1
set pos_err [hs::list_to_data {1 1 1 1 1 1 1 1 1 1}]
hs::hs_update

# Fill several 1d histograms simultaneously
for {set i 0} {$i < 20000} {incr i} {
    hs::fill_1d_hist $id4 [gauss_random 5 1] 1.0
    hs::fill_1d_hist $id5 [expr rand()] 1.0
    if {$i%100 == 0} {
        set example_contents {}
        set blue_contents {}
        for {set j 0} {$j < 10} {incr j} {
            lappend example_contents [expr 10.0+\
                    10.0*cos(0.31416 *($j+$i/2000.0))]
            lappend blue_contents [expr 10.0+\
                    10.0*sin(0.31416 *($j-$i/2000.0))]
        }
        set data3 [hs::list_to_data $example_contents]
        hs::1d_hist_block_fill $id1 $data3 $pos_err
        hs::1d_hist_block_fill $id10 $blue_contents
        hs::hs_update
    }
}
hs::hs_update

# Select a subrange of histogram $id4
set id4_1 [hs::1d_hist_subrange [next_uid] "1d histogram subrange"\
        $test_category $id4 20 39]

# Check 1d histogram linear least squares fitting algorithm
puts ""
puts -nonewline "Checking 1d linear least squares fitting... "
set a 3
set b 7
set npoints 50
set point_error 2
set fit1d_data {}
set fit1d_errors {}
for {set i 0} {$i < $npoints} {incr i} {
    lappend fit1d_data [expr $a * $i + $b + [gauss_random 0 $point_error]]
    lappend fit1d_errors $point_error
}
set id8 [hs::create_1d_hist [next_uid] "1d linear fit"\
        $test_category "X" "Y" $npoints 0 $npoints]
hs::1d_hist_block_fill $id8 $fit1d_data $fit1d_errors
foreach {afit bfit sig_a sig_b rho_ab chisq ndof} \
        [hs::1d_linear_fit $id8 0 1] {}
hs::delete $id8
set afit [expr round($afit * 10000.0) / 10000.0]
set bfit [expr round($bfit * 10000.0) / 10000.0]
set chisq [expr round($chisq * 10000.0) / 10000.0]
puts "Done"
puts "Generated a = $a, b = $b, $npoints bins"
puts "Fitted    a = $afit, b = $bfit, chisq/dof = $chisq/$ndof"
puts ""

######################################################
#                                                    #
#        2d histogram creation and filling           #
#                                                    #
######################################################
puts "Testing creation and filling of 2d histograms"

# Create an example 2d histogram
if {[catch {
    hs::create_2d_hist [next_uid] "Example 2d histogram"\
            $test_category "X" "Y" "Z" 2 5 0 2 0 5
} id2]} {
    puts stderr "ERROR: 2d histogram creation failed!! Further tests aborted."
    return
}
hs::hs_update

# Block-fill the example 2d histogram
hs::2d_hist_block_fill $id2 $data1
hs::hs_update

# 2d histogram ASCII dump
hs::ascii_dump stdout $id2

# 2d histogram transposition
set id2_t [hs::transpose_histogram [next_uid] "Transposed 2d histogram"\
        $test_category $id2]
hs::hs_update

# 2d histogram projection
set id2_2 [hs::project_2d_histogram [next_uid]\
        "Sum of the example 2d histogram rows"\
        $test_category $id2 y sum 0]
hs::hs_update

# Ntuple projection onto a 2d histogram
set id7 [hs::create_2d_hist [next_uid] "Ntuple projection 2d"\
         $test_category "X" "Y" "Z" 40 40 -10 10 -10 10]
hs::ntuple_project $ntuple_id $id7 {expr $D > 0} {expr 1}\
                   {expr $C+$D} {expr $C-$D}
hs::hs_update

# Create a 2d histogram for filling with random numbers
set id3 [hs::create_2d_hist [next_uid] "Random filled 2d histogram"\
         $test_category "X" "Y" "Z" 20 20 -5 5 -5 5]
hs::hs_update

# Put all 2d histograms created so far into a separate window
hs::multiplot hs_example_m2 show -title 2d histograms \
        -window second -geometry 700x600+120+100 \
        add $id2 0,0 \
        add $id2_t 1,0 \
        add $id7 0,1 -mode tartan \
        add $id3 1,1

for {set i 0} {$i < 10000} {incr i} {
    hs::fill_2d_hist $id3 [gauss_random 0 1] [gauss_random 0 2] 1.0
    if {$i%100 == 0} hs::hs_update
}
hs::hs_update

# Set errors for the random-filled 2d histogram
hs::hist_set_gauss_errors $id3
hs::hs_update

# Check 2d histogram linear least squares fitting
puts ""
puts -nonewline "Checking 2d linear least squares fitting... "
set a 5
set b 8
set c 2
set xpoints 10
set ypoints 7
set point_error 1
set fit2d_data {}
for {set i 0} {$i < $xpoints} {incr i} {
    for {set j 0} {$j < $ypoints} {incr j} {
        lappend fit2d_data [expr $a * $i + $b * $j + $c + \
                [gauss_random 0 $point_error]]
    }
}
set id9 [hs::create_2d_hist [next_uid] "2d linear fit" $test_category\
        "X" "Y" "Z" $xpoints $ypoints 0 $xpoints 0 $ypoints]
hs::2d_hist_block_fill $id9 $fit2d_data
foreach {afit bfit cfit - - - - - - chisq ndof} [hs::2d_linear_fit $id9 0] {}
hs::delete $id9
set afit [expr round($afit * 10000.0) / 10000.0]
set bfit [expr round($bfit * 10000.0) / 10000.0]
set cfit [expr round($cfit * 10000.0) / 10000.0]
set chisq [expr round($chisq * 10000.0) / 10000.0]
puts "Done"
puts "Generated a = $a, b = $b, c = $c, $xpoints x bins, $ypoints y bins"
puts "Fitted    a = $afit, b = $bfit, c = $cfit, chisq/dof = $chisq/$ndof"
puts ""

# Make a slider for the random-filled 2d histogram
hs::slice_slider $id3 x

puts "All Histo-Scope examples loaded"

puts "Histo-Scope items defined so far:"
hs::item_info

# Print a basic help message if the hs help file is installed
set helpfile [file dirname [info library]]/hs[package require hs]/hs_manual.txt
if {[file readable $helpfile]} {
    hs::help
}

# Check if we have Tk. If not, enter the event loop anyway.
#if {[catch {package require Tk}]} {
#    if {[catch {package require rdl}]} {
#        proc execute_from_stdin {} {
#            global cmdline
#            if {[gets stdin line] < 0} {
#                exit
#            } else {
#                append cmdline $line "\n"
#                if {[info complete $cmdline]} {
#                    set cmd $cmdline
#                    set cmdline ""
#                    catch {uplevel \#0 $cmd} result
#                    puts $result
#                    puts -nonewline "% "
#                    flush stdout   
#                }
#            }
#        }
#        fileevent stdin readable execute_from_stdin
#        puts -nonewline "% "
#        flush stdout
#        global wait-forever
#        vwait wait-forever
#    } else {
#        rdl::interact 1
#    }
#}
