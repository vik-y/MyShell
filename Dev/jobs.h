/*
 * jobs.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Vikas Yadav
 */

#ifndef JOBS_H_
#define JOBS_H_

#include "parse.h"

typedef struct job{
	int id;
	char command[30];
	int status;
}job;

void addJob(job *j, job newjob, int jobsIndex);

void deleteJob(job *j, int id);

void printJobs(job *j, int jobsIndex);

#endif /* JOBS_H_ */
