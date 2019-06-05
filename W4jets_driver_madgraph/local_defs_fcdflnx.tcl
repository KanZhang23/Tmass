global env fftw_lib_dir w4jets_dir jet_user_dir pythia_lib_dir compilers_lib_dir
set fftw_lib_dir $env(BASEDIR)/lib
set w4jets_dir $env(BASEDIR)/W4jets_madgraph
set jet_user_dir $env(BASEDIR)/JetUser
set pythia_lib_dir $env(BASEDIR)/pythia8/lib
set compilers_lib_dir /cdf/proj/401.top/igv/compilers/lib64

set env(PYTHIA8DATA) $pythia_lib_dir/../share/Pythia8/xmldoc

proc compile_local {args} {
    global env fftw_lib_dir w4jets_dir jet_user_dir pythia_lib_dir
    set thisdir [file dirname [info script]]
    set histo_include_dir [file join $env(HISTO_DIR) include]
    set tcl_include_dir [file join [file dirname [info library]] .. include]
    set soname [file rootname [file tail [lindex $args 0]]].so
    set outfile [file join $thisdir $soname]
    eval exec g++ -std=c++11 -g -shared -Wall -Wno-write-strings -fPIC \
        -I$thisdir -I${fftw_lib_dir}/../include -I$tcl_include_dir \
        -I$histo_include_dir -I$jet_user_dir -I$w4jets_dir \
        -I${pythia_lib_dir}/../include \
        $args \
        -L${w4jets_dir}/wellnu01234j_5f_LO_MLM/lib -lpdf \
        -o $outfile
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
        global fftw_lib_dir w4jets_dir jet_user_dir pythia_lib_dir compilers_lib_dir
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
        hs::sharedlib open -export_globals $fftw_lib_dir/libLHAPDF.so
        hs::sharedlib open -export_globals $jet_user_dir/libJetUser.so
        hs::sharedlib open -export_globals $w4jets_dir/libW4jetsTFs.so
        hs::sharedlib open -export_globals $w4jets_dir/libW4jets.so
        hs::sharedlib open -export_globals $pythia_lib_dir/libpythia8.so
        hs::sharedlib open -export_globals $pythia_lib_dir/libpythia8lhapdf6.so
        set preload_libs_guard_variable 1
    }
    return
}
