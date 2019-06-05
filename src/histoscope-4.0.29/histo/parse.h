#define MAX_DEGREE 20

Program *ParseExpr(char *expr, char **msg, int *failedAt);
void ResolveVariableReferences(Program *prog, hsNTuple *ntuple,
	ntupleExtension *ntExt);
int WarnUndefined(Program *prog, Widget parent);
