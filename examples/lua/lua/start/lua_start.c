#include "log.h"
#include "lprefix.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef CONFIG_LUA_LHAL
#include "llib_init.h"
#endif

#ifdef DBG_TAG
#undef DBG_TAG
#endif
#define DBG_TAG "LUA"

static char lua_path_default[] = LUA_PATH_DEFAULT;

#if !defined(LUA_SL_PROGNAME)
#define LUA_SL_PROGNAME "lua"
#endif

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR "LUA_INIT"
#endif

#define LUA_INITVARVERSION LUA_INIT_VAR LUA_VERSUFFIX

static lua_State *sl_globalL = NULL;

static const char *sl_progname = LUA_SL_PROGNAME;

static void (*sl_signalfunc)(void) = NULL;

static void sl_signal(void (*func)(void))
{
    sl_signalfunc = func;
}

/*
** Hook set by signal function to stop the interpreter.
*/
static void sl_stop(lua_State *L, lua_Debug *ar)
{
    (void)ar;                   /* unused arg. */
    lua_sethook(L, NULL, 0, 0); /* reset hook */
    luaL_error(L, "interrupted!");
}

/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
static void sl_action(void)
{
    int flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
    sl_signal(NULL); /* if another SIGINT happens, terminate process */
    lua_sethook(sl_globalL, sl_stop, flag, 1);
}

static void sl_print_usage(const char *badoption)
{
    lua_writestringerror("%s: ", sl_progname);

    if (badoption[1] == 'e' || badoption[1] == 'l') {
        lua_writestringerror("'%s' needs argument\n", badoption);
    } else {
        lua_writestringerror("unrecognized option '%s'\n", badoption);
    }

    lua_writestringerror(
        "usage: %s [options] [script [args]]\n"
        "Available options are:\n"
        "  -e stat   execute string 'stat'\n"
        "  -i        enter interactive mode after executing 'script'\n"
        "  -l mod    require library 'mod' into global 'mod'\n"
        "  -l g=mod  require library 'mod' into global 'g'\n"
        "  -v        show version information\n"
        "  -E        ignore environment variables\n"
        "  -W        turn warnings on\n"
        "  --        stop handling options\n"
        "  -         stop handling options and execute stdin\n",
        sl_progname);
}

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void sl_message(const char *pname, const char *msg)
{
    if (pname) {
        lua_writestringerror("%s: ", pname);
    }
    lua_writestringerror("%s\n", msg);
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'sl_msghandler'.
*/
static int sl_report(lua_State *L, int status)
{
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        sl_message(sl_progname, msg);
        lua_pop(L, 1); /* remove message */
    }
    return status;
}

/*
** Message handler used to run all chunks
*/
static int sl_msghandler(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL) {                           /* is error object not a string? */
        if (luaL_callmeta(L, 1, "__tostring") && /* does it have a metamethod */
            lua_type(L, -1) == LUA_TSTRING)      /* that produces a string? */
            return 1;                            /* that is the message */
        else
            msg = lua_pushfstring(L, "(error object is a %s value)",
                                  luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1); /* append a standard traceback */
    return 1;                     /* return the traceback */
}

/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int sl_docall(lua_State *L, int narg, int nres)
{
    int status;
    int base = lua_gettop(L) - narg;     /* function index */
    lua_pushcfunction(L, sl_msghandler); /* push message handler */
    lua_insert(L, base);                 /* put it under function and args */
    sl_globalL = L;                      /* to be available to 'sl_action' */
    sl_signal(sl_action);                /* set C-signal handler */
    status = lua_pcall(L, narg, nres, base);
    sl_signal(NULL);     /* reset C-signal handler */
    lua_remove(L, base); /* remove message handler from the stack */
    return status;
}

static void sl_print_version(void)
{
    char *token, *outer_ptr = NULL;
    LOG_I("%s\r\n", LUA_COPYRIGHT);
    LOG_I("LUA_ROOT          %s\r\n", LUA_ROOT);
    LOG_I("LUA_LDIR          %s\r\n", LUA_LDIR);
    LOG_I("LUA_CDIR          %s\r\n", LUA_CDIR);
    LOG_I("LUA_PATH_DEFAULT\r\n");
    token = strtok_r(lua_path_default, ";", &outer_ptr);
    while (token != NULL) {
        LOG_I("                  %s\r\n", token);
        token = strtok_r(NULL, ";", &outer_ptr);
    }

    LOG_I("\r\n");
    LOG_FLUSH();
}

/*
** Create the 'arg' table, which stores all arguments from the
** command line ('argv'). It should be aligned so that, at index 0,
** it has 'argv[script]', which is the script name. The arguments
** to the script (everything after 'script') go to positive indices;
** other arguments (before the script name) go to negative indices.
** If there is no script name, assume interpreter's name as base.
*/
static void sl_createargtable(lua_State *L, char **argv, int argc, int script)
{
    int i, narg;
    if (script == argc)
        script = 0;             /* no script name? */
    narg = argc - (script + 1); /* number of positive indices */
    lua_createtable(L, narg, script + 1);
    for (i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - script);
    }
    lua_setglobal(L, "arg");
}

static int sl_dochunk(lua_State *L, int status)
{
    if (status == LUA_OK)
        status = sl_docall(L, 0, 0);
    return sl_report(L, status);
}

static int sl_dofile(lua_State *L, const char *name)
{
    return sl_dochunk(L, luaL_loadfile(L, name));
}

static int sl_dostring(lua_State *L, const char *s, const char *name)
{
    return sl_dochunk(L, luaL_loadbuffer(L, s, strlen(s), name));
}

/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
*/
static int sl_dolibrary(lua_State *L, char *globname)
{
    int status;
    char *modname = strchr(globname, '=');
    if (modname == NULL)    /* no explicit name? */
        modname = globname; /* module name is equal to global name */
    else {
        *modname = '\0'; /* global name ends here */
        modname++;       /* module name starts after the '=' */
    }
    lua_getglobal(L, "require");
    lua_pushstring(L, modname);
    status = sl_docall(L, 1, 1); /* call 'require(modname)' */
    if (status == LUA_OK)
        lua_setglobal(L, globname); /* globname = require(modname) */
    return sl_report(L, status);
}

/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static int sl_pushargs(lua_State *L)
{
    int i, n;
    if (lua_getglobal(L, "arg") != LUA_TTABLE)
        luaL_error(L, "'arg' is not a table");
    n = (int)luaL_len(L, -1);
    luaL_checkstack(L, n + 3, "too many arguments to script");
    for (i = 1; i <= n; i++)
        lua_rawgeti(L, -i, i);
    lua_remove(L, -i); /* remove table from the stack */
    return n;
}

static int sl_handle_script(lua_State *L, char **argv)
{
    int status;
    const char *fname = argv[0];
    if (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
        fname = NULL; /* stdin */
    status = luaL_loadfile(L, fname);
    if (status == LUA_OK) {
        int n = sl_pushargs(L); /* push arguments to script */
        status = sl_docall(L, n, LUA_MULTRET);
    }
    return sl_report(L, status);
}

/* bits of various argument indicators in 'args' */
#define has_error 1  /* bad option */
#define has_i     2  /* -i */
#define has_v     4  /* -v */
#define has_e     8  /* -e */
#define has_E     16 /* -E */

/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code (or an error code if it finds
** any invalid argument). 'first' returns the first not-handled argument
** (either the script name or a bad argument in case of error).
*/
static int sl_collectargs(char **argv, int *first)
{
    int args = 0;
    int i;
    for (i = 1; argv[i] != NULL; i++) {
        *first = i;
        if (argv[i][0] != '-')          /* not an option? */
            return args;                /* stop handling options */
        switch (argv[i][1]) {           /* else check option */
            case '-':                   /* '--' */
                if (argv[i][2] != '\0') /* extra characters after '--'? */
                    return has_error;   /* invalid option */
                *first = i + 1;
                return args;
            case '\0':       /* '-' */
                return args; /* script "name" is '-' */
            case 'E':
                if (argv[i][2] != '\0') /* extra characters? */
                    return has_error;   /* invalid option */
                args |= has_E;
                break;
            case 'W':
                if (argv[i][2] != '\0') /* extra characters? */
                    return has_error;   /* invalid option */
                break;
            case 'i':
                args |= has_i; /* (-i implies -v) */ /* FALLTHROUGH */
            case 'v':
                if (argv[i][2] != '\0') /* extra characters? */
                    return has_error;   /* invalid option */
                args |= has_v;
                break;
            case 'e':
                args |= has_e;            /* FALLTHROUGH */
            case 'l':                     /* both options need an argument */
                if (argv[i][2] == '\0') { /* no concatenated argument? */
                    i++;                  /* try next 'argv' */
                    if (argv[i] == NULL || argv[i][0] == '-')
                        return has_error; /* no next argument or it is another option */
                }
                break;
            default: /* invalid option */
                return has_error;
        }
    }
    *first = i; /* no script name */
    return args;
}

/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static int sl_runargs(lua_State *L, char **argv, int n)
{
    int i;
    for (i = 1; i < n; i++) {
        int option = argv[i][1];
        lua_assert(argv[i][0] == '-'); /* already checked */
        switch (option) {
            case 'e':
            case 'l': {
                int status;
                char *extra = argv[i] + 2; /* both options need an argument */
                if (*extra == '\0')
                    extra = argv[++i];
                lua_assert(extra != NULL);
                status = (option == 'e') ? sl_dostring(L, extra, "=(command line)") : sl_dolibrary(L, extra);
                if (status != LUA_OK)
                    return 0;
                break;
            }
            case 'W':
                lua_warning(L, "@on", 0); /* warnings on */
                break;
        }
    }
    return 1;
}

static int sl_handle_luainit(lua_State *L)
{
    const char *name = "=" LUA_INITVARVERSION;
    const char *init = getenv(name + 1);
    if (init == NULL) {
        name = "=" LUA_INIT_VAR;
        init = getenv(name + 1); /* try alternative name */
    }
    if (init == NULL)
        return LUA_OK;
    else if (init[0] == '@')
        return sl_dofile(L, init + 1);
    else
        return sl_dostring(L, init, name);
}

/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#if !defined(LUA_PROMPT)
#define LUA_PROMPT  "> "
#define LUA_PROMPT2 ">> "
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT 512
#endif

/*
** sl_lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#define sl_lua_stdin_is_tty() 0 /* assume stdin is a tty */

/*
** sl_lua_readline defines how to show a prompt and then read a line from
** the standard input.
** sl_lua_saveline defines how to "save" a read line in a "history".
** sl_lua_freeline defines how to free a line read by sl_lua_readline.
*/

#define sl_lua_initreadline(L)   ((void)L, (void)0)
#define sl_lua_readline(L, b, p) ((void)L, (void)b, (void)p, 0)
#define sl_lua_saveline(L, line) ((void)L, (void)line, (void)0)
#define sl_lua_freeline(L, b)    ((void)L, (void)b, (void)0)

#ifndef sl_lua_initreadline
#define sl_lua_initreadline(L) ((void)L, (void)0)
#endif

#ifndef sl_lua_readline
#define sl_lua_readline(L, b, p)                                  \
    ((void)L, fputs(p, stdout), fflush(stdout), /* show prompt */ \
     fgets(b, LUA_MAXINPUT, stdin) != NULL)     /* get line */
#endif

#ifndef sl_lua_saveline
#define sl_lua_saveline(L, line) ((void)L, (void)line, (void)0)
#endif

#ifndef sl_lua_freeline
#define sl_lua_freeline(L, b) ((void)L, (void)b, (void)0)
#endif

/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
static const char *sl_get_prompt(lua_State *L, int firstline)
{
    if (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
        return (firstline ? LUA_PROMPT : LUA_PROMPT2); /* use the default */
    else {                                             /* apply 'tostring' over the value */
        const char *p = luaL_tolstring(L, -1, NULL);
        lua_remove(L, -2); /* remove original value */
        return p;
    }
}

/* mark in error messages for sl_incomplete statements */
#define SL_EOFMARK "<eof>"
#define sl_marklen (sizeof(SL_EOFMARK) / sizeof(char) - 1)

/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** sl_incomplete statements.
*/
static int sl_incomplete(lua_State *L, int status)
{
    if (status == LUA_ERRSYNTAX) {
        size_t lmsg;
        const char *msg = lua_tolstring(L, -1, &lmsg);
        if (lmsg >= sl_marklen && strcmp(msg + lmsg - sl_marklen, SL_EOFMARK) == 0) {
            lua_pop(L, 1);
            return 1;
        }
    }
    return 0; /* else... */
}

/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static int sl_pushline(lua_State *L, int firstline)
{
    char buffer[LUA_MAXINPUT];
    char *b = buffer;
    size_t l;
    const char *prmt = sl_get_prompt(L, firstline);
    int readstatus = sl_lua_readline(L, b, prmt);
    if (readstatus == 0)
        return 0;  /* no input (prompt will be popped by caller) */
    lua_pop(L, 1); /* remove prompt */
    l = strlen(b);
    if (l > 0 && b[l - 1] == '\n')              /* line ends with newline? */
        b[--l] = '\0';                          /* remove it */
    if (firstline && b[0] == '=')               /* for compatibility with 5.2, ... */
        lua_pushfstring(L, "return %s", b + 1); /* change '=' to 'return' */
    else
        lua_pushlstring(L, b, l);
    sl_lua_freeline(L, b);
    return 1;
}

/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int sl_addreturn(lua_State *L)
{
    const char *line = lua_tostring(L, -1); /* original line */
    const char *retline = lua_pushfstring(L, "return %s;", line);
    int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
    if (status == LUA_OK) {
        lua_remove(L, -2);            /* remove modified line */
        if (line[0] != '\0')          /* non empty? */
            sl_lua_saveline(L, line); /* keep history */
    } else
        lua_pop(L, 2); /* pop result from 'luaL_loadbuffer' and modified line */
    return status;
}

/*
** Read multiple lines until a complete Lua statement
*/
static int sl_multiline(lua_State *L)
{
    for (;;) { /* repeat until gets a complete statement */
        size_t len;
        const char *line = lua_tolstring(L, 1, &len);         /* get what it has */
        int status = luaL_loadbuffer(L, line, len, "=stdin"); /* try it */
        if (!sl_incomplete(L, status) || !sl_pushline(L, 0)) {
            sl_lua_saveline(L, line); /* keep history */
            return status;            /* cannot or should not try to add continuation line */
        }
        lua_pushliteral(L, "\n"); /* add newline... */
        lua_insert(L, -2);        /* ...between the two lines */
        lua_concat(L, 3);         /* join them */
    }
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int sl_loadline(lua_State *L)
{
    int status;
    lua_settop(L, 0);
    if (!sl_pushline(L, 1))
        return -1;                            /* no input */
    if ((status = sl_addreturn(L)) != LUA_OK) /* 'return ...' did not work? */
        status = sl_multiline(L);             /* try as command, maybe with continuation lines */
    lua_remove(L, 1);                         /* remove line from the stack */
    lua_assert(lua_gettop(L) == 1);
    return status;
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void sl_print(lua_State *L)
{
    int n = lua_gettop(L);
    if (n > 0) { /* any result to be printed? */
        luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
        lua_getglobal(L, "print");
        lua_insert(L, 1);
        if (lua_pcall(L, n, 0, 0) != LUA_OK)
            sl_message(sl_progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                                    lua_tostring(L, -1)));
    }
}

/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void sl_doREPL(lua_State *L)
{
    int status;
    const char *oldsl_progname = sl_progname;
    sl_progname = NULL; /* no 'sl_progname' on errors in interactive mode */
    sl_lua_initreadline(L);
    while ((status = sl_loadline(L)) != -1) {
        if (status == LUA_OK)
            status = sl_docall(L, 0, LUA_MULTRET);
        if (status == LUA_OK)
            sl_print(L);
        else
            sl_report(L, status);
    }
    lua_settop(L, 0); /* clear stack */
    lua_writeline();
    sl_progname = oldsl_progname;
}

/* }================================================================== */

/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static int sl_main(lua_State *L)
{
    int argc = (int)lua_tointeger(L, 1);
    char **argv = (char **)lua_touserdata(L, 2);
    int script;
    int args = sl_collectargs(argv, &script);
    luaL_checkversion(L); /* check that interpreter has correct version */
    if (argv[0] && argv[0][0])
        sl_progname = argv[0];
    if (args == has_error) {          /* bad arg? */
        sl_print_usage(argv[script]); /* 'script' has index of bad arg. */
        return 0;
    }
    if (args & has_v) /* option '-v'? */
        sl_print_version();
    if (args & has_E) {        /* option '-E'? */
        lua_pushboolean(L, 1); /* signal for libraries to ignore env. vars. */
        lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
    }
    luaL_openlibs(L);                         /* open standard libraries */
#ifdef CONFIG_LUA_LHAL
    luaL_preload_llibs(L);
#endif
    sl_createargtable(L, argv, argc, script); /* create table 'arg' */
    lua_gc(L, LUA_GCGEN, 0, 0);               /* GC in generational mode */
    if (!(args & has_E)) {                    /* no option '-E'? */
        if (sl_handle_luainit(L) != LUA_OK)   /* run LUA_INIT */
            return 0;                         /* error running LUA_INIT */
    }
    if (!sl_runargs(L, argv, script)) /* execute arguments -e and -l */
        return 0;                     /* something failed */
    if (script < argc &&              /* execute main script (if there is one) */
        sl_handle_script(L, argv + script) != LUA_OK)
        return 0;
    if (args & has_i)                                       /* -i option? */
        sl_doREPL(L);                                       /* do read-eval-print loop */
    else if (script == argc && !(args & (has_e | has_v))) { /* no arguments? */
        if (sl_lua_stdin_is_tty()) {                        /* running in interactive mode? */
            sl_doREPL(L);                                   /* do read-eval-print loop */
        } else
            sl_dofile(L, NULL); /* executes stdin as a file */
    }
    lua_pushboolean(L, 1); /* signal no errors */
    return 1;
}

int lua_start(int argc, char **argv)
{
    sl_print_version();
    int status, result;
    lua_State *L = luaL_newstate(); /* create state */
    if (L == NULL) {
        sl_message(argv[0], "cannot create state: not enough memory");
        return EXIT_FAILURE;
    }
    lua_pushcfunction(L, &sl_main); /* to call 'sl_main' in protected mode */
    lua_pushinteger(L, argc);       /* 1st argument */
    lua_pushlightuserdata(L, argv); /* 2nd argument */
    status = lua_pcall(L, 2, 1, 0); /* do the call */
    result = lua_toboolean(L, -1);  /* get result */
    sl_report(L, status);
    lua_close(L);
    return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void lua_end(void)
{
    if (sl_signalfunc) {
        sl_signalfunc();
    }
}