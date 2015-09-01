%{
#include "bicmd.h"
#include <stdio.h>
int yylex(void);
%}
%union {
	int i;
	char *s;
}
 %token <s> WORD TEXT
 %token TEST SETENV PRINTENV UNSETENV CD ALIAS UNALIAS NUMBER BYE EOL LT GT GT2 PIPE BS AMPERSAND ERR2FILE ERR2OUT

%type <s> WORD.args

%%

commands: 	builtin.cmd 
		| external.cmd  			{
								builtin = 0;
							}
		/*| end					{	
							}*/

/*end:		EOL					{	
								return -1;
							}*/
builtin.cmd: 	SETENV WORD WORD			{ 
								builtin = bisetenv; 
								bistr=mkstr($2); 
								bistr2=mkstr($3);  
							}
		| SETENV WORD TEXT			{ 
								builtin = bisetenv; 
								bistr=mkstr($2); 
								bistr2=rmvquotes($3);
							}
		| PRINTENV 				{ 
								builtin = biprintenv; 
							}
		| UNSETENV WORD				{ 
								builtin = biunsetenv; 
								bistr=mkstr($2);
							}
		| ALIAS WORD TEXT			{ 
								builtin = bialias; 
								bistr=mkstr($2); 
								bistr2=rmvquotes($3);  
							}
		| ALIAS WORD WORD			{ 
								builtin = bialias; 
								bistr=mkstr($2); 
								bistr2=mkstr($3);  
							}
		| ALIAS 				{ 
								builtin = biprintalias; 
							}
		| UNALIAS WORD				{ 
								builtin = biunalias; 
								bistr=mkstr($2);
							}
		| BYE 					{
								builtin = bibye; 
							}
		| CD WORD				{ 
								builtin = bicdPath; 
								bistr=mkstr($2);
							}
		| CD 					{ 
								builtin = bicdHome; 
							}

external.cmd:	ext.cmd					{
								/* Non-redirected command line */
							}
		| ext.cmd.iordr				{
								redirect_io = 1;
							}
		| ext.cmd.rdrerr			{
								redirect_err = 1;
							}
		| ext.cmd.background			{
								background = 1;
								printf("\n\nBACKGROUND\n\n");
							}
ext.cmd:	WORD.args				{
								/* WORD.args can be thought of as WORDS */
							}
		| pipeline				{
								pipeline = 1;
							}
WORD.args:	WORD					{
								/* Check aliases here so you don't have to reparse it */
								char * expanded = mkstr($1);
								// check if returned error
								checkAndExpandAlias(&expanded);
								if(strcmp(expanded, "loop") != 0){
									cmdtab[++currcmd] = (COMMAND*)malloc(sizeof(COMMAND));	
									cmdtab[currcmd]->atptr = (ARGTAB *)malloc(sizeof(ARGTAB));	
									cmdtab[currcmd]->nargs = 0;
									cmdtab[currcmd]->atptr->args[cmdtab[currcmd]->nargs++] = expanded;
									cmdtab[currcmd]->comname = expanded;
									/* Check WildCard */
									wildcardCheck(expanded);								
								}else{errorCheck = 1;}
							}
		| WORD.args WORD			{
								if(cmdtab[currcmd]->nargs + 1 > MAXARGS)
								{
									yyerror("Too many arguments");
								}else
								{
									char * word;
									word = mkstr($2);
									if (wildcardCheck(word)){
										executingWildcard(word);
									} else {
										cmdtab[currcmd]->atptr->args[cmdtab[currcmd]->nargs++] = word;
									}
								}
							}
		| WORD.args TEXT			{
								if(cmdtab[currcmd]->nargs + 1 > MAXARGS)
								{
									yyerror("Too many arguments");
								}else
								{
								cmdtab[currcmd]->atptr->args[cmdtab[currcmd]->nargs++] = rmvquotes($2);
								}
							}
pipeline:	WORD.args PIPE WORD.args		{
								/* example: cmd1 arg1 arg2 | cmd2 arg1 */						
							}
		| pipeline PIPE WORD.args		{
								/* Recursive definition for n pipes */
							}
rdrerr:		ERR2FILE WORD				{
								stderr_choice = TOFILE;
								err_outfile = mkstr($2);
								/* 2>file */
								/* Redirect standard error to a file */
							}
		| ERR2OUT				{
								stderr_choice = TOOUT;
								/* 2>&1 */
								/* Redirect std error to std output */
							}
GT.opt:		GT					{
								gt_choice = CREATE;
								/* > */
								/* Create file */
								/* (overwrite if existent) */
								/* example: A > B) */
								/* This redirects A's output to file B */
							}
		| GT2					{
								gt_choice = CONCAT;
								/* >> */
								/* Concatenate file */
								/* (create if nonexistent) */
								/* example: A >> B */
								/* This adds A's output to file B */
							}
ext.cmd.iordr:	ext.cmd GT.opt WORD			{
								redirection = OUT;
								outfile = mkstr($3);
								/* example1: cmd1 > file1 */
								/* example2: cmd2 >> file2 */
							}
		| ext.cmd LT WORD			{
								redirection = IN;
								infile = mkstr($3);
								/* Take input from file */
								/* example: cmd < file */
							}
		| ext.cmd GT.opt WORD LT WORD		{
								redirection = OUTIN;
								outfile = mkstr($3);
								infile = mkstr($5);
								/* Redirect output and input */
								/* Order of redirection does not matter */
								/* Output is redirected first here */
							}
		| ext.cmd LT WORD GT.opt WORD		{
								redirection = INOUT;
								outfile = mkstr($5);
								infile = mkstr($3);
								/* Redirect input and output */
								/* Order of redirection does not matter */
								/* Input is redirected first here */
							}
ext.cmd.rdrerr:	ext.cmd rdrerr				{	
								/* example: cmd1 arg | cmd2 arg 2> file */
							}
		| ext.cmd.iordr rdrerr			{
								redirect_io = 1;
								/* example: cmd1 | cmd2 > file1 2> file2 */
							}
ext.cmd.background: ext.cmd AMPERSAND			{
								/* Non-redirected command line + & */
							}
		| ext.cmd.rdrerr AMPERSAND		{	
								/* Redirected command line + & */
							}
%%




