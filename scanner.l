%{
#include <stdio.h>
#include "y.tab.h"
%}

%%
<<EOF>>     					{exit(1);}
setenv							{return SETENV;}
printenv						{return PRINTENV;}
unsetenv						{return UNSETENV;}
cd							{return CD;}
alias							{return ALIAS;}
unalias							{return UNALIAS;}
bye							{return BYE;}

\n							{return -1;}
[ \t]+                  				/* ignore whitespace */;
\"(.*)\"						{	
								yylval.s = yytext;
								return TEXT;
							}
[-/"$/{/}:~/./*/?,_A-Za-z0-9]*				{	
								yylval.s = yytext;
								return WORD;
							}

"|"							{return PIPE;}
"2>&1"							{return ERR2OUT;}
"2>"							{return ERR2FILE;}
">>"							{return GT2;}
">"							{return GT;}
"<"							{return LT;}
"&"							{return AMPERSAND;}
.							{yylex(); return 0;}

%%
