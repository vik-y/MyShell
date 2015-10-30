#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"   /*include declarations for parse-related structs*/
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "linkedList.h"
#include "jobs.h" // Created a jobs.h and jobs.c file to handle background processes effectively

// Including for shared memory usage
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

enum BUILTIN_COMMANDS {
	NO_SUCH_BUILTIN=0,
	EXIT,
	JOBS,
	CD,
	HISTORY,
	KILL,
	HELP,
	SETENV,
	PUSHD,
	POPD,
	DIRS
};

lnode *dirs; // Shell Part 2. To store directories for using pushd and popd dirs is a linked list. see linkedList.c and linkedList.h
// for more info
int historyCount=0; //Stores number of items in history
int *jobsCount; // Stores number of background processes
char history[100][20]; // Stores an array of commands
job *jobs; // An array of pending jobs
int jobsIndex=0; // Offset of where to add a new job in jobs array
static int pushdUsed;

void printHistory(){
	/*Prints History*/
	printf("History\n");
	int i = 0;

	while(history[i][0] != '\0'){
		printf("%d  %s\n", i+1, *(history + i));
		++i;
	}
}

void print_N_History(int n){
	/*
	 * prints last N commands
	 */

	int i;
	if(n<historyCount)
		i= historyCount-n;
	else
		i=0;

	while(history[i][0] != '\0'){
		printf("%d  %s\n", i+1, *(history + i));
		++i;
	}
}


/*prints current working directory*/
void printPrompt(){
	// Satisfies Requirement 1
	char cwd[256];
	char *username=getenv("USER");
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	if (getcwd(cwd, sizeof(cwd)) == NULL){
		perror("getcwd() is not working");
	}else{
		printf("%s@%s:~%s", username, hostname, cwd);
	}
}

char *
buildPrompt()
{ 
	printPrompt();
	return  "$ ";
}

void changeDirectory(char *loc){
	if(loc!=NULL){
		if(chdir(loc)==0 && pushdUsed>0)
			pushd(&dirs, getcwd(NULL, 0));
	}
	else{
		// goes to home if no argument provided
		char location[50] = "/home/";
		strcat(location, getenv("USER"));
		if(chdir(location)==0 && pushdUsed>0)
			pushd(&dirs, getcwd(NULL, 0)); // directory changed successfully
	}
}

int
isBuiltInCommand(char * cmd){
	// added more builtin commands as per the requirement
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
	if ( strncmp(cmd, "setenv", strlen("setenv")) == 0){
		return SETENV;
	}
	if ( strncmp(cmd, "pushd", strlen("pushd")) == 0){
		return PUSHD;
	}
	if ( strncmp(cmd, "popd", strlen("popd")) == 0){
		return POPD;
	}
	if ( strncmp(cmd, "dirs", strlen("dirs")) == 0){
		return DIRS;
	}

	return NO_SUCH_BUILTIN;
}

void runInBackground(job *j, char *command){
	/*h
	 * Used to run a job in background.
	 * Helps in implement background jobs functionality
	 */
	pid_t pid; int status;
	parseInfo * info;
	struct commandType *com;

	/*calls the parser*/
	info = parse(command);
	if (info == NULL){
		printf("Error\n"); exit(1);
	}

	com=&info->CommArray[0];
	if ((com == NULL)  || (com->command == NULL)) {
		free_info(info);
		//free(cmdLine);
		//continue;
	}
	/*parser done here*/

	/*forking to run execvp in background */
	printf("\nStarted job with pid: %d \n", jobsIndex); // Here pid is with respect to this shell - not OS
	pid = fork(); // Creating a new thread which runs execvp
	if(pid==0){
		j[jobsIndex].pid = getpid();
		// Storing the pid of `background process
		// Will be used to kill a process

		printf("running %d\n", (int)getpid());
		execvp(com->command, com->VarList); // Executing the program on a separate thread
		exit(1);
	}

	wait(&status);
	// Waiting for thread to terminate
	// No need to wait pid since this will only capture the process forked above.
	printf("\nCompleted background job with pid:%d \n", jobsIndex);// Job is complete

	j[jobsIndex].status=COMPLETE;// Setting the flag of job as Complete
	jobsCount[0]--;
	printf("Number of pending processes %d\n", jobsCount[0]);
}

void  INThandler(int sig)
{
	// Used to handle the Ctrl+C event
	// An add on feature
	char  c;
	signal(sig, SIG_IGN);
}

void killProcess(int kill_pid){
	/*
	 * Kills a process with pid <kill_pid> in the shell
	 * kill_pid is a pid with respect to the shell - not system
	 */
	if(kill_pid<=jobsIndex && jobs[kill_pid].status==0){
		kill(jobs[kill_pid].pid, SIGKILL); // Using inbuilt function to kill a program
		jobs[kill_pid].status = 1 ; // jobs[kill_pid] gives the pid allocated by OS
		//jobsCount[0]--;
		printf("PID: %d Killed\n", kill_pid);
	}
	else printf("Job Does not Exist. Unable to kill\n");
}

int main (int argc, char **argv)
{
	dirs = NULL;
	pushdUsed=0;
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
	jobsCount = (int *)shmat(shmid, NULL, 0); // Shared memory to keep track of background processes count

	jobsCount[0] = 0;
	STDIN = dup(fileno(stdin));
	STDOUT = dup(fileno(stdout));

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

		if (isBuiltInCommand(com->command) == CD){
			changeDirectory(com->VarList[1]);
		}

		if (isBuiltInCommand(com->command) == EXIT){
			if(jobsCount[0]>0){
				//pending processes are there
				printf("Pending processes are there\n");
				printf("Are you sure you want to exit, pending processes will be killed\n");

				char c = getchar();
				if(c=='y' || c=='Y'){
					// Kill all pending processes before exiting
					// This satisfies Requirement 9
					int i;
					for(i=0;i<=jobsIndex;i++){
						killProcess(i);
					}
				}
			}
			else exit(1); // No pending processes
		}

		/* Additonal inbuilt functions start here */

		if (isBuiltInCommand(com->command) == JOBS){
			printJobs(jobs, jobsIndex+1);
		}
		if (isBuiltInCommand(com->command) == HISTORY){
			if(com->VarNum>1){
				//This modification has been done to implement the history -s num command
				if(strcmp(com->VarList[1], "-s")==0)
				{
					printf("Changing the buffer size of history \n");
					int bufsize = atoi(com->VarList[2]);
					printf("Setting buffer size as: %d\n", bufsize);
				}
				else{
					int n = atoi(com->VarList[1]);
					print_N_History(n);
				}

			}
			else{
				printHistory(); // No argument given to history
			}
		}
		if (isBuiltInCommand(com->command) == KILL){

			const char * const path = com->VarList[1];
			int kill_pid = 0;
			kill_pid = atoi(com->VarList[1]); //Get 1st argument from command

			if(kill_pid <= 0){
				printf("\nInvalid pid:%d\n", kill_pid);
			}else{
				killProcess(kill_pid);
			}

		}
		if (isBuiltInCommand(com->command) == HELP){
			printf("help: prints this help screen.\n\n");
			printf(" jobs - shows the list of active background jobs.\n\n");
			printf(" kill <pid> - kills background process with pid showed in jobs.\n\n");
			printf(" history - history of last few commands.\n\n");
			printf(" !<int>- execute [int] command in history array\n\n");
			printf(" cd <location> - change working directory\n\n");
			printf(" command < infile.txt > outfile.txt - create a new process to run sometext and assign STDIN for the new process to infile and STDOUT for the new process to outfile\n\n");
			printf("setenv KEY=VALUE\n\n");
			printf(" exit - quit shell.\n\n");
			printf("Append & to the end of any command to run it in the background.\n\n");

		}


		// Shell Part 2 setenv
		if (isBuiltInCommand(com->command) == SETENV){
			// Appends key value pair at the end of the environment file
			if(com->VarNum>1){
				printf("Setenv detected\n");
				char *key, *value;

				key = strtok(com->VarList[1],"="); // Getting the key from passed parameter
				//printf("Key: %s\n", key);

				value = strtok(NULL, "="); // Getting the value from passed parameter
				//printf("Value: %s\n", value);

				setenv(key, value, 1); // 3rd parameter 1 will replace value if it exists
				// Setting the environment variable done
			}
			else
				printf("Usage: setenv KEY=VALUE\n");
		}

		// Shell part 2 pushd function

		if(isBuiltInCommand(com->command) == PUSHD){
			pushdUsed+=1;
			if(com->VarNum>1){
				changeDirectory(com->VarList[1]);
			}
			else{
				pushdUsed = 0;
				//swap directory
				char *location = swapDirectory(&dirs);
				if(location!=NULL){
					changeDirectory(location);
				}
			}
		}

		if(isBuiltInCommand(com->command) == POPD){
			pushdUsed=0;
			char *location = popd(&dirs);
			if(location!=NULL){
				changeDirectory(location);
			}
			pushdUsed=1;
		}
		// Shell part 2 dirs function
		if(isBuiltInCommand(com->command) == DIRS){
			printlist(dirs);
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

			addJob(jobs, &jobsIndex, cmdLine);
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
			int status, i;
			pid_t pid;
			struct commandType *comm;

			for (i=0; i<=info->pipeNum;i++) {
				// Should run till the number of pipes
				comm=&(info->CommArray[i]);
				if ((NULL == comm) || (NULL == comm->command)){
					printf("Command %d is NULL\n", i);
				} else {
					pid = fork(); // Making a child to run the command
					if(pid==0){
						if(info->boolInfile){
							// This case is changing input file
							// Checks if a file was specified to be used as stdin
							inputfile = fopen(info->inFile, "r");
							dup2(fileno(inputfile), 0);
						}
						if(info->boolOutfile){
							// This case is used for output redirection
							// Checks if a file was specified to be used as stdout
							printf("outFile specified %s\n", info->outFile);
							outputfile = fopen(info->outFile, "w");
							dup2(fileno(outputfile), 1);
						}
						if(info->pipeNum>0 && i<info->pipeNum){
							// This case is used for handling pipes "|"
							char filename[15];
							sprintf(filename, "out%d.txt", i);
							outputfile = fopen(filename, "w");
							dup2(fileno(outputfile), 1);
						}
						if(i!=0){
							// This case is used for handling pipes "|"
							char filename[15];
							sprintf(filename, "out%d.txt", i-1);
							inputfile = fopen(filename, "r");
							dup2(fileno(inputfile), 0);
						}
						execvp(comm->command, comm->VarList);
						printf("Invalid Command. Command Not found\n");
						// If error happens then control comes here
						exit(1);
					}

					signal(SIGINT, INThandler); // Used to handle Ctrl+C events
					wait(&status); // Waiting for execvp to end
				}
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
		free(cmdLine);
	}/* while(1) */
}
