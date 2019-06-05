
# Initialize Histo-Scope
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
histo_init
require_cernlib

# Create a function for a 2d Gaussian mixture. Clone the bivariate
# gaussian (since we have only one) and make the sum.
hs::function bivar_gauss copy bivar_gauss0
set gauss_sum [hs::function_sum 1 bivar_gauss 1 bivar_gauss0]

# Here is the list of parameters for the mixture
set parameters [list \
    {volume_0  1}    \
    {mean_x_0  4}    \
    {mean_y_0  4}    \
    {sigma_x_0 1}    \
    {sigma_y_0 1.5}  \
    {rho_0     0.7}  \
    {volume_1  0.5}  \
    {mean_x_1  6}    \
    {mean_y_1  7}    \
    {sigma_x_1 0.6}  \
    {sigma_y_1 0.8}  \
    {rho_1     -0.5} \
]

# Rename the function parameters for convenience
set parameter_names {}
foreach pair $parameters {
    foreach {name value} $pair {}
    lappend parameter_names $name
}
hs::function $gauss_sum configure -name "Sum of two 2d Gaussians" \
    -parameters $parameter_names

# Generate random numbers according to the 2d gaussian mixture. Direct
# generation of random numbers using arbitrary functions is supported
# only in 1d, so we will have to make a histogram first. Then random
# numbers will be generated according to that histogram.
set id1 [hs::create_2d_hist [next_uid] "2d Gaussian Mixture Function" \
           "Example category" X Y Z 100 100 0 10 0 10]

# Fill the histogram with function values. Note how the result
# specifier is protected with the additional "list" command.
eval hs::function $gauss_sum scan [list [list $id1 x y]] $parameters
hs::show_histogram $id1

# Create the histogram to hold the random values
set id2 [hs::create_2d_hist [next_uid] "Random 2d Gaussian Mixture"  \
           "Example category" X Y Z 50 50 0 10 0 10]

# Generate the random numbers and show the plot
hs::fill_histogram $id2 [hs::hist_random $id1 10000]
hs::show_histogram $id2

# Set gaussian errors for this histogram
hs::hist_set_gauss_errors $id2

# We will now create two histograms of simple 2d gaussians
# and will use them as templates to fit our mixture.
set id3 [hs::create_2d_hist [next_uid] "Template 0"  \
           "Example category" X Y Z 50 50 -5 5 -5 5]
hs::function bivar_gauss scan [list $id3 x y] {volume 1} \
    {mean_x 0} {mean_y 0} {sigma_x 1} {sigma_y 1} {rho 0}
set id4 [hs::copy_hist $id3 [next_uid] "Template 1" "Example category"]
hs::show_histogram $id3

# Turn template histograms into functions which can be used for fitting
hs::function data_2d copy data_2d0
hs::function data_2d config -mode $id3
hs::function data_2d0 config -mode $id4
set template_sum [hs::function_sum 1 data_2d 1 data_2d0]

# Here is the list of parameters for the template sum
# together with their initial values which will be used
# in the fit. Note that the fit starting point is quite
# far away from the generated mixture.
set parameters [list \
    x_scale_0 1 \
    y_scale_0 1 \
    x_shift_0 3 \
    y_shift_0 3 \
    angle_0   0 \
    v_scale_0 500 \
    v_shift_0 0 \
    x_scale_1 1 \
    y_scale_1 1 \
    x_shift_1 7 \
    y_shift_1 7 \
    angle_1   0 \
    v_scale_1 500 \
    v_shift_1 0 \
]

set parameter_names {}
foreach {name value} $parameters {
    lappend parameter_names $name
}
hs::function $template_sum configure -name "Sum of two templates" \
    -parameters $parameter_names

# We will use the least-squares fit in which errors are taken from
# the histogram error data. Note that in this case we need to assign
# some reasonable errors to 0-height bins. The following command sets
# minumum error value to 1.
hs::hist_set_error_range $id2 1.0 {}

# Create the fit
set myfit [hs::fit $id2 $template_sum -method "ls"]

# Set up initial parameter values and pin vertical template shifts at 0
foreach {name value} $parameters {
    $myfit parameter $name config -value $value
}
$myfit parameter v_shift_0 config -state fixed
$myfit parameter v_shift_1 config -state fixed

# Start the fit tuner and try to move the parameters
# so that the fit is close to the fitted distribution. 
# Then click on the "Fit" button of the tuner.
$myfit tune
