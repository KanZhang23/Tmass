global jet_corrections_api_loaded
if {![info exists jet_corrections_api_loaded]} {
    global jet_user_dir
    if {![info exists user_dir]} {
        set jet_user_dir [file dirname [info script]]
    }
    set sofile [file join $jet_user_dir ./jet_corrections_api.so]
    hs::sharedlib_compile [file join $jet_user_dir jet_corrections_api.c] $sofile
    set jet_corrections_api_loaded [hs::sharedlib open $sofile]
    hs::function_import jetcorr $jet_corrections_api_loaded jetcorr jetcorr \
        1 0 3 3 {icorr detEta emf} {} jetcorr {} {}
    hs::function_import l0pt $jet_corrections_api_loaded l0pt l0pt \
        1 0 3 3 {icorr detEta emf} {} l0pt {} {}
    hs::function_import invcheck $jet_corrections_api_loaded invcheck invcheck \
        1 0 3 3 {icorr detEta emf} {} invcheck {} {}
}
