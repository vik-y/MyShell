#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"   /*include declarations for parse-related structs*/
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


#define HISTORYLENGTH 100 // 100 commands will be stored in history


enum BUILTIN_COMMANDS {
	NO_SUCH_BUILTIN=0,
	EXIT,
	JOBS
};

int historyCount;
int jobsCount;
char history[100][20];
char jobs[100][20];
pid_t background_pid;

void printHistory(){
	/*Prints History*/
	int i = 0;

	while(history[i][0] != '\0'){
		printf("%d  %s\n", i+1, *(history + i));
		++i;
	}
}

void printJobs(){
	int i = 0;
	if(jobs[i][0] != '\0'){
		printf("%d %s\n", i+1, *(jobs + i));
		++i;
	}else{
		printf("\n No background jobs \n");
	}

}

/*prints current working directory*/
void printPrompt(){
	// Satisfies Requirement 1
	char cwd[256];

	if (getcwd(cwd, sizeof(cwd)) == 0){
		perror("getcwd() is not working");
	}else{
		printf("MyShell:%s",cwd);

	}
}


char *
buildPrompt()
{ 
	printPrompt();
	return  "$ ";
}

int
isBuiltInCommand(char * cmd){

	if ( strncmp(cmd, "exit", strlen("exit")) == 0){
		return EXIT;
	}
	return NO_SUCH_BUILTIN;
}


int main (int argc, char **argv)
{
	char * cmdLine;
	int historyIndex;
	int jobOffset;
	parseInfo *info; /*info stores all the information returned by parser.*/
	struct commandType *com; /*com stores command name and Arg list for one command.*/

#ifdef UNIX
	fprintf(stdout, "This is the UNIX/LINUX version\n");
#endif

#ifdef WINDOWS
	fprintf(stdout, "This is the WINDOWS version\n");
#endif

	while(1){
		/*insert your code to print prompt here*/

#ifdef UNIX
		cmdLine = readline(buildPrompt());
		if (cmdLine == NULL) {
			fprintf(stderr, "Unable to read command\n");
			continue;
		}
#endif

		/*insert your code about history and !x !-x here*/
		if(cmdLine[0] == '!'){
			// !-x or !x
			char *tmp;
			int num;
			tmp = cmdLine+1; // Take location after '!'
			num = atoi(tmp); // Convert the values after ! to int
			printf("Result %d\n", num);

			if(num<0){
				// !-x case

			}
			else{
				// !x case
			}
		}

		/*calls the parser*/
		info = parse(cmdLine);
		if (info == NULL){
			free(cmdLine);
			continue;
		}
		/*prints the info struct*/
		print_info(info);

		/*com contains the info. of the command before the first "|"*/
		com=&info->CommArray[0];
		if ((com == NULL)  || (com->command == NULL)) {
			free_info(info);
			free(cmdLine);
			continue;
		}
		/*com->command tells the command name of com*/
		if (isBuiltInCommand(com->command) == EXIT){
			exit(1);
		}

		strcpy(history[historyIndex], cmdLine);
		historyCount++;
		historyIndex++;
		/*insert your code here.*/

		printHistory();

		free_info(info);
		free(cmdLine);
	}/* while(1) */
}






