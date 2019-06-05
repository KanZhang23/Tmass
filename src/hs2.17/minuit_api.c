#include <string.h>
#include <stdlib.h>
#include "minuit.h"
#include "histoscope.h"
#include "minuit_api.h"
#include "fit_config.h"
#include "mn_interface.h"

#define STDERR_LUN 0
#define MAXLUN 512
#define MINLUN 12

static int busyluns[MAXLUN] = {0};

static int get_standard_lun(char *channel);

/* Local functions for which we will not create commands */
tcl_routine(fortranfile_open);
tcl_routine(fortanfile_close);

tcl_routine(mintio)
{
    int IREAD, IWRITE, ISAVE;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &IREAD) != TCL_OK)
    {
	IREAD = get_standard_lun(Tcl_GetStringFromObj(objv[1], NULL));
	if (IREAD != 5)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for read: ",
			     Tcl_GetStringFromObj(objv[1], NULL), NULL);
	    return TCL_ERROR;
	}
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &IWRITE) != TCL_OK)
    {
	IWRITE = get_standard_lun(Tcl_GetStringFromObj(objv[2], NULL));
	if (IWRITE != 6 && IWRITE != STDERR_LUN)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for write: ",
			     Tcl_GetStringFromObj(objv[2], NULL), NULL);
	    return TCL_ERROR;
	}	
    }
    if (Tcl_GetIntFromObj(interp, objv[3], &ISAVE) != TCL_OK)
    {
	ISAVE = get_standard_lun(Tcl_GetStringFromObj(objv[3], NULL));
	if (ISAVE != 6 && ISAVE != STDERR_LUN)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for save: ",
			     Tcl_GetStringFromObj(objv[3], NULL), NULL);
	    return TCL_ERROR;
	}	
    }
    Tcl_ResetResult(interp);
    mintio_(&IREAD, &IWRITE, &ISAVE);
    return TCL_OK;
}

tcl_routine(minuit)
{
    Fit_config *fit;
    Minuit_user_function *fcn;

    tcl_require_objc(1);
    fit = hs_current_fit_conf();
    fcn = hs_get_current_fit_fcn();
    if (fit == NULL || fcn == NULL)
    {
	Tcl_SetResult(interp, "fit is not set up", TCL_STATIC);
	return TCL_ERROR;
    }
    fit->interp = interp;
    fit->status = TCL_OK;
    minuit_(fcn, (void (*)())fit);
    return fit->status;
}

tcl_routine(init)
{
    int IREAD, IWRITE, ISAVE;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &IREAD) != TCL_OK)
    {
	IREAD = get_standard_lun(Tcl_GetStringFromObj(objv[1], NULL));
	if (IREAD != 5)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for read: ",
			     Tcl_GetStringFromObj(objv[1], NULL), NULL);
	    return TCL_ERROR;
	}
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &IWRITE) != TCL_OK)
    {
	IWRITE = get_standard_lun(Tcl_GetStringFromObj(objv[2], NULL));
	if (IWRITE != 6 && IWRITE != STDERR_LUN)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for write: ",
			     Tcl_GetStringFromObj(objv[2], NULL), NULL);
	    return TCL_ERROR;
	}	
    }
    if (Tcl_GetIntFromObj(interp, objv[3], &ISAVE) != TCL_OK)
    {
	ISAVE = get_standard_lun(Tcl_GetStringFromObj(objv[3], NULL));
	if (ISAVE != 6 && ISAVE != STDERR_LUN)
	{
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "invalid logical unit for save: ",
			     Tcl_GetStringFromObj(objv[3], NULL), NULL);
	    return TCL_ERROR;
	}	
    }
    Tcl_ResetResult(interp);
    mninit_(&IREAD, &IWRITE, &ISAVE);
    return TCL_OK;
}

tcl_routine(prti)
{
    char *cmd = "show title";
    int errflag;
    mncomd_(NULL, cmd, &errflag, NULL, strlen(cmd));
    if (errflag)
    {
	Tcl_AppendResult(interp, "Minuit command \"",
			 cmd, "\" failed", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(seti)
{
    char *title;
    unsigned int len;

    tcl_require_objc(2);
    title = strdup(Tcl_GetStringFromObj(objv[1], NULL));
    if (title == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    len = strlen(title);
    if (len > 50)
	len = 50;
    mnseti_(title, len);
    free(title);
    return TCL_OK;
}

tcl_routine(parm)
{
    int NUM, IERFLG=0; 
    char *CHNAM;
    double STVAL, STEP, BND1 = 0.0, BND2 = 0.0;
    unsigned len;
    
    tcl_objc_range(5,7);
    if (Tcl_GetIntFromObj(interp, objv[1], &NUM) != TCL_OK)
	return TCL_ERROR;
    CHNAM = Tcl_GetStringFromObj(objv[2], NULL);
    len = strlen(CHNAM);
    if (Tcl_GetDoubleFromObj(interp, objv[3], &STVAL) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &STEP) != TCL_OK)
	return TCL_ERROR;
    if (objc > 5)
    {
	if (Tcl_GetDoubleFromObj(interp, objv[5], &BND1) != TCL_OK)
	    return TCL_ERROR;
	if (objc > 6)
	{
	    if (Tcl_GetDoubleFromObj(interp, objv[6], &BND2) != TCL_OK)
		return TCL_ERROR;
	}
	else
	{
	    Tcl_SetResult(interp, "upper parameter bound must be specified"
			  " if the lower bound is specified", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    mnparm_(&NUM, CHNAM, &STVAL, &STEP, &BND1, &BND2, &IERFLG, len);
    if (IERFLG)
    {
	Tcl_AppendResult(interp, "failed to define parameter \"", 
			 CHNAM, "\"", NULL);
	return TCL_ERROR;
    }
    else
	return TCL_OK;
}

tcl_routine(pars)
{
    Tcl_DString dstring;
    int i, errflag;
    unsigned len;
    char *s, *arg;
    
    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_DStringInit(&dstring);
    arg = Tcl_GetStringFromObj(objv[1], NULL);
    s = Tcl_DStringAppend(&dstring, arg, strlen(arg));
    for (i=2; i<objc; ++i)
    {
	s = Tcl_DStringAppend(&dstring, " ", 1);
	arg = Tcl_GetStringFromObj(objv[i], NULL);
	s = Tcl_DStringAppend(&dstring, arg, strlen(arg));
    }
    len = Tcl_DStringLength(&dstring);
    mnpars_(s, &errflag, len);
    Tcl_DStringFree(&dstring);
    switch (errflag)
    {
    case 0:
	break;
    case 1:
	Tcl_SetResult(interp, 
		      "error, attempt to define parameter is ignored",
		      TCL_STATIC);
	return TCL_ERROR;
    case 2:
	Tcl_SetResult(interp,
		      "error, end of parameter definitions",
		      TCL_STATIC);
	return TCL_ERROR;
    default:
	Tcl_SetResult(interp,
		      "bad result, unknown error code",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(excm)
{
    char *command;
    double numargs[50];
    int i, NARG, IERFLG;
    unsigned len;
    Fit_config *fit;
    Minuit_user_function *fcn;

    tcl_objc_range(2,52);
    fit = hs_current_fit_conf();
    fcn = hs_get_current_fit_fcn();
    if (fit == NULL || fcn == NULL)
    {
	Tcl_SetResult(interp, "fit is not set up", TCL_STATIC);
	return TCL_ERROR;
    }
    command = Tcl_GetStringFromObj(objv[1], NULL);
    len = strlen(command);
    NARG = objc - 2;
    for (i=0; i<NARG; ++i)
	if (Tcl_GetDoubleFromObj(interp, objv[2+i], numargs+i) != TCL_OK)
	    return TCL_ERROR;
    IERFLG = 0;
    fit->status = TCL_OK;
    fit->interp = interp;
    mnexcm_(fcn, command, numargs, &NARG, &IERFLG, (void (*)())fit, len);
    if (IERFLG)
    {
	Tcl_AppendResult(interp, "failed to execute command \"", 
			 command, "\"", NULL);
	return TCL_ERROR;
    }
    return fit->status;
}

tcl_routine(comd)
{
    Tcl_DString dstring;
    int i, errflag;
    unsigned len;
    char *s, *arg;
    Fit_config *fit;
    Minuit_user_function *fcn;

    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    fit = hs_current_fit_conf();
    fcn = hs_get_current_fit_fcn();
    if (fit && fcn)
    {
	Tcl_DStringInit(&dstring);
	arg = Tcl_GetStringFromObj(objv[1], NULL);
	s = Tcl_DStringAppend(&dstring, arg, strlen(arg));
	for (i=2; i<objc; ++i)
	{
	    s = Tcl_DStringAppend(&dstring, " ", 1);
	    arg = Tcl_GetStringFromObj(objv[i], NULL);
	    s = Tcl_DStringAppend(&dstring, arg, strlen(arg));
	}
	len = Tcl_DStringLength(&dstring);
	fit->status = TCL_OK;
	fit->interp = interp;
	mncomd_(fcn, s, &errflag, (void (*)())fit, len);
	Tcl_DStringFree(&dstring);
	switch (errflag)
	{
	case 0:
	    break;
	case 1:
	    Tcl_AppendResult(interp, "blank command, ignored", NULL);
	    return TCL_ERROR;
	case 2:
	    Tcl_AppendResult(interp, "unreadable command, ignored", NULL);
	    return TCL_ERROR;
	case 3:
	    Tcl_AppendResult(interp, "unknown command, ignored", NULL);
	    return TCL_ERROR;
	case 4:
	    Tcl_AppendResult(interp, "command terminated abnormally", NULL);
	    return TCL_ERROR;
	default:
	    Tcl_AppendResult(interp, "bad result, unknown error code", NULL);
	    return TCL_ERROR;
	}
	return fit->status;
    }
    else
    {
	Tcl_SetResult(interp, "fit is not set up", TCL_STATIC);
	return TCL_ERROR;
    }
}

tcl_routine(pout)
{
    int NUM, IVARBL;
    char CHNAM[MINUIT_PARAM_NAME_LEN+1];
    double VAL, ERROR, BND1, BND2;
    Tcl_Obj *result[6];

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &NUM) != TCL_OK)
	return TCL_ERROR;
    mnpout_(&NUM, CHNAM, &VAL, &ERROR, &BND1, &BND2, &IVARBL, MINUIT_PARAM_NAME_LEN);
    CHNAM[MINUIT_PARAM_NAME_LEN] = '\0';
    result[0] = Tcl_NewStringObj(CHNAM, -1);
    result[1] = Tcl_NewDoubleObj(VAL);
    result[2] = Tcl_NewDoubleObj(ERROR);
    result[3] = Tcl_NewDoubleObj(BND1);
    result[4] = Tcl_NewDoubleObj(BND2);
    result[5] = Tcl_NewIntObj(IVARBL);
    Tcl_SetObjResult(interp, Tcl_NewListObj(6, result));
    return TCL_OK;
}

tcl_routine(stat)
{
    double FMIN, FEDM, ERRDEF;
    int NPARI, NPARX, ISTAT;
    Tcl_Obj *result[6];
    
    tcl_require_objc(1);
    mnstat_(&FMIN, &FEDM, &ERRDEF, &NPARI, &NPARX, &ISTAT);
    result[0] = Tcl_NewDoubleObj(FMIN);
    result[1] = Tcl_NewDoubleObj(FEDM);
    result[2] = Tcl_NewDoubleObj(ERRDEF);
    result[3] = Tcl_NewIntObj(NPARI);
    result[4] = Tcl_NewIntObj(NPARX);
    result[5] = Tcl_NewIntObj(ISTAT);
    Tcl_SetObjResult(interp, Tcl_NewListObj(6, result));
    return TCL_OK;  
}

tcl_routine(errs)
{
    int NUM;
    double EPLUS, EMINUS, EPARAB, GLOBCC;
    Tcl_Obj *result[4];

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &NUM) != TCL_OK)
	return TCL_ERROR;
    mnerrs_(&NUM, &EPLUS, &EMINUS, &EPARAB, &GLOBCC);
    result[0] = Tcl_NewDoubleObj(EPLUS);
    result[1] = Tcl_NewDoubleObj(EMINUS);
    result[2] = Tcl_NewDoubleObj(EPARAB);
    result[3] = Tcl_NewDoubleObj(GLOBCC);
    Tcl_SetObjResult(interp, Tcl_NewListObj(4, result));
    return TCL_OK;  
}

tcl_routine(intr)
{
    Fit_config *fit;
    Minuit_user_function *fcn;

    tcl_require_objc(1);
    fit = hs_current_fit_conf();
    fcn = hs_get_current_fit_fcn();
    if (fit == NULL || fcn == NULL)
    {
	Tcl_SetResult(interp, "fit is not set up", TCL_STATIC);
	return TCL_ERROR;
    }
    fit->status = TCL_OK;
    fit->interp = interp;
    mnintr_(fcn, (void (*)())fit);
    return fit->status;
}

tcl_routine(inpu)
{
    int NUNIT, IERR;
    
    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &NUNIT) != TCL_OK)
	return TCL_ERROR;
    mninpu_(&NUNIT, &IERR);
    if (IERR)
    {
	Tcl_SetResult(interp, "buffer of unit numbers is full",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(cont)
{
    /* Usage: mncont NUM1 NUM2 NPT id ?close_contour?
     *    NUM1 and NUM2 are parameter numbers with respect
     *        to which the contour is to be determined.
     *    NPT is the number of points to use for drawing.
     *    id is a Histo-Scope id of an ntuple with 2 variables
     *        which will be used to display the contour.
     *    close_contour is an optional boolean argument which 
     *        tells whether the contour should be closed;
     *        the default value of this argument is 1.
     */
    int i, id, NUM1, NUM2, NPT, NFOUND, close_contour = 1;
    double *XPT = 0, *YPT = 0;
    float data[2];
    Fit_config *fit;
    Minuit_user_function *fcn;

    tcl_objc_range(5,6);
    if (Tcl_GetIntFromObj(interp, objv[1], &NUM1) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &NUM2) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &NPT) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &id) != TCL_OK)
	return TCL_ERROR;
    if (objc > 5)
	if (Tcl_GetBooleanFromObj(interp, objv[5], &close_contour) != TCL_OK)
	    return TCL_ERROR;
    switch (hs_type(id))
    {
    case HS_NTUPLE:
	if (hs_num_variables(id) != 2)
	{
	    Tcl_AppendResult(interp, "incompatible ntuple ",
			     "dimensionality (ntuple id = ",
			     Tcl_GetStringFromObj(objv[4], NULL),
			     ")", NULL);
	    return TCL_ERROR;
	}
	break;
    case HS_NONE:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[4], NULL),
			 " doesn't exist", NULL);
	return TCL_ERROR;
    default:
	Tcl_AppendResult(interp, "Histo-Scope item with id ",
			 Tcl_GetStringFromObj(objv[4], NULL),
			 " is not an ntuple", NULL);
	return TCL_ERROR;
    }
    fit = hs_current_fit_conf();
    fcn = hs_get_current_fit_fcn();
    if (fit == NULL || fcn == NULL)
    {
	Tcl_SetResult(interp, "fit is not set up", TCL_STATIC);
	return TCL_ERROR;
    }
    if (NUM1 < 0 || NUM2 < 0 || NPT < 5)
    {
	Tcl_SetResult(interp, "argument(s) out of range", TCL_STATIC);
	return TCL_ERROR;
    }
    XPT = (double *)malloc(NPT*sizeof(double));
    YPT = (double *)malloc(NPT*sizeof(double));
    if (XPT == NULL || YPT == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    fit->interp = interp;
    fit->status = TCL_OK;
    mncont_(fcn, &NUM1, &NUM2, &NPT, XPT, YPT, &NFOUND, (void (*)())fit);
    if (fit->status != TCL_OK)
	goto fail0;
    if (NFOUND > 0)
    {
	hs_reset(id);
	for (i=0; i<NFOUND; ++i)
	{
	    data[0] = XPT[i];
	    data[1] = YPT[i];
	    if (hs_fill_ntuple(id, data) != id)
	    {
		Tcl_SetResult(interp, "failed to fill ntuple", TCL_STATIC);
		goto fail0;
	    }
	}
	/* Need to close the contour */
	if (close_contour)
	{
	    hs_row_contents(id, 0, data);
	    if (hs_fill_ntuple(id, data) != id)
	    {
		Tcl_SetResult(interp, "failed to fill ntuple", TCL_STATIC);
		goto fail0;
	    }
	}
    }
    else
    {
	if (NFOUND < 0)
	    Tcl_SetResult(interp, "invalid argument values", TCL_STATIC);
	else
	    Tcl_SetResult(interp, "contouring algorithm failed", TCL_STATIC);
	goto fail0;
    }
    free(XPT);
    free(YPT);
    return TCL_OK;

 fail0:
    if (XPT)
	free(XPT);
    if (YPT)
	free(YPT);
    return TCL_ERROR;
}

tcl_routine(emat)
{
    int row, col, ndim;
    double *emat;
    Tcl_Obj **matrix;
    Tcl_Obj **column;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &ndim) != TCL_OK)
	return TCL_ERROR;
    if (ndim <= 0)
    {
	Tcl_SetResult(interp, "argument is not positive", TCL_STATIC);
	return TCL_OK;
    }
    emat = (double *)malloc(ndim*ndim*sizeof(double));
    if (emat == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    matrix = (Tcl_Obj **)malloc(ndim*sizeof(Tcl_Obj *));
    if (matrix == NULL)
    {
	free(emat);
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    column = (Tcl_Obj **)malloc(ndim*sizeof(Tcl_Obj *));
    if (column == NULL)
    {
	free(emat);
	free(matrix);
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    mnemat_(emat, &ndim);
    for (col=0; col<ndim; ++col)
    {
	for (row=0; row<ndim; ++row)
	    column[row] = Tcl_NewDoubleObj(emat[col*ndim + row]);
	matrix[col] = Tcl_NewListObj(ndim, column);
    }
    Tcl_SetObjResult(interp, Tcl_NewListObj(ndim, matrix));
    free(emat);
    free(matrix);
    free(column);
    return TCL_OK;
}

tcl_routine(maxcalls)
{
    int nfcn, mxnfcn;
    double dcovar;

    tcl_objc_range(1,2);
    if (objc == 1)
        mngetcommon_(&nfcn, &dcovar, &mxnfcn);
    else
    {
        if (Tcl_GetIntFromObj(interp, objv[1], &mxnfcn) != TCL_OK)
            return TCL_ERROR;
        mnsetnfcnmx_(&mxnfcn);
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(mxnfcn));
    return TCL_OK;
}

tcl_routine(nfcn)
{
    int nfcn, mxnfcn;
    double dcovar;

    tcl_require_objc(1);
    mngetcommon_(&nfcn, &dcovar, &mxnfcn);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(nfcn));
    return TCL_OK;
}

tcl_routine(dcovar)
{
    int nfcn, mxnfcn;
    double dcovar;

    tcl_require_objc(1);
    mngetcommon_(&nfcn, &dcovar, &mxnfcn);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(dcovar));
    return TCL_OK;
}

tcl_routine(fortranfile_open)
{
    char *name, *rwr;
    int i, lun, mode = 0, ierr = 1;
    unsigned int len;
    
    tcl_require_objc(3);
    rwr = Tcl_GetStringFromObj(objv[2], NULL);
    if (strcmp(rwr, "r") == 0)
	mode = 0;
    else if (strcmp(rwr, "w") == 0)
	mode = 1;
    else
    {
	Tcl_AppendResult(interp, "invalid mode \"", rwr, "\"", NULL);
	return TCL_ERROR;
    }
    name = Tcl_GetStringFromObj(objv[1], NULL);
    len = strlen(name);

    lun = -1;
    for (i=MINLUN; i<MAXLUN; ++i)
	if (busyluns[i] == 0)
	{
	    lun = i;
	    break;
	}
    if (lun < 0)
    {
	Tcl_SetResult(interp, "all logical units busy", TCL_STATIC);
	return TCL_ERROR;
    }
    mnopen_(name, &mode, &lun, &ierr, len);
    if (ierr)
    {
	Tcl_AppendResult(interp, "failed to open file \"", 
			 name, "\" for ", NULL);
	if (mode)
	    Tcl_AppendResult(interp, "writing",  NULL);
	else
	    Tcl_AppendResult(interp, "reading",  NULL);
	return TCL_ERROR;
    }
    busyluns[lun] = 1;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(lun));
    return TCL_OK;
}

tcl_routine(fortanfile_close)
{
    int lun = 0, ierr = 1;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &lun) != TCL_OK)
	return TCL_ERROR;
    if (lun == 5 || lun == 6 || lun == STDERR_LUN)
    {
	Tcl_AppendResult(interp, "can't close logical unit ",
			 Tcl_GetStringFromObj(objv[1], NULL), NULL);
	return TCL_ERROR;
    }
    mnclos_(&lun, &ierr);
    if (ierr)
    {
	Tcl_AppendResult(interp, "failed to close logical unit ",
			 Tcl_GetStringFromObj(objv[1], NULL), NULL);
	return TCL_ERROR;
    }
    if (lun >= MINLUN && lun < MAXLUN)
	busyluns[lun] = 0;
    return TCL_OK;
}

tcl_routine(fortranfile)
{
    char *command;
    if (objc < 3)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    command = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcmp(command, "open") == 0)
    {
	return tcl_c_name(fortranfile_open) (clientData, interp, objc-1, objv+1);
    }
    else if (strcmp(command, "close") == 0)
    {
	return tcl_c_name(fortanfile_close) (clientData, interp, objc-1, objv+1);
    }
    else
    {
	Tcl_AppendResult(interp, "invalid option \"", command, "\"", NULL);
	return TCL_ERROR;
    }    
}

static int get_standard_lun(char *channel)
{
    if (strcmp(channel, "stdin") == 0)
	return 5;
    else if (strcmp(channel, "stdout") == 0)
	return 6;
    else if (strcmp(channel, "stderr") == 0)
	return STDERR_LUN;
    else
	return -1;
}
