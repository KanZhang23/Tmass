# If we are in the test mode, reduce the number of integration points
global test_mode
if {![info exists test_mode]} {
    set test_mode 0
}

# Define the integration parameters
global integ_params
set integ_params [list]

# Are we using quasi-random or pseudo-random points for integration?
lappend integ_params [list useQuasiMC true]

# Number of quasi- (or pseudo-) random points to use for integration.
# Must be a power of 2 (and must be 32 or larger).
if {$test_mode} {
    lappend integ_params [list nIntegrationPoints 512]
} else {
    lappend integ_params [list nIntegrationPoints 2048]
}

# How many points to use in the W mass scan?
if {$test_mode} {
    lappend integ_params [list nWmassScanPoints 5]
} else {
    lappend integ_params [list nWmassScanPoints 40]
}

# Relative tolerance to reach using the QMC uncertainty estimator
if {$test_mode} {
    lappend integ_params [list reltol 1.0e-10]
} else {
    lappend integ_params [list reltol 1.0e-2]
}

# Fraction of JES points which must satisfy the above tolerance
# requirement for successful completion of the integration process
lappend integ_params [list per_requirement 0.5]

# First power of 2 to store in the integration history
lappend integ_params [list firstPowerOfTwoToStore 8]

# How do we interpolate between jet transfer functions?
lappend integ_params [list useLogScaleForTFpt false]

# Are we processing W -> tau nu?
lappend integ_params [list allowTau false]

# Are we processing W -> l nu, l != tau?
lappend integ_params [list allowDirectDecay true]

# Are we using detector eta to bin the TFs in eta
lappend integ_params [list useDetectorEtaBins true]

# Maximum number of events to process
lappend integ_params [list maxevents 2147483647]

# Number of events to skip at the beginning of the ntuple
lappend integ_params [list skipevents 0]

# HS category where results will be placed
lappend integ_params [list category "Work"]

# Special script to run periodically (in terms of events processed).
# Empty script means do not run anything.
lappend integ_params [list periodic_script ""]
lappend integ_params [list script_period 1]

# Transfer function configuration file
# lappend integ_params [list tf_config_file "config.gssa"]

# The seed for pseudo-random number generator (in case
# we do not use quasi-MC). 0 means use a seed from /dev/urandom.
lappend integ_params [list seed 0]

# Interlacing factor for the high-order QMC
lappend integ_params [list interlacingFactor 2]

# Configuration parameters for scanning delta JES
lappend integ_params [list minDeltaJES -3.05]
lappend integ_params [list maxDeltaJES 3.05]
lappend integ_params [list deltaJesSteps 61]

# Configuration parameters for scanning W mass
lappend integ_params [list nominalWmass 80.385]
lappend integ_params [list nominalWwidth 2.085]
lappend integ_params [list maxWmassForScan [expr {80.385*2}]]

# Nominal beam energy (in GeV)
lappend integ_params [list eBeam 980.0]

# Maximum Pt of the parton to consider in the construction
# of parton Pt priors
lappend integ_params [list maxPartonPt 600.0]

# Minimal value of (pt parton)/(pt jet) to use when
# importance sampling densities are prepared
lappend integ_params [list minPartonPtFactor 0.3]

# Tight jet transverse momentum cut (at level 5)
lappend integ_params [list jetPtCut 20.0]

# Approximate wall clock time limit per event
lappend integ_params [list maxSeconds 14400]

# The code will print out a time stamp, periodically, after
# some number of QMC points processed. Configure this number.
# Set it to 0 to disable the printouts.
lappend integ_params [list reportInterval 0]

# File name to store random vectors in ascii form.
# Empty string means do not create such a file.
lappend integ_params [list rng_dump_file ""]

# File name to store raw function values in ascii form.
# Empty string means do not create such a file.
lappend integ_params [list fcn_dump_file ""]

# File to take the random numbers from
lappend integ_params [list qmc_input_file ""]

# Are we going to use a Pareto distribution prior or
# the prior which comes with the TF config file?
# For pareto prior, make the following parameter positive
# (this is the "alpha" parameter of Pareto).
lappend integ_params [list priorPowerParam 0.0]

# Bandwidth to use for smoothing the parton Pt priors
lappend integ_params [list priorBandwidth 0.04]

# Are we going to save parton Pt priors?
lappend integ_params [list saveJetPriors false]

# Integration components. Do not change these values -- instead,
# change the terms added to form the "integrate" value below.
set integ_prior_density 1
set integ_transfer_functions 2
set integ_efficiencies 4
set integ_matrix_element 8
set integ_jacobians 16

# Add up integration components to get the overall integration mode
set integrate [expr {$integ_prior_density + \
                     $integ_transfer_functions + \
                     $integ_efficiencies + \
                     $integ_matrix_element + \
                     $integ_jacobians}]

# Add the total integration mode to the parameter list
lappend integ_params [list integrationMode $integrate]

# Mysterious "mcuncert" parameters. Tony should explain them.
lappend integ_params [list qmcuncert_m_min 8]
lappend integ_params [list qmcuncert_l_star 4]
lappend integ_params [list qmcuncert_r_lag 4]

# Procedure for getting, setting, or modifying the parameters
proc parameter {name {value "ImpoSsiBle_ParAmeTer_VaLue"}} {
    global integ_params
    if {[string equal $value "ImpoSsiBle_ParAmeTer_VaLue"]} {
        foreach pair $integ_params {
            foreach {n v} $pair break
            if {[string equal $n $name]} {
                return $v
            }
        }
        error "Parameter \"$name\" is undefined"
    } else {
        set oldlist $integ_params
        set integ_params [list [list $name $value]]
        foreach pair $oldlist {
            foreach {n v} $pair break
            if {![string equal $n $name]} {
                lappend integ_params $pair
            }
        }
        return $value
    }
}
