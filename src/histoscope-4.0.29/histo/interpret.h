enum symTypes {CONSTANT, FUNCTION, SYMBOLIC_CONSTANT, NT_VAR,
	USER_DEF_VAR, DEFINED, UNDEFINED};
#define N_OPS 23
enum operations {OP_STOP, OP_POP, OP_PUSH_CONST, OP_PUSH_VAR, OP_EVAL, OP_ADD,
    OP_SUB, OP_MUL, OP_DIV, OP_NEGATE, OP_GT, OP_LT, OP_GE, OP_LE, OP_EQ, OP_NE,
    OP_AND, OP_OR, OP_NOT, OP_POWER, OP_ASSIGN, OP_PRINT, OP_BUILTIN};

/* symbol table entry */
typedef struct SymbolRec {
    char *name;
    short type;
    short index;	/* NT_VAR or USER_DEF_VAR, ntuple index */
    union {
       double val;	/* value of CONSTANT, DEFINED, NT_VAR or USER_DEF_VAR */
       int (*ptr)();    /* FUNCTION */
    } u;
    struct SymbolRec *next;     /* to link to another */  
} Symbol;

typedef int (*Inst)();

typedef struct {
    Symbol *localSymList;
    Symbol *externSymList;
    Inst *code;
} Program;

/* Routines for creating a program, (accumulated beginning with
   BeginCreatingProgram and returned via FinishCreatingProgram) */
void BeginCreatingProgram(void);
int AddOp(int op, char **msg);
int AddSym(Symbol *sym, char **msg);
Symbol *LookupSymbol(char *name);
Symbol *InstallSymbol(char *name, int type, double value);
Program *FinishCreatingProgram(void);
void FreeProgram(Program *prog);

/* Interpreter entry point, assumes that symbol values have already been
   assigned */
int ExecuteProgram(Program *userProg, double *result, char **msg);
