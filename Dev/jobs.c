/*
 * A Linked List Data Structure to make Background process handling easier
 * By Vikas Yadav
 */

#include "jobs.h"
#include <stdio.h>
#include <string.h>

void addJob(job *jobs, int jobsIndex, char *cmdLine){
	jobs[jobsIndex].id = jobsIndex;
	jobs[jobsIndex].status = 0;
	strcpy(jobs[jobsIndex].command, cmdLine);
}

void deleteJob(job *j, int id){
	j[id].status = 1;
	// jon completed
	// Not deleting job from memory as that would require us to shift values
	// Memory leak obviously but for the sake of time keeping it.
}

void printJobs(job *j, int jobsIndex){
	int i;
	printf("ID	Command jobsIndex %d\n", jobsIndex);

	for(i=0;i<jobsIndex;i++){
		if(i!=0 && j[i].status==0) printf("%d	%s\n", j[i].id, j[i].command);
	}
}

