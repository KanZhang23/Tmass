#include "histo_tcl_api.h"
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "hsTypes.h"
#include "hsFile.h"

extern void FreeItem (hsGeneral *item);

tcl_routine(dir)
{
    char *filename;
    Tcl_Obj *list;
    Tcl_Obj *iteminfo[4];
    hsFile *hsfile;
    char *errString = NULL;
    int i, stat, numItems, nfitFlag;
    hsGeneral *item;
    extern const char * const hs_typename_strings[];

    tcl_require_objc(2);
    filename = Tcl_GetStringFromObj(objv[1], NULL);
    hsfile = OpenHsFile(filename, HS_READ, &errString);
    if (errString)
    {
	Tcl_SetResult(interp, errString, TCL_VOLATILE);
    	free(errString);
	return TCL_ERROR;
    }
    if (hsfile == NULL)
    {
	/* This should not really happen */
	Tcl_SetResult(interp,
		      "Invalid file handle at 0. This is a bug. Please report.",
		      TCL_VOLATILE);
	return TCL_ERROR;
    }

    stat = DirHsFile(&hsfile, &nfitFlag, &errString);
    if (errString != NULL) {
	Tcl_SetResult(interp, errString, TCL_VOLATILE);
    	free(errString);
	return TCL_ERROR;
    }
    if (hsfile == NULL)
    {
	/* This should not really happen */
	Tcl_SetResult(interp,
		      "Invalid file handle at 1. This is a bug. Please report.",
		      TCL_VOLATILE);
	return TCL_ERROR;
    }

    list = Tcl_NewListObj(0, NULL);
    numItems = hsfile->fileversion < HS_FILE_C ? stat : hsfile->numOfItems;
    for (i = 0; i < numItems && i < stat; ++i) 
    {
	item = hsfile->locTblMem[i];
	iteminfo[0] = Tcl_NewIntObj(item->uid);
	iteminfo[1] = Tcl_NewStringObj(item->category, -1);
	iteminfo[2] = Tcl_NewStringObj(item->title, -1);
	iteminfo[3] = Tcl_NewStringObj(hs_typename_strings[item->type], -1);
	Tcl_ListObjAppendElement(interp, list, Tcl_NewListObj(4, iteminfo));
	FreeItem(item);
    }
    
    Tcl_SetObjResult(interp, list);
    FreeHsFileStruct(&hsfile);
    return TCL_OK;
}
