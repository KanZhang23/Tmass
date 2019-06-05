To run the top mass matrix element integration code, follow the steps
described below:

1) Set up the LD_LIBRARY_PATH environment. If you are using the
standard installation instructions for fcdflnx6 and your shell is
tcsh, you can do

source setup_fcdflnx.csh

2) Compile the top mass library tcl extension. This step is performed
with the script "compile_driver.tcl". Example:

./compile_driver.tcl ttkt70_gen6_100ev.hs

The script takes one argument -- the "augmented" minintuple file (this
file defines the input data format). The process of minintuple
"augmentation" will be described elsewhere.

If everything goes well, the script should print something like the
following:

Generated file data_integ_mtm3.c from template data_integ_template_mtm3.c
Compiled library ./data_integ_mtm3.so (and checked that it can be loaded)

3) Make sure you have the transfer functions. The following assumes that
they are placed in the directory /cdf/spool/$USER/Transfer_functions_new.
That directory should contain two files named "delta_r_efficiency.hs" and
"ttbar_jet_tfs.gssa".

4) Run the test script "mt_test_scan.tcl". If you run it without any
command line arguments, it will print its own usage instructions. Examples:

./mt_test_scan.tcl

./mt_test_scan.tcl data_parameters_fullscan.tcl \
  /cdf/spool/$USER/Transfer_functions_new ttkt70_gen6_100ev.hs myscan.hs 1024 2

5) Examine the output using the "view_2d_probs.tcl" script. Example (assuming
   that your DISPLAY environment is correctly set up):

./view_2d_probs.tcl 1 myscan.hs


Description of various files in this directory
----------------------------------------------

** browse_graphs.tcl, scan_utils.tcl, set_utils.tcl -- utility code
   for use by other scripts. The "load_transfer_functions" procedure
   in file "scan_utils.tcl" may be of special interest because it
   shows how to load all transfer/efficiency functions for use with
   the top mass library.

** check_scan_parameters.tcl -- script for checking correctness of the
   parameter definition files. Example usage:

   check_scan_parameters.tcl scan_parameters.c data_parameters_fullscan.tcl

** compile_driver.tcl -- script to compile the driver library for the
   top mass matrix element integration code. Example:

   ./compile_driver.tcl ttkt70_gen6_100ev.hs

** data_integ_mtm3.h, data_integ_template_mtm3.c -- template file for data
   and MC scans. To make a library out of it, run the "compile_driver.tcl"
   script.

** data_parameters_fullscan.tcl -- example parameter set for processing
   data/MC files. All integration dimensions are turned on.

** find_row.tcl -- a script for finding the ntuple row number for an event
   with given values of "thisRow", "run", and "event" variables

** local_defs.tcl, local_defs_fcdflnx.tcl -- definitions of various
   useful global variables for tcl scripts.

** Makefile.profile, mtm3sh.c -- using this makefile, one can statically
   link the "mtm3sh" executable and profile the top mass library. See the
   file "profiling.txt" for more details.

** mt_integration_parameters.txt -- description of various parameters used
   by the top mass library.

** mt_test_scan.tcl -- test script for processing data without using
   the full CAF attack framework. Run this script without any arguments
   to see its usage instructions. Examples:

   ./mt_test_scan.tcl

   ./mt_test_scan.tcl data_parameters_fullscan.tcl ../Transfer_functions \
        ttkt70_gen6_100ev.hs myscan.hs 1024 2

   Don't forget to run "compile_driver.tcl" before using this script.

** parameter_parser.c, parameter_parser.h -- utility functions used by
   the scan code.

** profiling.txt -- this file explains how to profile the top mass library
   code using gprof.

** setup.csh -- set up the environment for profiling (not for fcdflnx6).

** setup_fcdflnx.csh -- set up the environment for running the top mass
   library code on fcdflnxgpvm01.fnal.gov.

** scan_parameters.c -- parameter definition file for the top mass library.

** ttkt70_gen6_100ev.hs -- an example data file.

** view_2d_probs.tcl -- example script which can be used to examine matrix
   element integration results "by eye". Example usage:

   ./view_2d_probs.tcl 1 myscan.hs
