%{
#include <stdio.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include "../util/DialogF.h"
#include "../histo_util/hsTypes.h"
#include "interpret.h"
#include "variablePanel.h"
#include "parse.h"

typedef Symbol *SymPtr;
#define YYSTYPE SymPtr

/* Macros to add error processing to AddOp and AddSym calls */
#define ADD_OP(op) if (!AddOp(op, &ErrMsg)) return 1
#define ADD_SYM(sym) if (!AddSym(sym, &ErrMsg)) return 1
%}

%token NUMBER VAR BLTIN UNDEF

%right	'='
%left	OR
%left	AND
%left	GT GE LT LE EQ NE
%left	'+' '-'
%left	'*' '/'
%left	UNARYMINUS NOT
%right	POW

%%	/* Rules */

program:  asgns expr empty	{ ADD_OP(OP_STOP); return 0; }
	| error		  	{ return 1; }
	;
asgns:    /* nothing */
	| asgns '\n'
	| asgns asgn '\n' { ADD_OP(OP_POP); }
	;
asgn:     VAR '=' expr  { ADD_OP(OP_PUSH_VAR); ADD_SYM($1);
			  ADD_OP(OP_ASSIGN); $1->type = DEFINED; }
	;
expr:     NUMBER { ADD_OP(OP_PUSH_CONST); ADD_SYM($1); }
	| VAR    { ADD_OP(OP_PUSH_VAR); ADD_SYM($1); ADD_OP(OP_EVAL); }
	| asgn 
	| BLTIN '(' expr ')' { ADD_OP(OP_BUILTIN); ADD_SYM($1); }
	| '(' expr ')'
	| expr '+' expr { ADD_OP(OP_ADD); }
	| expr '-' expr { ADD_OP(OP_SUB); }
	| expr '*' expr { ADD_OP(OP_MUL); }
	| expr '/' expr { ADD_OP(OP_DIV); }
	| expr POW expr { ADD_OP(OP_POWER); }
	| '-' expr %prec UNARYMINUS  { ADD_OP(OP_NEGATE); }
	| expr GT expr  { ADD_OP(OP_GT); }
	| expr GE expr  { ADD_OP(OP_GE); }
	| expr LT expr  { ADD_OP(OP_LT); }
	| expr LE expr  { ADD_OP(OP_LE); }
	| expr EQ expr  { ADD_OP(OP_EQ); }
	| expr NE expr  { ADD_OP(OP_NE); }
	| expr AND expr { ADD_OP(OP_AND); }
	| expr OR expr  { ADD_OP(OP_OR); } 
	| NOT expr      { ADD_OP(OP_NOT); }
	;
empty:	  /* nothing */
	| empty '\n'
	;
	
%% /* User Subroutines Section */

/* Max. number of undefined symbols to print in the dialog */
#define MAX_UNDEFINED 15

static int follow(char expect, int ifyes, int ifno);

char *ErrMsg;
char *InputString;
char *InPtr;

/*
** Parse a null terminated string and create a program from it (this is the
** parser entry point).  The program created by this routine can be
** executed using ExecuteProgram.  Returns program on success, or NULL
** on failure.  If the command failed, the error message is returned
** as a pointer to a static string in msg, and the length of the string up
** to where parsing failed in failedAt.
*/
Program *ParseExpr(char *expr, char **msg, int *failedAt)
{
    Program *prog;
    Symbol *s, *toCheckList;
    
    BeginCreatingProgram();
    
    /* call yyparse to parse the string and check for success.  If the parse
       failed, return the error message and string index (the grammar aborts
       parsing at the first error) */
    InputString = expr;
    InPtr = InputString;
    if (yyparse()) {
    	*msg = ErrMsg;
    	*failedAt = InPtr - InputString;
    	FreeProgram(FinishCreatingProgram());
    	return NULL;
    }
    
    /* check that everything was parsed */
    if (*--InPtr != '\0') {
    	*msg = "Syntax Error";
    	*failedAt = InPtr - InputString;
    	FreeProgram(FinishCreatingProgram());
    	return NULL;
    }
    
    /* get the newly created program */
    prog = FinishCreatingProgram();
    
    /* move the symbols which are now defined out of the external symbol
       table and in to the local symbol table */
    toCheckList = prog->externSymList;
    prog->externSymList = NULL;
    while (toCheckList != NULL) {
    	s = toCheckList;
    	toCheckList = s->next;
    	if (s->type == DEFINED) {
    	    s->next = prog->localSymList;
    	    prog->localSymList = s;
    	} else {
    	    s->next = prog->externSymList;
    	    prog->externSymList = s;
    	}
    }
    
    /* parse succeeded */
    *msg = "";
    *failedAt = 0;
    return prog;
}


/*
** Resolve ntuple and user defined variable names in the program "prog".
*/
void ResolveVariableReferences(Program *prog, hsNTuple *ntuple,
	ntupleExtension *ntExt)
{
    Symbol *s;
    int i;
    
    /* Look thru all of the symbols in the symbol table, and try to resolve
       the undefined ones to ntuple variable names.  When found, change their
       symbol type for the interpreter to NT_VAR or USER_DEF_VAR, and
       record their variable index in the symbol */
    for (s=prog->externSymList; s!=NULL; s=s->next) {
	s->type = UNDEFINED;
	for (i=0; i<ntuple->nVariables; i++) {
	    if (!strcmp(s->name, ntuple->names[i])) {
	    	s->type = NT_VAR;
	    	s->index = i;
	    }
	}
	if (ntExt != NULL) {
	    for (i=0; i<ntExt->nVars; i++) {
		if (!strcmp(s->name, ntExt->vars[i].name)) {
	    	    s->type = USER_DEF_VAR;
	    	    s->index = i;
		}
    	    }
    	}
    }
}

/*
** Check for remaining undefined symbols in program prog,
** and warn the user with a nice dialog if there are any.
*/
int WarnUndefined(Program *prog, Widget parent)
{
    Symbol *s;
    char *undefinedSyms[MAX_UNDEFINED], *undefinedString;
    int nUndefined = 0;
    int i, nWritten, len = 0, pos = 0;
    
    /* Look thru all of the symbols in the symbol table, and count the
       undefined ones.  Also record a list of the first MAX_UNDEFINED ones
       to report to the user in a dialog */
    for (s = prog->externSymList; s != NULL; s = s->next) {
    	if (s->type == UNDEFINED) {
    	    if (nUndefined < MAX_UNDEFINED)
    		undefinedSyms[nUndefined] = s->name;
    	    nUndefined++;
    	}
    }
    
    /* Report the undefined symbols to the user */
    if (nUndefined > 0) {
        for (i=0; i<nUndefined && i<MAX_UNDEFINED; i++)
            len += strlen(undefinedSyms[i]) + 1;
        undefinedString = (char *)XtMalloc(sizeof(char)*(len + 7));
        for (i=0; i<nUndefined && i<MAX_UNDEFINED; i++) {
            sprintf(&undefinedString[pos], "%s\n%n", undefinedSyms[i],
            	    &nWritten);
            pos += nWritten;
        }
        if (nUndefined > MAX_UNDEFINED) {
            sprintf(&undefinedString[pos], ".\n.\n.\n%n", &nWritten);
            pos += nWritten;
        }
        undefinedString[pos-1] = '\0'; /* remove trailing newline */
    	DialogF(DF_WARN, parent, 1,
    	    	"%d undefined symbol(s) in expression:\n%s",
    	    	"Acknowledged", nUndefined, undefinedString);
	XtFree(undefinedString);
	return False;
    }
    return True;
}

int yylex()
{
    char c;
    int len;
    
    /* skip whitespace */
    while ((c=*InPtr++) == ' ' || c == '\t');
    
    /* return end of input at the end of the string */
    if (c == '\0')
	return 0;
    
    /* process number tokens */
    if (c == '.' || isdigit(c))  { /* number */
        double d;
        InPtr--;
        sscanf(InPtr, "%lf%n", &d, &len);
        InPtr += len;
        yylval = InstallSymbol("", CONSTANT, d);
        return NUMBER;
    }
    
    /* process symbol tokens */
    if (isalpha(c)) {
        Symbol *s;
        char sbuf[100], *p = sbuf;
        do {
	    if (p < sbuf + sizeof(sbuf) -1 ) /* truncate if too long */
		*p++=c;
	} while ((c=*InPtr++) != EOF && isalnum(c));
	InPtr--;
	*p = '\0';
	if ((s=LookupSymbol(sbuf)) == 0)
            s = InstallSymbol(sbuf, UNDEFINED, 0.0);
	yylval = s;
        if (s->type == FUNCTION)
            return BLTIN;
        else
            return VAR;
    }
    
    /* process quoted string symbols */
    if (c == '\"') {
        Symbol *s;
        char sbuf[100], *p = sbuf;
        while ((c=*InPtr++) != EOF && c != '\"') {
	    if (p < sbuf + sizeof(sbuf) -1) /* truncate if too long */
		*p++=c;
	}
	*p = '\0';
	if ((s=LookupSymbol(sbuf)) == 0)
            s = InstallSymbol(sbuf, UNDEFINED, 0.0);
	yylval = s;
        return VAR;
    }
    
    /* process remaining two character tokens or return single char as token */
    switch(c) {
    case '>':	return follow('=', GE, GT);
    case '<':	return follow('=', LE, LT);
    case '=':	return follow('=', EQ, '=');
    case '!':	return follow('=', NE, NOT);
    case '|':	return follow('|', OR, '|');
    case '&':	return follow('&', AND, '&');
    case '*':	return follow('*', POW, '*');
    case '^':	return POW;
    default:	return c;
    }
}

/*
** look ahead for >=, etc.
*/
static int follow(char expect, int ifyes, int ifno)
{
    if (*InPtr++ == expect)
	return ifyes;
    InPtr--;
    return ifno;
}

/*
** Called by yacc to report errors (just stores for returning when
** parsing is aborted.  The error token action is to immediate abort
** parsing, so this message is immediately reported to the caller
** of ParseExpr)
*/
void yyerror(char *s)
{
    ErrMsg = s;
}
