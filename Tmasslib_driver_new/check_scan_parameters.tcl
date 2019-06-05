#!/bin/sh
# The next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

if {$argc != 2} {
    puts ""
    puts "Usage: [file tail [info script]] scan_parameters_c_file scan_parameters_tcl_file"
    puts ""
    exit 1
}
foreach {scan_parameters_c_file scan_parameters_tcl_file} $argv break

set source_dir [file dirname [info script]]
source $source_dir/set_utils.tcl
source $source_dir/scan_utils.tcl

proc adjust_angular_tf_width {args} {return}
proc set_tf_parameters {args} {return}

proc strip_comma {name} {
    if {[string equal [string index $name end] ","]} {
        return [string range $name 0 end-1]
    } else {
        return $name
    }
}

# Get C parameter definitions
set optional_parameter_names [list]
set optional_parameter_values [list]
set required_parameter_names [list]
set lines [split [file_contents $scan_parameters_c_file] "\n"]
set linenum 0
foreach line $lines {
    incr linenum
    if {[string first "#define" $line] < 0} {
        if {[string first "setup_optional_parameter" $line] >= 0} {
            foreach {dumm name value} $line break
            lappend optional_parameter_names [strip_comma $name]
            lappend optional_parameter_values [strip_comma $value]
        } elseif {[string first "optional_string_parameter" $line] >= 0} {
            foreach dumm $line break
            set name [lindex [split $dumm "\(\)"] 1]
            lappend optional_parameter_names [strip_comma $name]
            lappend optional_parameter_values "<undefined>"
        } elseif {[string first "setup_required_parameter" $line] >= 0} {
            foreach {dumm name} $line break
            lappend required_parameter_names [strip_comma $name]
        } elseif {[string first "required_string_parameter" $line] >= 0} {
            foreach dumm $line break
            set name [lindex [split $dumm "\(\)"] 1]
            lappend required_parameter_names $name
        } elseif {[string first "setup_norm_parameters" $line] >= 0} {
            break
        }
    }
}
set all_parameter_names [concat $optional_parameter_names $required_parameter_names]

# Get tcl parameter definitions
set tmasslib_dir "TMASSLIB_DIR"
global scan_parameters
source $scan_parameters_tcl_file

set defined_names [list]
set defined_values [list]
foreach pair $scan_parameters {
    foreach {name value} $pair {}
    lappend defined_names $name
    lappend defined_values $value
}

if {![set_is_subset $required_parameter_names $defined_names]} {
    puts "Missing required parameters: [set_excess $required_parameter_names $defined_names]"
    exit 1
}

if {![set_is_subset $defined_names $all_parameter_names]} {
    puts "Invalid parameter names: [set_excess $defined_names $all_parameter_names]"
    exit 1
}

# Build the final set
set warnings [list]
foreach name $defined_names value $defined_values {
    if {[info exists final_parameter_set($name)]} {
        lappend warnings "WARNING: parameter \"$name\" is defined more than once"
    } else {
        set final_parameter_set($name) $value
    }
}

foreach name $optional_parameter_names value $optional_parameter_values {
    if {![info exists final_parameter_set($name)]} {
        set final_parameter_set($name) $value
    }
}

foreach name [lsort [array names final_parameter_set]] {
    puts "$name : $final_parameter_set($name)"
}

puts ""
if {[llength $warnings]} {
    foreach w $warnings {
        puts $w
    }
} else {
    puts "Parameter set is OK"
}

exit 0
