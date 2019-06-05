#
# Tcl package index file
#
package ifneeded rdl 1.1 \
    "[list load [file join $dir librdl1.1.so]]; \
     [list source [file join $dir rdl_utils.tcl]]"
