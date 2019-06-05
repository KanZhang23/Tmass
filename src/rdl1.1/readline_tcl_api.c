#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "readline_tcl_api.h"

static char **mycompleter(char *text, int start, int end);
static char *command_generator(const char *text, int state);
static void c_callback_handler(char *line);
static const char *default_history_file(void);

static Tcl_Interp *callback_interp = NULL;
static char **completion_table = NULL;
static int completion_table_len = 0;
static int completion_table_size = 0;

tcl_routine(tcl_api_version)
{
    tcl_require_objc(1);
    Tcl_AppendElement(interp, RDL_VERSION);
    return TCL_OK;
}

tcl_routine(readline)
{
    char *prompt, *line;

    tcl_require_objc(2);
    prompt = Tcl_GetStringFromObj(objv[1], NULL);
    line = readline(prompt);
    if (line)
    {
	if (*line)
	{
	    add_history(line);
	    Tcl_SetResult(interp, line, TCL_VOLATILE);
	}
	free(line);
	return TCL_OK;
    }
    else
    {
	Tcl_SetResult(interp, "EOF", TCL_STATIC);
	return TCL_ERROR;
    }
}

void init_readline_tcl_api(Tcl_Interp *interp)
{
    static char *readline_name = "tcl_rdl";
    static char *word_breaking_chars = " \t\n\r\f\"\\$><;|&{([";

    rl_readline_name = readline_name;
    rl_basic_word_break_characters = word_breaking_chars;
    rl_attempted_completion_function = (rl_completion_func_t*)mycompleter;
    callback_interp = interp;
}

tcl_routine(add_completion)
{
    int table_index;
    char *entry;
    
    tcl_require_objc(2);
    entry = Tcl_GetStringFromObj(objv[1], NULL);
    if (!*entry)
	return TCL_OK;
    for (table_index = 0; table_index < completion_table_len; table_index++)
	if (strcmp(completion_table[table_index], entry) == 0)
	    return TCL_OK;
    completion_table_len++;
    if (completion_table_len > completion_table_size)
    {
	completion_table_size += (completion_table_size/2 + 10);
	completion_table = (char **)realloc(
	    completion_table, completion_table_size*sizeof(char *));
	if (completion_table == NULL)
	{
	    fprintf(stderr, "Fatal error: out of memory\n");
	    fflush(stderr);
	    abort();
	}
    }
    completion_table[table_index] = strdup(entry);
    if (completion_table[table_index] == NULL)
    {
	fprintf(stderr, "Fatal error: out of memory\n");
	fflush(stderr);
	abort();
    }
    return TCL_OK;
}

tcl_routine(callback_handler_install)
{
    char *prompt;
    tcl_require_objc(2);
    prompt = Tcl_GetStringFromObj(objv[1], NULL);
    rl_callback_handler_install(prompt, c_callback_handler);
    return TCL_OK;
}

VOID_FUNCT_WITH_VOID_ARG(callback_read_char)
VOID_FUNCT_WITH_VOID_ARG(callback_handler_remove)

tcl_routine(clear_history)
{
    tcl_require_objc(1);
    clear_history();
    return TCL_OK;
}

tcl_routine(history_max_length)
{
    int maxlen;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &maxlen) != TCL_OK)
	return TCL_ERROR;
    if (maxlen >= 0)
	stifle_history(maxlen);
    else
	unstifle_history();
    return TCL_OK;
}

tcl_routine(read_history)
{
    const char *filename;
    int ierr;

    if (objc > 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    else if (objc == 2)
    {
	filename = Tcl_GetStringFromObj(objv[1],NULL);
	if (filename[0] == '\0')
	    filename = default_history_file();
    }
    else
	filename = default_history_file();
    ierr = read_history(filename);
    if (ierr)
    {
	Tcl_AppendResult(
	    interp, "Failed to read command history from file ",
	    filename, ": ", strerror(ierr), NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(write_history)
{
    const char *filename;
    int ierr;

    if (objc > 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    else if (objc == 2)
    {
	filename = Tcl_GetStringFromObj(objv[1],NULL);
	if (filename[0] == '\0')
	    filename = default_history_file();
    }
    else
	filename = default_history_file();
    ierr = write_history(filename);
    if (ierr)
    {
	Tcl_AppendResult(
	    interp, "Failed to write command history to file ",
	    filename, ": ", strerror(ierr), NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

static char **mycompleter(char *text, int start, int end)
{
    char **matches = (char **)NULL;
    int i, iscommand = 0;

    /* If the word contains :: (namespace delimiter) then 
     * this is probably a command.
     */
    for (i=start; i+1<end; ++i)
	if (rl_line_buffer[i] == ':' && rl_line_buffer[i+1] == ':')
	{
	    iscommand = 1;
	    break;
	}
    if (!iscommand)
    {
	/* Peek at the line buffer contents backwards. If, after skipping white
	 * space, we get to the beginning of the line or see '[' then this must be 
	 * a command. Otherwise it is a name of a file in the current directory.
	 */
	while (start > 0)
	    if (!isspace(rl_line_buffer[--start]))
		break;
	if (start == 0 || rl_line_buffer[start] == '[')
	    iscommand = 1;
    }
    if (iscommand)
	matches = (char **)rl_completion_matches(text, command_generator);

    return (matches);
}

static const char *default_history_file()
{
    static char *filename = NULL;
    static int firstcall = 1;
    const char *default_f = ".tclhistory";
    const char *home;
    
    if (firstcall)
    {
	firstcall = 0;
	home = getenv("HOME");
	if (home)
	{
	    filename = (char *)malloc(strlen(home) + strlen(default_f) + 2);
	    if (filename == NULL)
	    {
		fprintf(stderr, "Out of memory. Aborting.\n");
		fflush(stderr);
		abort();
	    }
	    strcpy(filename, home);
	    strcat(filename, "/");
	    strcat(filename, default_f);
	}
    }
    return filename;
}

static char *command_generator(const char *text, int state)
{
    static int table_index, len;

    if (!state)
    {
        table_index = 0;
        len = strlen(text);
    }
    
    for ( ; table_index < completion_table_len; table_index++)
        if (strncmp(completion_table[table_index], text, len) == 0)
            return strdup(completion_table[table_index++]);

    return ((char *)NULL);
}

static void c_callback_handler(char *line)
{
    static char *line_callback = "::rdl::line_callback";
    static char *eof_callback = "::rdl::eof_callback";
    static char *error_handler = "::rdl::callback_error_handler";
    const char *error;
    Tcl_Obj *command[3];
    int status;

    if (line)
    {
	if (*line)
	    add_history(line);
	command[0] = Tcl_NewStringObj(line_callback, 20);
	Tcl_IncrRefCount(command[0]);
	command[1] = Tcl_NewStringObj(line, strlen(line));
	Tcl_IncrRefCount(command[1]);
	status = Tcl_EvalObjv(callback_interp, 2, command, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount(command[0]);
	Tcl_DecrRefCount(command[1]);
	if (status != TCL_OK)
	{
	    command[1] = Tcl_NewStringObj(line_callback, 20);
	    Tcl_IncrRefCount(command[1]);
	}
    }
    else
    {
	command[0] = Tcl_NewStringObj(eof_callback, 19);
	Tcl_IncrRefCount(command[0]);
	status = Tcl_EvalObjv(callback_interp, 1, command, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount(command[0]);
	if (status != TCL_OK)
	{
	    command[1] = Tcl_NewStringObj(eof_callback, 19);
	    Tcl_IncrRefCount(command[1]);
	}
    }
    if (status != TCL_OK)
    {
	command[0] = Tcl_NewStringObj(error_handler, 29);
	Tcl_IncrRefCount(command[0]);
	error = Tcl_GetStringResult(callback_interp);
	command[2] = Tcl_NewStringObj(error, strlen(error));
	Tcl_IncrRefCount(command[2]);
	Tcl_EvalObjv(callback_interp, 3, command, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount(command[0]);
	Tcl_DecrRefCount(command[1]);
	Tcl_DecrRefCount(command[2]);
    }
}
