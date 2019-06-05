
# Check that we are running a sufficiently recent Tcl version
package require Tcl 8.3

# Check that we have the shared library loaded
if {[catch ::hs::tcl_api_version]} {
    error "Failed to load Histo-Scope Tcl interface.\
           Please check your installation."
    return
}

# Make sure that the HISTO_DIR environment is set appropriately
proc ::hs::Histo_dir_env {} {
    global ::env
    if {![info exists ::env(HISTO_DIR)]} {
        set hdir [file dirname [::hs::Histosope_lib_dir]]
        if {[file executable [file join $hdir bin histo]]} {
            set ::env(HISTO_DIR) $hdir
        }
    }
    return $::env(HISTO_DIR)
}

if {[catch ::hs::Histo_dir_env]} {
    error "Refusing to load Histo-Scope tcl interface.\
	   Please define HISTO_DIR\nenvironmental variable appropriately."
    return
}

# We might need the following function for proper initialization
proc ::hs::Parse_tclConfig_sh {} {
    variable TclConfig_sh_defs
    if {![info exists TclConfig_sh_defs(exists)]} {
	array unset TclConfig_sh_defs
	set filename [file join [file dirname [info library]] tclConfig.sh]
	if {![file readable $filename]} {
            set filename [hs::Tcl_config_file]
            if {![file readable $filename]} {
                error "File tclConfig.sh not found"
            }
	}
	set channel [open $filename r]
	while {[gets $channel line] >= 0} {
	    if {[string is space $line]} continue
	    if {[string equal -length 1 $line "#"]} continue
	    set eq [string first "=" $line]
	    if {$eq > 0} {
		set name [string range $line 0 [expr {$eq-1}]]
		set value [string range $line [expr {$eq+1}] end]
		set value [string trim $value " '\t"]
		set TclConfig_sh_defs($name) $value
	    }
	}
	close $channel
	# Verify that we've got all necessary variables
	foreach name {
            TCL_INCLUDE_SPEC
	    TCL_CC
	    TCL_SHLIB_CFLAGS
	    TCL_EXTRA_CFLAGS
	    TCL_SHLIB_LD
	} {
	    if {![info exists TclConfig_sh_defs($name)]} {
		error "Definition of $name is missing in file $filename"
	    }
            if {[string first "\$" $TclConfig_sh_defs($name)] >= 0} {
                error "Definition of $name is too complex in file $filename"
            }
	}
	set TclConfig_sh_defs(exists) 1
    }
    return
}

proc ::hs::Locate_tch_header {} {
    variable Tcl_C_header_file
    if {![info exists Tcl_C_header_file]} {
        set files_to_try [list]
        lappend files_to_try [file join [file dirname [info library]] .. include tcl.h]
        global tcl_version
        lappend files_to_try "/usr/include/tcl${tcl_version}/tcl.h"
        foreach filename $files_to_try {
            if {[file readable $filename]} {
                set Tcl_C_header_file $filename
                return $filename
            }
        }
        catch {::hs::Parse_tclConfig_sh}
        variable TclConfig_sh_defs
        set filename [file join [string range $TclConfig_sh_defs(TCL_INCLUDE_SPEC) 2 end] tcl.h]
        if {[file readable $filename]} {
            set Tcl_C_header_file $filename
            return $filename
        }
        error "Failed to locate file tcl.h"
    }
    return $Tcl_C_header_file
}

# Initialize the package API
package provide hs [lindex [::hs::tcl_api_version] 0]
if {[llength [info commands ::oldExit_Gt4My8D4Zlq2vJ9Sn]] == 0} {
    ::rename ::exit ::oldExit_Gt4My8D4Zlq2vJ9Sn
    proc ::exit {{returnCode 0}} {
        hs::periodic_update stop
        hs::kill_histoscope
        hs::hs_update
        # hs::wait_num_scopes == 0
        ::oldExit_Gt4My8D4Zlq2vJ9Sn $returnCode
    }
    # Put the rest of the initialization code into a few procs
    # to avoid polluting the global namespace with stuff...
    namespace eval ::fit:: {}
    proc ::fit::Init_tcl_api {} {
        # Initialize the namespace variables
        namespace eval ::fit:: {
    	    variable Temporary_function_counter 0
    	    variable Fit_plot_window_counter 0
    	    variable Fit_printout_precision 5
    	    variable Minuit_is_initialized 0
    	    variable Fit_tuner_counter 0
            variable Default_bandwidth_set_size 20
            variable Max_fit_plots_in_multi 30
    	    namespace export {[a-z123]*}
        }
        if {[::hs::have_cernlib]} {
    	    # Initialize Minuit
    	    ::mn::init stdin stdout stdout
    	    # Set up the standard minimizer
    	    ::fit::Fit_fcn_set "generic"
        }
    }
    proc ::hs::Import_simple_function {tag dll name descr pars c_name} {
        set npars [llength $pars]
        hs::function_import $tag $dll $name $descr \
    	    1 0 $npars $npars $pars {} $c_name {} {}
    }
    proc ::hs::Function_signature {tag} {
        list [hs::function $tag cget -dll] \
    	    [hs::function $tag cget -functions] \
    	    [hs::function $tag cget -parameters]
    }
    proc ::hs::Init_tcl_api {} {
        # Check the system encoding. Useful, e.g., in RedHat 9
        if {[string equal [encoding system] "utf-8"]} {
            encoding system iso8859-1
        }

        # Remove commands considered obsolete
        ::rename ::hs::slice_2d_histogram {}

        # Rename commands for which we will build wrappers
        ::rename ::hs::dir ::hs::Dir_uncached
        ::rename ::hs::function_list ::hs::Function_list_unsorted
        ::rename ::hs::unique_rows ::hs::Unique_rows
        if {[::hs::have_cernlib]} {
            ::rename ::hs::uniform_random_fill ::hs::Uniform_random_fill
        }
        ::rename ::hs::ntuple_so_scan ::hs::C_ntuple_so_or_dll_scan
        ::rename ::hs::histoscope ::hs::Histoscope
        ::rename ::hs::histo_with_config ::hs::Histo_with_config

        # Rename commands which should not be accessed by applications
        ::rename ::hs::lookup_command_info ::hs::Lookup_command_info
        ::rename ::hs::fill_slice ::hs::Fast_2d_slice_fill
        ::rename ::hs::sharedlib_handler ::hs::Sharedlib_handler

        # Make better command names for certain commands
        ::rename ::hs::Matrix ::hs::M
        ::rename ::hs::fill_coord_slice ::hs::fill_slice

        # Try to find the library of functions
        set version [lindex [::hs::tcl_api_version] 0]
        set ext [::info sharedlibextension]
        set found 0
        global ::env
        if {[info exists ::env(TCLLIBPATH)]} {
            foreach dir [split $::env(TCLLIBPATH)] {
                set file [file join $dir hs$version libmethods$ext]
                if {[file readable $file]} {
                    set found 1
                    break
                }
            }
        }
        if {!$found} {
            set file [file join [::info library] hs$version libmethods$ext]
            if {![file readable $file]} {
                # Maybe, somebody is trying to run this extension
                # without installing it. Check for libmethods.so
                # in the current directory.
                set file libmethods$ext
            }
        }
        # The functions are shared between interpreters.
        # We don't want to reload them in case they are
        # already loaded by a master interpreter.
        set already_loaded 0
        foreach dll [hs::sharedlib list] {
            set name [hs::sharedlib name $dll]
            if {[string equal $name $file]} {
                set already_loaded 1
                break
            }
        }
        if {!$already_loaded && [file readable $file]} {
            set file [file join [pwd] $file]
    	    set dll [hs::sharedlib open -export_globals $file]
    	foreach {star tag name descr pars c_name needs_cernlib} {
    	    * null "Null function" "Function which returns 0" {} null_fit_fun 0

    	    * unity "Unity function" "Function which returns 1" {} unity_fit_fun 0

    	    * const "Constant function"
    	    "Function which returns a constant value set by a parameter"
    	    {c} const_fit_fun 0

    	    * x "x" "Function which returns its x argument" {} x_fit_fun 0

    	    * linear_1d "Straight Line"
    	    "Straight line: f(x) = a*x + b"
    	    {a b} linear_1d_fit_fun 0

    	    * exp_pdf "Exponential pdf"
    	    "Exponential probability density: f(x) =\
    		    area/abs(width)*exp(-(x-x0)/abs(width))\nThis\
    		    formula is valid when width != 0 and x >= x0,\
    		    otherwise f(x) is set to 0."
    	    {area x0 width} exp_pdf 0

            * huber "Huber least informative pdf"
            " Least informative probability density function for Huber's M-estimate:\n\
    	        ______________________________________________________________________\n\
    	        |                                                                     |\n\
    	        |         area        /     / x - mean \\\\                             |\n\
    	        |  f(x) = ---- * exp ( -rho(  --------  ))                            |\n\
    	        |         norm        \\     \\  |width| //                             |\n\
    	        |                                                                     |\n\
    	        |  norm = 2*|width|*(exp(-a^2/2)/|a| + sqrt(Pi/2)*erf(|a|/sqrt(2)));  |\n\
    	        |                                                                     |\n\
    	        |  rho(t) = t^2/2             if |t| < |a|                            |\n\
    	        |                                                                     |\n\
    	        |  rho(t) = |a|*|t| - a^2/2   if |t| > |a|                            |\n\
    	        |_____________________________________________________________________|\n\n\
    	        Essentially, it is Gaussian in the center and exponential in the tails,\n\
                    smoothly joined at x = |a|*|width| + mean. Note that in the above formula\n\
                    \"width\" and \"a\" may be both positive and negative."
    	    {area mean width a} huber_pdf 0

    	    * gauss "Gaussian pdf" 
    	    "Gaussian probability density function:\n\
    		    ___________________________________________________________\n\
    		    |                                                          |\n\
    		    |                 area                 /  (x - mean)^2 \\   |\n\
    		    |  f(x) = --------------------- * exp ( - ------------  )  |\n\
    		    |         sqrt(2*Pi)*abs(sigma)        \\    2*sigma^2  /   |\n\
    		    |__________________________________________________________|\n\n\
    		    Note that in this formula sigma may be both positive and negative."
    	    {area mean sigma} gaussian_pdf 0

    	    * linear_pdf "Linear pdf"
    	    "Linear probability density function:\n\n\
    		    f(x) ^                .\n\
    		    \ \ \ \ \ |               /|         The distribution parameters are:\n\
    		    \ \ \ \ \ |              / |            area     -- the total area under\n\
    		    \ \ \ \ \ |             /  |                          the curve\n\
    		    \ \ \ \ \ |            /   |            center   -- see picture\n\
    		    \ \ \ \ \ |           /    |            base     -- see picture\n\
    		    \ \ \ \ \ |          /     |            logratio -- log(H_l/H_r)\n\
    		    \ \ \ \ \ |         /      |\n\
    		    \ \ \ \ \ |        /       |H_r      The log of the ratio of the side heights\n\
    		    \ \ \ \ \ |       /        |         is used as a parameter instead of the\n\
    		    \ \ \ \ \ |      /         |         ratio itself in order to make sure that\n\
    		    \ \ \ \ \ |      |         |         the side heights have the same sign for\n\
    		    \ \ \ \ \ |   H_l|  center |         every value of the unbounded parameter.\n\
    		    \ \ \ \ \ |______|____|____|______\n\
    		    \ \ \ \ \ \ \ \ \ \ \ \ |         |     x   When logratio is fixed at 0, this function\n\
    		    \ \ \ \ \ \ \ \ \ \ \ \ |abs(base)|         becomes the uniform distribution.\n\
                        \ \ \ \ \ \ \ \ \ \ \ \ |<------->|"
    	    {area center base logratio} linear_pdf 0

    	    * bifgauss "Bifurcated Gaussian"
    	    "Bifurcated Gaussian probability density function:\n\
    		    ____________________________________________________________________________\n\
    		    |                                                                           |\n\
    		    |              area * sqrt(2)              /  (x - peak)^2 \\                |\n\
    		    | f(x) = -------------------------- * exp ( - ------------  )  if x < peak  |\n\
    		    |        sqrt(Pi)*(sigma_l+sigma_r)        \\   2*sigma_l^2 /                |\n\
    		    |                                                                           |\n\
    		    |              area * sqrt(2)              /  (x - peak)^2 \\                |\n\
    		    | f(x) = -------------------------- * exp ( - ------------  )  if x >= peak |\n\
    		    |        sqrt(Pi)*(sigma_l+sigma_r)        \\   2*sigma_r^2 /                |\n\
    		    |___________________________________________________________________________|\n\n\
    		    The function value is always 0 when either sigma_l or sigma_r is negative."
    	    {area peak sigma_l sigma_r} bifurcated_gaussian 0

    	    * bifcauchy "Bifurcated Cauchy"
    	    "Bifurcated Cauchy probability density function:\n\
    		    __________________________________________________________________\n\
    		    |                                                                 |\n\
    		    |             2 * area               hwhm_l^2                     |\n\
    		    | f(x) = ------------------ * ---------------------  if x < peak  |\n\
    		    |        Pi*(hwhm_l+hwhm_r)   hwhm_l^2 + (x-peak)^2               |\n\
    		    |                                                                 |\n\
    		    |                                                                 |\n\
    		    |             2 * area               hwhm_r^2                     |\n\
    		    | f(x) = ------------------ * ---------------------  if x >= peak |\n\
    		    |        Pi*(hwhm_l+hwhm_r)   hwhm_r^2 + (x-peak)^2               |\n\
    		    |_________________________________________________________________|\n\n\
    		    The function value is always 0 when either hwhm_l or hwhm_r is negative."
    	    {area peak hwhm_l hwhm_r} bifurcated_cauchy 0

    	    * tanh_thresh "Tanh threshold"
    	    "Hyperbolic tangent function with adjustable location, scale, and limits:\n\
    	    ________________________________________________________________\n\
    	    |                                                               |\n\
    	    |              (H_r - H_l)    /        /   Pi     x - locat \\\\  |\n\
    	    | f(x) = H_l + ----------- * (1 + tanh( ------- * ---------- )) |\n\
    	    |                   2         \\        \\sqrt(12)  abs(width)//  |\n\
    	    |_______________________________________________________________|\n\n\
    	    In this form, the standard deviation of the df/dx distribution equals to\n\
    	    abs(width), f(x) -> H_l as x -> -Infinity, f(x) -> H_r as x -> +Infinity,\n\
    	    and locat is the x coordinate of the point of inflection."
    	    {H_l H_r locat width} threshold_tanh 0

    	    * atan_thresh "Atan threshold"
    	    "Arc tangent function with adjustable location, scale, and limits:\n\
    	    _____________________________________________________________\n\
    	    |                                                            |\n\
    	    |                             /1   1         / x - locat \\\\  |\n\
    	    | f(x) = H_l + (H_r - H_l) * ( - + -- * atan( ----------- )) |\n\
    	    |                             \\2   Pi        \\ abs(width)//  |\n\
    	    |____________________________________________________________|\n\n\
    	    In this form, df/dx is the Cauchy distribution with half width\n\
    	    at half maximum equal to abs(width), f(x) -> H_l as x -> -Infinity,\n\
    	    f(x) -> H_r as x -> +Infinity, and locat is the x coordinate of\n\
    	    the point of inflection."
    	    {H_l H_r locat width} threshold_atan 0

    	    * erf_thresh "Erf threshold"
    	    "Gaussian cumulative distribution function with adjustable location,\
    		    scale,\nand limits:\n\
    	    _____________________________________________________________\n\
    	    |                                                            |\n\
    	    |              (H_r - H_l)    /       /    x - locat     \\\\  |\n\
    	    | f(x) = H_l + ----------- * (1 + erf( ------------------ )) |\n\
    	    |                   2         \\       \\sqrt(2)*abs(width)//  |\n\
    	    |____________________________________________________________|\n\n\
    	    In this form, df/dx is the Gaussian pdf with standard deviation equal to\n\
    	    abs(width), f(x) -> H_l as x -> -Infinity, f(x) -> H_r as x -> +Infinity,\n\
    	    and locat is the x coordinate of the point of inflection."
    	    {H_l H_r locat width} threshold_erf 0

            * gamma_cdf "Gamma cdf"
            "Cumulative density function of the gamma distribution. Derivative of\nthis\
                    function is the gamma probability density with corresponding values\nof\
                    parameters x0, width, and A. Area corresponds to H_r - H_l."
            {H_l H_r x0 width A} gamcdf_ 1

    	    * cauchy "Cauchy pdf"
    	    "Cauchy probability density function (also known as Breit-Wigner pdf):\n\
    		    _____________________________________\n\
    		    |                                    |\n\
    		    |        area        abs(hwhm)       |\n\
    		    | f(x) = ---- * -------------------  |\n\
    		    |         Pi    hwhm^2 + (x-peak)^2  |\n\
    		    |____________________________________|\n\n\
    		    \"hwhm\" stands for half width at half maximum."
    	    {area peak hwhm} cauchy_pdf 0

    	    * logistic "Logistic pdf"
    	    "Probability density function of the logistic distribution:\n\
    		    _________________________________________________________________\n\
    		    |                                                                |\n\
    		    |               area*Pi           /    /   Pi      x - mean \\\\ 2 |\n\
    		    | f(x) = --------------------- * (sech( ------- * ---------- ))  |\n\
    		    |        2*sqrt(12)*abs(sigma)    \\    \\sqrt(12)  abs(sigma)//   |\n\
    		    |________________________________________________________________|\n\n\
    		    Note that in this formula sigma may be both positive and negative.\n\
                        When positive, it is the distribution's standard deviation. In literature,\n\
                        this density is usually parameterized as sech((x-mean)/(2*b))^2/(4*b)\n\
                        or, identically, exp(-(x-mean)/b)/b/(1+exp(-(x-mean)/b))^2. For such\n\
                        a parameterization, standard deviation is sqrt(Pi^2*b^2/3)."
    	    {area mean sigma} sechsquared_pdf 0

            * extreme_max "Extreme value distribution (maximum)"
            "Extreme value (Gumbel) probability density function of the largest\
                    extreme:\narea/abs(b)*exp(q-exp(q)) where q=-(x-a)/abs(b)."
            {area a b} gumbel_pos_pdf 0

            * extreme_min "Extreme value distribution (minimum)"
            "Extreme value (Gumbel) probability density function of the smallest\
                    extreme:\narea/abs(b)*exp(q-exp(q)) where q=(x-a)/abs(b)."
            {area a b} gumbel_neg_pdf 0

    	    * students_t "Student's t-distribution cdf"
    	    "Cumulative distribution function for Student's\
    		    t-distribution with n degrees\nof freedom.\
    		    The implementation is based on the STUDIS function, CERNLIB\
    		    entry\nG104, whose accuracy is about 6 decimal places."
    	    {n} studcd_ 1

    	    * chisq_tail "Chi-squared tail"
    	    "This function returns one minus chi-squared cumulative distribution function\nwith\
    		    n degrees freedom. The implementation is based on the PROB function,\nCERNLIB\
    		    entry G100, which has the accuracy of about 6 digits up to n\
    		    = 300.\nFor n > 300, the accuracy decreases for x > n with increasing x."
    	    {n} chtail_ 1

    	    * landau "Landau distribution"
    	    "The Landau probability density function describes the fluctuations of\nthe\
    		    ionization energy loss when a charged particle passes through a thin\n\layer\
    		    of matter. The function implemented in this program also includes\n\shift\
    		    and scale transformations:\n\
    		    ________________________________________\n\
    		    |                                       |\n\
    		    |           area          / x - peak \\  |\n\
    		    | f(x) = ---------- * phi( ---------- ) |\n\
    		    |        abs(width)       \\abs(width)/  |\n\
    		    |_______________________________________|\n\n\where\
    		    phi is the universal Landau function. Note that in this formula width\n\may\
    		    be both positive and negative. The implementation is based on the DENLAN\nfunction,\
    		    CERNLIB entry G110, which effectively has single precision."
    	    {area peak width} landau_ 1

    	    * lrc_rise "LRC step response"
    	    "This is the response of an L-R-C circuit\
    		    to a voltage step at time t0 taken\nas voltage across the capacitor:\n\nV(t) =\
    		    H_l + (H_r - H_l)*S(t-t0, beta, omega)  if beta >= 0 and omega >= -beta\nV(t) =\
    		    0  if beta < 0 or omega < -beta (unphysical values)\n\nIn\
    		    this formula, S(t, beta, omega) is the canonical L-R-C circuit step\nresponse,\
    		    t is time, beta is the damping coefficient, and omega is\nthe\
    		    resonant frequency. For a real L-R-C circuit beta is positive and\nequals\
    		    to R/(2*L), while omega = sqrt(omega0^2 - beta^2), where omega0\nis the\
    		    resonant frequency of the LC circuit: omega0 = 1/sqrt(L*C). Negative\nvalues\
    		    of omega correspond to overdamping."
    	    {H_l H_r t0 beta omega} lrcrise_ 0

    	    * jsu_t "Johnson's S_u (translation)"
                " Johnson's S_u probability density function parameterized by area and\n\
                translation parameters xi, lambda, gamma, and delta:\n\
                ____________________________________________________________________________\n\
                |                                                                           |\n\
                |                            /  1    /                        / x-xi \\\\2\\   |\n\
                |        area * delta * exp ( - - * ( gamma + delta * invsinh( ------ )) )  |\n\
                |			      \\  2    \\                        \\lambda// /   |\n\
                | f(x) = -----------------------------------------------------------------  |\n\
                |                                            /     / x-xi \\2\\               |\n\
                |	           lambda * sqrt(2*Pi) * sqrt( 1 + ( ------ ) )              |\n\
                |	                                      \\     \\lambda/ /               |\n\
                |___________________________________________________________________________|\n\n\
                where invsinh denotes the inverse hyperbolic sinus function. It is assumed\n\
                that lambda > 0, otherwise f(x) is set to be identically 0. Distribution of\n\
                z = gamma + delta * invsinh((x - xi)/lambda) is described by the standard\n\
                normal curve."
                {area xi lambda gamma delta} johnsn_ 1

    	    * jsb_t "Johnson's S_b (translation)"
                " Johnson's S_b probability density function parameterized by area and\n\
                translation parameters xi, lambda, gamma, and delta:\n\
                ____________________________________________________________________________\n\
                |                                                                           |\n\
                |                           /  1    /                    /    x-xi   \\\\2\\   |\n\
                |        area * delta * exp( - - * ( gamma + delta * log( ----------- )) )  |\n\
                |			     \\  2    \\                    \\xi+lambda-x// /   |\n\
                | f(x) = -----------------------------------------------------------------  |\n\
                |                                        x-xi     /     x-xi \\              |\n\
                |	           lambda * sqrt(2*Pi) * ------ * ( 1 - ------ )             |\n\
                |	                                 lambda    \\    lambda/              |\n\
                |___________________________________________________________________________|\n\n\
                This formula is valid when lambda > 0 and xi < x < xi + lambda, otherwise\n\
                f(x) is set to be identically 0. Distribution of\n\
                z = gamma + delta * log((x - xi)/(xi + lambda - x)) is described by\n\
                the standard normal curve."
    	    {area xi lambda gamma delta} johnb_ 1

    	    * lognormal "Log-normal pdf"
    	    "Log-normal probability density function is parameterized here by its area,\nmean\
    	    value, standard deviation, and skewness. In statistical literature it\nis usually\
    	    represented using some other parameters, for example\n\n\
    	    \ f(x) = area/(sqrt(2*Pi)*s*(x-x0)) * exp(-(log(x-x0) - mu)^2/s^2/2).\n\nThe\
    	    logarithm of (x-x0) has normal distribution. Although the formula which\nuses\
    	    the standard moments is too cumbersome to give here, it is more useful\nfor\
    	    data fitting because the parameters have natural meaning. Both positive\nand\
    	    negative values of the standard deviation are permitted, and the absolute\nvalue\
    	    is used in all calculations."
    	    {area mean sigma skew} lognorm_ 1

            * johnson "Johnson's system"
            "Johnson's system of frequency curves (Gaussian, Lognormal, S_b, S_u)\nparameterized\
            by area, mean, standard deviation, skewness, and kurtosis.\nThis\
            system spans all theoretically possible values of skewness and kurtosis.\nThe\
            value of the standard deviation must be\ positive, and kurtosis must be\nlarger\
            than skewness squared plus one, otherwise the function is set to be\nidentically\
            0. Note that this system has at best single precision (limited\nby S_b)."
            {area mean sigma skew kurt} johnsys_ 1

    	    * beta "Beta distribution"
    	    "Beta probability density function:\n\
    		    ________________________________________________________________________\n\
    		    |                                                                       |\n\
    		    |            area * Gamma(M + N)        / x-min \\(M-1)   / max-x \\(N-1) |\n\
    		    | f(x) = --------------------------- * ( ------- )    * ( ------- )     |\n\
    		    |        Gamma(M)*Gamma(N)*(max-min)    \\max-min/        \\max-min/      |\n\
    		    |_______________________________________________________________________|\n\n\
    		    The function value is by definition 0 for x <= min or x => max or max <= min\n\
    		    or M < 0 or N < 0. Many simple distributions are special cases of the beta\n\
    		    distribution: uniform, triangle, biweight, triweight, etc."
    	    {area min max M N} betafun_ 1

    	    * gamma "Gamma distribution"
    	    "Gamma probability density function:\n\
    		    _____________________________________________________________________________\n\
    		    |                                                                            |\n\
    		    |         area * sqrt(A)      /sqrt(A)*(x-x0)\\(A-1)      /  sqrt(A)*(x-x0)\\  |\n\
    		    | f(x) = ----------------- * ( -------------- )    * exp( - -------------- ) |\n\
    		    |       abs(width)*Gamma(A)   \\  abs(width)  /           \\    abs(width)  /  |\n\
    		    |____________________________________________________________________________|\n\n\
    		    The function value is by definition 0 for x <= x0 or A <= 0. The above\n\
    		    formula results in the standard deviation equal to the absolute value of\n\
    		    the width parameter. The exponential distribution is a special case of\n\
    		    the gamma distribution with A = 1. The chi-squared distribution with N\n\
    		    degrees of freedom is a special case of the gamma distribution with A = N/2.\n\
    		    The Erlang distribution is a special case of the gamma distribution when\n\
    		    A is a positive integer."
    	    {area x0 width A} gampdf_ 1

    	    * exp_rise "Exp risetime (RC step response)"
    	    "f(x) = H_l + (H_r - H_l) * (1 - exp(-(x - x0)/abs(width)))  if x >= x0\nf(x)\
    		    = H_l  if (x < x0)\n\nWhen H_r is fixed at 0,\
    		    this function becomes the exponential probability\ndensity function with\
    		    area equal to H_l * width."
    	    {H_l H_r x0 width} expthresh 0

    	    * weibull "Weibull/Frechet pdf"
    	    "Weibull/Frechet probability density function:\n\
    		    _____________________________________________________________________________\n\
    		    |                                                                            |\n\
    		    |        area*abs(beta)    /  x - x0  \\(beta-1)       /   /  x - x0  \\beta\\  |\n\
    		    | f(x) = -------------- * ( ---------- )       * exp ( - ( ---------- )    ) |\n\
    		    |          abs(scale)      \\abs(scale)/               \\   \\abs(scale)/    /  |\n\
    		    |____________________________________________________________________________|\n\n\
    		    This formula is valid when scale != 0, beta != 0, and x > x0, otherwise\n\
    		    f(x) is set to 0. When beta > 0 it becomes pdf of the Weibull distribution,\n\
    		    and when beta < 0 it becomes pdf of the Frechet distribution. Note that the\n\
    		    scale parameter may be both positive and negative."
    	    {area x0 scale beta} weibull_pdf 0
    	} {
    	    if {!$needs_cernlib || [::hs::have_cernlib]} {
    		::hs::Import_simple_function $tag $dll $name $descr $pars $c_name
    	    }
    	}

    	set parlist x0
    	for {set i 0} {$i < 255} {incr i} {lappend parlist a$i}
    	hs::function_import poly_1d $dll "Polynomial Function" \
    		"Polynomial function with moving origin:\n\
    		____________________________________________________________\n\
    		|                                                           |\n\
    		|  f(x) = a0 + a1*(x-x0) + a2*(x-x0)^2 + a3*(x-x0)^3 + ...  |\n\
    		|___________________________________________________________|\n\n\
    		The degree of the polynomial equals to (mode - 2). The x0 parameter is\n\
    		redundant; during fitting it should be fixed at some convenient value."\
    		1 4 3 256 $parlist {} poly_curve {} {}

    	set parlist {min max}
    	for {set i 0} {$i < 255} {incr i} {lappend parlist a$i}
    	hs::function_import poly_cheb $dll "Chebyshev Polynomial"\
    		"Chebyshev polynomial series:\n\
    		_________________________________________________________________\n\
    		|                                                                |\n\
    		|  f(x) = a0*C0(t) + a1*C1(t) + a2*C2(t) + ...,  min <= x <= max |\n\
    		|                                                                |\n\
    		|  f(x) = 0,  x < min or x > max or min >= max                   |\n\
    		|                                                                |\n\
    		|  t = 2*(x-min)/(max-min) - 1                                   |\n\
    		|                                                                |\n\
    		|  Cn(t) = cos(n*arccos(t))                                      |\n\
    		|________________________________________________________________|\n\n\
    		Note that parameters min and max should not be fitted for; rather, they\n\
    		should be fixed at some convenient values (usually, histogram limits)."\
    		1 5 3 257 $parlist {} poly_chebyshev {} {}

    	set parlist {min max}
    	for {set i 0} {$i < 255} {incr i} {lappend parlist a$i}
    	hs::function_import poly_legendre $dll "Legendre Polynomial"\
    		"Legendre polynomial series:\n\
    		_________________________________________________________________________\n\
    		|                                                                        |\n\
    		|  f(x) = a0*L0(t) + a1*L1(t) + a2*L2(t) + ...,  min <= x <= max         |\n\
    		|                                                                        |\n\
    		|  f(x) = 0,  x < min or x > max or min >= max                           |\n\
    		|                                                                        |\n\
    		|  t = 2*(x-min)/(max-min) - 1                                           |\n\
    		|                                                                        |\n\
    		|  L0(t) = 1,  L1(t) = t,  Ln(t) = ((2*n-1)*t*Ln-1(t) - (n-1)*Ln-2(t))/n |\n\
    		|________________________________________________________________________|\n\n\
    		Note that parameters min and max should not be fitted for; rather, they\n\
    		should be fixed at some convenient values (usually, histogram limits)."\
    		1 5 3 257 $parlist {} poly_legendre {} {}

    	hs::function_import data_1d $dll "Data Curve 1d" \
    		"1-dimensional data curve which can be shifted and scaled, both\
    		horizontally\nand vertically. The function mode should be set to\
    		the Histo-Scope id of\na 1d histogram used as a source of data or\
    		to the id of an ntuple with\ntwo variables. Note that as long as\
    		this function is used in a fit, it\ncaches the data internally,\
    		and it will not be updated when the histogram\nor ntuple is filled."\
                    1 -1 4 4 {h_scale h_shift v_scale v_shift} \
    		data_curve_1d_init data_curve_1d {} data_curve_1d_cleanup

    	hs::function_import data_2d $dll "Data Curve 2d" \
    		"2-dimensional data curve which can be shifted, scaled, and rotated\
    		in\nthe x-y plane as well as shifted and scaled vertically. The\
    		function\nmode should be set to the Histo-Scope id of a 2d histogram\
    		used as\na source of data. The histogram transformations in the x-y\
    		plane are\nperformed in the following order: 1) scaling, 2) shifting,\
    		3) rotation\naround the histogram center. Note that as long as this\
    		function is used\nin a fit, it caches the data internally, and it will\
    		not be updated\nwhen the histogram is filled."\
                    2 -1 7 7 {x_scale y_scale x_shift y_shift angle v_scale v_shift} \
    		data_curve_2d_init data_curve_2d {} data_curve_2d_cleanup

    	hs::function_import bivar_gauss $dll "Bivariate Gaussian" \
    		"Bivariate Gaussian density is parameterized using volume, means and\
    		standard\n\deviations along the coordinate axes, and the correlation coefficient:\n\
    		__________________________________________________________\n\
    		|                                                         |\n\
    		|            x - mean_x               y - mean_y          |\n\
    		|      dx = ------------        dy = ------------         |\n\
    		|           abs(sigma_x)             abs(sigma_y)         |\n\
    		|                                                         |\n\
    		|                         / dx^2 + dy^2 - 2*rho*dx*dy\\    |\n\
    		|           volume * exp (- ------------------------- )   |\n\
    		|                         \\      2 * (1 - rho^2)     /    |\n\
    		| f(x,y) = ---------------------------------------------  |\n\
    		|          2*Pi*abs(sigma_x)*abs(sigma_y)*sqrt(1 - rho^2) |\n\
    		|_________________________________________________________|\n\n\Note\
    		that in this formula standard deviations may be both positive\
    		and\nnegative. It is assumed that the absolute value of the\
    		correlation coefficient\n\is less than one, otherwise the\
    		function is set to be identically 0."\
    		2 0 6 6 {volume mean_x mean_y sigma_x sigma_y rho} \
    		{} bivariate_gaussian {} {}

    	hs::function_import trivar_gauss $dll "Trivariate Gaussian" \
    		"Trivariate Gaussian density" 3 0 10 10 \
    		{volume4d mean_x mean_y mean_z sigma_x sigma_y sigma_z rho_xy rho_xz rho_yz} \
    		{} trivariate_gaussian {} {}

    	hs::function_import linear_2d $dll "Plane" \
    		"2d plane: f(x,y) = a*x + b*y + c" 2 0 3 3 \
    		{a b c} {} linear_2d_fit_fun {} {}

    	hs::function_import quadratic_2d $dll "Quadratic 2d polynomial" \
    		"Quadratic 2d polynomial: f(x,y) = a*x^2 + b*x*y + c*y^2 + d*x + e*y + f" \
    		2 0 6 6 {a b c d e f} {} quadratic_2d_fit_fun {} {}

    	hs::function_import linear_3d $dll "Hyperplane" \
    		"3d hyperplane: f(x,y,z) = a*x + b*y + c*z + d" 3 0 4 4 \
    		{a b c d} {} linear_3d_fit_fun {} {}

    	# Some default parameter settings for the function browser
    	# and fit tuner
    	variable Hs_default_parameter_range
    	variable Hs_function_type
    	foreach {tag type settings} {
    	    bivar_gauss pdf {
    		volume  0 10 1
    		mean_x -5 5  0
    		mean_y -5 5  0
    		sigma_x 0 10 1
    		sigma_y 0 10 1
    		rho    -1 1  0
    	    }
    	    beta pdf {
    		area 0 10 1
    		min -10 10 -3
    		max -10 10 4
    		M    0 20 3
    		N    0 20 7
    	    }
    	    gamma pdf {
    		area 0 10 1
    		x0 -10 10 -3
    		width 0 10 1
    		A 0 100 5
    	    }
    	    bifgauss pdf {
    		area 0 10 1
    		peak -10 10 -1
    		sigma_l 0 10 0.5
    		sigma_r 0 10 1.5
    	    }
    	    bifcauchy pdf {
    		area 0 10 1
    		peak -10 10 -1
    		hwhm_l 0 10 0.5
    		hwhm_r 0 10 1.5
    	    }
    	    atan_thresh thresh {
    		H_l -5 5 0
    		H_r -5 5 1
    		locat -10 10 0
    		width 0 10 1
    	    }
    	    students_t thresh {
    		n 1 50 10
    	    }
    	    chisq_tail thresh {
    		n 1 20 5
    	    }
    	    tanh_thresh thresh {
    		H_l -5 5 0
    		H_r -5 5 1
    		locat -10 10 0
    		width 0 10 1
    	    }
    	    erf_thresh thresh {
    		H_l -5 5 0
    		H_r -5 5 1
    		locat -10 10 0
    		width 0 10 1
    	    }
            gamma_cdf thresh {
                H_l -5 5 0
    		H_r -5 5 1
    		x0 -10 10 -3
    		width 0 10 1
    		A 0 100 5
            }
    	    exp_rise thresh {
    		H_l -5 5 0
    		H_r -5 5 1
    		x0 -10 10 -3
    		width 0 10 1
    	    }
    	    gauss pdf {
    		area 0 10 1
    		mean -10 10 0
    		sigma 0 10 1
    	    }
    	    huber pdf {
    		area 0 10 1
    		mean -10 10 0
    		width 0 10 1
    		a     0 5  1
    	    }
    	    logistic pdf {
    		area 0 10 1
    		mean -10 10 0
    		sigma 0 10 1
    	    }
            extreme_max pdf {
                area  0  10 1
                a    -10 10 -2
                b     0  5  1
            }
            extreme_min pdf {
                area  0  10 1
                a    -10 10 2
                b     0  5  1
            }
    	    landau pdf {
    		area 0 10 1
    		peak -10 10 -2
    		width 0 3 0.3
    	    }
    	    cauchy pdf {
    		area 0 10 1
    		peak -10 10 0
    		hwhm 0 5 0.5
    	    }
            exp_pdf pdf {
    		area  0 10 1
    		x0   -10 10 -4
    		width 0 10 1
    	    }
    	    linear_pdf pdf {
    		area 0 10 1
    		center -10 10 0
    		base 0 10 5
    		logratio -10 10 -0.5
    	    }
    	    linear_1d poly {
    		a -10 10 1
    		b -50 50 1
    	    }
    	    lognormal pdf {
    		area 0 10 1
    		mean -10 10 -1
    		sigma 0 10 1
    		skew -10 10 2
    	    }
    	    lrc_rise misc {
    		H_l -5 5 0
    		H_r -5 5 1
    		t0 -10 10 -3
    		beta 0 10 0.5
    		omega -10 10 3
    	    }
    	    null trivial {}
    	    unity trivial {}
    	    x trivial {}
    	    const trivial {
    		c 0 10 5
    	    }
    	    johnson pdf {
    		area  0  10 1
    		mean -10 10 -1
    		sigma 0  10 1
    		skew -4  4 0.5
    		kurt  1  40 2
    	    }
    	    jsb_t pdf {
    		area  0   10  1
    		xi    -10 10 -3
    		lambda 0  15  6
    		gamma -10 10  0.3
                delta -10 10  0.5
    	    }
    	    jsu_t pdf {
    		area   0  10  1
    		xi    -10 10 -3
    		lambda 0  5 0.5
    		gamma -10 10 -1
    		delta -10 10  1
    	    }
    	    weibull pdf {
    		area   0  10  1
    		x0    -10 10 -3
    		scale  0  10  2
    		beta  -10 10 1.5
    	    }
    	    data_1d misc {
    		h_scale 0   10 1
    		h_shift -10 10 0
    		v_scale 0   10 1
    		v_shift -10 10 0
    	    }
    	} {
    	    # Check that all parameter names are OK
    	    if {[::hs::have_cernlib]} {
    		set parlist [hs::function $tag cget -parameters]
    	    } elseif {[catch {hs::function $tag cget -parameters} parlist]} {
    		# No such function. We are going to have undefined
    		# function in this iteration if the extension was
    		# compiled without CERNLIB.
    		continue
    	    }
    	    foreach {pname min max init} $settings {
    		if {[lsearch -exact $parlist $pname] < 0} {
    		    error "Can't set up default parameter settings:\
    			    bad parameter name $pname for function $tag"
    		}
    	    }
    	    set signature [hs::Function_signature $tag]
    	    set Hs_default_parameter_range($signature) $settings
    	    set Hs_function_type($signature) $type
    	}
    	# Polynomials do not have default parameter settings
    	# because the number of parameters is unknown, but they
    	# do have type.
    	foreach tag {poly_1d poly_cheb poly_legendre} {
    	    set signature [hs::Function_signature $tag]
    	    set Hs_function_type($signature) poly
    	}
        }

        # Initialize the namespace variables
        namespace eval ::hs:: {
    	variable Histogram_fill_styles {
    	    none
    	    solid
    	    fineHoriz
    	    coarseHoriz
    	    fineVert
    	    coarseVert
    	    fineGrid
    	    coarseGrid
    	    fineX
    	    coarseX
    	    fine45deg
    	    med45deg
    	    coarse45deg
    	    fine30deg
    	    coarse30deg
    	    fine60deg
    	    coarse60deg
    	    rFine45deg
    	    rMed45deg
    	    rCoarse45deg
    	    rFine30deg
    	    rCoarse30deg
    	    rFine60deg
    	    rCoarse60deg
    	    lFineHoriz
    	    lCoarseHoriz
    	    lFineVert
    	    lCoarseVert
    	    lFineGrid
    	    lCoarseGrid
    	    lFineX
    	    lCoarseX
    	    lFine45deg
    	    lMed45deg
    	    lCoarse45deg
    	    lFine30deg
    	    lCoarse30deg
    	    lFine60deg
    	    lCoarse60deg
    	}
    	variable Overlay_id 0
    	variable Parse_tclConfig_sh_error 0
    	variable Recognized_fortran_extensions {f for FOR F fpp FPP}
    	variable Temporary_category_name "Hs_plots_tmp"
    	variable Sharedlib_suffix [::info sharedlibextension]
    	variable Temporary_category_uid 0
    	variable Ntuple_plot_counter 0
    	variable Histo_task_counter 0
    	variable Expr_plot_counter 0
    	variable Parametric_plot_counter 0
    	variable List_plot_counter 0
        variable Function_plot_counter 0
    	variable Plot_tcl_overlay_counter 0
    	variable Compiled_function_counter 0
    	variable Hs_browse_collection_counter 0
    	variable Browse_collection_show_selected_winnum 0
    	variable Hs_function_browser_counter 0
    	variable Horizontal_separator_number 0
    	variable Color_scale_counter 0
    	variable C_struct_counter 0
    	variable Font_list_queried 0
    	variable History_buffer_size 100
    	variable Last_window_posted ""
    	variable Distinct_window_names 0
        variable Histoscope_header "histoscope_stub.h"
        catch {
            if {[string equal -nocase -length 5 [exec uname] "linux"]} {
                set Histoscope_header "histoscope.h"
            }
        }
    	namespace export {[a-z123]*}
        }
    }
    ::hs::Init_tcl_api
    ::fit::Init_tcl_api
    ::hs::Locate_tch_header
    ::rename ::hs::Init_tcl_api {}
    ::rename ::fit::Init_tcl_api {}
    ::rename ::hs::Import_simple_function {}
    global ::errorInfo
    set ::errorInfo ""
}

#########################################################################

proc ::hs::histoscope {flag} {
    hs::Histo_dir_env
    hs::Histoscope $flag
}

#########################################################################

proc ::hs::histo_with_config {flag filename} {
    hs::Histo_dir_env
    hs::Histo_with_config $flag $filename
}

#########################################################################

proc ::hs::replace_color_scale {old_name new_name} {
    variable Color_scale_data
    foreach name [list $old_name $new_name] {
	if {![info exists Color_scale_data($name,ncolors)]} {
	    error "Bad color scale tag \"$name\""
	}
    }
    if {[string equal $old_name $new_name]} {
	return
    }
    set Color_scale_data($old_name,cspace)    $Color_scale_data($new_name,cspace)
    set Color_scale_data($old_name,ncolors)   $Color_scale_data($new_name,ncolors)
    set Color_scale_data($old_name,start)     $Color_scale_data($new_name,start)
    set Color_scale_data($old_name,end)       $Color_scale_data($new_name,end)
    set Color_scale_data($old_name,underflow) $Color_scale_data($new_name,underflow)
    set Color_scale_data($old_name,overflow)  $Color_scale_data($new_name,overflow)
    set Color_scale_data($old_name,linear)    $Color_scale_data($new_name,linear)
    set Color_scale_data($old_name,pixels)    $Color_scale_data($new_name,pixels)
    if {$Color_scale_data($old_name,persistent)} {
	hs::use_color_scale $old_name
    }
    return
}

#########################################################################

proc ::hs::use_color_scale {scale_tcl_name} {
    variable Color_scale_data
    if {![info exists Color_scale_data($scale_tcl_name,ncolors)]} {
	error "Bad color scale tag \"$scale_tcl_name\""
    }
    if {[hs::num_connected_scopes] == 0} {
	return
    }
    set config_string ""
    hs::Append_color_scale_def config_string $scale_tcl_name $scale_tcl_name
    append config_string " PersistentColorScale\n"
    hs::load_config_string $config_string
    hs::hs_update
    incr Color_scale_data($scale_tcl_name,persistent)
    return
}

#########################################################################

proc ::hs::create_linear_color_scale {colorspace \
	ncolors underflow start end overflow} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }
    set valid_color_spaces {
	"RGB"
	"RGBi"
	"CIEXYZ"
	"CIEuvY"
	"CIExyY"
	"CIELab"
	"CIELuv"
	"TekHVC"
	"HSV"
	"HLS"
    }
    
    # Check the color space supplied
    set colorspace_found 0
    foreach cspace $valid_color_spaces {
	if {[string equal -nocase $colorspace $cspace]} {
	    set colorspace_found 1
	    set colorspace $cspace
	    break
	}
    }
    if {!$colorspace_found} {
	error "Unknown color space \"$colorspace\". Please use\
		one of [join $valid_color_spaces {, }]."
    }

    # Check the number of colors
    foreach {ncolors_min ncolors_max} [hs::Num_scale_colors_limits] {}
    if {![string is integer -strict $ncolors]} {
	error "Expected an integer, got \"$ncolors\""
    }
    if {$ncolors < $ncolors_min || $ncolors > $ncolors_max} {
	error "Number of colors is out of range,\
		should be between $ncolors_min and $ncolors_max"
    }

    # Check color definitions
    ::hs::Verify_color_names [list $start $end $underflow $overflow]

    # Looks like everything is fine
    variable Color_scale_data
    set name [hs::Next_color_scale_name]
    set Color_scale_data($name,cspace) $colorspace
    set Color_scale_data($name,ncolors) $ncolors
    set Color_scale_data($name,start) $start
    set Color_scale_data($name,end) $end
    set Color_scale_data($name,underflow) $underflow
    set Color_scale_data($name,overflow) $overflow
    set Color_scale_data($name,linear) 1
    set Color_scale_data($name,persistent) 0
    set Color_scale_data($name,pixels) {}

    return $name
}

#########################################################################

proc ::hs::Append_color_scale_def {string_name \
	scale_tcl_name scale_histoscope_name} {
    upvar $string_name config_spec
    variable Color_scale_data
    if {![info exists Color_scale_data($scale_tcl_name,ncolors)]} {
	error "Bad color scale tag \"$scale_tcl_name\""
    }
    append config_spec "ColorScale\n\
	    Title $scale_histoscope_name\n\
	    ColorSpace $Color_scale_data($scale_tcl_name,cspace)\n\
	    NumberOfColors $Color_scale_data($scale_tcl_name,ncolors)\n\
	    StartColor $Color_scale_data($scale_tcl_name,start)\n\
	    EndColor $Color_scale_data($scale_tcl_name,end)\n\
	    UnderflowColor $Color_scale_data($scale_tcl_name,underflow)\n\
	    OverflowColor $Color_scale_data($scale_tcl_name,overflow)\n"
    if {$Color_scale_data($scale_tcl_name,linear)} {
	append config_spec " LinearColorScale\n"
    } else {
	set i 0
	foreach color $Color_scale_data($scale_tcl_name,pixels) {
	    append config_spec " PixelColor $i $color\n"
	    incr i
	}
    }
    return
}

#########################################################################

proc ::hs::Generate_xlfd {fontname} {
    variable Generate_xlfd_cached_fontmap
    if {[info exists Generate_xlfd_cached_fontmap($fontname)]} {
	return $Generate_xlfd_cached_fontmap($fontname)
    }

    # Font name cannot be an empty string
    if {[string equal $fontname ""]} {
	error "font \"$fontname\" doesn't exist"
    }

    # Fonts should not contain []? characters
    # (these characters may later screw up string matching)
    if {[regexp {[\]\[\?]} $fontname]} {
	error "font \"$fontname\" doesn't exist"
    }
    if {[string first "*" $fontname] < 0} {
	set has_asterisk 0
    } else {
	set has_asterisk 1
    }

    # Try to figure out if this font is already in XLFD format.
    # An XLFD font name cannot contain a space immediately before a dash.
    set maybe_xlfd 1
    if {[string first " -" $fontname] >= 0} {
	set maybe_xlfd 0
    }

    # An XLFD font name must have at least one '*' in case it has a space
    # and fewer than 14 dashes (font aliases may not contain spaces)
    if {$maybe_xlfd} {
	if {$has_asterisk} {
	    if {[string first " " $fontname] >= 0} {
		if {[regexp -all -- "-" $fontname] < 14} {
		    set maybe_xlfd 0
		}
	    }
	}
    }

    # Load the list of fonts
    variable Font_list_queried
    variable Font_list_data
    if {!$Font_list_queried} {
	foreach font [split [exec xlsfonts] "\n"] {
	    set Font_list_data($font) 1
	}
	set Font_list_queried 1
    }

    # Check the match to the font list
    if {$maybe_xlfd} {
	if {$has_asterisk} {
	    if {[llength [array names Font_list_data $fontname]]} {
		# Matches at least one X font
		set Generate_xlfd_cached_fontmap($fontname) $fontname
		return $fontname
	    }
	} else {
	    if {[info exists Font_list_data($fontname)]} {
		# Matches exactly one X font
		set Generate_xlfd_cached_fontmap($fontname) $fontname
		return $fontname
	    }
	}
    }

    # At this point we are sure that fontname is either
    # not using XLFD format or such font does not exist
    if {$has_asterisk} {
	# Can't be a Tk font
	error "font \"$fontname\" doesn't exist"
    }

    # Generate XLFD from a Tk font
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }
    foreach {option value} [font actual $fontname] {
	regsub "^-" $option "" option
	set $option $value
    }
    set family [string tolower $family]

    # Pre-select fonts compatible with the given family
    array unset candidate_fonts
    array set candidate_fonts [array get Font_list_data "-*-$family-*"]
    if {[array size candidate_fonts] == 0} {
        set tryfamily [string tolower [lindex $fontname 0]]
        array set candidate_fonts [array get Font_list_data "-*-$tryfamily-*"]
        if {[array size candidate_fonts] > 0} {
            set family $tryfamily
        } else {
            error "xlsfonts gives no fonts in the family \"$family\""
        }
    }

    # Create an XLFD string
    if {[string equal $weight "normal"]} {
	set weightlist [list medium normal regular book light]
    } else {
	set weightlist [list bold demibold "demi bold"]
    }
    if {[string equal $slant "roman"]} {
	set slantlist [list r]
    } else {
	set slantlist [list i o]
    }
    # Try "normal" set width first because of its better
    # similarity to the corresponding PostScript font
    foreach set_width {normal semicondensed condensed} {
	foreach weight $weightlist {
	    foreach slant $slantlist {
		# Try both bitmapped and scalable fonts
		foreach sizepattern [list "$size-*" "0-0-*-*-*-0-*"] scaled {0 1} {
		    set xlfd_pattern "-*-$family-$weight-$slant-$set_width-*-$sizepattern"
		    if {[llength [array names candidate_fonts $xlfd_pattern]] > 0} {
			if {$scaled} {
			    set answer "-*-$family-$weight-$slant-$set_width-*-$size-*-*-*-*-*-*-*"
			} else {
			    set answer "-*-$family-$weight-$slant-$set_width-*-$size-*"
			}
			set Generate_xlfd_cached_fontmap($fontname) $answer
			return $answer
		    }
		}
	    }
	}
    }
    error "font \"$fontname\" doesn't exist"
}

#########################################################################

proc ::hs::Verify_color_names {colorlist} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }
    if {[llength $colorlist] == 0} {
	return
    }
    label .color_checking_label_dszf9ewfik
    global ::errorInfo
    set status [catch {
	foreach colorname $colorlist {
	    if {[string equal -nocase -length 4 $colorname "HSV:"] || \
		    [string equal -nocase -length 4 $colorname "HLS:"]} {
		set triple [string range $colorname 4 end]
		set data [split $triple /]
		if {[llength $data] != 3} {
		    error "unknown color name \"$colorname\""
		}
		foreach spec $data {
		    if {![string is double -strict $spec]} {
			error "unknown color name \"$colorname\""
		    }
		}
	    } else {
		.color_checking_label_dszf9ewfik configure -background $colorname
	    }
	}
    } errMes]
    set savedInfo $::errorInfo
    destroy .color_checking_label_dszf9ewfik
    if {$status} {
	error $errMes $savedInfo
    }
    return
}

#########################################################################

proc ::hs::create_general_color_scale {underflow colorlist overflow} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }

    # Check the number of colors
    set ncolors [llength $colorlist]
    foreach {ncolors_min ncolors_max} [hs::Num_scale_colors_limits] {}
    if {$ncolors < $ncolors_min || $ncolors > $ncolors_max} {
	error "Number of colors is out of range,\
		should be between $ncolors_min and $ncolors_max"
    }

    # Check color definitions
    ::hs::Verify_color_names [concat $underflow $overflow $colorlist]

    # Looks like everything is fine
    variable Color_scale_data
    set name [hs::Next_color_scale_name]
    set Color_scale_data($name,cspace) "RGBi"
    set Color_scale_data($name,ncolors) $ncolors
    set Color_scale_data($name,start) [lindex $colorlist 0]
    set Color_scale_data($name,end) [lindex $colorlist end]
    set Color_scale_data($name,underflow) $underflow
    set Color_scale_data($name,overflow) $overflow
    set Color_scale_data($name,linear) 0
    set Color_scale_data($name,persistent) 0
    set Color_scale_data($name,pixels) $colorlist

    return $name
}

#########################################################################

proc ::hs::delete_color_scale {name} {
    variable Color_scale_data
    if {![info exists Color_scale_data($scale_tcl_name,ncolors)]} {
	error "Bad color scale tag \"$scale_tcl_name\""
    }
    array unset Color_scale_data "$name,*"
    return
}

#########################################################################

proc ::hs::Next_color_scale_name {} {
    variable Color_scale_counter
    return "hs_colorscale_[incr Color_scale_counter]"
}

#########################################################################

proc ::hs::Num_scale_colors_limits {} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }
    set ncolors_min 2
    # The following settings must be synchronized with the definitions
    # of MAX_COLORS_HIGHCOLOR and MAX_COLORS_8BITCOLOR in file ColorScale.h
    # of the Histo-Scope distribution.
    set depth [winfo depth .]
    if {$depth > 8} {
	set ncolors_max 2000
    } elseif {$depth == 8} {
	set ncolors_max 50
    } else {
	error "Not enough colors to create a color scale"
    }
    list $ncolors_min $ncolors_max
}

#########################################################################

proc ::hs::show_histogram {id args} {
    # Check histogram type
    set type [hs::type $id]
    switch $type {
	HS_1D_HISTOGRAM {
	    set heading "Histogram"
	}
	HS_2D_HISTOGRAM {
	    set heading "2DHistogram"
	}
	HS_NONE {
	    error "Item with id $id doesn't exist"
	}
	default {
	    error "Item with id $id is not a histogram"
	}
    }

    # Parse the list of options
    set keys {-title -window -geometry -line -color -fill -fillcolor\
	    -xmin -xmax -ymin -ymax -zmin -zmax -font -ipadx -ipady\
	    -errors -xscale -yscale -zscale -colorscale -mode}
    hs::Parse_option_sequence $args $keys options

    # For 2d histograms, check color scale and mode options
    set config_string ""
    set colorscale_name "default"
    if {[string equal $type "HS_2D_HISTOGRAM"]} {
	if {$options(-mode,count) > 0} {
	    if {[llength $options(-mode,0)] == 1} {
		set heading [hs::Histo_2d_heading [lindex $options(-mode,0) 0]]
	    } elseif {[llength $options(-line,0)] == 0} {
		error "missing value for the \"-mode\" option"
	    } else {
		error "invalid mode \"$options(-mode,0)\""
	    }
	}

	# Check if we should include the color scale definition
	if {[string equal $heading "ColorCellPlot"]} {
	    if {$options(-colorscale,count) > 0} {
		if {[llength $options(-colorscale,0)] == 1} {
		    set colorscale_name [lindex $options(-colorscale,0) 0]
		} elseif {[llength $options(-colorscale,0)] == 0} {
		    error "missing value for the \"-colorscale\" option"
		} else {
		    set colorscale_name $options(-colorscale,0)
		}
		variable Color_scale_data
		if {![info exists Color_scale_data($colorscale_name,ncolors)]} {
		    error "bad color scale name \"$options(-colorscale,0)\""
		}
		if {!$Color_scale_data($colorscale_name,persistent)} {
		    set tmpname [hs::Next_color_scale_name]
		    hs::Append_color_scale_def config_string \
			    $colorscale_name $tmpname
		    set colorscale_name $tmpname
		}
	    }
	}
    }

    # Start constructing the plot configuration string
    append config_string "${heading}\n\
	    Category [hs::category $id]\n\
	    UID [hs::uid $id]\n"

    # Color scale
    if {[string equal $heading "ColorCellPlot"]} {
	append config_string " ColorScaleName $colorscale_name\n"
	if {$options(-zmin,count) == 0 && $options(-zmax,count) == 0} {
	    append config_string " DynamicColor\n"
	}
    }

    # Show errors?
    set show_errors 0
    if {$options(-errors,count) > 0} {
	if {[llength $options(-errors,0)] == 1} {
	    set value [lindex $options(-errors,0) 0]
	    if {[catch {
		if {$value} {
		    set show_errors 1
		} else {
		    set show_errors 0
		}
	    }]} {
		error "expected a boolean value for\
			the \"-errors\" option, got \"$value\""
	    }
	} elseif {[llength $options(-errors,0)] == 0} {
	    error "missing value for the \"-errors\" option"
	} else {
	    error "expected a boolean value for the \"-errors\"\
		    option, got \"$options(-errors,0)\""
	}
    }
    if {$show_errors} {
	if {[hs::hist_error_status $id] > 0} {
	    append config_string "\
		    ShowErrorData\n"
	}
    }

    # Check if we have the line style
    if {$options(-line,count) > 0} {
	if {[llength $options(-line,0)] == 1} {
	    set linestyle [lindex $options(-line,0) 0]
	    if {![string is integer $linestyle]} {
		error "invalid line style \"$linestyle\""
	    } elseif {$linestyle < 0 || $linestyle > 17} {
		error "line style $linestyle is out of range"
	    } else {
		append config_string "\
			LineStyle1 ${linestyle}\n"
	    }
	} elseif {[llength $options(-line,0)] == 0} {
	    error "missing value for the \"-line\" option"
	} else {
	    error "invalid line style \"$options(-line,0)\""
	}
    }
    
    # Check if we have the line color
    if {$options(-color,count) > 0} {
	if {[llength $options(-color,0)] == 1} {
	    append config_string "\
		    LineColor1 $options(-color,0)\n"
	} elseif {[llength $options(-color,0)] == 0} {
	    error "missing value for the \"-color\" option"
	} else {
	    error "invalid line color \"$options(-color,0)\""
	}
    }
    
    # Check if we have the fill style
    if {$options(-fill,count) > 0} {
	if {[llength $options(-fill,0)] == 1} {
	    variable Histogram_fill_styles
	    if {[lsearch -exact $Histogram_fill_styles $options(-fill,0)] >= 0} {
		set fillstyle $options(-fill,0)
	    } elseif {[string is integer $options(-fill,0)]} {
		set fillnumber $options(-fill,0)
		if {$fillnumber < 0 || $fillnumber >= [llength $Histogram_fill_styles]} {
		    error "fill style $fillnumber is out of range"
		} else {
		    set fillstyle [lindex $Histogram_fill_styles $fillnumber]
		}
	    } else {
		error "invalid fill style \"$options(-fill,0)\""
	    }
	    append config_string "\
		    FillStyle1 ${fillstyle}\n"
	} elseif {[llength $options(-fill,0)] == 0} {
	    error "missing value for the \"-fill\" option"
	} else {
	    error "invalid fill style \"$options(-fill,0)\""
	}
	# Check if we have the fill color. Use the line color
        # as the fill color if the fill color is not specified.
	if {$options(-fillcolor,count) > 0} {
	    if {[llength $options(-fillcolor,0)] > 0} {
		append config_string "\
			FillColor1 $options(-fillcolor,0)\n"
	    } else {
		error "missing value for the \"-fillcolor\" option"
	    }
	} elseif {$options(-color,count) > 0} {
	    append config_string "\
		    FillColor1 $options(-color,0)\n"
	}
    }

    # Check if we have the font
    if {$options(-font,count) > 0} {
	set len [llength $options(-font,0)]
	if {$len == 0} {
	    error "missing value for the \"-font\" option"
	} elseif {$len == 1} {
	    set fontname [lindex $options(-font,0) 0]
	} else {
	    set fontname $options(-font,0)
	}
	if {![string equal $fontname ""]} {
	    set xfont [hs::Generate_xlfd $fontname]
	    foreach {psfont psfontsize} [hs::Postscript_font $fontname] {}
	    append config_string "\
		    Font $xfont\n\
		    PSFont $psfont\n\
		    PSFontSize $psfontsize\n"
	}
    }

    # Check if we have the geometry
    if {$options(-geometry,count) > 0} {
	if {[llength $options(-geometry,0)] == 1} {
	    set geometry [hs::Parse_geometry_option $options(-geometry,0)]
	} elseif {[llength $options(-geometry,0)] == 0} {
	    error "missing value for the \"-geometry\" option"
	} else {
	    error "invalid window geometry \"$options(-geometry,0)\""
	}
    } else {
	set geometry "400x300+0+0"
    }
    append config_string "\
	    WindowGeometry ${geometry}\n"

    # Check if we have the window title
    if {$options(-title,count) > 0} {
	if {[llength $options(-title,0)] == 1} {
	    set title [lindex $options(-title,0) 0]
	} else {
	    set title $options(-title,0)
	}
    } else {
	set title [hs::title $id]
    }
    set title [string trimleft $title]
    if {![string equal "" $title]} {
	append config_string "\
		WindowTitle ${title}\n"
    }

    # Check if we have the window name
    set winname ""
    if {$options(-window,count) > 0} {
	if {[llength $options(-window,0)] == 1} {
	    set winname [lindex $options(-window,0) 0]
	} elseif {[llength $options(-window,0)] == 0} {
	    error "missing value for the \"-window\" option"
	} else {
	    set winname $options(-window,0)
	}
	set winname [string trimleft $winname]
	hs::Check_window_name $winname
	append config_string "\
		WindowName ${winname}\n"
    }

    # Check the axis limits
    foreach {switchname configname} {
	-xmin XMinLimit
	-xmax XMaxLimit
	-ymin YMinLimit
	-ymax YMaxLimit
	-zmin ZMinLimit
	-zmax ZMaxLimit
    } {
	if {$options($switchname,count) > 0} {
	    if {[llength $options($switchname,0)] == 1} {
		if {![string equal $options($switchname,0) "{}"]} {
		    if {[string is double $options($switchname,0)]} {
			set $configname $options($switchname,0)
			append config_string "\
				$configname $options($switchname,0)\n"
		    } else {
			error "invalid $switchname option value \"$options($switchname,0)\""
		    }
		}
	    } elseif {[llength $options($switchname,0)] == 0} {
		error "missing value for the \"$switchname\" option"
	    } else {
		error "invalid $switchname option value \"$options($switchname,0)\""
	    }
	}
    }
    if {[info exists XMinLimit] && [info exists XMaxLimit]} {
	if {$XMinLimit >= $XMaxLimit} {
	    error "Bad x axis limits: min value must be less than max value"
	}
    }
    if {[info exists YMinLimit] && [info exists YMaxLimit]} {
	if {$YMinLimit >= $YMaxLimit} {
	    error "Bad y axis limits: min value must be less than max value"
	}
    }
    if {[info exists ZMinLimit] && [info exists ZMaxLimit]} {
	if {$ZMinLimit >= $ZMaxLimit} {
	    error "Bad z axis limits: min value must be less than max value"
	}
    }

    # Check if we have additional margins
    foreach {switchname confignames} {
	-ipadx {AddToLeftMargin AddToRightMargin}
	-ipady {AddToTopMargin AddToBottomMargin}
    } {
	if {$options($switchname,count) > 0} {
	    if {[llength $options($switchname,0)] == 1} {
		set pair [lindex $options($switchname,0) 0]
		if {[llength $pair] != 2} {
		    error "invalid $switchname option value \"$options($switchname,0)\""
		}
		foreach configname $confignames value $pair {
		    if {![string is integer -strict $value]} {
			error "invalid $switchname option value \"$options($switchname,0)\""
		    }
		    append config_string "\
			    $configname $value\n"
		}
	    } elseif {[llength $options($switchname,0)] == 0} {
		error "missing value for the \"$switchname\" option"
	    } else {
		error "invalid $switchname option value \"$options($switchname,0)\""
	    }
	}
    }

    # Check if log scale is requested
    foreach {switchname configname limitname} {
	-xscale LogX XMinLimit
	-yscale LogY YMinLimit
	-zscale LogZ ZMinLimit
    } {
	if {$options($switchname,count) > 0} {
	    if {[llength $options($switchname,0)] == 1} {
		if {[string equal -nocase $options($switchname,0) "log"]} {
		    append config_string "\
			    $configname\n"
		    # The following trick forces the client to calculate
		    # reasonable axis limits if they are not provided
		    if {![info exists $limitname]} {
			append config_string "\
				$limitname 0.0\n"
		    }
		} elseif {[string equal -nocase $options($switchname,0) "linear"] || \
			[string equal -nocase $options($switchname,0) "lin"]} {
		    # Do nothing
		} else {
		    error "invalid $switchname option value \"$options($switchname,0)\""
		}
	    } elseif {[llength $options($switchname,0)] == 0} {
		error "missing value for the \"$switchname\" option"
	    } else {
		error "invalid $switchname option value \"$options($switchname,0)\""
	    }
	}
    }

    # We are almost done here ...
    append config_string "\
	    HideLegend\n"
    hs::Remember_window_name $winname
    hs::load_config_string $config_string
    hs::hs_update
    return
}

#########################################################################

proc ::hs::overlay {name args} {

    hs::Validate_overlay_name $name

    variable Overlay_info
    variable Overlay_count
    variable Ntuple_plot_info

    # Fill out some default settings
    if {![info exists Overlay_count($name)]} {
	set Overlay_count($name) 0
	set Overlay_info($name,style) user
	set Overlay_info($name,hidelegend) 0
	set Overlay_info($name,xmin) ""
	set Overlay_info($name,xmax) ""
	set Overlay_info($name,ymin) ""
	set Overlay_info($name,ymax) ""
	set Overlay_info($name,zmin) ""
	set Overlay_info($name,zmax) ""
	set Overlay_info($name,xscale) "linear"
	set Overlay_info($name,yscale) "linear"
	set Overlay_info($name,zscale) "linear"
	set Overlay_info($name,colorscale) ""
	set Overlay_info($name,xlabel) ""
	set Overlay_info($name,ylabel) ""
	set Overlay_info($name,xfont) ""
	set Overlay_info($name,psfont) ""
	set Overlay_info($name,geometry) "400x300+0+0"
	set Overlay_info($name,ipadx) [list 0 0]
	set Overlay_info($name,ipady) [list 0 0]
    }

    # Parse the list of options
    set keys {-title -window -geometry -style -legend -xlabel -ylabel\
	    -xmin -xmax -ymin -ymax -zmin -zmax -xscale -yscale -zscale\
	    -colorscale -font -ipadx -ipady add getconfig getids show clear}
    hs::Parse_option_sequence $args $keys commands

    if {$commands(getids,count) > 0} {
	if {$commands(getconfig,count) > 0} {
	    error "options \"getconfig\" and \"getids\" are mutually exclusive"
	}
    }

    # Check if we have the geometry
    if {$commands(-geometry,count) > 0} {
	if {[llength $commands(-geometry,0)] == 1} {
	    set Overlay_info($name,geometry) \
		    [hs::Parse_geometry_option $commands(-geometry,0)]
	} elseif {[llength $commands(-geometry,0)] == 0} {
	    error "missing value for the \"-geometry\" option"
	} else {
	    error "invalid window geometry \"$commands(-geometry,0)\""
	}
    }

    # Check if we have the legend option
    if {$commands(-legend,count) > 0} {
	if {[llength $commands(-legend,0)] == 1} {
	    set value [lindex $commands(-legend,0) 0]
	    # Value must be boolean
	    if {[catch {
		if {$value} {
		    set Overlay_info($name,hidelegend) 0
		} else {
		    set Overlay_info($name,hidelegend) 1
		}
	    }]} {
		error "expected a boolean value for\
			the \"-legend\" option, got \"$value\""
	    }
	} elseif {[llength $commands(-legend,0)] == 0} {
	    error "missing value for the \"-legend\" option"
	} else {
	    error "invalid legend mode \"$commands(-legend,0)\""
	}
    }

    # Check if we have the window title
    if {$commands(-title,count) > 0} {
	if {[llength $commands(-title,0)] == 1} {
	    set title [lindex $commands(-title,0) 0]
	} else {
	    set title $commands(-title,0)
	}
	set title [string trimleft $title]
	if {![string equal "" $title]} {
	    set Overlay_info($name,wintitle) $title
	}
    }

    # Check if we have the font
    if {$commands(-font,count) > 0} {
	set len [llength $commands(-font,0)]
	if {$len == 0} {
	    error "missing value for the \"-font\" option"
	} elseif {$len == 1} {
	    set fontname [lindex $commands(-font,0) 0]
	} else {
	    set fontname $commands(-font,0)
	}
	if {[string equal $fontname ""]} {
	    set Overlay_info($name,xfont) ""
	    set Overlay_info($name,psfont) ""
	} else {
	    set Overlay_info($name,xfont) [hs::Generate_xlfd $fontname]
	    set Overlay_info($name,psfont) [hs::Postscript_font $fontname]
	}
    }

    # Check axis labels
    foreach opt {xlabel ylabel} {
	if {$commands(-$opt,count) > 0} {
	    if {[llength $commands(-$opt,0)] == 1} {
		set $opt [lindex $commands(-$opt,0) 0]
	    } else {
		set $opt $commands(-$opt,0)
	    }
	    set $opt [string trimleft [set $opt]]
	    if {![string equal "" [set $opt]]} {
		set Overlay_info($name,$opt) [set $opt]
	    }
	}
    }

    # Check if we have the window name
    if {$commands(-window,count) > 0} {
	if {[llength $commands(-window,0)] == 1} {
	    set winname [lindex $commands(-window,0) 0]
	} elseif {[llength $commands(-window,0)] == 0} {
	    error "missing value for the \"-window\" option"
	} else {
	    set winname $commands(-window,0)
	}
	set winname [string trimleft $winname]
	hs::Check_window_name $winname
	set Overlay_info($name,window) $winname
    }

    # Check if we have the color scale name
    if {$commands(-colorscale,count) > 0} {
	if {[llength $commands(-colorscale,0)] == 1} {
	    set colorscale_name [lindex $commands(-colorscale,0) 0]
	} elseif {[llength $commands(-colorscale,0)] == 0} {
	    error "missing value for the \"-colorscale\" option"
	} else {
	    set colorscale_name $commands(-colorscale,0)
	}
	variable Color_scale_data
	if {![info exists Color_scale_data($colorscale_name,ncolors)]} {
	    error "bad color scale name \"$commands(-colorscale,0)\""
	}
	set Overlay_info($name,colorscale) $colorscale_name
    }

    # Check the additional margins
    foreach switchname {ipadx ipady} {
	if {$commands(-$switchname,count) > 0} {
	    if {[llength $commands(-$switchname,0)] == 1} {
		set pair [lindex $commands(-$switchname,0) 0]
		if {[llength $pair] != 2} {
		    error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		}
		foreach value $pair {
		    if {![string is integer -strict $value]} {
			error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		    }
		}
		set Overlay_info($name,$switchname) $pair
	    } elseif {[llength $commands(-$switchname,0)] == 0} {
		error "missing value for the \"-$switchname\" option"
	    } else {
		error "invalid -$switchname option value \"$commands(-$switchname,0)\""
	    }
	}
    }

    # Check the axis limits and scales
    set limit_attributes {
	xmin XMinLimit
	xmax XMaxLimit
	ymin YMinLimit
	ymax YMaxLimit
	zmin ZMinLimit
	zmax ZMaxLimit
    }
    set scale_attributes {
	xscale LogX xmin
	yscale LogY ymin
	zscale LogZ zmin
    }
    foreach {switchname configname} $limit_attributes {
	set old_axis_limits($switchname) $Overlay_info($name,$switchname)
    }
    foreach {switchname configname limitname} $scale_attributes {
	set old_scale_type($switchname) $Overlay_info($name,$switchname)
    }
    global ::errorInfo
    if {[catch {
	foreach {switchname configname} $limit_attributes {
	    if {$commands(-$switchname,count) > 0} {
		if {[llength $commands(-$switchname,0)] == 1} {
		    if {[string equal $commands(-$switchname,0) "{}"]} {
			set Overlay_info($name,$switchname) ""
		    } elseif {[string is double $commands(-$switchname,0)]} {
			set Overlay_info($name,$switchname) $commands(-$switchname,0)
		    } else {
			error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		    }
		} elseif {[llength $commands(-$switchname,0)] == 0} {
		    error "missing value for the \"-$switchname\" option"
		} else {
		    error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		}
	    }
	}
	if {![string equal $Overlay_info($name,xmin) ""] && \
		![string equal $Overlay_info($name,xmax) ""]} {
	    if {$Overlay_info($name,xmin) >= $Overlay_info($name,xmax)} {
		error "Bad x axis limits: min value must be less than max value"
	    }
	}
	if {![string equal $Overlay_info($name,ymin) ""] && \
		![string equal $Overlay_info($name,ymax) ""]} {
	    if {$Overlay_info($name,ymin) >= $Overlay_info($name,ymax)} {
		error "Bad y axis limits: min value must be less than max value"
	    }
	}
	if {![string equal $Overlay_info($name,zmin) ""] && \
		![string equal $Overlay_info($name,zmax) ""]} {
	    if {$Overlay_info($name,zmin) >= $Overlay_info($name,zmax)} {
		error "Bad z axis limits: min value must be less than max value"
	    }
	}
	foreach {switchname configname limitname} $scale_attributes {
	    if {$commands(-$switchname,count) > 0} {
		if {[llength $commands(-$switchname,0)] == 1} {
		    if {[string equal -nocase $commands(-$switchname,0) "log"]} {
			set Overlay_info($name,$switchname) "log"
		    } elseif {[string equal -nocase $commands(-$switchname,0) "linear"] || \
			    [string equal -nocase $commands(-$switchname,0) "lin"]} {
			set Overlay_info($name,$switchname) "linear"
		    } else {
			error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		    }
		} elseif {[llength $commands(-$switchname,0)] == 0} {
		    error "missing value for the \"-$switchname\" option"
		} else {
		    error "invalid -$switchname option value \"$commands(-$switchname,0)\""
		}
	    }
	}
    } limstatus]} {
	set savedInfo $::errorInfo
	foreach {switchname configname} $limit_attributes {
	    set Overlay_info($name,$switchname) $old_axis_limits($switchname)
	}
	foreach {switchname configname limitname} $scale_attributes {
	    set Overlay_info($name,$switchname) $old_scale_type($switchname)
	}
	error $limstatus $savedInfo
    }

    # Check if we have the style argument
    set known_styles {user plain color1 errorcontour\
	    rainbow quilt histo histocolor}
    if {$commands(-style,count) > 0} {
	if {[llength $commands(-style,0)] == 1} {
	    if {[lsearch -exact $known_styles $commands(-style,0)] >= 0} {
		set Overlay_info($name,style) $commands(-style,0)
	    } else {
		error "invalid overlay style \"$commands(-style,0)\""
	    }
	} elseif {[llength $commands(-style,0)] == 0} {
	    error "missing value for the \"-style\" option"
	} else {
	    error "invalid overlay style \"$commands(-style,0)\""
	}
    }

    # Check "add" commands
    for {set i 0} {$i < $commands(add,count)} {incr i} {
	if {[llength $commands(add,$i)] == 0} {
	    error "missing arguments for the \"add\" option"
	} else {
	    # Remember the number of histos in the overlay. We will
            # need to restore it if something fails.
	    set old_overlay_count $Overlay_count($name)
	    global ::errorInfo
	    if {[catch {
		foreach item [lindex $commands(add,$i) 0] {
		    if {[string is integer $item]} {
			# This must be a Histo-Scope item id
			switch [hs::type $item] {
			    HS_1D_HISTOGRAM -
			    HS_2D_HISTOGRAM {
				eval hs::Overlay_histogram [list $name]\
					$item [lrange $commands(add,$i) 1 end]
			    }
			    HS_NTUPLE {
				eval hs::Overlay_ntuple [list $name]\
					$item [lrange $commands(add,$i) 1 end]
			    }
			    HS_NONE {
				error "Histo-Scope item with id $item does not exist"
			    }
			    default {
				error "Can't overlay Histo-Scope item\
					of type [hs::type $item] (id is $item)"
			    }
			}
		    } elseif {[info exists Overlay_count($item)]} {
			# This is another overlay. Copy all items from that overlay.
			if {[llength $commands(add,$i)] > 1} {
			    error "invalid option \"[lindex $commands(add,$i) 1]\""
			}
			hs::Overlay_overlay $name $item
		    } else {
			error "invalid item id \"$item\""
		    }
		}
	    } errmess]} {
		set savedInfo $::errorInfo
		set Overlay_count($name) $old_overlay_count
		error $errmess $savedInfo
	    }
	}
    }

    # Construct the configuration string if we have "getconfig" or "show"
    if {$commands(getconfig,count) > 0 || $commands(show,count) > 0} {
	set plotcount 0
	set all_plain_histos 1
	set need_color_scale 0
	for {set i 0} {$i < $Overlay_count($name)} {incr i} {
	    set id $Overlay_info($name,$i,id)
	    if {[string equal $Overlay_info($name,$i,header) "NTupleItem"]} {
		set ntuple_id $Ntuple_plot_info($id,id)
		if {![string equal [hs::type $ntuple_id] "HS_NONE"]} {
		    incr plotcount
		    set all_plain_histos 0
		    set plottype $Ntuple_plot_info($id,plottype)
		    if {[string equal $plottype "tartan"] || \
			    [string equal $plottype "cscat2"]} {
			set need_color_scale 1
		    }
		}
	    } elseif {![string equal [hs::type $id] "HS_NONE"]} {
		if {[string equal $Overlay_info($name,$i,header) "ColorCellPlot"]} {
		    set need_color_scale 1
		}
		incr plotcount
	    }
	}
	if {$plotcount == 0} {
	    error "overlay \"$name\" contains no plots"
	}
	set last_plot_number [expr {$plotcount - 1}]

	# Modify the overlay info according to the requested style
	hs::Modify_overlay_style Overlay_tmp $name $Overlay_info($name,style)

	# Check if we need to prepend the color scale definition
	set config_string ""
	set colorscale_name $Overlay_tmp($name,colorscale)
	if {$need_color_scale} {
	    if {![string equal $colorscale_name ""]} {
		variable Color_scale_data
		if {!$Color_scale_data($colorscale_name,persistent)} {
		    set tmpname [hs::Next_color_scale_name]
		    hs::Append_color_scale_def config_string \
			    $colorscale_name $tmpname
		    set colorscale_name $tmpname
		}
	    }
	}

	# Add all plots to the configuration string
	set numplot 0
	set dynamic_color [expr [string equal $Overlay_tmp($name,zmin) ""] && \
		[string equal $Overlay_tmp($name,zmax) ""]]
	variable Overlay_id
	for {set i 0} {$i < $Overlay_count($name)} {incr i} {
	    set id $Overlay_tmp($name,$i,id)
	    if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		set ntuple_id $Ntuple_plot_info($id,id)
		if {[string equal [hs::type $ntuple_id] "HS_NONE"]} {
		    continue
		}
		append config_string [hs::Ntuple_plot_getconfig \
			$id $colorscale_name $dynamic_color]
	    } else {
		if {[string equal [hs::type $id] "HS_NONE"]} {
		    continue
		}
		set heading $Overlay_tmp($name,$i,header)
		append config_string "$heading\n\
			Category [hs::category $id]\n\
			UID [hs::uid $id]\n"
		if {[string equal $heading "ColorCellPlot"]} {
		    if {![string equal $colorscale_name ""]} {
			append config_string "\
				ColorScaleName $colorscale_name\n"
		    }
		    if {$dynamic_color} {
			append config_string "\
				DynamicColor\n"
		    }
		}
		if {$Overlay_tmp($name,$i,errors)} {
		    if {[hs::hist_error_status $id] > 0} {
			append config_string "\
				ShowErrorData\n"
		    }
		}
		append config_string "\
			LineStyle1 $Overlay_tmp($name,$i,line)\n\
			LineColor1 $Overlay_tmp($name,$i,color)\n\
			FillStyle1 $Overlay_tmp($name,$i,fill)\n\
			FillColor1 $Overlay_tmp($name,$i,fillcolor)\n"
	    }
	    if {$numplot == 0} {
		if {![string equal $Overlay_tmp($name,xfont) ""]} {
		    append config_string "\
			    Font $Overlay_tmp($name,xfont)\n\
			    PSFont [lindex $Overlay_tmp($name,psfont) 0]\n\
			    PSFontSize [lindex $Overlay_tmp($name,psfont) 1]\n"
		}
		append config_string "\
			WindowGeometry $Overlay_tmp($name,geometry)\n"
		if {[info exists Overlay_tmp($name,wintitle)]} {
		    append config_string "\
			    WindowTitle $Overlay_tmp($name,wintitle)\n"
		}
		if {[info exists Overlay_tmp($name,window)]} {
		    set windo_name $Overlay_tmp($name,window)
		    append config_string "\
			    WindowName $windo_name\n"
		} else {
		    set windo_name ""
		}
		foreach {switchname configname} $limit_attributes {
		    if {![string equal $Overlay_tmp($name,$switchname) ""]} {
			append config_string "\
				$configname $Overlay_tmp($name,$switchname)\n"
		    }
		}
		foreach {switchname padnames} {
		    ipadx {AddToLeftMargin AddToRightMargin}
		    ipady {AddToTopMargin AddToBottomMargin}
		} {
		    foreach value $Overlay_tmp($name,$switchname) configname $padnames {
			if {$value} {
			    append config_string "\
				    $configname $value\n"
			}
		    }
		}
		if {$plotcount > 1 ||\
			![string equal $Overlay_tmp($name,xlabel) ""] ||\
			![string equal $Overlay_tmp($name,ylabel) ""]} {
		    incr Overlay_id
		    append config_string "\
			    OverlayID $Overlay_id\n"
		    if {![string equal $Overlay_tmp($name,xlabel) ""]} {
			append config_string "\
				XLabel $Overlay_tmp($name,xlabel)\n"
		    }
		    if {![string equal $Overlay_tmp($name,ylabel) ""]} {
			append config_string "\
				YLabel $Overlay_tmp($name,ylabel)\n"
		    }
		    if {$Overlay_tmp($name,hidelegend)} {
			append config_string "\
				HideLegend\n"
		    }
		} else {
		    # Hide the legend if we have a histogram; live it
		    # up to the user if we have an ntuple plot.
		    if {$all_plain_histos || $Overlay_tmp($name,hidelegend)} {
			append config_string "\
				HideLegend\n"
		    }
		}
	    }
	    if {$numplot == $last_plot_number} {
		array unset tmp
		array set tmp $limit_attributes
		foreach {switchname configname limitname} $scale_attributes {
		    if {[string equal $Overlay_tmp($name,$switchname) "log"]} {
			append config_string "\
				$configname\n"
			if {[string equal $Overlay_tmp($name,$limitname) ""]} {
			    append config_string "\
				    $tmp($limitname) 0.0\n"
			}
		    }
		}
	    }
	    if {$numplot > 0} {
		append config_string "\
			InOverlay $Overlay_id\n"
		if {$Overlay_tmp($name,hidelegend)} {
		    append config_string "\
			    HideLegend\n"
		}
	    }
	    incr numplot
	}
	if {$numplot == 0} {
	    error "overlay \"$name\" contains no plots"
	}
    }

    # Send out the configuration string
    if {$commands(show,count) > 0} {
	hs::Remember_window_name $windo_name
	hs::load_config_string $config_string
	hs::hs_update
    }

    # Check if we have to clean up
    if {$commands(clear,count) > 0} {
	hs::Overlay_destroy $name
    }

    # What is our return value?
    if {$commands(getconfig,count) > 0} {
	return $config_string
    } elseif {$commands(getids,count) > 0} {
	if {![info exists Overlay_count($name)]} {
	    return {}
	} else {
	    set idlist {}
	    for {set index 0} {$index < $Overlay_count($name)} {incr index} {
		if {[string equal $Overlay_info($name,$index,header) "NTupleItem"]} {
		    set plotname $Overlay_info($name,$index,id)
		    if {![info exists Ntuple_plot_info($plotname,id)]} continue
		    set hs_id $Ntuple_plot_info($plotname,id)
		} else {
		    set hs_id $Overlay_info($name,$index,id)
		}
		if {![string equal [hs::type $hs_id] "HS_NONE"]} {
		    lappend idlist $hs_id
		}
	    }
	    return $idlist
	}
    }
    return
}

############################################################################

proc ::hs::Overlay_histogram {name id args} {
    # Check input arguments
    variable Overlay_count
    if {![info exists Overlay_count($name)]} {
	error "\"$name\" is not a valid overlay name"
    }
    if {![string is integer $id]} {
	error "$id is not a valid Histo-Scope id"
    }
    set hs_type [hs::type $id]
    if {![string equal $hs_type "HS_1D_HISTOGRAM"] && \
	    ![string equal $hs_type "HS_2D_HISTOGRAM"]} {
	if {[string equal $hs_type "HS_NONE"]} {
	    error "Histo-Scope item with id $id does not exist"
	} else {
	    error "Histo-Scope item with id $id is not a histogram"
	}
    }
    # Check if an item with this id already exists in this overlay
    variable Overlay_info
    set newhisto 1
    for {set index 0} {$index < $Overlay_count($name)} {incr index} {
	if {$Overlay_info($name,$index,id) == $id} {
	    set newhisto 0
	    break
	}
    }
    if {$newhisto} {
	# Check if the types are compatible
	if {$index > 0} {
	    if {![hs::Overlay_compatible $Overlay_info($name,0,type) $hs_type]} {
		error "Can't overlay items with types\
			$Overlay_info($name,0,type) and $hs_type"
	    }
	}
	# Fill some type-dependent settings
	switch $hs_type {
	    HS_1D_HISTOGRAM {
		set Overlay_info($name,$index,header) "Histogram"
	    }
	    HS_2D_HISTOGRAM {
		set Overlay_info($name,$index,header) "2DHistogram"
	    }
	    default {
		# This should never happen
		error "incomplete item type switch statement in\
			[lindex [info level 0] 0]. This is a bug. Please report."
	    }
	}
	set Overlay_info($name,$index,id) $id
	set Overlay_info($name,$index,type) $hs_type
	set Overlay_info($name,$index,owner) 0
	incr Overlay_count($name)
    }

    # For each histogram we need to remember
    # the following items: owner, line, color, fill, fillcolor, errors.
    # The -mode option may affect the header.
    set plotkeys {-owner -line -color -fill -fillcolor -errors -mode}
    hs::Parse_option_sequence $args $plotkeys options

    # Check if the plotting mode is specified
    if {[string equal $hs_type "HS_2D_HISTOGRAM"]} {
	if {$options(-mode,count) > 0} {
	    if {[llength $options(-mode,0)] == 1} {
		set Overlay_info($name,$index,header) \
			[hs::Histo_2d_heading [lindex $options(-mode,0) 0]]
	    } elseif {[llength $options(-line,0)] == 0} {
		error "missing value for the \"-mode\" option"
	    } else {
		error "invalid mode \"$options(-mode,0)\""
	    }
	}
    }

    # Check if we own the histogram
    set is_owner -1
    if {$options(-owner,count) > 0} {
	if {[llength $options(-owner,0)] == 1} {
	    set value [lindex $options(-owner,0) 0]
	    if {[catch {
		if {$value} {
		    set is_owner 1
		} else {
		    set is_owner 0
		}
	    }]} {
		error "expected a boolean value for\
			the \"-owner\" option, got \"$value\""
	    }
	} elseif {[llength $options(-owner,0)] == 0} {
	    error "missing value for the \"-owner\" option"
	} else {
	    error "expected a boolean value for the \"-owner\"\
		    option, got \"$options(-owner,0)\""
	}
    } 

    # Check if errors are requested
    if {$options(-errors,count) > 0} {
	if {[llength $options(-errors,0)] == 1} {
	    set value [lindex $options(-errors,0) 0]
	    if {[catch {
		if {$value} {
		    set Overlay_info($name,$index,errors) 1
		} else {
		    set Overlay_info($name,$index,errors) 0
		}
	    }]} {
		error "expected a boolean value for\
			the \"-errors\" option, got \"$value\""
	    }
	} elseif {[llength $options(-errors,0)] == 0} {
	    error "missing value for the \"-errors\" option"
	} else {
	    error "expected a boolean value for the \"-errors\"\
		    option, got \"$options(-errors,0)\""
	}
    } else {
	if {![info exists Overlay_info($name,$index,errors)]} {
	    set Overlay_info($name,$index,errors) 0
	}
    }

    # Check if we have the line style
    if {$options(-line,count) > 0} {
	if {[llength $options(-line,0)] == 1} {
	    if {[scan $options(-line,0) "%d" linestyle] != 1} {
		error "invalid line style \"$options(-line,0)\""
	    } elseif {$linestyle < 0 || $linestyle > 17} {
		error "line style $linestyle is out of range"
	    } else {
		set Overlay_info($name,$index,line)\
			$linestyle
	    }
	} elseif {[llength $options(-line,0)] == 0} {
	    error "missing value for the \"-line\" option"
	} else {
	    error "invalid line style \"$options(-line,0)\""
	}
    } else {
	if {![info exists Overlay_info($name,$index,line)]} {
	    set Overlay_info($name,$index,line) 1
	}
    }
    
    # Check if we have the line color
    if {$options(-color,count) > 0} {
	if {[llength $options(-color,0)] == 1} {
	    set Overlay_info($name,$index,color)\
		    $options(-color,0)
	} elseif {[llength $options(-color,0)] == 0} {
	    error "missing value for the \"-color\" option"
	} else {
	    error "invalid line color \"$options(-color,0)\""
	}
    } else {
	if {![info exists Overlay_info($name,$index,color)]} {
	    set Overlay_info($name,$index,color) black
	}
    }
    
    # Check if we have the fill style
    if {$options(-fill,count) > 0} {
	if {[llength $options(-fill,0)] == 1} {
	    variable Histogram_fill_styles	    
	    if {[lsearch -exact $Histogram_fill_styles \
		    $options(-fill,0)] >= 0} {
		set Overlay_info($name,$index,fill)\
			$options(-fill,0)
	    } elseif {[scan $options(-fill,0) "%d" fillnumber] == 1} {
		if {$fillnumber < 0 || $fillnumber >= \
			[llength $Histogram_fill_styles]} {
		    error "fill style $fillnumber is out of range"
		} else {
		    set Overlay_info($name,$index,fill)\
			    [lindex $Histogram_fill_styles $fillnumber]
		}
	    } else {
		error "invalid fill style \"$options(-fill,0)\""
	    }
	} elseif {[llength $options(-fill,0)] == 0} {
	    error "missing value for the \"-fill\" option"
	} else {
	    error "invalid fill style \"$options(-fill,0)\""
	}
    } else {
	if {![info exists Overlay_info($name,$index,fill)]} {
	    set Overlay_info($name,$index,fill) none
	}
    }
    
    # Check if we have the fill color. Use the line color
    # as the fill color if the fill color is not specified.
    if {$options(-fillcolor,count) > 0} {
	if {[llength $options(-fillcolor,0)] == 1} {
	    set Overlay_info($name,$index,fillcolor)\
		    $options(-fillcolor,0)
	} elseif {[llength $options(-fillcolor,0)] == 0} {
	    error "missing value for the \"-fillcolor\" option"
	} else {
	    error "invalid fill color \"$options(-color,0)\""
	}
    } else {
	if {![info exists Overlay_info($name,$index,fillcolor)]} {
	    set Overlay_info($name,$index,fillcolor)\
		    $Overlay_info($name,$index,color)
	}
    }

    # Check if the ownership property has changed
    if {$is_owner >= 0} {
	if {$is_owner && !$Overlay_info($name,$index,owner)} {
	    hs::Overlay_incr_refcount $Overlay_info($name,$index,id)
	} elseif {!$is_owner && $Overlay_info($name,$index,owner)} {
	    # Decrement the reference counter without deleting the histogram
	    hs::Overlay_disown_item $Overlay_info($name,$index,id)
	}
	set Overlay_info($name,$index,owner) $is_owner
    }

    return
}

############################################################################

proc ::hs::Overlay_ntuple {name args} {
    # args should look like this:
    # ntuple_id plot_type -y varname ...
    # Check input arguments
    variable Overlay_info
    variable Overlay_count
    if {![info exists Overlay_count($name)]} {
	error "\"$name\" is not a valid overlay name"
    }
    set id [lindex $args 0]
    if {![string is integer $id]} {
	error "$id is not a valid Histo-Scope id"
    }
    set hs_type [hs::type $id]
    if {![string equal $hs_type "HS_NTUPLE"]} {
	if {[string equal $hs_type "HS_NONE"]} {
	    error "Histo-Scope item with id $id does not exist"
	} else {
	    error "Histo-Scope item with id $id is not an Ntuple"
	}
    }
    # Check the ntuple plot type
    if {[llength $args] < 4} {
	error "not enough arguments to completely specify\
		an ntuple plot in overlay $name (ntuple id $id)"
    }
    set plottype [lindex $args 1]
    # Check the correctness of the plot type.
    # Type attributes in the big list below: type name, list of
    # names which will be mapped (in case-insensitive manner) into
    # the type name, the smallest allowed number of variables in
    # the ntuple, required switches, optional switches.
    set type_found 0
    foreach {type namelist require_nvars required_options optional_switches} {
	ts     {ts}        1 {-y}    {-owner -lang -snapshot -filter -line -color -marker -markersize}
	tse    {tse}       1 {-y}    {-owner -lang -snapshot -filter -ey -ey+ -ey- -line -color -marker -markersize}
	xy     {xy}        2 {-x -y} {-owner -lang -snapshot -filter -line -color -marker -markersize}
	xye    {xye}       2 {-x -y} {-owner -lang -snapshot -filter -ex -ey -ex+ -ex- -ey+ -ey- -line -color -marker -markersize}
	xys    {xys}       2 {-x -y} {-owner -lang -snapshot -filter -line -color -marker -markersize}
	xyse   {xyse xyes} 2 {-x -y} {-owner -lang -snapshot -filter -ey -ey+ -ey- -line -color -marker -markersize}
	scat2  {scat2 scat2d} 2 {-x -y} {-owner -lang -snapshot -filter -sliders}
	scat3  {scat3 scat3d} 3 {-x -y -z}  {-owner -lang -snapshot -filter -sliders}
	h1     {h1 h1d}    1 {-x -nbins} {-owner -lang -snapshot -filter -line -color -fill -fillcolor -sliders}
	h1a    {h1a h1da}  1 {-x}    {-owner -lang -snapshot -filter -line -color -fill -fillcolor -binlimit -sliders}
	h2     {h2 h2d}    2 {-x -y} {-owner -lang -snapshot -filter -sliders -nxbins -nybins}
	h2a    {h2a h2da}  2 {-x -y} {-owner -lang -snapshot -filter -sliders}
	cell   {cell}      2 {-x -y} {-owner -lang -snapshot -filter -sliders -nxbins -nybins}
	cscat2 {cscat2 cscat2d}   3 {-x -y -z} {-owner -lang -snapshot -filter -sliders}
        tartan {tartan colorcell} 3 {-x -y} {-owner -lang -snapshot -filter -sliders -nxbins -nybins}
    } {
	foreach typename $namelist {
	    if {[string equal -nocase $typename $plottype]} {
		set type_found 1
		break
	    }
	}
	if {$type_found} {
	    set plottype $type
	    set plot_options [concat $required_options $optional_switches]
	    break
	}
    }
    if {!$type_found} {
	error "Invalid plot type \"$plottype\".\
		Valid plot types are ts, tse, xy, xye, xys,\
		xyse, scat2, scat3, h1, h1a, h2, h2a, cell, cscat2, and tartan."
    }
    if {$Overlay_count($name) > 0} {
	if {![hs::Overlay_compatible $plottype $Overlay_info($name,0,type)]} {
	    error "Can't add $plottype ntuple plot\
		    (ntuple id is $id) to overlay ${name}:\nplot\
		    types $plottype and $Overlay_info($name,0,type)\
		    are not compatible"
	}
    }
    if {[hs::num_variables $id] < $require_nvars} {
	error "Ntuple $id does not have enough\
		variables for [lindex $args 1] plot"
    }

    # Process the options
    set pairlist [lrange $args 2 end]
    if {[expr [llength $pairlist] % 2] != 0} {
	error "wrong # of arguments"
    }

    # Check the -lang option first -- it will be needed
    # to interpret other options
    set index [lsearch -exact $pairlist -lang]
    if {$index >= 0} {
	set language [lindex $pairlist [expr {$index+1}]]
	set allowed_languages {none tcl c C Tcl}
	if {[lsearch -exact $allowed_languages $language] < 0} {
	    error "invalid $plottype ntuple plot language \"$language\",\
		    allowed languages are \"tcl\", \"C\", and \"none\""
	}
    } else {
	set language none
    }

    # Go over all other options
    set goodlist [list sliders {}]
    set option_list {}
    set variable_list {}
    foreach {option value} $pairlist {
	if {[lsearch -exact $plot_options $option] < 0} {
	    error "Invalid $plottype ntuple plot option \"$option\".\
		    Available options are [join [lsort $plot_options] {, }]."
	}
	lappend option_list $option
	regsub "^-" $option "" option
	switch -- $option {
	    snapshot -
	    owner {
		# Must be a boolean
		if {[catch {
		    if {$value} {
			set value 1
		    } else {
			set value 0
		    }
		}]} {
		    error "invalid $plottype ntuple plot $option value\
			    \"$value\", should be a boolean"
		}
	    }
	    nxbins -
	    nybins -
	    nbins -
	    binlimit {
		# Must be a positive integer
		if {![string is integer $value]} {
		    error "invalid $plottype ntuple plot $option value\
			    \"$value\", should be a positive integer"
		} elseif {$value <= 0} {
		    error "invalid $plottype ntuple plot $option value\
			    \"$value\", should be a positive integer"
		}
	    }
	    x -
	    y -
	    z -
	    ex -
	    ey -
	    ex+ -
	    ex- -
	    ey+ -
	    ey- {
		switch $language {
		    none {
			# This must be a valid ntuple variable name
			if {[hs::variable_index $id $value] < 0} {
			    error "\"$value\" is not a variable of the ntuple with id $id"
			}
		    }
		    Tcl -
		    tcl {
			# This must be a complete tcl expression
			if {[info complete $value] == 0} {
			    error "$option expression is not complete"
			}
		    }
		    C -
		    c {
			# All ntuple variables used in the expression
			# must have names compatible with C identifiers
			set offending_variable [hs::Is_ntuple_c_compatible $id [list $value]]
			if {![string equal $offending_variable ""]} {
			    error "ntuple $id column named \"$offending_variable\" can not be used in C code"
			}
		    }
		    default {
			error "incomplete language type switch statement (1) in\
				[lindex [info level 0] 0]. This is a bug. Please report."
		    }
		}
		lappend variable_list $value
	    }
	    filter {
		# This must be a valid expression in the given language
		switch $language {
		    none {
			if {![string equal $value ""]} {
			    error "please specify the language for\
				    the filter expression using the \"-lang\" option"
			}
		    }
		    Tcl -
		    tcl {
			if {[info complete $value] == 0} {
			    error "$option expression is not complete"
			}
		    }
		    C -
		    c {
			set offending_variable [hs::Is_ntuple_c_compatible $id [list $value]]
			if {![string equal $offending_variable ""]} {
			    error "ntuple $id column named \"$offending_variable\" can not be used in C code"
			}
		    }
		    default {
			error "incomplete language type switch statement (2) in\
				[lindex [info level 0] 0]. This is a bug. Please report."
		    }
		}
	    }
	    marker {
		set value [hs::Validate_marker_style $value]
	    }
	    markersize {
		set value [hs::Validate_marker_size $value]
	    }
	    line {
		if {[string equal -length 2 $plottype "h1"]} {
		    set nlines 18
		} else {
		    set nlines 13
		}
		# xy plot line styles are defined in the
		# Histo-Scope header file XY.h
		if {![string is integer $value]} {
		    error "invalid line style \"$value\""
		} elseif {$value < 0 || $value >= $nlines} {
		    error "line style $value is out of range"
		}
	    }
	    fill {
		variable Histogram_fill_styles
		if {[lsearch -exact $Histogram_fill_styles $value] < 0} {
		    if {[string is integer $value]} {
			if {$value < 0 || $value >= \
				[llength $Histogram_fill_styles]} {
			    error "fill style $value is out of range"
			} else {
			    set value [lindex $Histogram_fill_styles $value]
			}
		    } else {
			error "invalid fill style \"$value\""
		    }
		}
	    }
            sliders {
                # Every member must be a valid ntuple variable
                foreach varname $value {
                    if {[hs::variable_index $id $varname] < 0} {
                        error "\"$varname\" is not a variable of the ntuple with id $id"
                    }
                }
            }
	    default {
		# Pass the option as it is
	    }
	}
	if {[string equal $option "ex"]} {
	    lappend goodlist ex+ $value
	    lappend goodlist ex- $value
	} elseif {[string equal $option "ey"]} {
	    lappend goodlist ey+ $value
	    lappend goodlist ey- $value
	} else {
	    lappend goodlist $option $value
	}
    }

    # Check that all required options are present
    foreach opt $required_options {
	if {[lsearch -exact $option_list $opt] < 0} {
	    error "Required option $opt is missing for\
		    $plottype ntuple plot (ntuple id is $id).\
		    For this type of plot required options\
		    are [join $required_options {, }]."
	}
    }

    # Check if a similar plot already exists in this overlay.
    # "Similar" means the same ntuple id, plot type, all required
    # options, sliders, and filter
    variable Ntuple_plot_info
    array set provided_options $goodlist
    set plot_found 0
    for {set index 0} {$index < $Overlay_count($name)} {incr index} {
	if {![string equal $Overlay_info($name,$index,header) "NTupleItem"]} continue
	set plotname $Overlay_info($name,$index,id)
	if {$id != $Ntuple_plot_info($plotname,id)} continue
	if {![string equal $Ntuple_plot_info($plotname,plottype) $plottype]} continue
	set options_match 1
	foreach opt {x y z nbins sliders} {
	    if {[info exists provided_options($opt)]} {
		if {![string equal $provided_options($opt) \
			$Ntuple_plot_info($plotname,$opt)]} {
		    set options_match 0
		    break
		}
	    }
	}
	if {!$options_match} continue
	set plot_found 1
	break
    }

    if {!$plot_found} {
	# Set some default option values
	variable Ntuple_plot_counter
	set plotname "nTuPLe_Pl0T_[incr Ntuple_plot_counter]"

	set Ntuple_plot_info($plotname,id) $id
	set Ntuple_plot_info($plotname,plottype) $plottype
	set Ntuple_plot_info($plotname,marker) 0
	set Ntuple_plot_info($plotname,markersize) 1
	set Ntuple_plot_info($plotname,line) 1
	set Ntuple_plot_info($plotname,color) black
	set Ntuple_plot_info($plotname,fill) none
	set Ntuple_plot_info($plotname,binlimit) 20
	set Ntuple_plot_info($plotname,refcount) 1
	set Ntuple_plot_info($plotname,owner) 0
	set Ntuple_plot_info($plotname,nxbins) 0
	set Ntuple_plot_info($plotname,nybins) 0
	set Ntuple_plot_info($plotname,lang) none
	set Ntuple_plot_info($plotname,filter) ""
	set Ntuple_plot_info($plotname,snapshot) 0

	set Overlay_info($name,$index,id) $plotname
	set Overlay_info($name,$index,type) $plottype
	set Overlay_info($name,$index,header) "NTupleItem"
	incr Overlay_count($name)
    }

    # Now, remember the provided option values
    foreach {option value} $goodlist {
	set Ntuple_plot_info($plotname,$option) $value
    }
    if {![info exists Ntuple_plot_info($plotname,fillcolor)]} {
	set Ntuple_plot_info($plotname,fillcolor) $Ntuple_plot_info($plotname,color)
    }

    # Check that option settings are compatible
    if {![string equal $language "none"]} {
	if {[info exists provided_options(snapshot)]} {
	    if {$Ntuple_plot_info($plotname,snapshot) == 0} {
		error "mutually incompatible settings specified\
			for options \"-snapshot\" and \"-lang\""
	    }
	} else {
	    set Ntuple_plot_info($plotname,snapshot) 1
	}
	if {[info exists provided_options(owner)]} {
	    if {$Ntuple_plot_info($plotname,owner)} {
		error "mutually incompatible settings specified\
			for options \"-owner\" and \"-lang\""
	    }
	}
    }
    if {$Ntuple_plot_info($plotname,snapshot) && \
	    $Ntuple_plot_info($plotname,owner)} {
	error "mutually incompatible settings specified\
		for options \"-owner\" and \"-snapshot\""
    }

    # Project the ntuple in case we've got a snapshot
    if {$Ntuple_plot_info($plotname,snapshot)} {
	if {[string equal $language "none"]} {
	    # Save some memory by using only the necessary columns
	    set tmpid [hs::ntuple_subset [hs::Temp_uid] \
		    [hs::title $id] [hs::Temp_category] $id $variable_list]
	} else {
	    # Create a temporary ntuple
	    set tmpid [hs::create_ntuple [hs::Temp_uid] \
		    [hs::title $id] [hs::Temp_category] $variable_list]

	    # Figure out the filter expression (the default all-pass
	    # filter expression varies from language to language) and
	    # the command which will perform the projection
	    switch $language {
		Tcl -
		tcl {
		    if {[string equal $Ntuple_plot_info($plotname,filter) ""]} {
			set filter {expr 1}
		    } else {
			set filter $Ntuple_plot_info($plotname,filter)
		    }
		    set proj_command hs::ntuple_project
		}
		C -
		c {
		    if {[string equal $Ntuple_plot_info($plotname,filter) ""]} {
			set filter 1
		    } else {
			set filter $Ntuple_plot_info($plotname,filter)
		    }
		    set proj_command hs::ntuple_c_project
		}
		default {
		    error "incomplete language type switch statement (3) in\
			    [lindex [info level 0] 0]. This is a bug. Please report."
		}
	    }
	    
	    # Perform the projection
	    global ::errorInfo
	    set status [catch {eval $proj_command $id $tmpid \
		    [list $filter] $variable_list} errMes]
	    set savedInfo $::errorInfo
	    if {$status} {
		catch {hs::delete $tmpid}
		error $errMes $savedInfo
	    }
	}

	# Remember the snapshot id instead of the original ntuple id
	set Ntuple_plot_info($plotname,id) $tmpid
	set Ntuple_plot_info($plotname,owner) 1
    }

    return
}

############################################################################

proc ::hs::Ntuple_plot_getconfig {name colorscale_name is_color_dynamic} {
    # This is an internal hs procedure and should not be called
    # directly by an application
    variable Ntuple_plot_info

    set plottype $Ntuple_plot_info($name,plottype)
    set id $Ntuple_plot_info($name,id)
    set nplots 1

    # Construct the configuration string
    set type_strings {
	ts     TimeSeriesPlot
	tse    TSPlotWithErrors
	xy     XYPlot
	xye    XYPlotWithErrors
	xys    SortedXY
	xyse   SortedXYWithErrors
	scat2  XYScatterPlot
	scat3  XYZScatterPlot
	h1     1DHistogram
	h1a    1DAdaptiveHistogram
	h2     2DHistogram
	h2a    2DAdaptiveHistogram
        cell   CellPlot
        tartan ColorCellPlot
        cscat2 ColorXYScatterPlot
    }
    array set type_strings_array $type_strings

    # Basic plot info
    set config_string "NTupleItem\n\
	    Category [hs::category $id]\n\
	    UID [hs::uid $id]\n\
	    Title [hs::title $id]\n\
	    PlotType $type_strings_array($plottype)\n"

    # Add the color scale name for plots which may need it
    if {[string equal $plottype "tartan"] || \
	    [string equal $plottype "cscat2"]} {
	if {![string equal $colorscale_name ""]} {
	    append config_string "\
		    ColorScaleName $colorscale_name\n"
	}
	if {$is_color_dynamic} {
	    append config_string "\
		    DynamicColor\n"
	}
    }

    # Next part depends on the plot type
    if {[string equal -length 2 $plottype "ts"] || \
	    [string equal -length 2 $plottype "xy"]} {
	# This is an X-Y style plot
	set vnumber 0
	switch -- $plottype {
	    ts {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
	    }
	    tse {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey+)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey+)\n"
		}
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey-)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey-)\n"
		}
	    }
	    xy {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,x)\n"
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
	    }
	    xye {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,x)\n"
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey+)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey+)\n"
		}
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey-)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey-)\n"
		}
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ex+)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ex+)\n"
		}
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ex-)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ex-)\n"
		}
	    }
	    xys {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,x)\n"
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
	    }
	    xyse {
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,x)\n"
		incr vnumber
		append config_string "\
			V$vnumber $Ntuple_plot_info($name,y)\n"
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey+)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey+)\n"
		}
		incr vnumber
		if {[info exists Ntuple_plot_info($name,ey-)]} {
		    append config_string "\
			    V$vnumber $Ntuple_plot_info($name,ey-)\n"
		}
	    }
	    default {
		error "Incomplete plot type switch statement 3.\
			This is a bug. Please report."
	    }
	}
	# Set markers and colors
	foreach {option keyword} {
	    line_st        LineStyle
	    color_st       LineColor
	    marker_st      MarkerStyle
	    markersize_st  MarkerSize
	} {
	    append config_string "\
		    ${keyword}1 $Ntuple_plot_info($name,$option)\n"
	}
	# Set marker color to the line color
	append config_string "\
		MarkerColor1 $Ntuple_plot_info($name,color_st)\n"
    } else {
	# This is a histogram, scatter plot, or cell plot
	append config_string "\
		V1 $Ntuple_plot_info($name,x)\n"
	if {![string equal -length 2 $plottype "h1"]} {
	    append config_string "\
		    V2 $Ntuple_plot_info($name,y)\n"
	}
	if {[string equal $plottype "scat3"] || \
		[string equal $plottype "cscat2"]} {
	    append config_string "\
		    V3 $Ntuple_plot_info($name,z)\n"
	}
	# Add slider variables
	set snumber 0
	foreach slidervar $Ntuple_plot_info($name,sliders) {
	    incr snumber
	    append config_string "\
		    S$snumber $slidervar\n"
	}
	# Number of bins
	if {[string equal $plottype "h1"]} {
	    append config_string "\
		    NumberOfBins $Ntuple_plot_info($name,nbins)\n"
	}
	if {[string equal $plottype "h2"] || \
		[string equal $plottype "cell"] || \
		[string equal $plottype "tartan"]} {
	    if {$Ntuple_plot_info($name,nxbins) > 0} {
		append config_string "\
			NumberOfXBins $Ntuple_plot_info($name,nxbins)\n"
	    }
	    if {$Ntuple_plot_info($name,nybins) > 0} {
		append config_string "\
			NumberOfYBins $Ntuple_plot_info($name,nybins)\n"
	    }
	}
	# Bin limits
	if {[string equal $plottype "h1a"]} {
	    append config_string "\
		    BinLimit $Ntuple_plot_info($name,binlimit)\n"
	}
	# Style options. At this point, they can exist
        # only for h1 and h1a plots.
	foreach {option keyword} {
	    line_st      LineStyle1
	    color_st     LineColor1
            fill_st      FillStyle1
            fillcolor_st FillColor1
	} {
	    append config_string "\
		    $keyword $Ntuple_plot_info($name,$option)\n"
	}
    }
    return $config_string
}

#########################################################################

proc ::hs::Validate_overlay_name {name} {
    if {[string is integer $name]} {
	error "overlay name can not be just an integer"
    }
    if {[string equal $name ""]} {
	error "overlay name can not be just an empty string"
    }
    if {[string match {*[\*\?,\[]*} $name]} {
	error "overlay name can not contain special characters *?,\[\]"
    }
    return
}

############################################################################

proc ::hs::Get_overlay_property {name property} {
    variable Overlay_count
    if {[info exists Overlay_count($name)]} {
	variable Overlay_info
	if {[info exists Overlay_info($name,$property)]} {
	    return $Overlay_info($name,$property)
	} else {
	    error "Bad overlay property \"$property\""
	}
    } else {
	error "Overlay named \"$name\" does not exist"
    }
}

############################################################################

proc ::hs::Ntuple_plot_decr_refcount {name} {
    variable Ntuple_plot_info
    if {![info exists Ntuple_plot_info($name,id)]} return
    if {[incr Ntuple_plot_info($name,refcount) -1] == 0} {
	if {$Ntuple_plot_info($name,owner)} {
	    set hs_id $Ntuple_plot_info($name,id)
	    if {![string equal [hs::type $hs_id] "HS_NONE"]} {
		hs::delete $hs_id
	    }
	}
	array unset Ntuple_plot_info $name,*
    }
    return
}

############################################################################

proc ::hs::Overlay_destroy {name} {
    variable Overlay_count
    if {![info exists Overlay_count($name)]} return
    variable Overlay_info
    for {set index 0} {$index < $Overlay_count($name)} {incr index} {
	if {[string equal $Overlay_info($name,$index,header) "NTupleItem"]} {
	    hs::Ntuple_plot_decr_refcount $Overlay_info($name,$index,id)
	} elseif {$Overlay_info($name,$index,owner)} {
	    hs::Overlay_decr_refcount $Overlay_info($name,$index,id)
	}
    }
    array unset Overlay_info $name,*
    unset Overlay_count($name)
    return
}

############################################################################

proc ::hs::Parse_option_sequence {sequence keys arrname} {
    upvar $arrname options
    array unset options
    foreach key $keys {
	set options($key,count) 0
    }
    # Look for keywords
    set value {}
    set key {}
    foreach word $sequence {
	if {[lsearch -exact $keys $word] >= 0} {
	    # Found a keyword
	    if {[llength $key] > 0} {
		set options($key,$options($key,count)) $value
		incr options($key,count)
	    }
	    set key $word
	    set value {}
	} else {
	    if {[llength $key] > 0} {
		lappend value $word
	    } else {
		# The first word in the sequence is not a known key
		error "invalid option \"$word\""
	    }
	}
    }
    if {[llength $key] > 0} {
	set options($key,$options($key,count)) $value
	incr options($key,count)
    }
    return
}

#########################################################################

proc ::hs::periodic_update {msec} {
    set lowlevel_info [info commands ::hs::Periodic_update_l_o_w_level]
    if {[string equal $msec "stop"]} {
	if {[llength $lowlevel_info] > 0} {
	    after cancel ::hs::Periodic_update_l_o_w_level
	    ::rename ::hs::Periodic_update_l_o_w_level {}
	}
    } elseif {[string equal $msec "period"]} {
	if {[llength $lowlevel_info] > 0} {
	    return [lindex [info body ::hs::Periodic_update_l_o_w_level] 2]
	} else {
	    return 0
	}
    } else {
	if {![string is integer -strict $msec]} {
	    error "hs::periodic_update : bad argument \"$msec\""
	} elseif {$msec <= 0} {
	    error "hs::periodic_update : bad argument \"$msec\""
	}
	proc ::hs::Periodic_update_l_o_w_level {} "::hs::Push_update;\
		after $msec ::hs::Periodic_update_l_o_w_level; return"
	if {[llength $lowlevel_info] == 0} {
	    ::hs::Periodic_update_l_o_w_level
	}
    }
    return
}

#########################################################################

proc ::hs::Push_update {} {
    ::hs::hs_update
    while {[::hs::socket_status]} {
	::update
	::hs::hs_update
    }
}

#########################################################################

proc ::hs::wait_num_scopes {operator n} {

    set cycle_delay 100
    set timeout 60000

    if {[string compare $operator ">"] &&\
	    [string compare $operator "<"] &&\
	    [string compare $operator "<="] &&\
	    [string compare $operator ">="] &&\
	    [string compare $operator "=="] &&\
	    [string compare $operator "!="]} {
	error "invalid operator \"$operator\""
	return
    }
    set maxcycles [expr {$timeout / $cycle_delay}]
    for {set i 0} {$i < $maxcycles} {incr i} {
	hs::hs_update
        if [expr [hs::num_connected_scopes] $operator $n] break
	after $cycle_delay
    }
    if {$i == $maxcycles} {
	error "hs::wait_num_scopes timed out after [expr $timeout/1000] sec"
    }
    return
}

#############################################################################

proc ::hs::Config_scale_repeat {scalewidgets value} {
    foreach w $scalewidgets {
	$w configure -repeatinterval $value
    }
    return
}

#############################################################################

proc ::hs::Int_entry_incr {entry min max} {
    # This is an internal hs procedure and should not be called
    # directly by an application
    set i [$entry get]
    if {[string equal $i ""]} {
	set i $min
    } elseif {$i < $min} {
	set i $min
    } elseif {$i < $max} {
	set i [expr {$i + $i/5 + 1}]
	if {$i > $max} {
	    set i $max
	}
    } else {
	set i $max
    }
    $entry delete 0 end
    $entry insert 0 $i
    return
}

#############################################################################

proc ::hs::Int_entry_decr {entry min max} {
    # This is an internal hs procedure and should not be called
    # directly by an application
    set i [$entry get]
    if {[string equal $i ""]} {
	set i $max
    } elseif {$i > $max} {
	set i $max
    } elseif {$i > $min} {
	set i [expr {$i - $i/5 - 1}]
	if {$i < $min} {
	    set i $min
	}
    } else {
	set i $min
    }
    $entry delete 0 end
    $entry insert 0 $i
    return
}

#############################################################################

proc ::hs::Int_entry_validate {entry value failcolor min max command} {
    # This is an internal hs procedure and should not be called
    # directly by an application
    if {[string equal $value ""]} {
	return 1
    }
    if {![string is integer $value]} {
	return 0
    }
    set len [string length $value]
    if {$len > [string length $min] && $len > [string length $max]} {
	return 0
    }
    if {$value < $min || $value > $max} {
	$entry configure -foreground $failcolor
	return 1
    }
    $entry configure -foreground black
    if {![string equal $command ""]} {
	eval $command $value
    }
    return 1
}

#############################################################################

proc ::hs::Int_entry_value {frame} {
    $frame.left.entry get
}

#############################################################################

proc ::hs::Int_entry_init_bitmaps {} {
    variable Integer_entry_incr_xbm
    if {![info exists Integer_entry_incr_xbm]} {
	set Integer_entry_incr_xbm \
		[image create bitmap -data \
		"#define incr_width 7\n\
		#define incr_height 4\n\
		static char incr_bits[] = {\n\
		0x08, 0x1c, 0x3e, 0x7f};\n"]
    }
    variable Integer_entry_decr_xbm
    if {![info exists Integer_entry_decr_xbm]} {
	set Integer_entry_decr_xbm \
		[image create bitmap -data \
		"#define decr_width 7\n\
		#define decr_height 4\n\
		static char decr_bits[] = {\n\
		0x7f, 0x3e, 0x1c, 0x08};\n"]
    }
    return
}

#############################################################################

proc ::hs::Int_entry_widget {frame label failcolor \
	min max init incr_proc decr_proc run_command} {
    # This is an internal hs procedure and should not be called
    # directly by an application. First, check the arguments.
    foreach name {init min max} {
	if {![string is integer -strict [set $name]]} {
	    error "$name value is not an integer"
	}
    }
    if {$min > $max} {
	error "min value is higher than max value"
    }
    if {$init < $min || $init > $max} {
	error "init value is out of range"
    }
    # Build the widget
    frame $frame
    frame $frame.left
    frame $frame.right
    pack $frame.left $frame.right -side left
    label $frame.left.label -text $label
    set entrywidth [string length $min]
    set lenmax [string length $max]
    if {$lenmax > $entrywidth} {
	set entrywidth $lenmax
    }
    entry $frame.left.entry -width $entrywidth \
	    -validate key -validatecommand \
	    [list ::hs::Int_entry_validate %W %P \
	    $failcolor $min $max $run_command]
    $frame.left.entry insert 0 $init
    pack $frame.left.label $frame.left.entry -side left

    hs::Int_entry_init_bitmaps
    variable Integer_entry_incr_xbm
    button $frame.right.up -image $Integer_entry_incr_xbm \
	    -command [list $incr_proc $frame.left.entry $min $max]
    variable Integer_entry_decr_xbm
    button $frame.right.down -image $Integer_entry_decr_xbm \
	    -command [list $decr_proc $frame.left.entry $min $max]

    pack $frame.right.up $frame.right.down -side top
    return $frame
}

#############################################################################

proc ::hs::slice_slider {parent_id axis1 {axis2 {}}} {
    # Require Tk 8.3 or higher. Needed for -validatecommand in
    # the entry widget
    set tk_not_found 1
    if {[catch {package require Tk} version_tk] == 0} {
	foreach {major minor} [split $version_tk .] break
	if {$major > 8 || ($major == 8 && $minor >= 3)} {
	    set tk_not_found 0
	}
    }
    if {$tk_not_found} {
	error "Sorry, [lindex [info level 0] 0] needs Tk version 8.3 or higher"
    }
    if {[string equal $axis2 ""]} {
	return [hs::Slice_slider_1 $parent_id $axis1]
    }
    hs::Slice_slider_2 $parent_id $axis1 $axis2
}

#############################################################################

proc ::hs::Slice_slider_2 {parent_id axis1 axis2} {
    set parent_type [hs::type $parent_id]
    if {[string compare $parent_type "HS_3D_HISTOGRAM"]} {
	if {[string equal $parent_type "HS_2D_HISTOGRAM"]} {
	    error "can not specify two slice axes for a 2d histogram"
	} else {
	    error "item with id $parent_id is not a 3d histogram"
	}
    }
    if {[string equal -nocase $axis1 $axis2]} {
	error "please specify distinct slice axes"
    }
    set axis_list [list $axis1 $axis2]
    foreach axis $axis_list {
	if {[string compare -nocase $axis "x"] && \
		[string compare -nocase $axis "y"] && \
		[string compare -nocase $axis "z"]} {
	    error "invalid slice axis \"$axis\", expected x, y, or z"
	}
    }

    # Make the slice now. We will use its id as a unique index.
    set slice_id [hs::slice_histogram [hs::next_category_uid "Slices"]\
	    "Dummy title" "Slices" $parent_id $axis1 0 $axis2 0]

    # Start building the GUI
    set w .sliceslide_$slice_id
    toplevel $w
    wm withdraw $w
    frame $w.scales

    set hist_labels [hs::3d_hist_labels $parent_id]
    foreach {nxbins nybins nzbins} [hs::3d_hist_num_bins $parent_id] {}
    foreach {xmin xmax ymin ymax zmin zmax} [hs::3d_hist_range $parent_id] {}
    set num 0
    foreach axis [lsort -dictionary $axis_list] {
	switch [set direction_$num [string toupper $axis]] {
	    X {
		set label_$num [lindex $hist_labels 0]
		set lolim $xmin
		set hilim $xmax
		set nbins $nxbins
	    }
	    Y {
		set label_$num [lindex $hist_labels 1]
		set lolim $ymin
		set hilim $ymax
		set nbins $nybins
	    }
	    Z {
		set label_$num [lindex $hist_labels 2]
		set lolim $zmin
		set hilim $zmax
		set nbins $nzbins
	    }
	    default {
		error "invalid slice axis \"$axis\", expected x, y, or z"
	    }
	}

	set binwidth [expr {($hilim-$lolim)/1.0/$nbins}]
	label $w.scales.l$num -text "[set label_$num] :"
	scale $w.scales.s$num -orient horizontal -length 384 \
		-tickinterval [expr {$hilim-$binwidth-$lolim}] \
		-from $lolim -to [expr {$hilim-$binwidth}] \
		-resolution $binwidth -repeatinterval 300
        bind $w.scales.s$num <ButtonPress-1> +[list focus %W]
        bind $w.scales.s$num <KeyPress-Up> break
        bind $w.scales.s$num <KeyPress-Down> break
	grid $w.scales.l$num -column 0 -row $num -sticky e -padx 1
	grid $w.scales.s$num -column 1 -row $num -sticky ew -padx 1
	
	incr num
    }
    grid columnconfigure $w.scales 0 -weight 0
    grid columnconfigure $w.scales 1 -weight 1

    $w.scales.s0 configure -command [list hs::Slice_slider_2_scale_callback \
	    $slice_id $parent_id $direction_1 $w.scales.s1 $direction_0]
    $w.scales.s1 configure -command [list hs::Slice_slider_2_scale_callback \
	    $slice_id $parent_id $direction_0 $w.scales.s0 $direction_1]

    frame $w.controls
    button $w.controls.dismiss -text "Dismiss" -width 8 \
	    -command "destroy $w;\
	    hs::close_window sliceslide_$slice_id 1;\
	    hs::delete $slice_id"
    button $w.controls.snapshot -text "Snapshot" -width 8 \
	    -command "set snaptitle \"$label_0 = \[$w.scales.s0 get\],\
	    $label_1 = \[$w.scales.s1 get\] slice of \\\"[hs::title $parent_id]\\\"\";\
	    hs::show_histogram \[hs::copy_hist $slice_id\
	    \[hs::next_category_uid \"Slices\"\]\
	    \$snaptitle Slices\]"

    ::hs::Int_entry_widget $w.controls.speed "Frame delay (ms): " \
	    red2 1 9999 300 ::hs::Int_entry_incr ::hs::Int_entry_decr \
	    [list ::hs::Config_scale_repeat [list $w.scales.s0 $w.scales.s1]]

    pack $w.controls.speed -side left -padx 2 -fill x
    pack $w.controls.dismiss -side right -padx 2 -fill x
    pack $w.controls.snapshot -side right -padx 2 -fill x
    pack $w.scales -side top -padx 4 -fill x -expand 1
    pack $w.controls -side top -padx 5 -fill x -expand 1
    frame $w.spacer -height 8
    pack $w.spacer -side top -fill x -expand 1
    
    wm title $w "$label_0 and $label_1 slice coordinates\
	    for \"[hs::title $parent_id]\""
    wm geom $w "+100+490"
    update idletasks
    wm deiconify $w
    wm resizable $w 1 0
    wm protocol $w WM_DELETE_WINDOW "destroy $w; hs::delete $slice_id"

    hs::change_title $slice_id "$label_0 and $label_1\
	    slice of \"[hs::title $parent_id]\""
    hs::show_histogram $slice_id -geometry "400x300+100+150" \
	    -window "sliceslide_$slice_id"
    hs::hs_update

    list $w $slice_id
}

#############################################################################

proc ::hs::Slice_slider_2_scale_callback {slice_id \
	parent_id other_dir other_scale mydir x} {
    hs::fill_slice $slice_id $parent_id $other_dir [$other_scale get] $mydir $x
    hs::hs_update
}

#############################################################################

proc ::hs::Slice_slider_1_scale_callback {slice_id parent_id direction x} {
    hs::fill_slice $slice_id $parent_id $direction $x
    hs::hs_update
}

#############################################################################

proc ::hs::Slice_slider_1 {parent_id direction} {
    switch [hs::type $parent_id] {
	HS_2D_HISTOGRAM {
	    set hist_labels [hs::2d_hist_labels $parent_id]
	    foreach {nxbins nybins} [hs::2d_hist_num_bins $parent_id] {}
	    foreach {xmin xmax ymin ymax} [hs::2d_hist_range $parent_id] {}
	    if {[string equal -nocase $direction "Z"]} {
		error "Z is not a valid slice axis for a 2d histogram"
	    }
	}
	HS_3D_HISTOGRAM {
	    set hist_labels [hs::3d_hist_labels $parent_id]
	    foreach {nxbins nybins nzbins} [hs::3d_hist_num_bins $parent_id] {}
	    foreach {xmin xmax ymin ymax zmin zmax} [hs::3d_hist_range $parent_id] {}
	}
	default {
	    error "item with id $parent_id is neither 2d nor 3d histogram"
	}
    }
    if {[string equal -nocase $direction "X"]} {
	set new_label [lindex $hist_labels 0]
	set wingeometry +100+490
	set plotgeometry 400x300+100+150
	set direction "X"
	set lolim $xmin
	set hilim $xmax
	set nbins $nxbins
    } elseif {[string equal -nocase $direction "Y"]} {
	set new_label [lindex $hist_labels 1]
	set wingeometry +516+490
	set plotgeometry 400x300+516+150
	set direction "Y"
	set lolim $ymin
	set hilim $ymax
	set nbins $nybins
    } elseif {[string equal -nocase $direction "Z"]} {
	set new_label [lindex $hist_labels 2]
	set wingeometry +100+490
	set plotgeometry 400x300+100+150
	set direction "Z"
	set lolim $zmin
	set hilim $zmax
	set nbins $nzbins
    } else {
	error "invalid slice direction \"$direction\""
	return
    }
    set newtitle "$new_label slice of \"[hs::title $parent_id]\""
    set slice_id [hs::slice_histogram [hs::next_category_uid "Slices"]\
	    $newtitle "Slices" $parent_id $direction 0]
    hs::show_histogram $slice_id -geometry $plotgeometry \
	    -window "sliceslide_$slice_id"

    set w .sliceslide_$slice_id
    toplevel $w
    wm withdraw $w
    wm title $w "$new_label slice coordinate for \"[hs::title $parent_id]\""

    # Figure out the best stepping for the scale
    set binwidth [expr {($hilim-$lolim)/1.0/$nbins}]

    scale $w.scale -orient horizontal -length 384 \
	    -tickinterval [expr {$hilim-$binwidth-$lolim}]\
	    -from $lolim -to [expr {$hilim-$binwidth}] \
	    -resolution $binwidth -repeatinterval 300 -command \
	    "hs::Slice_slider_1_scale_callback $slice_id $parent_id $direction"
    bind $w.scale <ButtonPress-1> +[list focus %W]
    bind $w.scale <KeyPress-Up> break
    bind $w.scale <KeyPress-Down> break

    frame $w.controls
    button $w.controls.dismiss -text "Dismiss" -width 8 \
	    -command "destroy $w;\
	    hs::close_window sliceslide_$slice_id 1;\
	    hs::delete $slice_id"
    button $w.controls.snapshot -text "Snapshot" -width 8 \
	    -command "set snaptitle \"$new_label = \[$w.scale get\]\
	    slice of \\\"[hs::title $parent_id]\\\"\";\
	    hs::show_histogram \[hs::copy_hist $slice_id\
	    \[hs::next_category_uid \"Slices\"\]\
	    \$snaptitle Slices\]"

    ::hs::Int_entry_widget $w.controls.speed "Frame delay (ms): " \
	    red2 1 9999 300 ::hs::Int_entry_incr ::hs::Int_entry_decr \
	    [list ::hs::Config_scale_repeat [list $w.scale]]

    pack $w.controls.speed -side left -padx 2 -fill x
    pack $w.controls.dismiss -side right -padx 2 -fill x
    pack $w.controls.snapshot -side right -padx 2 -fill x
    pack $w.scale -side top -padx 5 -fill x -expand 1
    pack $w.controls -side top -padx 5 -fill x -expand 1
    frame $w.spacer -height 8
    pack $w.spacer -side top -fill x -expand 1

    update idletasks
    wm geom $w $wingeometry
    wm deiconify $w
    wm resizable $w 1 0
    wm protocol $w WM_DELETE_WINDOW "destroy $w; hs::delete $slice_id"
    list $w $slice_id
}

###########################################################################

proc ::hs::ntuple_variable_list {id {pattern *}} {
    set numvars [hs::num_variables $id]
    set varlist {}
    if {[string equal $pattern "*"]} {
        for {set i 0} {$i < $numvars} {incr i} {
            lappend varlist [hs::variable_name $id $i]
        }
    } else {
        for {set i 0} {$i < $numvars} {incr i} {
            set varname [hs::variable_name $id $i]
            if {[string match $pattern $varname]} {
                lappend varlist $varname
            }
        }
    }
    set varlist
}

###########################################################################

proc ::hs::Ntuple_project_onntuple {nt_id_hUJh78wdq proj_id_sJH9lvF8\
	f_expr_jhRF6ak8m var_expr_jhu79R_7ShkPiC} {
    #
    # Almost all variables in this proc must have obscure names so that
    # they do not interfere with variable names in the ntuple
    # 
    if {[string compare [hs::type $nt_id_hUJh78wdq] "HS_NTUPLE"]} {
	error "Item with id $nt_id_hUJh78wdq is not an ntuple"
    }
    if {[string compare [hs::type $proj_id_sJH9lvF8] "HS_NTUPLE"]} {
	error "Item with id $proj_id_sJH9lvF8 is not an ntuple"
    }
    if {[info complete $f_expr_jhRF6ak8m] == 0} {
	error "filter expression is not complete"
    }
    foreach ex_asdHFytfyhR $var_expr_jhu79R_7ShkPiC {
	if {[info complete $ex_asdHFytfyhR] == 0} {
	    error "expression \"$ex_asdHFytfyhR\" is not complete"
	}
    }
    set n_v_Tfl389GrQ [hs::num_variables $proj_id_sJH9lvF8]
    if {[llength $var_expr_jhu79R_7ShkPiC] != $n_v_Tfl389GrQ} {
	error "Wrong number of expressions for projection\
		onto ntuple with id $proj_id_sJH9lvF8"
    }
    # hs::reset $proj_id_sJH9lvF8
    set n_en_n76KjhteO [hs::num_entries $nt_id_hUJh78wdq]
    set n_v_kGs69oFU [hs::num_variables $nt_id_hUJh78wdq]
    set vl8KG9okF3hjY [::hs::ntuple_variable_list $nt_id_hUJh78wdq]
    for {set i_kjGD4xs9 0} {$i_kjGD4xs9 < $n_en_n76KjhteO} {incr i_kjGD4xs9} {
	binary scan [hs::row_contents $nt_id_hUJh78wdq $i_kjGD4xs9]\
		f$n_v_kGs69oFU hHJGi3489h
	foreach $vl8KG9okF3hjY $hHJGi3489h {}
	if {[eval $f_expr_jhRF6ak8m]} {
	    set val_list_gWl9c2Md9_wJ {}
	    foreach x_expr_6hjdas4e $var_expr_jhu79R_7ShkPiC {
		lappend val_list_gWl9c2Md9_wJ [eval $x_expr_6hjdas4e]
	    }
	    hs::fill_ntuple $proj_id_sJH9lvF8 $val_list_gWl9c2Md9_wJ
	}
    }
    return
}

###########################################################################

proc ::hs::Ntuple_project_onhisto {nt_id_hUJh78wdq hs_id_sJH9lvF8 f_expr_jhRF6ak8m\
        w_expr_jh7Kg39r x_expr_6hjdas4e {y_expr_53nsl7wqD {}} {z_expr_fK2Vo6mSdJ {}}} {
    #
    # Almost all variables in this proc must have obscure names so that
    # they do not interfere with variable names in the ntuple
    # 
    if {[string compare [hs::type $nt_id_hUJh78wdq] "HS_NTUPLE"]} {
	error "Item with id $nt_id_hUJh78wdq is not an ntuple"
    }
    if {[info complete $f_expr_jhRF6ak8m] == 0} {
	error "filter expression is not complete"
    }
    if {[info complete $w_expr_jh7Kg39r] == 0} {
	error "weight expression is not complete"
    }
    if {[info complete $x_expr_6hjdas4e] == 0} {
	error "x expression is not complete"
    }
    if {$y_expr_53nsl7wqD == {}} {
	# Expect 1d histogram as an argument
	set ndim 1
    } else {
	if {[info complete $y_expr_53nsl7wqD] == 0} {
	    error "y expression is not complete"
	}
	if {$z_expr_fK2Vo6mSdJ == {}} {
	    # Expect 2d histogram
	    set ndim 2
	} else {
	    # Expect 3d histogram
	    if {[info complete $z_expr_fK2Vo6mSdJ] == 0} {
		error "z expression is not complete"
	    }
	    set ndim 3
	}
    }
    if {[string compare [hs::type $hs_id_sJH9lvF8] "HS_${ndim}D_HISTOGRAM"]} {
	error "Item with id $hs_id_sJH9lvF8 is not a ${ndim}d histogram"
    }
    # hs::reset $hs_id_sJH9lvF8
    set n_en_n76KjhteO [hs::num_entries $nt_id_hUJh78wdq]
    set n_v_kGs69oFU [hs::num_variables $nt_id_hUJh78wdq]
    set vl8KG9okF3hjY [::hs::ntuple_variable_list $nt_id_hUJh78wdq]
    if {$ndim == 1} {
	for {set i_kjGD4xs9 0} {$i_kjGD4xs9 < $n_en_n76KjhteO} {incr i_kjGD4xs9} {
	    binary scan [hs::row_contents $nt_id_hUJh78wdq $i_kjGD4xs9]\
		    f$n_v_kGs69oFU hHJGi3489h
	    foreach $vl8KG9okF3hjY $hHJGi3489h {}
	    if {[eval $f_expr_jhRF6ak8m]} {
		hs::fill_1d_hist $hs_id_sJH9lvF8\
			[eval $x_expr_6hjdas4e] [eval $w_expr_jh7Kg39r]
	    }
	}
    } elseif {$ndim == 2} {
	for {set i_kjGD4xs9 0} {$i_kjGD4xs9 < $n_en_n76KjhteO} {incr i_kjGD4xs9} {
	    binary scan [hs::row_contents $nt_id_hUJh78wdq $i_kjGD4xs9]\
		    f$n_v_kGs69oFU hHJGi3489h
	    foreach $vl8KG9okF3hjY $hHJGi3489h {}
	    if {[eval $f_expr_jhRF6ak8m]} {
		hs::fill_2d_hist $hs_id_sJH9lvF8 [eval $x_expr_6hjdas4e]\
			[eval $y_expr_53nsl7wqD] [eval $w_expr_jh7Kg39r]
	    }
	}
    } else {
	for {set i_kjGD4xs9 0} {$i_kjGD4xs9 < $n_en_n76KjhteO} {incr i_kjGD4xs9} {
	    binary scan [hs::row_contents $nt_id_hUJh78wdq $i_kjGD4xs9]\
		    f$n_v_kGs69oFU hHJGi3489h
	    foreach $vl8KG9okF3hjY $hHJGi3489h {}
	    if {[eval $f_expr_jhRF6ak8m]} {
		hs::fill_3d_hist $hs_id_sJH9lvF8 [eval $x_expr_6hjdas4e]\
			[eval $y_expr_53nsl7wqD] [eval $z_expr_fK2Vo6mSdJ]\
                        [eval $w_expr_jh7Kg39r]
	    }
	}
    }
    return
}

############################################################################

proc ::hs::Exec_ignore_stderr {cmd} {
    # This command suppresses the errors produced by "exec"
    # when some program writes to the standard error.
    # Various compilers have a habit to write their warnings
    # to stderr, and we don't want to terminate the compilation
    # because of warnings.
    global errorCode ::errorInfo
    if {[catch {eval exec $cmd} result]} {
	set saveInfo $::errorInfo
	if {![string equal $errorCode "NONE"]} {
	    error $result $saveInfo
	}
    }
    set result
}

############################################################################

proc ::hs::sharedlib_compile {args} {
    # Arguments should be: file_list so_file
    # Options may be anywhere between the arguments
    set known_switches {-cflags -fflags -linkflags}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[llength $arglist] != 2} {
	error "wrong # of arguments or invalid option"
    }
    foreach {inp_file_list so_file} $arglist {}
    if {[info exists options(-cflags)]} {
	set cflags $options(-cflags)
    } else {
	set cflags {}
    }
    if {[info exists options(-fflags)]} {
	set fflags $options(-fflags)
    } else {
	set fflags {}
    }
    if {[info exists options(-linkflags)]} {
	set linkflags $options(-linkflags)
    } else {
	set linkflags {}
    }

    # Normalize the list of files. This is needed in order
    # to make sure that it is, indeed, a proper list rather
    # than a multiline string.
    set file_list [list]
    foreach file $inp_file_list {
        lappend file_list $file
    }

    variable Parse_tclConfig_sh_error
    if {[catch {hs::Parse_tclConfig_sh} errmes]} {
	if {$Parse_tclConfig_sh_error == 0} {
	    set Parse_tclConfig_sh_error 1
	    puts "$errmes. Will try using gcc/[hs::Fortran_executable]."
	}
	return [hs::Sharedlib_compile_gnu $file_list $so_file \
                $cflags $fflags $linkflags]
    } else {
	set Parse_tclConfig_sh_error 0
    }
    variable TclConfig_sh_defs
    variable Recognized_fortran_extensions
    global ::env ::errorInfo
    # Check that all tools can be found
    set cc [lindex $TclConfig_sh_defs(TCL_CC) 0]
    set ld [lindex $TclConfig_sh_defs(TCL_SHLIB_LD) 0]
    foreach name {cc ld} {
	set ${name}_exe [::auto_execok [set $name]]
	if {[string equal [set ${name}_exe] ""]} {
	    error "[set $name] executable not found"
	}
    }
    # Make sure to cleanup after we are done
    set ofiles {}
    set compile_status [catch {
	# Compile
	foreach file $file_list {
	    foreach {ofile chan} [::hs::tempfile \
		    [file join [::hs::tempdir] hs_ofile_] ".o"] {}
	    close $chan
	    lappend ofiles $ofile
	    set extension [string range [file extension $file] 1 end]
	    if {[lsearch -exact $Recognized_fortran_extensions $extension] >= 0} {
		# This is, probably, a Fortran file
                set fc f77
                foreach name {fc} {
                    set ${name}_exe [::auto_execok [set $name]]
                    if {[string equal [set ${name}_exe] ""]} {
                        error "[set $name] executable not found"
                    }
                }
		set cmd [concat [list $fc_exe] $fflags -c \
			$TclConfig_sh_defs(TCL_SHLIB_CFLAGS) \
			-o [list $ofile] [list $file]]
		if {[catch {hs::Exec_ignore_stderr $cmd}]} {
		    # Try without shlib flags
		    set cmd [concat [list $fc_exe] \
                            $fflags -c -o [list $ofile] [list $file]]
		    hs::Exec_ignore_stderr $cmd
		}
	    } else {
		# This is, probably, a C file
                variable Tcl_C_header_file
		set cmd [concat $TclConfig_sh_defs(TCL_CC) $cflags -c \
                        [list -I[file join [hs::Histo_dir_env] include]] \
                        [list -I[file dirname $Tcl_C_header_file]] \
			$TclConfig_sh_defs(TCL_EXTRA_CFLAGS) \
			$TclConfig_sh_defs(TCL_SHLIB_CFLAGS) \
			-o [list $ofile] [list $file]]
		hs::Exec_ignore_stderr $cmd
	    }
	}
	# Link
	set cmd [concat $TclConfig_sh_defs(TCL_SHLIB_LD) \
		-o [list $so_file] $ofiles $linkflags]
	hs::Exec_ignore_stderr $cmd
    } errmess]
    set saveInfo $::errorInfo
    foreach ofile $ofiles {
	catch {file delete $ofile}
    }
    if {$compile_status} {
	error $errmess $saveInfo
    }
    return
}

############################################################################

proc ::hs::Sharedlib_compile_gnu {file_list so_file cflags fflags linkflags} {

    # Try to determine the compiler
    set has_fortran 0
    variable Recognized_fortran_extensions
    foreach file $file_list {
	set extension [string range [file extension $file] 1 end]
	if {[lsearch -exact $Recognized_fortran_extensions $extension] >= 0} {
	    set has_fortran 1
	    break
	}
    }
    if {$has_fortran} {
	set compiler [hs::Fortran_executable]
        set compiler_flags [concat $cflags $fflags]
    } else {
	set compiler gcc
        set compiler_flags $cflags
    }

    # Find the compiler executable. auto_execok caches the results
    # of its searches, so it should be fast when called more than
    # once with the same argument.
    set compiler_exe [::auto_execok $compiler]
    if {[string equal $compiler_exe ""]} {
	error "$compiler executable not found"
    }

    # Compile the code
    set histo_include_dir [file join [hs::Histo_dir_env] include]
    variable Tcl_C_header_file
    eval exec $compiler_exe -fPIC -shared $compiler_flags \
	    [list -I$histo_include_dir] \
            [list -I[file dirname $Tcl_C_header_file]] \
	    -o [list $so_file] $file_list $linkflags
    return
}

###########################################################################

if {[llength [info procs ::hs::tempdir]] == 0} {
    proc ::hs::tempdir {} {
	return "/tmp"
    }
}

###########################################################################

proc ::hs::tempfile {prefix suffix} {
    set chars "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    set nrand_chars 10
    set maxtries 10
    # The file access mode and the permission should
    # guarantee some degree of security against an attack
    set access [list RDWR CREAT EXCL TRUNC]
    set permission 0600
    set channel ""
    set checked_dir_writable 0
    set mypid [pid]
    for {set i 0} {$i < $maxtries} {incr i} {
	set newname $prefix
	for {set j 0} {$j < $nrand_chars} {incr j} {
	    append newname [string index $chars \
		    [expr ([clock clicks] ^ $mypid) % 62]]
	}
	append newname $suffix
	if {[file exists $newname]} {
	    after 1
	} else {
	    if {[catch {open $newname $access $permission} channel]} {
		# Check that the directory is writable. This is a usual
		# source of errors. It is also possible that we are trying
		# to open a file which belongs to somebody else, but this
		# is far less likely -- this file has to appear between
		# the executions of "file exists" and "open" due to some
		# kind of a race condition.
		if {!$checked_dir_writable} {
		    set dirname [file dirname $newname]
		    if {![file writable $dirname]} {
			error "Directory $dirname is not writable"
		    }
		    set checked_dir_writable 1
		}
	    } else {
		# Success
		return [list $newname $channel]
	    }
	}
    }
    if {[string compare $channel ""]} {
	error "Failed to open a temporary file: $channel"
    } else {
	error "Failed to find an unused temporary file name"
    }
}

###########################################################################

proc ::hs::ntuple_c_covar {args} {
    # Expected arguments: nt_id c_filter_expr c_weight_expr column_list
    set known_switches {-include -eval}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-eval)]} {
        set utilCode $options(-eval)
    } else {
        set utilCode {}
    }
    if {[llength $arglist] != 4} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id c_filter_expr c_weight_expr column_list} $arglist {}

    if {[string compare [hs::type $nt_id] "HS_NTUPLE"]} {
	error "Item with id $nt_id is not an ntuple"
    }
    set nrows [hs::num_entries $nt_id]
    if {$nrows == 0} {
	error "Ntuple with id $nt_id is empty"
    }
    set namecount [llength $column_list]
    if {$namecount == 0} {
	error "List of expressions is empty"
    }

    # Check if all column expressions are just column names and if filter
    # and weight are trivial. In this case we can use a built-in function.
    set all_expr_simple 1
    set simple_column_numbers {}
    foreach name $column_list {
	set num [hs::variable_index $nt_id $name]
	if {$num < 0} {
	    set all_expr_simple 0
	    break
	}
	lappend simple_column_numbers $num
    }
    if {$all_expr_simple} {
	if {[hs::C_filter_is_allpass $c_filter_expr] && \
		[hs::C_weight_is_unit $c_weight_expr]} {
	    return [hs::Ntuple_covar $nt_id $simple_column_numbers]
	}
    }

    # Have to generate the C code
    set tmplist $column_list
    lappend tmplist $c_filter_expr $c_weight_expr
    set offending_variable [hs::Is_ntuple_c_compatible $nt_id $tmplist]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\"\
		can not be used in C code"
    }

    # Open a temporary file
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan $includeCode
    puts $chan "#define ZzZ_DATA_NTUPLE_ROWS $nrows"
    puts $chan "#define ZzZ_DATA_COLS_NEEDED $namecount"
    puts $chan ""
    puts $chan "int arr_2d_weighted_covar(const float *data, const float *w,\
	    const size_t ncols, const size_t nrows,\
	    const size_t *column_list, const size_t nused,\
	    float *averages, float *covar);"
    puts $chan "Tcl_Obj *new_matrix_obj(char mrep, int nrows, int ncols, double *data);"
    puts $chan "Tcl_Obj *new_list_of_doubles(int n, double *data);"
    puts $chan "static float *weights_jh4PgHdE = NULL;"
    puts $chan "static int row_cnt_Rhg8Sl2x = 0;"
    puts $chan "static int filter_count_F4NsQjBcSmHsxa = 0;"
    puts $chan ""
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id,\
	    const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "size_t memsize = ZzZ_DATA_NTUPLE_ROWS*(ZzZ_DATA_COLS_NEEDED+1)*sizeof(float) + "
    puts $chan "ZzZ_DATA_COLS_NEEDED*(ZzZ_DATA_COLS_NEEDED+1)*sizeof(float) + "
    puts $chan "ZzZ_DATA_COLS_NEEDED*(ZzZ_DATA_COLS_NEEDED+1)*sizeof(double);"
    puts $chan "weights_jh4PgHdE = (float *)malloc(memsize);"
    puts $chan "if (weights_jh4PgHdE == NULL)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "memset(weights_jh4PgHdE, 0, memsize);"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    puts $chan ""
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_Fic2DnAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $nt_id $chan row_dat_Fic2DnAE 0
    puts $chan "if ( $c_filter_expr ) \{"
    puts $chan "  float *offset_data_HJsF2hjgfTOb;"
    puts $chan "  $utilCode"
    puts $chan "  offset_data_HJsF2hjgfTOb =  weights_jh4PgHdE + \
	    row_cnt_Rhg8Sl2x*ZzZ_DATA_COLS_NEEDED + ZzZ_DATA_NTUPLE_ROWS;"
    set i 0
    foreach column $column_list {
	puts $chan "offset_data_HJsF2hjgfTOb\[$i\] = (float)($column);"
	incr i
    }
    puts $chan "  weights_jh4PgHdE\[row_cnt_Rhg8Sl2x++\] = (float)($c_weight_expr);"
    puts $chan "  ++filter_count_F4NsQjBcSmHsxa;"
    puts $chan "\} else"
    puts $chan "  weights_jh4PgHdE\[row_cnt_Rhg8Sl2x++\] = 0.f;"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    puts $chan ""
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "float *data, *averages, *covar;"
    puts $chan "int i, status;"
    puts $chan "double *daverages, *dcovar;"
    puts $chan "Tcl_Obj *res\[2\];"
    set column_numbers {}
    for {set i 0} {$i < $namecount} {incr i} {
	lappend column_numbers $i
    }
    puts $chan "size_t columns\[ZzZ_DATA_COLS_NEEDED\] =\
	    \{[join $column_numbers ,]\};"
    puts $chan "assert(row_cnt_Rhg8Sl2x == ZzZ_DATA_NTUPLE_ROWS);"
    puts $chan "if (filter_count_F4NsQjBcSmHsxa == 0) \{"
    puts $chan "Tcl_SetResult(interp, \"all ntuple rows have been\
	    rejected by the filter\", TCL_VOLATILE);"
    puts $chan "free(weights_jh4PgHdE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "data = weights_jh4PgHdE + ZzZ_DATA_NTUPLE_ROWS;"
    puts $chan "averages = data + ZzZ_DATA_NTUPLE_ROWS*ZzZ_DATA_COLS_NEEDED;"
    puts $chan "covar = averages + ZzZ_DATA_COLS_NEEDED;"
    puts $chan "daverages = (double *)(covar + ZzZ_DATA_COLS_NEEDED*ZzZ_DATA_COLS_NEEDED);"
    puts $chan "dcovar = daverages + ZzZ_DATA_COLS_NEEDED;"
    puts $chan "status = arr_2d_weighted_covar(data, weights_jh4PgHdE, ZzZ_DATA_COLS_NEEDED,\
	    ZzZ_DATA_NTUPLE_ROWS, columns, ZzZ_DATA_COLS_NEEDED, averages, covar);"
    puts $chan "if (status)"
    puts $chan "\{"
    puts $chan "switch (status)"
    puts $chan "\{"
    foreach {code message} {
	4 "found a negative weight"
	5 "all weights are 0"
    } {
	puts $chan "case $code:"
	puts $chan "Tcl_SetResult(interp, \"$message\", TCL_VOLATILE);"
	puts $chan "break;"
    }
    puts $chan "default:"
    puts $chan "   assert(0);"
    puts $chan "\}"
    puts $chan "free(weights_jh4PgHdE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "for (i=0; i<ZzZ_DATA_COLS_NEEDED; ++i)"
    puts $chan "daverages\[i\] = averages\[i\];"
    puts $chan "for (i=0; i<ZzZ_DATA_COLS_NEEDED*ZzZ_DATA_COLS_NEEDED; ++i)"
    puts $chan "dcovar\[i\] = covar\[i\];"
    puts $chan "res\[0\] = new_list_of_doubles(ZzZ_DATA_COLS_NEEDED, daverages);"
    puts $chan "res\[1\] = new_matrix_obj('c', ZzZ_DATA_COLS_NEEDED, ZzZ_DATA_COLS_NEEDED, dcovar);"
    puts $chan "if (res\[0\] == NULL || res\[1\] == NULL)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
    puts $chan "free(weights_jh4PgHdE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "Tcl_SetObjResult(interp, Tcl_NewListObj(2, res));"
    puts $chan "free(weights_jh4PgHdE);"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    close $chan

    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }

    # Scan the data ntuple using the new library
    set status [catch {hs::ntuple_so_scan $nt_id $sharedlib} result]
    file delete $filename $sharedlib
    if {$status} {error $result}
    set result
}

###########################################################################

proc ::hs::C_filter_is_allpass {filter} {
    set filter_expr [string trim $filter]
    if {[string is integer -strict $filter_expr]} {
	if {$filter_expr} {
	    return 1
	}
    }
    if {$filter_expr == 1 || \
	    [string equal $filter_expr "1.f"] || \
	    [string equal $filter_expr "1.0f"]} {
	return 1
    }
    return 0
}

###########################################################################

proc ::hs::C_weight_is_unit {w} {
    set weight_expr [string trim $w]
    if {$weight_expr == 1 || \
	    [string equal $weight_expr "1.f"] || \
	    [string equal $weight_expr "1.0f"]} {
	return 1
    }
    return 0
}

###########################################################################

proc ::hs::ntuple_project {nt_id proj_id filter_expr args} {
    switch [hs::type $proj_id] {
	HS_1D_HISTOGRAM {
	    if {[llength $args] != 2} {
		error "Wrong number of arguments for projection onto 1d histogram"
	    }
	    foreach {weight_expr x_expr} $args {}
	    return [hs::Ntuple_project_onhisto $nt_id $proj_id \
		    $filter_expr $weight_expr $x_expr]
	}
	HS_2D_HISTOGRAM {
	    if {[llength $args] != 3} {
		error "Wrong number of arguments for projection onto 2d histogram"
	    }
	    foreach {weight_expr x_expr y_expr} $args {}
	    return [hs::Ntuple_project_onhisto $nt_id $proj_id \
		    $filter_expr $weight_expr $x_expr $y_expr]
	}
        HS_3D_HISTOGRAM {
	    if {[llength $args] != 4} {
		error "Wrong number of arguments for projection onto 3d histogram"
	    }
	    foreach {weight_expr x_expr y_expr z_expr} $args {}
	    return [hs::Ntuple_project_onhisto $nt_id $proj_id \
		    $filter_expr $weight_expr $x_expr $y_expr $z_expr]
	}
	HS_NTUPLE {
	    return [hs::Ntuple_project_onntuple $nt_id $proj_id \
		    $filter_expr $args]
	}
	HS_NONE {
	    error "Item with id $proj_id doesn't exist"
	}
	default {
	    error "Item with id $proj_id is not a histogram or ntuple"
	}
    }
}

###########################################################################

proc ::hs::ntuple_c_project {args} {
    # Expected arguments: nt_id proj_id filter_expr args
    set known_switches {-include -eval -cflags}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-eval)]} {
	set utilCode $options(-eval)
    } else {
	set utilCode {}
    }
    if {[info exists options(-cflags)]} {
	set cflags $options(-cflags)
    } else {
	set cflags {}
    }
    if {[llength $arglist] < 4} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id proj_id filter_expr} $arglist break
    if {[string compare [hs::type $nt_id] "HS_NTUPLE"]} {
	error "Item with id $nt_id is not an ntuple"
    }
    switch [hs::type $proj_id] {
	HS_1D_HISTOGRAM {
	    if {[llength $arglist] != 5} {
		error "Wrong number of arguments for projection onto 1d histogram"
	    }
	    foreach {nt_id proj_id filter_expr weight_expr x_expr} $arglist {}
	    return [hs::Ntuple_c_project_onhisto $includeCode \
                        $utilCode $cflags $nt_id $proj_id \
		    $filter_expr $weight_expr $x_expr]
	}
	HS_2D_HISTOGRAM {
	    if {[llength $arglist] != 6} {
		error "Wrong number of arguments for projection onto 2d histogram"
	    }
	    foreach {nt_id proj_id filter_expr weight_expr x_expr y_expr} $arglist {}
	    return [hs::Ntuple_c_project_onhisto $includeCode \
                        $utilCode $cflags $nt_id $proj_id \
                        $filter_expr $weight_expr $x_expr $y_expr]
	}
	HS_3D_HISTOGRAM {
	    if {[llength $arglist] != 7} {
		error "Wrong number of arguments for projection onto 3d histogram"
	    }
	    foreach {nt_id proj_id filter_expr weight_expr x_expr y_expr z_expr} $arglist {}
	    return [hs::Ntuple_c_project_onhisto $includeCode \
                        $utilCode $cflags $nt_id $proj_id \
                        $filter_expr $weight_expr $x_expr $y_expr $z_expr]
	}
	HS_NTUPLE {
	    return [hs::Ntuple_c_project_onntuple $includeCode $utilCode $nt_id $proj_id \
                        $filter_expr [lrange $arglist 3 end] $cflags]
	}
	HS_NONE {
	    error "Item with id $proj_id doesn't exist"
	}
	default {
	    error "Item with id $proj_id is not a histogram or ntuple"
	}
    }
}

###########################################################################

proc ::hs::Ntuple_c_project_onntuple {includeCode \
        utilCode nt_id proj_id filter_expr varexprs cflags} {
    # Check the arguments
    set tmplist varexprs
    lappend tmplist $filter_expr
    set offending_variable [hs::Is_ntuple_c_compatible $nt_id $tmplist]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\"\
		can not be used in C code"
    }
    set nvars [hs::num_variables $proj_id]
    if {$nvars < 0} {
	if {[string equal [hs::type $proj_id] "HS_NONE"]} {
	    error "Item with id $proj_id doesn't exist"
	} else {
	    error "Item with id $proj_id is not an ntuple"
	}
    }
    if {[llength $varexprs] != $nvars} {
	error "Wrong number of expressions for projection\
		onto ntuple with id $proj_id"
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan $includeCode
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    puts $chan "float new_row_Jf9S2mOEzOJxGsUyA\[$nvars\];"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    puts $chan $utilCode
    set i 0
    foreach var $varexprs {
	puts $chan "new_row_Jf9S2mOEzOJxGsUyA\[$i\] = (float)($var);"
	incr i
    }
    puts $chan "hs_fill_ntuple($proj_id, new_row_Jf9S2mOEzOJxGsUyA);"
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    close $chan
    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib -cflags $cflags} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }
    # Scan the ntuple using the new library
    # hs::reset $proj_id
    set status [catch {hs::ntuple_so_scan $nt_id $sharedlib} errstat]
    file delete $filename $sharedlib
    if {$status} {error $errstat}
    return
}

###########################################################################

proc ::hs::Ntuple_c_project_onhisto {includeCode utilCode cflags nt_id hs_id \
	filter_expr weight_expr x_expr {y_expr {}} {z_expr {}}} {
    if {$y_expr == {}} {
	set ndim 1
    } elseif {$z_expr == {}} {
	set ndim 2
    } else {
	set ndim 3
    }
    if {[string compare [hs::type $hs_id] "HS_${ndim}D_HISTOGRAM"]} {
	error "Item with id $hs_id is not a ${ndim}d histogram"
    }
    # Try the built-in projection code first
    if {[string equal $includeCode ""] && [string equal $utilCode ""]} {
        if {[hs::Basic_ntuple_project_onhisto $nt_id $hs_id \
                $filter_expr $weight_expr $x_expr $y_expr $z_expr]} return
    }
    # Check the argument expressions
    set offending_variable [hs::Is_ntuple_c_compatible $nt_id \
	    [list $filter_expr $weight_expr $x_expr $y_expr $z_expr]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\" can not be used in C code"
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan $includeCode
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    puts $chan $utilCode
    if {$ndim == 1} {
	puts $chan "  hs_fill_1d_hist($hs_id, ($x_expr), ($weight_expr));"
    } elseif {$ndim == 2} {
	puts $chan "  hs_fill_2d_hist($hs_id, ($x_expr), ($y_expr), ($weight_expr));"
    } elseif {$ndim == 3} {
	puts $chan "  hs_fill_3d_hist($hs_id, ($x_expr), ($y_expr), ($z_expr), ($weight_expr));"
    } else {
        error "Invalid histogram dimensionality d = $ndim"
    }
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    close $chan
    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib -cflags $cflags} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }
    # Scan the ntuple using the new library
    # hs::reset $hs_id
    set status [catch {hs::ntuple_so_scan $nt_id $sharedlib} errstat]
    file delete $filename $sharedlib
    if {$status} {error $errstat}
    return
}

###########################################################################

proc ::hs::ntuple_c_stats {args} {
    # Expected arguments: nt_id filter_expr x_expr
    set known_switches {-include -eval}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-eval)]} {
	set utilCode $options(-eval)
    } else {
	set utilCode {}
    }
    if {[llength $arglist] != 3} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id filter_expr x_expr} $arglist {}

    set offending_variable [hs::Is_ntuple_c_compatible $nt_id [list $filter_expr $x_expr]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\" can not be used in C code"
    }
    if {[hs::num_entries $nt_id] == 0} {
	error "Ntuple with id $nt_id is empty"
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan "#define RANGE2SIG 0.7413011f"
    puts $chan $includeCode
    puts $chan "int arr_medirange(float *array, int n, int key,"
    puts $chan "        float *qmin, float *q25, float *qmed, float *q75, float *qmax);"
    puts $chan "int arr_stats(const float *array, int n, int key, float *mean, float *err);"
    # Global C variables. The names must be ugly to avoid collisions.
    puts $chan "static int index_fRsbU8D4l;"
    puts $chan "static float *data_BsPQn6Di7;"
    # Init function
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "index_fRsbU8D4l = 0;"
    puts $chan "if ((data_BsPQn6Di7 = (float *)malloc("
    puts $chan "    hs_num_entries(ntuple_id)*sizeof(float))) == NULL)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Scan function
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    puts $chan $utilCode
    puts $chan "data_BsPQn6Di7\[index_fRsbU8D4l++\] = (float)($x_expr);"
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Conclusion function
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "int status = TCL_ERROR;"
    puts $chan "float sum, mean, sigma, arrmin, q25, median, q75, arrmax;"
    puts $chan "Tcl_Obj *objlist\[8\];"
    puts $chan "if (index_fRsbU8D4l == 0)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"no ntuple rows pass the filter\", TCL_VOLATILE);"
    puts $chan "goto fail;"
    puts $chan "\}"
    puts $chan "if (arr_medirange(data_BsPQn6Di7, index_fRsbU8D4l, 1,"
    puts $chan "	      &arrmin, &q25, &median, &q75, &arrmax))"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"operation failed\", TCL_VOLATILE);"
    puts $chan "goto fail;"
    puts $chan "\}"
    puts $chan "arr_stats(data_BsPQn6Di7, index_fRsbU8D4l, 1, &mean, &sigma);"
    puts $chan "sum = mean*(float)index_fRsbU8D4l;"
    puts $chan "objlist\[0\] = Tcl_NewDoubleObj((double)sum);"
    puts $chan "objlist\[1\] = Tcl_NewDoubleObj((double)mean);"
    puts $chan "objlist\[2\] = Tcl_NewDoubleObj((double)sigma);"
    puts $chan "objlist\[3\] = Tcl_NewDoubleObj((double)arrmin);"
    puts $chan "objlist\[4\] = Tcl_NewDoubleObj((double)q25);"
    puts $chan "objlist\[5\] = Tcl_NewDoubleObj((double)median);"
    puts $chan "objlist\[6\] = Tcl_NewDoubleObj((double)q75);"
    puts $chan "objlist\[7\] = Tcl_NewDoubleObj((double)arrmax);"
    puts $chan "Tcl_SetObjResult(interp, Tcl_NewListObj(8, objlist));"
    puts $chan "status = TCL_OK;"
    puts $chan "fail:"
    puts $chan "if (data_BsPQn6Di7)"
    puts $chan "free(data_BsPQn6Di7);"
    puts $chan "data_BsPQn6Di7 = NULL;"
    puts $chan "index_fRsbU8D4l = 0;"
    puts $chan "return status;"
    puts $chan "\}"
    close $chan
    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }
    # Scan the ntuple using the new library
    set status [catch {hs::ntuple_so_scan $nt_id $sharedlib} answer]
    file delete $filename $sharedlib
    if {$status} {error $answer}
    return $answer
}

###########################################################################

proc ::hs::Unique_row_count_code {utilCode do_search \
                  chan nt_id filter_expr uniqueExprs} {
    # Additional definitions
    puts $chan "int ntuple_hashtable_key_length(int nkey_columns);"
    # Global C variables. The names must be ugly to avoid collisions.
    puts $chan "static Tcl_HashTable ntuple_toc_jgA5klRm;"
    puts $chan "static float *keydata_hg4mSiW0;"
    puts $chan "static int count_Gshv8NkW2l;"
    if {$do_search} {
        puts $chan "static int row_HvQ7hT8pMa8;"
        puts $chan "static Tcl_Obj *listRes_8ghGeMxIv3;"
    }
    # Init function
    set len [llength $uniqueExprs]
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id, const char *s)"
    puts $chan "\{"
    puts $chan "int keyType;"
    puts $chan "keyType = ntuple_hashtable_key_length($len);"
    puts $chan "keydata_hg4mSiW0 = (float *)calloc(keyType, sizeof(int));"
    puts $chan "if (keydata_hg4mSiW0 == NULL) \{"
    puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "Tcl_InitHashTable(&ntuple_toc_jgA5klRm, keyType);"
    puts $chan "count_Gshv8NkW2l = 0;"
    if {$do_search} {
        puts $chan "row_HvQ7hT8pMa8 = 0;"
        puts $chan "listRes_8ghGeMxIv3 = Tcl_NewListObj(0, NULL);"
        puts $chan "if (listRes_8ghGeMxIv3 == NULL)"
        puts $chan "\{"
        puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
        puts $chan "return TCL_ERROR;"
        puts $chan "\}"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Scan function
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    puts $chan "int created_Ndjw54KvEtV;"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    puts $chan $utilCode
    for {set i 0} {$i < $len} {incr i} {
        set uExpr [lindex $uniqueExprs $i]
	puts $chan "keydata_hg4mSiW0\[$i\] = (float)($uExpr);"
    }
    puts $chan "Tcl_CreateHashEntry(&ntuple_toc_jgA5klRm, (char *)keydata_hg4mSiW0, &created_Ndjw54KvEtV);"
    puts $chan "if (created_Ndjw54KvEtV)"
    puts $chan "\{"
    if {$do_search} {
        puts $chan "  Tcl_Obj *newint_Tvhc8X45M;"
        puts $chan "  newint_Tvhc8X45M = Tcl_NewIntObj(row_HvQ7hT8pMa8);"
        puts $chan "  if (newint_Tvhc8X45M == NULL)"
        puts $chan "  \{"
        puts $chan "  Tcl_SetResult(interp_gD8NeQb, \"out of memory\", TCL_VOLATILE);"
        puts $chan "  return TCL_ERROR;"
        puts $chan "  \}"
        puts $chan "  if (Tcl_ListObjAppendElement(interp_gD8NeQb, listRes_8ghGeMxIv3, newint_Tvhc8X45M) != TCL_OK)"
        puts $chan "    return TCL_ERROR;"
    }
    puts $chan "  ++count_Gshv8NkW2l;"
    puts $chan "\}"
    puts $chan "\}"
    if {$do_search} {
        puts $chan "++row_HvQ7hT8pMa8;"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Conclusion function
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id, const char *s)"
    puts $chan "\{"
    puts $chan "Tcl_DeleteHashTable(&ntuple_toc_jgA5klRm);"
    puts $chan "free(keydata_hg4mSiW0);"
    if {$do_search} {
        puts $chan "Tcl_SetObjResult(interp, listRes_8ghGeMxIv3);"
    } else {
        puts $chan "Tcl_SetObjResult(interp, Tcl_NewIntObj(count_Gshv8NkW2l));"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    return
}

###########################################################################

proc ::hs::Simple_row_count_code {do_search chan nt_id filter_expr} {
    # Global C variables. The names must be ugly to avoid collisions.
    puts $chan "static int count_fRsbU8D4l;"
    if {$do_search} {
        puts $chan "static int row_HvQ7hT8pMa8;"
        puts $chan "static Tcl_Obj *listRes_8ghGeMxIv3;"
    }
    # Init function
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "count_fRsbU8D4l = 0;"
    if {$do_search} {
        puts $chan "row_HvQ7hT8pMa8 = 0;"
        puts $chan "listRes_8ghGeMxIv3 = Tcl_NewListObj(0, NULL);"
        puts $chan "if (listRes_8ghGeMxIv3 == NULL)"
        puts $chan "\{"
        puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
        puts $chan "return TCL_ERROR;"
        puts $chan "\}"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Scan function
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    if {$do_search} {
        puts $chan "  Tcl_Obj *newint_Tvhc8X45M;"
        puts $chan "  newint_Tvhc8X45M = Tcl_NewIntObj(row_HvQ7hT8pMa8);"
        puts $chan "  if (newint_Tvhc8X45M == NULL)"
        puts $chan "  \{"
        puts $chan "  Tcl_SetResult(interp_gD8NeQb, \"out of memory\", TCL_VOLATILE);"
        puts $chan "  return TCL_ERROR;"
        puts $chan "  \}"
        puts $chan "  if (Tcl_ListObjAppendElement(interp_gD8NeQb, listRes_8ghGeMxIv3, newint_Tvhc8X45M) != TCL_OK)"
        puts $chan "    return TCL_ERROR;"
    }
    puts $chan "  ++count_fRsbU8D4l;"
    puts $chan "\}"
    if {$do_search} {
        puts $chan "++row_HvQ7hT8pMa8;"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Conclusion function
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    if {$do_search} {
        puts $chan "Tcl_SetObjResult(interp, listRes_8ghGeMxIv3);"
    } else {
        puts $chan "Tcl_SetObjResult(interp, Tcl_NewIntObj(count_fRsbU8D4l));"
    }
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    return
}

###########################################################################

proc ::hs::ntuple_c_search {args} {
    eval hs::Ntuple_c_search_or_count 1 $args
}

###########################################################################

proc ::hs::ntuple_c_count {args} {
    eval hs::Ntuple_c_search_or_count 0 $args
}

###########################################################################

proc ::hs::Ntuple_c_search_or_count {do_search args} {
    # Expected arguments: nt_id filter_expr
    set known_switches {-include -unique -eval}
    if {$do_search} {
        lappend known_switches "-reverse"
    }
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-unique)]} {
	set uniqueExprs [lsort -unique $options(-unique)]
    } else {
	set uniqueExprs {}
    }
    if {[info exists options(-eval)]} {
        set utilCode $options(-eval)
    } else {
        set utilCode {}
    }
    if {[info exists options(-reverse)]} {
        set reverse_scan $options(-reverse)
        if {![string is boolean -strict $reverse_scan]} {
            error "expected a boolean value for the\
                   \"-reverse\" option, got \"$reverse_scan\""
        }
    } else {
        set reverse_scan 0
    }
    if {[llength $arglist] != 2} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id filter_expr} $arglist {}

    # Check if we can do simple count using built-in code
    if {!$do_search} {
        if {[string equal $includeCode ""] && \
                [string equal $uniqueExprs ""]} {
            set count [hs::Basic_ntuple_count $nt_id $filter_expr]
            if {$count >= 0} {
                return $count
            }
        }
    }

    set offending_variable [hs::Is_ntuple_c_compatible $nt_id [list $filter_expr]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\" can not be used in C code"
    }
    if {[hs::num_entries $nt_id] == 0} {
	return 0
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan $includeCode

    # Write the rest of the code
    if {[llength $uniqueExprs] > 0} {
	hs::Unique_row_count_code $utilCode $do_search $chan $nt_id \
            $filter_expr $uniqueExprs
    } else {
	hs::Simple_row_count_code $do_search $chan $nt_id $filter_expr
    }
    close $chan

    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }

    # Scan the ntuple using the new library
    set status [catch {hs::ntuple_so_scan -reverse $reverse_scan \
                           $nt_id $sharedlib} answer]
    file delete $filename $sharedlib
    if {$status} {error $answer}

    # Check if we need to renumber the rows
    if {$do_search && $reverse_scan} {
        # Row numbers will be returned in the reverse order
        set nentries [hs::num_entries $nt_id]
        incr nentries -1
        set real_rows {}
        foreach row $answer {
            lappend real_rows [expr {$nentries - $row}]
        }
        set answer [lsort -integer $real_rows]
    }
    return $answer
}

###########################################################################

proc ::hs::adaptive_c_project {args} {
    # Expected arguments: nt_id uid title category filter_expr
    #                     weight_expr x_expr {width_mult 1.0}
    set known_switches {-include -eval -cflags}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-eval)]} {
	set utilCode $options(-eval)
    } else {
	set utilCode {}
    }
    if {[info exists options(-cflags)]} {
	set cflags $options(-cflags)
    } else {
	set cflags {}
    }
    if {[llength $arglist] == 7} {
	foreach {nt_id uid title category filter_expr weight_expr x_expr} $arglist {}
	set width_mult 1.0
    } elseif {[llength $arglist] == 8} {
	foreach {nt_id uid title category filter_expr weight_expr x_expr width_mult} $arglist {}
    } else {
	error "wrong # of arguments or invalid option"
    }

    if {![string is integer -strict $uid]} {
	error "Expected an integer but got \"$uid\""
    }
    if {![string is double -strict $width_mult]} {
	error "Expected a double but got \"$width_mult\""
    }
    if {$width_mult <= 0.0} {
	error "Bin width multiplier must be positive"
    }
    set offending_variable [hs::Is_ntuple_c_compatible $nt_id \
	    [list $filter_expr $weight_expr $x_expr]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $nt_id column named \"$offending_variable\"\
		can not be used in C code"
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan "#define RANGE2SIG 0.7413011f"
    puts $chan "struct weighted_point"
    puts $chan "\{"
    puts $chan "float x;"
    puts $chan "float w;"
    puts $chan "\};"
    puts $chan $includeCode
    puts $chan "int arr_medirange_weighted(struct weighted_point *array, int n,"
    puts $chan "	float *qmin, float *q25, float *qmed, float *q75, float *qmax);"
    puts $chan "int arr_medirange(float *array, int n, int key,"
    puts $chan "        float *qmin, float *q25, float *qmed, float *q75, float *qmax);"
    # Global C variables. The names must be ugly to avoid collisions.
    puts $chan "static int index_fRsbU8D4l;"
    set unit_weight [hs::C_weight_is_unit $weight_expr]
    if {$unit_weight} {
	set datatype "float"
    } else {
	set datatype "struct weighted_point"
    }
    puts $chan "static $datatype *data_BsPQn6Di7;"
    # Init function
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "int nrows;"
    puts $chan "index_fRsbU8D4l = 0;"
    puts $chan "data_BsPQn6Di7 = NULL;"
    puts $chan "nrows = hs_num_entries(ntuple_id);"
    puts $chan "if (nrows > 0)"
    puts $chan "\{"
    puts $chan "if ((data_BsPQn6Di7 = ($datatype *)malloc(nrows*sizeof($datatype))) == NULL)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"out of memory\", TCL_VOLATILE);"
    puts $chan "return TCL_ERROR;"
    puts $chan "\}"
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Scan function
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp_gD8NeQb,\
	    const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $nt_id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "if ($filter_expr)"
    puts $chan "\{"
    puts $chan $utilCode
    if {$unit_weight} {
	puts $chan "data_BsPQn6Di7\[index_fRsbU8D4l++\] = (float)($x_expr);"
    } else {
	puts $chan "data_BsPQn6Di7\[index_fRsbU8D4l\].x = (float)($x_expr);"
	puts $chan "data_BsPQn6Di7\[index_fRsbU8D4l\].w = (float)($weight_expr);"
	puts $chan "if (data_BsPQn6Di7\[index_fRsbU8D4l\].w < 0.f)"
	puts $chan "\{"
	puts $chan "if (data_BsPQn6Di7)"
	puts $chan "\{"
	puts $chan "free(data_BsPQn6Di7);"
	puts $chan "data_BsPQn6Di7 = NULL;"
	puts $chan "\}"
	puts $chan "index_fRsbU8D4l = 0;"
	puts $chan "Tcl_SetResult(interp_gD8NeQb, \"encountered negative weight\", TCL_VOLATILE);"
	puts $chan "return TCL_ERROR;"
	puts $chan "\}"
	puts $chan "++index_fRsbU8D4l;"
    }
    puts $chan "\}"
    puts $chan "return TCL_OK;"
    puts $chan "\}"
    # Conclusion function
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id, const char *dsfli_LTi79_dfAsw)"
    puts $chan "\{"
    puts $chan "int i, hs_id = -1, status = TCL_ERROR, new_bins = 50;"
    puts $chan "float xmin = 0.f, xmax = 1.f;"
    puts $chan "float binwidth = 0.02f;"
    puts $chan "float arrmin, q25, median, q75, arrmax;"
    puts $chan "if (index_fRsbU8D4l > 0)"
    puts $chan "\{"
    if {$unit_weight} {
	puts $chan "arr_medirange(data_BsPQn6Di7, index_fRsbU8D4l, 1,"
	puts $chan "         &arrmin, &q25, &median, &q75, &arrmax);"
    } else {
	puts $chan "arr_medirange_weighted(data_BsPQn6Di7, index_fRsbU8D4l,"
	puts $chan "                &arrmin, &q25, &median, &q75, &arrmax);"
    }
    puts $chan "if (arrmin == 0.f)"
    puts $chan "    xmin = 0.f;"
    puts $chan "else"
    puts $chan "    xmin = arrmin - 0.05f*(arrmax - arrmin);"
    puts $chan "xmax = arrmax + 0.05f*(arrmax - arrmin);"
    puts $chan "if (xmax == xmin)"
    puts $chan "    xmax += 0.1f;"
    puts $chan "binwidth = 3.49083f*(q75-q25)*RANGE2SIG*(float)pow((double)index_fRsbU8D4l,-1.0/3.0);"
    puts $chan "binwidth *= ${width_mult};"
    puts $chan "if (binwidth > 0.f)"
    puts $chan "    new_bins = (int)((xmax - xmin)/binwidth)+1;"
    puts $chan "if (new_bins < 0 || new_bins > 10000)"
    puts $chan "    new_bins = 10000;"
    puts $chan "\}"
    puts $chan "hs_id = hs_create_1d_hist($uid, \"$title\", \"$category\", \"$x_expr\","
    puts $chan "		      \"Frequency\", new_bins, xmin, xmax);"
    puts $chan "if (hs_id <= 0)"
    puts $chan "\{"
    puts $chan "Tcl_SetResult(interp, \"histogram creation failed\", TCL_VOLATILE);"
    puts $chan "goto fail;"
    puts $chan "\}"
    puts $chan "for (i=0; i<index_fRsbU8D4l; ++i)"
    if {$unit_weight} {
	puts $chan "hs_fill_1d_hist(hs_id, data_BsPQn6Di7\[i\], 1.f);"
    } else {
	puts $chan "hs_fill_1d_hist(hs_id, data_BsPQn6Di7\[i\].x, data_BsPQn6Di7\[i\].w);"
    }
    puts $chan ""
    puts $chan "Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_id));"
    puts $chan "status = TCL_OK;"
    puts $chan "fail:"
    puts $chan "if (status != TCL_OK && hs_id > 0)"
    puts $chan "hs_delete(hs_id);"
    puts $chan "if (data_BsPQn6Di7)"
    puts $chan "free(data_BsPQn6Di7);"
    puts $chan "data_BsPQn6Di7 = NULL;"
    puts $chan "index_fRsbU8D4l = 0;"
    puts $chan "return status;"
    puts $chan "\}"
    close $chan
    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib -cflags $cflags} errstat]} {
	file delete $sharedlib
	error "Failed to compile the C expressions.\
		Diagnostics:\n$errstat"
    }
    # Scan the ntuple using the new library
    set status [catch {hs::ntuple_so_scan $nt_id $sharedlib} new_id]
    file delete $filename $sharedlib
    if {$status} {error $new_id}
    return $new_id
}

###########################################################################

proc ::hs::ntuple_c_filter {args} {
    # Expected arguments: id uid title category filter_expr
    set known_switches {-include}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[llength $arglist] != 5} {
	error "wrong # of arguments or invalid option"
    }
    foreach {id uid title category filter_expr} $arglist {}

    if {![string is integer $uid]} {
	error "Expected an integer but got \"$uid\""
    }
    set offending_variable [hs::Is_ntuple_c_compatible $id [list $filter_expr]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $id column named \"$offending_variable\" can not be used in C code"
    }
    # Create a temporary file and write the code into it
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan $includeCode
    puts $chan "int hs_ntuple_filter_function(const float *row_dat_T3Ap9BxcThAE)"
    puts $chan "\{"
    hs::Ntuple_pack_code $id $chan row_dat_T3Ap9BxcThAE 0
    puts $chan "return ($filter_expr);"
    puts $chan "\}"
    close $chan
    # Compile the code
    variable Sharedlib_suffix
    foreach {sharedlib chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
    close $chan
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
	file delete $sharedlib
	error "Failed to compile the filter expression.\
		Diagnostics:\n$errstat"
    }
    # Filter the Ntuple using the new library
    set status [catch {hs::ntuple_so_filter $id $uid\
	    $title $category $sharedlib} new_id]
    file delete $filename $sharedlib
    if {$status} {error $new_id}
    return $new_id
}

###########################################################################

proc ::hs::ntuple_filter {id_kmJGk89kb3 uid title categ f_ex_JdOnop9asSOU} {
    #
    # Almost all variables in this proc must have obscure names so that
    # they do not interfere with variable names in the ntuple
    # 
    if {[string compare [hs::type $id_kmJGk89kb3] "HS_NTUPLE"]} {
	error "Item with id $id_kmJGk89kb3 is not an ntuple"
    }
    if {[info complete $f_ex_JdOnop9asSOU] == 0} {
	error "filter expression is not complete"
    }
    set Var8758Jjtkr2 [::hs::ntuple_variable_list $id_kmJGk89kb3]
    set New_id_JY76gHGi9 [hs::create_ntuple $uid $title $categ $Var8758Jjtkr2]
    set Num_jf57HMhi0sqm [hs::num_entries $id_kmJGk89kb3]
    set nvar_vJG68_kJog65 [hs::num_variables $id_kmJGk89kb3]
    global ::errorInfo
    if {[catch {
	for {set i_JGT5bk_k6iO 0} {$i_JGT5bk_k6iO < $Num_jf57HMhi0sqm}\
		{incr i_JGT5bk_k6iO} {
	    set row_kJMghlj_z5 [hs::row_contents $id_kmJGk89kb3 $i_JGT5bk_k6iO]
	    binary scan $row_kJMghlj_z5 f$nvar_vJG68_kJog65 jklLIl9oL_IYRj47
	    foreach $Var8758Jjtkr2 $jklLIl9oL_IYRj47 {}
	    if {[eval $f_ex_JdOnop9asSOU]} {
		hs::fill_ntuple $New_id_JY76gHGi9 $row_kJMghlj_z5
	    }
	}
    } ermess]} {
	set savedInfo $::errorInfo
	hs::delete $New_id_JY76gHGi9
	error $ermess $savedInfo
    }
    return $New_id_JY76gHGi9
}

###########################################################################

proc ::hs::item_info {{item_list some_default_UbTRh87kS}} {
    if {[string equal $item_list "some_default_UbTRh87kS"]} {
	# Build a complete list of Histo-Scope items defined so far
	set item_list [hs::list_items "" "..." 1]
    }
    # Check the input list for correctness. Find out maximum length
    # of various fields for nice formatting.
    set good_ids {}
    set malformed_ids {}
    set notfound_ids {}
    set lenid 2
    set lentype 4
    set lenuid 3
    set lencateg 8
    foreach id $item_list {
	if {[catch {hs::type $id} type]} {
	    lappend malformed_ids $id
	} else {
	    if {[string compare $type "HS_NONE"]} {
		lappend good_ids $id
		if {[string length $id] > $lenid} {
		    set lenid [string length $id]
		}
		if {[string length $type] > $lentype} {
		    set lentype [string length $type]
		}
		if {[string length [hs::uid $id]] > $lenuid} {
		    set lenuid [string length [hs::uid $id]]
		}
		if {[string length [hs::category $id]] > $lencateg} {
		    set lencateg [string length [hs::category $id]]
		}
	    } else {
		lappend notfound_ids $id
	    }
	}
    }
    # Info for the items which exist
    if {[llength $good_ids] > 0} {
	set lentype [expr {$lentype - 3}]
	if {$lentype < 4} {
	    set lentype 4
	}
	set formatstring "%-${lenid}s  %-${lentype}s \
		%-${lenuid}s  %-${lencateg}s"
	puts "[format $formatstring Id Type Uid Category]  Title"
	puts "[format $formatstring -- ---- --- --------]  -----"
	set formatstring "%-${lenid}d  %-${lentype}s \
		%-${lenuid}d  %-${lencateg}s"
	foreach id $good_ids {
	    set type [string range [hs::type $id] 3 end]
	    puts -nonewline [format $formatstring $id $type \
		    [hs::uid $id] [hs::category $id]]
	    puts "  [hs::title $id]"
	}
    }
    # Warn user that some items are not found
    if {[llength $notfound_ids] > 0} {
	puts ""
	foreach id $notfound_ids {
	    puts "Item with id $id does not exist"
	}
    }
    # Warn user about incorrect input
    if {[llength $malformed_ids] > 0} {
	puts ""
	foreach id $malformed_ids {
	    puts "\"$id\" is not a well-formed Histo-Scope id"
	}
    }    
    return
}

#########################################################################

proc ::hs::help {{command {}}} {
    if {$command == {}} {
	puts ""
	puts "Type \"hs::help command_name\" for help on a particular hs command."
        puts "You don't have to specify the namespace for the command and you can"
        puts "use wildcards. Type \"hs::help help\" for more information about"
        puts "using help."
	puts ""
	puts "The list of commands can be obtained by using Tcl command \"info\"."
	puts "For example, to see all command names defined in the namespace hs"
        puts "enter \"info commands hs::*\". To list command names which contain"
        puts "the string \"create\" use \"info commands hs::*create*\", etc."
	puts ""
    } else {
	# Check that the command exists in the namespace hs
	set qualified_name "::hs::[namespace tail $command]"
	set all_commands [info commands $qualified_name]
	# Remove all commands which start with a capital letter after ::hs::
	# These commands should not be used outside the extension code itself.
	set cominfo {}
	foreach name $all_commands {
	    set firstchar [string index $name 6]
	    if {[string equal $firstchar [string tolower $firstchar]]} {
		lappend cominfo $name
	    }
	}
	set numqual [llength $cominfo]
	if {$numqual == 0} {
	    if {[llength $all_commands] == 1 && \
		    ![string match {*[\*\?\[]*} $qualified_name]} {
		# This is an exact match without wildcards
		error "\nhs::[namespace tail $qualified_name] is an internal\
			hs extension command. Applications\nshould\
			never use it directly. No description exists.\n"
	    } else {
		error "name \"hs::[namespace tail $command]\" doesn't match anything"
	    }
	} elseif {$numqual > 1} {
	    error "ambiguous name; matches $cominfo"
	} else {
	    set library [file join [info library] hs[package require hs]]
	    if {[catch {hs::Lookup_command_info $cominfo \
		    [file join $library hs_manual.txt]} message]} {
		if {[string compare -length 15 \
			"can't open file" $message] == 0} {
		    # Oops, no help file
		    error "Hs manual text not found. Please check your installation."
		} elseif {[string compare -length 20 \
			"can't find description" $message] == 0} {
		    # Most probably, the user requested help
		    # for some internal undocumented command
		    error "\nhs::[namespace tail $cominfo] is an internal\
			    hs extension command. Applications\nshould\
			    never use it directly. No description exists.\n"
		} else {
		    error $message
		}
	    } else {
		puts -nonewline $message
		flush stdout
	    }
	}
    }
    return
}

#########################################################################

proc ::hs::Help_check {} {
    # Find the manual
    set library [file join [info library] hs[package require hs]]
    set manfile [file join $library hs_manual.txt]
    if {![file readable $manfile]} {
	puts "File $manfile not found (or unreadable)"
	return
    }

    # Go through the manual and check the line length
    set maxlen 79
    set chan [open $manfile r]
    set nline 0
    set longlines {}
    while {[gets $chan line] >= 0} {
	incr nline
	if {[string length $line] > $maxlen} {
	    lappend longlines $nline
	}
    }
    close $chan

    # Find commands which have no manual
    set no_manual {}
    set command_counter 0
    foreach command [info commands {::hs::[a-z123]*}] {
	if {[catch {hs::help $command}]} {
	    lappend no_manual $command
	}
	incr command_counter
    }

    # Report
    puts "Hs extension has $command_counter commands in the tcl API."
    if {[llength $no_manual] > 0} {
	puts "The following hs commands are not covered by the online help facility:"
	puts "[join $no_manual {, }]."
    } else {
	puts "All hs extension commands are covered by the online help facility."
    }
    if {[llength $longlines] > 0} {
	puts "The following lines are too long: $longlines."
    } else {
	puts "No long lines found."
    }
    return
}

#########################################################################

proc ::hs::Print_manual {fname} {
    set library [file join [info library] hs[package require hs]]
    set filename $library/$fname
    if {[catch {open $filename r} channel]} {
	error "Help file \"$filename\" not found. Please check your installation."
    }
    puts ""
    puts [read $channel [file size $filename]]
    close $channel
    return
}

#########################################################################

proc ::hs::Modify_overlay_style {array name style} {
    # This is an internal hs procedure and should not be called
    # directly by an application
    variable Overlay_count
    if {[info exists Overlay_count($name)] == 0} {
	error "overlay \"$name\" does not exist"
    }

    # Copy the data structures
    upvar $array Overlay_tmp
    variable Overlay_info
    array set Overlay_tmp [array get Overlay_info $name,*]

    # Initialize some internal variables
    variable Ntuple_plot_info
    set colorlist1 {red green3 blue1 magenta cyan brown\
	    DarkOrange LightGoldenrod4 blue4 green cyan4 yellow\
	    DarkViolet CornflowerBlue green4 salmon goldenrod}

    # The styles processed in the following switch statement 
    # must coincide with the list of known styles defined
    # in the hs::overlay procedure
    switch -- $style {
	user {
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    foreach option {line color fill fillcolor marker markersize} {
			set option_st ${option}_st
			set Ntuple_plot_info($id,$option_st) $Ntuple_plot_info($id,$option)
		    }
		}
	    }
	}
	plain {
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) 1
		    set Ntuple_plot_info($id,color_st) black
		    set Ntuple_plot_info($id,fill_st) none
		    set Ntuple_plot_info($id,fillcolor_st) black
		    set Ntuple_plot_info($id,marker_st) 0
		    set Ntuple_plot_info($id,markersize_st) 0
		} else {
		    set Overlay_tmp($name,$i,line) 1
		    set Overlay_tmp($name,$i,color) black
		    set Overlay_tmp($name,$i,fill) none
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	color1 {
	    set len [llength $colorlist1]
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {$i < $len} {
		    set thiscolor [lindex $colorlist1 $i]
		} else {
		    set thiscolor black
		}
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) 1
		    set Ntuple_plot_info($id,color_st) $thiscolor
		    set Ntuple_plot_info($id,fill_st) none
		    set Ntuple_plot_info($id,fillcolor_st) black
		    set Ntuple_plot_info($id,marker_st) 7
		    set Ntuple_plot_info($id,markersize_st) 2
		} else {
		    set Overlay_tmp($name,$i,line) 1
		    set Overlay_tmp($name,$i,color) $thiscolor
		    set Overlay_tmp($name,$i,fill) none
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	errorcontour {
	    set len [llength $colorlist1]
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {$i < $len} {
		    set thiscolor [lindex $colorlist1 $i]
		} else {
		    set thiscolor black
		}
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,color_st) $thiscolor
		    set Ntuple_plot_info($id,fill_st) none
		    set Ntuple_plot_info($id,fillcolor_st) black
		    if {$i == 0} {
			set Ntuple_plot_info($id,line_st) 0
			set Ntuple_plot_info($id,marker_st) 7
			set Ntuple_plot_info($id,markersize_st) 2
		    } else {
			set Ntuple_plot_info($id,line_st) 1
			set Ntuple_plot_info($id,marker_st) 0
			set Ntuple_plot_info($id,markersize_st) 0
		    }
		} else {
		    set Overlay_tmp($name,$i,line) 1
		    set Overlay_tmp($name,$i,color) $thiscolor
		    set Overlay_tmp($name,$i,fill) none
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	rainbow {
	    set len [llength $colorlist1]
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {$i < $len} {
		    set thiscolor [lindex $colorlist1 $i]
		    set thisfill solid
		} else {
		    set thiscolor black
		    set thisfill none
		}
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) 1
		    set Ntuple_plot_info($id,color_st) $thiscolor
		    set Ntuple_plot_info($id,fill_st) $thisfill
		    set Ntuple_plot_info($id,fillcolor_st) $thiscolor
		    set Ntuple_plot_info($id,marker_st) 7
		    set Ntuple_plot_info($id,markersize_st) 2
		} else {
		    set Overlay_tmp($name,$i,line) 1
		    set Overlay_tmp($name,$i,color) $thiscolor
		    set Overlay_tmp($name,$i,fill) $thisfill
		    set Overlay_tmp($name,$i,fillcolor) $thiscolor
		}
	    }
	}
	quilt {
	    variable Histogram_fill_styles
	    set len [expr [llength $Histogram_fill_styles] - 1]
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {$i < $len} {
		    set thisfill [lindex $Histogram_fill_styles [expr {$i + 1}]]
		} else {
		    set thisfill none
		}
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) 1
		    set Ntuple_plot_info($id,color_st) black
		    set Ntuple_plot_info($id,fill_st) $thisfill
		    set Ntuple_plot_info($id,fillcolor_st) black
		    set Ntuple_plot_info($id,marker_st) 7
		    set Ntuple_plot_info($id,markersize_st) 2
		} else {
		    set Overlay_tmp($name,$i,line) 1
		    set Overlay_tmp($name,$i,color) black
		    set Overlay_tmp($name,$i,fill) $thisfill
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	histo {
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) [expr {$i + 1}]
		    set Ntuple_plot_info($id,color_st) black
		    set Ntuple_plot_info($id,fill_st) none
		    set Ntuple_plot_info($id,fillcolor_st) black
		    set Ntuple_plot_info($id,marker_st) 0
		    set Ntuple_plot_info($id,markersize_st) 0
		} else {
		    set Overlay_tmp($name,$i,line) [expr {$i + 1}]
		    set Overlay_tmp($name,$i,color) black
		    set Overlay_tmp($name,$i,fill) none
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	histocolor {
	    set len [llength $colorlist1]
	    for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		if {$i < $len} {
		    set thiscolor [lindex $colorlist1 $i]
		} else {
		    set thiscolor black
		}
		if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
		    set id $Overlay_tmp($name,$i,id)
		    set Ntuple_plot_info($id,line_st) [expr {$i + 1}]
		    set Ntuple_plot_info($id,color_st) $thiscolor
		    set Ntuple_plot_info($id,fill_st) none 
		    set Ntuple_plot_info($id,fillcolor_st) black
		    set Ntuple_plot_info($id,marker_st) 0 
		    set Ntuple_plot_info($id,markersize_st) 0
		} else {
		    set Overlay_tmp($name,$i,line) [expr {$i + 1}]
		    set Overlay_tmp($name,$i,color) $thiscolor
		    set Overlay_tmp($name,$i,fill) none
		    set Overlay_tmp($name,$i,fillcolor) black
		}
	    }
	}
	default {
	    error "Unknown overlay style \"$style\".\
		    This is a bug. Please report."
	}
    }
    return
}

############################################################################

proc ::hs::Multiplot_destroy {mname} {
    variable Multiplot_info
    if {![info exists Multiplot_info($mname,nx)]} {
	# No such multiplot
	return
    }
    for {set x 0} {$x < $Multiplot_info($mname,nx)} {incr x} {
	for {set y 0} {$y < $Multiplot_info($mname,ny)} {incr y} {
	    if {[info exists Multiplot_info($mname,$x,$y,over)]} {
		catch {hs::Overlay_destroy $Multiplot_info($mname,$x,$y,over)}
	    }
	}
    }
    array unset Multiplot_info $mname,*
    return
}

#########################################################################

proc ::hs::multiplot {mname args} {

    if {[string equal $mname ""]} {
	error "multiplot name can not be just an empty string"
    }
    if {[regexp {[\*\?,\[\]]} $mname]} {
	error "multiplot name can not contain special characters *?,\[\]"
    }

    variable Multiplot_info
    if {[info exists Multiplot_info($mname,nx)] == 0} {
	set Multiplot_info($mname,nx) 0
	set Multiplot_info($mname,ny) 0
        set Multiplot_info($mname,hidelegend) ""
        set Multiplot_info($mname,xfont) ""
        set Multiplot_info($mname,psfont) ""
    }
    variable Overlay_count

    # Parse the list of options
    set keys {-title -window -geometry -legend -font\
                  add getconfig getcell show clear}
    hs::Parse_option_sequence $args $keys commands

    if {$commands(getcell,count) > 0} {
	if {$commands(getconfig,count) > 0} {
	    error "options \"getconfig\" and \"getcell\" are mutually exclusive"
	}
	if {[llength $commands(getcell,0)] == 1} {
	    foreach {getnx getny} [hs::Multiplot_parse_cell_id $commands(getcell,0)] {}
	} elseif {[llength $commands(getcell,0)] == 0} {
	    error "missing cell identifier for the \"getcell\" option"
	} else {
	    error "invalid cell identifier \"$commands(getcell,0)\""
	}
    }

    # Check if we have the geometry
    if {$commands(-geometry,count) == 1} {
	if {[llength $commands(-geometry,0)] == 1} {
	    set Multiplot_info($mname,geometry) \
		    [hs::Parse_geometry_option $commands(-geometry,0)]
	} elseif {[llength $commands(-geometry,0)] == 0} {
	    error "missing value for the \"-geometry\" option"
	} else {
	    error "invalid window geometry \"$commands(-geometry,0)\""
	}
    } elseif {$commands(-geometry,count) > 1} {
	error "Bad syntax: more than one -geometry keyword"
    }

    # Check if we have the window title
    if {$commands(-title,count) == 1} {
	if {[llength $commands(-title,0)] == 1} {
	    set title [lindex $commands(-title,0) 0]
	} else {
	    set title $commands(-title,0)
	}
	set title [string trimleft $title]
	if {![string equal "" $title]} {
	    set Multiplot_info($mname,title) $title
	}
    } elseif {$commands(-title,count) > 1} {
	error "Bad syntax: more than one -title keyword"
    }

    # Check if we have the window name
    if {$commands(-window,count) == 1} {
	if {[llength $commands(-window,0)] == 1} {
	    set winname [lindex $commands(-window,0) 0]
	} elseif {[llength $commands(-window,0)] == 0} {
	    error "missing value for the \"-window\" option"
	} else {
	    set winname $commands(-window,0)
	}
	set winname [string trimleft $winname]
	hs::Check_window_name $winname
	set Multiplot_info($mname,window) $winname
    } elseif {$commands(-window,count) > 1} {
	error "Bad syntax: more than one -window keyword"
    }

    # Check if we have the overall legend mode
    if {$commands(-legend,count) == 1} {
        set len [llength $commands(-legend,0)]
        if {$len == 0} {
	    error "missing value for the \"-legend\" option"
        } elseif {$len == 1} {
            set value [lindex $commands(-legend,0) 0]
        } else {
            set value $commands(-legend,0)
        }
        if {[string equal $value ""]} {
            set Multiplot_info($mname,hidelegend) ""
        } elseif {[string is boolean -strict $value]} {
            if {$value} {
                set Multiplot_info($mname,hidelegend) 0
            } else {
                set Multiplot_info($mname,hidelegend) 1
            }
        } else {
	    error "invalid \"-legend\" option value \"$value\""
        }
    } elseif {$commands(-legend,count) > 1} {
        error "Bad syntax: more than one -legend keyword"
    }

    # Check if we have the overall font option
    if {$commands(-font,count) == 1} {
	set len [llength $commands(-font,0)]
	if {$len == 0} {
	    error "missing value for the \"-font\" option"
	} elseif {$len == 1} {
	    set fontname [lindex $commands(-font,0) 0]
	} else {
	    set fontname $commands(-font,0)
	}
	if {[string equal $fontname ""]} {
	    set Multiplot_info($mname,xfont) ""
	    set Multiplot_info($mname,psfont) ""
	} else {
	    set Multiplot_info($mname,xfont) [hs::Generate_xlfd $fontname]
	    set Multiplot_info($mname,psfont) [hs::Postscript_font $fontname]
	}
    } elseif {$commands(-font,count) > 1} {
        error "Bad syntax: more than one -font keyword"
    }

    # Process "add" commands
    global ::errorInfo
    for {set i 0} {$i < $commands(add,count)} {incr i} {
	if {[llength $commands(add,$i)] < 2} {
	    error "not enough arguments for the \"add\" option"
	} else {
	    foreach {name_or_id position} $commands(add,$i) break
	    foreach {x y} [hs::Multiplot_parse_cell_id $position] {}
	    # Check if an overlay already exists in this cell. If not,
	    # create a new overlay. Pass the add command to the overlay.
	    if {[info exists Multiplot_info($mname,$x,$y,over)]} {
		set new_overlay 0
		set overname $Multiplot_info($mname,$x,$y,over)
	    } else {
		set new_overlay 1
		set overname "hS_oVeR_MuLT_${mname}_${x}_${y}"
		set Multiplot_info($mname,$x,$y,over) $overname
	    }
	    if {[catch {eval hs::overlay [list $overname] \
		    add [list $name_or_id] \
		    [lrange $commands(add,$i) 2 end]} errmess]} {
		set tmpInfo $::errorInfo
		if {$new_overlay} {
		    catch {hs::Overlay_destroy $overname}
		    catch {unset Multiplot_info($mname,$x,$y,over)}
		}
		error $errmess $tmpInfo
	    }
	    # Looks like "add" was successful
	    if {$x >= $Multiplot_info($mname,nx)} {
		set Multiplot_info($mname,nx) [expr {$x + 1}]
	    }
	    if {$y >= $Multiplot_info($mname,ny)} {
		set Multiplot_info($mname,ny) [expr {$y + 1}]
	    }
	}
    }

    # Check "getconfig" or "show"
    if {$commands(getconfig,count) > 0 || $commands(show,count) > 0} {
	if {$Multiplot_info($mname,nx) == 0} {
	    error "Multiplot \"$mname\" contains no plots"
	}
	set config_string "MultiPlotWindow\n"
	if {[info exists Multiplot_info($mname,title)]} {
	    append config_string "\
		    WindowTitle $Multiplot_info($mname,title)\n"
	}
	if {[info exists Multiplot_info($mname,window)]} {
	    set windo_name $Multiplot_info($mname,window)
	    append config_string "\
		    WindowName $windo_name\n"
	} else {
	    set windo_name ""
	}
	if {[info exists Multiplot_info($mname,geometry)]} {
	    set geometry $Multiplot_info($mname,geometry)
	} else {
	    set xlen [expr {$Multiplot_info($mname,nx) * 400}]
	    set ylen [expr {$Multiplot_info($mname,ny) * 300}]
	    set xmax 800.0
	    set ymax 600.0
	    # Fit the window into $xmax x $ymax rectangle 
            # preserving the aspect ratio
	    set xratio [expr {$xmax / $xlen}]
	    set yratio [expr {$ymax / $ylen}]
	    if {$xratio > $yratio} {
		set ratio $yratio
	    } else {
		set ratio $xratio
	    }
	    set xlen [expr {int($xlen * $ratio)}]
	    set ylen [expr {int($ylen * $ratio)}]
	    set geometry "${xlen}x${ylen}+0+0"
	}
	append config_string "\
		WindowGeometry $geometry\n"
	variable Multiplot_id
	if {[info exists Multiplot_id] == 0} {
	    set Multiplot_id 0
	}
	incr Multiplot_id
	append config_string "\
		WindowID $Multiplot_id\n\
		Rows $Multiplot_info($mname,ny)\n\
		Columns $Multiplot_info($mname,nx)\n"
	variable Overlay_info
	variable Ntuple_plot_info
	variable Overlay_id
	set limit_attributes {
	    xmin XMinLimit
	    xmax XMaxLimit
	    ymin YMinLimit
	    ymax YMaxLimit
	    zmin ZMinLimit
	    zmax ZMaxLimit
	}
	set scale_attributes {
	    xscale LogX xmin
	    yscale LogY ymin
	    zscale LogZ zmin
	}
	for {set x 0} {$x < $Multiplot_info($mname,nx)} {incr x} {
	    for {set y 0} {$y < $Multiplot_info($mname,ny)} {incr y} {
		if {[info exists Multiplot_info($mname,$x,$y,over)]} {
		    set name $Multiplot_info($mname,$x,$y,over)
		} else {
		    # Overlay is not defined for this cell
		    continue
		}
		if {[info exists Overlay_count($name)] == 0} {
		    # Looks like the overlay has been deleted
		    continue
		}
		if {$Overlay_count($name) == 0} {
		    # No plots in this overlay
		    continue
		}

		# Modify the overlay info according to the requested style
		hs::Modify_overlay_style Overlay_tmp $name $Overlay_info($name,style)

		# Figure out the number of plots in this overlay
		set plotcount 0
		set all_plain_histos 1
		set need_color_scale 0
		for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		    set id $Overlay_tmp($name,$i,id)
		    if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
			set ntuple_id $Ntuple_plot_info($id,id)
			if {![string equal [hs::type $ntuple_id] "HS_NONE"]} {
			    incr plotcount
			    set all_plain_histos 0
			    set plottype $Ntuple_plot_info($id,plottype)
			    if {[string equal $plottype "tartan"] || \
				    [string equal $plottype "cscat2"]} {
				set need_color_scale 1
			    }
			}
		    } elseif {![string equal [hs::type $id] "HS_NONE"]} {
			if {[string equal $Overlay_tmp($name,$i,header) "ColorCellPlot"]} {
			    set need_color_scale 1
			}
			incr plotcount
		    }
		}
		set last_plot_number [expr {$plotcount - 1}]

		# Check if we need to prepend the color scale definition
		set colorscale_name $Overlay_tmp($name,colorscale)
		if {$need_color_scale} {
		    if {![string equal $colorscale_name ""]} {
			variable Color_scale_data
			if {!$Color_scale_data($colorscale_name,persistent)} {
			    set tmpname [hs::Next_color_scale_name]
			    hs::Append_color_scale_def config_string \
				    $colorscale_name $tmpname
			    set colorscale_name $tmpname
			}
		    }
		}

		# Add all plots to the configuration string
		set numplot 0
		set dynamic_color [expr [string equal $Overlay_tmp($name,zmin) ""] && \
			[string equal $Overlay_tmp($name,zmax) ""]]
		for {set i 0} {$i < $Overlay_count($name)} {incr i} {
		    set id $Overlay_tmp($name,$i,id)
		    if {[string equal $Overlay_tmp($name,$i,header) "NTupleItem"]} {
			set ntuple_id $Ntuple_plot_info($id,id)
			if {[string equal [hs::type $ntuple_id] "HS_NONE"]} {
			    continue
			}
			append config_string [hs::Ntuple_plot_getconfig \
				$id $colorscale_name $dynamic_color]
		    } else {
			if {[string equal [hs::type $id] "HS_NONE"]} {
			    continue
			}
			set heading $Overlay_tmp($name,$i,header)
			append config_string "$heading\n\
				Category [hs::category $id]\n\
				UID [hs::uid $id]\n"
			if {[string equal $heading "ColorCellPlot"]} {
			    if {![string equal $colorscale_name ""]} {
				append config_string "\
					ColorScaleName $colorscale_name\n"
			    }
			    if {$dynamic_color} {
				append config_string "\
					DynamicColor\n"
			    }
			}
			if {$Overlay_tmp($name,$i,errors)} {
			    if {[hs::hist_error_status $id] > 0} {
				append config_string "\
					ShowErrorData\n"
			    }
			}
			append config_string "\
				LineStyle1 $Overlay_tmp($name,$i,line)\n\
				LineColor1 $Overlay_tmp($name,$i,color)\n\
				FillStyle1 $Overlay_tmp($name,$i,fill)\n\
				FillColor1 $Overlay_tmp($name,$i,fillcolor)\n"
		    }
		    if {$numplot == 0} {
			append config_string "\
				InMultiPlot $Multiplot_id\n\
				Row [expr $y + 1]\n\
				Column [expr $x + 1]\n"
                        if {![string equal $Multiplot_info($mname,xfont) ""]} {
			    append config_string "\
				    Font $Multiplot_info($mname,xfont)\n\
				    PSFont [lindex $Multiplot_info($mname,psfont) 0]\n\
				    PSFontSize [lindex $Multiplot_info($mname,psfont) 1]\n"
                        } elseif {![string equal $Overlay_tmp($name,xfont) ""]} {
			    append config_string "\
				    Font $Overlay_tmp($name,xfont)\n\
				    PSFont [lindex $Overlay_tmp($name,psfont) 0]\n\
				    PSFontSize [lindex $Overlay_tmp($name,psfont) 1]\n"
			}
			foreach {switchname configname} $limit_attributes {
			    if {![string equal $Overlay_tmp($name,$switchname) ""]} {
				append config_string "\
					$configname $Overlay_tmp($name,$switchname)\n"
			    }
			}
			foreach {switchname padnames} {
			    ipadx {AddToLeftMargin AddToRightMargin}
			    ipady {AddToTopMargin AddToBottomMargin}
			} {
			    foreach value $Overlay_tmp($name,$switchname) configname $padnames {
				if {$value} {
				    append config_string "\
					    $configname $value\n"
				}
			    }
			}
			if {$Overlay_count($name) > 1 ||\
				![string equal $Overlay_tmp($name,xlabel) ""] ||\
				![string equal $Overlay_tmp($name,ylabel) ""]} {
			    incr Overlay_id
			    append config_string "\
				    OverlayID $Overlay_id\n"
			    if {![string equal $Overlay_tmp($name,xlabel) ""]} {
				append config_string "\
					XLabel $Overlay_tmp($name,xlabel)\n"
			    }
			    if {![string equal $Overlay_tmp($name,ylabel) ""]} {
				append config_string "\
					YLabel $Overlay_tmp($name,ylabel)\n"
			    }
                            if {[string equal $Multiplot_info($mname,hidelegend) ""]} {
                                if {$Overlay_tmp($name,hidelegend)} {
                                    append config_string "\
					HideLegend\n"
                                }
                            } elseif {$Multiplot_info($mname,hidelegend)} {
                                append config_string "\
				    HideLegend\n"                                
                            }
			} else {
                            if {[string equal $Multiplot_info($mname,hidelegend) ""]} {
                                if {![string equal "NTupleItem" \
                                        $Overlay_tmp($name,0,header)] || \
                                        $Overlay_tmp($name,hidelegend)} {
                                    append config_string "\
					HideLegend\n"
                                }
                            } elseif {$Multiplot_info($mname,hidelegend)} {
                                append config_string "\
				    HideLegend\n"                                
                            }
			}
		    }
		    if {$numplot == $last_plot_number} {
			array unset tmp
			array set tmp $limit_attributes
			foreach {switchname configname limitname} $scale_attributes {
			    if {[string equal $Overlay_tmp($name,$switchname) "log"]} {
				append config_string "\
					$configname\n"
				if {[string equal $Overlay_tmp($name,$limitname) ""]} {
				    append config_string "\
					    $tmp($limitname) 0.0\n"
				}
			    }
			}
		    }
		    if {$numplot > 0} {
			append config_string "\
				InOverlay $Overlay_id\n"
                        if {[string equal $Multiplot_info($mname,hidelegend) ""]} {
                            if {$Overlay_tmp($name,hidelegend)} {
                                append config_string "\
				    HideLegend\n"
                            }
                        } elseif {$Multiplot_info($mname,hidelegend)} {
                            append config_string "\
				HideLegend\n"
                        }
		    }
		    incr numplot
		}
	    }
	}
    }

    # Send out the configuration string
    if {$commands(show,count) > 0} {
	hs::Remember_window_name $windo_name
	hs::load_config_string $config_string
	hs::hs_update
    }

    # Check if we have to clean up
    if {$commands(clear,count) > 0} {
	hs::Multiplot_destroy $mname
    }

    # What is our return value?
    if {$commands(getconfig,count) > 0} {
	return $config_string
    } elseif {$commands(getcell,count) > 0} {
	if {[info exists Multiplot_info($mname,$getnx,$getny,over)]} {
	    return $Multiplot_info($mname,$getnx,$getny,over)
	}
    }
    return
}

#########################################################################

proc ::hs::data_to_list {binary_string} {
    binary scan $binary_string f* varlist
    set varlist
}

#########################################################################

proc ::hs::list_to_data {list_of_numbers} {
    binary format "f[llength $list_of_numbers]" $list_of_numbers
}

#########################################################################

proc ::hs::lookup_title {args} {
    if {[llength $args] == 1} {
	set case_sensitive 1
	set pattern [lindex $args 0]
	set categ "..."
    } elseif {[llength $args] == 2} {
	set first [lindex $args 0]
	if {[string equal $first "-nocase"]} {
	    set case_sensitive 0
	    set pattern [lindex $args 1]
	    set categ "..."
	} else {
	    set case_sensitive 1
	    set pattern [lindex $args 0]
	    set categ [lindex $args 1]
	}
    } elseif {[llength $args] == 3} {
	set option [lindex $args 0]
	set pattern [lindex $args 1]
	set categ [lindex $args 2]
	if {[string equal $option "-nocase"]} {
	    set case_sensitive 0
	} elseif {[string equal [string index $option 0] "-"]} {
	    error "invalid option \"${option}\""
	} else {
	    error "wrong # of args"
	}
    } else {
	error "wrong # of args"
    }
    if {$pattern == {}} {
	return {}
    }
    set idlist {}
    if {$case_sensitive} {
	foreach id [hs::list_items "" $categ 1] {
	    if {[string match $pattern [hs::title $id]]} {
		lappend idlist $id
	    }
	}
    } else {
	foreach id [hs::list_items "" $categ 1] {
	    if {[string match -nocase $pattern [hs::title $id]]} {
		lappend idlist $id
	    }
	}
    }
    return $idlist
}

#########################################################################

proc ::hs::browse_collection {args} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }

    # Check the input list for correctness
    set item_list [lindex $args 0]
    set args_to_pass [lrange $args 1 end]
    set good_ids {}
    foreach id $item_list {
        if {[catch {hs::type $id} type]} {
            puts stderr "\"${id}\" is not a well-formed Histo-Scope id. Ignored."
        } else {
            switch $type {
                HS_1D_HISTOGRAM -
                HS_2D_HISTOGRAM {
                    lappend good_ids $id
                }
                HS_NONE {
                    puts stderr "Item with id $id doesn't exist. Ignored."
                }
                default {
                    puts stderr "Item with id $id is not a histogram. Ignored."
                }
            }
        }
    }
    if {[llength $good_ids] == 0} {
        puts stderr "No valid Histo-Scope ids provided"
        return
    }
    set tmp_ids [lsort -integer $good_ids]
    set max_id [lindex $tmp_ids end]
    set max_len [string length $max_id]
    if {$max_len < 2} {set max_len 2}

    # Create the toplevel window
    variable Hs_browse_collection_counter
    set status 1
    while {$status} {
	set topwin .hs_browse_collection_[incr Hs_browse_collection_counter]
	set status [catch {toplevel $topwin}]
    }
    wm withdraw $topwin
    set window_title "hs::browse_collection $Hs_browse_collection_counter"
    wm title $topwin $window_title

    # Sorting buttons
    frame $topwin.ceiling
    pack $topwin.ceiling -side top -fill x
    label $topwin.ceiling.id -text [format "  %${max_len}s    " ID] -relief groove
    bind $topwin.ceiling.id <ButtonPress-1> \
	    [list hs::Browse_collection_sort $topwin integer 0]
    label $topwin.ceiling.title -height 1 -text "  Title" -anchor w -relief groove
    bind $topwin.ceiling.title <ButtonPress-1> \
	    [list hs::Browse_collection_sort $topwin ascii 1]
    pack $topwin.ceiling.id -side left
    pack $topwin.ceiling.title -side left -fill x -expand 1

    # Listbox frame
    frame $topwin.top
    pack $topwin.top -side top -expand yes -fill both
    set w $topwin.top
    scrollbar $w.scroll -width 14 -command "$w.box yview"
    listbox $w.box -yscroll "$w.scroll set" -height 12 \
            -selectmode browse -selectforeground black
    pack $w.box -side left -fill both -expand 1
    pack $w.scroll -side right -fill y

    # Title search frame
    frame $topwin.sframe
    pack $topwin.sframe -fill x
    label $topwin.sframe.label_s -text " Search:"
    label $topwin.sframe.empty -text ""
    entry $topwin.sframe.entry_s -width 20 -background white \
	    -selectforeground black -selectborderwidth 0
    bind $topwin.sframe.entry_s <KeyPress-Return> \
	    "if \{\[hs::Browse_collection_find_title $topwin\]\} break"
    bind $topwin.sframe.entry_s <1> \
	    "set hs::Browse_collection_search_trigger($topwin) 1"
    grid $topwin.sframe.label_s -row 0 -column 0 -padx 3 -pady 2 -sticky e
    grid $topwin.sframe.entry_s -row 0 -column 1 -padx 2 -pady 2 -sticky ew
    grid $topwin.sframe.empty -row 0 -column 2 -pady 2 -sticky w
    grid columnconfigure $topwin.sframe 1 -weight 1

    # Command buttons
    set buttonwidth 10
    frame $topwin.bottom
    pack $topwin.bottom -fill x -padx 4 -pady 3
    button $topwin.bottom.n -text "Next" -width $buttonwidth \
	    -activebackground gray95 -activeforeground black \
	    -command "if \{\[hs::Browse_collection_move_down $topwin\]\}\
	    \{[concat hs::Browse_collection_show_selected $topwin $args_to_pass]\}"
    button $topwin.bottom.p -text "Previous" -width $buttonwidth \
	    -activebackground gray95 -activeforeground black \
	    -command "if \{\[hs::Browse_collection_move_up $topwin\]\}\
	    \{[concat hs::Browse_collection_show_selected $topwin $args_to_pass]\}"
    button $topwin.bottom.cw -text "Close Plots" -width $buttonwidth \
	    -activebackground gray95 -activeforeground black \
	    -command [list hs::Browse_collection_close_windows $topwin]
    button $topwin.bottom.e -text "Dismiss" -width $buttonwidth \
	    -activebackground gray95 -activeforeground red \
	    -command [list hs::Browse_collection_destroy $topwin]
    grid $topwin.bottom.p -column 0 -row 0 -padx 3 -pady 2 -sticky ew
    grid $topwin.bottom.n -column 1 -row 0 -padx 3 -pady 2 -sticky ew
    grid $topwin.bottom.cw -column 0 -row 1 -padx 3 -pady 2 -sticky ew
    grid $topwin.bottom.e -column 1 -row 1 -padx 3 -pady 2 -sticky ew
    grid columnconfigure $topwin.bottom 0 -weight 1
    grid columnconfigure $topwin.bottom 1 -weight 1

    # Make sure the colors of the sorting buttons are the same
    set bgcolor [$topwin.bottom.n cget -background]
    set activecolor [$topwin.bottom.n cget -activebackground]
    foreach sortbutton [list $topwin.ceiling.id $topwin.ceiling.title] {
	$sortbutton configure -background $bgcolor
	bind $sortbutton <Enter> [list $sortbutton configure -background $activecolor]
	bind $sortbutton <Leave> [list $sortbutton configure -background $bgcolor]
    }

    # Associated data structure
    variable Browse_collection_windows_list
    set Browse_collection_windows_list($topwin) {}
    variable Browse_collection_title_list
    set Browse_collection_title_list($topwin) {}
    variable Browse_collection_search_string
    set Browse_collection_search_string($topwin) ""
    variable Browse_collection_search_trigger
    set Browse_collection_search_trigger($topwin) 0
    variable Browse_collection_sort_order
    set Browse_collection_sort_order($topwin) [list -1 "increasing"]
    variable Browse_collection_format_string
    set formatString "  %${max_len}d      %s"
    set Browse_collection_format_string($topwin) $formatString

    foreach id $good_ids {
	set title [hs::title $id]
	lappend Browse_collection_title_list($topwin) [list $id $title]
        set title [format $formatString $id $title]
        $w.box insert end $title
    }

    bind $topwin.top.box <Double-1> \
	    [concat hs::Browse_collection_show_selected $topwin $args_to_pass]
    bind $topwin <KeyPress-Return> \
	    [concat hs::Browse_collection_show_selected $topwin $args_to_pass]
    bind $topwin <KeyRelease-Up> "hs::Browse_collection_move_up $topwin"
    bind $topwin <KeyRelease-Down> "hs::Browse_collection_move_down $topwin"
    bind $topwin <KeyRelease-Prior> "$topwin.top.box yview scroll -1 pages"
    bind $topwin <KeyRelease-Next> "$topwin.top.box yview scroll 1 pages"

    update idletasks
    wm deiconify $topwin
    wm protocol $topwin WM_DELETE_WINDOW \
	    [list hs::Browse_collection_destroy $topwin]
    return $topwin
}

#########################################################################

proc ::hs::Browse_collection_sort {topwin sorttype index} {
    variable Browse_collection_sort_order
    foreach {old_index old_seq} $Browse_collection_sort_order($topwin) {}
    if {$old_index == $index} {
	if {[string equal $old_seq "increasing"]} {
	    set seq "decreasing"
	} else {
	    set seq "increasing"
	}
    } else {
	set seq "increasing"
    }
    set Browse_collection_sort_order($topwin) [list $index $seq]
    variable Browse_collection_title_list
    set Browse_collection_title_list($topwin) [lsort -$seq -$sorttype\
	    -index $index $Browse_collection_title_list($topwin)]
    set lbox $topwin.top.box
    set selected_indices [$lbox curselection]
    if {[llength $selected_indices] > 0} {
	set selected_id [lindex [$lbox get [lindex $selected_indices 0]] 0]
    } else {
	set selected_id -1
    }
    $lbox selection clear 0 end
    $lbox delete 0 end
    variable Browse_collection_format_string
    set formatString $Browse_collection_format_string($topwin)
    if {$selected_id >= 0} {
	set index 0
	set selected_index -1
	foreach pair $Browse_collection_title_list($topwin) {
	    foreach {id title} $pair {}
	    $lbox insert end [format $formatString $id $title]
	    if {$id == $selected_id} {
		set selected_index $index
	    }
	    incr index
	}
	if {$selected_index >= 0} {
	    $topwin.top.box see $selected_index
	    $topwin.top.box selection set $selected_index
	    $topwin.top.box activate $selected_index
	}
    } else {
	foreach pair $Browse_collection_title_list($topwin) {
	    foreach {id title} $pair {}
	    $lbox insert end [format $formatString $id $title]
	}
    }
    return
}

#########################################################################

proc ::hs::Browse_collection_find_title {topwin} {
    variable Browse_collection_search_trigger
    variable Browse_collection_search_string
    set str [$topwin.sframe.entry_s get]
    if {[string equal $str $Browse_collection_search_string($topwin)] && \
	    $Browse_collection_search_trigger($topwin) == 0} {
	return 0
    }
    set Browse_collection_search_string($topwin) $str
    set Browse_collection_search_trigger($topwin) 0
    variable Browse_collection_title_list
    set match_found 0
    set index 0
    foreach pair $Browse_collection_title_list($topwin) {
	foreach {id title} $pair {}
	if {[string match $str $title]} {
	    set match_found 1
	    break
	}
	incr index
    }
    $topwin.top.box selection clear 0 end
    if {$match_found} {
	$topwin.top.box see $index
	$topwin.top.box selection set $index
	$topwin.top.box activate $index
    }
    return 1
}

#########################################################################

proc ::hs::Browse_collection_destroy {topwin} {
    variable Browse_collection_windows_list
    unset Browse_collection_windows_list($topwin)
    variable Browse_collection_title_list
    unset Browse_collection_title_list($topwin)
    variable Browse_collection_search_string
    unset Browse_collection_search_string($topwin)
    variable Browse_collection_search_trigger
    unset Browse_collection_search_trigger($topwin)
    variable Browse_collection_sort_order
    unset Browse_collection_sort_order($topwin)
    variable Browse_collection_format_string
    unset Browse_collection_format_string($topwin)
    destroy $topwin
}

#########################################################################

proc ::hs::Browse_collection_move_down {topwin} {
    set index [$topwin.top.box index active]
    set oldindex $index
    incr index
    set nentries [$topwin.top.box index end]
    if {$index < $nentries} {
        $topwin.top.box selection clear $oldindex
        $topwin.top.box selection set $index
        $topwin.top.box activate $index
        # Check that we are still inside the window
        foreach {t b} [$topwin.top.box yview] {}
        set botind [expr {int($nentries * $b)}]
        if {$index >= $botind} {
            $topwin.top.box yview scroll 1 pages
        }
        return 1
    } else {
        return 0
    }
}

#########################################################################

proc ::hs::Browse_collection_move_up {topwin} {
    set index [$topwin.top.box index active]
    if {$index > 0} {
        $topwin.top.box selection clear $index
	set index [expr $index - 1]
        $topwin.top.box selection set $index
        $topwin.top.box activate $index
        # Check that we are still inside the window
        foreach {t b} [$topwin.top.box yview] {}
        set topind [expr int([$topwin.top.box index end] * $t)]
        if {$index <= $topind} {
            $topwin.top.box yview scroll -1 pages
        }
        return 1
    } else {
        return 0
    }
}

#########################################################################

proc ::hs::Browse_collection_close_windows {topwin} {
    variable Browse_collection_windows_list
    foreach winname $Browse_collection_windows_list($topwin) {
	hs::close_window $winname 1
    }
    set Browse_collection_windows_list($topwin) {}
}

#########################################################################

proc ::hs::Browse_collection_show_selected {topwin args} {
    variable Browse_collection_show_selected_winnum
    set winname Browse_collection_[incr Browse_collection_show_selected_winnum]
    if {[catch {selection get} title]} {
	return
    }
    set id [lindex $title 0]
    if {[string is integer $id]} {
	if {[catch {eval hs::show_histogram $id -window $winname $args} result]} {
	    puts stderr $result
	    hs::Browse_collection_destroy $topwin
	}
	variable Browse_collection_windows_list
	lappend Browse_collection_windows_list($topwin) $winname
    }
    return
}

#########################################################################

proc ::hs::close_window {winname {nocomplain 0} {wait_till_completion 0}} {
    variable Known_window_names
    if {![info exists Known_window_names($winname)] && !$nocomplain} {
	error "Invalid window name \"$winname\""
    }
    variable Last_window_posted
    if {[string equal $Last_window_posted $winname]} {
	set Last_window_posted ""
    }
    if {[hs::Use_distinct_window_names]} {
	unset Known_window_names($winname)
    }

    # Convert nocomplain from bool into int
    if {$nocomplain} {
	set nc_i 1
    } else {
	set nc_i 0
    }
    if {![string is boolean -strict $wait_till_completion]} {
	error "Expected a boolean argument, got \"$wait_till_completion\""
    }
    set config_string "WindowAction\n\
	    WindowName ${winname}\n Action Close\n NoComplain $nc_i\n"
    if {$nocomplain} {
	catch {hs::Histoscope_command $config_string $wait_till_completion {;#}}
    } else {
	hs::Histoscope_command $config_string $wait_till_completion {;#}
    }
    return
}

#########################################################################

proc ::hs::close_all_windows {} {
    variable Known_window_names
    array unset Known_window_names
    variable Last_window_posted
    set Last_window_posted ""
    hs::load_config_string "WindowAction\n\
	    WindowName dummy\n Action CloseAll\n"
    hs::hs_update
    return
}

#########################################################################

proc ::hs::html_manual {filename} {
    set library [file join [info library] hs[package require hs]]
    set text_manual [file join $library hs_manual.txt]
    if {![file readable $text_manual]} {
	error "Hs manual text not found. Please check your installation."
    }
    set calc_manual [file join $library hs_calc.txt]
    if {![file readable $calc_manual]} {
	error "hs::calc extended manual not found. Please check your installation."
    }
    if {[hs::have_cernlib]} {
	set fit_manual [file join $library hs_fit.txt]
	if {![file readable $fit_manual]} {
	    error "Hs fitting manual not found. Please check your installation."
	}
    }

    set comlist {}
    foreach name [info commands ::hs::*] {
	set firstchar [string index $name 6]
	if {[string equal $firstchar [string tolower $firstchar]]} {
	    lappend comlist [string range $name 6 end]
	}
    }
    set comlist [lsort $comlist]
    if {[catch {open $filename "w"} chan]} {
	error "unable to write file $filename : $chan"
    }
    set title "Reference Manual for the Hs Tcl Extension v[hs::tcl_api_version]"
    if {![hs::have_cernlib]} {
	append title " (no CERNLIB)"
    }
    puts $chan "<html>\n\
            <head>\n\
            <title>${title}</title>\n\
            </head>\n\
            <body>"
    puts $chan "<center><b><H1>${title}</H1></b></center>"

    # Command groups
    set command_groups {}
    lappend command_groups "Initialization and Cleanup" {
	initialize
	histoscope*
	histo_with_config
	num_connected_scopes
	wait_num_scopes
	server_port
	load_config_file
	*_update
	*_histoscope
	complete*
    }
    lappend command_groups "Operations with One-Dimensional Histograms" {
	create_1d_hist
	fill_1d_hist
	*1d_*fill*
	*1d_*
	!*fit*
	!1d_fft
	!1d_fourier*
    }
    lappend command_groups "Fourier Transform" {
	1d_fft
	1d_fourier*
    }
    lappend command_groups "Operations with Two-Dimensional Histograms" {
	create_2d_hist
	fill_2d_hist
	*2d_*fill*
	*2d_*
	transpose_histogram
        !*project*
	!*fit*
    }
    lappend command_groups "Operations with Three-Dimensional Histograms" {
	create_3d_hist
	fill_3d_hist
	*3d_*fill*
	*3d_*
        !*project*
	!*fit*
    }
    lappend command_groups "Histogram Slices and Projections" {
        *slice*
        project*histogram
    }
    lappend command_groups "Histogram Math and Statistics" {
        divide_*
        multiply_*
	sum_*
	hist_integral
	hist_*_norm
	calc
	special_*
	stats_histogram
	adaptive_stats_histogram
        hist_function_chisq
        !sum_ntuple_columns
    }
    lappend command_groups "Miscellaneous Histogram Operations" {
        create_histogram
        fill_histogram
	*_histograms
        hist_*
        swap_data_errors
        !*random*
    }
    lappend command_groups "Operations with Ntuples" {
	create_ntuple
	fill_ntuple
	ntuple_*_fill
	*ntuple*
	num_variables
	variable_*
	column_range
	column_contents
        replace_column_contents
	row_contents
        add_filled_columns
	*column_*
	adaptive_c_project
	merge_entries
        join_entries
        unique_rows
        unbinned_ks_test
        weighted_unbinned_ks_test
	!*fit*
        !*pack_*
	!duplicate_ntuple_header
    }
    lappend command_groups "Kernel Density Estimation" {
	*kernel*
    }
    lappend command_groups "Operations with Fit Functions" {
	sharedlib*
	function*
    }
    lappend command_groups "Data Fitting" {
	*fit*
        !*fit_manual
    }
    lappend command_groups "Displaying Plots" {
	show_histogram
	overlay
	multiplot
	*_plot
	browse_collection
	close*window*
    }
    lappend command_groups "Drawing Labels, Formulas, and Primitives" {
	*draw*
	*comment*
	*latex*
	undo
	clear
    }
    lappend command_groups "Making and Importing PostScript Files" {
	generate_ps
	epsf
    }
    lappend command_groups "Color Management" {
	*color_scale
    }
    lappend command_groups "Looking up Items" {
	num_items
	id*
	list_items
	lookup_title
    }
    lappend command_groups "Finding Item Properties" {
	type
	uid
	category
	title
	item_info
	num_entries
        item_properties
    }
    lappend command_groups "Changing Item Properties" {
	change*
    }
    lappend command_groups "Item Copying, Deleting, and Resetting" {
	copy_hist
	duplicate_*
	copy_data
	delete*
	reset*
    }
    lappend command_groups "Triggers, Indicators, and Controls" {
	create_trigger
	check_trigger
	*indicator*
	*control*
    }
    lappend command_groups "Data Input and Output" {
	dir
	*read*file*
	*read*items*
	*save*
        *pack_*
	ascii_dump
    }
    lappend command_groups "Random Numbers" {
        *random*
    }
    lappend command_groups "Getting Help" {
	help
	fit_manual
	html_manual
    }
    lappend command_groups "Miscellaneous Commands" {*}

    # Split commands into groups
    foreach {group pattern_list} $command_groups {
	set groupcount($group) 0
	set match_patterns($group) {}
	set exclude_patterns($group) {}
	foreach pattern $pattern_list {
	    if {[string equal -length 1 $pattern "!"]} {
		lappend exclude_patterns($group) [string range $pattern 1 end]
	    } else {
		lappend match_patterns($group) $pattern
		set groupcommands($group,$pattern) {}
	    }
	}
    }
    foreach command $comlist {
	foreach {group pattern_list} $command_groups {
	    # Check if this command is explicitly
	    # excluded from this group
	    set exclusion_found 0
	    foreach pattern $exclude_patterns($group) {
		if {[string match $pattern $command]} {
		    set exclusion_found 1
		    break
		}
	    }
	    if {!$exclusion_found} {
		# Check if this command matches some pattern
		set match_found 0
		foreach pattern $match_patterns($group) {
		    if {[string match $pattern $command]} {
			set match_found 1
			lappend groupcommands($group,$pattern) $command
			incr groupcount($group)
			break
		    }
		}
		if {$match_found} break
	    }
	}
    }

    # Print the list of topics
    puts $chan "<P><b>Jump to topic:</b><br>"
    set group_number 0
    foreach {group pattern_list} $command_groups {
	if {$groupcount($group) > 0} {
	    incr group_number
	    puts $chan "<br><a href=\"#topic${group_number}\">${group}</a>"
	}
    }
    puts $chan "<br><a href=\"#hs::calc_Details\">hs::calc Details</a>"
    if {[hs::have_cernlib]} {
	puts $chan "<br><a href=\"#Fitting_Details\">Fitting Details</a>"
    }

    # Print the list of letters
    puts $chan "<P><b>Jump to letter:&nbsp; "
    set firstletter ""
    foreach command $comlist {
	set letter [string index $command 0]
	if {![string equal $letter $firstletter]} {
	    set firstletter $letter
	    puts $chan "<a href=\"#hs::${command}\">${letter}</a>"
	}
    }
    puts $chan "</b>"

    # Print out the groups
    puts $chan "<p><HR><b><H2>Hs Commands by Topic</H2></b>"
    set group_number 0
    foreach {group pattern_list} $command_groups {
	if {$groupcount($group) > 0} {
	    incr group_number
	    puts $chan "<a NAME=\"topic${group_number}\"></a>"
	    puts $chan "<P><b>${group}</b><br>"
	    puts $chan "<table><tr>"
	    set count 0
	    foreach pattern $match_patterns($group) {
		foreach command [lsort $groupcommands($group,$pattern)] {
		    if {$count != 0 && [expr {$count % 3}] == 0} {
			puts $chan "</tr><tr>"
		    }
		    incr count
		    puts $chan "<td><a href=\"#hs::${command}\">hs::${command}</a>&nbsp;"
		}
	    }
	    puts $chan "</tr></table>"
	}
    }

    # Alphabetical list of commands
    puts $chan "<p><HR><b><H2>Hs Commands by Name</H2></b>"
    foreach command $comlist {
	puts $chan "<a NAME=\"hs::${command}\"></a>"
	puts $chan "<pre>"
	if {[catch {hs::Lookup_command_info $command $text_manual} text]} {
	    puts $chan ""
	    puts $chan "hs::$command"
	    puts $chan ""
	    puts $chan "    No description exists."
	    puts $chan ""
	} else {
	    # Display the command name in bold font
	    set text [string trim [string map {& &amp; < &lt; > &gt;} $text]]
	    set num [string first " " $text]
	    puts -nonewline $chan "<b>[string range $text 0 [expr {$num - 1}]]</b>"
	    # We want to link all hs:: references except the reference
	    # to to the current command
	    regsub -all hs::$command [string range $text $num end] THISCOMMANDD text
	    regsub -all {hs::[0-9_a-z]+} $text {<a href="#&">&</a>} text
	    regsub -all THISCOMMANDD $text hs::$command text
	    # Link all URLs
            regsub -all {(http|ftp)://([^[:blank:]]*)} $text {<a href="&">&</a>} text
	    if {[string equal $command "calc_manual"]} {
		regsub {extended description} $text {<a href="#hs::calc_Details">&</a>} text
	    }
            puts $chan $text
	}
	puts $chan "</pre>"
    }

    # hs::calc manual
    puts $chan "<p><HR><a NAME=\"hs::calc_Details\"></a>"
    puts $chan "<b><H2>hs::calc Details</H2></b>"
    puts $chan "<pre>"
    set channel [open $calc_manual r]
    while {[gets $channel line] >= 0} {puts $chan $line}
    close $channel
    puts $chan "</pre>"

    # Fitting manual
    if {[hs::have_cernlib]} {
	puts $chan "<p><HR><a NAME=\"Fitting_Details\"></a>"
	puts $chan "<b><H2>Fitting Details</H2></b>"
	puts $chan "<pre>"
	set channel [open $fit_manual r]
	set linenum 0
	while {[gets $channel line] >= 0} {
	    if {[incr linenum] > 3} {puts $chan $line}
	}
	close $channel
	puts $chan "</pre>"
    }

    puts $chan "<HR WIDTH=\"100%\"><i>Generated by\
	    <a href=\"#hs::html_manual\">hs::html_manual</a> on\
	    [clock format [clock seconds]]"
    puts $chan "</body>\n</html>"
    close $chan
    return
}

#########################################################################

proc ::hs::generate_ps {winname filename {wait_till_completion 0}} {
    set nscopes [hs::num_connected_scopes]
    if {$nscopes == 1} {
	variable Known_window_names
	if {![info exists Known_window_names($winname)]} {
	    error "Invalid window name \"$winname\""
	}
	# Make sure Histo-Scope will be able to see the complete file name
	if {[string length $filename] > 480} {
	    error "File name is too long"
	}
	if {[file exists $filename]} {
	    if {[file isdirectory $filename]} {
		error "$filename is a directory"
	    }
	    if {![file writable $filename]} {
		error "file $filename is not writable"
	    }
	} else {
	    set dir [file dirname $filename]
	    if {![file writable $dir]} {
		error "directory $dir is not writable"
	    }
	}
	if {$wait_till_completion} {
	    # We have enough time to check that the named window exists
	    set window_status [lindex [hs::Windows_exist $winname] 0]
	    if {$window_status == 0} {
		error "Invalid window name \"$winname\""
	    }
	}
	set config_string "WindowAction\n\
		WindowName ${winname}\n Action Save ${filename}\n"
	# Nothing should prevent Histo-Scope from generating the file now
	hs::Histoscope_command $config_string $wait_till_completion {;#}
    } elseif {$nscopes > 1} {
	error "Can't write the file safely:\
		more than one Histo-Scope connected"
    } else {
	error "Histo-Scope is not connected"
    }
    return
}

#########################################################################

proc ::hs::Overlay_compatible {type1 type2} {
    # types may take the following values:
    # HS_1D_HISTOGRAM, HS_2D_HISTOGRAM, ts, tse, xy, 
    # xye, xys, xyse, scat2, scat3, h1, h1a, h2, h2a,
    # cell. Only X-Y type plots can be overlayed.
    foreach n {1 2} {
	switch -- [set type$n] {
	    HS_1D_HISTOGRAM -
	    ts -
	    tse -
	    xy -
	    xye -
	    xys -
	    xyse -
	    h1 -
	    h1a {
		set overlay_allowed_$n 1
	    }
	    default {
		set overlay_allowed_$n 0
	    }
	}
    }
    if {$overlay_allowed_1 && $overlay_allowed_2} {
	return 1
    }
    return 0
}

############################################################################

proc ::hs::dir {filename} {
    # Check the file creation time. Reread the list of items
    # if current ctime or size is different from the old one.
    # Cache the list of items because creation of this list may
    # be a time-consuming operation for files with many items.
    variable Hsdir_cache_dirinfo
    variable Hsdir_cache_stats
    file stat $filename hsfile_stat
    set reread_item_list 0
    if {![info exists Hsdir_cache_dirinfo($filename)]} {
	set reread_item_list 1
    } elseif {$Hsdir_cache_stats(size,$filename) != $hsfile_stat(size) ||\
	    $Hsdir_cache_stats(ctime,$filename) != $hsfile_stat(ctime)} {
	set reread_item_list 1
    }
    if {$reread_item_list} {
	set Hsdir_cache_dirinfo($filename) [hs::Dir_uncached $filename]
	set Hsdir_cache_stats(ctime,$filename) $hsfile_stat(ctime)
	set Hsdir_cache_stats(size,$filename) $hsfile_stat(size)
    }
    return $Hsdir_cache_dirinfo($filename)
}

############################################################################

proc ::hs::ntuple_paste {uid title category id1 id2} {
    set vlist1 [hs::ntuple_variable_list $id1]
    set vlist2 [hs::ntuple_variable_list $id2]
    set len1 [hs::num_entries $id1]
    set len2 [hs::num_entries $id2]
    if {$len1 != $len2} {
        error "ntuples with ids $id1 and $id2 have different numbers of rows"
    }
    set new_id [hs::create_ntuple $uid $title $category \
            [concat $vlist1 $vlist2]]
    hs::Ntuple_paste $new_id $id1 $id2
    set new_id
}

############################################################################

proc ::hs::read_items_bytitle {args} {
    set first [lindex $args 0]
    if {[string equal $first "-nocase"]} {
	set case_sensitive 0
	set not_options [lrange $args 1 end]
    } else {
	set case_sensitive 1
        set not_options $args
    }
    if {[llength $not_options] == 3} {
	# Use any category
	foreach {filename category_prefix title_pattern} $not_options {}
	# Avoid using conditional statements inside the inner loops
	if {$case_sensitive} {
	    foreach item [hs::dir $filename] {
		foreach {uid category title type} $item {}
		if {[string match $title_pattern $title]} {
		    lappend uid_list($category) $uid
		}
	    }
	} else {
	    foreach item [hs::dir $filename] {
		foreach {uid category title type} $item {}
		if {[string match -nocase $title_pattern $title]} {
		    lappend uid_list($category) $uid
		}
	    }
	}
    } elseif {[llength $not_options] == 4} {
	# Match against the given category pattern
	foreach {filename category_prefix\
		title_pattern category_pattern} $not_options {}
	if {$case_sensitive} {
	    foreach item [hs::dir $filename] {
		foreach {uid category title type} $item {}
		if {[string match $title_pattern $title]} {
		    if {[string match $category_pattern $category]} {
			lappend uid_list($category) $uid
		    }
		}
	    }
	} else {
	    foreach item [hs::dir $filename] {
		foreach {uid category title type} $item {}
		if {[string match -nocase $title_pattern $title]} {
		    if {[string match -nocase $category_pattern $category]} {
			lappend uid_list($category) $uid
		    }
		}
	    }
	}
    } else {
	error "hs::read_items_bytitle : wrong # of arguments"
    }
    set read_count 0
    foreach category [array names uid_list] {
	set nread [hs::read_file_items $filename \
                $category_prefix $category $uid_list($category)]
	if {$nread < 0} {
	    error "hs::read_items_bytitle : failed to read\
		    items from file $filename, category $category"
	}
        incr read_count $nread
    }
    return $read_count
}

############################################################################

proc ::hs::Parse_geometry_option {value} {
    set scanned [scan $value "%dx%d%d%d" w h x y]
    if {$scanned < 2 || $scanned > 4} {
	error "invalid window geometry \"$value\""
    } elseif {$w <= 0 || $h <= 0} {
	error "invalid window geometry \"$value\""
    }
    if {$scanned < 4} {
	set y 0
    }
    if {$scanned < 3} {
	set x 0
    }
    if {$x >= 0} {set x "+$x"}
    if {$y >= 0} {set y "+$y"}
    return "${w}x${h}${x}${y}"
}

############################################################################

proc ::hs::Multiplot_parse_cell_id {cell_id} {
    set xy [split $cell_id ,]
    if {[llength $xy] != 2} {
	error "invalid cell identifier \"$cell_id\""
    }
    foreach {x y} $xy {}
    if {![string is integer $x] || ![string is integer $y]} {
	error "invalid cell identifier \"$cell_id\""
    }
    if {$x < 0 || $y < 0} {
	error "invalid cell identifier \"$cell_id\""
    }
    return [list $x $y]
}


############################################################################

proc ::hs::Overlay_overlay {name tag} {
    # Copy items from overlay named $tag into overlay named $name
    variable Overlay_count
    if {![info exists Overlay_count($name)]} {
        error "\"$name\" is not a valid overlay name"
    }
    if {![info exists Overlay_count($tag)]} {
        error "\"$tag\" is not a valid overlay name"
    }
    if {[string equal $name $tag]} return
    variable Overlay_info
    variable Ntuple_plot_info
    if {$Overlay_count($name) > 0 && $Overlay_count($tag) > 0} {
	if {![hs::Overlay_compatible $Overlay_info($name,0,type) \
		$Overlay_info($tag,0,type)]} {
	    error "overlays $name and $tag have incompatible plot types"
	}
    }
    for {set j 0} {$j < $Overlay_count($tag)} {incr j} {
        set newhisto 1
        for {set index 0} {$index < $Overlay_count($name)} {incr index} {
            if {$Overlay_info($name,$index,id) == $Overlay_info($tag,$j,id)} {
                set newhisto 0
                break
            }
        }
	if {!$newhisto} {
	    if {![string equal $Overlay_info($name,$index,header) "NTupleItem"]} {
		set old_owner $Overlay_info($name,$index,owner)
		set new_owner $Overlay_info($tag,$j,owner)
		# Figure out the change of ownership
		if {$old_owner && !$new_owner} {
		    hs::Overlay_disown_item $Overlay_info($name,$index,id)
		} elseif {!$old_owner && $new_owner} {
		    hs::Overlay_incr_refcount $Overlay_info($name,$index,id)
		}
	    }
	}
        foreach {iarray value} [array get Overlay_info $tag,$j,*] {
            set listindex [list $name $index]
            eval lappend listindex [lrange [split $iarray ,] 2 end]
            set newindex [join $listindex ,]
            set Overlay_info($newindex) $value
        }
        if {$newhisto} {
            incr Overlay_count($name)
	    if {[string equal $Overlay_info($name,$index,header) "NTupleItem"]} {
		set plotname $Overlay_info($name,$index,id)
		incr Ntuple_plot_info($plotname,refcount)
	    } elseif {$Overlay_info($name,$index,owner)} {
		hs::Overlay_incr_refcount $Overlay_info($name,$index,id)
	    }
	}
    }
    return
}

############################################################################

proc ::hs::Overlay_disown_item {id} {
    variable Overlay_item_refcount
    incr Overlay_item_refcount($id) -1
    if {$Overlay_item_refcount($id) == 0} {
	unset Overlay_item_refcount($id)
	return 0
    }
    return $Overlay_item_refcount($id)
}

############################################################################

proc ::hs::Overlay_decr_refcount {id} {
    set count [hs::Overlay_disown_item $id]
    if {$count == 0} {
	if {![string equal [hs::type $id] "HS_NONE"]} {
	    hs::delete $id
	}
    }
    return $count
}

############################################################################

proc ::hs::Overlay_incr_refcount {id} {
    variable Overlay_item_refcount
    if {![info exists Overlay_item_refcount($id)]} {
	set Overlay_item_refcount($id) 0
    }
    incr Overlay_item_refcount($id)
}

############################################################################

proc ::hs::Validate_marker_size {value} {
    # The marker sizes are defined in the
    # Histo-Scope header file XY.h
    if {![string is integer $value]} {
	switch -- $value {
	    tiny {
		set value 0
	    }
	    small {
		set value 1
	    }
	    medium {
		set value 2
	    }
	    large {
		set value 3
	    }
	    default {
		error "invalid marker size \"$value\""
	    }
	}
    } elseif {$value < 0 || $value >= 4} {
	error "marker size $value is out of range"
    }
    return $value
}

############################################################################

proc ::hs::Validate_marker_style {value} {
    # The marker styles are defined in the
    # Histo-Scope header file XY.h
    if {![string is integer $value]} {
	set allowed_names {none square circle star x triangle\
		solidsquare solidcircle thicksquare thickcircle}
	if {[set number [lsearch -exact $allowed_names $value]] < 0} {
	    error "invalid marker style \"$value\""
	}
	return $number
    } elseif {$value < 0 || $value >= 10} {
	error "marker style $value is out of range"
    }
    return $value
}

############################################################################

proc ::hs::Label_from_expr {expression} {
    set ylabel ""
    set line_count 0
    foreach exp_line [split $expression "\n"] {
	set trimmed_line [string trim $exp_line]
	if {![string equal $trimmed_line ""]} {
	    incr line_count
	    if {[string equal $ylabel ""]} {
		set ylabel $trimmed_line
	    }
	}
    }
    if {[string length $ylabel] > 73} {
        set ylabel "[string range $ylabel 0 72] ..."
    } elseif {$line_count > 1} {
	append ylabel " ..."
    }
    set ylabel
}

############################################################################

proc ::hs::Put_plot_into_overlay {} {
    uplevel {
	if {[string equal $overlay ""]} {
	    set overlay [hs::Unused_plot_name]
	    set new_overlay 1
	} else {
	    set new_overlay 0
	}
	hs::overlay $overlay -geometry $geometry \
                add $id xy -x [hs::variable_name $id 0] \
		-y [hs::variable_name $id 1] -owner 1 -line $line \
		-color $color -marker $marker -markersize $markersize
	# Set overlay limits and labels
	set other_options {}
	foreach {option_name} {xmin xmax ymin ymax} {
	    if {![string equal [set $option_name] "undefined"]} {
		lappend other_options -$option_name [set $option_name]
	    }
	}
	foreach {option_name} {xlabel ylabel} {
	    if {![string is space [set $option_name]]} {
		lappend other_options -$option_name [list [set $option_name]]
	    }
	}
	if {[llength $other_options] > 0} {
	    if {[catch {eval hs::overlay [list $overlay] \
		    $other_options} limit_error]} {
		puts stderr "$limit_error. All limits and labels ignored."
	    }
	}
	if {$show == 2 && $new_overlay || $show == 1} {
            if {[catch {
                set command [list hs::overlay $overlay show -title $title \
                                 -ipadx $ipadx -ipady $ipady]
            }]} {
                set command [list hs::overlay $overlay show \
                                 -title $ntuple_title \
                                 -ipadx $ipadx -ipady $ipady]
            }
	    if {![string equal $window ""]} {
		lappend command -window $window
	    }
	    if {![string equal $font ""]} {
		lappend command -font $font
	    }
	    eval $command
	    hs::hs_update
	}
    }
    return
}

############################################################################

proc ::hs::Parse_plot_options {} {
    uplevel {
	if {[expr [llength $args] % 2] != 0} {
	    error "wrong # of arguments"
	}
	set available_options {-plotpoints -id -overlay\
		-line -color -marker -markersize -show\
		-xlabel -ylabel -title -xmin -xmax -ymin -ymax\
                -window -font -ipadx -ipady -geometry}
	set procname [namespace tail [lindex [info level 0] 0]]
	if {[string equal $procname "parametric_plot"]} {
	    lappend available_options -hardzoom
	}
	# Default option values
	set plotpoints 1000
	set id -1
	set overlay ""
	set line 1
	set color black
	set marker 0
	set markersize 1
	set show 2
	set xmin "undefined"
	set xmax "undefined"
	set ymin "undefined"
	set ymax "undefined"
	set hardzoom 0
	set window ""
	set xlabel ""
	set ylabel ""
	set font ""
        set geometry 400x300
	set ipadx {0 0}
	set ipady {0 0}
	foreach {option value} $args {
	    if {[lsearch -exact $available_options $option] < 0} {
		error "Invalid option \"$option\". Available options\
			are [join [lsort $available_options] {, }]."
	    }
	    regsub "^-" $option "" option
	    switch -- $option {
		xmin -
		xmax -
		ymin -
		ymax {
		    # Should be either an empty string or a double
		    if {![string equal $value ""]} {
			if {![string is double $value]} {
			    error "expected a double value for\
				    option -$option, got \"$value\""
			}
		    }
		}
		show {
		    # Should be a boolean value
		    if {[catch {
			if {$value} {
			    set value 1
			} else {
			    set value 0
			}
		    }]} {
			error "expected a boolean value, got \"$value\""
		    }
		    if {$id != -1} {
			error "options -id and -show are mutually exclusive"
		    }
		}
		hardzoom {
		    # Should be a boolean value
		    if {[catch {
			if {$value} {
			    set value 1
			} else {
			    set value 0
			}
		    }]} {
			error "expected a boolean value, got \"$value\""
		    }	    
		}
                geometry {
		    set value [hs::Parse_geometry_option $value]
		}
		plotpoints {
		    # Must be a positive integer > 1
		    if {![string is integer $value]} {
			error "expected an integer larger than 1, got \"$value\""
		    } elseif {$value <= 1} {
			error "expected an integer larger than 1, got $value"
		    }
		}
		id {
		    # Must be an id of an ntuple with 2 variables
		    if {![string is integer $value]} {
			error "expected a Histo-Scope id, got \"$value\""
		    } elseif {![string equal [hs::type $value] "HS_NTUPLE"]} {
			if {[string equal [hs::type $value] "HS_NONE"]} {
			    error "Histo-Scope item with id $value does not exist"
			} else {
			    error "Histo-Scope item with id $value is not an ntuple"
			}
		    } elseif {[hs::num_variables $value] != 2} {
			error "Ntuple with id $value has\
				[hs::num_variables $value] variables, need 2"
		    }
		    if {![string equal $overlay ""]} {
			error "options -id and -overlay are mutually exclusive"
		    }
		    if {$show != 2} {
			error "options -id and -show are mutually exclusive"
		    }
		}
		overlay {
		    hs::Validate_overlay_name $value
		    if {$id != -1} {
			error "options -id and -overlay are mutually exclusive"
		    }
		}
		line {
		    # xy plot line styles are defined in the
		    # Histo-Scope header file XY.h
		    if {![string is integer $value]} {
			error "invalid line style \"$value\""
		    } elseif {$value < 0 || $value >= 13} {
			error "line style $value is out of range"
		    }
		}
		color {
		    # Too many colors to check... Let the client complain
		    # if something goes wrong.
		}
		font {
		    # Just make sure that this font exists
		    if {![string equal $value ""]} {
			hs::Generate_xlfd $value
		    }
		}
		ipadx -
		ipady {
		    # Should be a two-element list of integers
		    if {[llength $value] != 2} {
			error "invalid margin padding \"$value\""
		    }
		    foreach padv $value {
			if {![string is integer -strict $padv]} {
			    error "invalid margin padding \"$value\""
			}
		    }
		}
		window {
		    # The argument is (almost) an arbitrary string
		    set value [string trimleft $value]
		    hs::Check_window_name $value
		}
		marker {
		    set value [hs::Validate_marker_style $value]
		}
		markersize {
		    set value [hs::Validate_marker_size $value]
		}
                title -
                xlabel -
		ylabel {
		    # This can be an arbitrary string
		}
		default {
		    error "Incomplete switch statement in [lindex [info level 0] 0].\
			    This is a bug. Please report."
		}
	    }
	    set $option $value
	}
	if {$hardzoom} {
	    set plot_point_accept_body "if \{1"
	    ::fit::Eval_at_precision 17 {
		foreach {variable_name if_condition} {
		    xmin {$xc >=}
		    xmax {$xc <=}
		    ymin {$yc >=}
		    ymax {$yc <=}
		} {
		    set value [set $variable_name]
		    if {![string equal $value "undefined"]} {
			append plot_point_accept_body " && $if_condition [expr {1.0 * $value}]"
		    }		    
		}
	    }
	    append plot_point_accept_body "\} {return 1} else {return 0}"
	} else {
	    set plot_point_accept_body "return 1"
	}
	proc ::hs::Plot_point_accept {xc yc} $plot_point_accept_body
    }
    return
}

############################################################################

proc ::hs::list_plot {value_list args} {

    # Parse the optional arguments
    hs::Parse_plot_options

    # Check the list size
    if {[expr [llength $value_list] % 2] != 0} {
	error "odd number of elements in the list of points, must be even"
    }

    # Create a new ntuple if necessary
    if {$id == -1} {
	variable List_plot_counter
	set ntuple_title "List plot [incr List_plot_counter]"
	set id [hs::create_ntuple [hs::Temp_uid] \
		$ntuple_title [hs::Temp_category] {x y}]
	set new_ntuple 1
    } else {
	set new_ntuple 0
    }

    # Fill the ntuple
    if {[catch {hs::ntuple_block_fill $id $value_list} errmess]} {
	# Clean up
	if {$new_ntuple} {
	    hs::delete $id
	}
	error $errmess
    }

    # Return if we are required to fill an existing ntuple
    if {!$new_ntuple} {
	return
    }

    # Make the plot
    hs::Put_plot_into_overlay
    return $overlay
}

############################################################################

proc ::hs::column_minimum {id column} {
    foreach {rowmin vmin rowmax vmax} [hs::Column_minmax $id $column] {}
    list $rowmin $vmin
}

############################################################################

proc ::hs::column_maximum {id column} {
    foreach {rowmin vmin rowmax vmax} [hs::Column_minmax $id $column] {}
    list $rowmax $vmax
}

############################################################################

proc ::hs::ntuple_block_fill {id data} {
    # Before resetting, verify that this is an ntuple
    hs::num_variables $id
    hs::reset $id
    hs::fill_ntuple $id $data
}

############################################################################

proc ::hs::function_plot {tag varname lolim uplim args} {

    # Check that the specified function exists
    if {![hs::function $tag exists]} {
	error "invalid function tag \"$tag\""
    }

    # Check the provided name
    set valid_names [concat x y z [hs::function $tag cget -parameters]]
    if {[lsearch -exact $valid_names $varname] < 0} {
	error "invalid parameter or variable name \"$varname\""
    }

    # Check the limits
    if {![string is double $lolim]} {
	error "expected a real number, got \"$lolim\""
    }
    if {![string is double $uplim]} {
	error "expected a real number, got \"$uplim\""
    }

    # Go over the arguments and find 2-element
    # lists of name-value pairs
    set param_values {}
    set options {}
    foreach tmp $args {
	if {[llength $tmp] == 2} {
	    foreach {name value} $tmp {}
	    if {[lsearch -exact $valid_names $name] >= 0} {
		lappend param_values $tmp
	    } else {
		error "invalid parameter or variable name \"$name\""
	    }
	} else {
	    lappend options $tmp
	}
    }

    # Parse the optional arguments
    set args $options
    hs::Parse_plot_options

    # Create a new ntuple if necessary
    if {$id == -1} {
	variable Function_plot_counter
	set ntuple_title "Function plot [incr Function_plot_counter]"
	set id [hs::create_ntuple [hs::Temp_uid] \
		$ntuple_title [hs::Temp_category] [list $varname $tag]]
	set new_ntuple 1
    } else {
	set new_ntuple 0
    }

    # Fill the ntuple
    if {[catch {eval hs::function [list $tag] scan \
	    [list [list $id [list $varname $lolim $uplim $plotpoints]]] \
	    $param_values} errmess]} {
	# Clean up
	if {$new_ntuple} {
	    hs::delete $id
	}
	error $errmess
    }

    # Return if we are required to fill an existing ntuple
    if {!$new_ntuple} {
	return
    }

    # Make the plot
    hs::Put_plot_into_overlay
    return $overlay
}

############################################################################

proc ::hs::parametric_plot {x_expr y_expr varname lolim uplim args} {

    # Check the limits
    if {![string is double $lolim]} {
	error "expected a real number, got \"$lolim\""
    }
    if {![string is double $uplim]} {
	error "expected a real number, got \"$uplim\""
    }

    # Parse the optional arguments
    hs::Parse_plot_options

    # Make sure that the expressions can at least be evaluated
    # at the limits before making a new ntuple or resetting
    # an old ntuple.
    upvar $varname x
    if {[info exists x]} {
	set old_value $x
    }
    if {[catch {
	set x [expr {1.0 * $lolim}]
	set x_loval [uplevel $x_expr]
	set x [expr {1.0 * $uplim}]
	set x_hival [uplevel $x_expr]
    } errmess]} {
	set x_fail $x
	if {[info exists old_value]} {
	    set x $old_value
	} else {
	    unset x
	}
	error "Expression \"$x_expr\" can not be\
		evaluated for $varname value of ${x_fail}: $errmess"
    }
    if {[catch {
	set x [expr {1.0 * $lolim}]
	set y_loval [uplevel $y_expr]
	set x [expr {1.0 * $uplim}]
	set y_hival [uplevel $y_expr]
    } errmess]} {
	set x_fail $x
	if {[info exists old_value]} {
	    set x $old_value
	} else {
	    unset x
	}
	error "Expression \"$y_expr\" can not be\
		evaluated for $varname value of ${x_fail}: $errmess"
    }

    # Create a new ntuple or reset an existing ntuple
    if {$id == -1} {
	variable Parametric_plot_counter
	set ntuple_title "Parametric plot [incr Parametric_plot_counter]"
	if {[string is space $xlabel]} {
	    set xlabel [hs::Label_from_expr $x_expr]
	}
	if {[string is space $ylabel]} {
	    set ylabel [hs::Label_from_expr $y_expr]
	}
	set id [hs::create_ntuple [hs::Temp_uid] \
		$ntuple_title [hs::Temp_category] [list $xlabel $ylabel]]
	set new_ntuple 1
    } else {
	hs::reset $id
	set new_ntuple 0
    }

    # Fill the ntuple
    set n_intervals [expr {$plotpoints - 1}]
    set step [expr {($uplim - $lolim) / (1.0 * $n_intervals)}]
    if {[hs::Plot_point_accept $x_loval $y_loval]} {
	hs::fill_ntuple $id [list $x_loval $y_loval]
    }
    if {[catch {
	for {set i 1} {$i < $n_intervals} {incr i} {
	    set x [expr {$lolim + $i * $step}]
	    set xval [uplevel $x_expr]
	    set yval [uplevel $y_expr]
	    if {[hs::Plot_point_accept $xval $yval]} {
		hs::fill_ntuple $id [list $xval $yval]
	    }
	}
    } errmess]} {
	# Clean up
	if {$new_ntuple} {
	    hs::delete $id
	} else {
	    hs::reset $id
	}
	set x_fail $x
	if {[info exists old_value]} {
	    set x $old_value
	} else {
	    unset x
	}
	error "At least one of the expressions \"$x_expr\" and \"$y_expr\" can\
		not be evaluated for $varname value of ${x_fail}: $errmess"
    }
    if {[hs::Plot_point_accept $x_hival $y_hival]} {
	hs::fill_ntuple $id [list $x_hival $y_hival]
    }

    # Restore the state of the variable with name $varname
    if {[info exists old_value]} {
	set x $old_value
    } else {
	unset x
    }

    # Return if we are required to fill an existing ntuple
    if {!$new_ntuple} {
	return
    }

    # Make the plot
    hs::Put_plot_into_overlay
    return $overlay
}

############################################################################

proc ::hs::expr_plot {expression varname lolim uplim args} {

    # Check the limits
    if {![string is double $lolim]} {
	error "expected a real number, got \"$lolim\""
    }
    if {![string is double $uplim]} {
	error "expected a real number, got \"$uplim\""
    }

    # Parse the optional arguments
    hs::Parse_plot_options

    # Make sure that the expression can at least be evaluated
    # at the limits before making a new ntuple or resetting
    # an old ntuple.
    upvar $varname x
    if {[info exists x]} {
	set old_value $x
    }
    if {[catch {
	set x [expr {1.0 * $lolim}]
	set loval [uplevel $expression]
	set x [expr {1.0 * $uplim}]
	set hival [uplevel $expression]
    } errmess]} {
	set x_fail $x
	if {[info exists old_value]} {
	    set x $old_value
	} else {
	    unset x
	}
	error "Expression \"$expression\" can not be\
		evaluated for $varname value of ${x_fail}: $errmess"
    }

    # Create a new ntuple or reset an existing ntuple
    if {$id == -1} {
	variable Expr_plot_counter
	set ntuple_title "Expr plot [incr Expr_plot_counter]"
	# Create a good name for the function variable -- it will
	# show up as the y label on the plots.
	if {[string is space $ylabel]} {
	    set ylabel [hs::Label_from_expr $expression]
	}
	set id [hs::create_ntuple [hs::Temp_uid] \
		$ntuple_title [hs::Temp_category] [list $varname $ylabel]]
	set new_ntuple 1
    } else {
	hs::reset $id
	set new_ntuple 0
    }

    # Fill the ntuple
    set n_intervals [expr {$plotpoints - 1}]
    set step [expr {($uplim - $lolim) / (1.0 * $n_intervals)}]
    hs::fill_ntuple $id [list $lolim $loval]
    if {[catch {
	for {set i 1} {$i < $n_intervals} {incr i} {
	    set x [expr {$lolim + $i * $step}]
	    hs::fill_ntuple $id [list $x [uplevel $expression]]
	}
    } errmess]} {
	# Clean up
	if {$new_ntuple} {
	    hs::delete $id
	} else {
	    hs::reset $id
	}
	set x_fail $x
	if {[info exists old_value]} {
	    set x $old_value
	} else {
	    unset x
	}
	error "Expression \"$expression\" can not be\
		evaluated for $varname value of ${x_fail}: $errmess"
    }
    hs::fill_ntuple $id [list $uplim $hival]

    # Restore the state of the variable with name $varname
    if {[info exists old_value]} {
	set x $old_value
    } else {
	unset x
    }

    # Return if we are required to fill an existing ntuple
    if {!$new_ntuple} {
	return
    }

    # Make the plot
    hs::Put_plot_into_overlay
    return $overlay
}

############################################################################

proc ::hs::Write_standard_c_headers {chan} {
    puts $chan "#include <assert.h>"
    puts $chan "#include <ctype.h>"
    puts $chan "#include <errno.h>"
    puts $chan "#include <float.h>"
    puts $chan "#include <limits.h>"
    puts $chan "#include <locale.h>"
    puts $chan "#include <math.h>"
    puts $chan "#include <setjmp.h>"
    puts $chan "#include <signal.h>"
    puts $chan "#include <stdarg.h>"
    puts $chan "#include <stddef.h>"
    puts $chan "#include <stdio.h>"
    puts $chan "#include <stdlib.h>"
    puts $chan "#include <string.h>"
    puts $chan "#include <time.h>"
    return
}

############################################################################

proc ::hs::Temp_uid {} {
    variable Temporary_category_uid
    incr Temporary_category_uid
}

############################################################################

proc ::hs::Temp_category {} {
    variable Temporary_category_name
    set Temporary_category_name
}

############################################################################

proc ::hs::Is_ntuple_c_compatible {ntuple_id expr_list} {
    # This proc checks that the variable names in the ntuple
    # are valid C variable names. If not then it searches for
    # these variable names in the expressions specified by expr_list.
    # If found, it returns the name of the first offending variable,
    # otherwise it returns an empty string.
    foreach name [hs::ntuple_variable_list $ntuple_id] {
	if {![hs::Is_simple_c_postfix $name]} {
	    foreach text $expr_list {
		if {[string first $name $text] >= 0} {
		    return $name
		}
	    }
	}
    }
    return ""
}

############################################################################

proc ::hs::Unused_plot_name {} {
    variable Plot_tcl_overlay_counter
    variable Overlay_count
    variable Multiplot_info
    set overlay "hs_PL0T_[incr Plot_tcl_overlay_counter]"
    while {[info exists Overlay_count($overlay)] || \
	    [info exists Multiplot_info($overlay,nx)]} {
	set overlay "hs_PL0T_[incr Plot_tcl_overlay_counter]"
    }
    set overlay
}

############################################################################

proc ::hs::function_list {{pattern *}} {
    lsort -dictionary [hs::Function_list_unsorted $pattern]
}

############################################################################

proc ::hs::Multiplot_optimal_grid {nplots} {
    # Returns the list {n_columns n_rows}
    if {![string is integer $nplots]} {
	error "expected an integer, got \"$nplots\""
    }
    if {$nplots <= 0} {
        set best_choice {0 0}
    } elseif {$nplots == 1} {
        set best_choice {1 1}
    } elseif {$nplots == 3} {
        set best_choice {1 3}
    } else {
        set n_columns [expr {int(sqrt(0.5*$nplots))}]
        set n_rows [expr {$nplots / $n_columns}]
        if {[expr {$n_columns * $n_rows}] < $nplots} {
            incr n_rows
        }
        if {[expr {(1.0 * $n_rows) / $n_columns}] > 2.0} {
            incr n_columns
            set n_rows [expr {$nplots / $n_columns}]
            if {[expr {$n_columns * $n_rows}] < $nplots} {
                incr n_rows
            }
        }
        set best_empty $n_columns
        while {$n_columns <= $n_rows} {
            set n_empty [expr {$n_columns * $n_rows - $nplots}]
            if {$n_empty == 0} {
                return [list $n_columns $n_rows]
            }
            if {$n_empty < $best_empty} {
                set best_empty $n_empty
                set best_choice [list $n_columns $n_rows]
            }
            incr n_columns
            set n_rows [expr {$nplots / $n_columns}]
            if {[expr {$n_columns * $n_rows}] < $nplots} {
                incr n_rows
            }
        }
    }
    return $best_choice
}

############################################################################

proc ::hs::ntuple_creation_template {filename varlist} {
    foreach name $varlist {
	if {![hs::Is_simple_c_postfix $name]} {
	    error "\"$name\" is not a valid C variable name"
	}
    }
    set chan [open $filename w]
    puts $chan "#include <stdio.h>"
    puts $chan "#include <stdlib.h>"
    puts $chan "#include <string.h>"
    puts $chan "#include <assert.h>"
    puts $chan "#include \"histoscope.h\""
    puts $chan ""
    puts $chan "#define N_NTUPLE_VARIABLES [llength $varlist]"
    puts $chan ""
    puts $chan "/* The following function should be called at some point near"
    puts $chan " * the beginning of the job. Don't forget to call \"hs_initialize\""
    puts $chan " * before making this and any other Histo-Scope calls."
    puts $chan " */"
    puts $chan "static int get_ntuple_id(int uid, char *title, char *category)"
    puts $chan "\{"
    puts $chan "    static int ntuple_id = 0;"
    puts $chan "    if (ntuple_id == 0)"
    puts $chan "    \{"
    puts -nonewline $chan "        char *names\[N_NTUPLE_VARIABLES\] = {\n            \""
    puts -nonewline $chan [join $varlist "\",\n            \""]
    puts $chan "\"\n        };"
    puts $chan "        assert(title);"
    puts $chan "        assert(category);"
    puts $chan "        ntuple_id = hs_create_ntuple(uid, title, category,"
    puts $chan "                                     N_NTUPLE_VARIABLES, names);"
    puts $chan "        assert(ntuple_id > 0);"
    puts $chan "    \}"
    puts $chan "    return ntuple_id;"
    puts $chan "\}"
    puts $chan ""
    puts $chan "/* The following code should be executed every time"
    puts $chan " * a new ntuple entry is made. Edit it to suit your needs."
    puts $chan " */"
    puts $chan "\{"
    puts $chan "    float row_data\[N_NTUPLE_VARIABLES\];"
    hs::Varlist_pack_code $varlist $chan row_data 1
    puts $chan "    assert(ntuple_column == N_NTUPLE_VARIABLES);"
    puts $chan "    if (hs_fill_ntuple(get_ntuple_id(0, NULL, NULL), row_data) <= 0) \{"
    puts $chan "        if (hs_type(get_ntuple_id(0, NULL, NULL)) == HS_NTUPLE) \{"
    puts $chan "            fprintf(stderr, \"Ntuple fill error. Out of memory. Exiting.\");"
    puts $chan "            exit(EXIT_FAILURE);"
    puts $chan "        \} else \{"
    puts $chan "            fprintf(stderr, \"Ntuple fill error: ntuple id is lost. Aborting.\");"
    puts $chan "            fflush(stderr);"
    puts $chan "            abort();"
    puts $chan "        \}"
    puts $chan "    \}"
    puts $chan "\}"
    puts $chan ""
    puts $chan "/* Don't forget to call \"hs_save_file\" at the end of the job */"
    close $chan
    return
}

############################################################################

proc ::hs::ntuple_scan_template {ntuple_id filename} {
    set hs_type [hs::type $ntuple_id]
    if {![string equal $hs_type "HS_NTUPLE"]} {
	if {[string equal $hs_type "HS_NONE"]} {
	    error "Histo-Scope item with id $ntuple_id does not exist"
	} else {
	    error "Histo-Scope item with id $ntuple_id is not an Ntuple"
	}
    }

    # Check that we can generate the unpacking code
    set offending_variable [hs::Is_ntuple_c_compatible $ntuple_id \
	    [hs::ntuple_variable_list $ntuple_id]]
    if {![string equal $offending_variable ""]} {
	error "Ntuple $ntuple_id column named \"$offending_variable\"\
		can not be used in C code"
    }

    set chan [open $filename w]

    set comment " *"
    puts $chan "/*"
    puts $chan "$comment Compile and run this code using the following commands:"
    puts $chan "$comment"
    variable Sharedlib_suffix
    puts $chan "$comment hs::sharedlib_compile $filename mylib$Sharedlib_suffix"
    puts $chan "$comment hs::ntuple_so_scan \$ntuple_id\
	    ./mylib$Sharedlib_suffix some_string"
    puts $chan " */"

    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""
    puts $chan ""
    puts $chan "/* The following function will be called first */"
    puts $chan "int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id,"
    puts $chan "                        const char *some_string)"
    puts $chan "\{"
    puts $chan "    return TCL_OK;"
    puts $chan "\}"
    puts $chan ""
    puts $chan "/* The following function will be called for every ntuple row."
    puts $chan " * Scanning can be terminated at any time by returning TCL_ERROR."
    puts $chan " */"
    puts $chan "int hs_ntuple_scan_function(Tcl_Interp *interp, const float *row_data)"
    puts $chan "\{"
    hs::Ntuple_pack_code $ntuple_id $chan row_data 0
    puts $chan "    return TCL_OK;"
    puts $chan "\}"
    puts $chan ""
    puts $chan "/* The function below will be called at the end of a successful scan */"
    puts $chan "int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id,"
    puts $chan "                            const char *some_string)"
    puts $chan "\{"
    puts $chan "    return TCL_OK;"
    puts $chan "\}"
    close $chan
    return
}

############################################################################

proc ::hs::function_template {filename args} {

    # Check all the arguments before actually writing the file
    if {[expr [llength $args] % 2] != 0} {
	error "wrong # of arguments"
    }
    set known_switches {-lang -init -fname -grad -cleanup}
    set init {}
    set fname "myfit"
    set grad {}
    set cleanup {}

    # Figure out the default language from the file extension
    set extension [string range [file extension $filename] 1 end]
    variable Recognized_fortran_extensions
    if {[lsearch -exact $Recognized_fortran_extensions $extension] < 0} {
	set lang c
    } else {
	set lang f
    }

    foreach {option value} $args {
	if {[lsearch -exact $known_switches $option] < 0} {
	    error "Invalid switch \"$option\".\
		    Valid switches are [join $known_switches {, }]."
	}
	regsub "^-" $option "" option
	switch -- $option {
	    lang {
		if {![string equal -nocase $value "c"] && \
			![string equal -nocase $value "f"] && \
			![string equal -nocase $value "fortran"]} {
		    error "Invalid language \"$value\", expected \"c\" or \"fortran\""
		}
		set value [string tolower $value]
	    }
	    init -
	    fname -
	    grad -
	    cleanup {
		if {![string equal $value ""]} {
		    if {![hs::Is_valid_c_identifier $value]} {
			error "String \"$value\" can not be used\
				as a function name"
		    }
		}
	    }
	    default {
		error "Incomplete switch statement.\
			This is a bug. Please report."
	    }
	}
	set $option $value
    }

    # Check that option settings make sense
    if {[string equal $fname ""]} {
	error "Fitting function name can not be an empty string"
    }
    set non_trivial_names {}
    foreach name [list $init $fname $grad $cleanup] {
	if {![string equal $name ""]} {
	    lappend non_trivial_names $name
	}
    }
    if {[llength $non_trivial_names] != \
	    [llength [lsort -unique $non_trivial_names]]} {
	error "All function names must be distinct"
    }
    if {[string equal $lang "c"]} {
	set language C
    } else {
	set language Fortran
	# Force the correct fortran extension
	if {[lsearch -exact $Recognized_fortran_extensions $extension] < 0} {
	    error "Please use one of the following extensions for the FORTRAN\
		    file name so that\nit can be recognized by the compiler:\
		    [join $Recognized_fortran_extensions {, }]."
	}
    }

    # Now, write the code
    set chan [open $filename w]
    if {[string equal $lang "c"]} {
	hs::Write_standard_c_headers $chan
	puts $chan "#include \"tcl.h\""
        variable Histoscope_header
	puts $chan "#include \"$Histoscope_header\""
	puts $chan ""
    }
    if {![string equal $init ""]} {
	hs::${language}_aux_template $chan $init "init"
	puts $chan ""
	puts $chan "/********************************************************************/"
	puts $chan ""
    }
    hs::${language}_fit_template $chan $fname
    if {![string equal $grad ""]} {
	puts $chan ""
	puts $chan "/********************************************************************/"
	puts $chan ""
	hs::${language}_grad_template $chan $grad
    }
    if {![string equal $cleanup ""]} {
	puts $chan ""
	puts $chan "/********************************************************************/"
	puts $chan ""
	hs::${language}_aux_template $chan $cleanup "cleanup"
    }

    # Write the compilation instructions
    puts $chan ""
    if {[string equal $lang "c"]} {
	puts $chan "/*"
	set comment " * "
    } else {
	set comment "C "
    }
    puts $chan "$comment Compile and load this code using the following commands:"
    puts $chan "$comment"
    variable Sharedlib_suffix
    puts $chan "$comment hs::sharedlib_compile $filename mylib$Sharedlib_suffix"
    puts $chan "$comment set dlltoken \[hs::sharedlib open ./mylib$Sharedlib_suffix\]"
    puts $chan "$comment hs::function_import <tag> \$dlltoken <name> <description> \\"
    puts $chan "$comment     <n_variables> <default_mode> <npars_min> <npars_max> \\"
    puts -nonewline $chan "$comment     <parameter_names>"
    if {![string equal $lang "c"]} {
	foreach name {init fname grad cleanup} {
	    set $name [string tolower [set $name]]
	}
    }
    foreach name [list $init $fname $grad $cleanup] {
	if {[string equal $name ""]} {
	    puts -nonewline $chan " \{\}"
	} else {
	    puts -nonewline $chan " $name"
	}
    }
    puts $chan ""
    puts $chan "$comment"
    puts $chan "$comment All arguments in <> brackets should be replaced by actual function"
    puts $chan "$comment attributes. Please see the description of hs::function_import"
    puts $chan "$comment command for more details about the meaning of various arguments."
    if {[string equal $lang "c"]} {
	puts $chan " */"
    } else {
	puts $chan ""
    }

    close $chan
    return
}

############################################################################

proc ::hs::Fortran_fit_template {chan fname} {
    set FNAME [string toupper $fname]
    puts $chan "      DOUBLE PRECISION FUNCTION ${FNAME}(X, Y, Z, MODE, PARS, IERR)"
    puts $chan "C"
    puts $chan "      IMPLICIT NONE"
    puts $chan "      DOUBLE PRECISION X, Y, Z, PARS(*)"
    puts $chan "      INTEGER MODE, IERR"
    puts $chan "C"
    puts $chan "C Inputs:"
    puts $chan "C     X      is the first variable"
    puts $chan "C     Y      is the second variable (may be ignored)"
    puts $chan "C     Z      is the third variable (may be ignored)"
    puts $chan "C     MODE   is an external parameter which will not"
    puts $chan "C            be changed by Minuit (may be ignored)"
    puts $chan "C     PARS   is the array of fitted parameters"
    puts $chan "C"
    puts $chan "C Outputs:"
    puts $chan "C     IERR   is the error status of the calculation (0 means OK)."
    puts $chan "C            Also, it may be useful to print an informative error"
    puts $chan "C            message to the standard output if something goes wrong."
    puts $chan "C"
    puts $chan "C Returns:"
    puts $chan "C     $FNAME  is the function value for given arguments and parameters"
    puts $chan "C"
    puts $chan "      IERR = "
    puts $chan "      $FNAME = "
    puts $chan "C"
    puts $chan "      RETURN"
    puts $chan "      END"
    return
}

############################################################################

proc ::hs::Fortran_grad_template {chan fname} {
    set FNAME [string toupper $fname]
    puts $chan "      INTEGER FUNCTION ${FNAME}(X, Y, Z, MODE, PARS, GRAD)"
    puts $chan "C"
    puts $chan "      IMPLICIT NONE"
    puts $chan "      DOUBLE PRECISION X, Y, Z, PARS(*), GRAD(*)"
    puts $chan "      INTEGER MODE"
    puts $chan "C"
    puts $chan "C Inputs:"
    puts $chan "C     X      is the first variable"
    puts $chan "C     Y      is the second variable (may be ignored)"
    puts $chan "C     Z      is the third variable (may be ignored)"
    puts $chan "C     MODE   is an external parameter which will not"
    puts $chan "C            be changed by Minuit (may be ignored)"
    puts $chan "C     PARS   is the array of fitted parameters"
    puts $chan "C"
    puts $chan "C Outputs:"
    puts $chan "C     GRAD   is the array of partial derivatives over parameters"
    puts $chan "C"
    puts $chan "C Returns:"
    puts $chan "C     $FNAME  is the error status of the calculation (0 means OK)"
    puts $chan "C"
    puts $chan "      GRAD(1) = "
    puts $chan "      $FNAME = "
    puts $chan "C"
    puts $chan "      RETURN"
    puts $chan "      END"
    return
}

############################################################################

proc ::hs::Fortran_aux_template {chan fname init_or_cleanup} {
    if {[string equal $init_or_cleanup "init"]} {
	set init 1
    } elseif {[string equal $init_or_cleanup "cleanup"]} {
	set init 0
    } else {
	error "invalid argument \"$init_or_cleanup\",\
		expected \"init\" or \"cleanup\""
    }
    set FNAME [string toupper $fname]
    puts $chan "      INTEGER FUNCTION ${FNAME}(MODE)"
    puts $chan "C"
    puts $chan "      IMPLICIT NONE"
    puts $chan "      INTEGER MODE"
    puts $chan "C"
    puts $chan "C Inputs:"
    puts $chan "C     MODE    is the operation mode (may be ignored)"
    puts $chan "C"
    puts $chan "C Returns:"
    if {$init} {
	puts $chan "C     $FNAME  is the initialization error status (0 means OK)"
    } else {
	puts $chan "C     $FNAME  is the cleanup error status (0 means OK)"
    }
    puts $chan "C"
    puts $chan "      $FNAME = "
    puts $chan "C"
    puts $chan "      RETURN"
    puts $chan "      END"
    return
}

############################################################################

proc ::hs::C_fit_template {chan fname} {
    puts $chan "double ${fname}(double x, double y, double z, int mode,"
    puts $chan "	     const double *pars, int *ierr)"
    puts $chan "{"
    puts $chan "    /* Input arguments:"
    puts $chan "     *   x     -- the first variable"
    puts $chan "     *   y     -- the second variable (may be ignored)"
    puts $chan "     *   z     -- the third variable (may be ignored)"
    puts $chan "     *   mode  -- an external parameter which will not"
    puts $chan "     *            be changed by Minuit (may be ignored)"
    puts $chan "     *   pars  -- array of fitted parameters"
    puts $chan "     *"
    puts $chan "     * Output:"
    puts $chan "     *   *ierr -- error status. 0 means everything is OK."
    puts $chan "     *            If something goes wrong, the function is expected"
    puts $chan "     *            to print an informative error message to stdout"
    puts $chan "     *            and set *ierr to a non-zero value."
    puts $chan "     * Returns:"
    puts $chan "     *   The function value at given coordinates with given parameters."
    puts $chan "     */"
    puts $chan "    return ?;"
    puts $chan "}"
    return
}

############################################################################

proc ::hs::C_grad_template {chan fname} {
    puts $chan "int ${fname}(double x, double y, double z, int mode,"
    puts $chan "	   const double *pars, double *grad)"
    puts $chan "{"
    puts $chan "    /* Input arguments:"
    puts $chan "     *   x     -- the first variable"
    puts $chan "     *   y     -- the second variable (may be ignored)"
    puts $chan "     *   z     -- the third variable (may be ignored)"
    puts $chan "     *   mode  -- an external parameter which will not"
    puts $chan "     *            be changed by Minuit (may be ignored)"
    puts $chan "     *   pars  -- array of fitted parameters"
    puts $chan "     *"
    puts $chan "     * Output:"
    puts $chan "     *   grad  -- array of partial derivatives over parameters"
    puts $chan "     *"
    puts $chan "     * Returns:"
    puts $chan "     *   Error status. 0 means everything is OK."
    puts $chan "     */"
    puts $chan "    grad[0] = ;"
    puts $chan "    return 0;"
    puts $chan "}"
    return
}

############################################################################

proc ::hs::C_aux_template {chan fname init_or_cleanup} {
    if {[string equal $init_or_cleanup "init"]} {
	set init 1
    } elseif {[string equal $init_or_cleanup "cleanup"]} {
	set init 0
    } else {
	error "invalid argument \"$init_or_cleanup\",\
		expected \"init\" or \"cleanup\""
    }
    puts $chan "int ${fname}(const int *pmode)"
    puts $chan "{"
    puts $chan "    /* Input arguments:"
    puts $chan "     *   *pmode  -- operation mode (may be ignored)"
    puts $chan "     *"
    puts $chan "     * Returns:"
    if {$init} {
	puts $chan "     *   Initialization error status. 0 means OK."
    } else {
	puts $chan "     *   Cleanup error status. 0 means OK."
    }
    puts $chan "     */"
    puts $chan "    return 0;"
    puts $chan "}"
    return
}

#########################################################################

proc ::hs::function_browser {args} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }

    # Some default settings
    set default_xmin -5.0
    set default_xmax 5.0
    set use_histos 0
    set linestyle 1
    set default_points 0
    set default_function ""

    # Check the options
    if {[expr [llength $args] % 2] != 0} {
	error "wrong # of arguments"
    }
    set option_list {}
    foreach {opt value} $args {
	set pass_to_overlay 1
	switch -- $opt {
	    -histo {
		if {[catch {
		    if {$value} {
			set use_histos 1
		    } else {
			set use_histos 0
		    }
		}]} {
		    error "expected a boolean value for\
			    the \"-histo\" option, got \"$value\""
		}
		continue
	    }
	    -line {
		if {![string is integer $value]} {
		    error "invalid line style \"$value\""
		}
		set linestyle $value
	    }
	    -function {
		if {[lsearch -exact [hs::function_list] $value] < 0} {
		    error "function tag \"$value\" not found"
		}
		set default_function $value
		set pass_to_overlay 0
	    }
	    -fill {
		variable Histogram_fill_styles
		if {[lsearch -exact $Histogram_fill_styles $value] < 0} {
		    if {[string is integer $value]} {
			if {$value < 0 || $value >= \
				[llength $Histogram_fill_styles]} {
			    error "fill style $value is out of range"
			} else {
			    set value [lindex $Histogram_fill_styles $value]
			}
		    } else {
			error "invalid fill style \"$value\""
		    }
		}
	    }
	    -fillcolor -
	    -color {
		# Pass the option as it is
	    }
	    -marker {
		set value [hs::Validate_marker_style $value]
	    }
	    -markersize {
		set value [hs::Validate_marker_size $value]
	    }
	    -plotpoints {
		if {![hs::Validate_string integer >1 $value]} {
		    error "Expected an integer value larger than 1\
			    for the -plotpoints option, got \"$value\""
		}
		set default_points $value
		set pass_to_overlay 0
	    }
	    -xmin {
		if {![string is double -strict $value]} {
		    error "Expected a double value\
			    for the -xmin option, got \"$value\""
		}
		set default_xmin $value
		set pass_to_overlay 0
	    }
	    -xmax {
		if {![string is double -strict $value]} {
		    error "Expected a double value\
			    for the -xmax option, got \"$value\""
		}
		set default_xmax $value
		set pass_to_overlay 0
	    }
	    default {
		error "Invalid option \"$opt\". Valid options are\
			-color, -fill, -fillcolor, -function, -histo, -line,\
			-marker, -markersize, -plotpoints, -xmin, and -xmax."
	    }
	}
	if {$pass_to_overlay} {
	    lappend option_list $opt $value
	}
    }
    if {$default_xmin == $default_xmax} {
	error "plot range is empty"
    }
    if {$default_xmax < $default_xmin} {
	set kjhgkjkhHFDjk $default_xmax
	set default_xmax $default_xmin
	set default_xmin $kjhgkjkhHFDjk
	unset kjhgkjkhHFDjk
    }
    if {$use_histos} {
	set invalid_options {-marker -markersize}
	set nlines 18
	if {$default_points == 0} {
	    set default_points 1000
	}
    } else {
	set invalid_options {-fill -fillcolor}
	set nlines 13
	if {$default_points == 0} {
	    set default_points 500
	}
    }
    if {$linestyle < 0 || $linestyle >= $nlines} {
	error "line style $linestyle is out of range"
    }
    foreach {opt value} $option_list {
	if {[lsearch -exact $invalid_options $opt] >= 0} {
	    error "Option $opt requires the use of\
		    \"-histo [expr {!$use_histos}]\" switch"
	}
    }

    # Create the toplevel window
    variable Hs_function_browser_counter
    set status 1
    while {$status} {
	set browser_id [incr Hs_function_browser_counter]
	set topwin .hs_function_browser_$browser_id
	set status [catch {toplevel $topwin}]
    }
    wm withdraw $topwin
    set window_title "hs::function_browser $browser_id"
    wm title $topwin $window_title

    # Function chooser
    frame $topwin.fchooser
    pack $topwin.fchooser -padx 4 -pady 4 -fill x -side top
    label $topwin.fchooser.l -text "Choose function: "
    set buttonwidth 8
    menubutton $topwin.fchooser.bn -text "By name" -relief raised \
	    -width $buttonwidth -menu $topwin.fchooser.bn.m
    menu $topwin.fchooser.bn.m -tearoff 0 -postcommand [list \
	    hs::Post_function_menu 0 $browser_id $topwin.fchooser.bn.m]
    menubutton $topwin.fchooser.bt -text "By tag" -relief raised \
	    -width $buttonwidth -menu $topwin.fchooser.bt.m
    menu $topwin.fchooser.bt.m -tearoff 0 -postcommand [list \
	    hs::Post_function_menu 1 $browser_id $topwin.fchooser.bt.m]

    # Basic function properties
    set entrywidth 5
    label $topwin.fchooser.lname -text "Name:"
    label $topwin.fchooser.name -text "" -anchor w
    label $topwin.fchooser.ltag -text "Tag:"
    label $topwin.fchooser.tag -text "" -anchor w
    label $topwin.fchooser.lmode -text "Mode:"
    entry $topwin.fchooser.mode -width $entrywidth
    bind $topwin.fchooser.mode <KeyPress-Return> \
	    [list hs::Function_browser_mode_set $browser_id %W]
    label $topwin.fchooser.lxmin -text "X min:"
    entry $topwin.fchooser.xmin -width $entrywidth
    $topwin.fchooser.xmin insert end $default_xmin
    bind $topwin.fchooser.xmin <KeyPress-Return> \
	    [list hs::Function_browser_x_set $browser_id %W xmin]
    label $topwin.fchooser.lxmax -text "  X max:"
    entry $topwin.fchooser.xmax -width $entrywidth
    $topwin.fchooser.xmax insert end $default_xmax
    bind $topwin.fchooser.xmax <KeyPress-Return> \
	    [list hs::Function_browser_x_set $browser_id %W xmax]
    label $topwin.fchooser.lnpoints -text "Points:"
    entry $topwin.fchooser.npoints -width $entrywidth
    $topwin.fchooser.npoints insert end $default_points
    bind $topwin.fchooser.npoints <KeyPress-Return> \
	    [list hs::Function_browser_point_set $browser_id %W]
    foreach entryname [list $topwin.fchooser.mode $topwin.fchooser.xmin \
	    $topwin.fchooser.xmax $topwin.fchooser.npoints] {
	$entryname configure -validate key -vcmd {
	    %W configure -foreground red2
	    return 1
	}
    }
    set row 0
    grid $topwin.fchooser.l -row $row -column 0 -columnspan 2 -sticky w
    grid $topwin.fchooser.bt -row $row -column 2 -columnspan 2 -sticky ew
    grid $topwin.fchooser.bn -row $row -column 4 -columnspan 2 -sticky ew
    incr row
    grid $topwin.fchooser.lname -row $row -column 0 -sticky e
    grid $topwin.fchooser.name -row $row -column 1 -columnspan 5 -sticky w
    incr row
    grid $topwin.fchooser.ltag -row $row -column 0 -sticky e
    grid $topwin.fchooser.tag -row $row -column 1 -columnspan 3 -sticky w
    grid $topwin.fchooser.lmode -row $row -column 4 -sticky e
    grid $topwin.fchooser.mode -row $row -column 5 -sticky w
    incr row
    foreach {name col sticky} {
	lnpoints 0 e
	npoints  1 w
	lxmin    2 e
	xmin     3 w
	lxmax    4 e
	xmax     5 w
    } {
	grid $topwin.fchooser.$name -row $row -column $col -sticky $sticky
    }
    for {set i 0} {$i <= $row} {incr i} {
	grid rowconfigure $topwin.fchooser $i -pad 2
    }

    # Separator and main buttons
    ::hs::Horizontal_separator $topwin
    frame $topwin.buttons
    pack $topwin.buttons -side top -fill x -padx 4 -pady 4
    set buttonwidth 9
    button $topwin.buttons.plot -text "New plot" -width $buttonwidth -command \
	    [list hs::Function_browser_newplot $browser_id]
    button $topwin.buttons.snap -text "Snapshot" -width $buttonwidth -command \
	    [list hs::Function_browser_snapshot $browser_id]
    # button $topwin.buttons.browse -text "New browser" -width $buttonwidth -command \
    #	    [concat hs::function_browser $args]
    button $topwin.buttons.pars -text "Parameters" -width $buttonwidth -command \
	    [list hs::Function_browser_print_params $browser_id]
    button $topwin.buttons.defaults -text "Defaults" -width $buttonwidth -command \
	    [list hs::Function_browser_default_pars $browser_id]
    button $topwin.buttons.descr -text "Description" -width $buttonwidth -command \
	    [list hs::Function_browser_print_descr $browser_id]
    button $topwin.buttons.dismiss -text "Dismiss" -width $buttonwidth -command \
	    [list hs::Function_browser_destroy $browser_id $topwin]
    foreach {name row col} {
	plot     0 0
	snap     0 1
	descr    0 2
	pars     1 0
	defaults 1 1
	dismiss  1 2
    } {
	grid $topwin.buttons.$name -row $row -column $col -padx 1 -sticky ew
    }
    foreach row {0 1} {
	grid rowconfigure $topwin.buttons $row -weight 1 -pad 2
    }
    foreach col {0 1 2} {
	grid columnconfigure $topwin.buttons $col -weight 1 -pad 2
    }

    # Initialize data structures
    variable Hs_function_browser_data
    set Hs_function_browser_data($browser_id,tag) ""
    set Hs_function_browser_data($browser_id,hs_id) 0
    set Hs_function_browser_data($browser_id,idlist) {}
    set Hs_function_browser_data($browser_id,wincount) 0
    set Hs_function_browser_data($browser_id,xmin) $default_xmin
    set Hs_function_browser_data($browser_id,xmax) $default_xmax
    set Hs_function_browser_data($browser_id,npoints) $default_points
    set Hs_function_browser_data($browser_id,args) $option_list
    set Hs_function_browser_data($browser_id,histo) $use_histos

    # Set the default function
    if {![string equal $default_function ""]} {
	hs::Function_browser_choose $browser_id $default_function 1
    }

    # Update the geometry and draw the toplevel window 
    update idletasks
    wm deiconify $topwin
    wm protocol $topwin WM_DELETE_WINDOW \
	    [list hs::Function_browser_destroy $browser_id $topwin]
    wm resizable $topwin 1 0
    return $topwin
}

#########################################################################

proc ::hs::Function_browser_default_pars {browser_id} {
    set tag [hs::Function_browser_verify_tag $browser_id]
    hs::Function_browser_choose $browser_id $tag 1
    return
}

#########################################################################

proc ::hs::Function_browser_newplot {browser_id} {
    set tag [hs::Function_browser_verify_tag $browser_id]
    variable Hs_function_browser_data
    set i $Hs_function_browser_data($browser_id,wincount)
    set overlay Hs_function_browser_overlay_${browser_id}_$i
    set win Hs_function_browser_window_${browser_id}_$i
    set options $Hs_function_browser_data($browser_id,args)
    if {$Hs_function_browser_data($browser_id,histo)} {
	# Make a new histogram
	foreach name {xmin xmax npoints} {
	    set $name $Hs_function_browser_data($browser_id,$name)
	}
	set hs_id [hs::create_1d_hist [hs::Temp_uid] \
		"Function plot" [hs::Temp_category] \
		"X" "Function" $npoints $xmin $xmax]
	if {$hs_id <= 0} {error "Failed to create a plot histogram"}
	if {[catch {eval hs::overlay [list $overlay] show \
		-window [list $win] add $hs_id $options} ermess]} {
	    hs::delete $hs_id
	    error $ermess
	}
	lappend Hs_function_browser_data($browser_id,idlist) $hs_id
    } else {
	# Make a new ntuple
	set hs_id [hs::create_ntuple [hs::Temp_uid] \
		"Function plot" [hs::Temp_category] {X Function}]
	if {$hs_id <= 0} {error "Failed to create a plot ntuple"}
	hs::allow_reset_refresh $hs_id 0
	if {[catch {eval hs::overlay [list $overlay] show -window [list $win] \
		add $hs_id xy -x X -y Function -owner 1 $options} ermess]} {
	    hs::delete $hs_id
	    error $ermess
	}
    }
    set Hs_function_browser_data($browser_id,hs_id) $hs_id
    incr Hs_function_browser_data($browser_id,wincount)
    hs::Function_browser_redraw_current $browser_id
    return
}

#########################################################################

proc ::hs::Function_browser_print_descr {browser_id} {
    set tag [hs::Function_browser_verify_tag $browser_id]
    puts "\nTag: $tag"
    puts "Name: [hs::function $tag cget -name]"
    puts [hs::function $tag cget -description]
    return
}

#########################################################################

proc ::hs::Function_browser_print_params {browser_id} {
    hs::Function_browser_verify_tag $browser_id
    set pvalues [hs::Function_browser_pvalues $browser_id]
    if {[llength $pvalues] > 0} {
	puts $pvalues
    }
    return
}

#########################################################################

proc ::hs::Function_browser_pvalues {browser_id} {
    set pvalues {}
    variable Hs_function_browser_data
    set tag $Hs_function_browser_data($browser_id,tag)
    if {[hs::function $tag exists]} {
	foreach pname [hs::function $tag cget -usedpars] {
	    lappend pvalues [list $pname \
		    $Hs_function_browser_data($browser_id,pvalues,$pname)]
	}
    }
    set pvalues
}

#########################################################################

proc ::hs::Function_browser_verify_tag {browser_id} {
    variable Hs_function_browser_data
    set tag $Hs_function_browser_data($browser_id,tag)
    set topwin .hs_function_browser_$browser_id
    if {[string equal $tag ""]} {
	tk_dialog $topwin.warn_dialog "Warning"\
		"Please choose a function first"\
		warning 0 OK
	return -code return
    }
    if {![hs::function $tag exists]} {
	tk_dialog $topwin.warn_dialog "Warning"\
		"Function tag \"$tag\" is no longer valid"\
		warning 0 Acknowledged
	return -code return
    }
    set tag
}

#########################################################################

proc ::hs::Function_browser_snapshot {browser_id} {
    set tag [hs::Function_browser_verify_tag $browser_id]
    variable Hs_function_browser_data
    set hs_id $Hs_function_browser_data($browser_id,hs_id)
    if {$hs_id <= 0} return
    set new_id [hs::copy_hist $hs_id [hs::Temp_uid] \
	    [hs::title $hs_id] [hs::Temp_category]]
    if {$new_id <= 0} {error "Failed to copy a plot"}
    set i $Hs_function_browser_data($browser_id,wincount)
    set overlay Hs_function_browser_overlay_${browser_id}_$i
    set win Hs_function_browser_window_${browser_id}_$i
    set options $Hs_function_browser_data($browser_id,args)

    global ::errorInfo
    if {[catch {
	set wintitle "$tag at [hs::Function_browser_pvalues $browser_id]"
	if {[string length $wintitle] > 159} {
	    set wintitle [string range $wintitle 0 155]
	    append wintitle "..."
	}
	if {$Hs_function_browser_data($browser_id,histo)} {
	    # This is a histogram
	    eval hs::overlay [list $overlay] show \
		    -window [list $win] -title [list $wintitle] \
		    add $new_id $options
	    lappend Hs_function_browser_data($browser_id,idlist) $new_id
	} else {
	    # This is an Ntuple
	    foreach {xname yname} [hs::ntuple_variable_list $new_id] {}
	    eval hs::overlay [list $overlay] show \
		    -window [list $win] -title [list $wintitle] \
		    add $new_id xy -x [list $xname] -y [list $yname] \
		    -owner 1 $options
	}
    } ermess]} {
	set savedInfo $::errorInfo
	hs::delete $new_id
	error $ermess $savedInfo
    }

    incr Hs_function_browser_data($browser_id,wincount)
    return
}

#########################################################################

proc ::hs::Function_browser_redraw_current {browser_id} {
    variable Hs_function_browser_data
    foreach name {tag hs_id xmin xmax npoints histo} {
	set $name $Hs_function_browser_data($browser_id,$name)
    }
    if {![hs::function $tag exists]} return
    if {$hs_id <= 0} return
    if {$histo} {
	set result_specifier [list $hs_id x]
    } else {
	set result_specifier [list $hs_id [list x $xmin $xmax $npoints]]
    }
    eval hs::function [list $tag] scan [list $result_specifier] \
	    [hs::Function_browser_pvalues $browser_id]
    hs::hs_update
    return
}

#########################################################################

proc ::hs::Function_browser_x_set {browser_id entry which} {
    set value [$entry get]
    if {[string is double -strict $value]} {
	variable Hs_function_browser_data
	set Hs_function_browser_data($browser_id,$which) $value
	$entry configure -foreground black
	hs::Function_browser_redraw_current $browser_id
    }
    return
}

#########################################################################

proc ::hs::Function_browser_adjust_parameter {browser_id parname newvalue} {
    variable Hs_function_browser_data
    set Hs_function_browser_data($browser_id,pvalues,$parname) $newvalue
    hs::Function_browser_redraw_current $browser_id
    return
}

#########################################################################

proc ::hs::Function_browser_point_set {browser_id entry} {
    set value [$entry get]
    if {[string is integer -strict $value]} {
	if {$value > 1} {
	    variable Hs_function_browser_data
	    set Hs_function_browser_data($browser_id,npoints) $value
	    $entry configure -foreground black
	    set hs_id $Hs_function_browser_data($browser_id,hs_id)
	    if {[string equal [hs::type $hs_id] "HS_NTUPLE"]} {
		hs::allow_reset_refresh $hs_id 1
		hs::reset $hs_id
		hs::hs_update
		hs::allow_reset_refresh $hs_id 0
	    }
	    hs::Function_browser_redraw_current $browser_id
	}
    }
    return
}

#########################################################################

proc ::hs::Function_browser_mode_set {browser_id entry} {
    set tag [hs::Function_browser_verify_tag $browser_id]
    set value [$entry get]
    if {[string is integer -strict $value]} {
	# Check that parameters stay the same after the mode change
	set parlist_old [lsort [hs::function $tag cget -usedpars]]
	hs::function $tag configure -mode $value
	set topwin .hs_function_browser_$browser_id
	$topwin.fchooser.mode configure -foreground black
	set parlist_new [lsort [hs::function $tag cget -usedpars]]
	if {![string equal $parlist_old $parlist_new]} {
	    hs::Function_browser_choose $browser_id $tag
	} else {
	    hs::Function_browser_redraw_current $browser_id
	}
    }
    return
}

#########################################################################

proc ::hs::Post_function_menu {bytag browser_id widget} {

    variable Hs_function_type

    set type_attributes {
	pdf "Probability densities"
	thresh "Threshold / cdfs"
	poly "Polynomials"
	trivial "Trivial functions"
	misc "Miscellaneous"
	user "User-defined"
    }
    array set type_names $type_attributes

    $widget delete 0 end
    foreach tag [hs::function_list] {
	if {[hs::function $tag cget -ndim] == 1} {
	    set signature [hs::Function_signature $tag]
	    if {[info exists Hs_function_type($signature)]} {
		set type $Hs_function_type($signature)
	    } else {
		set type user
	    }
	    if {![info exists type_names($type)]} {
		error "Internal error: unknown function type \"$type\".\
			This is a bug. Please report."
	    }
	    lappend function_list($type) [list [hs::function $tag cget -name] $tag]
	}
    }
    foreach {type text} $type_attributes {
	if {[info exists function_list($type)]} {
	    $widget add cascade -menu $widget.$type -label $text
	    catch {destroy $widget.$type}
	    set m [menu $widget.$type -tearoff 0]
	    foreach pair [lsort -index $bytag $function_list($type)] {
		foreach {name tag} $pair {}
		$m add command -label [lindex $pair $bytag] -command \
			[list hs::Function_browser_choose $browser_id $tag]
	    }
	}
    }

    return
}

#########################################################################

proc ::hs::Function_browser_choose {browser_id tag {use_defaults 0}} {
    set topwin .hs_function_browser_$browser_id
    $topwin.fchooser.tag configure -text $tag -background white
    $topwin.fchooser.name configure -background white \
	    -text [hs::function $tag cget -name]
    variable Hs_function_browser_data
    set oldtag $Hs_function_browser_data($browser_id,tag)
    set parameter_frame $topwin.param
    if {[hs::function $oldtag exists]} {
	catch {set Hs_function_browser_data($browser_id,oldspecs,$oldtag) \
		[hs::Scale_pack_specs $parameter_frame]}
    }
    set Hs_function_browser_data($browser_id,tag) $tag
    $topwin.fchooser.mode delete 0 end
    $topwin.fchooser.mode insert end [hs::function $tag cget -mode]
    $topwin.fchooser.mode configure -foreground black

    # Figure out the parameter scale specs
    set scale_specs {}
    if {[hs::function $tag cget -npars] > 0} {
	# Try to find previously remembered parameter settings
	if {!$use_defaults} {
	    if {[info exists Hs_function_browser_data($browser_id,oldspecs,$tag)]} {
		set old_specs $Hs_function_browser_data($browser_id,oldspecs,$tag)
		# Check that parameter names are still the same
		set parnames {}
		foreach {parname dumm min max init} $old_specs {
		    lappend parnames $parname
		}
		if {[llength $parnames] == [hs::function $tag cget -npars]} {
		    if {[llength $parnames] == [llength [lsort -unique \
			    [concat $parnames [hs::function $tag cget -usedpars]]]]} {
			set scale_specs $old_specs
		    }
		}
	    }
	}
	# Try to find the default parameter settings
	if {[llength $scale_specs] == 0} {
	    variable Hs_default_parameter_range
	    set signature [hs::Function_signature $tag]
	    if {[info exists Hs_default_parameter_range($signature)]} {
		foreach {parname min max init} $Hs_default_parameter_range($signature) {
		    lappend scale_specs $parname $parname $min $max $init
		}
	    }
	}
	# Make up some dummy settings if there are none found
	if {[llength $scale_specs] == 0} {
	    if {[string equal $tag "poly_1d"]} {
		foreach parname [hs::function $tag cget -usedpars] {
		    lappend scale_specs $parname $parname -10 10 1
		}
	    } elseif {[string equal $tag "poly_cheb"]} {
		lappend scale_specs min min -10 10 -4
		lappend scale_specs max max -10 10 4
		foreach parname [lrange [hs::function $tag cget -usedpars] 2 end] {
		    lappend scale_specs $parname $parname -10 10 1
		}
	    } elseif {[string equal $tag "poly_legendre"]} {
		lappend scale_specs min min -10 10 -4
		lappend scale_specs max max -10 10 4
		foreach parname [lrange [hs::function $tag cget -usedpars] 2 end] {
		    lappend scale_specs $parname $parname -10 10 1
		}
	    } else {
		foreach parname [hs::function $tag cget -usedpars] {
		    lappend scale_specs $parname $parname 0 1 0.5
		}
		# We have no idea if the settings make any sense.
		# Disable the automatic redrawing of the plot.
		set Hs_function_browser_data($browser_id,hs_id) 0
	    }
	}
    }

    # Check mode settings for sensitive functions
    if {[string equal $tag "data_1d"]} {
	if {[hs::function $tag cget -mode] <= 0} {
	    # The data item in the function is not set.
	    # Disable the automatic redrawing of the plot.
	    set Hs_function_browser_data($browser_id,hs_id) 0
	}
    }

    # Set the parameter values
    array unset Hs_function_browser_data $browser_id,pvalues,*
    foreach {parname dumm pmin pmax pvalue} $scale_specs {
	set Hs_function_browser_data($browser_id,pvalues,$parname) $pvalue
    }

    # Build the scales
    hs::Scale_pack_destroy $parameter_frame
    hs::Scale_pack $parameter_frame 0 red2 $scale_specs [list \
	    hs::Function_browser_adjust_parameter $browser_id]
    pack $parameter_frame -side top -after $topwin.fchooser \
	    -padx 4 -pady 4 -fill x
    update idletasks

    # Finally, attempt to redraw the active plot
    hs::Function_browser_redraw_current $browser_id
    return
}

#########################################################################

proc ::hs::Function_browser_destroy {browser_id w} {
    variable Hs_function_browser_data
    for {set i 0} {$i < $Hs_function_browser_data($browser_id,wincount)} {incr i} {
	set win Hs_function_browser_window_${browser_id}_$i
	hs::close_window $win 1
	set overlay Hs_function_browser_overlay_${browser_id}_$i
	hs::overlay $overlay clear
    }
    foreach id $Hs_function_browser_data($browser_id,idlist) {
	hs::delete $id
    }
    array unset Hs_function_browser_data $browser_id,*
    destroy $w
    hs::Scale_pack_destroy $w.param
    return
}

#########################################################################

proc ::hs::Scale_pack {w scalewidth typed_color scale_specs update_command} {

    # Scale pack widget. The widget data structures use globals,
    # so it can be easily reused in non-hs applications just by
    # renaming the command names (and updating the relevant
    # "-command" switches and bindings). The user interface
    # consists of the following commands:
    #
    # 1) hs::Scale_pack w scalewidth typed_color scale_specs update_command
    #       w              -- the window path
    #       scalewidth     -- the width of the scales. Set to 0 in order
    #                         to control the width by the geometry manager.
    #       typed_color    -- the color to which the entry foreground changes
    #                         when the user types into the entry
    #       scale_specs    -- a flat list of scale specifications. Each
    #                         scale uses up 5 elements: string scale id,
    #                         string label text, doubles scale lower limit,
    #                         upper limit, and initial value. All scale ids
    #                         in the list must be different.
    #       update_command -- a script which will be called when the scale
    #                         or entry are set. The scale id and the new
    #                         scale value will be apended.
    #    This command returns the widget window path.
    #
    # 2) hs::Scale_pack_set w scale_id value ?from? ?to?
    #       w              -- the window path of an existing widget
    #       scale_id       -- the scale id
    #       value          -- new scale value. May be specified as an
    #                         empty string if the user wants to change
    #                         only the scale limits.
    #       from, to       -- scale limits (optional)
    #    Sets the scale value and/or limits. Returns nothing.
    #
    # 3) hs::Scale_pack_ids w
    #       w              -- the window path of an existing widget
    #    Returns the list of scale ids.
    #
    # 4) hs::Scale_pack_get w scale_id
    #       w              -- the window path of an existing widget
    #       scale_id       -- the scale id
    #    Returns the current value of the given scale.
    #
    # 5) hs::Scale_pack_limits w scale_id
    #       w              -- the window path of an existing widget
    #       scale_id       -- the scale id
    #    Returns the scale limits as a list {from to}.
    #
    # 6) hs::Scale_pack_specs w
    #       w              -- the window path of an existing widget
    #    Returns the current state of the scale pack in the form
    #    which can be fed back into the hs::Scale_pack command as
    #    the "scale_specs" argument.
    #
    # 7) hs::Scale_pack_destroy w
    #    Should be called when the scale pack is no longer needed.
    #
    # Internal commands include hs::Scale_pack_entry_set and
    # hs::Scale_pack_scale_movement.

    # Some parameters
    set entrywidth 10
    set scale_divisions 1000
    set repeat_interval 1
    
    # Check the scale specifications
    if {[expr [llength $scale_specs] % 5] != 0} {
	error "bad number of elements in the scale specification list"
    }
    set taglist {}
    foreach {label labeltext llim rlim initvalue} $scale_specs {
	lappend taglist $label
	foreach value [list $llim $rlim $initvalue] {
	    if {![string is double -strict $value]} {
		error "expected a double, got \"$value\""
	    }
	}
    }
    if {[llength $taglist] != [llength [lsort -unique $taglist]]} {
	error "all scale ids must be distinct"
    }

    # Make sure we can create the widget frame
    frame $w

    # Init some data
    global _Hs_scale_pack_data
    set _Hs_scale_pack_data($w,_,command) $update_command
    set _Hs_scale_pack_data($w,_,divisions) [expr {1.0 * $scale_divisions}]
    set _Hs_scale_pack_data($w,_,labels) $taglist
    
    # Create the scales. Use globals rather than variables
    # so that this widget can be easily transferred into 
    # a non-hs application.
    set row 0
    foreach {label labeltext llim rlim initvalue} $scale_specs {
	set varname _Hs_scale_pack_scale_${w}_$row
	set radioname _Hs_scale_pack_radio_${w}_$row
	global $varname $radioname

	# Create the labels
	label $w.l$row -text $labeltext

	# Scales
	if {$llim < $rlim} {
	    if {$initvalue < $llim} {set llim $initvalue}
	    if {$initvalue > $rlim} {set rlim $initvalue}
	} else {
	    if {$initvalue < $rlim} {set rlim $initvalue}
	    if {$initvalue > $llim} {set llim $initvalue}
	}
	scale $w.s$row -orient horizontal -from $llim -to $rlim \
		-resolution [expr {abs(($rlim - $llim)/\
		$_Hs_scale_pack_data($w,_,divisions))}] \
		-digits 0 -showvalue 0 -repeatinterval $repeat_interval \
		-command [list hs::Scale_pack_scale_movement \
		1 $w $label $row]
        bind $w.s$row <ButtonPress-1> +[list focus %W]
        bind $w.s$row <KeyPress-Up> break
        bind $w.s$row <KeyPress-Down> break
	if {$scalewidth > 0} {
	    $w.s$row configure -length $scalewidth 
	}
	$w.s$row set $initvalue

	# Entries to show/change the scale positions
	entry $w.e$row -relief groove -width $entrywidth \
		-foreground black -textvariable $varname
	set $varname $initvalue
	$w.e$row configure -validate key -vcmd \
		"[list $w.e$row] configure -foreground\
		[list $typed_color]; return 1"

	# Initialize parameter data
	set _Hs_scale_pack_data($w,_,pvalues,$label) $initvalue

	# Radiobuttons to indicate that we want to change scale limits
	radiobutton $w.from$row -text "\[" -indicatoron 0 \
		-variable $radioname -value -1 -selectcolor green2
	radiobutton $w.to$row -text "\]" -indicatoron 0 \
		-variable $radioname -value 1  -selectcolor green2
	set $radioname 0

	grid $w.l$row -row $row -column 0 -sticky e
	grid $w.s$row -row $row -column 1 -sticky ew
	grid $w.from$row -row $row -column 2 -sticky w
	grid $w.e$row -row $row -column 3 -sticky w
	grid $w.to$row -row $row -column 4 -sticky w

	bind $w.e$row <KeyPress-Return> \
		[list hs::Scale_pack_entry_set $w $label $row]
	incr row
    }
    set _Hs_scale_pack_data($w,_,nrows) $row
    if {$row > 0} {
	grid columnconfigure $w 1 -weight 1
    }
    return $w
}

#########################################################################

proc ::hs::Scale_pack_set {w scale_id newvalue {llim {}} {rlim {}}} {
    global _Hs_scale_pack_data
    if {![info exists _Hs_scale_pack_data($w,_,labels)]} {
	error "Scale pack widget named \"$w\" does not exist"
    }
    set row [lsearch -exact $_Hs_scale_pack_data($w,_,labels) $scale_id]
    if {$row < 0} {
	error "Invalid scale id \"$scale_id\" for scale pack $w. Valid\
		scale ids are [join $_Hs_scale_pack_data($w,_,labels) {, }]."
    }
    foreach value [list $newvalue $llim $rlim] {
	if {![string is double $value]} {
	    error "expected a double or an empty string, got \"$value\""
	}
    }
    if {![string equal $llim ""] || ![string equal $rlim ""]} {
	# Limits are going to change
	if {[string equal $llim ""]} {
	    set llim [$w.s$row cget -from]
	} else {
	    $w.s$row configure -from $llim
	}
	if {[string equal $rlim ""]} {
	    set rlim [$w.s$row cget -to]
	} else {
	    $w.s$row configure -to $rlim
	}
	$w.s$row configure -resolution [expr {abs(($rlim - $llim)/\
		$_Hs_scale_pack_data($w,_,divisions))}]
    }
    if {![string equal $newvalue ""]} {
	set varname _Hs_scale_pack_scale_${w}_$row
	set radioname _Hs_scale_pack_radio_${w}_$row
	global $varname $radioname
	set $radioname 0
	set $varname $newvalue
	hs::Scale_pack_entry_set $w $scale_id $row
    }
    return
}

#########################################################################

proc ::hs::Scale_pack_scale_movement {skip_one w label row newvalue} {
    if {$skip_one} {
	$w.s$row configure -command [list \
		hs::Scale_pack_scale_movement 0 $w $label $row]
	return
    }
    set varname _Hs_scale_pack_scale_${w}_$row
    set radioname _Hs_scale_pack_radio_${w}_$row
    global $varname $radioname
    set $radioname 0
    set $varname $newvalue
    $w.e$row configure -foreground black
    global _Hs_scale_pack_data
    if {$newvalue != $_Hs_scale_pack_data($w,_,pvalues,$label)} {
	set _Hs_scale_pack_data($w,_,pvalues,$label) $newvalue
	if {![string equal $_Hs_scale_pack_data($w,_,command) ""]} {
	    eval $_Hs_scale_pack_data($w,_,command) [list $label] $newvalue
	}
    }
    return
}

#########################################################################

proc ::hs::Scale_pack_entry_set {w label row} {
    set varname _Hs_scale_pack_scale_${w}_$row
    set radioname _Hs_scale_pack_radio_${w}_$row
    global $varname $radioname
    set newvalue [set $varname]
    if {[string is double -strict $newvalue]} {
	global _Hs_scale_pack_data
	set radiovalue [set $radioname]
	set llim [$w.s$row cget -from]
	set rlim [$w.s$row cget -to]
	set within_limits 0
	if {$llim < $rlim} {
	    if {$newvalue >= $llim && $newvalue <= $rlim} {
		set within_limits 1
	    }
	} else {
	    if {$newvalue >= $rlim && $newvalue <= $llim} {
		set within_limits 1
	    }
	}
	if {$radiovalue || !$within_limits} {
	    # Will have to change the scale limits
	    if {$radiovalue < 0} {
		set option from
	    } elseif {$radiovalue > 0} {
		set option to
	    } elseif {$llim < $rlim} {
		if {$newvalue < $llim} {
		    set option from
		} else {
		    set option to
		}
	    } else {
		if {$newvalue > $llim} {
		    set option from
		} else {
		    set option to
		}
	    }
	    $w.s$row configure -$option $newvalue
	    if {[string equal option "from"]} {
		set llim $newvalue
	    } else {
		set rlim $newvalue
	    }
	    $w.s$row configure -resolution [expr {abs(($rlim - $llim)/\
		    $_Hs_scale_pack_data($w,_,divisions))}]
	}
	$w.s$row configure -command [list \
		hs::Scale_pack_scale_movement 1 $w $label $row]
	$w.s$row set $newvalue
	set $radioname 0
	$w.e$row configure -foreground black
	set _Hs_scale_pack_data($w,_,pvalues,$label) $newvalue
	if {![string equal $_Hs_scale_pack_data($w,_,command) ""]} {
	    eval $_Hs_scale_pack_data($w,_,command) [list $label] $newvalue
	}
    }
    return
}

#########################################################################

proc ::hs::Scale_pack_destroy {w} {
    global _Hs_scale_pack_data
    if {![info exists _Hs_scale_pack_data($w,_,nrows)]} return
    catch {destroy $w}
    for {set row 0} {$row < $_Hs_scale_pack_data($w,_,nrows)} {incr row} {
	global _Hs_scale_pack_scale_${w}_$row _Hs_scale_pack_radio_${w}_$row
	unset _Hs_scale_pack_scale_${w}_$row _Hs_scale_pack_radio_${w}_$row
    }
    array unset _Hs_scale_pack_data $w,_,*
    return
}

#########################################################################

proc ::hs::Scale_pack_ids {w} {
    global _Hs_scale_pack_data
    set _Hs_scale_pack_data($w,_,labels)
}

#########################################################################

proc ::hs::Scale_pack_get {w scale_id} {
    global _Hs_scale_pack_data
    set _Hs_scale_pack_data($w,_,pvalues,$scale_id)
}

#########################################################################

proc ::hs::Scale_pack_specs {w} {
    global _Hs_scale_pack_data
    if {![info exists _Hs_scale_pack_data($w,_,labels)]} {
	error "Scale pack widget named \"$w\" does not exist"
    }
    set scale_specs {}
    set row 0
    foreach scale_id $_Hs_scale_pack_data($w,_,labels) {
	lappend scale_specs $scale_id [$w.l$row cget -text] \
		[$w.s$row cget -from] [$w.s$row cget -to] \
		$_Hs_scale_pack_data($w,_,pvalues,$scale_id)
	incr row
    }
    set scale_specs
}

#########################################################################

proc ::hs::Scale_pack_limits {w scale_id} {
    global _Hs_scale_pack_data
    if {![info exists _Hs_scale_pack_data($w,_,labels)]} {
	error "Scale pack widget named \"$w\" does not exist"
    }
    set row [lsearch -exact $_Hs_scale_pack_data($w,_,labels) $scale_id]
    if {$row < 0} {
	error "Invalid scale id \"$scale_id\" for scale pack $w. Valid\
		scale ids are [join $_Hs_scale_pack_data($w,_,labels) {, }]."
    }
    list [$w.s$row cget -from] [$w.s$row cget -to]
}

#########################################################################

proc ::hs::Horizontal_separator {parent} {
    variable Horizontal_separator_number
    set f $parent._separ_[incr Horizontal_separator_number]
    frame $f -height 2 -borderwidth 1 -relief sunken
    pack $f -side top -fill x
    return
}

#########################################################################

proc ::hs::function_compile {tag name expression parameters {ndim {}}} {
    # Check that the tag can be loaded
    if {[lsearch -exact [hs::function_list] $tag] >= 0} {
	error "fit function with tag \"$tag\" already exists"
    }
    if {[string equal $tag ""]} {
	error "fit function tag may not be an empty string"
    }
    # Check that parameter names are OK
    if {[llength $parameters] != [llength [lsort -unique $parameters]]} {
	error "duplicate parameter name found"
    }
    foreach pname $parameters {
	if {![hs::Is_valid_c_identifier $pname]} {
	    error "bad parameter name \"$pname\": not a valid C variable name"
	}
	if {[lsearch -exact {x y z mode} $pname] >= 0} {
	    error "bad parameter name \"$pname\": this name is reserved"
	}
    }
    # Figure out the function dimensionality
    if {$ndim == {}} {
	foreach varname {x y z} {
	    if {[string equal $expression \
		    [::fit::Parse_fitter_pars_in_a_map \
		    $expression $varname dumm]]} {
		set has_$varname 0
	    } else {
		set has_$varname 1
	    }
	}
	set ndim 0
	if {$has_x} {set ndim 1}
	if {$has_y} {set ndim 2}
	if {$has_z} {set ndim 3}
    } elseif {![string is integer $ndim]} {
	error "Expected an integer between 0 and 3, got \"$ndim\""
    }
    if {$ndim < 0 || $ndim > 3} {
	error "Function dimensionality is out of range"
    }

    # Open file for the C code
    foreach {filename chan} [::hs::tempfile \
	    [file join [::hs::tempdir] hs_cfile_] ".c"] {}

    # Make sure the channel is closed properly
    global ::errorInfo
    set status [catch {
        # Include the standard C headers
        hs::Write_standard_c_headers $chan
        variable Compiled_function_counter
	set fname "hs_meta_fit_fun_[incr Compiled_function_counter]"
	set arrname "funct_pars_JHhgc86BJY5bG"
	puts $chan ""
	puts $chan "double ${fname}(double x, double y, double z, int mode,"
	puts $chan "	     const double *$arrname, int *ierr_I9afw7jo2jQiOu)"
	puts $chan "{"
        if {[llength $parameters]} {
            puts $chan "    double [join $parameters ,];"
        }
	set npars 0
	foreach pname $parameters {
	    puts $chan "    $pname = ${arrname}\[$npars\];"
	    incr npars
	}
	puts $chan "    return ($expression);"
	puts $chan "}"
	variable Sharedlib_suffix
	foreach {sharedlib chan1} [::hs::tempfile \
		[file join [::hs::tempdir] hs_sofile_] $Sharedlib_suffix] {}
	close $chan1
    } errstat]
    set savedInfo $::errorInfo
    close $chan
    if {$status} {
        file delete $filename
        error $errstat $savedInfo
    }

    # Compile the code
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
        file delete $sharedlib
        error "Failed to compile expression\n$expression\nCompiler\
		diagnostics:\n$errstat"
    }
    file delete $filename

    # Load the shared library
    set dll_id [hs::sharedlib open $sharedlib]
    file delete $sharedlib

    # Import the function and tell it that it owns the dll number
    set newtag [hs::function_import $tag $dll_id $name $expression \
	    $ndim 0 $npars $npars $parameters {} $fname {} {}]
    hs::Function_owns_dll $newtag 1

    return $newtag
}

############################################################################

if {[::hs::have_cernlib]} {
    proc ::hs::fit_manual {} {
	hs::Print_manual hs_fit.txt
	return
    }
}

############################################################################
# Random number generator
if {[::hs::have_cernlib]} {
    proc ::hs::random {n} {
	set tmp [hs::Ranlux $n]
	variable Random_gen_initialized 1
	set tmp
    }
    proc ::hs::random_init {n} {
	hs::Rluxgo [list 3 $n 0 0]
	variable Random_gen_initialized 1
	return
    }
    proc ::hs::random_get_state {} {
	variable Random_gen_initialized
	if {![info exists Random_gen_initialized]} {
	    error "random number generator is not initialized"
	}
	hs::Rluxut
    }
    proc ::hs::random_set_state {data} {
	hs::Rluxin $data
	return
    }
    proc ::hs::gauss_random {mean stdev {npoints ""}} {
        if {[string equal $npoints ""]} {
            set result [lindex [hs::Rnormx $mean $stdev 1] 0]
        } elseif {$npoints < 0} {
            error "expected a non-negative integer, got \"$npoints\""
        } elseif {$npoints == 0} {
            set result {}
        } else {
            set result [hs::Rnormx $mean $stdev $npoints]
        }
        set result
    }
} else {
    proc ::hs::random {n} {
	if {![string is integer -strict $n]} {
	    error "Expected a positive integer, got \"$n\""
	} elseif {$n <= 0} {
	    error "Expected a positive integer, got \"$n\""
	}
	for {set i 0} {$i < $n} {incr i} {
	    lappend data [expr {rand()}]
	}
	set data
    }
    proc ::hs::random_init {n} {
	if {![string is integer -strict $n]} {
	    error "Expected an integer, got \"$n\""
	}
	expr {srand($n)}
	return
    }
}

############################################################################

proc ::hs::fit {datasets functions args} {
    # Available command options
    array set available_options {
        name        {}
        title       {}
        description {}
        method      "ls"
        warnings    on
        ignore      off
        errdef      1.0
        precision   0.0
        strategy    1
        verbose     0
	timeout     0
	minimizer   "migrad"
	minos       1
        mapping     "sequential"
        parameters  {}
    }
    # Possible future additions to the list of
    # available options: callbacks, gradient, status.

    # Parse the options
    if {[expr [llength $args] % 2] != 0} {
        error "wrong # of arguments"
    }
    foreach {option value} $args {
        regsub "^-" $option "" option
        if {![info exists available_options($option)]} {
            error "Invalid option \"$option\". Available options are\
                    -[join [lsort [array names available_options]] {, -}]."
        }
        switch -- $option {
            default {
                # Pass the option as is
                set available_options($option) $value
            }
        }
    }

    # Check that Minuit is not locked
    if {[fit::Fit_lock_minuit]} {
	error "Minuit is busy"
    }

    # Check if the name is OK
    set fitname [::fit::Fit_next_name]
    if {[string equal $available_options(name) ""]} {
        set procname ::$fitname
        while {[lsearch -exact [info commands $procname] $procname] >= 0} {
            set fitname [::fit::Fit_next_name]
            set procname ::$fitname
        }
    } else {
        if {[string equal -length 2 $available_options(name) "::"]} {
            set procname $available_options(name)
        } else {
            set procname ::$available_options(name)
        }
        if {[lsearch -exact [info commands $procname] $procname] >= 0} {
            error "command named \"$procname\" already exists"
        }
    }

    # Check that at least one dataset is specified
    if {[llength $datasets] == 0} {
        error "please specify at least one dataset"
    }

    # Is $datasets a single or multiple dataset specifier?
    # The first element of any dataset specifier must be an item id.
    set multiple 1
    foreach spec $datasets {
        foreach id $spec break
        if {![string is integer -strict $id]} {
            set multiple 0
            break
        }
    }
    if {!$multiple} {
        # Protect it from the "eval" command
        set datasets [list $datasets]
    }

    # Create the fit
    eval ::fit::Fit_create [list $fitname] $datasets \
            -title [list $available_options(title)] \
            -description [list $available_options(description)] \
            -method [list $available_options(method)] \
            -warnings [list $available_options(warnings)] \
            -ignore [list $available_options(ignore)] \
            -errdef [list $available_options(errdef)] \
            -precision [list $available_options(precision)] \
            -strategy [list $available_options(strategy)] \
            -verbose [list $available_options(verbose)] \
	    -timeout [list $available_options(timeout)] \
	    -minimizer [list $available_options(minimizer)] \
	    -minos [list $available_options(minos)]

    # Remember the old fit name and activate the new one
    set old_fit_name [::fit::Fit_get_active]
    ::fit::Fit_activate $fitname

    # Make sure that the fit is deleted and old fit is reactivated
    # in case we can't configure the functions and the parameters
    global ::errorInfo
    if {[catch {
        # Add the functions
        foreach function $functions {
            ::fit::Fit_function add $function -subsets [::fit::Fit_subset list]
        }
        # Add the parameters
        if {![string equal $available_options(parameters) "none"]} {
            if {[string equal $available_options(parameters) ""]} {
                # Figure out good parameter names (unique, only 
                # the first ten characters significant)
                set parameter_specs {}
                set fnum 0
                foreach function [::fit::Fit_function list] {
                    foreach funpar [::fit::Fit_function $function cget -parnames] {
                        lappend parameter_specs $funpar $function $fnum
                    }
                    incr fnum
                }
                fit::Fit_generate_valid_parnames $parameter_specs good_names
                # Define the parameter set
                variable ::hs::Hs_default_parameter_range
                foreach function [::fit::Fit_function list] {
                    # Try to see if we can find reasonable initial values
                    array unset initial_parameter_value
                    set signature [::hs::Function_signature $function]
                    if {[info exists ::hs::Hs_default_parameter_range($signature)]} {
                        foreach {parname min max init} \
                                $Hs_default_parameter_range($signature) {
                            set initial_parameter_value($parname) $init
                        }
                    }
                    # Define the parameters and their initial values
                    foreach funpar [::fit::Fit_function $function cget -parnames] {
                        if {[info exists initial_parameter_value($funpar)]} {
                            set initvalue $initial_parameter_value($funpar)
                            if {$initvalue == 0.0} {
                                set step 0.1
                            } else {
                                set step [expr {abs($initvalue/10.0)}]
                            }
                            ::fit::Fit_parameter add $good_names($funpar,$function) \
                                    -value $initvalue -step $step
                        } else {
                            ::fit::Fit_parameter add $good_names($funpar,$function)
                        }
                    }
                }
            } else {
                # Assume all parameters are specified
                foreach parameter $available_options(parameters) {
                    foreach {pname pvalue pstep pstate pbounds} $parameter {}
                    ::fit::Fit_parameter add $pname -value $pvalue \
                            -step $pstep -state $pstate -bounds $pbounds
                }
            }
            # Set up the parameter mapping
            if {![string equal $available_options(mapping) "none"]} {
                if {[string equal $available_options(mapping) "sequential"]} {
                    # Check that we have the correct number of parameters
                    set numpars 0
                    foreach function [::fit::Fit_function list] {
                        incr numpars [::fit::Fit_function $function cget -npars]
                    }
                    if {$numpars != [llength [::fit::Fit_parameter list]]} {
                        error "Can't set up sequential parameter mapping:\
                                the number of function parameters is different\
                                from the number of Minuit parameters."
                    }
                    foreach function [::fit::Fit_function list] {
                        ::fit::Fit_function $function configure -mapping sequential
                    }
                } else {
                    # This must be a non-trivial mapping
                    foreach parmap $available_options(mapping) {
                        foreach {funct code} $parmap {}
                        ::fit::Fit_function $funct configure -mapping $code
                    }
                }
            }
        }
    } errmess]} {
        set savedInfo $::errorInfo
        ::fit::Fit_rename $fitname {}
        ::fit::Fit_activate $old_fit_name
        error $errmess $savedInfo
    }

    # Create the handler proc. This should succeed because 
    # we have already checked that there is no such command.
    proc $procname {args} "eval ::fit::Fit_process [list $fitname] \$args"
    return $procname
}

if {![hs::have_cernlib]} {
    ::rename ::hs::fit {}
}

############################################################################

proc ::fit::Fit_generate_valid_parnames {parameter_specs name_array} {
    # Assign parameter names automatically. Use the function
    # parameter names if all names are unique; otherwise use
    # some heuristics to come up with good names. Only the first
    # ten characters of the parameter name are significant -- this
    # is because Minuit stores only the first ten. parameter_specs
    # is supposed to be a flat list which contains three elements
    # for each function parameter: parameter name, function name,
    # and function number. $name_array is the name of an array where
    # good names will be stored, indexed by $parameter_name,$function_name
    upvar $name_array parnames
    foreach format {
        {$pname}
        {${pname}_$fname}
        {${pname}_$fnum}
        {p${pnum}_$pname}
    } {
        set parlist {}
        set pnum 0
        foreach {pname fname fnum} $parameter_specs {
            eval set newname $format
            regsub -all {[^[:alnum:]]} $newname _ newname
            lappend parlist $newname
            set parnames($pname,$fname) $newname
            incr pnum
        }
        if {[llength $parlist] == [llength [lsort -unique \
                -command {string compare -length 10} $parlist]]} {
            # All names are unique -- good format found
            break
        }
    }
    return
}

############################################################################

proc ::fit::Fit_process {name args} {
    set cmdname [lindex [info level -1] 0]
    set arglen [llength $args]
    if {$arglen == 0} {
        # Special case -- no additional arguments.
        # Just return the command name.
        return $cmdname
    }

    # Requested action
    set action [lindex $args 0]
    if {[string equal $action "id"]} {
	if {$arglen != 1} {
	    error "wrong # of arguments"
	}
	return $name
    }

    # Activate the named fit. It also ensures
    # that the fit with the given name exists
    # and that Minuit is not currently busy.
    ::fit::Fit_activate $name

    # Perform the requested action
    set result {}
    switch -- $action {
        copy {
            if {$arglen > 2} {
                error "wrong # of arguments"
            }
            set fitname [::fit::Fit_next_name]
            if {$arglen == 1} {
                set procname ::$fitname
                while {[lsearch -exact [info commands $procname] $procname] >= 0} {
                    set fitname [::fit::Fit_next_name]
                    set procname ::$fitname
                }
            } else {
                set procname [lindex $args 1]
                if {![string equal -length 2 $procname "::"]} {
                    set procname "::$procname"
                }
                if {[lsearch -exact [info commands $procname] $procname] >= 0} {
                    error "command named \"$procname\" already exists"
                }
            }
            ::fit::Fit_copy $name $fitname
            proc $procname {args} "eval ::fit::Fit_process [list $fitname] \$args"
            set result $procname
        }
        del -
        delete {
            if {$arglen != 1} {
                error "wrong # of arguments"
            }
            ::fit::Fit_rename $name {}
            ::rename [lindex [info level -1] 0] {}
        }
        info {
            if {$arglen != 1} {
                error "wrong # of arguments"
            }
            set result [::fit::Fit_info]
        }
        subset {
            set result [eval ::fit::Fit_subset [lrange $args 1 end]]
        }
        param -
        parameter -
        parameters {
            set result [eval ::fit::Fit_parameter [lrange $args 1 end]]
        }
        function {
            set result [eval ::fit::Fit_function [lrange $args 1 end]]
        }
        config -
        configure {
            set result [eval ::fit::Fit_config [lrange $args 1 end]]
        }
        cget {
            set result [eval ::fit::Fit_cget [lrange $args 1 end]]
        }
	tcldata {
	    set result [eval ::fit::Fit_tcldata [lrange $args 1 end]]
	}
        plot -
        show {
            set result [eval ::fit::Fit_plot [lrange $args 1 end]]
        }
        contour -
        contours {
            set result [eval ::fit::Fit_error_contour [lrange $args 1 end]]
        }
	mini -
        fit {
            set result [eval ::fit::Fit_fit [lrange $args 1 end]]
        }
        tune {
            set result [eval ::fit::Fit_tuner [list $cmdname] [lrange $args 1 end]]
        }
        callback {
            set result [eval ::fit::Fit_callback [list $name] [lrange $args 1 end]]
        }
	report {
	    set result [eval ::fit::Fit_report [lrange $args 1 end]]
	}
	print {
            if {$arglen != 1} {
                error "wrong # of arguments"
            }
	    set result [::fit::Fit_report stdout text]
	}
	goodness -
        quality {
	    set result [eval ::fit::Fit_goodness [lrange $args 1 end]]
        }
        default {
            error "Invalid subcommand \"$action\".\
                    Valid subcommands are:\
                    [join [lsort {id copy delete info subset\
                    parameter function configure cget tcldata show report\
                    print contours fit tune callback goodness}] {, }]."
        }
    }
    return $result
}

############################################################################

proc ::fit::Fit_goodness {test_type {mcsamples {}} {show_progress 0}} {
    if {![fit::Fit_cget -complete]} {
        error "Fitting is not complete"
    }
    # Default number of pseudo experiments to run
    if {[string equal $mcsamples ""]} {
        set mcsamples 1000
    }
    switch -- $test_type {
	chisq {
	    foreach {chisq ndof} [fit::Fit_chisq] {}
	    if {$chisq >= 0.0} {
		set cl [::hs::function chisq_tail eval \
			[list x $chisq] [list n $ndof]]
	    } else {
		error "Chi-square goodness-of-fit test\
			can not be performed for this fit"
	    }
	}
	ks {
	    set cl_list {}
            set subset_list [fit::Fit_subset list]
	    foreach subset $subset_list {
		foreach {kslevel d npoints} [fit::Fit_subset $subset kstest] {}
		lappend cl_list $kslevel
	    }
            set nsets [llength $subset_list]
	    set clmin [lindex [lsort -real $cl_list] 0]
            # The following formula assumes that the p-values for
            # each subset are independent and uniformly distributed.
	    # For this particular test it is probably a bad assumption.
            set cl [expr {1.0 - pow(1.0-$clmin,$nsets)}]
	}
	ksmc {
            set distance_list {}
            set subset_list [fit::Fit_subset list]
            foreach subset $subset_list {
                foreach {kslevel d npoints} [fit::Fit_subset $subset kstest] {}
                lappend distance_list $d
            }
	    set ntuple_id [fit::Fit_ksmc $mcsamples $show_progress]
            set varnames [hs::ntuple_variable_list $ntuple_id]
	    set cl_list {}
            foreach v $varnames d $distance_list {
                set count [hs::Basic_ntuple_count $ntuple_id [list $v > $d]]
                if {$count < 0} {
                    error "Internal error. This is a bug. Please report."
                }
                lappend cl_list [expr {$count*1.0/$mcsamples}]
            }
            hs::delete $ntuple_id
            set nsets [llength $subset_list]
	    set clmin [lindex [lsort -real $cl_list] 0]
            # The following formula assumes that the p-values for
            # each subset are independent and uniformly distributed
            set cl [expr {1.0 - pow(1.0-$clmin,$nsets)}]
	}
        multires {
            foreach {cl worst_sigma} [fit::Fit_multires_test \
                    $mcsamples $show_progress] {}
        }
	default {
	    error "Invalid goodness-of-fit test type \"$test_type\".\
		    Valid test types are:\nchisq, ks, ksmc, and multires."
	}
    }
    return $cl
}

############################################################################

proc ::fit::Fit_ksmc {mcsamples show_progress} {
    # Generates an ntuple which contains Kolmogorov
    # distances for each dataset in the fit
    if {![fit::Fit_cget -complete]} {
        error "Fitting is not complete"
    }
    if {![string is integer -strict $mcsamples]} {
        error "Expected a positive integer, got \"$mcsamples\""
    }
    if {$mcsamples <= 0} {
        error "Expected a positive integer, got $mcsamples"
    }
    if {![string is boolean -strict $show_progress]} {
        error "Expected a boolean, got \"$show_progress\""
    }

    # Extract the KS statistics. Here, they are only
    # used to figure out the number of points to generate.
    set subset_list [fit::Fit_subset list]
    foreach subset $subset_list {
	set ksdata($subset) [fit::Fit_subset $subset kstest]
    }

    # Copy the fit and create new data items
    set thisfit [fit::Fit_get_active]
    set copyfit [fit::Fit_next_name]
    set procname ::$copyfit
    while {[lsearch -exact [info commands $procname] $procname] >= 0} {
	set copyfit [fit::Fit_next_name]
	set procname ::$copyfit
    }
    fit::Fit_copy $thisfit $copyfit
    set idlist [fit::Fit_copy_data $copyfit "Temporary MC fit category"]
    if {[llength $subset_list] != [llength $idlist]} {
        error "Failed to copy fit data items"
    }
    foreach id $idlist {
        fit::Fit_callback $copyfit add destruct "hs::delete $id ;#"
    }
    proc $procname {args} "eval fit::Fit_process [list $copyfit] \$args"
    fit::Fit_activate $copyfit
    fit::Fit_config -verbose -1 -minos off

    # Create the ntuple in which we will remember Kolmogorov distances
    # for each dataset. Each pseudo experiment will take one ntuple row.
    set varlist {}
    foreach subset $subset_list {
        lappend varlist "D_$subset"
    }
    set ntuple_id [hs::create_ntuple [hs::Temp_uid] \
        "Ntuple of Kolmogorov distances for $thisfit" \
        [hs::Temp_category] $varlist]

    # Carry out the pseudo experiments
    global ::errorInfo
    set status [catch {
        for {set i 0} {$i < $mcsamples} {incr i} {
            fit::Fit_activate $thisfit
            foreach subset $subset_list id $idlist {
                foreach {level d npoints} $ksdata($subset) {}
                hs::reset $id
                fit::Fit_subset $subset random $npoints $id
            }
            fit::Fit_activate $copyfit
            fit::Fit_fit
            set distances {}
            foreach subset [fit::Fit_subset list] {
                foreach {level d npoints} [fit::Fit_subset $subset kstest] {}
                lappend distances $d
            }
            hs::fill_ntuple $ntuple_id $distances
            if {$show_progress} {
                puts -nonewline " $i"
                flush stdout
            }
        }
        if {$show_progress} {
            puts ""
            flush stdout
        }
    } ermess]
    set savedInfo $::errorInfo
    catch {
	$procname delete
	fit::Fit_activate $thisfit
    }
    if {$status} {
        hs::delete $ntuple_id
	error $ermess $savedInfo
    }
    return $ntuple_id
}

############################################################################

proc ::fit::Fit_chisq {} {
    # Retuns the list {chisq n_dof} for the current fit.
    # chisq will be negative in case chi-squared test
    # is impossible/was not performed on the active fit.
    if {![fit::Fit_cget -complete]} {
        error "Fitting is not complete"
    }
    set total_chisq 0.0
    set total_points 0
    foreach subset [fit::Fit_subset list] {
	set chisq_ok 0
	if {[fit::Fit_subset $subset cget -binned]} {
	    if {![string equal [fit::Fit_subset $subset cget -method] "L2"]} {
		set npoints [fit::Fit_subset $subset cget -ndof]
		if {$npoints > 0} {
		    set chisq_ok 1
		    incr total_points $npoints
		    set chisq [fit::Fit_subset $subset cget -chisq]
		    set total_chisq [expr {$total_chisq + $chisq}]
		}
	    }
	}
	if {!$chisq_ok} {
	    return {-1.0 0}
	}
    }
    # Subtract the number of variable parameters
    # from the number of degrees of freedom
    foreach {fmin fedm errdef npari nparx istat} \
	    [fit::Fit_cget -ministat] {}
    incr total_points -$npari
    list $total_chisq $total_points
}

############################################################################

proc ::fit::Fit_report {chan format} {
    # Check that the format is a supported one
    set supported_formats {text}
    if {[lsearch -exact $supported_formats $format] < 0} {
	error "Format \"$format\" is not supported"
    }
    # Check that there is an active fit
    set name [fit::Fit_get_active]
    if {[string equal $name ""]} {
        error "No active fit"
    }
    # Check that the fit is complete, and the status is ok
    if {![fit::Fit_cget -complete]} {
	error "Fitting is not complete"
    }
    set fit_status [fit::Fit_cget -status]
    if {![string equal $fit_status "ok"]} {
	error "Fit status is \"$fit_status\""
    }

    # Print a generic header
    fit::Fit_report_header $chan $format

    # Describe the data sets and filters
    foreach subset [fit::Fit_subset list] {
	fit::Fit_report_subset $subset $chan $format
    }

    # Describe the fitting functions and mappings
    foreach fun [fit::Fit_function list] {
	fit::Fit_report_function $fun $chan $format
    }
    
    # Describe parameters
    fit::Fit_report_parameters $chan $format
    return
}

############################################################################

proc ::fit::Fit_report_parameters {chan format} {
    # This command should be called only from fit::Fit_report
    set parameter_names [fit::Fit_parameter list]
    foreach parname $parameter_names {
	set parameter_data(value,_,$parname) [fit::Fit_parameter $parname cget -value]
	set parameter_data(state,_,$parname) [fit::Fit_parameter $parname cget -state]
	if {![string equal $parameter_data(state,_,$parname) "fixed"]} {
	    foreach {eparab eneg epos globcc} [fit::Fit_parameter $parname cget -errinfo] {}
	    foreach el {eparab eneg epos globcc} {
		set parameter_data($el,_,$parname) [set $el]
	    }
	}
    }
    set have_minos [fit::Fit_cget -minos]
    switch -- $format {
	text {
	    # Will try to pretty-print the parameter table.
            # The table will have the following columns: parameter
	    # name, fitted value, parabolic error, minos negative error,
	    # minos positive error, and global correlation coefficient.
	    # The minos columns may be skipped in case MINOS was not run.
	    puts $chan ""
	    puts $chan "@@@@ Fit parameters @@@@"
	    if {$have_minos} {
		puts $chan "   Parameter        Fitted    Parabolic           Minos errors         Global  "
		puts $chan "        name         value        error      negative     positive  correlation"
		puts $chan "------------  ------------  -----------  ------------  -----------  -----------"
	    } else {
		puts $chan "   Parameter        Fitted    Parabolic     Global  "
		puts $chan "        name         value        error  correlation"
		puts $chan "------------  ------------  -----------  -----------"
	    }
	    foreach parname $parameter_names {
		puts -nonewline $chan "[format {%12s} [string range $parname 0 11]] \
			[format {%12.6g} $parameter_data(value,_,$parname)]  "
		if {[string equal $parameter_data(state,_,$parname) "fixed"]} {
		    puts $chan "    fixed"
		} else {
		    # CHANGE THIS!!! Need to process the special 
		    # case when a parameter is at the limit
		    puts -nonewline $chan "[format {%11.6g} $parameter_data(eparab,_,$parname)]  "
		    if {$have_minos} {
			puts -nonewline $chan "[format {%12.6g} $parameter_data(eneg,_,$parname)] \
				[format {%11.6g} $parameter_data(epos,_,$parname)]  "
		    }
		    puts $chan "[format {%11.6g} $parameter_data(globcc,_,$parname)]"
		}
	    }
	}
	default {
	    error "Incomplete report format switch statement in\
		    [lindex [info level 0] 0]. This is a bug. Please report."
	}
    }
    return
}

############################################################################

proc ::fit::Fit_report_function {tag chan format} {
    # This command should be called only from fit::Fit_report
    set fun_param_values [fit::Fit_function $tag cget -params]
    switch -- $format {
	text {
	    puts $chan ""
	    puts $chan "#### Fit function $tag ####"
	    puts $chan "Function name: [hs::function $tag cget -name]"
	    puts $chan "Used in the following datasets:\
		    [join [fit::Fit_function $tag cget -subsets] {, }]"
	    puts $chan "Parameter mapping: [string trimright \
		    [fit::Fit_function $tag cget -mapping]]"
	    set line "Parameter values:"
	    set len [string length $line]
	    set maxlen 79
	    fit::Eval_at_precision 10 {
		foreach pair $fun_param_values {
		    set pairlen [string length $pair]
		    incr pairlen 3
		    set newlen [expr {$len + $pairlen}]
		    if {$newlen > $maxlen} {
			append line "\n"
			set len $pairlen
		    } else {
			set len $newlen
		    }
		    append line " \{$pair\}"
		}
	    }
	    puts $chan $line
	}
	default {
	    error "Incomplete report format switch statement in\
		    [lindex [info level 0] 0]. This is a bug. Please report."
	}
    }
    return
}

############################################################################

proc ::fit::Fit_report_subset {subset chan format} {
    # This command should be called only from fit::Fit_report
    foreach {id ndim binned columns filter_string functions weight\
	    method points normregion} [fit::Fit_subset $subset info] break
    foreach {uid category title itemtype} [::hs::item_properties $id] {}
    switch $itemtype {
	HS_1D_HISTOGRAM {
	    set typename "1d histogram"
	}
	HS_2D_HISTOGRAM {
	    set typename "2d histogram"
	}
	HS_NTUPLE {
	    set typename "ntuple"
	}
	HS_NONE {
	    error "Dataset ${subset}: Histo-Scope item with id $id no longer exists"
	}
	default {
	    error "Don't know how to deal with item type \"$itemtype\" in\
		    [lindex [info level 0] 0]. This is a bug. Please report."
	}
    }
    if {$binned} {
	set ndof [fit::Fit_subset $subset cget -ndof]
	set chisq [fit::Fit_subset $subset cget -chisq]
	if {$ndof > 0} {
	    set cl [::hs::function chisq_tail eval [list x $chisq] [list n $ndof]]
	    set cl [expr {$cl * 100.0}]
	}
    }
    # Get the fit statistics
    foreach {data_npoints data_is_pdf data_sum} [fit::Fit_subset \
	    $subset stats data {npoints is_pdf sum}] {}
    foreach {fit_npoints fit_is_pdf fit_sum} [fit::Fit_subset \
	    $subset stats fit {npoints is_pdf sum}] {}
    if {$data_npoints > 0 && $data_is_pdf && $fit_npoints > 0 && $fit_is_pdf} {
	foreach {data_mean_x data_mean_y data_mean_z data_s_x data_s_y data_s_z \
		data_rho_xy data_rho_xz data_rho_yz} [fit::Fit_subset $subset \
		stats data {mean_x mean_y mean_z s_x s_y s_z rho_xy rho_xz rho_yz}] {}
	foreach {fit_mean_x fit_mean_y fit_mean_z fit_s_x fit_s_y fit_s_z \
		fit_rho_xy fit_rho_xz fit_rho_yz} [fit::Fit_subset $subset \
		stats fit {mean_x mean_y mean_z s_x s_y s_z rho_xy rho_xz rho_yz}] {}
    }
    # Write the subset info out
    switch -- $format {
	text {
	    puts $chan ""
	    puts $chan "**** Dataset $subset ****"
	    puts $chan "Histo-Scope item id: $id"
	    if {[string equal $itemtype "HS_NTUPLE"]} {
		puts -nonewline $chan "Item type: $typename, ${ndim}d "
		if {$binned} {
		    puts $chan "binned fit"
		} else {
		    puts $chan "unbinned fit"
		}
	    } else {
		puts $chan "Item type: $typename"
	    }
	    puts $chan "User id: $uid"
	    puts $chan "Category: $category"
	    puts $chan "Title: $title"
	    if {[string equal $itemtype "HS_NTUPLE"]} {
		# Print out the column mapping
		foreach {nx ny nz ndata nerrors} $columns break
		set columninfo {}
		foreach type {x y z data errors} {
		    set column [set n$type]
		    if {$column >= 0} {
			lappend columninfo "[::hs::variable_name $id $column] is $type"
		    }
		}
		puts $chan "Column mapping: [join $columninfo {, }]"
	    }
	    puts -nonewline $chan "Fit function"
	    if {[llength $functions] > 1} {
		puts -nonewline $chan "s"
	    }
	    puts $chan ": [join $functions {, }]"
	    puts $chan "Fitting method: $method"
	    if {[string equal $method "eml"]} {
		puts $chan "Normalization region: $normregion"
	    }
	    puts $chan "Dataset weight: $weight"
	    puts -nonewline $chan "Data filter: "
	    if {[string is space $filter_string]} {
		puts $chan "none"
	    } else {
		puts $chan $filter_string
	    }
	    puts $chan "Data points used in the fit: $points"
	    puts -nonewline $chan "Chi-square: "
	    set have_chisq 0
	    if {$binned} {
		if {![string equal $method "L2"]} {
		    set have_chisq 1
		    puts -nonewline $chan "[format {%.5g} $chisq] / $ndof points"
		    if {$ndof > 0} {
			puts -nonewline $chan ", CL = [format {%.5g} $cl]%"
		    }
		    puts $chan ""
		}
	    }
	    if {!$have_chisq} {
		puts $chan "undefined"
	    }
	    # Print the data/fit statistics
	    set stats_format "%.6g"
	    if {$fit_npoints > 0} {
		puts $chan "Events data / fit: [format $stats_format\
			$data_sum] / [format $stats_format $fit_sum]"
		if {$data_npoints > 0 && $data_is_pdf && $fit_is_pdf} {
		    foreach {title q} {
			"X mean" mean_x
			"X standard deviation" s_x
		    } {
			puts $chan "$title data / fit: [format $stats_format\
			    [set data_$q]] / [format $stats_format [set fit_$q]]"
		    }
		    if {$ndim > 1} {
			foreach {title q} {
			    "Y mean" mean_y
			    "Y standard deviation" s_y
			    "XY correlation" rho_xy
			} {
			    puts $chan "$title data / fit: [format $stats_format\
				    [set data_$q]] / [format $stats_format [set fit_$q]]"
			}
		    }
		    if {$ndim > 2} {
			foreach {title q} {
			    "Z mean" mean_z
			    "Z standard deviation" s_z
			    "XZ correlation" rho_xz
			    "YZ correlation" rho_yz
			} {
			    puts $chan "$title data / fit: [format $stats_format\
				    [set data_$q]] / [format $stats_format [set fit_$q]]"
			}
		    }
		}
	    }
	    # It would be nice to print a p-value for some distribution
	    # test statistic at this point, for example, Kolmogorov-Smirnov.
	    # However, this may lead some users to believe that these
	    # statistics are, indeed, applicable to the fitting problem
	    # without a detailed Monte-Carlo study...
	}
	default {
	    error "Incomplete report format switch statement in\
		    [lindex [info level 0] 0]. This is a bug. Please report."
	}
    }
    return
}

############################################################################

proc ::fit::Fit_error_matrix_status {status_code} {
    switch $status_code {
	0 {set description "not calculated"}
	1 {set description "not accurate, diagonal approximation only"}
	2 {set description "forced positive-definite"}
	3 {set description "accurate"}
	default {
	    error "Invalid error matrix status code \"$status_code\""
	}
    }
    return $description
}

############################################################################

proc ::fit::Fit_report_header {chan format} {
    # This command should be called only from fit::Fit_report
    global ::env
    set name [fit::Fit_get_active]
    set datestring [clock format [clock seconds] -format "%D %T"]
    foreach {chisq ndof} [fit::Fit_chisq] {}
    if {$ndof > 0} {
	set cl [::hs::function chisq_tail eval [list x $chisq] [list n $ndof]]
	set cl [expr {$cl * 100.0}]
    }
    foreach {fmin fedm errdef npari nparx istat} [fit::Fit_cget -ministat] {}
    set errmat_status [fit::Fit_error_matrix_status $istat]
    switch -- $format {
	text {
	    puts $chan "Fit $name report"
	    puts $chan "Generated $datestring by $::env(USER)@[info hostname]"
	    puts $chan "Hs version: [::hs::tcl_api_version]"
	    puts $chan "Fit title: [fit::Fit_cget -title]"
	    puts $chan "Fit description: [fit::Fit_cget -description]"
	    puts $chan "Datasets fitted: [llength [fit::Fit_subset list]]"
	    puts -nonewline $chan "Chi-square: "
	    if {$chisq >= 0.0} {
		puts -nonewline $chan "[format {%.5g} $chisq] / $ndof d.o.f."
		if {$ndof > 0} {
		    puts -nonewline $chan ", CL = [format {%.5g} $cl]%"
		}
		puts $chan ""
	    } else {
		puts $chan "undefined"
	    }
	    puts $chan "Minuit parameter error matrix $errmat_status"
	}
	default {
	    error "Incomplete report format switch statement in\
		    [lindex [info level 0] 0]. This is a bug. Please report."
	}
    }
    return
}

############################################################################

proc ::fit::Fit_fit {{maxcalls {}} {tolerance {}}} {

    # Check the input arguments
    set use_maxcalls 2147483647
    if {[string is integer -strict $maxcalls]} {
        if {$maxcalls > 0} {
            set use_maxcalls $maxcalls
        } else {
            error "expected a positive integer, got \"$maxcalls\""
        }
    } else {
        if {![string equal $maxcalls ""]} {
            error "expected a positive integer, got \"$maxcalls\""
        }
    }

    set use_tolerance ""
    if {[string is double -strict $tolerance]} {
        if {$tolerance > 0.0} {
            set use_tolerance $tolerance
        } else {
            error "expected a positive real number, got \"$tolerance\""
        }
    } else {
        if {![string equal $tolerance ""]} {
            error "expected a positive real number, got \"$tolerance\""
        }
    }

    # Check that it makes sense to run the minimization
    if {[fit::Fit_has_tcl_fcn]} {
        fit::Fit_fcn_set "user"
    } else {
        if {[llength [fit::Fit_subset list]] == 0} {
            error "No datasets defined"
        }
        if {[llength [fit::Fit_function list]] == 0} {
            error "No fitting functions defined"
        }
        fit::Fit_fcn_set "generic"
        fit::Fit_compile
    }

    # Make sure the configuration is OK
    fit::Fit_reset
    fit::Fit_config apply
    fit::Fit_parameter apply

    # Find out if L2 is the only minimization method used.
    # In this case we will have to figure out reasonable errors.
    if {[fit::Fit_has_tcl_fcn]} {
        set all_l2 0
    } else {
        set all_l2 1
        foreach subset [fit::Fit_subset list] {
            if {![string equal [fit::Fit_subset $subset cget -method] "L2"]} {
                set all_l2 0
                break
            }
        }
    }

    # Figure out Minuit minimizer to use and the error definition
    set minimizer [fit::Fit_cget -minimizer]
    if {[string equal use_tolerance ""]} {
	if {[string equal $minimizer "simplex"]} {
	    # The minimization will stop when the estimated distance
	    # to the minimum (EDM) is less than $use_tolerance.
	    # Minuit default tolerance is 0.1*$UP.
	    set UP [fit::Fit_cget -errdef]
	    set use_tolerance [expr {0.01 * $UP}]
	} elseif {[string equal $minimizer "migrad"]} {
	    # The minimization will stop when the estimated distance
	    # to the minimum is less than 0.001*$use_tolerance*$UP.
	    # Minuit default tolerance is 0.1.
	    set use_tolerance 0.001
	} else {
	    # Will use Minuit defaults...
	}
    }

    # Run Minuit. Make sure to unlock it at the end.
    global ::errorInfo
    fit::Fit_lock_minuit 1
    set status [catch {
	set total_fcn_calls 0
	::mn::comd call 1
	eval ::mn::excm $minimizer $use_maxcalls $use_tolerance
	incr total_fcn_calls [mn::nfcn]
	if {$all_l2} {
	    # Run couple more iterations to get some idea 
	    # about errors. This is going to be meaningful
	    # only in case all datasets have the same vertical
	    # scale. Note that at this point the minimization
	    # status is available only directly from Minuit,
	    # it has not been copied into the fit configuration
	    # structure yet.
	    foreach {fmin fedm errdef npari nparx istat} [::mn::stat] {}
	    set datapoints [fit::Fit_cget -wpoints]
	    if {$datapoints > $npari} {
		# Note the usage of ::mn::excm rather than ::mn::comd below.
		# This is necessary because Minuit does not parse doubles
		# with many digits correctly.
		::mn::excm "set errdef" [expr {$fmin/($datapoints-$npari)}]
		eval ::mn::excm $minimizer $use_maxcalls $use_tolerance
		incr total_fcn_calls [mn::nfcn]
		# Skip the last iteration in case we have already called
		# the function too many times
		if {$total_fcn_calls < $use_maxcalls} {
		    foreach {fmin fedm errdef npari nparx istat} [::mn::stat] {}
		    set datapoints [fit::Fit_cget -wpoints]
		    if {$datapoints > $npari} {
			::mn::excm "set errdef" [expr {$fmin/($datapoints-$npari)}]
			eval ::mn::excm $minimizer $use_maxcalls $use_tolerance
			incr total_fcn_calls [mn::nfcn]
		    }
		}
	    }
	}
	# Calculate the covariance matrix
	::mn::comd hesse $use_maxcalls
	# Run MINOS (if requested)
	if {[fit::Fit_cget -minos]} {
	    ::mn::comd minos $use_maxcalls
	}
	# Remember the results
	::mn::comd call 3
    } errMess]
    set savedInfo $::errorInfo
    catch {
	fit::Fit_lock_minuit 0
	# Restore the "UP" value which may have been changed
	::mn::comd set pri -1
	::mn::excm "set errdef" [fit::Fit_cget -errdef]
	::mn::comd set pri [fit::Fit_cget -verbose]
    }
    if {$status} {
	error $errMess $savedInfo
    }
    return
}

############################################################################

proc ::fit::Fit_names {} {
    lsort -dictionary [fit::Fit_list]
}

############################################################################

proc ::fit::Fit_compile {} {
    if {[string equal [fit::Fit_get_active] ""]} {
        error "No active fit"
    }
    # Check if we actually need to compile anything
    if {[fit::Fit_cget compiled]} return

    # Check that Minuit is not locked
    if {[fit::Fit_lock_minuit]} {
	error "Minuit is busy"
    }

    # Go over the mappings
    set have_nontrivial_mapping 0
    foreach fitter [fit::Fit_function list] {
        if {![fit::Fit_function $fitter compiledmap]} {
            set mapping [fit::Fit_function $fitter cget -mapping]
            if {[string is space $mapping]} {
                # Mapping is a blank string. Use some reasonable default.
                if {[fit::Fit_function $fitter cget -npars] > 0} {
                    fit::Fit_function $fitter compiledmap -1 "sequential"
                } else {
                    fit::Fit_function $fitter compiledmap -1 "null"
                }
            } else {
                if {[string equal $mapping "sequential"] || \
                        [string equal $mapping "identical"] || \
                        [string equal $mapping "null"]} {
                    fit::Fit_function $fitter compiledmap -1 $mapping
                } else {
                    set have_nontrivial_mapping 1
                    set nontrivial_mapping($fitter) $mapping
                }
            }
        }
    }

    # Go over dataset filters
    set have_exclusion_regions 0
    foreach i [fit::Fit_subset list] {
        if {[fit::Fit_subset $i compiledfilter]} continue
        set filter_string [fit::Fit_subset $i cget -filter]
        if {[string is space $filter_string]} {
            fit::Fit_subset $i compiledfilter -1 {}
        } else {
            set have_exclusion_regions 1
            set filter_functions($i) $filter_string
        }
    }

    if {!$have_nontrivial_mapping && !$have_exclusion_regions} {
        if {![fit::Fit_cget -compiled]} {
            error "Failed to set up parameter mappings and/or\
                    dataset filters"
        }
        return
    }

    # Open file for the C code
    set tempdir [::hs::tempdir]
    foreach {filename chan} [::hs::tempfile \
	    [file join $tempdir hs_cfile_] ".c"] {}

    # Make sure the channel is closed properly
    global ::errorInfo
    set status [catch {
        # Include the standard C headers
        hs::Write_standard_c_headers $chan
        puts $chan "#include \"$::hs::Histoscope_header\""

        # Write mapping functions
        variable Temporary_function_counter
        foreach {fitter mapping} [array get nontrivial_mapping] {
            foreach {init_name fitter_cname} \
                    [hs::function $fitter cget -functions] break
            set minuit_pars [fit::Fit_parameter list]
            set fitter_pars [hs::function $fitter cget -parameters]
            set fitter_maxpars [llength $fitter_pars]
            set mapname($fitter) "hs_parameter_map_[incr\
                    Temporary_function_counter]_for_$fitter_cname"
            set arrname "minuit_pars_JK75vKDghm6df"
            set toname "local_pars_BChgfwh_5ymSqj"
            set offset "offset_nUyTE_jlyfHTk"
            set mapping [fit::Parse_minuit_pars_in_a_map $mapping $minuit_pars $arrname]
            set mapping [fit::Parse_fitter_pars_in_a_map $mapping $fitter_pars $toname]
            puts $chan ""
            puts $chan "const double * $mapname($fitter)(const double *$arrname, int $offset)"
            puts $chan "\{"
            puts $chan "  static double $toname\[$fitter_maxpars\] = \{0.0\};"
            puts $chan ""
            puts $chan $mapping
            puts $chan ""
            puts $chan "  return $toname;"
            puts $chan "\}"
        }
        
        # Write dataset filters
        foreach {subset filter_expr} [array get filter_functions] {
            set filter_name($subset) "hs_dataset_filterfun_[incr\
                    Temporary_function_counter]_$subset"
            puts $chan ""
            puts $chan "int $filter_name($subset)(double x, double y, double z)"
            puts $chan "\{"
            puts $chan "  return ($filter_expr);"
            puts $chan "\}"
        }

        # File name for the shared library
	foreach {sharedlib chan1} [::hs::tempfile \
		[file join $tempdir hs_sofile_] $::hs::Sharedlib_suffix] {}
	close $chan1
    } errstat]
    set savedInfo $::errorInfo
    close $chan
    if {$status} {
        file delete $filename
        error $errstat $savedInfo
    }

    # Compile the code
    if {[catch {hs::sharedlib_compile $filename $sharedlib} errstat]} {
        file delete $sharedlib
        error "Failed to compile parameter mappings and/or dataset\
                filters.\nCompiler diagnostics:\n$errstat"
    }
    file delete $filename

    # Load the shared library and assign the functions
    set dll_id [hs::sharedlib open $sharedlib]
    foreach {fitter mapfun} [array get mapname] {
        fit::Fit_function $fitter compiledmap $dll_id $mapfun
    }
    foreach {subset filter_fun} [array get filter_name] {
        fit::Fit_subset $subset compiledfilter $dll_id $filter_fun
    }
    file delete $sharedlib

    # Make sure the mapping functions are unloaded
    # when the fit is deleted
    fit::Fit_append_dll $dll_id

    if {![fit::Fit_cget -compiled]} {
        error "Failed to compile parameter mappings and/or\
                dataset filters"
    }
    return
}

############################################################################

proc ::fit::Fit_error_contour {pname1 pname2 args} {
    # Default option values
    set plotpoints 200
    set levels {1 2 3}
    set show 1
    set window {}
    set errdef {}
    set font ""

    # Check that the fit is complete
    if {![fit::Fit_cget -complete]} {
        set message "Fitting is not complete"
        if {![fit::Fit_cget -psync]} {
            append message " -- parameters have changed"
        } elseif {![fit::Fit_cget -osync]} {
            append message " -- fit options have changed"
        }
        error $message
    }

    # Go over the list of options
    set available_options {-font -plotpoints -levels -show -window -errdef}
    if {[expr [llength $args] % 2] != 0} {
        error "wrong # of arguments"
    }
    foreach {option value} $args {
        if {[lsearch -exact $available_options $option] < 0} {
            error "Invalid option \"$option\". Available options\
                    are [join [lsort $available_options] {, }]."
        }
        regsub "^-" $option "" option
        switch -- $option {
            show {
                # Should be a boolean value
                if {[catch {
                    if {$value} {
                        set value 1
                    } else {
                        set value 0
                    }
                }]} {
                    error "expected a boolean value, got \"$value\""
                }
            }
            plotpoints {
                # Must be a positive integer > 4
                if {![string is integer $value]} {
                    error "expected an integer larger than 4, got \"$value\""
                } elseif {$value <= 4} {
                    error "expected an integer larger than 4, got $value"
                }
            }
            errdef {
                # Must be a positive double
                if {![string is double $value]} {
                    error "expected a positive real number, got \"$value\""
                }
                if {![string equal "" $value]} {
                    if {$value <= 0.0} {
                        error "expected a positive real number, got \"$value\""
                    }
                }
            }
            levels {
                # Must be a list of positive real numbers
                if {[llength $value] == 0} {
                    error "expected a list of positive real numbers, got \"$value\""
                }
                foreach error_level $value {
                    if {![string is double -strict $error_level]} {
                        error "expected a positive real number, got \"$error_level\""
                    } elseif {$error_level <= 0.0} {
                        error "expected a positive real number, got $error_level"
                    }
                }
            }
            window {
		set value [string trimleft $value]
		hs::Check_window_name $value
	    }
	    font {
		if {![string equal $value ""]} {
		    hs::Generate_xlfd $value
		}
	    }
            default {
                error "Incomplete switch statement in [lindex [info level 0] 0].\
                        This is a bug. Please report."
            }
        }
        set $option $value
    }

    # Check that Minuit is available
    if {[fit::Fit_lock_minuit]} {
	error "Minuit is busy"
    }    

    # Draw the plot
    set overlay [fit::Draw_error_contours $pname1 \
            $pname2 $plotpoints $errdef $levels]
    if {[string compare $font ""]} {
	hs::overlay $overlay -font $font
    }
    if {$show} {
        if {[string equal $window ""]} {
	    variable Fit_plot_window_counter
	    set window "Hs_Fit_Plot_Window_[incr Fit_plot_window_counter]"
        }
	hs::overlay $overlay show -window $window
	fit::Fit_tcldata set wincont $window
    }
    return $overlay
}

############################################################################

proc ::fit::Default_title {} {
    set title [fit::Fit_cget -title]
    if {[string equal $title ""]} {
        set title "Fit [fit::Fit_get_active]"
    }
    return $title
}

############################################################################

proc ::fit::Draw_error_contours {pname1 pname2 plotpoints errdef error_levels} {
    # Check that the parameter names are valid and that
    # the parameters are not fixed
    set namelist [fit::Fit_parameter list]

    set pn1 [lsearch -exact $namelist $pname1]
    if {$pn1 < 0} {
        error "\"$pname1\" is not a valid parameter name"
    } elseif {[string equal [fit::Fit_parameter $pname1 cget -state] "fixed"]} {
        error "parameter \"$pname1\" is fixed"
    }
    incr pn1

    set pn2 [lsearch -exact $namelist $pname2]
    if {$pn2 < 0} {
        error "\"$pname2\" is not a valid parameter name"
    } elseif {[string equal [fit::Fit_parameter $pname2 cget -state] "fixed"]} {
        error "parameter \"$pname2\" is fixed"
    }
    incr pn2

    # Check that parameter names are distinct
    if {$pn1 == $pn2} {
        error "parameter names must be distinct"
    }

    # Get the parameter values
    set value1 [fit::Fit_parameter $pname1 cget -value]
    set value2 [fit::Fit_parameter $pname2 cget -value]

    # Default window title
    if {[llength $error_levels] > 1} {
        set S "s"
    } else {
        set S ""
    }
    set methlist {}
    foreach subset [fit::Fit_subset list] {
        lappend methlist [fit::Fit_subset $subset cget -method]
    }
    if {[llength $methlist] > 1} {
        set MS "s"
    } else {
        set MS ""
    }
    set title "Error contour$S $pname2 vs. $pname1 for\
            \"[fit::Default_title]\", method$MS [join $methlist {, }]"

    # Ntuples to show
    set varnames [list X "Fit result"]
    set id_center [hs::create_ntuple [hs::Temp_uid] "Fit result" \
            [hs::Temp_category] $varnames]
    hs::fill_ntuple $id_center [list $value1 $value2]

    set overlay [hs::Unused_plot_name]
    hs::overlay $overlay -xlabel $pname1 -ylabel $pname2 \
	    -style errorcontour -title $title \
            add $id_center xy -x X -y "Fit result" -line 0 -owner 1
    foreach {fmin fedm old_errdef} [::mn::stat] break
    if {[string equal $errdef ""]} {
        set errdef [fit::Fit_cget -errdef]
    }
    ::mn::comd set pri -1
    global ::errorInfo
    fit::Fit_lock_minuit 1
    set plotstatus [catch {
        foreach level $error_levels {
	    ::mn::excm "set errdef" [expr {$level * $level * $errdef}]
            set varnames [list X "$level sigma"]
            set id_ntuple [hs::create_ntuple [hs::Temp_uid] \
                    "Error contour at $level sigma" [hs::Temp_category] $varnames]
            if {$id_ntuple <= 0} {
                error "Failed to create a temporary ntuple"
            }
            ::mn::cont $pn1 $pn2 $plotpoints $id_ntuple
            hs::overlay $overlay add $id_ntuple xy -x X -y "$level sigma" -owner 1
        }
    } errmess]
    set savedInfo $::errorInfo
    fit::Fit_lock_minuit 0
    if {[info exists old_errdef]} {
	::mn::excm "set errdef" $old_errdef
    } else {
	::mn::excm "set errdef" [fit::Fit_cget -errdef]
    }
    ::mn::comd set pri [fit::Fit_cget -verbose]
    if {$plotstatus} {
        hs::overlay $overlay clear
        error $errmess $savedInfo
    }
    return $overlay
}

############################################################################

proc ::fit::Fit_plot {args} {
    set plotpoints 1000
    set fitcolor red
    set geometry_multi 550x750
    set geometry_single_1d 550x412
    set geometry_single_2d 800x300
    set draw_axis_labels 1
    set show_info 0

    # Parse the options
    set winlist_name {}
    set script_name {}
    set wintitle ""
    set plotfont ""
    set haveerrors on
    if {[expr [llength $args] % 2] != 0} {
        error "wrong # of arguments"
    }
    foreach {option value} $args {
        regsub "^-" $option "" optname
        switch -- $optname {
            info {
                if {[string is boolean -strict $value]} {
                    set show_info $value
                } else {
                    error "expected a boolean value for the\
                            \"-info\" option, got \"$value\""
                }
            }
            winlist {
                set winlist_name $value
            }
            script {
                set script_name $value
            }
            errors {
                set haveerrors $value
            }
	    title {
		set wintitle $value
	    }
	    font {
		set plotfont $value
		# Check that the font name is correct
		if {![string equal $plotfont ""]} {
		    hs::Generate_xlfd $plotfont
		}
	    }
            default {
                error "Invalid option \"$option\". Available options are\
                        -font, -info, -script, -title, and -winlist."
            }
        }
    }

    # Check that there is an active fit
    set fitname [fit::Fit_get_active]
    if {[string equal $fitname ""]} {
        error "No active fit"
    }

    # Check that the fit is compiled
    if {![fit::Fit_cget -compiled]} {
        fit::Fit_compile
    }

    # Check that we can show fit info if requested
    if {![fit::Fit_cget -complete] && $show_info} {
        error "Can't show the info: the fit is not complete"
    }

    # Basic check that we can make the plots
    set valid_sets [fit::Fit_subset list]
    if {[llength $valid_sets] == 0} {
        error "No datasets defined"
    }
    set short_subset_specs {}
    foreach i $valid_sets {
        foreach {id ndim binned column_map} [fit::Fit_subset $i info] break
        if {$ndim > 2} {
            puts "Sorry, can't plot n-dimentional fits with n > 2 yet."
            puts "Fit display will not be generated for dataset $i."
        } elseif {[string equal [hs::type $id] "HS_NONE"]} {
            error "Histo-Scope item with id $id has been deleted"
        } else {
            lappend short_subset_specs $i $id $ndim $binned $column_map
        }
    }

    # Map fit parameters into function parameters
    fit::Fit_apply_mapping

    # Initialize the updating script
    set update_script {}
    lappend update_script "fit::Fit_eval $fitname \{"
    lappend update_script {
        if {![fit::Fit_cget -compiled]} {
            error "The fit is not compiled"
        }
        fit::Fit_config -status ok
        fit::Fit_apply_mapping
    }

    # Build the overlays
    set overlays_1d {}
    set mplot_2d_row 0
    set mplot2d {}
    foreach {subnum id ndim binned column_map} $short_subset_specs {
        if {$ndim == 1} {
            set overlay [hs::Unused_plot_name]
            lappend overlays_1d $overlay
            # Plot the dataset
            if {$binned} {
                # The dataset is either a 1d histogram or columns of an ntuple
                if {[string equal [hs::type $id] "HS_NTUPLE"]} {
                    foreach {ix iy iz iv ie} $column_map break
		    set xname [hs::variable_name $id $ix]
		    set vname [hs::variable_name $id $iv]
                    if {$ie < 0} {
                        # Do not have errors
                        hs::overlay $overlay \
				-xlabel $xname -ylabel $vname \
				add $id xys -x $xname -y $vname \
                                -line 0 -marker x -markersize medium
                    } else {
                        # Do have errors
                        hs::overlay $overlay \
				-xlabel $xname -ylabel $vname \
				add $id xyse -x $xname -y $vname \
                                -ey [hs::variable_name $id $ie] \
                                -line 0 -marker x -markersize medium
                    }
                    foreach {xmin xmax} [hs::column_range $id $ix] {}
                } else {
		    foreach {x_label y_label} [hs::1d_hist_labels $id] {}
                    if {[hs::hist_error_status $id] > 0} {
                        hs::overlay $overlay \
				-xlabel $x_label -ylabel $y_label \
				add $id -errors $haveerrors
                    } else {
                        hs::overlay $overlay \
				-xlabel $x_label -ylabel $y_label add $id
                    }
                    foreach {xmin xmax} [hs::1d_hist_range $id] {}
                }
            } else {
                # The dataset must be an ntuple row
                foreach {ix iy iz iv ie} $column_map break
                set varname [hs::variable_name $id $ix]
                set id_scat1d [hs::Prepare_1d_scatter_plot $id $varname \
                        [hs::Temp_uid] "Fit plot" [hs::Temp_category]]
                hs::overlay $overlay \
			add $id_scat1d xy -x $varname -y "00" \
                        -line 0 -marker x -markersize tiny -owner 1
                foreach {xmin xmax} [hs::column_range $id $ix] {}
            }
            # Overlay the fit
            set full_range [list $xmin $xmax $plotpoints]
            set rangelist [fit::Fit_subset $subnum ranges $full_range]
            if {![string equal $full_range [lindex $rangelist 0]]} {
                set id_under [hs::create_ntuple [hs::Temp_uid] "Fit plot" \
                        [hs::Temp_category] [list "X" "Excluded"]]
                if {$id_under <= 0} {
                    error "Failed to build a temporary ntuple"
                }
                hs::allow_reset_refresh $id_under 0
                lappend update_script "fit::Fit_subset $subnum\
                        plotfit \{[list $id_under $full_range]\}"
                hs::overlay $overlay add $id_under xy -x X -y Excluded \
                        -color $fitcolor -line 3 -owner 1
            }
            foreach range $rangelist {
                set id_over [hs::create_ntuple [hs::Temp_uid] "Fit plot" \
                        [hs::Temp_category] [list "X" "Fit"]]
                if {$id_over <= 0} {
                    error "Failed to build a temporary ntuple"
                }
                hs::allow_reset_refresh $id_over 0
                lappend update_script "fit::Fit_subset $subnum\
                        plotfit \{[list $id_over $range]\}"
                hs::overlay $overlay add $id_over xy  -x X -y Fit \
                        -color $fitcolor -line 11 -owner 1
            }
        } elseif {$ndim == 2} {
            # Can't overlay, have to plot side-by-side
            if {$mplot_2d_row == 0} {
                set mplot2d [hs::Unused_plot_name]
            }

            # Plot the dataset
            if {[string equal [hs::type $id] "HS_NTUPLE"]} {
                foreach {ix iy iz iv ie} $column_map break
                foreach {xmin xmax} [hs::column_range $id $ix] {}
                foreach {ymin ymax} [hs::column_range $id $iy] {}
                set x_label [hs::variable_name $id $ix]
                set y_label [hs::variable_name $id $iy]
                if {$binned} {
                    set z_label [hs::variable_name $id $iv]
                    hs::multiplot $mplot2d \
                            add $id 0,$mplot_2d_row scat3 \
                            -x $x_label -y $y_label -z $z_label \
                            -font $plotfont
                } else {
                    set z_label "Fit value"
                    hs::multiplot $mplot2d \
			    add $id 0,$mplot_2d_row scat2 \
                            -x $x_label -y $y_label -font $plotfont
                }
            } else {
                hs::multiplot $mplot2d \
			add $id 0,$mplot_2d_row -font $plotfont
            }

            # Plot the fit
            if {[string equal [hs::type $id] "HS_NTUPLE"]} {
		set n_bins_x 50
		set n_bins_y 50
                set id_fit [hs::create_2d_hist [hs::Temp_uid] "Fit plot" \
                        [hs::Temp_category] $x_label $y_label $z_label \
                        $n_bins_x $n_bins_y $xmin $xmax $ymin $ymax]
            } else {
                set id_fit [fit::Fit_subset $subnum fitvalues [hs::Temp_uid] \
                        "Fit plot" [hs::Temp_category] 0]
            }
	    lappend update_script [subst -nocommands {
		fit::Fit_subset $subnum plotfit [list $id_fit]
	    }]
	    if {!$binned} {
		# For unbinned plot, divide by the number of bins per unit
		# area (or, equivalently, multiply by bin area). In this way
		# the histogram created from data points will in the ideal
		# case look like the fit display.
		set binarea [expr {($xmax-$xmin)/$n_bins_x*($ymax-$ymin)/$n_bins_y}]
		lappend update_script [subst -nocommands {
		    hs::copy_data $id_fit $id_fit $binarea
		}]
	    }
            hs::multiplot $mplot2d add $id_fit 1,$mplot_2d_row -owner 1 -font $plotfont

            # Plot the difference between the data and the fit
            if {$binned} {
                if {[string equal [hs::type $id] "HS_NTUPLE"]} {
                    set id_diff [fit::Fit_subset $subnum fitvalues [hs::Temp_uid] \
                            "Data - fit difference" [hs::Temp_category] 1]
		    hs::allow_reset_refresh $id_diff 0
                    hs::multiplot $mplot2d add $id_diff 2,$mplot_2d_row scat3 \
			    -x $x_label -y $y_label -z $z_label -owner 1 -font $plotfont
		    lappend update_script [subst -nocommands {
			hs::allow_item_send 0
			set id_tmp [fit::Fit_subset $subnum fitvalues 2 \
				"Temp difference ntuple" "Fit_plot_update_tempdir" 1]
			hs::ntuple_block_fill $id_diff [hs::ntuple_contents [set id_tmp]]
			hs::delete [set id_tmp]
			hs::allow_item_send 1
		    }]
                } else {
                    set id_diff [hs::copy_hist $id [hs::Temp_uid] \
                            "Data - fit difference" [hs::Temp_category]]
                    hs::multiplot $mplot2d add $id_diff 2,$mplot_2d_row \
			    -owner 1 -font $plotfont
                    lappend update_script [subst -nocommands {
                        hs::allow_item_send 0
                        set id_tmp [hs::sum_histograms 1 "Temp difference histo"\
                                "Fit_plot_update_tempdir" $id $id_fit 1.0 -1.0]
                        catch {hs::copy_data $id_diff [set id_tmp]}
                        hs::delete [set id_tmp]
                        hs::allow_item_send 1
                    }]
                }
            } else {
                # Unfortunately, nothing useful comes to mind
		# here except to make yet another histogram.
		# This is not a good solution since, most often,
		# the data scarcity is the reason for not making
		# a data histogram to begin with. In the future,
		# we may want to at least choose the bin width
		# appropriately.
                set id_diff [hs::create_2d_hist [hs::Temp_uid] \
			"Data - fit difference" \
                        [hs::Temp_category] $x_label $y_label "Difference" \
                        $n_bins_x $n_bins_y $xmin $xmax $ymin $ymax]
		hs::multiplot $mplot2d add $id_diff 2,$mplot_2d_row \
			-owner 1 -font $plotfont
		# Refill the difference histo every time since
		# the ntuple could be changing
		set xv {$x}; set yv {$y}
		lappend update_script [subst -nocommands {
		    hs::copy_data $id_diff $id_fit -1.0
		    foreach x [hs::data_to_list [hs::column_contents $id $ix]]\
			    y [hs::data_to_list [hs::column_contents $id $iy]] {
			hs::fill_2d_hist $id_diff $xv $yv 1.0
		    }
		}]
            }
            incr mplot_2d_row
        } else {
            error "You have found a bug in [lindex [info level 0] 0]. Please report."
        }
    }

    # Arrange the 1d overlays into a multiplot
    set mplot1d_list [list]
    set n_overlays [llength $overlays_1d]
    if {$n_overlays > 0} {
        variable Max_fit_plots_in_multi
        set nchunks [expr {$n_overlays / $Max_fit_plots_in_multi}]
        if {[expr {$n_overlays % $Max_fit_plots_in_multi}]} {
            incr nchunks
        }
        for {set ichunk 0} {$ichunk < $nchunks} {incr ichunk} {
            set chunk_base [expr {$ichunk * $Max_fit_plots_in_multi}]
            if {$ichunk == [expr {$nchunks - 1}]} {
                set chunk_plots [expr {$n_overlays - $chunk_base}]
            } else {
                set chunk_plots $Max_fit_plots_in_multi
            }
            foreach {ncolumns nrows} [hs::Multiplot_optimal_grid $chunk_plots] {}
            set mplot1d_elem [hs::Unused_plot_name]
            for {set i 0} {$i < $chunk_plots} {incr i} {
                set overlay [lindex $overlays_1d [expr {$i + $chunk_base}]]
                set row [expr {$i / $ncolumns}]
                set column [expr {$i % $ncolumns}]
                ::hs::multiplot $mplot1d_elem add $overlay $column,$row
                set cell_over [hs::multiplot $mplot1d_elem getcell $column,$row]
                ::hs::overlay $cell_over -legend off -font $plotfont
                if {$draw_axis_labels} {
                    ::hs::overlay $cell_over \
			-xlabel [::hs::Get_overlay_property $overlay xlabel] \
			-ylabel [::hs::Get_overlay_property $overlay ylabel]
                }
                ::hs::overlay $overlay clear
            }
            lappend mplot1d_list $mplot1d_elem
        }
    }

    # Display the plots
    if {[string equal $wintitle ""]} {
	set fit_title [fit::Default_title]
    } else {
	set fit_title $wintitle
    }
    set winlist {}
    variable Fit_plot_window_counter
    if {$n_overlays > 0 && $mplot_2d_row > 0} {
        set fit_title_1d "$fit_title, plot 1 of 2"
        set fit_title_2d "$fit_title, plot 2 of 2"
    } else {
        set fit_title_1d $fit_title
        set fit_title_2d $fit_title
    }
    if {$n_overlays >= 1} {
        if {$n_overlays == 1} {
            set win "Hs_Fit_Plot_Window_[incr Fit_plot_window_counter]"
            lappend winlist $win
            hs::multiplot [lindex $mplot1d_list 0] show -title $fit_title_1d \
                    -geometry $geometry_single_1d -window $win
            fit::Fit_tcldata set win1d $win
        } else {
            set n_mplot1ds [llength $mplot1d_list]
            set mpcount 0
            foreach mplot1d_elem $mplot1d_list {
                set win "Hs_Fit_Plot_Window_[incr Fit_plot_window_counter]"
                lappend winlist $win
                if {$n_mplot1ds > 1} {
                    set wintitle "$fit_title_1d ([incr mpcount]/$n_mplot1ds)"
                } else {
                    set wintitle $fit_title_1d
                }
                hs::multiplot $mplot1d_elem show -title $wintitle \
                    -geometry $geometry_multi -window $win
            }
        }
    }
    if {$mplot_2d_row >= 1} {
        set win "Hs_Fit_Plot_Window_[incr Fit_plot_window_counter]"
        lappend winlist $win
        if {$mplot_2d_row == 1} {
            hs::multiplot $mplot2d show -title $fit_title_2d \
                    -geometry $geometry_single_2d -window $win
        } else {
            hs::multiplot $mplot2d show -title $fit_title_2d \
                    -geometry $geometry_multi -window $win
        }
	fit::Fit_tcldata set win2d $win
    }

    # Finalize the updating script
    lappend update_script "\}"
    set update_script [join $update_script "\n"]
    eval $update_script

    # Check if we need to return list of windows and updating script
    if {![string equal $winlist_name ""]} {
        upvar $winlist_name wlist
        set wlist $winlist
    }
    if {![string equal $script_name ""]} {
        upvar $script_name scr
        set scr $update_script
    }

    # Show the fit info if requested and if we have
    # only one window in the multiplot
    if {$show_info} {
        variable Fit_printout_precision
        fit::Eval_at_precision $Fit_printout_precision {
            set result_info [fit::Fit_parameters_comment]
        }
        if {$n_overlays == 1} {
            hs::comment $result_info {-7 winabs r -7 winabs r} \
                -bg white -refpoint {1 1} -border on -anchor ne \
                -font {courier 12} -window [fit::Fit_tcldata get win1d]
        }
        if {$mplot_2d_row == 1} {
            hs::comment $result_info {-7 winabs r -7 winabs r} \
                -bg white -refpoint {1 1} -border on -anchor ne \
                -font {courier 12} -window [fit::Fit_tcldata get win2d]
        }
    }

    # Return the list of multiplots
    return [concat $mplot1d_list $mplot2d]
}

############################################################################

proc ::fit::Fit_tuner {cmdname args} {
    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }

    # Initialize necessary local variables
    set plotfont ""
    set haveerrors on

    # Go over the list of options
    set available_options {-font -errors}
    if {[expr [llength $args] % 2] != 0} {
        error "wrong # of arguments"
    }
    foreach {option value} $args {
        if {[lsearch -exact $available_options $option] < 0} {
            error "Invalid option \"$option\". Available options\
                    are [join [lsort $available_options] {, }]."
        }
        regsub "^-" $option "" option
        switch -- $option {
	    font {
		set plotfont $value
		if {![string equal $plotfont ""]} {
		    hs::Generate_xlfd $plotfont
		}
	    }
            errors {
                set haveerrors $value
            }
            default {
                error "Incomplete switch statement in [lindex [info level 0] 0].\
                        This is a bug. Please report."
            }
        }
    }

    # Check active fit
    set fitname [fit::Fit_get_active]
    if {[string equal $fitname ""]} {
	error "No active fit"
    }

    # Make sure that we can tune something...
    set parnames [fit::Fit_parameter list]
    if {[llength $parnames] == 0} {
	error "No parameters defined -- nothing to tune!"
    }

    # Make sure we can create the fit plot.
    # This will generate an error if Minuit is busy right now.
    set multiplot_list [fit::Fit_plot -font $plotfont \
	    -script update_script -winlist winlist -errors $haveerrors]
    ::hs::hs_update

    # Create the toplevel window
    variable Fit_tuner_counter
    set status 1
    while {$status} {
	set tuner_id [incr Fit_tuner_counter]
	set topwin .hs_fit_tuner_$tuner_id
	set status [catch {toplevel $topwin}]
    }
    wm withdraw $topwin
    set fittitle [fit::Fit_cget -title]
    if {[string is space $fittitle]} {
        set window_title "Fit tuner for $cmdname"
    } else {
        set window_title "Fit tuner: $fittitle ($cmdname)"
    }
    wm title $topwin $window_title

    # Initialize internal data
    variable Fit_tuner_data
    set Fit_tuner_data($tuner_id,name) $fitname
    set Fit_tuner_data($tuner_id,globals) {}
    set Fit_tuner_data($tuner_id,multiplots) $multiplot_list
    set Fit_tuner_data($tuner_id,overlays) {}
    set Fit_tuner_data($tuner_id,update_script) $update_script
    set Fit_tuner_data($tuner_id,winlist) $winlist
    set Fit_tuner_data($tuner_id,win0) [lindex $winlist 0]
    set Fit_tuner_data($tuner_id,cleanup_script) {}
    set Fit_tuner_data($tuner_id,locked) 0
    set Fit_tuner_data($tuner_id,plotresult) 0
    set Fit_tuner_data($tuner_id,font) $plotfont
    set Fit_tuner_data($tuner_id,maxcalls) 2147483647
    set Fit_tuner_data($tuner_id,haveerrors) $haveerrors

    # The "busy lock" dummy frame
    frame $topwin.busylock
    pack $topwin.busylock -side top

    # Create the dataset properties frame
    pack [::fit::Fit_tuner_dataset_frame $topwin.dataframe $tuner_id]\
            -side top -padx 4 -pady 3 -fill x

    # Create the parameter tuning frame
    # ::hs::Horizontal_separator $topwin
    pack [::fit::Fit_tuner_parameter_frame $topwin.pframe $tuner_id]\
            -side top -padx 4 -pady 4 -fill x

    # Create the fit options frame
    pack [::fit::Fit_tuner_options_frame $topwin.optframe $tuner_id]\
            -side top -padx 4 -pady 4 -fill x    

    # Create the fit status display
    ::hs::Horizontal_separator $topwin
    frame $topwin.done
    pack $topwin.done -padx 2 -pady 2 -side top -fill x -expand 1
    frame $topwin.done.status
    label $topwin.done.status.lcl -text " CL (%) :"
    set labelfont [$topwin.done.status.lcl cget -font]
    set thinfont [lrange $labelfont 0 1]
    set labelbg [$topwin.done.status.lcl cget -background]
    set t $topwin.done.status.t
    label $t -text "Fit \u03C7\u00B2 / d.o.f :"
    foreach name {chisq cl} {
        set l $topwin.done.status.$name
        label $l -text "" -anchor w
        set cmd "$l configure -text {} -bg $labelbg ;#"
        fit::Fit_callback $fitname add lostsync $cmd
        lappend Fit_tuner_data($tuner_id,cleanup_script) \
                [list fit::Fit_callback $fitname delete lostsync $cmd]
    }

    # Create the error contours frame
    if {[llength $parnames] > 1} {
        fit::Fit_tuner_contours_frame $topwin.done.contframe $tuner_id
        pack $topwin.done.contframe -side right
        grid $t -column 0 -row 0 -sticky e -pady 2
        grid $topwin.done.status.lcl -column 0 -row 1 -sticky e -padx 1 -pady 4
        grid $topwin.done.status.chisq -column 1 -row 0 -sticky sew
        grid $topwin.done.status.cl  -column 1 -row 1 -sticky ew
        pack $topwin.done.status -side left
    } else {
        grid $t -column 0 -row 0 -sticky e -pady 2
        $topwin.done.status.chisq configure -width 12
        grid $topwin.done.status.chisq -column 1 -row 0 -sticky sw
        grid $topwin.done.status.lcl -column 2 -row 0 -sticky se -padx 1
        grid $topwin.done.status.cl -column 3 -row 0 -sticky sw
        grid columnconfigure $topwin.done.status 4 -weight 1
        pack $topwin.done.status -side top -fill x -expand 1
    }

    # Main action buttons
    ::hs::Horizontal_separator $topwin
    frame $topwin.buttons
    pack $topwin.buttons -side top -fill x -padx 4 -pady 4
    set buttonwidth 9
    button $topwin.buttons.plot -text "New plot" -width $buttonwidth \
            -command [list ::fit::Fit_tuner_newplot $tuner_id]
    button $topwin.buttons.fit -text "Fit" -width $buttonwidth \
            -command [list ::fit::Fit_tuner_fit $tuner_id]
    set destroy_cmd [list ::fit::Fit_tuner_destroy $tuner_id $topwin]
    button $topwin.buttons.dismiss -text "Dismiss" -width $buttonwidth \
            -command $destroy_cmd
    button $topwin.buttons.report -text "Report" -width $buttonwidth \
	    -command [list ::fit::Fit_tuner_report $tuner_id] \
	    -state disabled
    foreach {callback_type state} {
	lostsync disabled
	complete normal
    } {
	set cmd "$topwin.buttons.report configure -state $state ;#"
	fit::Fit_callback $fitname add $callback_type $cmd
	lappend Fit_tuner_data($tuner_id,cleanup_script) \
		[list fit::Fit_callback $fitname delete $callback_type $cmd]
    }
    foreach {name row col} {
        fit     0 0
        plot    0 1
	report  0 2
        dismiss 0 3
    } {
        grid $topwin.buttons.$name -row $row -column $col -padx 1 -sticky ew
        if {$row == 0} {
            grid columnconfigure $topwin.buttons $col -weight 1 -pad 2
        }
    }

    # We will send "clear" to the last window in case
    # the sync is lost and we draw results on it
    set cmd "fit::Fit_tuner_clear_plotwindow $tuner_id ;#"
    fit::Fit_callback $fitname add lostsync $cmd
    lappend Fit_tuner_data($tuner_id,cleanup_script) \
	    [list fit::Fit_callback $fitname delete lostsync $cmd]

    # Update the geometry and draw the toplevel window 
    update idletasks
    wm deiconify $topwin
    wm protocol $topwin WM_DELETE_WINDOW $destroy_cmd
    wm resizable $topwin 1 0

    fit::Fit_callback $fitname add destruct $destroy_cmd
    lappend Fit_tuner_data($tuner_id,cleanup_script) [list\
            fit::Fit_callback $fitname delete destruct $destroy_cmd]
    return $topwin
}

############################################################################

proc ::fit::Fit_tuner_options_frame {w tuner_id} {
    variable Fit_tuner_data

    # Assume that the active fit is set correctly
    # Create the minimizer choice menu
    frame $w
    set label_bg [$w cget -background]
    label $w.minilabel -text "Minimizer :" -bg $label_bg
    global _Hs_Fit_tuner_minimizer_$tuner_id
    lappend Fit_tuner_data($tuner_id,globals) _Hs_Fit_tuner_minimizer_$tuner_id
    tk_optionMenu $w.minimenu _Hs_Fit_tuner_minimizer_$tuner_id \
	    "migrad" "mini" "simplex"
    set _Hs_Fit_tuner_minimizer_$tuner_id [fit::Fit_cget -minimizer]
    set cmd [list ::fit::Fit_tuner_options_tracer $tuner_id minimizer]
    trace variable _Hs_Fit_tuner_minimizer_$tuner_id w $cmd

    # Create the precision entry
    label $w.space0 -bg $label_bg -text "     "
    label $w.preclabel -text "Precision :"
    entry $w.precentry -width 10
    $w.precentry insert 0 [fit::Fit_cget -precision]
    $w.precentry configure -validate key -vcmd {
	%W configure -foreground red2
	return 1
    }
    bind $w.precentry <KeyPress-Return> \
	    [list ::fit::Fit_tuner_option_entry \
	    $tuner_id precision $w.precentry \
	    [list hs::Validate_string double >=0.0]]

    # Create the number of iterations entry
    label $w.space1 -bg $label_bg -text "     "
    label $w.maxclabel -text "Max calls :"
    entry $w.maxcentry -width 10
    $w.maxcentry insert 0 $Fit_tuner_data($tuner_id,maxcalls)
    $w.maxcentry configure -validate key -vcmd {
	%W configure -foreground red2
	return 1
    }
    bind $w.maxcentry <KeyPress-Return> \
	    [list ::fit::Fit_tuner_set_maxcalls $tuner_id $w.maxcentry]

    pack $w.maxclabel $w.maxcentry $w.space0 \
	    $w.preclabel $w.precentry $w.space1 \
	    $w.minilabel $w.minimenu -side left
    return $w
}

############################################################################

proc ::fit::Fit_tuner_set_maxcalls {tuner_id entryname} {
    set value [$entryname get]
    if {[::hs::Validate_string integer >0 $value]} {
	$entryname configure -fg black
	variable Fit_tuner_data
	set Fit_tuner_data($tuner_id,maxcalls) $value
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_clear_plotwindow {tuner_id} {
    variable Fit_tuner_data
    if {$Fit_tuner_data($tuner_id,plotresult)} {
	hs::clear -window $Fit_tuner_data($tuner_id,win0)
	set Fit_tuner_data($tuner_id,plotresult) 0
    }
    return
}

############################################################################

proc ::fit::Fit_busy_eval {tuner_id script} {
    set topwin .hs_fit_tuner_$tuner_id
    set busyframe $topwin.busylock
    set fwin [focus]
    focus $busyframe
    grab set $busyframe
    set cursor [$topwin cget -cursor]
    $topwin configure -cursor watch
    variable Fit_tuner_data
    set Fit_tuner_data($tuner_id,locked) 1
    # set period [::hs::periodic_update period]
    # ::hs::periodic_update stop
    update

    global ::errorInfo
    set status [catch {uplevel $script} result]
    set saveInfo $::errorInfo

    # if {$period > 0 && [::hs::periodic_update period] == 0} {
    #	::hs::periodic_update $period
    # }
    if {[info exists Fit_tuner_data($tuner_id,locked)]} {
	set Fit_tuner_data($tuner_id,locked) 0
    }
    catch {$topwin configure -cursor $cursor}
    catch {grab release $busyframe}
    catch {focus $fwin}
    if {$status} {
	error $result $saveInfo
    }
    return $result
}

############################################################################

proc ::fit::Fit_tuner_report {tuner_id} {
    variable Fit_tuner_data
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
	if {![fit::Fit_cget -complete]} {
	    error "Fitting is not complete"
	}
        set types [list \
                [list "Text files"  ".txt"]\
                [list "All files"      "*"]\
                ]
	set filename [tk_getSaveFile -filetypes $types \
		-parent .hs_fit_tuner_$tuner_id -initialfile "stdout"]
	if {![string equal $filename ""]} {
	    set tail [file tail $filename]
	    if {[string equal $tail "stdout"] || \
                    [string equal $tail "stdout.txt"]} {
		set chan stdout
		set close_when_done 0
	    } elseif {[string equal $tail "stderr"] || \
                          [string equal $tail "stderr.txt"]} {
		set chan stderr
		set close_when_done 0
	    } else {
		if {[catch {open $filename w} chan]} {
		    error "failed to open file named\n\"$filename\""
		}
		set close_when_done 1
	    }
	    fit::Fit_report $chan text
	    if {$close_when_done} {
		close $chan
	    }
	}
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_newplot {tuner_id} {
    variable Fit_tuner_data
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        set multiplot_list [fit::Fit_plot \
		-font $Fit_tuner_data($tuner_id,font) \
                -script update_script -winlist winlist \
                -errors $Fit_tuner_data($tuner_id,haveerrors)]
    }
    eval lappend Fit_tuner_data($tuner_id,multiplots) $multiplot_list
    eval lappend Fit_tuner_data($tuner_id,winlist) $winlist
    set Fit_tuner_data($tuner_id,win0) [lindex $winlist 0]
    set Fit_tuner_data($tuner_id,update_script) $update_script
    ::hs::hs_update
    return
}

############################################################################

proc ::fit::Fit_tuner_valid_methods {tuner_id subset menu} {
    variable Fit_tuner_data
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        set valid_methods [::fit::Fit_subset $subset cget -validmethods]
        set meth [::fit::Fit_subset $subset cget -method]
    }
    global _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id
    if {![string equal $meth [set _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id]]} {
        trace vdelete _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id w \
                [list ::fit::Fit_tuner_trace_methods $tuner_id $subset]
        set _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id $meth
        trace variable _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id w \
                [list ::fit::Fit_tuner_trace_methods $tuner_id $subset]
    }
    $menu delete 0 end
    foreach meth $valid_methods {
        $menu add radiobutton -label $meth -value $meth \
                -variable _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id
    }
}

############################################################################

proc ::fit::Fit_tuner_trace_methods {tuner_id subset name el op} {
    variable Fit_tuner_data
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        global _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id
        ::fit::Fit_subset $subset configure -method \
                [set _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id]
    }
}

############################################################################

proc ::fit::Fit_tuner_dataset_frame {frame tuner_id} {

    variable Fit_tuner_data
    set fitname $Fit_tuner_data($tuner_id,name)

    frame $frame
    # label $frame.label -text "Data properties" -background white
    # pack $frame.label -side top -fill x -pady 4

    set tbl $frame.data
    frame $tbl
    pack $tbl -side top -fill x
    set bgcolor [$tbl cget -background]

    label $tbl.lsubset -text "Set #"
    label $tbl.lid -text "Item id"
    label $tbl.ltitle -text "Item title"
    label $tbl.lmeth -text "Fit method"
    label $tbl.lweight -text "Weight"
    set labelfont [$tbl.lweight cget -font]
    set thinfont [lrange $labelfont 0 1]
    set t $tbl.lfilter
    label $t -text "\u03C7\u00B2 / Points"
    label $tbl.lcl -text "CL (%)"
    set nsets [llength [::fit::Fit_subset list]]
    set ncolums 14

    set row 0
    # frame $tbl.horsep0 -height 2 -borderwidth 1 -relief sunken
    # grid $tbl.horsep0 -row $row -column 0 -columnspan $ncolums -sticky ew
    # incr row

    set col 0
    foreach name {subset id title meth} {
        grid $tbl.l$name -row $row -column $col -sticky ew
        incr col
        frame $tbl.lsep$col -width 2 -borderwidth 1 -relief sunken
        grid $tbl.lsep$col -row 0 -column $col \
                -rowspan [expr {$nsets + 2}] -sticky ns
        incr col
    }
    grid $tbl.lweight -row $row -column $col -columnspan 2 -sticky ew
    incr col 2
    frame $tbl.lsep$col -width 2 -borderwidth 1 -relief sunken
    grid $tbl.lsep$col -row 0 -column $col -padx 3 \
            -rowspan [expr {$nsets + 2}] -sticky ns
    incr col
    # grid $tbl.lfilter -row $row -column $col -sticky ew
    grid $tbl.lfilter -row $row -column $col
    incr col
    frame $tbl.lsep$col -width 2 -borderwidth 1 -relief sunken
    grid $tbl.lsep$col -row 0 -column $col -padx 3 \
            -rowspan [expr {$nsets + 2}] -sticky ns
    incr col
    grid $tbl.lcl -row $row -column $col -sticky ew
    incr row

    frame $tbl.horsep1 -height 2 -borderwidth 1 -relief sunken
    grid $tbl.horsep1 -row $row -column 0 -columnspan $ncolums -sticky ew -pady 1
    incr row

    foreach subset [::fit::Fit_subset list] {
        foreach {id ndim binned columns filter functions weight meth} \
                [fit::Fit_subset $subset info] break
        label $tbl.subset$subset -text $subset
        label $tbl.id$subset -text $id
        set title " [::hs::title $id]"
        label $tbl.title$subset -text $title -anchor w

        global _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id
        lappend Fit_tuner_data($tuner_id,globals) \
                _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id
        tk_optionMenu $tbl.meth$subset \
                _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id $meth
        $tbl.meth$subset configure -width 5
        set _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id $meth
        set menu $tbl.meth$subset.menu
        $menu configure -postcommand [list \
                ::fit::Fit_tuner_valid_methods $tuner_id $subset $menu]
        trace variable _Hs_Fit_tuner_choose_meth_${subset}_$tuner_id w \
                [list ::fit::Fit_tuner_trace_methods $tuner_id $subset]

        label $tbl.filter$subset -text "" -font $thinfont
	label $tbl.cl$subset -text "" -font $thinfont
        global _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id
        lappend Fit_tuner_data($tuner_id,globals) \
                _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id
        entry $tbl.wentry$subset -width 4 -textvariable \
                _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id \
                -validate key -validatecommand \
                [list ::fit::Fit_tuner_validate_weight $tuner_id $subset %P]
        set _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id $weight
        button $tbl.wsw$subset -text "0/1" -width 1 -command [list\
                fit::Fit_tuner_switch_weight $tuner_id $subset $tbl.wsw$subset 0]
        set col 0
        foreach name {subset id title} {
            grid $tbl.${name}$subset -row $row -column $col -sticky ew
            incr col 2
        }
        grid $tbl.meth$subset -row $row -column $col -sticky ew -padx 3
        incr col 2
        grid $tbl.wentry$subset -row $row -column $col -sticky ew -padx 3
        incr col
        grid $tbl.wsw$subset -row $row -column $col -sticky ew
        foreach name {filter cl} {
            incr col 2
            grid $tbl.${name}$subset -row $row -column $col -sticky ew
            set cmd "$tbl.${name}$subset configure -text {} ;#"
            fit::Fit_callback $fitname add lostsync $cmd
            lappend Fit_tuner_data($tuner_id,cleanup_script) \
                    [list fit::Fit_callback $fitname delete lostsync $cmd]
        }
        incr row
    }
    frame $tbl.horsep2 -height 2 -borderwidth 1 -relief sunken
    grid $tbl.horsep2 -row $row -column 0 -columnspan $ncolums -sticky ew -pady 1
    grid columnconfigure $tbl 4 -weight 1

    return $frame
}

############################################################################

proc ::fit::Fit_tuner_validate_weight {tuner_id subset newvalue} {
    if {![string is double $newvalue]} {
        return 0
    }
    if {![string equal $newvalue ""]} {
        if {$newvalue < 0.0} {
            return 0
        }
        variable Fit_tuner_data
        fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
            fit::Fit_subset $subset config -weight $newvalue
        }
    }
    return 1
}

############################################################################

proc ::fit::Fit_tuner_switch_weight {tuner_id subset button value} {
    global _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id
    if {$value} {
        set weight 1.0
        set newvalue 0
    } else {
        set weight 0.0
        set newvalue 1
    }
    $button configure -command [list\
            fit::Fit_tuner_switch_weight $tuner_id $subset $button $newvalue]
    set _Hs_Fit_tuner_choose_weight_${subset}_$tuner_id $weight
    return
}

############################################################################

proc ::fit::Fit_tuner_fit {tuner_id} {
    variable Fit_tuner_data
    fit::Fit_activate $Fit_tuner_data($tuner_id,name)

    # Check that there is at least one dataset with non-zero weight
    set have_nonzero_weight 0
    set subset_list [fit::Fit_subset list]
    set n_subsets [llength $subset_list]
    foreach subset $subset_list {
        if {[fit::Fit_subset $subset cget -weight] > 0.0} {
            set have_nonzero_weight 1
            break
        }
    }
    if {!$have_nonzero_weight} {
        if {$n_subsets > 1} {
            set errmess "Please make at least one\
                dataset weight positive"
        } else {
            set errmess "Please make the dataset weight positive"
        }
        tk_dialog .hs_fit_tuner_$tuner_id.error_dialog \
                "Fit Error" $errmess error 0 "Acknowledged"
        return
    }

    # Check if a dataset with L2 norm minimization
    # is combined with any other dataset
    set have_l2 0
    foreach subset $subset_list {
        if {[string equal [fit::Fit_subset $subset cget -method] "L2"]} {
            set have_l2 1
            break
        }
    }
    if {$have_l2 && $n_subsets > 1} {
        set errmess "Fitting a dataset using L2 distance minimization\
                together with any other dataset is extremely\
                unlikely to produce sensible parameter error estimates"
        variable post_multiple_l2_datasets_warning
        if {![info exists post_multiple_l2_datasets_warning]} {
            set post_multiple_l2_datasets_warning 1
        }
        if {$post_multiple_l2_datasets_warning} {
            set index [tk_dialog .hs_fit_tuner_$tuner_id.error_dialog \
                    "Warning" $errmess warning 0 "Abort" \
                    "Proceed Anyway" "Proceed Always"]
            if {$index == 0} return
            if {$index == 2} {
                set post_multiple_l2_datasets_warning 0
            }
        }
    }

    # Check that Minuit is not busy
    if {[fit::Fit_lock_minuit]} {
	error "Minuit is busy"
    }

    variable Fit_printout_precision
    fit::Eval_at_precision $Fit_printout_precision {
	# Remember the "ignore errors" status
	set ignore_errors [fit::Fit_cget -ignore]
	# Make sure this status is restored before return
	global ::errorInfo
	set status [catch {
	    set ignore_on 0
	    while {[set has_errors [catch {fit::Fit_busy_eval $tuner_id\
		    [list fit::Fit_fit $Fit_tuner_data($tuner_id,maxcalls)]} errmess]]} {
		if {$ignore_on} {
		    catch {::mn::comd call 3}
		    break
		}
		set index [tk_dialog .hs_fit_tuner_$tuner_id.error_dialog \
			"Fit Error" $errmess error 0 \
			"Abort Fit Attempt" "Ignore Errors"]
		# While the dialog is up, the user may do other things,
		# for example, run another fit. Reactivate the current one.
		fit::Fit_activate $Fit_tuner_data($tuner_id,name)
		if {$index == 1} {
		    # Retry the fit while ignoring errors
		    set ignore_on 1
		    fit::Fit_config -ignore 1
		} else {
		    break
		}
	    }
	    # Update the scales if the minimization is OK,
	    # or renew the fit parameters if it went sour.
	    set topwin .hs_fit_tuner_$tuner_id
	    set scales $topwin.pframe.scales
	    set pnum 0
	    if {!$has_errors || $ignore_on} {
		# Disable the function which links scales with fit parameters.
		# We don't want to touch the fit at this point.
		::rename ::fit::Fit_tuner_adjust_parameter \
			::fit::Fit_tuner_adjust_parameter_tmp
		::rename ::fit::Fit_tuner_adjust_parameter_dummy \
			::fit::Fit_tuner_adjust_parameter
		set stat2 [catch {
		    foreach parameter [fit::Fit_parameter info] {
			foreach {pname value state step bounds} $parameter break
			::hs::Scale_pack_set $scales $pname $value
			global _Fit_tuner_param_step_${tuner_id}_$pnum
			set _Fit_tuner_param_step_${tuner_id}_$pnum $step
			$topwin.pframe.step_$pnum configure -foreground black
			incr pnum
		    }
		} errira]
		set localError $::errorInfo
		# Reenable the function which links scales with fit parameters
		::rename ::fit::Fit_tuner_adjust_parameter \
			::fit::Fit_tuner_adjust_parameter_dummy
		::rename ::fit::Fit_tuner_adjust_parameter_tmp \
			::fit::Fit_tuner_adjust_parameter
		if {$stat2} {
		    error $errira $localError
		}
		# Update the Minos error display
		foreach bname {llolim lhilim} {
		    $topwin.pframe.$bname invoke
		    global _Hs_Fit_tuner_is_${bname}_$tuner_id
		    if {![string equal -length 5 "Minos" \
			    [set _Hs_Fit_tuner_is_${bname}_$tuner_id]]} {
			$topwin.pframe.$bname invoke
		    }
		}
		# Update the confidence levels
		set tbl $topwin.dataframe.data
		foreach subset [fit::Fit_subset list] {
		    set can_use_chisq 0
		    if {[fit::Fit_subset $subset cget -binned]} {
			if {![string equal \
				[fit::Fit_subset $subset cget -method] "L2"]} {
			    set npoints [fit::Fit_subset $subset cget -ndof]
			    if {$npoints > 0} {
				set can_use_chisq 1
				set chisq [fit::Fit_subset $subset cget -chisq]
				$tbl.filter$subset configure -text \
					"[format {%.4g} $chisq] / $npoints"
				set cl [::hs::function chisq_tail eval \
					[list x $chisq] [list n $npoints]]
				$tbl.cl$subset configure -text \
					[format {%.4g} [expr {$cl * 100.0}]]
			    }
			}
		    }
		    if {!$can_use_chisq} {
			$tbl.filter$subset configure -text "---"
			$tbl.cl$subset configure -text "---"
		    }
		}
		foreach {total_chisq total_points} [fit::Fit_chisq] {}
		if {$total_chisq >= 0.0} {
		    $topwin.done.status.chisq configure -bg white -text \
			    "[format {%.4g} $total_chisq] / $total_points"
		    if {$total_points > 0} {
			set cl [::hs::function chisq_tail eval \
				[list x $total_chisq] [list n $total_points]]
			$topwin.done.status.cl configure -bg white -text \
				[format {%.4g} [expr {$cl * 100.0}]]
		    }
		} else {
		    $topwin.done.status.chisq configure -text "---"
		    $topwin.done.status.cl configure -text "---"
		}
		# Show the result on top of the plot
		# in case we have just one 1d dataset
		if {$n_subsets == 1} {
                    if {[fit::Fit_subset 0 cget -ndim] == 1} {
                        set result_info [fit::Fit_parameters_comment]
                        set Fit_tuner_data($tuner_id,plotresult) 1
                        hs::comment $result_info {-7 winabs r -7 winabs r} \
			    -bg white -refpoint {1 1} -border on -anchor ne \
			    -font {courier 12} -window $Fit_tuner_data($tuner_id,win0)
                    }
		}
	    } else {
		foreach pname [fit::Fit_parameter list] {
		    set value [::hs::Scale_pack_get $scales $pname]
		    global _Fit_tuner_param_step_${tuner_id}_$pnum
		    set step [set _Fit_tuner_param_step_${tuner_id}_$pnum]
		    fit::Fit_parameter $pname configure -value $value -step $step
		    incr pnum
		}
	    }
	    # Update the plot
	    eval $Fit_tuner_data($tuner_id,update_script)
	    ::hs::hs_update
	} errmess]
	set tempInfo $::errorInfo
	catch {fit::Fit_config -ignore $ignore_errors}
	if {$status} {
	    error $errmess $tempInfo
	}
    }
    return
}

############################################################################

proc ::fit::Fit_parameters_comment {} {
    set mxlen 2
    set parameter_names [fit::Fit_parameter list]
    foreach name $parameter_names {
	set len [string length $name]
	if {$len > $mxlen} {
	    set mxlen $len
	}
    }
    set format "%${mxlen}s = %s"
    set result ""
    foreach name $parameter_names {
	set value [fit::Fit_parameter $name cget -value]
	set status [fit::Fit_parameter $name cget -state]
	if {[string equal $status "fixed"]} {
	    set description "$value (fixed)"
	} else {
	    set error [fit::Fit_parameter $name cget -error]
	    set description "$value \xb1 $error"
	}
	append result [format $format $name $description] "\n"
    }
    # Not all types of fits support chi-square goodness of fit test
    if {![catch {fit::Fit_goodness chisq} cl]} {
	set cl [expr {$cl * 100.0}]
	append result [format $format "CL" "$cl %"]
    }
    set result
}

############################################################################

proc ::fit::Fit_tuner_option_entry {tuner_id optname entryname validator} {
    variable Fit_tuner_data
    ::fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
	set value [$entryname get]
	if {[eval $validator [list $value]]} {
	    $entryname config -fg black
	    if {![string equal $optname ""]} {
		::fit::Fit_config -$optname $value
	    }
	}
    }
    return
}

############################################################################

proc ::hs::Validate_string {type condition value} {
    if {[string is $type -strict $value]} {
	if {[expr $value $condition]} {
	    return 1
	}
    }
    return 0
}

############################################################################

proc ::fit::Fit_tuner_options_tracer {tuner_id optname name el op} {
    variable Fit_tuner_data
    ::fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
	global $name
	::fit::Fit_config -$optname [set $name]
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_contours_tracer {tuner_id button name el op} {
    global _Hs_Fit_tuner_choose_p1_$tuner_id _Hs_Fit_tuner_choose_p2_$tuner_id
    set p1 [set _Hs_Fit_tuner_choose_p1_$tuner_id]
    set p2 [set _Hs_Fit_tuner_choose_p2_$tuner_id]
    set enable_button [string compare $p1 $p2]
    if {$enable_button} {
        set levels {}
        for {set i 1} {$i < 6} {incr i} {
            global _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id
            if {[set _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id]} {
                lappend levels $i
            }
        }
        set enable_button [llength $levels]
    }
    if {$enable_button} {
        variable Fit_tuner_data
        ::fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
            if {![::fit::Fit_cget -complete]} {
                set enable_button 0
            }
            if {$enable_button} {
                if {[string equal [::fit::Fit_parameter $p1 cget -state] "fixed"]} {
                    set enable_button 0
                }
            }
            if {$enable_button} {
                if {[string equal [::fit::Fit_parameter $p2 cget -state] "fixed"]} {
                    set enable_button 0
                }
            }
        }
    }
    if {$enable_button} {
        $button configure -state normal
    } else {
        $button configure -state disabled
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_enable_contours {tuner_id fitname callback_type} {
    # Just trigger the trace -- it will perform all relevant research
    global _Hs_Fit_tuner_choose_p1_$tuner_id
    set _Hs_Fit_tuner_choose_p1_$tuner_id [set _Hs_Fit_tuner_choose_p1_$tuner_id]
    # Turn off the Minos error display if necessary
    if {[string equal $callback_type "lostsync"]} {
        set topwin .hs_fit_tuner_$tuner_id
        foreach bname {llolim lhilim} {
            global _Hs_Fit_tuner_is_${bname}_$tuner_id
            if {[string equal -length 5 "Minos" \
                    [set _Hs_Fit_tuner_is_${bname}_$tuner_id]]} {
                $topwin.pframe.$bname invoke
                $topwin.pframe.$bname invoke
            }    
        }
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_set_contourpoints {tuner_id npoints} {
    variable Fit_tuner_data
    set Fit_tuner_data($tuner_id,contourpoints) $npoints
    return
}

############################################################################

proc ::fit::Fit_tuner_plot_contours {tuner_id} {
    set topwin .hs_fit_tuner_$tuner_id
    variable Fit_tuner_data
    set npoints $Fit_tuner_data($tuner_id,contourpoints)
    variable Fit_plot_window_counter
    set wname "Hs_Fit_Plot_Window_[incr Fit_plot_window_counter]"
    global _Hs_Fit_tuner_choose_p1_$tuner_id _Hs_Fit_tuner_choose_p2_$tuner_id
    set pname1 [set _Hs_Fit_tuner_choose_p1_$tuner_id]
    set pname2 [set _Hs_Fit_tuner_choose_p2_$tuner_id]
    set levels {}
    for {set i 1} {$i < 6} {incr i} {
        global _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id
        if {[set _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id]} {
            lappend levels $i
        }
    }
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        set all_l2 1
        foreach subset [fit::Fit_subset list] {
            if {![string equal [fit::Fit_subset $subset cget -method] "L2"]} {
                set all_l2 0
                break
            }
        }
        set default_errdef {}
        if {$all_l2} {
            foreach {fmin fedm errdef npari nparx istat} [::mn::stat] {}
            set datapoints [fit::Fit_cget -wpoints]
            if {$datapoints > $npari} {
                set default_errdef [expr {$fmin/($datapoints-$npari)}]
            }
        }
        set overlay [fit::Fit_busy_eval $tuner_id {fit::Fit_error_contour \
		$pname2 $pname1 -levels $levels -errdef $default_errdef \
                -plotpoints $npoints -window $wname \
		-font $Fit_tuner_data($tuner_id,font)}]
    }
    lappend Fit_tuner_data($tuner_id,overlays) $overlay
    lappend Fit_tuner_data($tuner_id,winlist) $wname
    return
}

############################################################################

proc ::fit::Fit_tuner_contours_frame {frame tuner_id} {
    frame $frame ;# -relief groove -borderwidth 4
    set topframe $frame.top
    frame $topframe
    set botframe $frame.bot
    frame $botframe
    pack $topframe $botframe -side top -padx 4 -pady 2 -fill x -expand 1

    button $topframe.plot -text "Plot error contours" -command \
            [list ::fit::Fit_tuner_plot_contours $tuner_id]
    variable Fit_tuner_data
    set fitname $Fit_tuner_data($tuner_id,name)
    set cmd [list ::fit::Fit_tuner_enable_contours $tuner_id]
    fit::Fit_callback $fitname add complete $cmd
    fit::Fit_callback $fitname add lostsync $cmd
    lappend Fit_tuner_data($tuner_id,cleanup_script) \
            [list fit::Fit_callback $fitname delete complete $cmd] \
            [list fit::Fit_callback $fitname delete lostsync $cmd]

    label $topframe.lbl1 -text "of"
    # Figure out optimal menu width
    set menuwidth 0
    set parnames [fit::Fit_parameter list]
    foreach pname $parnames {
        set len [string length $pname]
        if {$len > $menuwidth} {
            set menuwidth $len
        }
    }
    global _Hs_Fit_tuner_choose_p1_$tuner_id
    lappend Fit_tuner_data($tuner_id,globals) _Hs_Fit_tuner_choose_p1_$tuner_id
    eval tk_optionMenu $topframe.par1 _Hs_Fit_tuner_choose_p1_$tuner_id $parnames
    $topframe.par1 configure -width $menuwidth
    label $topframe.lbl2 -text "vs"
    global _Hs_Fit_tuner_choose_p2_$tuner_id
    lappend Fit_tuner_data($tuner_id,globals) _Hs_Fit_tuner_choose_p2_$tuner_id
    eval tk_optionMenu $topframe.par2 _Hs_Fit_tuner_choose_p2_$tuner_id $parnames
    $topframe.par2 configure -width $menuwidth
    foreach [list _Hs_Fit_tuner_choose_p2_$tuner_id \
            _Hs_Fit_tuner_choose_p1_$tuner_id] $parnames break
    set cmd [list ::fit::Fit_tuner_contours_tracer $tuner_id $topframe.plot]
    trace variable _Hs_Fit_tuner_choose_p1_$tuner_id w $cmd
    trace variable _Hs_Fit_tuner_choose_p2_$tuner_id w $cmd
    pack $topframe.plot $topframe.lbl1 $topframe.par1 \
            $topframe.lbl2 $topframe.par2 -side left \
            -fill x -expand 1

    label $botframe.lbl3 -text "at"
    set bg [$botframe cget -background]
    set sframe $botframe.sigmas
    frame $sframe
    for {set i 1} {$i < 6} {incr i} {
        global _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id
        lappend Fit_tuner_data($tuner_id,globals) \
                _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id
        checkbutton $botframe.sigmas.s$i -text $i \
                -variable _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id \
                -command "global _Hs_Fit_tuner_choose_p1_$tuner_id;\
                set _Hs_Fit_tuner_choose_p1_$tuner_id\
                \[set _Hs_Fit_tuner_choose_p1_$tuner_id\]"
        set _Hs_Fit_tuner_choose_sigma_${i}_$tuner_id [expr {$i < 4}]
    }
    pack $sframe.s1 $sframe.s2 $sframe.s3 $sframe.s4 $sframe.s5 -side left
    label $botframe.lbl4 -text "sigma."
    set npoints 200
    ::hs::Int_entry_widget $botframe.ie "Points per contour:" red2 \
            5 999 $npoints ::hs::Int_entry_incr ::hs::Int_entry_decr \
            [list ::fit::Fit_tuner_set_contourpoints $tuner_id]
    fit::Fit_tuner_set_contourpoints $tuner_id $npoints
    pack $botframe.lbl3 $sframe $botframe.lbl4 $botframe.ie -side left \
            -fill x -expand 1

    # Fire the trace which determines the "Plot error contours" button state
    set _Hs_Fit_tuner_choose_p2_$tuner_id [set _Hs_Fit_tuner_choose_p2_$tuner_id]
    return $frame
}

############################################################################

proc ::fit::Fit_tuner_swap_lim_values {tuner_id b} {
    set onvalue [$b cget -onvalue]
    set offvalue [$b cget -offvalue]
    $b configure -onvalue $offvalue
    $b configure -offvalue $onvalue
    set which [string range [winfo name $b] 1 end]
    global _Hs_Fit_tuner_is_l${which}_$tuner_id
    set newval [set _Hs_Fit_tuner_is_l${which}_$tuner_id]
    set isminos [string equal -length 5 $newval "Minos"]
    set parmopt(lolim) eneg
    set parmopt(hilim) epos
    set parent [winfo parent $b]

    variable Fit_printout_precision
    fit::Eval_at_precision $Fit_printout_precision {
	variable Fit_tuner_data
        fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
            set iscomplete [fit::Fit_cget -complete]
            set pnum 0
            foreach parameter [fit::Fit_parameter list] {
                set entry $parent.${which}_$pnum
                set entryglobal _Fit_tuner_param_${which}_${tuner_id}_$pnum
                global $entryglobal
                if {$isminos} {
                    if {$iscomplete} {
                        set $entryglobal [fit::Fit_parameter \
                                $parameter cget -$parmopt($which)]
                    } else {
                        set $entryglobal ""
                    }
                    $entry configure -foreground black -state disabled
                } else {
                    set lolim ""
                    set hilim ""
                    foreach {lolim hilim} [fit::Fit_parameter \
                            $parameter cget -bounds] {}
                    set $entryglobal [set $which]
                    $entry configure -foreground black -state normal
                }
                incr pnum
            }
        }
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_parameter_frame {w tuner_id} {

    variable Fit_tuner_data

    # Some parameters
    set entrywidth 10

    # Assume that the active fit is set correctly
    frame $w
    set label_bg [$w cget -background]
    label $w.lvalues -text "Parameter starting value" -bg $label_bg
    label $w.lfixed -text " Fix " -bg $label_bg
    label $w.lstep -text "Step/Error" -bg $label_bg
    global _Hs_Fit_tuner_is_llolim_$tuner_id _Hs_Fit_tuner_is_lhilim_$tuner_id
    checkbutton $w.llolim -indicatoron 0 -width $entrywidth \
            -variable _Hs_Fit_tuner_is_llolim_$tuner_id\
            -textvariable _Hs_Fit_tuner_is_llolim_$tuner_id\
            -offvalue "Low limit" -onvalue "Minos E-"\
            -background $label_bg -selectcolor $label_bg -command \
            [list ::fit::Fit_tuner_swap_lim_values $tuner_id $w.llolim]
    set _Hs_Fit_tuner_is_llolim_$tuner_id "Low limit"
    checkbutton $w.lhilim -indicatoron 0 -width $entrywidth \
            -variable _Hs_Fit_tuner_is_lhilim_$tuner_id\
            -textvariable _Hs_Fit_tuner_is_lhilim_$tuner_id\
            -offvalue "High limit" -onvalue "Minos E+"\
            -background $label_bg -selectcolor $label_bg -command \
            [list ::fit::Fit_tuner_swap_lim_values $tuner_id $w.lhilim]
    set _Hs_Fit_tuner_is_lhilim_$tuner_id "High limit"
    lappend Fit_tuner_data($tuner_id,globals) \
            _Hs_Fit_tuner_is_llolim_$tuner_id _Hs_Fit_tuner_is_lhilim_$tuner_id
    grid $w.lvalues -row 0 -column 0 -sticky ew
    grid $w.lfixed -row 0 -column 1 -sticky ew
    grid $w.lstep -row 0 -column 2 -sticky ew
    grid $w.llolim -row 0 -column 3 -sticky ew
    grid $w.lhilim -row 0 -column 4 -sticky ew

    # Parameter scales
    set scale_specs {}
    set npars 0
    foreach parameter [fit::Fit_parameter info] {
        foreach {pname value state step bounds} $parameter break
        if {[llength $bounds] == 2} {
            foreach {lolim hilim} $bounds {}
        } else {
            if {$value == 0.0} {
                set lolim [expr {-100.0 * $step}]
                set hilim [expr {100.0 * $step}]
            } elseif {$value > 0.0} {
                set lolim 0.0
                set hilim [expr {10.0 * $value}]
            } else {
                set lolim [expr {10.0 * $value}]
                set hilim 0.0
            }
        }
        lappend scale_specs $pname $pname $lolim $hilim $value
        incr npars
    }

    ::hs::Scale_pack $w.scales 200 red2 $scale_specs [list \
            ::fit::Fit_tuner_adjust_parameter $tuner_id]
    grid $w.scales -row 1 -rowspan $npars -column 0 -sticky ew

    variable Fit_printout_precision
    fit::Eval_at_precision $Fit_printout_precision {
	set pnum 0
	foreach parameter [fit::Fit_parameter info] {
	    foreach {pname value state step bounds} $parameter break
	    set lolim ""
	    set hilim ""
	    foreach {lolim hilim} $bounds {}

	    # Checkbutton for "fixed" state
	    set bg [$w cget -background]
	    global _Fit_tuner_param_fixed_${tuner_id}_$pnum
	    lappend Fit_tuner_data($tuner_id,globals) \
		    _Fit_tuner_param_fixed_${tuner_id}_$pnum
	    checkbutton $w.fixed$pnum -command \
		    [list ::fit::Fit_tuner_fix_parameter $tuner_id $pname $pnum] \
		    -variable _Fit_tuner_param_fixed_${tuner_id}_$pnum \
		    -onvalue "fixed" -offvalue "variable"
	    set _Fit_tuner_param_fixed_${tuner_id}_$pnum $state
	    grid $w.fixed$pnum -row [expr {$pnum + 1}] -column 1 -sticky ew

	    # Entries for step size and limits
	    foreach {name column} {step 2 lolim 3 hilim 4} {
		global _Fit_tuner_param_${name}_${tuner_id}_$pnum
		lappend Fit_tuner_data($tuner_id,globals) \
			_Fit_tuner_param_${name}_${tuner_id}_$pnum
		entry $w.${name}_$pnum -width $entrywidth \
			-textvariable _Fit_tuner_param_${name}_${tuner_id}_$pnum
		set _Fit_tuner_param_${name}_${tuner_id}_$pnum [set $name]
		$w.${name}_$pnum configure -validate key -vcmd {
		    %W configure -foreground red2
		    return 1
		}
                # The -disabledforeground option appeared only in Tk 8.4
                catch {$w.${name}_$pnum configure -disabledforeground black}
		bind $w.${name}_$pnum <KeyPress-Return> \
			[list ::fit::Fit_tuner_set_param_aux \
			$tuner_id $w.${name}_$pnum $w.scales $name $pname $pnum]
		grid $w.${name}_$pnum -row [expr {$pnum + 1}] -column $column
	    }

	    incr pnum
	}
    }
    grid columnconfigure $w 0 -weight 1
    return $w
}

############################################################################

proc ::fit::Fit_tuner_fix_parameter {tuner_id parname pnum} {
    variable Fit_printout_precision
    fit::Eval_at_precision $Fit_printout_precision {
        variable Fit_tuner_data
        fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
            global _Fit_tuner_param_fixed_${tuner_id}_$pnum
            set newvalue [set _Fit_tuner_param_fixed_${tuner_id}_$pnum]
            fit::Fit_parameter $parname configure -state $newvalue
            set w .hs_fit_tuner_$tuner_id.pframe
            set step [fit::Fit_parameter $parname cget -step]
            set lolim ""
            set hilim ""
            foreach {lolim hilim} [fit::Fit_parameter $parname cget -bounds] {}
	    global _Hs_Fit_tuner_is_llolim_$tuner_id _Hs_Fit_tuner_is_lhilim_$tuner_id
	    if {[string equal [set _Hs_Fit_tuner_is_llolim_$tuner_id] "Minos E-"]} {
		set lolim ""
	    }
	    if {[string equal [set _Hs_Fit_tuner_is_lhilim_$tuner_id] "Minos E+"]} {
		set hilim ""
	    }
            foreach name {step lolim hilim} {
                global _Fit_tuner_param_${name}_${tuner_id}_$pnum
                set _Fit_tuner_param_${name}_${tuner_id}_$pnum [set $name] 
                if {[string equal $newvalue "fixed"]} {
                    $w.${name}_$pnum configure -foreground black -state disabled
                } else {
                    $w.${name}_$pnum configure -foreground black -state normal
                }
            }
            # Trigger the trace on the error contour menus
            global _Hs_Fit_tuner_choose_p1_$tuner_id
            set _Hs_Fit_tuner_choose_p1_$tuner_id \
                    [set _Hs_Fit_tuner_choose_p1_$tuner_id]
        }
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_set_param_aux {tuner_id \
        entryname scales which parname parnum} {
    variable Fit_tuner_data
    global _Fit_tuner_param_${which}_${tuner_id}_$parnum
    set newvalue [set _Fit_tuner_param_${which}_${tuner_id}_$parnum]

    # Validate the new value
    if {![string is double $newvalue]} return
    if {[string equal $which "step"]} {
        if {[string equal $newvalue ""]} return
        if {$newvalue <= 0.0} return
        $entryname configure -foreground black
    } else {
        # This must be one of the limits
        set $which $newvalue
        if {[string equal $which "lolim"]} {
            set othername "hilim"
        } elseif {[string equal $which "hilim"]} {
            set othername "lolim"
        }
        global _Fit_tuner_param_${othername}_${tuner_id}_$parnum
        set $othername [set _Fit_tuner_param_${othername}_${tuner_id}_$parnum]
        if {![string is double [set $othername]]} return
        if {[string equal $lolim ""] && ![string equal $hilim ""]} return
        if {![string equal $lolim ""] && [string equal $hilim ""]} return
        if {![string equal $lolim ""]} {
            if {$hilim <= $lolim} return
        }
        $entryname configure -foreground black
        regsub $which $entryname $othername otherentry
        $otherentry configure -foreground black
        set which "bounds"
        set newvalue [concat $lolim $hilim]

        # Update the scales to reflect new limits
        if {[llength $newvalue] > 0} {
            set pvalue [fit::Fit_eval $Fit_tuner_data($tuner_id,name) \
                    {fit::Fit_parameter $parname cget -value}]
            if {$pvalue < $lolim} {
                set pvalue $lolim
            }
            if {$pvalue > $hilim} {
                set pvalue $hilim
            }
            ::hs::Scale_pack_set $scales $parname $pvalue $lolim $hilim
        }
    }

    # Change the parameter accordingly
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        fit::Fit_parameter $parname configure -$which $newvalue
    }
    return
}

############################################################################

proc ::fit::Fit_tuner_adjust_parameter_dummy {args} {
    return
}

############################################################################

proc ::fit::Fit_tuner_adjust_parameter {tuner_id pname newvalue} {
    variable Fit_tuner_data
    fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        fit::Fit_parameter $pname configure -value $newvalue
    }
    eval $Fit_tuner_data($tuner_id,update_script)
    ::hs::hs_update
    return
}

############################################################################

proc ::fit::Fit_tuner_destroy {tuner_id topwin args} {
    # Check that this toplevel is not locked
    variable Fit_tuner_data
    if {$Fit_tuner_data($tuner_id,locked)} return
    foreach w $Fit_tuner_data($tuner_id,winlist) {
        hs::close_window $w 1
    }
    foreach plot $Fit_tuner_data($tuner_id,multiplots) {
        hs::multiplot $plot clear
    }
    foreach plot $Fit_tuner_data($tuner_id,overlays) {
        hs::overlay $plot clear
    }
    ::fit::Fit_eval $Fit_tuner_data($tuner_id,name) {
        eval [join $Fit_tuner_data($tuner_id,cleanup_script) "\n"]
    }
    destroy $topwin
    # Must unset global variables only after the window is destroyed
    eval global $Fit_tuner_data($tuner_id,globals)
    eval unset [lsort -unique $Fit_tuner_data($tuner_id,globals)]
    array unset Fit_tuner_data $tuner_id,*
    ::hs::Scale_pack_destroy $topwin.pframe.scales
    return
}

############################################################################

proc ::fit::Eval_at_precision {n script} {
    global tcl_precision ::errorInfo
    set old_precision $tcl_precision
    set tcl_precision $n
    if {[catch {uplevel $script} result]} {
        set savedInfo $::errorInfo
        set tcl_precision $old_precision
        error $result $savedInfo
    }
    set tcl_precision $old_precision
    set result
}

############################################################################

proc ::fit::Fit_eval {fitname script} {
    # Evaluates given script using $fitname as the active fit
    set old_fit_name [::fit::Fit_get_active]
    ::fit::Fit_activate $fitname
    global ::errorInfo
    if {[catch {uplevel $script} local_result]} {
        set savedInfo $::errorInfo
        catch {::fit::Fit_activate $old_fit_name}
        error $local_result $savedInfo
    }
    # Use "catch" because it is possible that old fit no longer exists
    catch {::fit::Fit_activate $old_fit_name}
    set local_result
}

############################################################################

proc ::hs::config_histoscope {args} {
    if {[expr [llength $args] % 2]} {
	error "wrong # of arguments"
    }
    array set headers {
	"updatePeriod"       "UpdateFrequency"    
	"minLineWidth"       "MinPSLineWidth"     
	"bufferGraphics"     "BufferGraphics"     
	"scatThicken"        "ThickenPointsScat"  
	"scat3dThicken"      "ThickenPointsScat3D"
        "autoHelp"           "AutomaticHelp"
	"commentFont"        "Font"
	"printTitles"        "Title"
        "printCoords"        "PrintCoords"
        "errorMarkerRatio"   "ErrorMarkerRatio"
        "enableLeftArrows"   "DisplayLeftArrow"
        "enableRightArrows"  "DisplayRightArrow"
        "enableTopArrows"    "DisplayTopArrow"
        "enableBottomArrows" "DisplayBottomArrow"
    }
    set change_default_color_scale 0
    set commentFont ""
    set titleFont ""
    set mplotTitleFont ""
    set config_values {}
    foreach {option value} $args {
	regsub "^-" $option "" option
	set error_flag 0
	set add_pair 1
	switch -- $option {
	    updatePeriod {
		if {![string is integer -strict $value]} {
		    set error_flag 1
		    break
		}
		if {$value <= 0} {
		    set error_flag 1
		    break
		}
	    }
	    minLineWidth {
		if {![string is double -strict $value]} {
		    set error_flag 1
		    break
		}
		if {$value < 0.0} {
		    set error_flag 1
		    break
		}
	    }
	    mplotTitleFont -
	    titleFont {
		set $option $value
		set add_pair 0
	    }
	    defaultColorScale {
		variable Color_scale_data
		if {![info exists Color_scale_data($value,ncolors)]} {
		    error "Bad color scale tag \"$value\""
		}
		set add_pair 0
		set change_default_color_scale 1
		set new_default_color_scale $value
	    }
            autoHelp -
	    bufferGraphics -
	    scatThicken -
	    scat3dThicken -
            printCoords -
            enableLeftArrows -
            enableRightArrows -
            enableTopArrows -
            enableBottomArrows -
	    printTitles {
		if {![string is boolean -strict $value]} {
		    set error_flag 1
		    break
		}
		if {$value} {
		    set value 1
		} else {
		    set value 0
		}
	    }
	    commentFont {
		set commentFont $value
		set value [hs::Generate_xlfd $value]
	    }
            errorMarkerRatio {
		if {![string is double -strict $value]} {
		    set error_flag 1
		    break
		}
		if {$value <= 0.0} {
		    set error_flag 1
		    break
		}
            }
	    default {
		error "Unknown option \"$option\""
	    }
	}
	if {$error_flag} {
	    error "Bad option $option value \"$value\""
	}
	if {$add_pair} {
	    lappend config_values $option $value
	}
    }
    set config_string ""
    if {$change_default_color_scale} {
	hs::Append_color_scale_def config_string \
		$new_default_color_scale "default"
    }
    if {[llength $config_values] > 0} {
	append config_string "ConfigData\n"
	foreach {option value} $config_values {
	    append config_string " " $headers($option) " " $value "\n"
	}
    }
    if {![string equal $commentFont ""]} {
	foreach {psfont psfontsize} [hs::Postscript_font $commentFont] {}
	append config_string " PSFont $psfont\n"
	append config_string " PSFontSize $psfontsize\n"
    }
    # Generate a separate config command for the title font
    if {![string equal $titleFont ""]} {
	foreach {psfont psfontsize} [hs::Postscript_font $titleFont] {}
	append config_string "ConfigData\n\
		Title WinTitleFont\n\
		PSFont $psfont\n\
		PSFontSize $psfontsize\n"
    }
    # Generate a separate config command for the miniplot title font
    if {![string equal $mplotTitleFont ""]} {
	foreach {psfont psfontsize} [hs::Postscript_font $mplotTitleFont] {}
	append config_string "ConfigData\n\
		Title MplotTitleFont\n\
		PSFont $psfont\n\
		PSFontSize $psfontsize\n"
    }
    if {![string equal $config_string ""]} {
	hs::load_config_string $config_string
	hs::hs_update
    }
    return
}

############################################################################

proc ::hs::Histo_2d_heading {plotmode} {
    switch -- $plotmode {
	lego {
	    set heading "2DHistogram"
	}
	cell {
	    set heading "CellPlot"
	}
	tartan {
	    set heading "ColorCellPlot"
	}
	default {
	    # Invalid plotting mode
	    error "invalid plotting mode \"$plotmode\",\
		    should be \"lego\", \"cell\", or \"tartan\""
	}
    }
    return $heading
}

############################################################################
# The following procedure is called when Histo-Scope completes some task.
# It is always called at the global level. Status value 0 means OK.
proc ::hs::Task_completion_callback {connect_id task_number status result} {
    variable Histo_task_data
    if {[info exists Histo_task_data($task_number,cmd)]} {
	if {[string compare $Histo_task_data($task_number,callback) ""]} {
	    uplevel \#0 $Histo_task_data($task_number,callback) $status $result
	} elseif {$status} {
	    puts stderr "WARNING! Histo-Scope failed to execute\
		    command\n$Histo_task_data($task_number,cmd): $result"
	}
	unset Histo_task_data($task_number,cmd)
	unset Histo_task_data($task_number,callback)
    } elseif {$status} {
	puts stderr "WARNING! Histo-Scope task $task_number failed: $result"
    }
    return
}

############################################################################

proc ::hs::Histoscope_command {config_string wait_completion {callback ""}} {

    set cycle_delay 1
    set maxcycles 50000

    if {[hs::num_connected_scopes] == 0} {
	return
    }
    if {!$wait_completion && [string equal $callback ""]} {
	hs::load_config_string $config_string
	return
    }
    variable Histo_task_counter
    set task_number [incr Histo_task_counter]
    append config_string " TaskNumber $task_number\n"
    variable Histo_task_data
    set Histo_task_data($task_number,cmd) $config_string
    set Histo_task_data($task_number,callback) $callback
    hs::load_config_string $config_string
    if {$wait_completion} {
	for {set i 0} {$i < $maxcycles} {incr i} {
	    hs::hs_update
	    if {![info exists Histo_task_data($task_number,cmd)]} {
		break
	    }
	    after $cycle_delay
	}
	if {$i == $maxcycles} {
	    error "Histo-Scope task timed out. Config string is $config_string."
	}
    }
    return
}

############################################################################

proc ::hs::Windows_exist {winnames} {

    set cycle_delay 1
    set maxcycles 50000

    if {[llength $winnames] == 0} {
	return
    }
    set result {}
    set nscopes [hs::num_connected_scopes]
    if {$nscopes == 0} {
	foreach win $winnames {
	    lappend result 0
	}
    } elseif {$nscopes == 1} {
	set config_string ""
	variable Histo_task_counter
	variable Histo_task_data
	foreach win $winnames {
	    set task_number [incr Histo_task_counter]
	    set action "WindowAction\n\
		    WindowName ${win}\n Action Exists\n TaskNumber $task_number\n"
	    append config_string $action
	    set Histo_task_data($task_number,cmd) $action
	    set Histo_task_data($task_number,callback) [list \
		    ::hs::Windows_exist_callback $win]
	}
	variable Histo_windows_status
	array unset Histo_windows_status
	hs::load_config_string $config_string
	for {set i 0} {$i < $maxcycles} {incr i} {
	    hs::hs_update
	    # Check the status of the last window. If found
	    # then it also exists for all other windows.
	    if {[info exists Histo_windows_status($win)]} {
		break
	    }
	    after $cycle_delay
	}
	if {$i == $maxcycles} {
	    error "Histo-Scope task timed out. Config string is $config_string."
	}
	foreach win $winnames {
	    lappend result $Histo_windows_status($win)
	}
    } else {
	error "Ambiguous: more than one Histo-Scope connected"
    }
    return $result
}

############################################################################

proc ::hs::Windows_exist_callback {winname status result} {
    if {$status} {
	error "Error receiving window $winname status from Histo-Scope: $result"
    }
    variable Histo_windows_status
    set Histo_windows_status($winname) $result
    return
}

############################################################################
# The following function returns a two-element list {PsFontName PsFontSize}

proc ::hs::Postscript_font {tkfont} {
    variable Postscript_font_cached_fontmap
    if {[info exists Postscript_font_cached_fontmap($tkfont)]} {
	return $Postscript_font_cached_fontmap($tkfont)
    }

    # Require Tk
    if {[catch {package require Tk}]} {
        error "Sorry, [lindex [info level 0] 0] needs Tk"
    }
    foreach {option value} [font actual $tkfont] {
	regsub "^-" $option "" option
	set $option $value
    }
    # The following code is more or less direct
    # translation of the Tk_PostscriptFontName
    # Tk C API function into tcl.
    set family_mapping {
	Arial             Helvetica
	Geneva            Helvetica
	{Times New Roman} Times
	{New York}        Times
	{Courier New}     Courier
        Monaco            Courier
        AvantGarde        AvantGarde
        ZapfChancery      ZapfChancery
        ZapfDingbats      ZapfDingbats
    }
    if {[string equal -nocase -length 4 $family "itc "]} {
	set family [string range $family 4 end]
    }
    set family_mapped 0
    foreach {orig_family target_family} $family_mapping {
	if {[string equal -nocase $orig_family $family]} {
	    set family $target_family
	    set family_mapped 1
	    break
	}
    }
    if {!$family_mapped} {
	# Capitalize the first letter of each word,
	# lowercase the rest of the letters in each word,
	# and then take out the spaces between the words.
	set wordlist [split $family]
	set family ""
	foreach word $wordlist {
	    append family [string totitle $word]
	}
    }
    if {[string equal $family "NewCenturySchoolbook"]} {
	set family "NewCenturySchlbk"
    }

    # Get the string to use for the weight
    if {[string equal $weight "normal"]} {
	switch -- $family {
	    Bookman {
		set weightString "Light"
	    }
	    AvantGarde {
		set weightString "Book"
	    }
	    ZapfChancery {
		set weightString "Medium"
	    }
	    default {
		set weightString ""
	    }
	}
    } elseif {[lsearch -exact {Bookman AvantGarde} $family] >= 0} {
	set weightString "Demi"
    } else {
	set weightString "Bold"
    }

    # Get the string to use for the slant
    if {[string equal $slant "roman"]} {
	set slantString ""
    } elseif {[lsearch -exact {Helvetica Courier AvantGarde} $family] >= 0} {
	set slantString "Oblique"
    } else {
	set slantString "Italic"
    }

    # The string "Roman" needs to be added to some fonts
    # that are not bold and not italic.
    set familyOne $family
    if {[string equal $slantString ""] && [string equal $weightString ""]} {
	if {[lsearch -exact {Times NewCenturySchlbk Palatino} $family] >= 0} {
	    append family "-Roman"
	}
    } else {
	append family "-"
	append family $weightString
	append family $slantString
    }

    # We will use ISOLatin1 encoding for certain types of fonts.
    # See psUtils.c file in the Histo-Scope distribution to
    # find out which fonts may be used in the ISOLatin1 encoding.
    if {[lsearch -exact {Courier Helvetica\
	    Times NewCenturySchlbk} $familyOne] >= 0} {
	append family "-ISOLatin1"
    }

    set Postscript_font_cached_fontmap($tkfont) [list $family $size]
}

############################################################################

proc ::hs::comment {args} {
    # Arguments should be: text position
    # However, we should expect to see the switches
    # interspersed anywhere between the arguments.

    # Known switches
    set known_switches {-fg -foreground -bg -background \
	    -row -column -refpoint -coord -coords -justify \
	    -origin -border -anchor -font -window -immediate}

    # Default colors
    set foreground "black"
    set background "" ;# transparent

    # Default coordinate system and origin
    set default_coords "winrel"
    set default_origin "O"

    # Parse the input arguments
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[llength $arglist] != 2} {
	error "wrong # of arguments or invalid option"
    }
    foreach {comment_text position} $arglist {}
    if {[string length $comment_text] > 4075} {
	error "Comment text is too long"
    }

    # Strip leading newlines. Motif will not display
    # them, but PostScript will -- so, we have a problem.
    # Ideally, this should be fixed inside the Histo-Scope
    # utility function PSDrawXmString
    regsub "^\n+" $comment_text "" comment_text

    # Remap remaining newlines for transmission to Histo-Scope
    set comment_text [string map {"\n" "\\n" "\\" "\\\\"} $comment_text]

    # Figure out the window name
    if {[info exists options(-window)]} {
	set window $options(-window)
    } else {
	variable Last_window_posted
	if {[string equal $Last_window_posted ""]} {
	    error "Can't figure out window name.\
		    Please use the \"-window\" option."
	} else {
	    set window $Last_window_posted
	}
    }
    variable Known_window_names
    if {![info exists Known_window_names($window)]} {
	error "Invalid window name \"$window\""
    }

    # Check if we should suppress the window refresh
    if {[info exists options(-immediate)]} {
	if {$options(-immediate)} {
	    set suppress_redraw 0
	} else {
	    set suppress_redraw 1
	}
    } else {
	set suppress_redraw 0
    }

    # Check coordinate system and origin
    if {[info exists options(-coord)]} {
	set default_coords [hs::Subst_coords_type $options(-coord)]
    }
    if {[info exists options(-coords)]} {
	set default_coords [hs::Subst_coords_type $options(-coords)]
    }
    if {[info exists options(-origin)]} {
	set default_origin [hs::Subst_origin_type $options(-origin)]
    }

    # Check reference point
    if {[info exists options(-refpoint)]} {
	set refpoint [hs::Parse_drawing_point $options(-refpoint) \
		$default_coords $default_origin]
    } else {
	set refpoint "" ;# Will use Histo-Scope default
    }

    # Check the position argument
    set position [hs::Parse_drawing_point $position \
	    $default_coords $default_origin]

    # Check colors
    set check_colors {}
    foreach colortype {background foreground} \
	    switchlist {{-bg -background} {-fg -foreground}} {
	foreach sw $switchlist {
	    if {[info exists options($sw)]} {
		set newcolor $options($sw)
		if {![string equal $newcolor "black"] && \
			![string equal $newcolor ""]} {
		    lappend check_colors $newcolor
		}
		set $colortype $newcolor
	    }
	}
    }
    if {[llength $check_colors] > 0} {
	hs::Verify_color_names $check_colors
    }

    # Check row and column
    if {[info exists options(-row)]} {
	set row $options(-row)
	if {![string is integer -strict $row]} {
	    error "Bad multiplot row \"$row\""
	} elseif {$row < 0} {
	    error "Bad multiplot row \"$row\""
	}
    } else {
	set row -1
    }
    if {[info exists options(-column)]} {
	set column $options(-column)
	if {![string is integer -strict $column]} {
	    error "Bad multiplot column \"$column\""
	} elseif {$column < 0} {
	    error "Bad multiplot column \"$column\""
	}
    } else {
	set column -1
    }
    if {$row < 0 && $column >= 0} {
	error "Multiplot row not specified"
    }
    if {$row >= 0 && $column < 0} {
	error "Multiplot column not specified"
    }

    # Check the border
    if {[info exists options(-border)]} {
	set border $options(-border)
	if {![string is boolean -strict $border]} {
	    error "expected a boolean argument for\
		    the -border option, got \"$border\""
	}
    } else {
	set border 0
    }

    # Check the anchor
    if {[info exists options(-anchor)]} {
	set anchor [hs::Verify_comment_anchor $options(-anchor)]
    } else {
	set anchor ""
    }

    # Check text justification
    set justify -1
    if {[info exists options(-justify)]} {
	set justify $options(-justify)
	if {[string equal -nocase $justify "left"] || \
		[string equal -nocase $justify "l"]} {
	    set justify 0
	} elseif {[string equal -nocase $justify "right"] || \
		[string equal -nocase $justify "r"]} {
	    set justify 2
	} elseif {[string equal -nocase $justify "center"] || \
		[string equal -nocase $justify "c"] || \
		[string equal -nocase $justify "centre"]} {
	    set justify 1
	} else {
	    error "invalid value for the -justify option,\
		    expected \"left\", \"center\", or \"right\""
	}
    }

    # Check the font
    if {[info exists options(-font)]} {
	set font [hs::Generate_xlfd $options(-font)]
	foreach {psfont psfontsize} [hs::Postscript_font $options(-font)] {}
    } else {
	set font ""
	set psfont ""
	set psfontsize 0
    }

    # Generate the configuration string
    set config_string "WindowAction\n\
	    WindowName $window\n\
	    Action Comment $comment_text\n\
	    Point1 $position\n"
    if {$row >= 0} {
	append config_string "\
		Row [expr {$row + 1}]\n"
    }
    if {$column >= 0} {
	append config_string "\
		Column [expr {$column + 1}]\n"
    }
    if {$suppress_redraw} {
	append config_string "\
		NoRedraw\n"
    }
    if {![string equal $background ""]} {
	append config_string "\
		Background $background\n"
    }
    if {![string equal $foreground ""]} {
	append config_string "\
		Foreground $foreground\n"
    }
    if {![string equal $refpoint ""]} {
	append config_string "\
		ReferencePoint $refpoint\n"
    }
    if {$border} {
	append config_string "\
		DrawBorder\n"
    }
    if {![string equal $anchor ""]} {
	append config_string "\
		Alignment $anchor\n"
    }
    if {$justify >= 0} {
	append config_string "\
		Justify $justify\n"
    }
    if {![string equal $font ""]} {
	append config_string "\
		Font $font\n\
		PSFont $psfont\n\
		PSFontSize $psfontsize\n"
    }

    hs::Push_history any [list $window $row $column comment]
    hs::load_config_string $config_string
    if {!$suppress_redraw} {
	hs::hs_update
    }
    return
}

############################################################################

proc ::hs::Verify_comment_anchor {anchor} {
    array set valid_anchors {
	nw     "NW"
	n      "NC"
	ne     "NE"
	w      "CW"
	c      "CC"
        center "CC"
	e      "CE"
	sw     "SW"
	s      "SC"
	se     "SE"
    }
    if {![info exists valid_anchors($anchor)]} {
	error "Invalid anchor specification \"$anchor\". Valid anchors are\
		[join [lsort [array names valid_anchors]] {, }]."
    }
    set valid_anchors($anchor)
}

############################################################################

proc ::hs::Simple_window_command {window row column wincommand {nocomplain 0}} {
    variable Known_window_names
    if {![info exists Known_window_names($window)] && !$nocomplain} {
	error "Invalid window name \"$window\""
    }
    # Convert nocomplain from bool into int
    if {$nocomplain} {
	set nc_i 1
    } else {
	set nc_i 0
    }
    set config_string "WindowAction\n\
	    WindowName $window\n\
	    Action $wincommand\n\
	    NoComplain $nc_i\n"
    if {$row >= 0} {
	append config_string "\
		Row [expr {$row + 1}]\n"
    }
    if {$column >= 0} {
	append config_string "\
		Column [expr {$column + 1}]\n"
    }
    hs::load_config_string $config_string
    hs::hs_update
    return
}

############################################################################

proc ::hs::undo {} {
    set last_win [hs::Pop_history any]
    if {[llength $last_win] == 0} return
    foreach {window row column objtype} $last_win {}
    variable Known_window_names
    if {[info exists Known_window_names($window)]} {
	hs::Simple_window_command $window $row $column "Delete" 1
    }
    return
}

############################################################################

proc ::hs::redraw {args} {
    eval hs::Redraw_or_Clear Redraw $args
}

############################################################################

proc ::hs::clear {args} {
    eval hs::Redraw_or_Clear Clear $args
}

############################################################################

proc ::hs::Redraw_or_Clear {winaction args} {
    # Parse the input arguments
    set known_switches {-row -column -window}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[llength $arglist] != 0} {
	error "wrong # of arguments or invalid option"
    }

    # Figure out the window name
    if {[info exists options(-window)]} {
	set window $options(-window)
    } else {
	variable Last_window_posted
	if {[string equal $Last_window_posted ""]} {
	    error "Can't figure out window name.\
		    Please use the \"-window\" option."
	} else {
	    set window $Last_window_posted
	}
    }
    variable Known_window_names
    if {![info exists Known_window_names($window)]} {
	error "Invalid window name \"$window\""
    }

    # Figure out row and column
    if {[info exists options(-row)]} {
	set row $options(-row)
	if {![string is integer -strict $row]} {
	    error "Bad multiplot row \"$row\""
	} elseif {$row < 0} {
	    error "Bad multiplot row \"$row\""
	}
    } else {
	set row -1
    }
    if {[info exists options(-column)]} {
	set column $options(-column)
	if {![string is integer -strict $column]} {
	    error "Bad multiplot column \"$column\""
	} elseif {$column < 0} {
	    error "Bad multiplot column \"$column\""
	}
    } else {
	set column -1
    }
    if {$row < 0 && $column >= 0} {
	error "Multiplot row not specified"
    }
    if {$row >= 0 && $column < 0} {
	error "Multiplot column not specified"
    }

    # Generate the configuration string
    set config_string "WindowAction\n\
	    WindowName ${window}\n Action $winaction\n"
    if {$row >= 0} {
	append config_string "\
		Row [expr {$row + 1}]\n"
    }
    if {$column >= 0} {
	append config_string "\
		Column [expr {$column + 1}]\n"
    }
    hs::load_config_string $config_string
    hs::hs_update
    return
}

############################################################################

proc ::hs::draw {args} {
    # Arguments should be: item_type point1 point2 ...
    # However, we should expect to see the switches
    # interspersed anywhere between the arguments.

    # Known switches
    set known_switches {-fg -foreground -bg -background -row -column \
	    -refpoint -coord -coords -origin -arrow -line -window -immediate}

    # Default colors. Empty string means not specified.
    set background ""
    set foreground "black"

    # Default coordinate system and origin
    set default_coords "winrel"
    set default_origin "O"

    # Item attributes: type and its aliases, Histo-Scope category,
    # minimum number of points, maximum number of points.
    set item_attributes {
	{points point}   "point"     1 32
	line             "line"      1 32
	{rectangle rect} "rectangle" 2 2
	ellipse          "ellipse"   2 3
	polygon          "polygon"   3 32
    }

    # Parse the input arguments
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[llength $arglist] < 2} {
	error "wrong # of arguments or invalid option"
    }

    # Figure out the window name
    if {[info exists options(-window)]} {
	set window $options(-window)
    } else {
	variable Last_window_posted
	if {[string equal $Last_window_posted ""]} {
	    error "Can't figure out window name.\
		    Please use the \"-window\" option."
	} else {
	    set window $Last_window_posted
	}
    }
    variable Known_window_names
    if {![info exists Known_window_names($window)]} {
	error "Invalid window name \"$window\""
    }

    # Check if we should suppress the window refresh
    if {[info exists options(-immediate)]} {
	if {$options(-immediate)} {
	    set suppress_redraw 0
	} else {
	    set suppress_redraw 1
	}
    } else {
	set suppress_redraw 0
    }

    # Check item type
    set item_type [lindex $arglist 0]
    set type_found 0
    set valid_types {}
    foreach {typelist category minpoints maxpoints} $item_attributes {
	lappend valid_types [lindex $typelist 0]
	foreach type $typelist {
	    if {[string equal $type $item_type]} {
		set type_found 1
		break
	    }
	}
	if {$type_found} break
    }
    if {!$type_found} {
	error "Invalid object type \"$item_type\".\
		Valid types are: [join $valid_types {, }]."
    }

    # Figure out global coordinate system and origin
    if {[info exists options(-coord)]} {
	set default_coords [hs::Subst_coords_type $options(-coord)]
    }
    if {[info exists options(-coords)]} {
	set default_coords [hs::Subst_coords_type $options(-coords)]
    }
    if {[info exists options(-origin)]} {
	set default_origin [hs::Subst_origin_type $options(-origin)]
    }

    # Go over the list of points
    set pointlist {}
    foreach point [lrange $arglist 1 end] {
	lappend pointlist [hs::Parse_drawing_point $point \
		$default_coords $default_origin]
    }
    if {[llength $pointlist] < $minpoints} {
	error "Insufficient number of coordinate points"
    }
    if {[llength $pointlist] > $maxpoints} {
	error "Too many coordinate points"
    }
    foreach {x system_x origin_x y system_y origin_y} [lindex $pointlist 0] {}
    if {[string equal $origin_x "L"] || [string equal $origin_y "L"]} {
	error "The first point in the sequence can not\
		be specified relative to a previous point"
    }

    # Check line style
    set linestyle -1
    if {[string equal $category "line"]} {
	if ([info exists options(-line)]) {
	    set linestyle $options(-line)
	    if {![string is integer -strict $linestyle]} {
		error "Expected an integer value for\
			the -line option, got \"$linestyle\""
	    }
	    if {$linestyle < 0 || $linestyle > 12} {
		error "Invalid line style $linestyle.\
			Valid line styles are 0 through 12."
	    }
	}
    }

    # Check colors
    set check_colors {}
    foreach colortype {background foreground} \
	    switchlist {{-bg -background} {-fg -foreground}} {
	foreach sw $switchlist {
	    if {[info exists options($sw)]} {
		set newcolor $options($sw)
		if {![string equal $newcolor "black"] && \
			![string equal $newcolor ""]} {
		    lappend check_colors $newcolor
		}
		set $colortype $newcolor
	    }
	}
    }
    if {[llength $check_colors] > 0} {
	hs::Verify_color_names $check_colors
    }

    # Check remaining options (-row, -column, -arrow, and -refpoint)
    if {[info exists options(-row)]} {
	set row $options(-row)
	if {![string is integer -strict $row]} {
	    error "Bad multiplot row \"$row\""
	} elseif {$row < 0} {
	    error "Bad multiplot row \"$row\""
	}
    } else {
	set row -1
    }

    if {[info exists options(-column)]} {
	set column $options(-column)
	if {![string is integer -strict $column]} {
	    error "Bad multiplot column \"$column\""
	} elseif {$column < 0} {
	    error "Bad multiplot column \"$column\""
	}
    } else {
	set column -1
    }

    if {$row < 0 && $column >= 0} {
	error "Multiplot row not specified"
    }
    if {$row >= 0 && $column < 0} {
	error "Multiplot column not specified"
    }

    if {[info exists options(-arrow)]} {
	switch -- $options(-arrow) {
	    none {
		set arrowMode 0
	    }
	    forward {
		set arrowMode 1
	    }
	    backward -
	    back {
		set arrowMode 2
	    }
	    both {
		set arrowMode 3
	    }
	    default {
		error "Invalid arrow mode \"$options(-arrow)\".\
			Valid arrow modes are \"forward\", \"backward\",\
			and \"both\"."
	    }
	}
    } else {
	set arrowMode 0
    }

    if {[info exists options(-refpoint)]} {
	set refpoint [hs::Parse_drawing_point $options(-refpoint) \
		$default_coords $default_origin]
    } else {
	set refpoint "" ;# Will use Histo-Scope default
    }

    # Generate the configuration string
    set config_string "WindowAction\n\
	    WindowName $window\n\
	    Action Draw\n\
	    Category $category\n"
    if {$row >= 0} {
	append config_string "\
		Row [expr {$row + 1}]\n"
    }
    if {$column >= 0} {
	append config_string "\
		Column [expr {$column + 1}]\n"
    }
    if {$suppress_redraw} {
	append config_string "\
		NoRedraw\n"
    }
    if {![string equal $background ""]} {
	append config_string "\
		Background $background\n"
    }
    if {![string equal $foreground ""]} {
	append config_string "\
		Foreground $foreground\n"
    }
    if {![string equal $refpoint ""]} {
	append config_string "\
		ReferencePoint $refpoint\n"
    }
    if {$arrowMode} {
	append config_string "\
		ArrowMode $arrowMode\n"
    }
    if {$linestyle >= 0} {
	append config_string "\
		LineStyle1 $linestyle\n"
    }
    set i 1
    foreach point $pointlist {
	append config_string "\
		Point$i $point\n"
	incr i
    }

    hs::Push_history any [list $window $row $column draw]
    hs::load_config_string $config_string
    if {!$suppress_redraw} {
	hs::hs_update
    }
    return
}

############################################################################

proc ::hs::Push_history {which data} {
    variable History_buffer_size
    variable History_buffer_pointer
    variable History_buffer_data
    if {![info exists History_buffer_pointer($which)]} {
	set History_buffer_pointer($which) 0
	for {set i 1} {$i < $History_buffer_size} {incr i} {
	    set History_buffer_data($which,$i) {}
	}
    } else {
	incr History_buffer_pointer($which)
	if {$History_buffer_pointer($which) == $History_buffer_size} {
	    set History_buffer_pointer($which) 0
	}
    }
    set History_buffer_data($which,$History_buffer_pointer($which)) $data
    return
}

############################################################################

proc ::hs::Pop_history {which} {
    variable History_buffer_size
    variable History_buffer_pointer
    variable History_buffer_data
    if {![info exists History_buffer_pointer($which)]} {
	set History_buffer_pointer($which) 0
	for {set i 0} {$i < $History_buffer_size} {incr i} {
	    set History_buffer_data($which,$i) {}
	}
    }
    set datum $History_buffer_data($which,$History_buffer_pointer($which))
    set History_buffer_data($which,$History_buffer_pointer($which)) {}
    incr History_buffer_pointer($which) -1
    if {$History_buffer_pointer($which) < 0} {
	set History_buffer_pointer($which) [expr {$History_buffer_size - 1}]
    }
    set datum
}

############################################################################

proc ::hs::Parse_drawing_point {value default_coords default_origin} {
    set len [llength $value]
    if {$len == 2} {
	foreach c $value axis {x y} {
	    if {![string is double -strict $c]} {
		error "Invalid $axis coordinate in point \"$value\""
	    }
	}
	foreach {x y} $value {}
	return [list $x $default_coords $default_origin \
		$y $default_coords $default_origin]
    } elseif {$len == 6} {
	set point {}
	foreach {c coords origin} $value axis {x y} {
	    if {![string is double -strict $c]} {
		error "Invalid $axis coordinate in point \"$value\""
	    }
	    lappend point $c
	    lappend point [hs::Subst_coords_type $coords]
	    lappend point [hs::Subst_origin_type $origin]
	}
	return $point
    } else {
	error "Invalid point specification \"$value\""
    }
}

############################################################################

proc ::hs::Subst_coords_type {coords} {
    if {[string equal $coords "winabs"] ||\
	    [string equal $coords "winrel"] ||\
	    [string equal $coords "plot"]} {
	return $coords
    } else {
	error "Invalid coordinate system type \"$coords\".\
		Valid systems are \"winabs\", \"winrel\", and \"plot\"."
    }
}

############################################################################

proc ::hs::Subst_origin_type {origin} {
    switch -- $origin {
	o -
	O {
	    set orig O
	}
	l -
	L {
	    set orig L
	}
	r -
	R {
	    set orig R
	}
	default {
	    error "Invalid coordinate origin type \"$origin\".\
		    Valid origin designators are O, L, and R."
	}
    }
    set orig
}

############################################################################

proc ::hs::Parse_single_arg_switches {sequence keys arrname} {
    upvar $arrname options_local
    array unset options_local
    set result {}
    set this_is_value 0
    foreach word $sequence {
	if {$this_is_value} {
	    set options_local($key) $word
	    set this_is_value 0
	} elseif {[lsearch -exact $keys $word] >= 0} {
	    # Found a keyword
	    set key $word
	    set this_is_value 1
	} else {
	    lappend result $word
	}
    }
    if {$this_is_value} {
	error "Missing value for the \"$key\" option"
    }
    set result
}

############################################################################

proc ::hs::Check_window_name {winname} {
    if {[string equal $winname ""]} {
	error "Window name can not be an empty string"
    }
    if {[hs::Use_distinct_window_names]} {
	variable Known_window_names
	if {[info exists Known_window_names($winname)]} {
	    error "Duplicate window name \"$winname\""
	}
    }
    return
}

############################################################################

proc ::hs::Remember_window_name {winname} {
    if {![string equal $winname ""]} {
	variable Known_window_names
	set Known_window_names($winname) 1
	variable Last_window_posted
	set Last_window_posted $winname
    }
    return
}

#########################################################################

proc ::hs::Is_simple_c_postfix {name} {
    # Expect that a "simple" C postfix expression should look
    # like this:
    #    simple-postfix:
    #        identifier
    #        simple-postfix[non-negative-integer]
    #        simple-postfix.identifier
    #
    # We should be able to use such expressions as ntuple
    # variable names. Unlike real C, we will not allow for
    # spaces anywhere in the expressions so that ntuple
    # packing/unpacking code may be created relatively easily
    # from tcl.
    if {[hs::Is_valid_c_identifier $name]} {
	return 1
    }
    set last_index [string length $name]
    if {$last_index == 0} {
	return 0
    }
    incr last_index -1
    if {[string equal [string index $name $last_index] "\]"]} {
	set index [string last "\[" $name]
	if {$index <= 0} {
	    return 0
	}
	set i [string range $name [expr {$index+1}] [expr {$last_index-1}]]
	if {![string is integer -strict $i]} {
	    return 0
	}
	if {$i < 0} {
	    return 0
	}
	return [hs::Is_simple_c_postfix [string range $name 0 [expr {$index-1}]]]
    }
    set index [string last "." $name]
    if {$index <= 0} {
	return 0
    }
    set id [string range $name [expr {$index+1}] $last_index]
    if {![hs::Is_valid_c_identifier $id]} {
	return 0
    }
    return [hs::Is_simple_c_postfix [string range $name 0 [expr {$index-1}]]]
}

#########################################################################

proc ::hs::Varlist_pack_code {varlist chan c_array_name true_for_pack} {
    #
    # This function assumes that every name from varlist has already
    # been checked with the hs::Is_simple_c_postfix function
    #
    # Analyze the variable names
    array unset pack_data
    hs::Ntuple_packing_structure $varlist pack_data

    # Declare the data structures and/or variables
    puts $chan "#ifndef DO_NOT_DECLARE_NTUPLE_PACKING_STRUCTURE"
    hs::Declare_ntuple_structures $chan pack_data
    puts $chan "    int ntuple_column;"

    # Do we need to reset the data structures?
    if {$true_for_pack} {
	puts $chan ""
	hs::Reset_ntuple_structures $chan pack_data
    }
    puts $chan "#endif /* DO_NOT_DECLARE_NTUPLE_PACKING_STRUCTURE */"
    puts $chan ""

    # Actual ntuple packing/unpacking
    puts $chan "    ntuple_column = 0;"
    if {$true_for_pack} {
	foreach varname $varlist {
	    puts $chan "    $c_array_name\[ntuple_column++\] = (float)($varname);"
	}
    } else {
	foreach varname $varlist {
	    puts $chan "    $varname = $c_array_name\[ntuple_column++\];"
	}
    }
    return
}

#########################################################################

proc ::hs::Ntuple_pack_code {ntuple_id chan c_array_name true_for_pack} {
    set varlist [hs::ntuple_variable_list $ntuple_id]
    foreach varname $varlist {
	if {![hs::Is_simple_c_postfix $varname]} {
	    error "Column name \"$varname\" is not a valid C variable name"
	}
    }
    hs::Varlist_pack_code $varlist $chan $c_array_name $true_for_pack
    puts $chan ""
    return
}

#########################################################################

proc ::hs::Ntuple_packing_structure {varlist arrname} {
    upvar $arrname children
    array unset children
    foreach name [hs::Compactify_arrays $varlist] {
	set components [split $name {.}]
	if {[info exists children($components)]} {
	    error "\"$name\" is either a duplicate name or\
		    a struct used as a variable"
	} else {
	    set children($components) {}
	}
	set depth [llength $components]
	if {$depth > 1} {
	    incr depth -1
	    for {set level 0} {$level < $depth} {incr level} {
		set parent [lrange $components 0 $level]
		set child [lindex $components [expr {$level+1}]]
		if {[info exists children($parent)]} {
		    if {[llength $children($parent)] == 0} {
			error "\"$name\" is either a duplicate name or\
				contains struct used as a variable"
		    }
		    if {[lsearch -exact $children($parent) $child] < 0} {
			lappend children($parent) $child
		    }
		} else {
		    set children($parent) [list $child]
		}
	    }
	}
    }
    return
}

#########################################################################

proc ::hs::Reset_ntuple_structures {chan pack_array} {
    upvar $pack_array children
    foreach name [lsort -dictionary [array names children]] {
	set len [llength $name]
	if {$len == 1} {
	    # This is a top-level name
	    set name [lindex $name 0]
	    set arr_bracket [string first "\[" $name]
	    if {$arr_bracket >= 0} {
		# This is an array
		set name [string range $name 0 [incr arr_bracket -1]]
		puts $chan "    memset($name, 0, sizeof($name));"
	    } elseif {[llength $children($name)] == 0} {
		# This is a single variable
		puts $chan "    $name = 0.f;"
	    } else {
		# This is a single struct
		puts $chan "    memset(&$name, 0, sizeof($name));"
	    }
	} elseif {$len == 0} {
	    error "Structure \$name\" has an empty member"
	}
    }
    return
}

#########################################################################

proc ::hs::Declare_ntuple_structures {chan pack_array} {
    upvar $pack_array children
    foreach name [lsort -dictionary [array names children]] {
	set len [llength $name]
	if {$len == 1} {
	    # This is a top-level name
	    hs::Declare_ntuple_element $chan children $name 4
	} elseif {$len == 0} {
	    error "Structure \$name\" has an empty member"
	}
    }
    return
}

#########################################################################

proc ::hs::Generate_c_struct_name {namelist} {
    variable C_struct_counter
    set newname "_[incr C_struct_counter]"
    foreach name $namelist {
	set arr_bracket [string first "\[" $name]
	if {$arr_bracket >= 0} {
	    set name [string range $name 0 [incr arr_bracket -1]]
	}
	append newname "_" $name
    }
    set newname
}

#########################################################################

proc ::hs::Declare_ntuple_element {chan array name skipspaces} {
    upvar $array children
    set lastname [lindex [lrange $name end end] 0]
    if {[llength $children($name)] == 0} {
	# This is a simple variable
	puts -nonewline $chan [format %${skipspaces}s ""]
	puts $chan "float $lastname;"
    } else {
	# This is a struct
	puts -nonewline $chan [format %${skipspaces}s ""]
	puts $chan "struct [hs::Generate_c_struct_name $name] \{"
	foreach child [lsort -dictionary $children($name)] {
	    hs::Declare_ntuple_element $chan children \
		    [concat $name [list $child]] [expr {$skipspaces + 4}]
	}
	puts -nonewline $chan [format %${skipspaces}s ""]
	puts $chan "\} $lastname;"
    }
    return
}

#########################################################################

proc ::hs::Compactify_arrays {varlist} {
    # First pass: determine largest indices used
    array unset maxindices
    foreach name $varlist {
	set array_key ""
	set right_ind -1
	set left_ind [string first "\[" $name [expr {$right_ind + 1}]]
	while {$left_ind >= 0} {
	    append array_key [string range $name \
		    [expr {$right_ind + 1}] [expr {$left_ind - 1}]]
	    set right_ind [string first "\]" $name [expr {$left_ind + 1}]]
	    set index [string range $name \
		    [expr {$left_ind + 1}] [expr {$right_ind - 1}]]
	    if {[info exists maxindices($array_key)]} {
		if {$maxindices($array_key) < 0} {
		    error "Incomplete array specification \"$name\""
		}
		if {$maxindices($array_key) < $index} {
		    set maxindices($array_key) $index
		}
	    } else {
		set maxindices($array_key) $index
	    }
	    append array_key "\[\]"
	    set left_ind [string first "\[" $name [expr {$right_ind + 1}]]
	}
	append array_key [string range $name [expr {$right_ind + 1}] end]
	if {[info exists maxindices($array_key)]} {
	    if {$maxindices($array_key) >= 0} {
		error "Incomplete array specification \"$name\""
	    }
	} else {
	    set maxindices($array_key) -1
	}
    }

    # Increment the largest index to get the array size
    foreach array_key [array names maxindices] {
	incr maxindices($array_key)
    }

    # Second pass: substitute the array sizes instead of all indices
    set newnames {}
    foreach name $varlist {
	set newname ""
	set array_key ""
	set right_ind -1
	set left_ind [string first "\[" $name [expr {$right_ind + 1}]]
	while {$left_ind >= 0} {
	    set chunk [string range $name \
		    [expr {$right_ind + 1}] [expr {$left_ind - 1}]]
	    append array_key $chunk
	    append newname $chunk
	    set maxind $maxindices($array_key)
	    if {$maxind} {
		append newname "\[$maxind\]"
	    }
	    append array_key "\[\]"
	    set right_ind [string first "\]" $name [expr {$left_ind + 1}]]
	    set left_ind [string first "\[" $name [expr {$right_ind + 1}]]
	}
	set chunk [string range $name [expr {$right_ind + 1}] end]
	append array_key $chunk
	if {$maxindices($array_key)} {
	    error "Internal algorithm error"
	}
	append newname $chunk
	lappend newnames $newname
    }
    lsort -unique -dictionary $newnames
}

#########################################################################

proc ::hs::ntuple_c_add_variables {args} {
    # Expected arguments: nt_id uid title category c_filter_expr args
    set known_switches {-include -eval -cflags}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-cflags)]} {
	set cflags $options(-cflags)
    } else {
	set cflags {}
    }
    if {[info exists options(-eval)]} {
	set utilCode $options(-eval)
    } else {
	set utilCode {}
    }
    set arglen [llength $arglist]
    if {$arglen < 5 || [expr {($arglen - 5) % 2}]} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id uid title category c_filter_expr} $arglist break
    if {[string compare [hs::type $nt_id] "HS_NTUPLE"]} {
	error "Item with id $nt_id is not an ntuple"
    }
    if {$arglen == 5} {
	# No new variables defined
	return [hs::ntuple_c_filter -include $includeCode \
		$nt_id $uid $title $category $c_filter_expr]
    }
    set varlist [hs::ntuple_variable_list $nt_id]
    set deflist $varlist
    foreach {name definition} [lrange $arglist 5 end] {
	lappend varlist $name
	lappend deflist $definition
    }
    set new_id [hs::create_ntuple $uid $title $category $varlist]
    global ::errorInfo
    if {[catch {
	hs::Ntuple_c_project_onntuple $includeCode $utilCode \
		$nt_id $new_id $c_filter_expr $deflist $cflags
    } ermess]} {
	set savedInfo $::errorInfo
	hs::delete $new_id
	error $ermess $savedInfo
    }
    return $new_id
}

#########################################################################

proc ::hs::ntuple_c_replace_variables {args} {
    # Expected arguments: nt_id uid title category c_filter_expr args
    set known_switches {-include -eval -project -cflags}    
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[info exists options(-include)]} {
	set includeCode $options(-include)
    } else {
	set includeCode {}
    }
    if {[info exists options(-eval)]} {
	set utilCode $options(-eval)
    } else {
	set utilCode {}
    }
    if {[info exists options(-cflags)]} {
	set cflags $options(-cflags)
    } else {
	set cflags {}
    }
    if {[info exists options(-project)]} {
        if {[catch {
            if {$options(-project)} {
                set projMode 1
            } else {
                set projMode 0
            }
        }]} {
            error "expected a boolean value for the \"-project\"\
                    option, got \"$options(-project)\""
        }
    } else {
        set projMode 0
    }
    set arglen [llength $arglist]
    if {$arglen < 5 || [expr {($arglen - 5) % 3}]} {
	error "wrong # of arguments or invalid option"
    }
    foreach {nt_id uid title category c_filter_expr} $arglist break
    if {[string compare [hs::type $nt_id] "HS_NTUPLE"]} {
	error "Item with id $nt_id is not an ntuple"
    }
    if {$projMode} {
        set new_id [hs::id $uid $category]
        if {$new_id <= 0} {
            error "Item with uid $uid and category\
                    \"$category\" does not exist"
        }
        if {[string compare [hs::type $new_id] "HS_NTUPLE"]} {
            error "Item with uid $uid and category\
                    \"$category\" is not an ntuple"
        }
    }
    if {$arglen == 5 && !$projMode} {
	# No replacement variables defined
	return [hs::ntuple_c_filter -include $includeCode \
		$nt_id $uid $title $category $c_filter_expr]
    }
    set varlist [hs::ntuple_variable_list $nt_id]
    set nvars [llength $varlist]
    set i 0
    foreach varname $varlist {
        set vararray($i) $varname
        set defarray($i) $varname
        incr i
    }
    foreach {oldname newname definition} [lrange $arglist 5 end] {
        set i [lsearch -exact $varlist $oldname]
        if {$i < 0} {
            error "\$oldname\" is not a variable of ntuple with id $nt_id"
        }
        set vararray($i) $newname
        set defarray($i) $definition
    }
    set varlist {}
    set deflist {}
    for {set i 0} {$i < $nvars} {incr i} {
        lappend varlist $vararray($i)
        lappend deflist $defarray($i)
    }
    if {$projMode} {
        if {[hs::ntuple_variable_list $new_id] != $varlist} {
            error "incompatible variable names in the projection ntuple"
        }
    } else {
        set new_id [hs::create_ntuple $uid $title $category $varlist]
    }
    global ::errorInfo
    if {[catch {
	hs::Ntuple_c_project_onntuple $includeCode $utilCode \
            $nt_id $new_id $c_filter_expr $deflist $cflags
    } ermess]} {
	set savedInfo $::errorInfo
        if {!$projMode} {
            hs::delete $new_id
        }
	error $ermess $savedInfo
    }
    return $new_id
}

#########################################################################

proc ::hs::ntuple_add_variables {id_kmJGk89kb3 uid title categ f_ex_JdOnop9asSOU args} {
    #
    # Almost all variables in this proc must have obscure names so that
    # they do not interfere with variable names in the ntuple
    #
    if {[string compare [hs::type $id_kmJGk89kb3] "HS_NTUPLE"]} {
	error "Item with id $id_kmJGk89kb3 is not an ntuple"
    }
    set arglen [llength $args]
    if {$arglen == 0} {
	return [hs::ntuple_filter $id_kmJGk89kb3 $uid $title $categ $f_ex_JdOnop9asSOU]
    }
    if {[expr {$arglen % 2}]} {
	error "wrong # of arguments"
    }
    unset arglen
    if {[info complete $f_ex_JdOnop9asSOU] == 0} {
	error "filter expression is not complete"
    }
    set Var8758Jjtkr2 [::hs::ntuple_variable_list $id_kmJGk89kb3]
    set varnames $Var8758Jjtkr2
    set deflist_gQn8KvpQzM {}
    foreach {name definition} $args {
	lappend varnames $name
	if {![info complete $definition]} {
	    error "expression \"$definition\" is not complete"
	}
	lappend deflist_gQn8KvpQzM $definition
    }
    set New_id_JY76gHGi9 [hs::create_ntuple $uid $title $categ $varnames]
    unset name definition varnames
    set Num_jf57HMhi0sqm [hs::num_entries $id_kmJGk89kb3]
    set nvar_vJG68_kJog65 [hs::num_variables $id_kmJGk89kb3]
    global ::errorInfo
    if {[catch {
	for {set i_JGT5bk_k6iO 0} {$i_JGT5bk_k6iO < $Num_jf57HMhi0sqm}\
		{incr i_JGT5bk_k6iO} {
	    set row_kJMghlj_z5 [hs::row_contents $id_kmJGk89kb3 $i_JGT5bk_k6iO]
	    binary scan $row_kJMghlj_z5 f$nvar_vJG68_kJog65 jklLIl9oL_IYRj47
	    foreach $Var8758Jjtkr2 $jklLIl9oL_IYRj47 {}
	    if {[eval $f_ex_JdOnop9asSOU]} {
		foreach x_expr_6hjdas4e $deflist_gQn8KvpQzM {
		    lappend jklLIl9oL_IYRj47 [eval $x_expr_6hjdas4e]
		}
		hs::fill_ntuple $New_id_JY76gHGi9 $jklLIl9oL_IYRj47
	    }
	}
    } ermess]} {
	set savedInfo $::errorInfo
	hs::delete $New_id_JY76gHGi9
	error $ermess $savedInfo
    }
    return $New_id_JY76gHGi9
}

#########################################################################

proc ::hs::Use_distinct_window_names {} {
    variable Distinct_window_names
    if {$Distinct_window_names == 0} {
	# Automatic mode
	global tcl_interactive
	return [expr {!$tcl_interactive}]
    }
    expr {$Distinct_window_names > 0}
}

############################################################################

proc ::hs::window_names_distinct {onoff} {
    variable Distinct_window_names
    if {$onoff} {
	set Distinct_window_names 1
    } else {
	set Distinct_window_names -1
    }
    return
}

############################################################################

proc ::hs::latex {args} {
    eval hs::Epsf_or_Latex latex $args
}

############################################################################

proc ::hs::epsf {args} {
    eval hs::Epsf_or_Latex epsf $args
}

############################################################################

proc ::hs::default_latex_packages {} {
    list color
}

############################################################################

proc ::hs::Epsf_or_Latex {what args} {
    # Arguments should be: what file_or_text position
    # However, we should expect to see the switches
    # interspersed anywhere between the arguments.
    set known_switches [list -bordercolor -scale -bg -background \
            -row -column -ipadx -ipady -refpoint -coord -coords \
            -origin -border -anchor -window -immediate]

    if {[string equal $what "latex"]} {
	set latex 1
	lappend known_switches -packages -interact -complete -rotate
	set winaction Latex
    } elseif {[string equal $what "epsf"]} {
	set latex 0
	set winaction PSPixmap
    } else {
	error "Invalid item type. This is a bug. Please report."
    }

    # Figure out the ghostscript version
    variable Ghostscript_version
    if {![info exists Ghostscript_version]} {
	global ::env
	if ([info exists ::env(GHOSTSCRIPT)]) {
	    set ghostscript_exe $::env(GHOSTSCRIPT)
	} else {
	    set ghostscript_exe [auto_execok gs]
	}
	if {[string equal $ghostscript_exe ""]} {
	    error "ghostscript executable not found"
	}
	set Ghostscript_version [exec $ghostscript_exe --version]
    }
    if {[lindex [split $Ghostscript_version .] 0] < 7} {
	error "Please upgrade your Ghostscript to version 7.0 or higher\
		and restart [file tail [info nameofexecutable]]"
    }

    # Default settings
    set bordercolor "" ;# undefined, Histo-Scope will use black
    set border 1       ;# border is on
    set background ""  ;# undefined, Histo-Scope will use white

    # Default coordinate system and origin
    set default_coords "winrel"
    set default_origin "O"

    # Parse the input arguments
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    if {[llength $arglist] != 2} {
	error "wrong # of arguments or invalid option"
    }
    foreach {file_or_text position} $arglist {}

    # Figure out the window name
    if {[info exists options(-window)]} {
	set window $options(-window)
    } else {
	variable Last_window_posted
	if {[string equal $Last_window_posted ""]} {
	    error "Can't figure out window name.\
		    Please use the \"-window\" option."
	} else {
	    set window $Last_window_posted
	}
    }
    variable Known_window_names
    if {![info exists Known_window_names($window)]} {
	error "Invalid window name \"$window\""
    }

    # Check if we should suppress the window refresh
    if {[info exists options(-immediate)]} {
	if {$options(-immediate)} {
	    set suppress_redraw 0
	} else {
	    set suppress_redraw 1
	}
    } else {
	set suppress_redraw 0
    }

    # Check coordinate system and origin
    if {[info exists options(-coord)]} {
	set default_coords [hs::Subst_coords_type $options(-coord)]
    }
    if {[info exists options(-coords)]} {
	set default_coords [hs::Subst_coords_type $options(-coords)]
    }
    if {[info exists options(-origin)]} {
	set default_origin [hs::Subst_origin_type $options(-origin)]
    }

    # Check reference point
    if {[info exists options(-refpoint)]} {
	set refpoint [hs::Parse_drawing_point $options(-refpoint) \
		$default_coords $default_origin]
    } else {
	set refpoint "" ;# Will use Histo-Scope default
    }

    # Check the position argument
    set position [hs::Parse_drawing_point $position \
	    $default_coords $default_origin]

    # Check border color
    set check_colors {}
    if {[info exists options(-bordercolor)]} {
	set bordercolor $options(-bordercolor)
	if {![string equal $bordercolor "black"] && \
		![string equal $bordercolor ""]} {
	    lappend check_colors $bordercolor
	}
    }

    # Check background
    if {[info exists options(-background)]} {
	set background $options(-background)
    } elseif {[info exists options(-bg)]} {
	set background $options(-bg)
    }
    if {![string equal $background "black"] && \
	    ![string equal $background "white"] && \
	    ![string equal $background ""]} {
	lappend check_colors $background
    }

    # Verify the validity all colors
    if {[llength $check_colors] > 0} {
	hs::Verify_color_names $check_colors
    }

    # Check internal padding
    foreach {switchname confignames} {
	-ipadx {AddToLeftMargin AddToRightMargin}
	-ipady {AddToTopMargin AddToBottomMargin}
    } {
	if {[info exists options($switchname)]} {
	    if {[llength $options($switchname)] != 2} {
		error "invalid $switchname option value \"$options($switchname)\""
	    }
	    foreach configname $confignames value $options($switchname) {
		if {![string is integer -strict $value]} {
		    error "invalid $switchname option value \"$options($switchname)\""
		}
		set $configname $value
	    }
	}
    }

    # Check row and column
    if {[info exists options(-row)]} {
	set row $options(-row)
	if {![string is integer -strict $row]} {
	    error "Bad multiplot row \"$row\""
	} elseif {$row < 0} {
	    error "Bad multiplot row \"$row\""
	}
    } else {
	set row -1
    }
    if {[info exists options(-column)]} {
	set column $options(-column)
	if {![string is integer -strict $column]} {
	    error "Bad multiplot column \"$column\""
	} elseif {$column < 0} {
	    error "Bad multiplot column \"$column\""
	}
    } else {
	set column -1
    }
    if {$row < 0 && $column >= 0} {
	error "Multiplot row not specified"
    }
    if {$row >= 0 && $column < 0} {
	error "Multiplot column not specified"
    }

    # Check the border
    if {[info exists options(-border)]} {
	set border $options(-border)
	if {![string is boolean -strict $border]} {
	    error "expected a boolean argument for\
		    the -border option, got \"$border\""
	}
    }

    # Check the anchor
    if {[info exists options(-anchor)]} {
	set anchor [hs::Verify_comment_anchor $options(-anchor)]
    } else {
	set anchor ""
    }

    # Check the scale
    if {[info exists options(-scale)]} {
	set scale $options(-scale)
	if {![string is double -strict $scale]} {
	    error "expected a positive double argument for\
		    the -scale option, got \"$scale\""
	}
	if {$scale < 0.0} {
	    error "expected a positive double argument for\
		    the -scale option, got \"$scale\""
	}
    } else {
	set scale  1.0
    }

    # Generate EPS from latex
    if {$latex} {
	# Check extra latex packages
	if {[info exists options(-packages)]} {
	    set packages $options(-packages)
	} else {
	    set packages {}
	}
	# Check interaction mode
	if {[info exists options(-interact)]} {
	    set imode $options(-interact)
	    set valid_modes {batch nonstop errorstop}
	    if {[lsearch -exact $valid_modes $imode] < 0} {
		error "Invalid latex interaction mode \"$imode\".\
			Valid modes are [join $valid_modes {, }]."
	    }
	} else {
	    set imode "batch"
	}
        # Is it a complete document?
        if {[info exists options(-complete)]} {
	    if {![string is boolean -strict $options(-complete)]} {
		error "Invalid argument \"$options(-complete)\" for\
			the \"-complete\" option, expected a boolean"
	    }
	    set complete $options(-complete)
	} else {
	    set complete 0
	}
        # Check rotation option
        if {[info exists options(-rotate)]} {
            set rotate_opt_valid 1
            set rotate $options(-rotate)
            if {[string is integer -strict $rotate]} {
                while {$rotate < 0 || $rotate >= 360} {
                    if {$rotate < 0} {
                        incr rotate 360
                    } else {
                        incr rotate -360
                    }
                }
                if {[expr {$rotate % 90}]} {
                    set rotate_opt_valid 0
                }
            } else {
                switch -- $rotate {
                    none {
                        set rotate 0
                    }
                    left {
                        set rotate 90
                    }
                    right {
                        set rotate 270
                    }
                    around {
                        set rotate 180
                    }
                    default {
                        set rotate_opt_valid 0
                    }
                }
            }
            if {!$rotate_opt_valid} {
		error "Invalid argument \"$options(-rotate)\" for\
			the \"-rotate\" option, expected an integer\
                        divisible by 90, or one of the keywords \"none\",\
                        \"left\", \"right\", \"around\""
            }
        } else {
            set rotate 0
        }
	# Convert the string to PostScript
	if {[catch {hs::Generate_ps_fromlatex $packages \
		$imode $file_or_text $complete $rotate} filename]} {
	    error "Failed to generate eps from latex:\
		    $filename. Check your latex input."
	}
    } else {
	set filename $file_or_text
	if {![file readable $filename]} {
	    error "File \"$filename\" not found (or unreadable)"
	}
    }

    # Figure out if we must draw the border
    if {[string equal $border ""]} {
	if {[string equal $bordercolor ""]} {
	    set draw_border 0
	} else {
	    set draw_border 1
	}
    } else {
	set draw_border $border
    }

    # Generate the configuration string
    set config_string "WindowAction\n\
	    WindowName $window\n\
	    Action $winaction $filename\n\
	    Point1 $position\n\
	    Magnification $scale\n"
    if {$row >= 0} {
	append config_string "\
		Row [expr {$row + 1}]\n"
    }
    if {$column >= 0} {
	append config_string "\
		Column [expr {$column + 1}]\n"
    }
    if {$suppress_redraw} {
	append config_string "\
		NoRedraw\n"
    }
    if {$draw_border} {
	append config_string "\
		DrawBorder\n"
    }
    if {![string equal $bordercolor ""]} {
	append config_string "\
		Foreground $bordercolor\n"
    }
    if {![string equal $background ""]} {
	append config_string "\
		Background $background\n"
    }
    if {![string equal $refpoint ""]} {
	append config_string "\
		ReferencePoint $refpoint\n"
    }
    if {![string equal $anchor ""]} {
	append config_string "\
		Alignment $anchor\n"
    }
    foreach name {
	AddToLeftMargin
	AddToRightMargin
	AddToTopMargin
	AddToBottomMargin
    } {
	if {[info exists $name]} {
	    append config_string "\
		    $name [set $name]\n"
	}
    }
    if {$latex} {
	# Remove the temporary ps file
	append config_string "\
		Unlink\n"
    }

    hs::Push_history any [list $window $row $column $what]
    hs::load_config_string $config_string
    if {!$suppress_redraw} {
	hs::hs_update
    }
    return
}

############################################################################

proc ::hs::Generate_ps_fromlatex {extra_packages intermode \
                                  text iscomplete rotate} {
    # Find ghsostscript, latex and dvips executables
    set latex_exe [auto_execok latex]
    if {[string equal $latex_exe ""]} {
	error "latex executable not found"
    }
    set dvips_exe [auto_execok dvips]
    if {[string equal $dvips_exe ""]} {
	error "dvips executable not found"
    }

    # Check the latex interaction mode
    switch $intermode {
	batch {
	    set redir ">&/dev/null"
	    set dvipsflags -q
	}
	nonstop {
	    set redir ">&@stdout"
	    set dvipsflags ""
	}
	errorstop {
	    set redir ">&@stdout <@stdin"
	    set dvipsflags ""
	}
	default {
	    error "Invalid latex interaction mode \"$intermode\".\
		    Valid modes are batch, nonstop, and errorstop."
	}
    }

    # Check the rotation angle
    if {![string is integer -strict $rotate]} {
        error "Invalid rotation angle \"$rotate\""
    }

    # Go to a temporary directory (latex always writes
    # files into the current directory)
    set current_dir [pwd]
    cd [::hs::tempdir]

    # Reserve files with extensions tex, log, aux, dvi, and ps.
    # Try to do it in a reasonably secure way.
    set access [list RDWR CREAT EXCL TRUNC]
    set permission 0600
    set maxtries 5
    set extension_list {log aux dvi ps}
    if {$rotate} {
        lappend extension_list eps
    }
    for {set i 0} {$i < $maxtries} {incr i} {
	set filelist {}
	if {[catch {
	    foreach {texfile chan} [hs::tempfile hs_latex_ ".tex"] {}
	    close $chan
	    lappend filelist $texfile
	    set basename [file rootname $texfile]
	    foreach ext $extension_list {
		set newname $basename.$ext
		set chan [open $newname $access $permission]
		close $chan
		lappend filelist $newname
	    }
	}]} {
	    eval file delete $filelist
	} else {
	    break
	}
    }
    if {[llength $filelist] != [expr [llength $extension_list] + 1]} {
	cd $current_dir
	error "Failed to open a temporary file"
    }

    global ::errorInfo
    set status [catch {
	# Write out the basic latex code
	set chan [open $basename.tex "w"]
	if {!$iscomplete} {
	    puts $chan {\documentclass[12pt,notitlepage]{article}}
	    set packages [concat $extra_packages [hs::default_latex_packages]]
	    foreach package [lsort -unique $packages] {
		puts $chan "\\usepackage\{$package\}"
	    }
	    puts $chan {\begin{document}}
	    puts $chan {\pagestyle{empty}}
	    puts $chan ""
	}
	puts $chan $text
	if {!$iscomplete} {
	    puts $chan ""
	    puts $chan {\end{document}}
	}
	close $chan

	# Process the tex file
	eval exec $redir [list $latex_exe] \
		-interaction=${intermode}mode [list $basename.tex]

        # Process the dvi file
        if {$rotate} {
            set epsfile $basename.eps
        } else {
            set epsfile $basename.ps
        }
	eval exec [list $dvips_exe] $redir $dvipsflags -E -o \
		[list $epsfile] [list $basename.dvi]
	if {[file size $epsfile] == 0} {
	    error "Postscript generation failed"
	}

        # Rotate the eps if necessary
        if {$rotate} {
            hs::Rotate_latex_eps $epsfile $basename.ps $rotate
        }
    } errMes]
    set savedInfo $::errorInfo
    foreach ext {tex dvi log aux eps} {
        catch {file delete $basename.$ext}
    }
    if {$status} {
        catch {file delete $basename.ps}
    }
    cd $current_dir
    if {$status} {
	error $errMes $savedInfo
    }
    file join [::hs::tempdir] $basename.ps
}

############################################################################

proc ::hs::Rotate_latex_eps {infile outfile angle} {
    if {$angle} {
        set chan [open $infile r]
        set lines [split [read $chan [file size $infile]] "\n"]
        close $chan
        # Figure out the bounding box
        set bbox_found 0
        set has_end_prolog 0
        set has_end_setup 0
        foreach line $lines {
            if {[string equal -length 14 $line "%%BoundingBox:"]} {
                foreach {dumm xmin ymin xmax ymax} $line {}
                set bbox_found 1
            }
            if {[string equal -length 11 $line "%%EndProlog"]} {
                set has_end_prolog 1
            }
            if {[string equal -length 10 $line "%%EndSetup"]} {
                set has_end_setup 1
            }
        }
        if {!$bbox_found} {
            error "Bounding box not found in file \"$infile\""
        }
        if {!$has_end_prolog && !$has_end_setup} {
            error "No prolog or setup found in file \"$infile\""
        }
        # Calculate the transform
        switch -- $angle {
            90 {
                set width  [expr {$ymax - $ymin}]
                set height [expr {$xmax - $xmin}]
                set xshift $width
                set yshift 0
            }
            180 {
                set width  [expr {$xmax - $xmin}]
                set height [expr {$ymax - $ymin}]
                set xshift $width
                set yshift $height
            }
            270 {
                set width  [expr {$ymax - $ymin}]
                set height [expr {$xmax - $xmin}]
                set xshift 0
                set yshift $height
            }
            default {
                error "Bad rotation angle $angle, must be 0, 90, 180, or 270."
            }
        }
        # Write the output file
        set chan [open $outfile w]
        set rotation_state 1
        set replace_bbox 1
        foreach line $lines {
            if {$replace_bbox} {
                if {[string equal -length 14 $line "%%BoundingBox:"]} {
                    set line "%%BoundingBox: 0 0 $width $height"
                    set replace_bbox 0
                }
            }
            if {$rotation_state} {
                if {$rotation_state == 2} {
                    # Put in the rotation code
                    puts $chan "$xshift $yshift translate"
                    puts $chan "$angle rotate"
                    puts $chan "[expr {-1*$xmin}] [expr {-1*$ymin}] translate"
                    set rotation_state 0
                } else {
                    # Search for the end of prolog/setup
                    if {$has_end_setup} {
                        if {[string equal -length 10 $line "%%EndSetup"]} {
                            set rotation_state 2
                        }
                    } else {
                        if {[string equal -length 11 $line "%%EndProlog"]} {
                            set rotation_state 2
                        }
                    }
                }
            }
            puts $chan $line
        }
        close $chan
    } else {
        file copy -force $infile $outfile
    }
    return
}

############################################################################

proc ::hs::kernel_density_1d {args} {
    # Arguments: ntuple_id points_column bw id [xmin xmax npoints]
    set known_switches {-weight -localbw -kernel}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    set arglen [llength $arglist]
    if {$arglen == 4} {
	foreach {nt_id points_column bw id} $arglist {}
    } elseif {$arglen == 7} {
	foreach {nt_id points_column bw id xmin xmax npoints} $arglist {}
    } else {
	error "wrong # of arguments"
    }
    set points_col [hs::variable_index $nt_id $points_column]
    if {$points_col < 0} {
	set hs_type [hs::type $nt_id]
	if {![string equal $hs_type "HS_NTUPLE"]} {
	    if {[string equal $hs_type "HS_NONE"]} {
		error "Histo-Scope item with id $nt_id does not exist"
	    } else {
		error "Histo-Scope item with id $nt_id is not an Ntuple"
	    }
	}
	error "wrong column name \"$points_column\" for ntuple with id $nt_id"
    }

    # Check the options
    if {[info exists options(-weight)]} {
	set weight_col [hs::variable_index $nt_id $options(-weight)]
	if {$weight_col < 0} {
	    error "wrong column name \"$options(-weight)\" for ntuple with id $nt_id"
	}
    } else {
	set weight_col -1
    }
    if {[info exists options(-localbw)]} {
	set localbw_col [hs::variable_index $nt_id $options(-localbw)]
	if {$localbw_col < 0} {
	    error "wrong column name \"$options(-localbw)\" for ntuple with id $nt_id"
	}
    } else {
	set localbw_col -1
    }
    if {[info exists options(-kernel)]} {
	set kernel $options(-kernel)
    } else {
	set kernel "gaussian"
    }

    # Create the density estimate
    if {$arglen == 4} {
	hs::Kernel_density_1d $nt_id $points_col $weight_col \
		$localbw_col $kernel $bw $id
    } elseif {$arglen == 7} {
	hs::Kernel_density_1d $nt_id $points_col $weight_col \
		$localbw_col $kernel $bw $id $xmin $xmax $npoints
    } else {
	error "Internal error. This is a bug. Please report."
    }
    return
}

############################################################################

proc ::hs::kernel_density_2d {args} {
    # Arguments: ntuple_id x_column y_column bw_matrix id \
    #        [xmin xmax nxpoints ymin ymax nypoints]
    set known_switches {-weight -sxsq -sysq -sxsy -kernel -transform}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    set arglen [llength $arglist]
    if {$arglen == 5} {
	foreach {nt_id x_column y_column bw_matrix id} $arglist {}
    } elseif {$arglen == 11} {
	foreach {nt_id x_column y_column bw_matrix id \
		xmin xmax nxpoints ymin ymax nypoints} $arglist {}
    } else {
	error "wrong # of arguments"
    }
    set x_col [hs::variable_index $nt_id $x_column]
    if {$x_col < 0} {
	set hs_type [hs::type $nt_id]
	if {![string equal $hs_type "HS_NTUPLE"]} {
	    if {[string equal $hs_type "HS_NONE"]} {
		error "Histo-Scope item with id $nt_id does not exist"
	    } else {
		error "Histo-Scope item with id $nt_id is not an Ntuple"
	    }
	}
	error "wrong column name \"$x_column\" for ntuple with id $nt_id"
    }
    set y_col [hs::variable_index $nt_id $y_column]
    if {$y_col < 0} {
	error "wrong column name \"$y_column\" for ntuple with id $nt_id"
    }

    # Check the options
    foreach optname {weight sxsq sysq sxsy} {
	if {[info exists options(-$optname)]} {
	    set ${optname}_col [hs::variable_index $nt_id $options(-$optname)]
	    if {[set ${optname}_col] < 0} {
		error "wrong column name \"$options(-$optname)\"\
			for ntuple with id $nt_id"
	    }
	} else {
	    set ${optname}_col -1
	}
    }
    if {[info exists options(-kernel)]} {
	set kernel $options(-kernel)
    } else {
	set kernel "gaussian"
    }
    if {[info exists options(-transform)]} {
	if {[catch {
	    if {$options(-transform)} {
		set is_transform 1
	    } else {
		set is_transform 0
	    }
	}]} {
	    error "expected a boolean value for the -transform\
		    option, got \"$options(-transform)\""
	}
    } else {
	set is_transform 0
    }

    # Decompose the global covariance matrix if necessary
    if {$is_transform} {
	set transform_matrix $bw_matrix
    } else {
	# We need to find matrix A such that A_t*A = S^-1
	# We will also require that A_t = A.
	set transform_matrix [hs::Inverse_symmetric_sqrt_2d $bw_matrix]
    }

    # Create the density estimate
    if {$arglen == 5} {
	hs::Kernel_density_2d $nt_id $x_col $y_col $weight_col $sxsq_col\
		$sysq_col $sxsy_col $kernel $transform_matrix $id
    } elseif {$arglen == 11} {
	hs::Kernel_density_2d $nt_id $x_col $y_col $weight_col $sxsq_col\
		$sysq_col $sxsy_col $kernel $transform_matrix $id\
		$xmin $xmax $nxpoints $ymin $ymax $nypoints
    } else {
	error "Internal error. This is a bug. Please report."
    }
    return
}

############################################################################

proc ::hs::ntuple_so_scan {args} {
    # Arguments should be: ntuple_id so_file some_string?
    eval hs::Ntuple_so_or_dll_scan 0 $args
}

############################################################################

proc ::hs::ntuple_dll_scan {args} {
    # Arguments should be: ntuple_id dll_token some_string?
    eval hs::Ntuple_so_or_dll_scan 1 $args
}

############################################################################

proc ::hs::Ntuple_so_or_dll_scan {is_already_loaded args} {
    set known_switches {-reverse}
    set arglist [hs::Parse_single_arg_switches $args $known_switches options]
    set arglen [llength $arglist]
    if {$arglen == 2} {
        foreach {ntuple_id so_file} $arglist {}
        set some_string ""
    } elseif {$arglen == 3} {
        foreach {ntuple_id so_file some_string} $arglist {}
    } else {
	error "wrong # of arguments or invalid option"
    }
    if {[info exists options(-reverse)]} {
        set reverse_order $options(-reverse)
    } else {
        set reverse_order 0
    }
    hs::C_ntuple_so_or_dll_scan $ntuple_id $so_file \
            $reverse_order $is_already_loaded $some_string
}

############################################################################

proc ::hs::unique_rows {uid title category id varnames} {
    if {[llength $varnames] == 0} {
	return [hs::copy_hist $id $uid $title $category]
    }
    hs::Unique_rows $uid $title $category $id [lsort -unique $varnames]
}

############################################################################

if {[::hs::have_cernlib]} {
    proc ::hs::globcc {covmat index {n_events -9091929}} {
	set rank [llength $covmat]
	if {$rank <= 1} {
	    error "matrix dimensionality must be higher than 1"
	}
	if {$index < 0 || $index >= $rank} {
	    error "matrix index is out of range"
	}
	if {$n_events != -9091929} {
	    if {[expr {$n_events-$rank}] <= 0} {
		error "number of events is too small"
	    }
	}
	set invcov [hs::Invert_sym_pos_matrix $covmat]
	set c1 [lindex [lindex $covmat $index] $index]
	set c2 [lindex [lindex $invcov $index] $index]
	set rsquare [expr {1.0 - 1.0/($c1*$c2)}]
	if {$rsquare <= 0.0} {
	    set r 0.0
	} else {
	    set r [expr {sqrt($rsquare)}]
	}
	if {$n_events > 0} {
	    # Calculate adjusted rsquare
	    set rsquare [expr {1.0-(1.0-$rsquare)*($n_events-1)/($n_events-$rank)}]
	    if {$rsquare <= 0.0} {
		lappend r 0.0
	    } else {
		lappend r [expr {sqrt($rsquare)}]
	    }
	}
	set r
    }
}

############################################################################
# Multiple linear regression

if {[::hs::have_cernlib]} {
    proc ::hs::ntuple_c_linear_regress {args} {
	# Expected arguments: ?-include someCode? nt_id c_filter_expr 
        #                     c_weight_expr response_expr predictor_list
	set known_switches {-include -eval}
	set arglist [hs::Parse_single_arg_switches $args $known_switches options]
	if {[info exists options(-include)]} {
	    set includeCode $options(-include)
	} else {
	    set includeCode {}
	}
        if {[info exists options(-eval)]} {
            set utilCode $options(-eval)
        } else {
            set utilCode {}
        }
	if {[llength $arglist] != 5} {
	    error "wrong # of arguments or invalid option"
	}
	foreach {nt_id c_filter_expr c_weight_expr response_expr predictor_list} $arglist {}

	# Print a warning for non-unit weight
	if {![hs::C_weight_is_unit $c_weight_expr]} {
	    variable Hs_ntuple_c_mlr_warned
	    if {![info exists Hs_ntuple_c_mlr_warned]} {
		puts "WARNING from [lindex [info level 0] 0]:\
			weighted regression is not really working yet"
		set Hs_ntuple_c_mlr_warned 1
	    }
	}

	# Basic check for the number of degrees of freedom
	set n_events [hs::num_entries $nt_id]
	set n_predictors [llength $predictor_list]
	set ndof [expr {$n_events-$n_predictors-1}]
	if {$ndof <= 0} {
	    error "number of entries in the ntuple is too small"
	}

	# Calculate the system covariance matrix
	set covar_list $predictor_list
	lappend covar_list $response_expr
	foreach {mean covar} [hs::ntuple_c_covar -include $includeCode \
                -eval $utilCode $nt_id $c_filter_expr $c_weight_expr \
                $covar_list] {}
	set full_response_variance [lindex [lindex $covar end] end]
	if {$full_response_variance == 0.0} {
	    # We have been duped...
	    error "response expression is invariant for given data"
	}

	# Solve normal equations
	if {[catch {hs::Invert_sym_pos_matrix \
		[hs::Matrix_subrange $covar 0 end-1 0 end-1]} Vinv]} {
	    if {[string equal $Vinv "matrix is singular"]} {
		error "predictor covariance matrix is singular"
	    } else {
		error $Vinv
	    }
	}
	set v [lrange [lindex $covar end] 0 end-1]
	set regression_coefficients [hs::M $Vinv . $v]

	# Figure out the constant coefficient
	set predictor_mean [lrange $mean 0 end-1]
	set response_mean [lindex $mean end]
	set const_coeff $response_mean
	foreach b $regression_coefficients mu $predictor_mean {
	    set const_coeff [expr {$const_coeff - $b*$mu}]
	}

	# Simple estimate of the residual variance
	set residual_variance [expr $full_response_variance - \
		[hs::M $regression_coefficients t. $v]]
	if {$residual_variance <= 0.0} {
	    # May happen because of rounding errors. This is bad though.
	    set residual_variance 0.0
	}
	if {$residual_variance >= $full_response_variance} {
	    # This should never happen, but just in case...
	    set residual_variance $full_response_variance
	}

	# More compilcated code below can be used in troublesome cases.
	# Unfortunately, it doesn't seem to produce full 17-digit
	# precision numbers in printed strings. Not sure why, so this
	# code is in a dead branch for now...
	if {0} {
	    set tmpid [hs::create_ntuple [hs::Temp_uid] \
		    "Temporary regression ntuple" [hs::Temp_category] {res w}]
	    global ::errorInfo
	    set status [catch {
		::fit::Eval_at_precision 17 {
		    set predictor_terms {}
		    foreach b $regression_coefficients \
			    mu $predictor_mean \
			    predictor $predictor_list {
			lappend predictor_terms "$b*(($predictor)-$mu)"
		    }
		    set res_expr "(($response_expr)-$response_mean) -\
			    ([join $predictor_terms { + }])"
		    global tcl_precision
		}
		hs::ntuple_c_project $nt_id $tmpid $c_filter_expr $res_expr $c_weight_expr
		foreach {sum mean stdev min q25 median q75 max} \
			[hs::weighted_column_stats $tmpid 0 1] {}
		set residual_variance [expr {$stdev*$stdev}]
	    } errMes]
	    set savedInfo $::errorInfo
	    hs::delete $tmpid
	    if {$status} {
		error $errMes $savedInfo
	    }
	}

	# Figure out covariance matrix of the regression coefficients
	set x [hs::M [hs::M $Vinv . $predictor_mean] * -1.0]
	set b [lindex [hs::M $Vinv sim $predictor_mean] 0]
	set b [expr {$b + 1.0}]
	set regression_cov {}
	set row 0
	foreach rowdata $Vinv {
	    lappend rowdata [lindex $x $row]
	    lappend regression_cov $rowdata
	    incr row
	}
	lappend x $b
	lappend regression_cov $x
	set normfactor [expr {$residual_variance / $ndof}]
	set regression_cov [hs::M $regression_cov * $normfactor]

	# Global correlation coefficients
	set rsquare [expr {1.0 - $residual_variance/$full_response_variance}]
	set globcorr [expr sqrt($rsquare)]
	set adj_rsquare [expr {1.0-(1.0-$rsquare)*($n_events-1)/($ndof)}]
	if {$adj_rsquare > 0.0} {
	    set adj_globcorr [expr {sqrt($adj_rsquare)}]
	} else {
	    set adj_globcorr 0.0
	}

	# Form the result.
	# Just a memo: unbiased residual variance is
        #  ($residual_variance*$n_events)/$ndof
	set chisq [expr {$residual_variance*$n_events}]
	lappend regression_coefficients $const_coeff
	list $regression_coefficients $regression_cov $chisq $ndof $globcorr $adj_globcorr
    }
}

############################################################################
# Convert covariance matrix into correlation matrix

proc ::hs::covar_to_corr {covmat {allow_zero_sigmas 0}} {
    foreach {nrows ncols} [hs::M $covmat dim] {}
    if {$nrows != $ncols} {
	error "matrix is not square"
    }
    # Calculate standard deviations
    set row 0
    foreach rowdata $covmat {
        set diag [lindex $rowdata $row]
	if {$diag <= 0.0} {
            if {$allow_zero_sigmas} {
                if {$diag < 0.0} {
                    error "diagonal element $row is negative"
                }
            } else {
                error "diagonal element $row is not positive"
            }
	}
        set sig($row) [expr {sqrt($diag)}]
        incr row
    }
    # Build the correlation matrix
    set newmat {}
    set row 0
    foreach rowdata $covmat {
	set newrow {}
	set col 0
	foreach value $rowdata {
	    if {$row == $col} {
		lappend newrow 1.0
	    } else {
                if {$sig($row) > 0.0 && $sig($col) > 0.0} {
                    lappend newrow [expr {$value/$sig($row)/$sig($col)}]
                } else {
                    lappend newrow 0.0
                }
	    }
	    incr col
	}
	lappend newmat $newrow
	incr row
    }
    set newmat
}

############################################################################

proc ::hs::Matrix_subrange {matrix rowmin rowmax colmin colmax} {
    set newmat {}
    foreach rowdata [lrange $matrix $rowmin $rowmax] {
	lappend newmat [lrange $rowdata $colmin $colmax]
    }
    set newmat
}

############################################################################

proc ::hs::Matrix_square_subset {matrix index_list} {
    foreach {nrows ncols} [hs::M $matrix dim] {}
    if {$nrows < $ncols} {
	set size $nrows
    } else {
	set size $ncols
    }
    foreach el $index_list {
	if {![string is integer -strict $el]} {
	    error "expected a non-negative integer, got \"$el\""
	}
	if {$el < 0 || $el >= $size} {
	    error "index is out of range"
	}
    }
    set newsize [llength $index_list]
    if {$newsize != [llength [lsort -unique $index_list]]} {
	error "indices must be unique"
    }
    set newmat {}
    for {set i 0} {$i < $newsize} {incr i} {
	set oldrow [lindex $matrix [lindex $index_list $i]]
	set newrow {}
	for {set j 0} {$j < $newsize} {incr j} {
	    lappend newrow [lindex $oldrow [lindex $index_list $j]]
	}
	lappend newmat $newrow
    }
    set newmat
}

############################################################################

proc ::hs::create_histogram {uid title category qlabel args} {
    set arglen [llength $args]
    set ndim [expr {$arglen / 4}]
    set remainder [expr {$arglen % 4}]
    if {$remainder != 0 || $ndim < 1 || $ndim > 3} {
	error "wrong # of arguments"
    }
    switch $ndim {
	1 {
	    foreach {xlabel xmin xmax xbins} $args {}
	    set id [hs::create_1d_hist $uid $title $category\
		    $xlabel $qlabel $xbins $xmin $xmax]
	}
	2 {
	    foreach {xlabel xmin xmax xbins ylabel ymin ymax ybins} $args {}
	    set id [hs::create_2d_hist $uid $title $category $xlabel\
		    $ylabel $qlabel $xbins $ybins $xmin $xmax $ymin $ymax]
	}
	3 {
	    foreach {xlabel xmin xmax xbins ylabel ymin\
		    ymax ybins zlabel zmin zmax zbins} $args {}
	    set id [hs::create_3d_hist $uid $title $category $xlabel\
		    $ylabel $zlabel $qlabel $xbins $ybins $zbins\
		    $xmin $xmax $ymin $ymax $zmin $zmax]
	}
	default {
	    error "Incomplete switch statement in [lindex [info level 0] 0].\
		    This is a bug. Please report."
	}
    }
    set id
}

############################################################################

proc ::hs::Comm_debug {onoff} {
    if {$onoff} {
	if {[llength [info commands ::hs::Load_config_string]] == 0} {
	    ::rename ::hs::load_config_string ::hs::Load_config_string
	    proc ::hs::load_config_string {message} {
		puts "Sending config string (here in quotes):"
		puts "\"$message\""
		hs::Load_config_string $message
		return
	    }
	}
    } else {
	if {[llength [info commands ::hs::Load_config_string]] > 0} {
	    ::rename ::hs::load_config_string {}
	    ::rename ::hs::Load_config_string ::hs::load_config_string
	}
    }
    return
}

############################################################################

proc ::fit::Fit_multires_sequence {subset id_data id_fit \
                 id_fft shift smoothing_curves rangelist} {
    set item_id [fit::Fit_subset $subset cget -id]
    set item_type [hs::type $item_id]
    hs::allow_item_send 0

    # Make sure temporary histograms are deleted
    global ::errorInfo
    set status [catch {
        # Extract the fitted data
        if {[string equal $item_type "HS_1D_HISTOGRAM"]} {
            hs::Copy_1d_data_with_offset $item_id $id_data 4 0
        } elseif {[string equal $item_type "HS_NTUPLE"]} {
            set column_number [lindex [fit::Fit_subset $subset cget -columns] 0]
            hs::reset $id_data
            hs::Project_ntuple_onto_1d_hist $item_id $column_number $id_data 1.0
        } else {
            error "You have reached unreachable code\
            in [lindex [info level 0] 0].\
            This is a bug. Please report."
        }

        # Extract the fit values
        fit::Fit_subset $subset plotfit $id_fit

        # Subtract the two, normalizing to unit area
        set fitnorm [hs::hist_l1_norm $id_fit]
        if {$fitnorm > 0.0} {
            set fitnorm [expr {1.0/$fitnorm}]
        }
        if {[string equal [::fit::Fit_cget -method] "eml"]} {
            # Include the difference in normalizations in the test
            if {$fitnorm > 0.0} {
                set datanorm [expr {-$fitnorm}]
            } else {
                set datanorm [hs::hist_l1_norm $id_data]
            }
        } else {
            set datanorm [hs::hist_l1_norm $id_data]
            if {$datanorm > 0.0} {
                set datanorm [expr {-1.0/$datanorm}]
            }
        }
        set diff_id [hs::sum_histograms [hs::Temp_uid] "Fit difference" \
                         [hs::Temp_category] $id_data $id_fit $datanorm $fitnorm]

        # Apply the filter mask if necessary
        if {![string equal [fit::Fit_subset $subset cget -filter] ""]} {
            hs::Apply_1d_range_masks $diff_id $rangelist
        }

        # Embed the difference into the FFT histogram
        hs::Copy_1d_data_with_offset $diff_id $id_fft 1 $shift
        hs::delete $diff_id

        # Fourier transform
        hs::1d_fft $id_fft $id_fft 1

        # Cycle over the smoothing curves and get the list of differences
        set normlist [fit::Fit_multires_fast_cycle $id_fft $smoothing_curves]
    } errMess]
    set savedInfo $::errorInfo
    hs::allow_item_send 1
    if {$status} {
	error $errMes $savedInfo
    }
    set normlist
}

############################################################################

if {[::hs::have_cernlib]} {
    proc ::hs::uniform_random_fill {args} {
        set known_switches {-weight -xmin -xmax -ymin -ymax -zmin -zmax}
        set arglist [hs::Parse_single_arg_switches $args $known_switches options]
        set arglen [llength $arglist]
        if {$arglen != 2} {
            error "wrong # of arguments"
        }
        foreach {id npoints} $arglist {}
        if {[info exists options(-weight)]} {
            set weight $options(-weight)
        } else {
            set weight 1.0
        }
        foreach optname {xmin xmax ymin ymax zmin zmax} {
            if {[info exists options(-$optname)]} {
                set $optname $options(-$optname)
            } else {
                set $optname ""
            }
        }
        hs::Uniform_random_fill $id $npoints $weight \
            $xmin $xmax $ymin $ymax $zmin $zmax
        return
    }
}

############################################################################

proc ::fit::Fit_multires_test {mcsamples {show_progress {}} {nbwpoints {}}} {
    if {[string equal $show_progress ""]} {
        set show_progress 0
    }
    if {[string equal $nbwpoints ""]} {
        variable Default_bandwidth_set_size
        set nbwpoints $Default_bandwidth_set_size
    }
    if {![fit::Fit_cget -complete]} {
        error "Fitting is not complete"
    }
    if {![string is integer -strict $mcsamples]} {
        error "Expected a positive integer, got \"$mcsamples\""
    }
    if {$mcsamples <= 0} {
        error "Expected a positive integer, got $mcsamples"
    }
    if {![string is boolean -strict $show_progress]} {
        error "Expected a boolean, got \"$show_progress\""
    }
    if {![string is integer -strict $nbwpoints]} {
        error "Expected a positive integer, got \"$nbwpoints\""
    }
    if {$nbwpoints <= 1} {
        error "Expected an integer larger than 1, got $nbwpoints"
    }
    set subset_list [fit::Fit_subset list]
    if {[llength $subset_list] != 1} {
	error "This test can only be performed on single-dataset fits"
    }
    set subset [lindex $subset_list 0]
    set ndim [fit::Fit_subset $subset cget -ndim]
    if {$ndim != 1} {
	error "This test can only be performed on fits to 1d datasets"
    }
    set item_id [fit::Fit_subset $subset cget -id]
    set item_type [hs::type $item_id]
    if {[string equal $item_type "HS_NONE"]} {
        error "Histo-Scope item with id $item_id no longer exists"
    }
    if {[fit::Fit_subset $subset cget -binned]} {
        if {[string equal $item_type "HS_1D_HISTOGRAM"]} {
            set nbins [hs::1d_hist_num_bins $item_id]
            foreach {xmin xmax} [hs::1d_hist_range $item_id] {}
        } else {
            error "Can not test binned fits of ntuple data"
        }
        set binwidth [expr {($xmax - $xmin)/1.0/$nbins}]
        set l1norm [hs::hist_l1_norm $item_id]
        set datapoints [expr {round($l1norm/$binwidth)}]
        if {$datapoints < 1} {
            error "Can't determine the number of data points"
        }
    } elseif {[string equal $item_type "HS_NTUPLE"]} {
        set normregion [fit::Fit_subset $subset cget -normregion]
        if {[llength $normregion] == 0} {
            error "Normalization region is not defined for dataset $subset"
        }
        foreach {xmin xmax nbins} $normregion {}
        if {$nbins <= 0 || $xmin >= $xmax} {
            error "Bad normalization region for dataset $subset"
        }
        set binwidth [expr {($xmax - $xmin)/1.0/$nbins}]
        set datapoints [fit::Fit_subset $subset cget -points]
    } else {
        error "You have reached unreachable code\
            in [lindex [info level 0] 0].\
            This is a bug. Please report."
    }

    # Figure out the appropriate number of FFT bins
    # for the convolution histograms. First, check if
    # the number of bins is an exact power of 2.
    set exact 1
    set tmp $nbins
    while {$tmp > 1} {
        if [expr {$tmp % 2}] {
            set exact 0
            break
        }
        set tmp [expr {$tmp / 2}]
    }
    
    # We will use 4 bins of the FFT curve per 1 bin
    # of the original histogram or normalization region.
    # The range of the FFT curve will be 4 times wider
    # than the range of the original data.
    if {$exact} {
        set fft_bins [expr {$nbins * 16}]
    } else {
        set next_power_of_2 [expr {int(log($nbins)/log(2.0)) + 1}]
        set fft_bins [expr {1 << ($next_power_of_2 + 4)}]
    }
    
    # Now, figure out the shift which will position the original
    # histogram in the center of the FFT histogram. Also, set the
    # FFT histogram limits.
    set shift [expr ($fft_bins - $nbins * 4)/2]
    set fft_binwidth [expr {$binwidth/4.0}]
    set fft_xmin [expr {$xmin - $shift * $fft_binwidth}]
    set fft_xmax [expr {$xmax + $shift * $fft_binwidth}]
    
    # Calculate the width set
    set minwidth $fft_binwidth
    set maxwidth [expr {($xmax - $xmin)/2.0}]
    set logfactor [expr {log($maxwidth/$minwidth)/($nbwpoints-1)}]
    set width_list {}
    lappend width_list $minwidth
    for {set i 1} {$i < [expr {$nbwpoints-1}]} {incr i} {
        lappend width_list [expr {exp(log($minwidth) + $i*$logfactor)}]
    }
    lappend width_list $maxwidth
    
    # Make sure temporary histos are deleted 
    # even if we encounter an error
    set temp_histos {}
    global ::errorInfo
    set status [catch {
        # For each width, calculate FFT of the smoothing curve
        set smoothing_curves {}
        foreach bw $width_list {
            set id [hs::create_1d_hist [hs::Temp_uid] \
                        "Smoothing spectrum for bw $bw" \
                        [hs::Temp_category] "X" "Y" $fft_bins \
                        $fft_xmin $fft_xmax]
            hs::Periodic_gauss $id 1.0 $bw
            hs::1d_fft $id $id 1
            lappend smoothing_curves $id
            lappend temp_histos $id
        }

        # Get the exclusion regions
        set rangelist {}
        if {![string equal [fit::Fit_subset $subset cget -filter] ""]} {
            foreach range [fit::Fit_subset $subset ranges \
                               [list $xmin $xmax [expr {$nbins * 4}]]] {
                lappend rangelist [lrange $range 0 1]
            }
        }

        # Prepare histograms for extracting fit and data values
        set id_data [hs::create_1d_hist [hs::Temp_uid] "Fit data" \
                         [hs::Temp_category] "X" "Y" \
                         [expr {$nbins * 4}] $xmin $xmax]
        lappend temp_histos $id_data
        set id_fit [hs::create_1d_hist [hs::Temp_uid] "Fit values" \
                        [hs::Temp_category] "X" "Y" \
                        [expr {$nbins * 4}] $xmin $xmax]
        lappend temp_histos $id_fit

        # Prepare the histogram used for FFT
        set id_fft [hs::create_1d_hist [hs::Temp_uid] \
                        "FFT fit difference" \
                        [hs::Temp_category] "X" "Y" \
                        $fft_bins $fft_xmin $fft_xmax]
        lappend temp_histos $id_fft

        # Create the ntuple for saving the results of
        # pseudo experiments
        set varlist {}
        for {set i 0} {$i < $nbwpoints} {incr i} {
            lappend varlist "v$i"
        }
        set ntuple_id [hs::create_ntuple [hs::Temp_uid] \
                "Bw ntuple" [hs::Temp_category] $varlist]
        lappend temp_histos $ntuple_id

        # Get the list of norms for the original fit
        set normlist_data [fit::Fit_multires_sequence $subset $id_data \
                    $id_fit $id_fft $shift $smoothing_curves $rangelist]
        hs::fill_ntuple $ntuple_id $normlist_data

        # Copy the fit and create new data items
        set thisfit [fit::Fit_get_active]
        set copyfit [fit::Fit_next_name]
        set procname ::$copyfit
        while {[lsearch -exact [info commands $procname] $procname] >= 0} {
            set copyfit [fit::Fit_next_name]
            set procname ::$copyfit
        }
        fit::Fit_copy $thisfit $copyfit
        set idlist [fit::Fit_copy_data $copyfit "Temporary MC fit category"]
        if {[llength $idlist] != 1} {
            error "Failed to copy fit data items"
        }
        set copy_id [lindex $idlist 0]
        fit::Fit_callback $copyfit add destruct "hs::delete $copy_id ;#"
        proc $procname {args} "eval fit::Fit_process [list $copyfit] \$args"
        fit::Fit_activate $copyfit
        fit::Fit_config -verbose -1 -minos off

        # Carry out the pseudo experiments
        global ::errorInfo
        set status [catch {
            for {set i 0} {$i < $mcsamples} {incr i} {
                hs::reset $copy_id
                fit::Fit_activate $thisfit
                fit::Fit_subset $subset random $datapoints $copy_id
                fit::Fit_activate $copyfit
                fit::Fit_fit
                set normlist_mc [fit::Fit_multires_sequence $subset $id_data \
                          $id_fit $id_fft $shift $smoothing_curves $rangelist]
                hs::fill_ntuple $ntuple_id $normlist_mc
                if {$show_progress} {
                    puts -nonewline " $i"
                    flush stdout
                }
            }
            if {$show_progress} {
                puts ""
                flush stdout
            }
        } ermess]
        set savedInfo $::errorInfo
        catch {
            fit::Fit_multires_fast_cycle 0 {}
            $procname delete
            fit::Fit_activate $thisfit
        }
        if {$status} {
            error $ermess $savedInfo
        }

        # Compare the norms from pseudo experiments
        # to the norms in the original fit
        foreach {cl bwindex} [fit::Analyze_multires_ntuple $ntuple_id] {}
        set worst_width [lindex $width_list $bwindex]
    } errMes]
    set savedInfo $::errorInfo
    foreach id $temp_histos {
        catch {hs::delete $id}
    }
    if {$status} {
	error $errMes $savedInfo
    }
    list $cl $worst_width
}

############################################################################

proc ::hs::tcl_command_template {command filename} {
    # Generate a safe name for the C function
    set cfunction "c_"
    regsub -all "\\W" $command _ newstring
    append cfunction $newstring
    if {[string equal [string index $cfunction end] "_"]} {
        append cfunction "c"
    }

    # Open the output file and write standard headers
    set chan [open $filename "w"]
    hs::Write_standard_c_headers $chan
    puts $chan "#include \"tcl.h\""
    variable Histoscope_header
    puts $chan "#include \"$Histoscope_header\""

    # Template code
    set boilerplate {
static int CFUNCTION(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static Tcl_Interp *commandInterp = 0;

#define tcl_require_objc(N) do {\\
  if (objc != N)\\
  {\\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\\
                       " : wrong # of arguments", NULL);\\
      return TCL_ERROR;\\
  }\\
} while(0);

#define tcl_objc_range(N,M) do {\\
  if (objc < N || objc > M)\\
  {\\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\\
                       " : wrong # of arguments", NULL);\\
      return TCL_ERROR;\\
  }\\
} while(0);

#define verify_1d_histo(id,objnum) do {\\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\\
	return TCL_ERROR;\\
    if (hs_type(id) != HS_1D_HISTOGRAM)\\
    {\\
	if (hs_type(id) == HS_NONE)\\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a valid Histo-Scope id", NULL);\\
	else\\
	    Tcl_AppendResult(interp, "item with id ",\\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a 1d histogram", NULL);\\
	return TCL_ERROR;\\
    }\\
} while(0);

#define verify_2d_histo(id,objnum) do {\\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\\
	return TCL_ERROR;\\
    if (hs_type(id) != HS_2D_HISTOGRAM)\\
    {\\
	if (hs_type(id) == HS_NONE)\\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a valid Histo-Scope id", NULL);\\
	else\\
	    Tcl_AppendResult(interp, "item with id ",\\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a 2d histogram", NULL);\\
	return TCL_ERROR;\\
    }\\
} while(0);

#define verify_3d_histo(id,objnum) do {\\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\\
	return TCL_ERROR;\\
    if (hs_type(id) != HS_3D_HISTOGRAM)\\
    {\\
	if (hs_type(id) == HS_NONE)\\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a valid Histo-Scope id", NULL);\\
	else\\
	    Tcl_AppendResult(interp, "item with id ",\\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a 3d histogram", NULL);\\
	return TCL_ERROR;\\
    }\\
} while(0);

#define verify_ntuple(id,objnum) do {\\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\\
	return TCL_ERROR;\\
    if (hs_type(id) != HS_NTUPLE)\\
    {\\
	if (hs_type(id) == HS_NONE)\\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not a valid Histo-Scope id", NULL);\\
	else\\
	    Tcl_AppendResult(interp, "item with id ",\\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\\
			     " is not an ntuple", NULL);\\
	return TCL_ERROR;\\
    }\\
} while(0);

#ifdef __cplusplus
extern "C" {
#endif

int _hs_init(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "COMMAND",
			     CFUNCTION,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;

    commandInterp = interp;
    return TCL_OK;
}

void _hs_fini(void)
{
    if (commandInterp)
    {
	Tcl_DeleteCommand(commandInterp, "COMMAND");
    }
}

#ifdef __cplusplus
}
#endif

static int CFUNCTION(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_AppendResult(interp, "command \"",
		     Tcl_GetStringFromObj(objv[0], NULL),
		     "\" is not implemented yet", NULL);
    return TCL_ERROR;
}
    }

    # Write out the template code
    puts $chan [string map [list COMMAND $command \
            CFUNCTION $cfunction "\\\\" "\\"] $boilerplate]

    # Write out the compilation instructions
    variable Sharedlib_suffix
    set soname [file rootname $filename]$Sharedlib_suffix
    if {![string equal [string index $soname 0] "/"]} {
        set soname [file join . $soname]
    }
    puts $chan "/*"
    puts $chan "  Compile and load this code using the following commands:"
    puts $chan ""
    puts $chan "  hs::sharedlib_compile $filename $soname"
    puts $chan "  set dlltoken \[hs::sharedlib open $soname\]"
    puts $chan ""
    puts $chan "  When you are done using this command, unload it with"
    puts $chan ""
    puts $chan "  hs::sharedlib close \$dlltoken"
    puts $chan "*/"

    close $chan
    return
}

############################################################################

if {[::hs::have_cernlib]} {
    proc ::hs::hist_function_chisq {id function_tag scan_variables args} {
        set hs_type [hs::type $id]
        if {![string match "HS_?D_HISTOGRAM" $hs_type]} {
            if {[string equal $hs_type "HS_NONE"]} {
                error "Histo-Scope item with id $id does not exist"
            } else {
                error "Histo-Scope item with id $id is not a histogram"
            }
        }
        set ndim [string index $hs_type 3]
        if {![hs::function $function_tag exists]} {
            error "\"$function_tag\" is not a valid function tag"
        }
        if {[llength $scan_variables] != $ndim} {
            error "Bad number of scan variables"
        }
        set errsta [hs::hist_error_status $id]
        if {$errsta <= 0} {
            error "Histogram with id $id has no errors"
        }
        hs::allow_item_send 0
        global ::errorInfo
        set stat [catch {
            set scan_id [hs::duplicate_axes $id [hs::Temp_uid] \
                    "Temp histo" [hs::Temp_category]]
            eval hs::function [list $function_tag] scan \
                    [list [concat $scan_id $scan_variables]] $args
            foreach {ndof chisq cl} [hs::Precise_reference_chisq $id $scan_id] {}
        } errira]
        set localError $::errorInfo
        catch {hs::delete $scan_id}
        hs::allow_item_send 1
        if {$stat} {
            error $errira $localError
        }
        list $cl $chisq $ndof
    }
}
    
############################################################################

proc hs::import_ntuple_data {infile ntuple_id {strict_columns 1}} {
    set nvars [hs::num_variables $ntuple_id]
    set nentries [hs::num_entries $ntuple_id]
    set chan [open $infile "r"]
    global ::errorInfo
    set status [catch {
        set linenum 0
        set data [list]
        while {[gets $chan line] >= 0} {
            incr linenum
            if {[regexp {^\s*($|\#)} $line]} {
                # This is an empty line or a comment
                continue
            }
            set nwords 0
            foreach num [regexp -inline -all -- {\S+} $line] {
                if {$nwords == $nvars} {
                    if {$strict_columns} {
                        error "Too many columns in line $linenum"
                    } else {
                        break
                    }
                } elseif {[string is double $num]} {
                    lappend data $num
                    incr nwords
                } else {
                    error "Not a number in line $linenum: \"$num\""
                }
            }
            if {$nwords < $nvars} {
                if {$strict_columns} {
                    error "Missing columns in line $linenum"
                } else {
                    while {$nwords < $nvars} {
                        lappend data 0
                        incr nwords
                    }
                }
            }
        }
        hs::fill_ntuple $ntuple_id $data
    } errmess]
    set localError $::errorInfo
    catch {close $chan}
    if {$status} {
        error $errmess $localError
    }
    expr {[hs::num_entries $ntuple_id] - $nentries}
}

############################################################################

proc ::hs::bin_number {id axis value} {
    set axis [string toupper $axis]
    switch [hs::type $id] {
        HS_1D_HISTOGRAM {
            if {[string equal $axis "X"]} {
                foreach {min max} [hs::1d_hist_range $id] {}
                set nbins [hs::1d_hist_num_bins $id]
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        HS_2D_HISTOGRAM {
            foreach {n_x_bins n_y_bins} [hs::2d_hist_num_bins $id] {}
            foreach {x_min x_max y_min y_max} [hs::2d_hist_range $id] {}
            if {[string equal $axis "X"]} {
                set nbins $n_x_bins
                set min $x_min
                set max $x_max
            } elseif {[string equal $axis "Y"]} {
                set nbins $n_y_bins
                set min $y_min
                set max $y_max
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        HS_3D_HISTOGRAM {
            foreach {n_x_bins n_y_bins n_z_bins} [hs::3d_hist_num_bins $id] {}
            foreach {x_min x_max y_min y_max z_min z_max} [hs::3d_hist_range $id] {}
            if {[string equal $axis "X"]} {
                set nbins $n_x_bins
                set min $x_min
                set max $x_max
            } elseif {[string equal $axis "Y"]} {
                set nbins $n_y_bins
                set min $y_min
                set max $y_max
            } elseif {[string equal $axis "Z"]} {
                set nbins $n_z_bins
                set min $z_min
                set max $z_max
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        default {
            error "Item with id $id is not a histogram"
        }
    }
    if {$value < $min} {
        return -1
    }
    # The following formula is consistent with Histo-Scope
    set ibin [expr {int((($value - $min)*$nbins)/($max - $min))}]
    if {$ibin < $nbins} {
        return $ibin
    } else {
        return -1
    }
}

############################################################################

proc ::hs::bin_coord {id axis number {left_edge 0}} {
    set axis [string toupper $axis]
    switch [hs::type $id] {
        HS_1D_HISTOGRAM {
            if {[string equal $axis "X"]} {
                foreach {min max} [hs::1d_hist_range $id] {}
                set nbins [hs::1d_hist_num_bins $id]
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        HS_2D_HISTOGRAM {
            foreach {n_x_bins n_y_bins} [hs::2d_hist_num_bins $id] {}
            foreach {x_min x_max y_min y_max} [hs::2d_hist_range $id] {}
            if {[string equal $axis "X"]} {
                set nbins $n_x_bins
                set min $x_min
                set max $x_max
            } elseif {[string equal $axis "Y"]} {
                set nbins $n_y_bins
                set min $y_min
                set max $y_max
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        HS_3D_HISTOGRAM {
            foreach {n_x_bins n_y_bins n_z_bins} [hs::3d_hist_num_bins $id] {}
            foreach {x_min x_max y_min y_max z_min z_max} [hs::3d_hist_range $id] {}
            if {[string equal $axis "X"]} {
                set nbins $n_x_bins
                set min $x_min
                set max $x_max
            } elseif {[string equal $axis "Y"]} {
                set nbins $n_y_bins
                set min $y_min
                set max $y_max
            } elseif {[string equal $axis "Z"]} {
                set nbins $n_z_bins
                set min $z_min
                set max $z_max
            } else {
                error "Invalid axis specification \"$axis\" for item with id $id"
            }
        }
        default {
            error "Item with id $id is not a histogram"
        }
    }
    if {$number < 0 || $number >= $nbins} {
        error "Bin number is out of range"
    }
    set bw [expr {($max - $min)/$nbins}]
    if {$left_edge} {
        return [expr {$min + $bw*$number}]
    } else {
        return [expr {$min + $bw*($number + 0.5)}]
    }
}
