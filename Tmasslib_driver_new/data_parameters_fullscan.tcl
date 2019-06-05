# Define the scan parameters
global scan_parameters
set scan_parameters [list]

# Top-level behavior
lappend scan_parameters [list "process_tcl_events" 1]
lappend scan_parameters [list "print_integration_parameters" 1]
lappend scan_parameters [list "debug_level" 10]

# Are we using the leptonic side mask?
lappend scan_parameters [list "mc_lside_max_seconds" 300]
lappend scan_parameters [list "mc_lside_max_points" 524288]
lappend scan_parameters [list "mc_lside_rand_method" "sobol"]
lappend scan_parameters [list "mc_lside_rnd_param" 0]

# Some general algorithmic parameters
lappend scan_parameters [list "matrel_code" 5]
lappend scan_parameters [list "mc_efficiency_mask" 1]
lappend scan_parameters [list "mc_fudge_to_treelevel" 1]
lappend scan_parameters [list "prob_factor" 1.0e3]

# The "permute_jets" parameter should be set to 2 normally,
# when the b tagging probability is understood.
lappend scan_parameters [list "permute_jets" 2]

# Parameters for leptonic MW^2 integration
lappend scan_parameters [list "wlep_npoints" 40]
lappend scan_parameters [list "wlep_coverage" 0.99]

# Parameters for log(pq/pqbar) integration
lappend scan_parameters [list "param_grid_absolute" 0]
lappend scan_parameters [list "mc_abs_param_min" -3.5]
lappend scan_parameters [list "mc_abs_param_max"  3.5]
lappend scan_parameters [list "mc_rel_param_min" -1.7]
lappend scan_parameters [list "mc_rel_param_max"  1.7]

# Various less important coverage limits
lappend scan_parameters [list "whad_coverage" 0.998]
lappend scan_parameters [list "thad_coverage" 0.98]
lappend scan_parameters [list "tlep_coverage" 0.98]

# Convergence and termination parameters
lappend scan_parameters [list "mc_check_factor" 2.0]
lappend scan_parameters [list "mc_worst_perm_cutoff" 1.5]
lappend scan_parameters [list "mc_precision_fraction" 0.68]
lappend scan_parameters [list "mc_precision_target" 0.1]
lappend scan_parameters [list "mc_min_points" 512]
lappend scan_parameters [list "mc_max_points" 262144]
lappend scan_parameters [list "mc_max_zeroprob_pts" 8192]
lappend scan_parameters [list "mc_max_event_seconds" 7200]

# Scales for the width of massless jet angular transfer functions. Used
# only when the masses of the corresponding jet are not integrated over.
lappend scan_parameters [list "eta_q_sigma_factor" 1.059]
lappend scan_parameters [list "phi_q_sigma_factor" 1.059]
lappend scan_parameters [list "eta_b_sigma_factor" 1.147]
lappend scan_parameters [list "phi_b_sigma_factor" 1.132]

# Jet masses. These masses are used only when
# the jet mass integration is turned off.
lappend scan_parameters [list "light_jet_mass" -1.0]
lappend scan_parameters [list "lep_b_jet_mass" -1.0]
lappend scan_parameters [list "had_b_jet_mass" -1.0]

# Jet masses used at tree level
lappend scan_parameters [list "mc_nominal_q_mass" 0.1]
lappend scan_parameters [list "mc_nominal_b_mass" 4.8]

# Configure the quasi-random grid generator
lappend scan_parameters [list "mc_rand_method" "fortsobol"]
lappend scan_parameters [list "mc_random_gen_param" 2]

# Are we running in the 3-jet mode?
lappend scan_parameters [list "mc_prob_to_acquire_jet" "none"]
lappend scan_parameters [list "mc_prob_to_loose_parton" "none"]
lappend scan_parameters [list "mc_excluded_jet" "none"]

# Integration dimensions
foreach {dim flag} {
    param      1
    whad       1
    tau        0
    ttbarpt    1
    toplep     1
    tophad     1
    mq         1
    mqbar      1
    mbhad      1
    mblep      1
    angle_q    1
    angle_qbar 1
    angle_bhad 1
    angle_blep 1
} {
    lappend scan_parameters [list "mc_grid_mask_$dim" $flag]
}

# Parameters used when the TFs are loaded
lappend scan_parameters [list "minExceedance" 0.01]
lappend scan_parameters [list "partonMassSplit" 1.0]
lappend scan_parameters [list "drStretchFactor" 1.01]
lappend scan_parameters [list "jetPtCutoff" 12.0]
lappend scan_parameters [list "interpolate_tfs_over_params" 0]
