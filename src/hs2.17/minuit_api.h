#ifndef _MINUIT_API_H
#define _MINUIT_API_H

#include "simple_tcl_api.h"

tcl_routine(fortranfile);
tcl_routine(mintio);
tcl_routine(minuit);
tcl_routine(init);
tcl_routine(seti);
tcl_routine(prti);
tcl_routine(pars);
tcl_routine(parm);
tcl_routine(comd);
tcl_routine(excm);
tcl_routine(pout);
tcl_routine(stat);
tcl_routine(errs);
tcl_routine(intr);
tcl_routine(inpu);
tcl_routine(cont);
tcl_routine(emat);

/* The following function gets/sets the max number of FCN 
   calls. Can be used when a minimizer is running in order
   to initiate a quick stop */
tcl_routine(maxcalls);

/* The following function returns the number of FCN calls 
   Minuit has made during its last minimization attempt */
tcl_routine(nfcn);

/* The following function returns the relative change in the covariance
   matrix. Can be used as a measure of the covariance matrix precision. */
tcl_routine(dcovar);

#endif /* _MINUIT_API_H */
