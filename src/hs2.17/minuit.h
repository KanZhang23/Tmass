#ifndef MINUIT_H
#define MINUIT_H

/* The length of parameter names in Minuit */
#define MINUIT_PARAM_NAME_LEN 10

#ifdef __cplusplus
extern "C" {
#endif

void mintio_(int *IREAD, int *IWRITE, int *ISAVE);
/* Communicate to Minuit the I/O units.
 *
 * Input parameters:
 *   IREAD  -- Fortran unit number for reading (default 5). 
 *   IWRITE -- Fortran unit number for writing (default 6). 
 *   ISAVE  -- Fortran unit number for saving (default 7).
 */

void minuit_(void (*FCN)(), void (*FUTIL)());
/* Start Minuit in data-driven mode.
 *
 * Input parameters:
 *   FCN    -- Name of the function being analyzed.
 *   FUTIL  -- Name of a function called by FCN (or NULL if not used).
 *
 * In data-driven mode, that is with CALL MINUIT, you should not call MNINIT,
 * since Minuit takes care of all initialization. To change unit numbers,
 * call MINTIO _before_ calling MINUIT.
 */

void mninit_(int *IRD, int *IWR, int *ISAV);
/* Initialize Minuit
 *
 * Input parameters:
 *   IRD  -- Unit number for input to Minuit.
 *   IWR  -- Unit number for output from Minuit.
 *   ISAV -- Unit number for use of the SAVE command.
 */

void mnseti_(const char *TITLE, unsigned TITLE_len);
/* Specify a title for the problem 
 *
 * Input parameters:
 *   TITLE -- Character string of up to 50 characters containing
 *            an identification text for the present job or fit.
 *   TITLE_len -- Length of the title string.
 */

void mnparm_(int *NUM, char *CHNAM, double *STVAL, double *STEP,
	     double *BND1, double *BND2, int *IERFLG,
	     unsigned CHNAM_len);
/* Define a parameter
 *
 * Input parameters:
 *   NUM   -- Parameter number as referenced by user in FCN. 
 *   CHNAM -- Character string of up to 10 characters containing the name
 *            which the user assigned to the given parameter. 
 *   STVAL -- Starting value 
 *   STEP  -- Starting step size or approximate parameter error. 
 *   BND1  -- Lower bound (limit) on parameter value, if any. 
 *   BND2  -- Upper bound (limit) on parameter value, if any. 
 *            If BND1 == 0.0 && BND2 == 0.0, then the parameter is considered
 *            unbounded, which is recommended unless limits are needed to
 *            make things behave well.
 *   CHNAM_len -- Length of the parameter name string.
 *
 * Output parameter: 
 *   IERFLG -- Error return code: 0 if no error, >0 if request failed.
 */

void mnpars_(char *CHSTR, int *IERFLG, unsigned CHSTR_len);
/* Define a parameter from a string
 *
 * Input parameters:
 *   CHSTR -- String which specifies the parameter definition in the usual
 *            Minuit format, as on a data record. The fields are in the same
 *            order as the arguments to MNPARM.
 *   CHSTR_len -- Length of the parameter definition string.
 *
 * Output parameter:
 *   IERFLG -- Error return code. Its possible values are
 *             0 -- all OK
 *             1 -- error, attempt to define parameter is ignored
 *             2 -- end of parameter definitions (parameter number zero)
 */

void mnexcm_(void (*FCN)(), char *CHCOM, double *ARGLIS, int *NARG, 
	     int *IERFLG, void (*FUTIL)(), unsigned CHCOM_len);
/* Execute a Minuit command
 *
 * Input parameters:
 *   FCN    -- Name of the function being analyzed.
 *   CHCOM  -- Character string containing the name of the Minuit command
 *             to be executed. 
 *   ARGLIS -- Array of dimension MAXARG, containing the numeric arguments
 *             to the command (if any).
 *   NARG   -- Number of arguments specified (NARG <= MAXARG).
 *   FUTIL  -- Name of a function called by FCN (or NULL if not used).
 *   CHCOM_len -- Length of the command name string.
 * 
 * Output parameter: 
 *   IERFLG -- Error return code: 0 if the command was executed normally,
 *             >0 otherwise.
 */

void mncomd_(void (*FCN)(), char *CHSTR, int *IERFLG,
	     void (*FUTIL)(), unsigned CHSTR_len);
/* Execute a Minuit command specified as a character string. Subroutine MNCOMD
 * causes the execution of the Minuit command specified as the second argument.
 * It therefore works like MNEXCM, except that it accepts the entire command
 * with arguments as one character string.
 *
 * Input parameters:
 *   FCN    -- Name of the function being analyzed.
 *   CHSTR  -- The full Minuit command with arguments.
 *   FUTIL  -- Name of a function called by FCN (or NULL if not used).
 *   CHSTR_len -- Length of the command string.
 * 
 * Output parameter: 
 *   IERFLG -- Error return code: 0 if the command was executed normally,
 *             >0 otherwise.
 * Some abnormal conditions: 
 *   IERFLG == 1 command was blank, ignored 
 *   IERFLG == 2 command line was unreadable, ignored
 *   IERFLG == 3 command was unknown, ignored 
 *   IERFLG == 4 abnormal termination (e.g., MIGRAD not converged) 
 */

void mnpout_(int *NUM, char *CHNAM, double *VAL, double *ERROR,
	     double *BND1, double *BND2, int *IVARBL,
	     unsigned CHNAM_len);
/* Get the current value of a parameter
 *
 * Input parameters:
 *   NUM  -- Parameter number as referenced by user in FCN and about
 *           which information is required.
 *   CHNAM_len -- Lenght of the CHNAM buffer.
 *
 * Output parameters: 
 *   CHNAM  -- Character string of up to 10 characters containing the
 *             name which the user assigned to the given parameter. 
 *   VAL    -- Current parameter value (fitted value if fit has converged).
 *   ERROR  -- Current estimate of parameter uncertainty (or zero if constant).
 *   BND1   -- Lower limit on parameter value, if any (otherwise zero). 
 *   BND2   -- Upper limit on parameter value, if any (otherwise zero). 
 *   IVARBL -- Internal parameter number if parameter is variable, or zero
 *             if parameter is constant, or negative if parameter is undefined.
 */

void mnstat_(double *FMIN, double *FEDM, double *ERRDEF,
	     int *NPARI, int *NPARX, int *ISTAT);
/* Get the current status of minimization
 *
 * Output parameters (no input):
 *   FMIN   -- The best function value found so far.
 *   FEDM   -- The estimated vertical distance remaining to minimum.
 *   ERRDEF -- The value of UP defining parameter uncertainties.
 *   NPARI  -- The number of currently variable parameters.
 *   NPARX  -- The highest (external) parameter number defined by user.
 *   ISTAT  -- A status integer indicating how good is the covariance matrix: 
 *             0 -- Not calculated at all 
 *             1 -- Diagonal approximation only, not accurate 
 *             2 -- Full matrix, but forced positive-definite 
 *             3 -- Full accurate covariance matrix (After MIGRAD, this is
 *                  the indication of normal convergence).
 */

void mnemat_(double *EMAT, int *NDIM);
/* Get the current value of the covariance matrix 
 *
 * Input parameters:
 *   NDIM  -- Integer variable specifying the number of rows and columns
 *            the issuer has reserved in EMAT to store the matrix elements.
 *            NDIM should be at least as large as the number of parameters
 *            variable at the time of the call, otherwise the user will get
 *            only part of the full matrix.
 *
 * Output parameters:
 *   EMAT  -- Array declared as DIMENSION EMAT(NDIM,NDIM) which is to be
 *            filled with the covariance matrix.
 */

void mnerrs_(int *NUM, double *EPLUS, double *EMINUS,
	     double *EPARAB, double *GLOBCC);
/* Access current parameter errors. Note that this call does not cause the
 * errors to be calculated, it merely returns the current existing values.
 *
 * Input parameters:
 *   NUM    -- Parameter number. If NUM>0, this is taken to be an external
 *             parameter number; if NUM<0, it is the negative of an internal
 *             parameter number.
 *
 * Output parameters:
 *   EPLUS  -- The positive MINOS error of parameter NUM.
 *   EMINUS -- The negative MINOS error (a negative number).
 *   EPARAB -- The ``parabolic'' parameter error, from the error matrix.
 *   GLOBCC -- The global correlation coefficient for parameter NUM. This is
 *             a number between zero and one which gives the correlation
 *             between parameter NUM and that linear combination of all other
 *             parameters which is most strongly correlated with NUM.
 */

void mncont_(void (*FCN)(), int *NUM1, int *NUM2, int *NPT,
	     double *XPT, double *YPT, int *NFOUND, void (*FUTIL)());
/* Find a function contour with the MNContour method
 *
 * Input parameters:
 *   FCN        -- Name of the function being treated
 *   NUM1, NUM2 -- Parameter numbers with respect to which the contour
 *                 is to be determined.
 *   NPT        -- The number of points required on the contour (>4).
 *   FUTIL      -- Name of a function called by FCN (or NULL if not used).
 *
 * Output parameters:
 *   XPT        -- Array of x-coordinates of contour points with values for
 *                 parameter NUM1. Must be able to hold at least NPT values.
 *   YPT        -- Array of y-coordinates of contour points with values for
 *                 parameter NUM2. Must be able to hold at least NPT values.
 *   NFOUND     -- The number of points actually found on the contour.
 *                 If all goes well, this will be equal to NPT, but it can
 *                 be negative (if the input arguments are not valid), or
 *                 zero if less than four points have been found, or less
 *                 than NPT if the program could not find NPT points.
 */

void mnintr_(void (*FCN)(), void (*FUTIL)());
/* Switch to interactive mode.
 *
 * Input parameters:
 *   FCN        -- Name of the function being treated
 *   FUTIL      -- Name of a function called by FCN (or NULL if not used).
 *
 * The call to MNINTR will cause Minuit to read commands from the unit IRD
 * (originally specified by the user in his call to MNINIT, IRD is usually 5
 * by default, which in turn is usually the terminal by default). Minuit
 * then reads and executes commands until it encounters a command END, EXIT,
 * RETurn, or STOP, or an end-of-file on input (or an unrecoverable error
 * condition while reading or trying to execute a command), in which case
 * control returns to the program which called MNINTR.
 */

void mninpu_(int *NUNIT, int *IERR);
/* Sets logical unit number of input unit from which Minuit will read
 * the next command.
 *
 * Input parameters:
 *   NUNIT -- The I/O unit number, which must be a valid unit, opened
 *            for reading (Minuit makes no checks at this level and
 *            will not attempt to open any files).
 *   IERR  -- returned as zero unless Minuit's internal buffer which
 *            stores unit numbers is full, which is a fatal error. If NUNIT
 *            is specified as zero, Minuit returns to reading the previous
 *            unit (which is why it has to store them).
 */

#ifdef __cplusplus
}
#endif

#endif /* MINUIT_H */
