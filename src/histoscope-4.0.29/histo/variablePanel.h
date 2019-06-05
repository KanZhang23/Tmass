/*******************************************************************************
*									       *
* variablePanel.h -- Histoscope create/modify variable window		       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* May 12, 1995								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
/* These constraints are mostly for the configuration file reader.  I think
   the rest of the code can handle an unlimited number of variables and
   expressions of any length */
#define MAX_USER_VARS 99
#define MAX_EXPR_LENGTH 512

typedef struct {
    char *name;
    char *expression;
    Program *program;
} userDefNTVar;

typedef struct _ntupleExtension {
    struct _ntupleExtension *next;
    int ntupleID;
    userDefNTVar *vars;
    int nVars;
} ntupleExtension;

int AddUserVariable(hsNTuple *ntuple, char *name, char *expression,
	Program *program);
void DeleteUserVariable(hsNTuple *ntuple, ntupleExtension *ntExt, char *name);
int TestUserVariable(hsNTuple *ntuple, ntupleExtension *ntExt, int varIndex,
	char **errMsg);
int CheckForCircularReferences(ntupleExtension *ntExt, char **errVar);
ntupleExtension *GetNTupleExtension(int id);
int ExtNTRef(hsNTuple *ntuple, ntupleExtension *ntExt, int var, int index,
	float *value);
int ResetExtNTRefErrors(void);
int CheckExtNTRefErrors(char **lastMessage);
void RemoveNTupleExtension(int id);
char *ExtNTVarName(hsNTuple *ntuple, int var);
void ExtCalcNTVarRange(hsNTuple *ntuple, ntupleExtension *ntExt, int var,
	float *minReturn, float *maxReturn);
void CreateVariablePanel(Widget parent, hsNTuple *ntuple, char *name);
void UpdateVariablePanels(hsNTuple *ntuple, ntupleExtension *ntExt);
void ChangeNTupleExtensionItemID(int oldID, int newID);
void ChangeVariablePanelItemIDs(int oldID, int newID);
void CloseVariablePanelsWithItem(int id);
void WriteNTupleExtensions(FILE *fp);
