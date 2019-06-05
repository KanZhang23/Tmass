package ifneeded hs 2.17 \
    "[list load [file join $dir libshlibhandler.so]]; \
     [list hs::sharedlib_handler open -export_globals [file join $dir libhs2.17.so]]; \
     [list source [file join $dir hs_utils.tcl]]"
