#include "shell.h"

extern int yyparse();
extern int yy_scan_string(const char *);

struct alias aliastab[MAXALIAS];
struct env envtab[MAXENV];
struct env envvar;
COMMAND *cmdtab[MAXCMDS];


void nuterr(char* string){
	printf("Error %s", string);
}

void undoit(){
	printf("Undoit");
	// call yylex until completed 
}

void shell_init(){
	signal (SIGQUIT, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	//signal (SIGTSTP, SIG_IGN);
	struct alias aliastab[MAXALIAS];
	struct env envtab[MAXENV];
	
	// Set $PATH variable
	char * path = getenv("PATH");
	addInEnvTable("PATH", path);
}

void init_scanner_and_parser(){
	builtin = 0;
	bistr = NULL;
	bistr2 = NULL;	
	cmd = 0;
	errorCheck = 0;
	redirect_io = 0;
	redirection = 0;
	gt_choice = 0;
	pipeline = 0;
}

int understand_errors(){
	return (ERRORS);
}

int getCommand(){
	init_scanner_and_parser();
	if(yyparse()){
		return understand_errors();
	} else if (errorCheck){
		return (LOOP);
	}else if (builtin == bibye){
		return (BYERETURN);
	}else {
		return (OK);
	}
}

void recover_from_errors(){
	// Check where the error occurred and eat until the end of the tail
	// Use yylex() directly	
	//printf("An error occurred\n");
}

char* getAliasFromTab(char * name){
	int i;
	for(i = 0; i < MAXALIAS; ++i){
		if(aliastab[i].used && strcmp(aliastab[i].alname, name) == 0){
			return aliastab[i].alstr;
		}
	}
	
	return "";
}

void checkAndExpandAlias(char **cmdname){
	
	char * current;
	char * containedAlias;
	current = *cmdname;
	while(1){
		containedAlias = getAliasFromTab(current);
		if(strlen(containedAlias) != 0 && strcmp(containedAlias, *cmdname) != 0){
			current = containedAlias;
		}else if(strcmp(containedAlias, *cmdname) == 0){
			printf("Alias loop.\n");
			*cmdname = "loop";
			return;
		}else {
			break;
		}
	}
	*cmdname = current;
}
char * checkEnvAndExpandHelper(char* current){
	char * envStart;
	char * envEnd;
  	envStart = strstr (current,"${");
	envEnd = strstr (current,"}");

	if(envStart != NULL && envEnd != NULL){ 
		int i, sizeStart, sizeEnd, difference;
		sizeStart = (int)strlen(envStart);
		sizeEnd = (int)strlen(envEnd);
		difference = sizeStart - sizeEnd;
		char * old, * oldCB; 
		char * new; 
		char * newString;

		old = strndup (envStart+2, difference-2);
		oldCB = strndup (envStart, difference+1);
		new = getenvFromTab(old);
		newString = replace(current, oldCB, new);
		return newString;
	}else{
		return "";
	}	
}
char * checkEnvAndExpandBuiltIn(char * bistr){
	int i;
	char * current;

	char * containedEnv = NULL;
	if(bistr != NULL){
		while(1){
			containedEnv = checkEnvAndExpandHelper(bistr);
			if(strlen(containedEnv) != 0){
				bistr = containedEnv;
			}else{
				break;
			}
		}
	}
	return bistr;
	
}
void checkEnvAndExpandNonBuiltIn(char ** args, int argn){
	int i;
	char * current;
	for(i = 0; i < argn; ++i){	
		current;

		char * containedEnv = NULL;

		while(1){
			containedEnv = checkEnvAndExpandHelper(args[i]);
			if(strlen(containedEnv) != 0){
				args[i] = containedEnv;
			}else{
				break;
			}
		}
	}
}

void tildeExpansionNonBuiltIn(char ** args, int argn){
	int size, i;
	char * path;
	for(i = 0; i < argn; ++i){
		size = (int)strlen(args[i]);	
		if (size >= 2 && args[i][0] == '~' && args[i][1] != '/' && args[i][1] != ' '){
			struct passwd *pwd;
			path = strndup (args[i]+1, size-1);
			pwd = getpwnam(path);
			args[i] = pwd->pw_dir;
		}else if (size >= 1 && args[i][0]=='~'){
			path = getenv("HOME");
			args[i] = replace(args[i], "~", path);
			printf("args[%i]: %s\n", i, args[i]);
		}
	}
}

char * tildeExpansionBuiltIn(char * bistr){
	int size;
	char * path;
	size = (int)strlen(bistr);
	if (size >= 2 && bistr[0] == '~' && bistr[1] != '/' && bistr[1] != ' '){
		struct passwd *pwd;
		path = strndup (bistr+1, size-1);
		pwd = getpwnam(path);
		bistr = pwd->pw_dir;
	}else if (size >= 1 && bistr[0]=='~'){
		// check if "~" and replace with home dir
		path = getenv("HOME");
		bistr = replace(bistr, "~", path);
	}
	return bistr;
}
void processCommand(){
	if (builtin){
		if(builtin == bicdPath){
			bistr = checkEnvAndExpandBuiltIn(bistr);
			bistr = tildeExpansionBuiltIn(bistr);
		}
		doBuiltinCommands();
	}else{
		execute_it();
	}	
}

int checkInEnvTableAndUpdate(char * name, char * value){

	int i;
	for(i = 0; i < MAXENV; ++i){
		if(envtab[i].used && strcmp(envtab[i].envname, name) == 0){
			envtab[i].envstr = value;
			return 1;
		}
	}
	return 0;
}

void addInEnvTable(char * name, char * value){
	int i;
	for(i = 0; i < MAXENV; ++i){
		if(envtab[i].used == 0){
			envtab[i].used = 1;
			envtab[i].envname = name;
			envtab[i].envstr = value;
			return;
		}
	}
	printf("No more avalable space\n");
}

void printEnvTable(){
	int i;
	for(i = 0; i < MAXENV; ++i){
		if(envtab[i].used){
		printf("%s=%s\n", envtab[i].envname, envtab[i].envstr);				
		}
	}
}

void checkInEnvTableAndRemove(char * name){
	int i;
	for(i = 0; i < MAXENV; ++i){
		if(envtab[i].used && strcmp(envtab[i].envname, name) == 0){
			envtab[i].used = 0;
			envtab[i].envname = NULL;			
			envtab[i].envstr = NULL;
			return;
		}
	}
	// There is no env variable with this name so silently return
	return;
}

char* getenvFromTab(char * env){
	int i;
	for(i = 0; i < MAXENV; ++i){
		if(envtab[i].used && strcmp(envtab[i].envname, env) == 0){
			return envtab[i].envstr;
		}
	}
	// There is no env variable with this name so silently return
	return NULL;
}

int checkInAliasTableAndUpdate(char * name, char * value){

	int i;
	for(i = 0; i < MAXALIAS; ++i){
		if(aliastab[i].used && strcmp(aliastab[i].alname, name) == 0){

			aliastab[i].alstr = value;
			return 1;
		}
	}
	return 0;
}

void addInAliasTable(char * name, char * value){
	int i;
	for(i = 0; i < MAXALIAS; ++i){
		if(aliastab[i].used == 0){
			aliastab[i].used = 1;
			aliastab[i].alname = name;
			aliastab[i].alstr = value;
			return;
		}
	}
	printf("No more avalable space\n");
}

void printAliasTable(){
	int i;
	for(i = 0; i < MAXALIAS; ++i){
		if(aliastab[i].used){
		printf("%s\t%s\n", aliastab[i].alname, aliastab[i].alstr);				
		}
	}
}

void checkInAliasTableAndRemove(char * name){
	int i;
	for(i = 0; i < MAXALIAS; ++i){
		if(aliastab[i].used && strcmp(aliastab[i].alname, name) == 0){
			aliastab[i].used = 0;
			aliastab[i].alname = NULL;			
			aliastab[i].alstr = NULL;
			return;
		}
	}
	// There is no alias with this name so silently return
	return;
}

void doBuiltinCommands(){
	int updated;
	char * value, * path;
	switch (builtin){
		case bicdHome:
			path = getenv("HOME");
			chdir(path);
			break;

		case bicdPath:
			updated = chdir(bistr);
			if(updated){
				printf("%s: No such file or directory.\n", bistr);
			}
			break;

		case bisetenv:
			updated = checkInEnvTableAndUpdate(bistr, bistr2);		
			if(!updated){				
				addInEnvTable(bistr, bistr2);
			}
			break;

		case biprintenv:
			printEnvTable();		
			break;
	
		case biunsetenv:
			checkInEnvTableAndRemove(bistr);
			break;
		
		case bigetenv:
			value = getenvFromTab(bistr);
			printf("%s\n", value);
			break;
		
		case bialias:
			updated = checkInAliasTableAndUpdate(bistr, bistr2);		
			if(!updated){				
				addInAliasTable(bistr, bistr2);
			}
			break;
			
		case biunalias:
			checkInAliasTableAndRemove(bistr);
			break;

		case biprintalias:
			printAliasTable();
			break;
	}
}

void printCommandTable()
{
	printf("\n****************************** COMMAND TABLE ******************************\n");
	printf("\n\n\tCommand Name\t\tArgument Count\t\tArguments\n");
	int c,a;
	for(c = 0; c <= currcmd; c++)
	{
		printf("\n%i)\t%s\t\t%i\t\t\t\t",c, cmdtab[c]->comname,cmdtab[c]->nargs);
		for(a = 0; a < cmdtab[c]->nargs; a++)
		{
			printf("%s, ",cmdtab[c]->atptr->args[a]);
		}
		printf("\n");
	}
}

void cmdtab_cleanup()
{
	int j;
	for(j=0; j<currcmd; j++)
		free(cmdtab[j]);
	currcmd = -1; //reset currcmd so that yacc (parser.y) knows where to start adding new cmdtab entries
}
void saveSTD()
{
	savestdin = dup(STDIN);
	savestdout = dup(STDOUT);
	savestderr = dup(STDERR);
}

void restoreSTD()
{
	//close(STDIN);
	close(STDOUT);
	//close(STDERR);
	//dup2(savestdin, STDIN);
	dup2(savestdout, STDOUT);
	//dup2(savestderr, STDERR);
}

void handleIORedirection(){
	saveSTD();
	//int outfile_fd = -1;
	//int infile_fd = -1;
	switch(redirection)
	{
		case OUT:
			if(gt_choice == CREATE)
			{
				outfile_fd = creat(outfile, 0666/* O_RDWR | O_CREAT, S_IRUSR | S_IWUSR */); // create file
				close(STDOUT);			// disconnect terminal from stdout
				
				dup2(outfile_fd, STDOUT);		// connect outfile to stdout
				//printf("\nAFTER DUP2");
				
		// disconnect outfile from outfile_fd
				
			}
			else if(gt_choice == CONCAT)
			{
				if(access(outfile, F_OK) != 0) // file does not exist
				{
					outfile_fd = creat(outfile, 0666); // create file
					close(STDOUT);		// disconnect terminal from stdout
					dup2(outfile_fd, STDOUT);	// connect outfile to stdout
				}
				else // file exists
				{
					// concatenate file
					
					outfile_fd = open(outfile, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR); // create file
					close(STDOUT);
					dup2(outfile_fd, STDOUT);
				}
			}
			break;
		case IN:
			if(access(infile, F_OK) == 0) // input file exists
			{
				infile_fd = open(infile, O_RDWR);
				close(STDIN);
				dup(infile_fd);
				close(infile_fd);
			}
			else // input file does not exist
			{
				printf("\n*** ERROR: Input file [ %s ] does not exist.\n\n", infile);
				exit(0);	
			}
			break;
		case OUTIN:
			break;
		case INOUT:
			break;
	}
}

void handleErrRedirection()
{
	int outfile_fd;
	switch(stderr_choice)
	{
		case TOFILE:
			outfile_fd = creat(err_outfile, 0666);
			close(STDERR);
			dup2(outfile_fd, STDERR);
			close(outfile_fd);
			break;
		case TOOUT:
			close(STDERR);
			dup2(STDOUT, STDERR);
			break;

	}
	redirect_err = 0;
}

void execute_it()
{
	// Check Env table here !!!!!!!!!!!

	if (pipeline)
	{
		/* MUST terminate argv list with null to use with execvp() later */
		int i;
		for(i=cmdstart; i<=currcmd; i++)
			cmdtab[i]->atptr->args[cmdtab[i]->nargs++] = 0;
		
		/* Update cmdstart to index next open spot in command table. */
		/* NOTE: this is only needed if we do not clean up command table after each use */
		/* Uncomment the line below ONLY when you disable cmdtab_cleanup() (because cmdstart is used above) */
		//cmdstart = currcmd + 1;

		handlePipeline();
	}
	else
	{
		checkEnvAndExpandNonBuiltIn(cmdtab[currcmd]->atptr->args, cmdtab[currcmd]->nargs);
		tildeExpansionNonBuiltIn(cmdtab[currcmd]->atptr->args, cmdtab[currcmd]->nargs);
		cmdtab[currcmd]->atptr->args[cmdtab[currcmd]->nargs++] = 0;	
	 	pid_t  pid;
		int    status;

		if ((pid = fork()) < 0) {     /* fork a child process           */
			printf("*** ERROR: forking child process failed\n");
		  	exit(1);
		}
		else if (pid == 0)          /* for the child process:         */
		{	if(redirect_io)
			{
				handleIORedirection();
			}
			if(redirect_err)
			{
				handleErrRedirection();
			}
			if (execvp(*(cmdtab[currcmd]->atptr->args), cmdtab[currcmd]->atptr->args) < 0)   /* execute the command  */
			{    
				printf("%s: Command not found.\n", *(cmdtab[currcmd]->atptr->args));
			       	exit(1);
			}
		}
		if(pid != 0)  /* for the parent:      */
		{        
			close(outfile_fd);
			while (wait(&status) != pid)       /* wait for completion  */
		       ;
		}
		/* Uncomment the line below ONLY when you disable cmdtab cleanup (because cmdstart is used above) */
		//cmdstart = currcmd + 1;
	}
	//restoreSTD();
	
	cmdtab_cleanup();
}

void printPrompt(){ 
	printf(">> ");
}

/* From Stackoverflow */
char * replace(char const * const original, char const * const pattern, char const * const replacement) {
	size_t const replen = strlen(replacement);
	size_t const patlen = strlen(pattern);
	size_t const orilen = strlen(original);

	size_t patcnt = 0;
	const char * oriptr;
	const char * patloc;

	// find how many times the pattern occurs in the original string
	for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
	{
		patcnt++;
	}

	{
	// allocate memory for the new string
	size_t const retlen = orilen + patcnt * (replen - patlen);
	char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

	if (returned != NULL)
	{
		// copy the original string, 
		// replacing all the instances of the pattern
		char * retptr = returned;
		for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
		{
			size_t const skplen = patloc - oriptr;
			// copy the section until the occurence of the pattern
			strncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// copy the replacement 
			strncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// copy the rest of the string.
		strcpy(retptr, oriptr);
	}
	return returned;
	}
}


void handlePipeline() //trying to do a 2 process pipeline for now
{
	int pid_1;              /* will be process id of first child */
	int pid_2;		/* will be process id of second child */
	int pid;
	int numPipes = 2;

	int pfd[currcmd][2];	/* pipe file descriptor table. */
int pipeReceive[2];
		int pipeSend[2];
	//if ( pipe ( pfd[0] ) == ERR )		/* create a pipe. must do before a fork */
		//exit_err("There was an error with this command.\n");
	int i;
	for(i=0; i<=currcmd; i++){
	
		//int pipeReceive[2];
		//int pipeSend[2];
	
		if(i != numPipes) {
			// create sending pipe
			pipe(pipeSend);
		}

		pid = fork();
		if(pid > 0) {
			close(pipeReceive[0]);
			close(pipeReceive[1]);
		}
		else if(pid < 0) {
			printf("Error pid is negative\n");
		}
		else if(pid == 0) {	// child
		
			if( numPipes == 0 ) {
				//do nothing
			}
			
			else if(i == 0) {
				printf("firstCommand is \n");
				dup2(pipeSend[1], STDOUT);
				close(pipeSend[0]);
			}
			else if(i == numPipes) {
				printf("lastCommand\n");
				dup2(pipeReceive[0], STDIN);
				close(pipeReceive[1]);
			}
			else {
				printf("middleCommand\n");
				dup2(pipeReceive[0], STDIN);
				dup2(pipeSend[1], STDOUT);
				// CLOSING IN CHILD
				close(pipeSend[0]);
				close(pipeReceive[1]);
			}
			execvp(*(cmdtab[i]->atptr->args), cmdtab[i]->atptr->args);
			//exit(0);
		}
		pipeReceive[0] = pipeSend[0];
		pipeReceive[1] = pipeSend[1];

		
	}

	wait(pid, NULL, 0);
		

}
void exit_err(const char *msg)
{
	perror(msg);
	//exit(ERR);
}


























































