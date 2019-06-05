global env fftw_lib_dir w4jets_dir jet_user_dir compilers_lib_dir
set fftw_lib_dir $env(BASEDIR)/lib
set tmasslib_dir $env(BASEDIR)/Tmasslib_new
set jet_user_dir $env(BASEDIR)/JetUser
set compilers_lib_dir /cdf/proj/401.top/igv/compilers/lib64

proc compile_local {args} {
    global env fftw_lib_dir tmasslib_dir jet_user_dir
    set thisdir [file dirname [info script]]
    set histo_include_dir [file join $env(HISTO_DIR) include]
    set tcl_include_dir [file join [file dirname [info library]] .. include]
    set soname [file rootname [file tail [lindex $args 0]]].so
    set outfile [file join $thisdir $soname]
    eval exec gcc -g -shared -Wall -Wno-write-strings -fPIC \
        -I$thisdir -I${fftw_lib_dir}/../include -I$tcl_include_dir \
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
        global fftw_lib_dir tmasslib_dir jet_user_dir compilers_lib_dir
        hs::sharedlib open -export_globals $fftw_lib_dir/libblas.so
        hs::sharedlib open -export_globals $fftw_lib_dir/liblapack.so
        hs::sharedlib open -export_globals $compilers_lib_dir/libstdc++.so.6
        hs::sharedlib open -export_globals /usr/lib64/libz.so
        hs::sharedlib open -export_globals /usr/lib64/libbz2.so
        hs::sharedlib open -export_globals $fftw_lib_dir/libfftw3.so
        hs::sharedlib open -export_globals $fftw_lib_dir/librk.so
        hs::sharedlib open -export_globals $fftw_lib_dir/libgeners.so
        hs::sharedlib open -export_globals $fftw_lib_dir/libkstest.so
        hs::sharedlib open -export_globals $fftw_lib_dir/libnpstat.so
        hs::sharedlib open -export_globals $fftw_lib_dir/libmcuncert.so
        hs::sharedlib open -export_globals $jet_user_dir/libJetUser.so
        hs::sharedlib open -export_globals $tmasslib_dir/libtopmass.so
        set preload_libs_guard_variable 1
    }
    return
}
