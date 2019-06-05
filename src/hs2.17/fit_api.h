#ifndef _FIT_API_H
#define _FIT_API_H

#include "simple_tcl_api.h"

/* Functions which do not act on any particular fit */
tcl_routine(Fit_list);
tcl_routine(Fit_get_active);
tcl_routine(Fit_fcn_set);
tcl_routine(Fit_fcn_get);
tcl_routine(Fit_next_name);
tcl_routine(Fit_method_list);
tcl_routine(Fit_lock_minuit);

/* Functions which need the name of the fit as one of the arguments */
tcl_routine(Fit_activate);
tcl_routine(Fit_create);
tcl_routine(Fit_copy);
tcl_routine(Fit_copy_data);
tcl_routine(Fit_rename);
tcl_routine(Fit_callback);
tcl_routine(Fit_tcl_fcn);

/* Functions which can take the name of the fit as
   an argument but use the active fit as the default */
tcl_routine(Fit_exists);
tcl_routine(Fit_info);

/* Functions which act on the currently active fit */
tcl_routine(Fit_subset);
tcl_routine(Fit_parameter);
tcl_routine(Fit_function);
tcl_routine(Fit_config);
tcl_routine(Fit_cget);
tcl_routine(Fit_tcldata);
tcl_routine(Fit_append_dll);
tcl_routine(Fit_reset);
tcl_routine(Fit_apply_mapping);
tcl_routine(Fit_tcl_fcn_args);
tcl_routine(Fit_has_tcl_fcn);

/* Helper functions */
tcl_routine(Parse_minuit_pars_in_a_map);
tcl_routine(Parse_fitter_pars_in_a_map);
tcl_routine(Analyze_multires_ntuple);
tcl_routine(Fit_multires_fast_cycle);

#endif /* _FIT_API_H */

/*
Fit API description
-------------------

fit::Fit_list

     Arguments : none
     Returns   : list of strings

     Returns the list of names for currently defined fits.

fit::Fit_get_active

     Arguments : none
     Returns   : string

     Returns the name of the active fit, or an empty string when there is
     no active fit.

fit::Fit_fcn_set $fcn_id

     Arguments : string fcn_id
     Returns   : nothing

     Sets the function which will be minimized by Minuit (FCN) using its
     identification string. Right now, there is only one such function,
     and it is identified by string "generic".

fit::Fit_fcn_get

     Arguments : none
     Returns   : string

     Returns the identification string of the function minimized by Minuit.

fit::Fit_next_name

     Arguments : none
     Returns   : string

     Returns an unused fit name which can be used with a subsequent
     fit::Fit_create command.

fit::Fit_method_list

     Arguments : none
     Returns   : list of strings

     Returns the complete list of tags for supported minimization methods.

fit::Fit_lock_minuit ?lock?

     Arguments : optional boolean argument lock
     Returns   : boolean

     When called without the "lock" argument, returns 1 if Minuit is locked,
     otherwise returns 0. When called with the "lock" argument, attempts to
     lock or unlock Minuit. In this case the command returns 1 on success
     and 0 on failure. When Minuit is locked, the Minuit main loop assumes
     that it is safe to process tcl events, and does so. Also, certain
     commands become disabled (for example, it becomes impossible to
     change anything in the active fit).

fit::Fit_activate $name

     Arguments : string name
     Returns   : nothing

     Activates the fit named $name, making it "current". The previous
     fit looses synchronization with Minuit, and its "complete" status
     is changed to "false" which may trigger some callbacks (please
     see the fit::Fit_callback command description for details).

fit::Fit_create $name data1? data2? ... opt1? value1? ...

     Arguments : string name, mixed lists data1, data2, ..., various options
                 are provided as strings
     Returns   : string

     Creates a fit named $name. $name must not be a name of an existing fit.
     If $name is specified as an empty string then a unique fit name will be
     generated automatically. Each dataset specifier data1, data2, ... is
     itself a list. The first element of this list must be a Histo-Scope id
     of a histogram or an ntuple, and the rest are switches. The following
     switches may be used in dataset specifiers which refer to either
     histograms or ntuples:

     -weight $W     Specifies the dataset weight in the fit ($W should be
                    a non-negative double). The default weight is 1.0.

     -method $Mn    Specifies the fitting method for this dataset. Possible
                    Mn values are:
		       "L2"    -- L2 distance
		       "ls"    -- Least Squares
		       "chisq" -- Chi-Square Minimization
		       "ml"    -- Maximum Likelihood
                       "eml"   -- Extended Maximum Likelihood
		    If this switch is not provided then the default method
                    of this fit will be used for this dataset. Note that
		    unbinned fits may be performed only with the maximum
		    likelihood methods.

     Dataset specifiers which refer to ntuples may use the following additional
     switches: -x, -y, -z, -v, and -e. Values provided after these switches
     specify ntuple column names used to define x coordinates of the
     data points, y coordinates, z coordinates, data values, and errors,
     respectively.

     The fit::Fit_create command recognizes the following options:

     -title $T          Specifies the fit title which will be used in various
                        printouts. Minuit printouts can only use the first
			50 characters of the title string.

     -description $D    Specifies the fit description (an arbitrary string).
                        The default D value is an empty string.

     -method $M         Sets the default fitting method which will be
                        assigned to every dataset which does not explicitly
			specify its fitting method. The alloved M values
			are "ls", "L2", "chisq", "ml", and "eml". They have
			the same meanings as for the dataset-specific "-method"
			switch. The default M value is "ls".

     -warnings $Warn    If set to "false", instructs Minuit to suppress
                        warning messages about suspicious conditions which
			may indicate unreliable results. The default is
			to print such messages.

     -status $Stat      Specifies the fit status. $Stat must be either "ok"
                        or "error". The default value is "ok". This option
			affects the internal behavior of the fitting code,
			and should not normally be set by the application.

     -gradient $G       $G is a boolean value which specifies whether Minuit
                        should use an external procedure to calculate
			the function gradient over parameters. Right now,
			this option should be left at its default value of 0.

     -ignore $Ign       If $Ign is set "true", the program will not print
                        error messages generated by fitting functions.
			The default Ign value is "false".

     -errdef $UP        Sets Minuit value of UP defining parameter errors.
                        Minuit defines parameter errors as the change in
                        the parameter value required to change the function
			value by $UP. Unless you know exactly what you are
                        doing, leave this option at its default value of 1.0.

     -precision $Eps    Informs Minuit that the relative floating point
                        arithmetic precision is $Eps. The default value
                        of this option is 0.0 which means that the program
                        will use the system default for doubles.

     -strategy $Stra    Sets the Minuit strategy to be used in calculating
                        first and second derivatives and in certain
			minimization methods. Low values of $Stra mean fewer
			function calls and high values mean more reliable
			minimization. Currently allowed values are
			0, 1 (default), and 2.

     -minimizer $Min    Sets the Minuit minimization method. $Min must be
                        one of the keywords "migrad", "mini", or "simplex".

     -minos $Use        If $Use is false, instructs the program NOT to run
                        MINOS after the minimizer. The default is "true".

     -verbose $Level    Minuit verbose level:
			   -1 -- no output except from SHOW commands
			    0 -- minimum output (no starting values
                                    or intermediate results)
			    1 -- Minuit "normal" output
			    2 -- print intermediate results
			    3 -- show the progress of minimization
                        The default value of this option is 0.

     -timeout $sec      Timeout in seconds for the Minuit main loop. 0 or
                        negative values mean no timeout will be set (this
			is the default). Note that timeout does not immediately
			stop the main loop since Minuit is not capable of
			receiving status information from the fitted function.
			Rather, all function values returned to Minuit after
			timeout will be set to 0. This works well if Minuit
			is using MIGRAD as its fitting method (the "plateau"
                        is quickly recognized and the minimization is stopped),
                        but SIMPLEX may just go into an infinite loop.

     The command returns the name of the created fit (normally just $name)
     which can be subsequently used with such commands as fit::Fit_activate,
     fit::Fit_copy, and fit::Fit_rename.

     Examples
     --------
     Prepare a fit of a histogram with id 1:
     hs::Fit_create f1 1 -method chisq -title "Fitting a histogram"

     Unbinned fit of an ntuple column:
     hs::Fit_create f2 {10 -x var1 -method ml} -title "Unbinned fit"

fit::Fit_copy $old_name new_name?

     Arguments : strings old_name and, optionally, new_name
     Returns   : string

     Creates a fit named $new_name as a copy of a fit named $old_name.
     $new_name must not be a name of an existing fit. If $new_name is
     specified as an empty string or omitted then a unique fit name
     will be generated automatically. Returns the name of the copy.

fit::Fit_copy_data $name $category

     Arguments : strings name and category
     Returns   : list of integers

     Makes copies of all data items in the fit named $name and associates
     copies with fit subsets instead of the original Histo-Scope items.
     The new items are placed in the category specified. Returns the list
     of ids of copied items.

fit::Fit_rename $old_name $new_name

     Arguments : strings old_name and new_name
     Returns   : string

     Changes the name of a fit from $old_name into $new_name or deletes
     the fit in case $new_name is an empty string. Returns the new fit name.

fit::Fit_callback $name $subcommand $cbtype ?script?

     Arguments : strings subcommand, cbtype, and script
     Returns   : depends on the given subcommand

     This command may be used to set up callbacks for the named fit.
     Callbacks are tcl scripts called when the fit state changes in
     a certain way. The following command forms may be used:

   * fit::Fit_callback $name add $cbtype $script

     Adds a callback to fit $name. One of the following keywords must be
     used as the "cbtype" argument: "lostsync", "complete", and "delete".
     The callback scripts are triggered under the following circumstances:

     lostsync -- when the fit structure looses its synchronization with Minuit
     complete -- when the fit converges to a stable minimum
     destruct -- just before the fit structure is deleted

     The callback scripts will be executed in the global context with two
     additional arguments appended: name and cbtype. The "lostsync" and
     "complete" callbacks are guaranteed to be called when the fit is active.
     Any changes made by the callback scripts to the list of callback scripts
     of the same type will become visible only at the time of the next
     trigger. Note that when a fit copy is made, the callbacks are _not_
     automatically transferred to the copy.

   * fit::Fit_callback $name delete $cbtype $script

     Removes the script $script of type $cbtype (if exists) from the list of
     callbacks which belong to the fit identified by $name. Returns 1 if
     a callback is removed and 0 if not.

   * fit::Fit_callback $name list $cbtype

     Returns the list of all callbacks of the given type.

   * fit::Fit_callback $name clear $cbtype

     Deletes all callbacks of the given type.

fit::Fit_tcl_fcn $name $fcn $dll

     Arguments : string name
     Returns   : nothing

     Associates the user tcl fcn from the shared library with given id with
     the fit identified by name.

fit::Fit_exists name?

     Arguments : string name
     Returns   : boolean

     Returns 1 if the fit named $name exists, 0 otherwise. If the name
     argument is omitted then the command returns 1 in case there is
     an active fit, and 0 otherwise.

fit::Fit_info name?

     Arguments : string name
     Returns   : mixed list with 16 elements

     Returns the description of a fit named $name in a list. If the name
     argument is omitted then the command returns the description of the
     active fit. The returned list has the following structure:

     {title description default_fit_method minimizer minos minuit_options \
      global_status timeout ignore_function_errors minimization_status \
      parameters error_matrix_pars error_matrix_data dlls datasets \
      fit_functions}

     The list elements are:

     title                  -- String. Fit title (note, not name).
     description            -- String. Fit description.
     default_fit_method     -- String. Default fitting method which will be
                               assigned to every dataset which does not
                               explicitly specify its fitting method.
     minimizer              -- String. Minuit minimizer to use.
     minos                  -- Boolean. It will be 1 if the fitting procedure
                               uses MINOS or 0 if MINOS is skipped.
     minuit_options         -- List {errdef verbose warn strategy eps grad}
     global_status          -- String "ok" or "error".
     timeout                -- Integer. 0 or negative means no timeout.
     ignore_function_errors -- Boolean.
     minimization_status    -- List {fmin fedm errdef npari nparx istat}.
                               See the description of the Minuit subroutine
                               MNSTAT for the meaning of the list elements.
                               This will be an empty list if the minimization
			       has not been performed yet after the last
			       configuration change.
     parameters             -- The list of parameters. Each element is itself
                               a list {name value state step bounds}.
			       state is a string which is either "fixed" or
			       "variable". bounds is either an empty list
			       (unbounded) or a two-element list {min max}.
     error_matrix_pars      -- The list of names of parameters for which
                               the error matrix is defined (or an empty list
			       if the fit is not performed).
     error_matrix_data      -- Error matrix, each row (or column since it
                               is symmetric) is itself a list (or an empty list
			       if the fit is not performed).
     dlls                   -- Dll numbers to unload when the fit is deleted.
     datasets               -- List of dataset info lists. Please see the
                               description of the command fit::Fit_subset,
                               option info, for details about the dataset info
                               list structure.
     fit_functions          -- List of fitting function info lists. Please
                               see the description of the command
			       fit::Fit_function, option info, for details
			       about the function info list structure.

fit::Fit_subset opt1? value1? ...

     Arguments : various options supplied as strings
     Returns   : depends on the option given

     This command manipulates datasets of the active fit. The following
     forms can be used:

   * fit::Fit_subset add $dataset_specifier opt? value? ...

     Adds a new dataset to the active fit and returns the dataset id
     (an integer). dataset_specifier is a list which specifies Histo-Scope
     histogram or ntuple to fit. It has the same structure as in the
     fit::Fit_create command. The following options are supported:

     -filter $filter_code    Specifies the filter expression (in C notation)
                             which will be applied to all dataset points
                             before they are fitted. The filter code can only
                             depend on the data point coordinates but not on
                             the point values. It can use variables x, y, 
                             and z, and must result in an integer value.
                             A point will be accepted if the result of
                             the filter expression on the point coordinates
                             is not 0. For example, the expression
			     {x < 1 || x > 2} defines an exclusion region
			     [1, 2]. The C code generated from such filter
			     expression looks like this:
			     int filter(double x, double y, double z)
			     {
			         return ( x < 1 || x > 2 );
			     }
			     The default filter_code value is an empty string
			     which means that all data points will be used
			     in the fit. You can use all functions defined
			     in the standard C headers inside your filter
			     expression.

     -functions $fun_list    Specifies the list of tags of the functions
                             which will be used to fit the dataset. All
                             these functions will be summed in the fit.

     -normregion $descr      Specifies the region to use for calculation
                             of the fit normalization. This option may be
                             used with unbinned fits only. The descr argument
                             is a flat list which contains 3 numbers for
                             every dimension of the dataset: min, max, and
			     number of intervals to use. For example, this
			     is how $descr may look like for a 2d fit:
			     {-5.0 5.0 50 -10.0 10.0 100}. $descr may be
			     an empty list in which case the normalization
			     region becomes undefined.

   * fit::Fit_subset count

     Returns the upper limit for the dataset ids in the active fit.
     Note that this number is not necessarily equal to the total number
     of datasets since their ids are not required to be contiguous.

   * fit::Fit_subset list

     Returns the list of ids of datasets used in the active fit.

   * fit::Fit_subset $id $subcommand opt1? value1? ...

     Manipulates the dataset identified by $id in the active fit.
     The following subcommands are supported:

     exists                      Returns 1 if dataset identified by $id
                                 exists in the active fit, 0 otherwise.

     delete                      Removes the dataset identified by $id
                                 from the active fit. Also, removes
                                 functions which were used to fit just
                                 this particular dataset (note that in
                                 such a case fit parameters may have to be
                                 redefined).

     info                        Returns the information about the dataset
                                 identified by $id in the form of a list
                                 {id ndim binned columns filter_string \
				 functions weight method points normregion}
                                    id      -- Histo-Scope id of the data item
                                  ndim      -- dataset dimensionality
                                  binned    -- set to 1 for binned datasets,
                                               to 0 for unbinned
				  columns   -- five-element list of ntuple
                                               column numbers used to define
                                               x, y, z, data value, and
					       errors. For histograms, all
                                               these numbers are set to -1.
                                  filter_string  -- dataset filter expression
                                  functions -- the set of fitting functions
                                  weight    -- the dataset weight in the
                                               statistic being minimized
                                  method    -- fit method to use for this
                                               dataset. Please see the
                                               description of fit::Fit_create
                                               command for the list of valid
                                               fit methods.
				  points    -- the number of points used to
                                               calculate the minimization
                                               statistics during the most
                                               recent FCN call.
				  normregion  -- normalization region for
				               an unbinned dataset (or an
                                               empty list if undefined).

     stats $what $quantities      Returns basic statistical information
                                  about the data or fit. The "what"
				  argument must be either "data" or "fit".
				  The "quantities" argument is a a list
				  which may contain one or more of the
				  following keywords: npoints, is_pdf, sum,
				  mean_x, mean_y, mean_z, s_x, s_y, s_z,
				  rho_xy, rho_xz, rho_yz. If the fit has been
				  completed then the result will be a list
				  of values with the same number of elements
				  and in the requested order. If the fit has
				  not been completed, the command generates
				  a tcl error. The fit stats are not calculated
                                  in case an unbinned fit is done using the
                                  "ml" method because in this case we don't
				  know the support of the fitting function.
				  The keywords have the following meaning:

                                  npoints -- Number of points used in the fit
                                             (that is, points which passed the
                                             filter and did not cause any
                                             function errors). For data, this
                                             is the same as the result of
                                             "Fit_subset $num cget -npoints"
					     command. For the fit, this may
                                             depend on the grid used for
					     function normalization (this will
					     be set to 0 in case of unbinned
					     "ml" fit).
                                  is_pdf  -- This is set to 1 if all values
                                             are non-negative.
				  sum     -- The total number of events. In the
                                             binned case, this is just the sum
					     of bin values (for the fit it uses
					     Simpson's integration rule in
					     which bins are taken to be
					     integration intervals). In the
					     unbinned case, for the data this
					     is just the total number of points
					     used in the fit. For the unbinned
					     "eml" fit, this is the fit area
					     covered by the grid of points used
					     in the fit. For the unbinned "ml"
					     fit it is undefined.

				  All other keywords are self-explanatory.
                                  The values corresponding to these keywords
                                  will make sense only if npoints > 0,
				  is_pdf == 1, and the dimensionality of the
				  fit is sufficient for creation of relevant
				  statistics.

     configure opt? value? ...    Sets the dataset attributes. Available
                                  options are -filter, -functions, -method,
                                  -normregion, and -weight. The meaning of
				  these options is the same as in the
				  "fit::Fit_create" and "fit::Fit_subset add"
				  commands.

     cget $option                 Returns the dataset property identified
                                  by $option. In addition to the standard
                                  configurable options -filter, -functions,
                                  -method, -normregion, and -weight, the
				  following options are available read-only:
				  -id, -ndim, -binned, -columns, -points.
				  The values returned for these options are
				  the same as the corresponding elements in
				  the list produced by the "info" subcommand.
				  Also, there are several special options which
				  are not directly related to the dataset
				  definition: -validmethods, -ndof, and -chisq.
				  Option -validmethods may be used to return
				  the list of all fitting methods compatible
				  with this dataset. Options -ndof, and -chisq
				  return the number of points used in the
				  chi-square test and the chi-square value for
				  the fitted data. Note that these options are
				  only meaningful when the fit is complete and
				  the dataset is binned, otherwise their
				  return values are 0.

     compiledfilter dll? proc?    When issued without the dll and proc
                                  arguments, this subcommand returns 1 in case
				  the filter expression has already been
                                  compiled and 0 otherwise. Optional dll and
                                  proc arguments can be used to set the dll
                                  number and the C function name to use for
				  the filtering functions. The proc argument
				  may be specified as an empty string in
				  which case the filter is removed (all point
				  will be used in the fit). It should not be
				  necessary for a user code to use this
				  subcommand.

     ranges $scan_specifier       Scans filter functions of 1-dimensional
                                  datasets and returns the list of allowed
                                  regions in the form {{min1 max1 npoints1} \
                                  {min2 max2 npoints2} ...}. The scan_specifier
                                  argument is a three-element list
				  {min max npoints} which defines the region
				  to scan and the density of trial points.

     plotfit $result_specifier    Fills a Histo-Scope item with fitted values.
                                  $result_specifier is just an id for
				  a histogram or it must look like this
				  for ntuple: {id {min max npoints}}
				  in 1d case or {id {min1 max1 npoints1} \
				  {min2 max2 npoints2}} in 2d case.

     kstest npoints?              Returns 3-element list {CL D npoints_used}
                                  where CL is the confidence level of the
                                  "classic" Kolmogorov-Smirnov test, D is the
                                  maximum distance (used as a statistic in the
                                  test) and npoints_used is the number of
                                  points used in the CL calculation. The fit
                                  must be complete. $npoints is the number of
                                  data points. When $npoints is 0 or omitted,
                                  the program will figure out the number of
                                  data points automatically. Note that it may
                                  do so incorrectly if the fitted histogram
                                  has been scaled from its original form. If
                                  you provide the number of points, be careful
                                  to apply the dataset filter when counting
                                  them.

     fitvalues $uid $title \      This command calculates the fitted values
     $category $is_difference     at the coordinates of the data points.
                                  "is_difference" is a boolean argument which
                                  specifies if the program should calculate
                                  the difference between the data and the fit
                                  (when "true") or just calculate the fit.
				  The command will create the same kind
                                  of Histo-Scope item as the one used
                                  to represent the data: an ntuple or
				  a histogram. This subcommand returns
                                  the Histo-Scope id of the result.

fit::Fit_parameter opt1? value1? ...

     Arguments : various options supplied as strings
     Returns   : depends on the option given

     This command manipulates parameters of the active fit. The following
     forms can be used:

   * fit::Fit_parameter add $parname opt? value? ...

     Adds a parameter named $parname to the active fit. $parname must be
     different from the names of already existing paramaters in its first
     ten characters (FORTRAN-like comparison, dictated by Minuit interface).
     The following options are supported:

     -value $v         $h is the initial parameter value (default is 0.0).

     -step $h          $h is the initial parameter step size for Minuit.
                       It must be positive. The default h value is 0.1.

     -state $s         $s is one of the stings "fixed" or "variable".
                       The default is "variable".

     -bounds $blist    $blist is a two-element list of parameter limits
                       {min max}. The default blist value is an empty
                       list which means that the parameter has no limits.

   * fit::Fit_parameter list

     Returns the list of parameter names for the active fit.

   * fit::Fit_parameter clear

     Deletes all parameters of the active fit.

   * fit::Fit_parameter info

     Returns the list of parameter definitions. Each element is itself
     a list {name value state step bounds}.

   * fit::Fit_parameter apply

     Passes the current parameter definitions to Minuit. Returns nothing.

   * fit::Fit_parameter $name $subcommand opt1? value1? ...

     Manipulates the parameter named $name in the active fit. The following
     subcommands may be used:

     delete                       Deletes the parameter named $name.

     exists                       Returns 1 if the parameter named $name
                                  exists, 0 otherwise.

     set newvalue?                Returns four-element list {value state \
                                  step bounds} for the parameter named $name.
				  When newvalue is specified, sets the value
				  of an existing parameter. newvalue may be
				  either a double value or a four-element list.

     configure opt1? value1? ...  Sets the parameter properties. Available
                                  options are -value, -state, -step, and
                                  -bounds. The meaning of these options is
                                  the same as in the "fit::Fit_parameter add"
                                  command

     cget $option                 Returns the parameter property specified
                                  by $option. When the fit is finished, the
                                  following options are available in addition
                                  to the standard options -value, -state,
                                  -step, and -bounds:

				  -error   -- returns "parabolic" error
				  -eneg    -- returns negative error from Minos
                                              (as a negative number)
				  -epos    -- returns positive error from Minos
                                  -globcc  -- returns the global correlation
                                              coefficient. This is a number
                                              between zero and one which gives
                                              the correlation between the
                                              parameter named $name and that
                                              linear combination of all other
                                              parameters which is most strongly
                                              correlated with $name.
				  -errinfo -- returns the previous four
                                              properties in a single list
					      {error eneg epos globcc}

fit::Fit_function opt1? value1? ...

     Arguments : various options supplied as strings
     Returns   : depends on the option given

     This command manipulates data fitting functions of the active fit.
     The following forms can be used:

   * fit::Fit_function add $funct opt1? value1? ...

     Adds a data fitting function with tag $funct to the active fit. $funct
     must be a valid funcion tag as defined by commands hs::function_import,
     hs::function_compose, etc. It is not possible to add a function
     with the same tag to a fit more than once but it is possible to use
     the same function with more than one dataset included in the same fit.
     The following options are allowed:

     -subsets $datasets      $datasets is a list of dataset numbers to
                             which this function will be added. This option
			     may be omitted if the active fit includes only
			     one dataset, but for fits with multiple datasets
			     this option is mandatory.

     -mapping $codestring    Specifies the mapping from fit parameters into
                             the function parameters. The mapping is either
                             a string of pseudo-C code or a special keyword
                             which designates a precompiled mapping. In the
			     pseudo-C code the fit parameters should be
			     designated by name or by number and prefixed
			     with the '%' character. The function parameters
			     are referred to by names. Here is an example
			     mapping:
			         { double bin_width = 0.05;
				   area  = %nevents * bin_width;
			           mean  = %1;
			           sigma = sqrt(%variance); }
			     Besides the special designators for the fit
			     parameters (whose type should not be declared),
			     everything else should be normal C code which
			     can use all the functions provided in the
			     standard C headers. The character '%' itself
			     can be written as the "%%" sequence. The mapping
			     code will be compiled and loaded before the fit
			     is performed.
			     The following keywords may be used instead of
			     a C code map: "sequential", "identical", and
			     "null". Null mapping is the default mapping
			     for fitting functions without parameters, and
			     sequential mapping is the default for functions
			     with parameters.

     -params $parlist        $parlist specifies initial values of the function
                             parameters (which can be used, for example,
			     to plot the function) in a list of name-value
                             pairs. For example, {{mean 0.0} {sigma 1.0}}.
                             The default value for all parameters is 0.0.

   * fit::Fit_function list

     Returns the list of tags for data fitting functions used in
     the active fit.

   * fit::Fit_function $tag $subcommand opt1? value1? ...

     Manipulates the fitting function identified by tag $tag in the active fit.
     The following subcommands may be used:

     exists                        Returns 1 if the function with the given
                                   tag is included into the active fit, 
				   0 otherwise.

     info                          Returns the info about a function in a list
                                   {name mapping offset subsets parlist}.
				   offset is the total number of parameters
				   in all preceeding functions used to set up
				   the sequential parameter mapping.

     delete                        Removes the function from the fit and all
                                   its datasets.

     compiledmap dll? proc?        When issued without the dll and proc
                                   arguments, this subcommand returns 1 in case
				   the mapping has already been compiled and
				   0 otherwise. Optional dll and proc arguments
				   can be used to set the dll number and the
				   C function name to use for mapping. dll
				   number can be -1 to indicate that one of
				   the precompiled maps should be used.
				   In this case a precompiled map keyword
                                   should be used in place of the "proc"
				   argument (see the description of -mapping
				   option in the "fit::Fit_function add"
				   command for the list of such keywords).
				   Also, the "proc" argument can be set to an
				   empty string in which case the mapping
				   is removed (so that some code can later
				   install a default mapping). It should not
				   be necessary for a user code to use this
				   subcommand.

     configure opt1? value1? ...   Configures the options -subsets, -mapping,
                                   and -params which have the same meaning as
				   in the "fit::Fit_function add" command.

     cget $option                  Returns the function property identified
                                   by $option. In addition to the standard
                                   options -subsets, -mapping, and -params,
				   this subcommand suppots convenience options
				   "-npars" which instructs it to return
				   the number of function parameters,
				   "-parnames" which returns the list of
				   just parameter names, and "-parvalues"
				   which returns the list of just parameter
				   values.

fit::Fit_config opt1? value1? ...

     Arguments : various options supplied as strings
     Returns   : nothing

     This command has two forms:

     1) fit::Fit_config apply

     In this form, the command passes configuration options to Minuit.
     The following options are set: errdef, warn, strategy, eps, grad,
     verbose.

     2) fit::Fit_config sw1 value1 ...

     In this form, the command defines various properties of the active fit.
     The set of switches is the same as in the fit::Fit_create command.

fit::Fit_cget $option

     Arguments : string option
     Returns   : the return type depends on the option given

     This command can be used to examine various properties of the active fit.
     The following options are supported: -compiled, -complete, -description,
     -errdef, -gradient, -method, -precision, -strategy, -title, -epars, -emat,
     -verbose, -ignore, -warnings, -psync, -osync, -dlls, -ministat, -minos,
     -minimizer, -wpoints, -timeout, and -status. Most switches have the same
     meaning as in the fit::Fit_create command. Additional switches are:

     -compiled    With this switch, the command returns 1 if all parameter
                  mappings and dataset filters have been compiled, and
                  0 otherwise.

     -complete    Returns 0 if there were parameter or mapping changes since
                  the last minimization run or if the minimization has never
                  been performed, otherwise returns 1.

     -epars       Returns the list of parameters for which the error matrix
                  has been estimated, or an empty list if the matrix
                  has never been calculated.

     -emat        Returns the error matrix as a list whose elements are
                  the matrix rows, or an empty list if the matrix has never
		  been calculated.

     -osync       Returns 1 if the fitting options have been passed to Minuit
                  since the last change, 0 otherwise (use the command
		  "fit::Fit_config apply" to synchronize the options).

     -psync       Returns 1 if the fit parameters have been synchronized 
                  with Minuit, 0 otherwise (use the command
		  "fit::Fit_parameter apply" to synchronize the parameters).

     -dlls        Returns the list of shared libraries which may be unloaded
                  when the fit is deleted.

     -ministat    Returns the minimization status as a list
                  {fmin fedm errdef npari nparx istat}. See the description
		  of the Minuit subroutine MNSTAT for the meaning of the list
		  elements. This will be an empty list if the minimization
		  has not been performed yet after the last configuration
		  change.

     -wpoints     Returns the combined total weighed number of points in all
                  datasets used to calculate the minimization statistics
		  during the most recent FCN call.

     These additional switches correspond to the fit properties which can be
     examined from tcl but can not be set explicitly using "configure".

fit::Fit_tcldata $action index? value?

     Arguments : string action, string index, value can be anything
     Returns   : depends on action

     This command may be used to associate some arbitrary data with
     the fit. The following command forms may be used:

   * fit::Fit_tcldata set $index $value

     Associates the given value with the given index. Returns nothing.

   * fit::Fit_tcldata set $index_value_list

     $index_value_list must be a tcl list with an even number of elements:
     {index1 value1 index2 value2 ...}. This command associates given values
     with the given indices. All previous associations are discarded. Returns
     nothing.

   * fit::Fit_tcldata get

     Returns the list of all associations

   * fit::Fit_tcldata get $index

     Returns the value associated with the given index.

   * fit::Fit_tcldata exists $index

     Returns 1 if the given index exists, 0 if not.

fit::Fit_append_dll $n

     Arguments : ineger n
     Returns   : nothing

     Appends the dll number (obtained from an hs::sharedlib call) to the
     list of dlls which should be unloaded when the fit which is currently
     active is deleted. This is useful for keeping track of various
     autogenerated code such as parameter mappings and data exclusion regions.

fit::Fit_apply_mapping

     Arguments : none
     Returns   : nothing

     Calls the parameter mapping functions of the active fit.

fit::Fit_tcl_fcn_args

     Arguments : arbitrary
     Returns   : nothing

     Sets up objc and objv arguments for the current fit configuiration.
     The first object (command name) is skipped.

fit::Fit_has_tcl_fcn

     Arguments : none
     Returns   : boolean

     Returns "true" if the current fit has user-defined fcn.

fit::Fit_reset

     Arguments : none
     Returns   : nothing

     Resets the active fit (possibly, after minimization failure) before
     making Minuit calls and sets up Munuit job title. This call should be
     followed by option and parameter synchronization (use commands
     "fit::Fit_config apply" and "fit::Fit_parameter apply").

fit::Parse_minuit_pars_in_a_map $map_string $minuit_pars $c_array_name

     Arguments : strings map_string and c_array_name, list of strings
                 minuit_pars
     Returns   : string

     Minuit parameters in the parameter maps are specified as %N
     or %name, where N is an integer from 0 to (# of parameters - 1),
     and name is a parameter name, as specified in the hs::Fit_parameter
     command. The percent sign indicates the start of a parameter string.
     The % sign itself can be written as %%. This command replaces all
     parameter definitions in $map_string by strings "${c_array_name}\[$N\]"
     where $N is the parameter number in the $minuit_pars list. This command
     is used internally for generating C code which maps Minuit parameters
     into fitting function parameters. It returns the map_string  with
     the necessary replacements.

fit::Parse_fitter_pars_in_a_map map_string fitter_pars c_array_name

     Arguments : strings map_string and c_array_name, list of strings
                 fitter_pars
     Returns   : string

     Replaces all words in $map_string which match any of the parameter
     names given in the fitter_pars list by strings "${c_array_name}\[$N\]"
     where $N is the parameter number in the $fitter_pars sequence. This
     command is used internally for generating C code which maps Minuit
     parameters into fitting function parameters. It returns the map_string
     with the necessary replacements.

fit::Analyze_multires_ntuple id

     Arguments : integer id
     Returns   : double

     Analyzes the ntuple obtained in the process of the multiresolution
     smoothing test. Returns the confidence level assigned to the first
     ntuple row (this row should contain the set of norms for the original
     fit).

fit::Fit_multires_fast_cycle id_fft smoothing_curves

     Arguments : integer id_fft, list of integers smoothing_curves
     Returns   : list of doubles

     Speed up and cacheing function for the following tcl code:

     proc ::fit::Fit_multires_fast_cycle {id_fft smoothing_curves} {
         set normlist {}
         foreach id $smoothing_curves {
             set newid [hs::1d_fourier_multiply [hs::Temp_uid] \
                 "Convoluted fit difference for curve with id $id" \
                 [hs::Temp_category] $id $id_fft]
             hs::1d_fft $newid $newid 0
             lappend normlist [hs::hist_l1_norm $newid]
             hs::delete $newid
         }
         set normlist
     }
*/
