/*
 * jobs.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Vikas Yadav
 */

#ifndef JOBS_H_
#define JOBS_H_

#include "parse.h"

#define COMPLETE 1

typedef struct job{
	int id; // Stores Id of the process
	char command[30]; // Stores the command passed in command line
	int status; // 1 = complete, 0 = incomplete
	int pid; // the pid assigned to this job by OS. Will be used to kill
}job;

void addJob(job *j, int *, char *);

void deleteJob(job *j, int id);

void printJobs(job *j, int jobsIndex);

int getProcessId(job *j, int jobsIndex);

#endif /* JOBS_H_ */
