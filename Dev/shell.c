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
	JOBS,
	CD,
	HISTORY,
	KILL,
	HELP
};

int historyCount;
int jobsCount;
char history[100][20];
char jobs[100][20];
pid_t background_pid;

void printHistory(){
	/*Prints History*/

	printf("History\n");
	int i = 0;

	while(history[i][0] != '\0'){
		printf("%d  %s\n", i+1, *(history + i));
		++i;
	}
}

void printJobs(){
	printf("Background Jobs\n");
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
	if ( strncmp(cmd, "jobs", strlen("jobs")) == 0){
		return JOBS;
	}
	if ( strncmp(cmd, "cd", strlen("cd")) == 0){
		return CD;
	}
	if ( strncmp(cmd, "kill", strlen("kill")) == 0){
		return KILL;
	}
	if ( strncmp(cmd, "history", strlen("history")) == 0){
		return HISTORY;
	}
	if ( strncmp(cmd, "help", strlen("help")) == 0){
		return HELP;
	}
	return NO_SUCH_BUILTIN;
}


int main (int argc, char **argv)
{
	char * cmdLine;
	int historyIndex =0;
	int jobOffset;
	int jobsIndex = 0;
	parseInfo *info; /*info stores all the information returned by parser.*/
	struct commandType *com; /*com stores command name and Arg list for one command.*/

	FILE *inputfile = NULL;
	FILE *outputfile = NULL;
	int STDIN, STDOUT;

	STDIN = dup(fileno(stdin)); // Telling shell to use default STDIN
	STDOUT = dup(fileno(stdout)); // Telling shell to use default STDOUT

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
				strcpy(cmdLine, history[historyCount+num]);
			}
			else{
				// !x case
				strcpy(cmdLine, history[num-1]);
			}
		}

		/*calls the parser*/
		info = parse(cmdLine);
		if (info == NULL){
			free(cmdLine);
			continue;
		}
		/*prints the info struct*/
		//print_info(info);

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

		/* Additonal inbuilt functions start here */
		if (isBuiltInCommand(com->command) == JOBS){
			printJobs();
		}
		if (isBuiltInCommand(com->command) == HISTORY){
			printHistory();
		}
		if (isBuiltInCommand(com->command) == KILL){

			const char * const path = com->VarList[1];
			int kill_pid = 0;
			kill_pid = atoi(com->VarList[1]); //Get 1st argument from command

			if(kill_pid <= 0){
				printf("\nInvalid pid:%d\n", kill_pid);
			}else{
				kill(kill_pid, SIGKILL); // Using inbuilt function to kill a program
			}

		}
		if (isBuiltInCommand(com->command) == HELP){
			printf("help: prints this help screen.\n\n");
			printf(" jobs - prints out the list of running background jobs.\n\n");
			printf(" kill [process_id] - kills background process_id to kill a running background job.\n\n");
			printf(" history - display a record of the last 100 commands typed.\n\n");
			printf(" ![int] - execute [int] command in history array\n\n");
			printf(" cd [location] - change directory\n\n");
			printf(" sometext < infile.txt > outfile.txt - create a new process to run sometext and assign STDIN for the new process to infile and STDOUT for the new process to outfile\n\n");
			printf(" exit - exit or quit shell.\n\n");
			printf("Append & to the end of any command to run it in the background.\n\n");

		}

		/* Additional inbuilt functions end here */

		/*insert your code here.*/


		//adds to history
		strcpy(history[historyCount], cmdLine);
		historyCount++;
		//printHistory();
		//printJobs();


		/*check if Background process request*/
		if(info->boolBackground){
			printf("& found\n");
			// Run the process in background
			char * previous_command;
			previous_command = (char*)com->command;
			printf("%s\n", previous_command);

			strcpy(jobs[jobsIndex], cmdLine);
			jobsIndex++;
		}
		else{
			// If not background then it should run in the foreground
			int status;
			pid_t pid;
			pid = fork(); // Making a child to run the command
			if(pid==0){
				if(strcmp(info->inFile,"")!=0){
					// Checks if a file was specified to be used as stdin
					inputfile = fopen(info->inFile, "r");
					dup2(fileno(inputfile), 0);
				}

				if(strcmp(info->outFile,"")!=0){
					// Checks if a file was specified to be used as stdout
					printf("outFile specified %s\n", info->outFile);
					outputfile = fopen(info->outFile, "w");
					dup2(fileno(outputfile), 1);
				}
				execvp(com->command, com->VarList);
				printf("Invalid Command. Command Not found\n");
				// If error happens then control comes here
				exit(1);
			}
			else {
				wait(&status); // Waiting for execvp to end
				printf("\n");
			}
		}

		fflush(stdout); // flushing the output
		fflush(stdin); // flushing the input

		if (inputfile != NULL){
			fclose(inputfile);
		}

		if (outputfile != NULL){
			fclose(outputfile);
		}

		dup2(STDIN, 0); // Resetting standard input to STDIN
		dup2(STDOUT,1); // Resetting standard output to STDOUT

		free_info(info);
		/*
		 * Note: There was a bug in free_info function which was not letting the
		 * input and output pipes work ( "<" , ">") properly.
		 */
		free(cmdLine);
	}/* while(1) */
}






