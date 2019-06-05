
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init

# Make a histogram
set id [random_histo gauss 30 -3 3 0 1 800]
set nstyle 0
foreach style {
    none solid fineHoriz coarseHoriz fineVert
    coarseVert fineGrid coarseGrid fineX coarseX
    fine45deg med45deg coarse45deg fine30deg coarse30deg
    fine60deg coarse60deg rFine45deg rMed45deg
    rCoarse45deg rFine30deg rCoarse30deg rFine60deg
    rCoarse60deg lFineHoriz lCoarseHoriz lFineVert
    lCoarseVert lFineGrid lCoarseGrid lFineX lCoarseX
    lFine45deg lMed45deg lCoarse45deg lFine30deg
    lCoarse30deg lFine60deg lCoarse60deg
} {
    # Duplicate the histogram with a new title
    set newid [hs::copy_hist $id [next_uid] "$style ($nstyle)" "Tmp"]
    # Add it to a multiplot
    if {$nstyle < 20} {
	set w mfill1
    } else {
	set w mfill2
    }
    set row [expr {($nstyle % 20) / 4}]
    set col [expr {($nstyle % 20) % 4}]
    hs::multiplot $w add $newid $col,$row -fill $style
    incr nstyle
}
hs::multiplot mfill2 -title "Histo-Scope histogram fill styles (2 of 2)" \
	-geometry 580x750 show clear
hs::multiplot mfill1 -title "Histo-Scope histogram fill styles (1 of 2)" \
	-geometry 580x750 show clear
