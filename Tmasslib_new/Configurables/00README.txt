
Files which contain configuration data for Tmasslib (grids, distributions,
correction factors, etc.):

generate_ttbar_pt.c    -- Generates ttbar Pt (2d vector) distribution. Needed
                          for global normalization studies. Current version
                          assumes that ttbar Pt distribution is independent
                          from the rest of the event (e.g., ttbar Pz) and
                          models ttbar Pt by throwing events from the ttbar
                          Pt distribution histogram originally filled from
                          Herwig events without any cuts. There are some
                          arguments that ttbar Pt should have some correlation
                          with ttbar Pz and E but this correlation appears
                          to be weak.

hadronic_w_grid.c      -- Models W mass shape as a function of a "predictor"
                          variable. This is the "effective" shape -- the W
                          mass is misreconstructed because hadronization
                          effects are not taken into account. The predictor
                          is currently taken in the form of 
                          sqrt((d MW^2/d mq^2)^2 + (d MW^2/d mqbar^2)^2) 
                          under the constraint that the jet energies are
                          correct.

                          This grid is needed because of the "catch 22" effect:
                          in order to integrate over Mt^2, MW^2 grid on the
                          hadronic side, the program has to decide first
                          which effective propagator to use. In order to
                          choose the effective propagator, the program
                          has to know the event kinematics and calculate
                          Mt^2, MW^2 covariance matrix, so it has to assume
                          some Mt, MW value. "hadronic_w_grid.c" allows us
                          to choose a reasonable preliminary MW grid without
                          assuming the top mass.

henris_acceptances.c   -- Dependence of the jet reconstruction efficiency
                          on the parton Pt. It is not obvious whether
                          efficiency modeling in the parton space is needed.
                          Perhaps, efficiency model in the space of observed
                          variables using just 0 or 1 for every point
                          is sufficient.

lepside_tree.c         -- Search tree for event-by-event MW^2/Mt^2 effective
                          propagators on the leptonic side. The search
                          is performed in the 3d space of MW^2 error,
                          Mt^2 error, and correlation coefficient between
                          Mt^2, MW^2. The error matrix is due to hadronization
                          and angular resolution effects only, jet energy
                          resolution is handled by transfer functions.

                          Note that on the leptonic side the "catch 22"
                          problem is solved differently from the hadronic
                          side. The leptonic integration grid does not
                          change from event to event (it is generated using
                          average density in MW^2, Mt^2 space). The effective
                          propagators are represented as ratios between
                          the propagator density and average density at
                          the average grid points.

parameter_grid.c       -- Grid for integrating over log(pq/pqbar) parameter.

propagator_tree.c      -- Search tree for event-by-event MW^2/Mt^2 effective
                          propagators on the hadronic side.

punzi_tree_had.c       -- Search tree for Punzi correction factors on the
                          hadronic side. The whole idea of using Punzi
                          corrections in this analysis may be misguided
                          because the covariance matrix is just a function
                          of the phase space point. So, the concept of
                          "covariance matrix probability" on which Punzi
                          corrections are based is moot.

punzi_tree_lep.c       -- Search tree for Punzi correction factors on the
                          leptonic side.

random_jet_angles.c    -- Models eta and phi jet resolution as a function
                          of jet Pt. Based on simulated Herwig events. Ask
                          Paul if you want to know exactly which selection
                          cuts were applied when the reference distributions
                          were built.

random_jet_masses.c    -- Defines the typical width of jet mass squared.
                          The function "random_jet_mass" is not implemented
                          yet (the prototype is there but it always returns 0).

random_lepton_momentum.c  Models Pt resolution for electrons and muons.
                          This code needs some check because, as far as
                          I can remember, the numbers are based on Run 1!

transfer_function.c    -- Models the jet transfer functions (could be in Pt,
                          p, E, or any other variable).

There are also several binary data files (this stuff works on little endian
platforms only):

1) The file with hadronic side effective propagator data. File name
   could be anything. It is later given to the scanning code as the
   "had_propagator_file" parameter. This file is normally generated
   together with the propagator_tree.c file.

   The hadronic side effective propagator data file can be omitted
   if the parameter "single_mthad_point" is set to 1. In this case
   there will be no scan of the hadronic top propagator.

2) The file with leptonic side MW^2, Mt^2 grid. File name is arbitrary,
   the corresponding parameter is "lep_grid_file".

3) The file with leptonic side effective propagator data. File name is
   arbitrary, the corresponding parameter is "lep_ratios_file".

   The files described in 2) and 3) are normally generated together with
   the "lepside_tree.c" file. These files must always be specified, there
   is no way to turn off the leptonic side grid.

4) The file with Punzi correction factors for the hadronic side.
   Parameter name is "had_punzi_file". 

5) The file with Punzi correction factors for the leptonic side.
   Parameter name is "lep_punzi_file".

   Hadronic and leptonic side Punzi corrections file names must be specified
   only if the parameter "use_punzi_correction" is set to 1.

Additional information about generating these binary files can be found at
http://www-cdf.lbl.gov/~igv/Topmass/generating_correlated_propagators.txt

