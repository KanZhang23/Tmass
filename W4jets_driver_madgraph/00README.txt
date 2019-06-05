This directory contains a number of scripts needed to test (and later
run on Fermigrid) the W + 4 jets matrix element integration code.

The W + 4 jets code is just a library -- it needs a main executable.
Here, "tclsh" is used as such an executable, while the W + 4 jets code
is interfaced to it as a tcl extension.

To run the code, follow the steps described below.

1) Set up the LD_LIBRARY_PATH environment. If you are using the
standard installation instructions for fcdflnx6 and your shell is
tcsh, you can do

source setup_fcdflnx.csh

2) Compile the W + 4 jets tcl extension. This step is performed with
the script "compile_driver.tcl". Example:

./compile_driver.tcl ttkt72-htktm5_mc_1000ev.hs

The script takes one argument -- the "augmented" minintuple file (this
file defines the input data format). The process of minintuple
"augmentation" will be described elsewhere.

If everything goes well, the script should print something like the
following:

Generated file integ_w4jets.cc from template integ_template_w4jets.cc
Compiled library ./integ_w4jets.so (and checked that it can be loaded)

3) Run the test script "w4jets_test_scan.tcl". If you run it without
any command line arguments, it will print its own usage instructions.
Examples:

./w4jets_test_scan.tcl

./w4jets_test_scan.tcl 1 w4jets_integration_parameters.tcl config.gssa \
    ttkt72-htktm5_mc_1000ev.hs out.hs 3

4) Examine the output using the "view_results.tcl" script. Example:

./view_results.tcl Work 0 out.hs

In the GUI which shows up, there will be two subcategories called
"Pseudo" and "Quasi". The first subcategory contains histograms whose
errors were set to the RMS of the functions values while the second
subcategory contains histograms with the errors set by the "mcuncert"
package. Double click on the subcategory to descend there. Then double
click on "Result 1" in order to see its plot. You can interact with
the plots by clicking inside them with your mouse (different mouse
buttons do different things). In particular, you can turn errors on 
by clicking on the plot with the right mouse button and then checking
"Error Data" in the pop-up menu.

Note that the .hs data files and the transfer function file
"config.gssa" are distributed with this package for testing purposes
only. These files do not represent the final data selection or the
realistic transfer functions. Note also that the integration results
are incomplete -- they need to be multiplied by the acceptance factor
as a function of delta JES. This factor is the same for all events and
it is not taken into account by the matrix element integration code.

Example data files:

ttkt72-htktm5_mc_1000ev.hs          -- ttbar (Mt = 172 GeV)
ptkt4w-utkt04_hfandmistag_1000ev.hs -- W(e nu) + (>= 4p)
ptkt9w-utkt09_hfandmistag_1000ev.hs -- W(mu nu) + (>= 4p)
utkt4w-utkt14_hfandmistag_1000ev.hs -- W(tau nu) + (>= 4p)


In this directory you can also find the following utility scripts:

save_results_madgraph.tcl -- This script will allow you to save the
    integration results as "standard" .hs histograms. You will then
    be able to load the resulting file with "histo", "hswish", or
    to run "hs2root" on it.

row_from_event_number.tcl -- This script will allow you to find the
    sequential event number in the file using its run number and
    event number. When you know the sequential event number, you
    can easily run the integration code on that event only (this is
    useful for debugging). For example, if you want to run the
    "w4jets_test_scan.tcl" script on the event with sequential number
    123 in file "ttkt72-htktm5_mc_1000ev.hs", do it as follows:

    ./w4jets_test_scan.tcl 1 w4jets_integration_parameters.tcl config.gssa \
        ttkt72-htktm5_mc_1000ev.hs event_123.hs 124 123
