#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20111219

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

#define YYPREFIX "yy"

#define YYPURE 0

#line 2 "parse.y"
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
#line 35 "parse.c"

#ifndef YYSTYPE
typedef int YYSTYPE;
#endif

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#define YYERROR_DECL() yyerror(const char *s)
#define YYERROR_CALL(msg) yyerror(msg)

extern int YYPARSE_DECL();

#define NUMBER 257
#define VAR 258
#define BLTIN 259
#define UNDEF 260
#define OR 261
#define AND 262
#define GT 263
#define GE 264
#define LT 265
#define LE 266
#define EQ 267
#define NE 268
#define UNARYMINUS 269
#define NOT 270
#define POW 271
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    1,    1,    1,    4,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    3,    3,
};
static const short yylen[] = {                            2,
    3,    1,    0,    2,    3,    3,    1,    1,    1,    4,
    3,    3,    3,    3,    3,    3,    2,    3,    3,    3,
    3,    3,    3,    3,    3,    2,    0,    2,
};
static const short yydefred[] = {                         0,
    2,    0,    0,    7,    0,    0,    0,    0,    4,    0,
    0,    0,    0,    0,    0,    9,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    5,    0,    0,   11,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   28,
   10,
};
static const short yydgoto[] = {                          2,
    3,   11,   32,   16,
};
static const short yysindex[] = {                      -251,
    0,    0,  -10,    0,  -54,  -31,  -13,  -13,    0,  -13,
  146,    6,  -13,  -13, -246,    0, -246,  -39,  -13,  -13,
  -13,  -13,  -13,  -13,  -13,  -13,  -13,  -13,  -13,  -13,
  -13,   19,    0,  146,  -28,    0,  156,  165,  -21,  -21,
  -21,  -21,  -21,  -21,   11,   11, -246, -246, -246,    0,
    0,
};
static const short yyrindex[] = {                       145,
    0,    0,    0,    0,    1,    0,    0,    0,    0,    0,
   18,   12,    0,    0,   23,    0,   31,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   34,    0,   75,    0,    0,   10,  139,   88,   99,
  107,  115,  123,  131,   69,   77,   39,   50,   61,    0,
    0,
};
static const short yygindex[] = {                         0,
    0,  427,    0,   33,
};
#define YYTABLESIZE 458
static const short yytable[] = {                          9,
    8,   36,   29,   27,    1,   28,   13,   30,   14,   25,
    8,    9,   51,   29,   27,   33,   28,   27,   30,   25,
   29,   27,   17,   28,   31,   30,   10,   27,   50,   10,
   26,    7,   17,    1,    7,   12,    0,    0,   14,    0,
   26,    8,    8,    8,    0,    8,    0,    8,   14,   15,
   25,    0,   29,    9,    9,    0,    9,   30,    9,   15,
   16,    0,    0,   17,   17,   17,    0,   17,   12,   17,
   16,   26,   26,   26,    6,   26,   13,   26,   12,   14,
   14,   14,    0,   14,    6,   14,   13,   18,    0,    0,
   15,   15,   15,    0,   15,    0,   15,   18,   19,    0,
    0,   16,   16,   16,    0,   16,   20,   16,   19,   12,
    0,   12,    0,   12,   21,    6,   20,   13,    0,   13,
    0,   13,   22,    0,   21,    0,    0,    0,   18,    0,
   23,    0,   22,    0,    0,    0,    0,    0,   24,   19,
   23,    0,    0,    0,    0,    0,    0,   20,   24,    0,
    0,    0,    0,    0,    3,   21,    0,    0,    0,    0,
    0,    0,    0,   22,    0,    0,    0,    0,    0,    0,
    0,   23,    0,    0,    0,    0,    0,    0,    0,   24,
    0,    0,    0,    0,    3,    0,    0,   29,   27,    3,
   28,    0,   30,    0,    0,    0,    0,   29,   27,    0,
   28,    0,   30,    0,    0,    0,   29,   27,    0,   28,
    0,   30,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   19,   20,   21,   22,   23,   24,   25,   26,    0,
    0,   31,   19,   20,   21,   22,   23,   24,   25,   26,
    0,    0,   31,    4,    5,    6,    4,    5,    6,   31,
    0,    0,    0,    0,    0,    0,    8,    0,    0,    8,
    0,    8,    8,    8,    8,    8,    8,    8,    8,    0,
   25,    8,    9,    9,    9,    9,    9,    9,    9,    9,
    0,   31,    9,   17,   17,   17,   17,   17,   17,   17,
   17,   26,   26,   26,   26,   26,   26,   26,   26,   14,
   14,   14,   14,   14,   14,   14,   14,    0,    0,    0,
   15,   15,   15,   15,   15,   15,   15,   15,    0,    0,
    0,   16,   16,   16,   16,   16,   16,   16,   16,   12,
   12,   12,   12,   12,   12,   12,   12,   13,   13,   13,
   13,   13,   13,   13,   13,    0,    0,    0,   18,   18,
   18,   18,   18,   18,   18,   18,    0,    0,    0,   19,
   19,   19,   19,   19,   19,   19,   19,   20,   20,   20,
   20,   20,   20,   20,   20,   21,   21,   21,   21,   21,
   21,   21,   21,   22,   22,   22,   22,   22,   22,   22,
   22,   23,   23,   23,   23,   23,   23,   23,   23,   24,
   24,    3,    3,    3,    0,    0,   19,   20,   21,   22,
   23,   24,   25,   26,    3,    0,   31,   20,   21,   22,
   23,   24,   25,   26,    0,    0,   31,   21,   22,   23,
   24,   25,   26,   15,   17,   31,   18,    0,    0,   34,
   35,    0,    0,    0,    0,   37,   38,   39,   40,   41,
   42,   43,   44,   45,   46,   47,   48,   49,
};
static const short yycheck[] = {                         10,
    0,   41,   42,   43,  256,   45,   61,   47,   40,    0,
   10,    0,   41,   42,   43,   10,   45,    0,   47,   10,
   42,   43,    0,   45,  271,   47,   40,   10,   10,   40,
    0,   45,   10,    0,   45,    3,   -1,   -1,    0,   -1,
   10,   41,   42,   43,   -1,   45,   -1,   47,   10,    0,
   41,   -1,   42,   42,   43,   -1,   45,   47,   47,   10,
    0,   -1,   -1,   41,   42,   43,   -1,   45,    0,   47,
   10,   41,   42,   43,    0,   45,    0,   47,   10,   41,
   42,   43,   -1,   45,   10,   47,   10,    0,   -1,   -1,
   41,   42,   43,   -1,   45,   -1,   47,   10,    0,   -1,
   -1,   41,   42,   43,   -1,   45,    0,   47,   10,   41,
   -1,   43,   -1,   45,    0,   41,   10,   41,   -1,   43,
   -1,   45,    0,   -1,   10,   -1,   -1,   -1,   41,   -1,
    0,   -1,   10,   -1,   -1,   -1,   -1,   -1,    0,   41,
   10,   -1,   -1,   -1,   -1,   -1,   -1,   41,   10,   -1,
   -1,   -1,   -1,   -1,   10,   41,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   40,   -1,   -1,   42,   43,   45,
   45,   -1,   47,   -1,   -1,   -1,   -1,   42,   43,   -1,
   45,   -1,   47,   -1,   -1,   -1,   42,   43,   -1,   45,
   -1,   47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  261,  262,  263,  264,  265,  266,  267,  268,   -1,
   -1,  271,  261,  262,  263,  264,  265,  266,  267,  268,
   -1,   -1,  271,  257,  258,  259,  257,  258,  259,  271,
   -1,   -1,   -1,   -1,   -1,   -1,  270,   -1,   -1,  270,
   -1,  261,  262,  263,  264,  265,  266,  267,  268,   -1,
  261,  271,  261,  262,  263,  264,  265,  266,  267,  268,
   -1,  271,  271,  261,  262,  263,  264,  265,  266,  267,
  268,  261,  262,  263,  264,  265,  266,  267,  268,  261,
  262,  263,  264,  265,  266,  267,  268,   -1,   -1,   -1,
  261,  262,  263,  264,  265,  266,  267,  268,   -1,   -1,
   -1,  261,  262,  263,  264,  265,  266,  267,  268,  261,
  262,  263,  264,  265,  266,  267,  268,  261,  262,  263,
  264,  265,  266,  267,  268,   -1,   -1,   -1,  261,  262,
  263,  264,  265,  266,  267,  268,   -1,   -1,   -1,  261,
  262,  263,  264,  265,  266,  267,  268,  261,  262,  263,
  264,  265,  266,  267,  268,  261,  262,  263,  264,  265,
  266,  267,  268,  261,  262,  263,  264,  265,  266,  267,
  268,  261,  262,  263,  264,  265,  266,  267,  268,  261,
  262,  257,  258,  259,   -1,   -1,  261,  262,  263,  264,
  265,  266,  267,  268,  270,   -1,  271,  262,  263,  264,
  265,  266,  267,  268,   -1,   -1,  271,  263,  264,  265,
  266,  267,  268,    7,    8,  271,   10,   -1,   -1,   13,
   14,   -1,   -1,   -1,   -1,   19,   20,   21,   22,   23,
   24,   25,   26,   27,   28,   29,   30,   31,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 271
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,
0,0,"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"NUMBER","VAR","BLTIN","UNDEF","OR","AND","GT","GE","LT","LE","EQ","NE",
"UNARYMINUS","NOT","POW",
};
static const char *yyrule[] = {
"$accept : program",
"program : asgns expr empty",
"program : error",
"asgns :",
"asgns : asgns '\\n'",
"asgns : asgns asgn '\\n'",
"asgn : VAR '=' expr",
"expr : NUMBER",
"expr : VAR",
"expr : asgn",
"expr : BLTIN '(' expr ')'",
"expr : '(' expr ')'",
"expr : expr '+' expr",
"expr : expr '-' expr",
"expr : expr '*' expr",
"expr : expr '/' expr",
"expr : expr POW expr",
"expr : '-' expr",
"expr : expr GT expr",
"expr : expr GE expr",
"expr : expr LT expr",
"expr : expr LE expr",
"expr : expr EQ expr",
"expr : expr NE expr",
"expr : expr AND expr",
"expr : expr OR expr",
"expr : NOT expr",
"empty :",
"empty : empty '\\n'",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 67 "parse.y"
 /* User Subroutines Section */

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
#line 548 "parse.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = data->s_mark - data->s_base;
    newss = (short *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack)) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 32 "parse.y"
	{ ADD_OP(OP_STOP); return 0; }
break;
case 2:
#line 33 "parse.y"
	{ return 1; }
break;
case 5:
#line 37 "parse.y"
	{ ADD_OP(OP_POP); }
break;
case 6:
#line 39 "parse.y"
	{ ADD_OP(OP_PUSH_VAR); ADD_SYM(yystack.l_mark[-2]);
			  ADD_OP(OP_ASSIGN); yystack.l_mark[-2]->type = DEFINED; }
break;
case 7:
#line 42 "parse.y"
	{ ADD_OP(OP_PUSH_CONST); ADD_SYM(yystack.l_mark[0]); }
break;
case 8:
#line 43 "parse.y"
	{ ADD_OP(OP_PUSH_VAR); ADD_SYM(yystack.l_mark[0]); ADD_OP(OP_EVAL); }
break;
case 10:
#line 45 "parse.y"
	{ ADD_OP(OP_BUILTIN); ADD_SYM(yystack.l_mark[-3]); }
break;
case 12:
#line 47 "parse.y"
	{ ADD_OP(OP_ADD); }
break;
case 13:
#line 48 "parse.y"
	{ ADD_OP(OP_SUB); }
break;
case 14:
#line 49 "parse.y"
	{ ADD_OP(OP_MUL); }
break;
case 15:
#line 50 "parse.y"
	{ ADD_OP(OP_DIV); }
break;
case 16:
#line 51 "parse.y"
	{ ADD_OP(OP_POWER); }
break;
case 17:
#line 52 "parse.y"
	{ ADD_OP(OP_NEGATE); }
break;
case 18:
#line 53 "parse.y"
	{ ADD_OP(OP_GT); }
break;
case 19:
#line 54 "parse.y"
	{ ADD_OP(OP_GE); }
break;
case 20:
#line 55 "parse.y"
	{ ADD_OP(OP_LT); }
break;
case 21:
#line 56 "parse.y"
	{ ADD_OP(OP_LE); }
break;
case 22:
#line 57 "parse.y"
	{ ADD_OP(OP_EQ); }
break;
case 23:
#line 58 "parse.y"
	{ ADD_OP(OP_NE); }
break;
case 24:
#line 59 "parse.y"
	{ ADD_OP(OP_AND); }
break;
case 25:
#line 60 "parse.y"
	{ ADD_OP(OP_OR); }
break;
case 26:
#line 61 "parse.y"
	{ ADD_OP(OP_NOT); }
break;
#line 843 "parse.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (short) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
