#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

package require hs
hs::initialize "Test"

set rows_to_fill 30

set id [hs::create_ntuple 1 "Random ntuple" "Input" {a b c d e}]
hs::fill_ntuple $id [hs::random [expr {$rows_to_fill*[hs::num_variables $id]}]]

set file "test_project_in1.hs"
puts "Saved [hs::save_file $file] item(s) in file $file"

hs::fill_ntuple $id [hs::random [expr {$rows_to_fill*[hs::num_variables $id]}]]
set file "test_project_in2.hs"
puts "Saved [hs::save_file $file] item(s) in file $file"

exit 0
