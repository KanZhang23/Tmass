global load_results_as_histos_loaded
if {![info exists load_results_as_histos_loaded]} {
    global source_dir
    if {![info exists source_dir]} {
        set source_dir [file dirname [info script]]
    }
    set load_results_as_histos_loaded [lindex [compile_and_load \
          $source_dir/load_results_as_histos.cc \
          $source_dir/packStringToHSItem.cc \
          $source_dir/loadResultsFromHS.cc \
          $source_dir/makeResultHisto.cc] 0]
}
