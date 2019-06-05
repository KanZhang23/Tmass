#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#ifdef notdef
#include <signal.h>
#include <setjmp.h>
#endif
#include <errno.h>
#include "interpret.h"

#define STACK_SIZE 512		/* Maximum stack size */
#define PROGRAM_SIZE  4096	/* Maximum program size */
#define MAX_ERR_MSG_LEN 256	/* Max. length for error messages */

#ifdef VMS
#ifndef M_E
#define M_E 2.7182818284590452354
#endif /*M_E*/
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /*M_PI*/
#endif /*VMS*/

/* data type for stack elements */
typedef union datum {
        double val;
        Symbol *sym;
} datum;

static void push(datum d);
static datum pop(void);
static int popOp(void);
static int constpush(void);
static int varpush(void);
static int eval(void);
static int add(void);
static int sub(void);
static int mul(void);
static int divide(void);
static int negate(void);
static int gt(void);
static int lt(void);
static int ge(void);
static int le(void);
static int eq(void);
static int ne(void);
static int and(void);
static int or(void);
static int not(void);
static int power(void);
static int assign(void);
static int print(void);
static int bltin(void);
static int sinFn(void);
static int cosFn(void);
static int tanFn(void);
static int asinFn(void);
static int acosFn(void);
static int atanFn(void);
static int absFn(void);
static int logFn(void);
static int log10Fn(void);
static int expFn(void);
static int sqrtFn(void);
static int intFn(void);
static int randFn(void);
static void freeSymbolTable(Symbol *symTab);
static int errCheck(char *s);
static int execError(char *s1, char *s2);

/* Global data for accumulating programs */
Symbol *PermSymList = NULL;	/* permanent symbol table linked list */
Symbol *LocalSymList = NULL;	/* symbols local to the program */
Symbol *ExternSymList = NULL;	/* external references (ntuples) & undefineds */
static Inst Prog[PROGRAM_SIZE];	/* the program */
static Inst *ProgP;		/* next free spot for code generation */

/* Global data for the interpreter */
static datum Stack[STACK_SIZE];	/* the stack */
static datum *StackP;		/* next free spot on stack */
static Inst *PC;		/* program counter during execution */
static char *ErrMsg;		/* global for returning error messages from
				   executing functions */

/* Array for mapping operations to functions for performing the operations
   Must correspond to the enum called "operations" in interpret.h */
static int (*OpFns[N_OPS])() = {NULL, popOp, constpush, varpush, eval, add,
    sub, mul, divide, negate, gt, lt, ge, le, eq, ne,
    and, or, not, power, assign, print, bltin};

/* Constants */
static struct  {
    char   *name;
    double cval;
} consts[] = {
    {"pi",    M_PI},
    {"e",     M_E},
    {"gamma", 0.57721566490153286060},	/* Euler */
    {"deg",  57.29577951308232087860},	/* deg/radian */
    {0,       0}
};

/* Built-ins */
static struct  {
    char *name;
    int (*func)(void);
} builtins[] = {
    {"sin", sinFn},
    {"cos", cosFn},
    {"tan", tanFn},
    {"asin", asinFn},
    {"acos", acosFn},
    {"atan",atanFn},
    {"log", logFn},
    {"log10", log10Fn},
    {"exp", expFn},
    {"sqrt", sqrtFn},
    {"int", intFn},
    {"abs", absFn},
    {"rand", randFn},
    {0,     0}
};

/*
** To build a program for the interpreter, call BeginCreatingProgram, to
** begin accumulating the program, followed by calls to AddOp, AddSym,
** and InstallSymbol to add symbols and operations.  When the new program
** is finished, collect the results with FinishCreatingProgram.  This returns
** a self contained program that can be run with ExecuteProgram.
*/

/*
** Start collecting instructions for a program. Clears the program
** and the symbol table.
*/
void BeginCreatingProgram(void)
{ 
    int i;
    Symbol *s;

    /* Initialize the permanent symbol table and clear out
       any old symbol table contents that might still exist */
    if (PermSymList == NULL) {
	for (i = 0; consts[i].name; i++)
	    InstallSymbol(consts[i].name, SYMBOLIC_CONSTANT, consts[i].cval);
	for (i = 0; builtins[i].name; i++) {
	    s = InstallSymbol(builtins[i].name, FUNCTION, 0.0);
	    s->u.ptr = builtins[i].func;
	}
    }
    freeSymbolTable(LocalSymList);
    LocalSymList = NULL;
    freeSymbolTable(ExternSymList);
    ExternSymList = NULL;
    
    /* Begin collecting instructions in the array Prog */
    ProgP = Prog;
}

/*
** Finish up the program under construction, and return it (code and
** symbol table) as a package that ExecuteProgram can execute.  This
** program must be freed with FreeProgram.
*/
Program *FinishCreatingProgram(void)
{
    Program *newProg;
    int progLen;
    
    newProg = (Program *)malloc(sizeof(Program));
    progLen = ((char *)ProgP) - ((char *)Prog);
    newProg->code = (Inst *)malloc(progLen);
    memcpy(newProg->code, Prog, progLen);
    newProg->localSymList = LocalSymList;
    LocalSymList = NULL;
    newProg->externSymList = ExternSymList;
    ExternSymList = NULL;
    return newProg;
}

void FreeProgram(Program *prog)
{
    freeSymbolTable(prog->localSymList);
    freeSymbolTable(prog->externSymList);
    free(prog->code);
    free(prog);    
}

/*
** Add an operator (instruction) to the end of the current program
*/
int AddOp(int op, char **msg)
{
    /* Inst *oProgP = ProgP; */

    if (ProgP >= &Prog[PROGRAM_SIZE]) {
	*msg = "expression too large";
	return 0;
    }
    *ProgP++ = OpFns[op];
    return 1;
}

/*
** Add an operand (a symbol) to the current program
*/
int AddSym(Symbol *sym, char **msg)
{
    /* Inst *oProgP = ProgP; */
    
    if (ProgP >= &Prog[PROGRAM_SIZE]) {
	*msg = "expression too large";
	return 0;
    }
    *ProgP++ = (Inst)sym;
    return 1;
}

/*
** Execute the current program.  Note that this assumes that the symbol
** values in userProg have already been assigned.
**
** Note that the interpreter is not at all optimized.  To speed it up:
**
**   1) remove stack overflow/underflow checking,
**   2) make push and pop macros,
**   3) use setjmp and longjmp for errors instead of returning status values
**      and checking at every instruction
*/
int ExecuteProgram(Program *userProg, double *result, char **msg)
{
    StackP = Stack;
    for (PC = userProg->code; *PC != NULL; ) {
	if (!(*(*PC++))()) {
	    if (msg != NULL)
	    	*msg = ErrMsg;
	    *result = 0.;
	    return 0;
	}
    }
    *result = pop().val;
    *msg = "";
    return 1;
}

/*
** find a symbol in the symbol table
*/
Symbol *LookupSymbol(char *name)
{
    Symbol *s;

    for (s = PermSymList; s != NULL; s = s->next)
	if (strcasecmp(s->name, name) == 0)
	    return s;
    for (s = LocalSymList; s != NULL; s = s->next)
	if (strcasecmp(s->name, name) == 0)
	    return s;
    for (s = ExternSymList; s != NULL; s = s->next)
	if (strcasecmp(s->name, name) == 0)
	    return s;
    return 0;      /* 0 ==> not found */
}

/*
** install s in symbol table
*/
Symbol *InstallSymbol(char *name, int type, double value)
{
    Symbol *s;

    s = (Symbol *)malloc(sizeof(Symbol));
    s->name = (char *)malloc(strlen(name)+1); /* +1 for '\0' */
    strcpy(s->name, name);
    s->type = type;
    s->u.val = value;
    if (type == SYMBOLIC_CONSTANT || type == FUNCTION) {
    	s->next = PermSymList;
    	PermSymList = s;
    } else if (type == CONSTANT || type == DEFINED) {
    	s->next = LocalSymList;
    	LocalSymList = s;
    } else if (type == NT_VAR || type == USER_DEF_VAR || type == UNDEFINED) {
        s->next = ExternSymList;
    	ExternSymList = s;
    }
    return s;
}

static void freeSymbolTable(Symbol *symTab)
{
    Symbol *s;
    
    while(symTab != NULL) {
    	s = symTab;
    	free(s->name);
    	symTab = s->next;
    	free((char *)s);
    }    
}

static void push(datum d)
{
    if (StackP >= &Stack[STACK_SIZE]) {
	fprintf(stderr, "expression stack overflow\n");
	return;
    }
    *StackP++ = d;
}

static datum pop(void)
{
    if (StackP == Stack) {
	fprintf(stderr, "expression stack underflow\n");
	return *StackP;
    }
    return *--StackP;
}

static int popOp()
{
    pop();
    return 1;
}

static int constpush()
{
    datum d;
    d.val = ((Symbol *)*PC++)->u.val;
    push(d);
    return 1;
}

static int varpush()
{
    datum d;
    d.sym = (Symbol *)(*PC++);
    push(d);
    return 1;
}

static int eval()    /* evaluate variable on stack */
{
    datum d;
    d = pop();
    switch (d.sym->type) {
    	case DEFINED:
    	case CONSTANT:
    	case SYMBOLIC_CONSTANT:
    	case NT_VAR:
    	case USER_DEF_VAR:
    	    d.val = d.sym->u.val;
    	    break;
    	case UNDEFINED:
    	    return execError("undefined variable: ", d.sym->name);
    	default:
    	    return execError("attempt to evaluate non-variable: ", d.sym->name);
    }
    push(d);
    return 1;
}
static int add()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val += d2.val;
    push(d1);
    return 1;
}
static int sub()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val -= d2.val;
    push(d1);
    return 1;
}

static int mul()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val *= d2.val;
    push(d1);
    return 1;
}

static int divide()
{
    datum d1, d2;
    d2 = pop();
    if (d2.val == 0.0) 
    return execError("division by zero", "");
    d1 = pop();
    d1.val /= d2.val;
    push(d1);
    return 1;
}

static int negate()
{
    datum d;
    d = pop();
    d.val = -d.val;
    push(d);
    return 1;
}

static int gt()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val > d2.val);
    push(d1);
    return 1;
}

static int lt()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val < d2.val);
    push(d1);
    return 1;
}

static int ge()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val >= d2.val);
    push(d1);
    return 1;
}

static int le()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val <= d2.val);
    push(d1);
    return 1;
}

static int eq()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val == d2.val);
    push(d1);
    return 1;
}

static int ne()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != d2.val);
    push(d1);
    return 1;
}

static int and()
{ 
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != 0.0 && d2.val != 0.0);
    push(d1);
    return 1;
}

static int or()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != 0.0 || d2.val != 0.0);
    push(d1);
    return 1;
}
    
static int not()
{
    datum d;
    d = pop();
    d.val = (double)(d.val == 0.0);
    push(d);
    return 1;
}

static int power()
{
    datum d1, d2;
    d2 = pop();
    d1 = pop();
    errno = 0;
    d1.val = pow(d1.val, d2.val);
    push(d1);
    return errCheck("exponentiation");
}

static int assign()      /* assign top value to next value */
{
    datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != DEFINED && d1.sym->type != UNDEFINED)
	 return execError("assignment to non-variable: ", d1.sym->name);
    d1.sym->u.val = d2.val;
    d1.sym->type = DEFINED;
    push(d2);
    return 1;
}
static int print()       /* pop top value from stack, print it */
{
    datum d;
    d = pop();
    printf("\t%.8g\n", d.val);
    return 1;
}

static int bltin()
{
    return ((Symbol *)*PC++)->u.ptr();
}

/*
** Built in functions
*/
static int sinFn(void)
{
    datum d;
    d = pop();
    d.val = sin(d.val);
    push(d);
    return 1;
}
static int cosFn(void)
{
    datum d;
    d = pop();
    d.val = cos(d.val);
    push(d);
    return 1;
}
static int tanFn(void)
{
    datum d;
    d = pop();
    d.val = tan(d.val);
    push(d);
    return 1;
}
static int asinFn(void)
{
    datum d;
    d = pop();
    d.val = asin(d.val);
    push(d);
    return 1;
}
static int acosFn(void)
{
    datum d;
    d = pop();
    d.val = acos(d.val);
    push(d);
    return 1;
}
static int atanFn(void)
{
    datum d;
    d = pop();
    d.val = atan(d.val);
    push(d);
    return 1;
}
static int absFn(void)
{
    datum d;
    d = pop();
    d.val = fabs(d.val);
    push(d);
    return 1;
}
static int logFn(void)
{
    datum d;
    d = pop();
    errno = 0;
    d.val = log(d.val);
    push(d);
    return errCheck("log");
}
static int log10Fn(void)
{
    datum d;
    d = pop();
    errno = 0;
    d.val = log10(d.val);
    push(d);
    return errCheck("log10");
}
static int expFn(void)
{
    datum d;
    d = pop();
    errno = 0;
    d.val = exp(d.val);
    push(d);
    return errCheck("exp");
}
static int sqrtFn(void)
{
    datum d;
    d = pop();
    errno = 0;
    d.val = sqrt(d.val);
    push(d);
    return errCheck("sqrt");
}
static int intFn(void)
{
    datum d;
    d = pop();
    d.val = (double)(long)d.val;
    push(d);
    return 1;
}
static int randFn(void)
{
    datum d;
    d = pop();
    d.val = (double)(long)d.val * ((double)rand()/RAND_MAX);
    push(d);
    return 1;
}

/*
** checks errno after operations which can set it.  If an error occured,
** creates appropriate error messages and returns false
*/
static int errCheck(char *s)
{
    if (errno == EDOM)
	return execError(s, " argument out of domain");
    else if (errno == ERANGE)
	return execError(s, " result out of range");
    return 1;
}

/*
** combine two strings in a static area and set ErrMsg to point to the
** result.  Returns false so a single return execError() statement can
** be used to both process the message and return.
*/
static int execError(char *s1, char *s2)
{
    static char msg[MAX_ERR_MSG_LEN];
    
    strcpy(msg, s1);
    strcat(msg, s2);
    ErrMsg = msg;
    return 0;
}
