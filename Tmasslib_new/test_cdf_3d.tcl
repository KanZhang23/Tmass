# Use with "hswish"

hs::sharedlib open -export_globals /home/igv/TopJes/JetUser/libJetUser.so
hs::sharedlib open -export_globals ./libtopmass.so

set id_3d [hs::create_3d_hist [next_uid] "Example 3d histo" "" x y z v \
              50 50 50 -5 5 -6 4 -3 7]
hs::function trivar_gauss scan [list $id_3d x y z] {volume4d 1} \
    {mean_x 0} {mean_y -1} {mean_z 2} {sigma_x 1} {sigma_y 0.7} \
    {sigma_z 1.3} {rho_xy 0} {rho_xz 0} {rho_yz 0}
hs::hist_scale_data $id_3d [expr {1.0/[hs::hist_integral $id_3d]}]

set cdf [cdf_3d $id_3d]

set id_dens [hs::duplicate_axes $id_3d [next_uid] "Density copy" ""]
$cdf density $id_dens

set integ [[[[hs::calc $id_3d] - [hs::calc $id_dens]] abs] integ]
puts "Integrated absolute density difference is $integ"

set val1 [$cdf eval 0 1 1]
set id1 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id_3d x 0 24]
set id2 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id1 y 0 34]
set id3 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id2 z 0 19]
set newval [expr {[hs::hist_integral $id3]}]
puts "Expected $newval, got $val1"

set val2 [$cdf eval 1 2 3]
set id1 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id_3d x 0 29]
set id2 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id1 y 0 39]
set id3 [hs::3d_hist_subrange [next_uid] "Dummy" "" $id2 z 0 29]
set newval [expr {[hs::hist_integral $id3]}]
puts "Expected $newval, got $val2"

set coverage 0.3
foreach {xmin ymin zmin xmax ymax zmax} [$cdf window $coverage 1 1 1] {}

# Try to estimate the integral using a linear interpolator
set interp [create_linear_interpolator_nd $id_3d]
set integ [$interp integrate [list $xmin $xmax 200] \
               [list $ymin $ymax 200] [list $zmin $zmax 200]]
puts "Requested coverage is $coverage, estimated integral is $integ"
