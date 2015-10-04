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

int historyCount=0;
int *jobsCount;
char history[100][20];
job *jobs;
pid_t background_pid;
int jobsIndex=0;
int historyIndex=0;

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
	j[jobsIndex].pid = getpid();
	// Storing the pid of background process
	// Will be used to kill a process

	pid_t pid; int status;
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
	printf("\nStarted job with pid: %d \n", jobsIndex);
	pid = fork();
	if(pid==0){
		printf("runnning %d\n", (int)getpid());
		execvp(com->command, com->VarList); // Executing the program on a separate thread
		exit(1);
	}
	wait(&status); // Waiting for thread to terminate
	printf("\nCompleted job with pid:%d \n", jobsIndex);// Job is complete

	j[jobsIndex].status=COMPLETE;// Setting the flag of job as Complete
	jobsCount[0]--;
	printf("Number of pending processes %d\n", jobsCount[0]);
}

void  INThandler(int sig)
{
	char  c;

	signal(sig, SIG_IGN);
	printf("\n");
	signal(SIGINT, INThandler);
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

	shmid = shmget(IPC_PRIVATE, 300*sizeof(job), IPC_CREAT | 0666);
	jobs = (job *)shmat(shmid, NULL, 0); // Assigning shared memory to jobs so that all processes can see pending jobs
	jobs[0].id = 0;

	shmid = shmget(IPC_PRIVATE, 1*sizeof(int), IPC_CREAT | 0666);
	jobsCount = (int *)shmat(shmid, NULL, 0); // Shared memory to keep track of incomplete processes count

	jobsCount[0] = 0;
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
			if(jobsCount[0]!=0){
				//pending processes are there
				printf("Pending processes are there\n");
				printf("Are you sure you want to exit, pending processes will be killed\n");
			}
			else exit(1);
		}

		/* Additonal inbuilt functions start here */
		if (isBuiltInCommand(com->command) == JOBS){
			printJobs(jobs, jobsIndex+1);
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
				kill(jobs[kill_pid].pid, SIGKILL); // Using inbuilt function to kill a program
				jobs[kill_pid].status = 1 ; //
				jobsCount[0]--;
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


		if(info->boolBackground){
			/*check if '&' was found | Need to run process in background*/
			pid_t pid;

			//Adding a new job
			// convert this into a function
			jobsIndex++;
			jobsCount[0]++;
			addJob(jobs, jobsIndex, cmdLine);
			//Done adding a new job

			printf("Number of pending processes %d\n", jobsCount[0]);
			pid = fork();
			if(pid==0){
				//forking to run this process in background since '&' was found
				runInBackground(jobs, cmdLine);
				exit(1);
			}
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
				signal(SIGINT, INThandler);
				//kill(pid, SIGKILL);
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






