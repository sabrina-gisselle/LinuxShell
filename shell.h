#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <signal.h>
#include <pwd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXALIAS 100
#define MAXCMDS 100
#define MAXENV	100
#define MAXARGS 300

#define ERRORS 1
#define OK 2
#define BYERETURN 3
#define LOOP 4

#define ERR    (-1)             /* indicates an error condition */
#define READ   0                /* read end of a pipe */
#define WRITE  1                /* write end of a pipe */
#define STDIN  0                /* file descriptor of standard in */
#define STDOUT 1                /* file descriptor of standard out */
#define STDERR 2                /* file descriptor of standard error */

/* used for I/O redirection */
#define OUT	1
#define IN	2
#define OUTIN	3
#define INOUT	4

/* used to choose > or >> */
#define CREATE	1
#define CONCAT	2

#define TOFILE	1
#define TOOUT	2

/* alias structure */
typedef struct alias {
	int used;
	char *alname;
	char *alstr;
}alias;

/* environment structure */
typedef struct env {
	int used;
	char *envname;
	char *envstr;
}env;

/* command line arguments structure */
typedef struct comargs {
	char *args[MAXARGS];
} ARGTAB;

/* command structure */
typedef struct com {
	char *comname;
	int infd;
	int outfd;
	int nargs;
	ARGTAB *atptr;
} COMMAND;

/* Built-in commands */
enum BUILTIN{
	none = 0,
	bisetenv = 1,
	biprintenv = 2,
	biunsetenv = 3,
	bicdPath = 4,
	bicdHome = 5,
	bialias = 6,
	biunalias = 7,
	biprintalias = 8,
	bibye = 9,
	bigetenv = 10
};

/* externals */
extern struct alias aliastab[];
extern struct env envtab[];
extern COMMAND *cmdtab[];
extern struct env envvar;
extern int builtin;
extern char *bistr;
extern char *bistr2;
extern char *outfile;
extern char *infile;
extern char *err_outfile;
extern int cmd;
extern int pipeline;
extern int background;
extern int redirect_io;
extern int redirect_err;
extern int currcmd;	// count of commands in the command table
extern int cmdstart;
extern int errorCheck;
extern int redirection;
extern int gt_choice;
extern int stderr_choice;
extern int savestdin;
extern int savestdout;
extern int savestderr;
extern int outfile_fd;
extern int infile_fd;

/* prototyes */
void nuterr(char* string);
void undoit();
void shell_init();
void init_scanner_and_parser();
int understand_errors();
int getCommand();
void recover_from_errors();
void processCommand();
// Env
int checkInEnvTableAndUpdate(char *, char *);
void addInEnvTable(char *, char * );
void printEnvTable();
void checkInEnvTableAndRemove(char *);
char* containsEnvAndReplace(char *);
char* getenvFromTab(char *);
// Alias
int checkInAliasTableAndUpdate(char *, char *);
void addInAliasTable(char *, char *);
void printAliasTable();
void checkInAliasTableAndRemove(char *);
char* getAliasFromTab(char *);

void doBuiltinCommands();
void execute_it();
void printPrompt();

int checkAndExpand();
char * replace(char const * const original, char const * const pattern, char const * const replacement);

void handlePipeline();
void printCommandTable();
void exit_err(const char*);
void cmdtab_cleanup();
void handleIORedirection();
void handleErrRedirection();
void saveSTD();
void restoreSTD();
