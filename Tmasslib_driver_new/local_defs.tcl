global local_lib_dir
set local_lib_dir /usr/local/lib

global tmasslib_dir
set tmasslib_dir /home/igv/TopJes/Scripts/Tmasslib_new

global jet_user_dir
set jet_user_dir /home/igv/TopJes/JetUser

proc compile_local {args} {
    global env local_lib_dir tmasslib_dir jet_user_dir
    set thisdir [file dirname [info script]]
    set histo_include_dir [file join $env(HISTO_DIR) include]
    set tcl_include_dir /usr/include/tcl8.6
    set soname [file rootname [file tail [lindex $args 0]]].so
    set outfile [file join $thisdir $soname]
    eval exec gcc -g -O0 -shared -Wall -Wno-write-strings -fPIC \
        -I$thisdir -I${local_lib_dir}/../include -I$tcl_include_dir \
        -I$histo_include_dir -I$jet_user_dir -I$tmasslib_dir \
        $args -o $outfile
    return $outfile
}

proc compile_and_load {args} {
    set outfile [eval compile_local $args]
    set dll [hs::sharedlib open $outfile]
    list $dll $outfile
}

proc preload_libs {} {
    global preload_libs_guard_variable
    if {![info exists preload_libs_guard_variable]} {
        global local_lib_dir tmasslib_dir jet_user_dir
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libblas.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/liblapack.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libstdc++.so.6
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libz.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libbz2.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libfftw3.so
        hs::sharedlib open -export_globals $local_lib_dir/libLHAPDF.so
        hs::sharedlib open -export_globals $local_lib_dir/librk.so
        hs::sharedlib open -export_globals $local_lib_dir/libgeners.so
        hs::sharedlib open -export_globals $local_lib_dir/libkstest.so
        hs::sharedlib open -export_globals $local_lib_dir/libnpstat.so
        hs::sharedlib open -export_globals $jet_user_dir/libJetUser.so
        hs::sharedlib open -export_globals $tmasslib_dir/libtopmass.so
        set preload_libs_guard_variable 1
    }
    return
}
