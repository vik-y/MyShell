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
	int id;
	char command[30];
	int status;
	int pid;
}job;

void addJob(job *j, int, char *);

void deleteJob(job *j, int id);

void printJobs(job *j, int jobsIndex);

#endif /* JOBS_H_ */
