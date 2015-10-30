/*
 * linkedList.h
 *
 *  Created on: Oct 30, 2015
 *      Author: bobo
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

struct lnode{
	/*
	 * Each node of a linked list will contain the value and the address of the next node
	 */
	char dir[30];
	struct lnode *next;
};

typedef struct lnode lnode;

lnode *newNode(char *val);

void pushd(lnode **node, char *value);

char * popd(lnode **node);

void insert(lnode **node, char *value);

void printlist(lnode *node);

#endif /* LINKEDLIST_H_ */
