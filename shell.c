// NOTE: nargs (number of arguments) includes the actual command, argument list, and a null terminated character. so for example: [ cmd1 arg1 arg2 ] <-- nargs for this command line would = 4

#include "shell.h"
int cmdstart = 0;



int main(int argc, char **argv) {
	currcmd = -1;
	//cmdstart = 0;
	int running = 1;
	shell_init();

	while(running){
		if (isatty(fileno(stdin)))	{
		printPrompt();
		}
		switch(cmd = getCommand()){
			case BYERETURN:	
				running = 0;
				break;
			case ERRORS:	
				recover_from_errors();
				break;
			case OK:	
				processCommand();
				break;
			case LOOP:	
				break;
		}
	}
}
