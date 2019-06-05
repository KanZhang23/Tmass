
# Load the function
hs::sharedlib open -export_globals /home/igv/TopJes/JetUser/libJetUser.so
set dlltoken [hs::sharedlib open -export_globals ./libtopmass.so]

hs::function_import p0_interpolator $dlltoken p0_interpolator \
    "p0 interpolator" 2 0 0 0 {} {} p0_interpolator_hs_fcn {} {}

set id [hs::create_2d_hist [next_uid] "P0 interpolator scan" "" jes eta p0 \
           100 100 -6 6 -2 2]
hs::function p0_interpolator scan [list $id x y]
hs::show_histogram $id
