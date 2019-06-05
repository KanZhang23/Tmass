set datadir "/cdf/home/igv/Topmass/Mt_master_worker_ssh/Test"

set project [add_project auto test_project_code.tcl {sum_columns 2}]

foreach number {1 2} {
    add_workload $project {} "$datadir/test_project_in${number}.hs" {Random ntuple} {Input} {Output category} "$datadir/test_project_out${number}.hs"
}
