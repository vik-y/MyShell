/*
 * By Vikas Yadav
 *
 * Tried linked list here but gave trouble when linked list was used as a shared memory
 * Dynamic allocation gave issues with linked list while sharing memory and hence moved
 * to array for the sake of time
 */

#include "jobs.h"
#include <stdio.h>
#include <string.h>

extern int *jobsCount;

void addJob(job *jobs, int *jobsIndex, char *cmdLine){
	jobsIndex[0]++;
	jobsCount[0]++;
	jobs[*jobsIndex].id = *jobsIndex;
	jobs[*jobsIndex].status = 0;
	strcpy(jobs[*jobsIndex].command, cmdLine);
}

void deleteJob(job *j, int id){
	j[id].status = 1;
	// job completed
	// Not deleting job from memory as that would require us to shift values
	// Memory overhead obviously but for the sake of time keeping it.
	// Tried using linked list to save memory but linked list gives trouble
	// when memory is shared
}

void printJobs(job *j, int jobsIndex){
	int i;
	if(jobsCount[0]==0){
		printf("No pending processes\n");
	}
	else{
		printf("Number of pending processes %d\n", jobsCount[0]);
		printf("ID	Command\n");

		for(i=0;i<jobsIndex;i++){
			if(i!=0 && j[i].status==0) printf("%d	%s\n", j[i].id, j[i].command);
		}
	}
}

int getProcessId(job *j, int jobsIndex){
	return j[jobsIndex].pid;
}

