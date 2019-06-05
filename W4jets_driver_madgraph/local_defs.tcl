global fftw_lib_dir
set fftw_lib_dir /usr/local/lib

global w4jets_dir
set w4jets_dir /home/igv/TopJes/Scripts/W4jets_madgraph

global jet_user_dir
set jet_user_dir /home/igv/TopJes/JetUser

global pythia_lib_dir
set pythia_lib_dir /home/igv/Code/pythia8/lib

global env
set env(PYTHIA8DATA) $pythia_lib_dir/../share/Pythia8/xmldoc

proc compile_local {args} {
    global env fftw_lib_dir w4jets_dir jet_user_dir pythia_lib_dir
    set thisdir [file dirname [info script]]
    set histo_include_dir [file join $env(HISTO_DIR) include]
    set tcl_include_dir "/usr/include/tcl8.6"
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
        global fftw_lib_dir w4jets_dir jet_user_dir pythia_lib_dir
        hs::sharedlib open -export_globals /usr/lib/libblas.so
        hs::sharedlib open -export_globals /usr/lib/liblapack.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libstdc++.so.6
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libz.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libbz2.so
        hs::sharedlib open -export_globals /usr/lib/x86_64-linux-gnu/libfftw3.so

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
