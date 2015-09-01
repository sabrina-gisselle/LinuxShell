#include "shell.h"
#include <stdio.h>
#include <string.h>
int builtin = 0;
char *bistr = NULL;
char *bistr2 = NULL;
char *outfile = NULL;
char *infile = NULL;
char *err_outfile = NULL;
int cmd = 0;
int pipeline = 0;
int background = 0;
int redirect_io = 0;
int redirect_err = 0;
int redirection = 0;
int arguments = 0;
int currcmd = 0;
int errorCheck = 0;
int gt_choice = 0;
int stderr_choice = 0;
int savestdin = -1;
int savestdout = -1;
int savestderr = -1;
int outfile_fd = -1;
int infile_fd = -1;

void yyerror(const char *str){}

int yywrap(){return 1;}

char* mkstr(char* s){
	int i, size;
	size = (int)strlen(s);
	for(i=0; i < size; ++i){
		if(s[i]==' ' || s[i]=='\n' || s[i]=='|' ){
			break;
		}
	}
	char *substring = strndup (s, i);
	
	return substring;
}

char* rmvquotes(char* s){
	int size;
	size = (int)strlen(s);

	if(size == 2){
		yyerror("Empty text\n");
		return NULL;
	}
	char *substring = strndup(s+1, size-2);
	return substring;
}

char* rmvbracket(char* s){
	int size;
	size = (int)strlen(s);

	if(size == 3){
		yyerror("Environment variable cannot be the empty\n");
		return NULL;
	}
	char *substring = strndup(s+2, size-3);
	return substring;
}

int wildcardCheck(char * arg) {
	char * asteriskCheck;
  	asteriskCheck = strstr (arg,"*");

	char * questionCheck;
  	questionCheck = strstr (arg,"?");

	if(asteriskCheck != NULL || questionCheck != NULL){
		return 1;
	}
	return 0;
}

void executingWildcard(char * arg){	
	glob_t results;
	int passes;

	results.gl_pathc = 0;
	results.gl_pathv = NULL;
	results.gl_offs = 0;

	passes = glob( arg, GLOB_NOCHECK | GLOB_TILDE, NULL, &results );

	if( passes == 0 ) {
		int i;

		for( i = 0; i < results.gl_pathc; ++i ) {
			cmdtab[currcmd]->atptr->args[cmdtab[currcmd]->nargs++] = results.gl_pathv[i];
		}
	}
	// else silently failed and continue with request
}
