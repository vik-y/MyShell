#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"   /*include declarations for parse-related structs*/
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "jobs.h" // Created a jobs.h and jobs.c file to handle background processes effectively

// Including for shared memory usage
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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
job *jobs;
pid_t background_pid;
int jobsIndex;
int historyIndex;

void printHistory(){
	/*Prints History*/

	printf("History\n");
	int i = 0;

	while(history[i][0] != '\0'){
		printf("%d  %s\n", i+1, *(history + i));
		++i;
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

void runInBackground(job *j, char *command){
	parseInfo * info;
	struct commandType *com;
	/*calls the parser*/
	info = parse(command);
	if (info == NULL){
		printf("Error\n"); exit(1);
	}
	/*prints the info struct*/
	//print_info(info);

	/*com contains the info. of the command before the first "|"*/
	com=&info->CommArray[0];
	if ((com == NULL)  || (com->command == NULL)) {
		free_info(info);
		//free(cmdLine);
		//continue;
	}
	execvp(com->command, com->VarList);

	printf("\njob with pid %d completed\n", jobsIndex);
	deleteJob(jobs, jobsIndex);
}

int main (int argc, char **argv)
{
	char * cmdLine;
	int shmid;
	int jobOffset;
	parseInfo *info; /*info stores all the information returned by parser.*/
	struct commandType *com; /*com stores command name and Arg list for one command.*/

	FILE *inputfile = NULL;
	FILE *outputfile = NULL;
	int STDIN, STDOUT;

	shmid = shmget(IPC_PRIVATE, 1*sizeof(job), IPC_CREAT | 0666);
	jobs = (job *)shmat(shmid, NULL, 0);


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
			printJobs(jobs);
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
			pid_t pid;
			job *temp;

			temp = (job *)malloc(sizeof(job));
			temp->command = (char *)malloc(sizeof(cmdLine));

			printf("Writing: %s\n", cmdLine);

			jobsIndex++;
			temp->id = jobsIndex; temp->next = NULL; strcpy(temp->command, cmdLine);
			addJob(jobs, temp);

			pid = fork();
			if(pid==0){
				runInBackground(jobs, cmdLine);
				exit(1);
			}
			/*pid_t pid;
			job *temp;

			temp = (job *)malloc(sizeof(job));
			temp->command = (char *)malloc(sizeof(cmdLine));

			printf("Writing: %s\n", cmdLine);

			jobsIndex++;
			temp->id = jobsIndex; temp->next = NULL; strcpy(temp->command, cmdLine);
			addJob(jobs, temp);
			printJobs(jobs);

			 Adding job into background ends here
			pid = fork(); // fork a thread inside which we will wait for processes to terminate;
			if(pid==0){
				int status;
				// We will fork another process inside a fork process as we want to wait for the process
				// to terminate and then update the jobs table
				pid = fork();
				if(pid==0){
					execvp(com->command, com->VarList);
					exit(1);
				}
				wait(&status);
				printf("\njob with pid %d completed\n", jobsIndex);

				deleteJob(jobs, jobsIndex);
				exit(1);
			}
			else{
				printf("Background job with pid: %d started\n", jobsIndex);
				printJobs(jobs);
			}*/
		}
		else if(isBuiltInCommand(com->command)==NO_SUCH_BUILTIN){
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






