/*
 * linkedList.c
 *
 *  Created on: Oct 29, 2015
 *      Author: Vikas Yadav
 *
 * Used to implement stack which is used to implement pushd and popd
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"


lnode *newNode(char *val){
	//A helper function to allocate memory whenever needed
	//Returns the pointer to which memory is allocated
	lnode *temp;
	temp = (lnode *)malloc(sizeof(lnode));
	strcpy(temp->dir, val);
	return temp;
}



void pushd(lnode **node, char *value){
	// Used to implement shell part 2 required of pushd function
	if(*node==NULL){
		*node = newNode(value);
		(*node)->next = NULL;
	}
	else{
		lnode *temp = *node;
		*node = newNode(value);
		(*node)->next =temp;
	}
}

char * popd(lnode **node){
	// Used to implement shell part 2 required of popd function
	// returns location of the directory where OS has to go now
	if(*node!=NULL){
		lnode *temp = *node;
		*node = (*node)->next;
		free(temp);
		return (*node)->dir;
	}
	else return NULL;
}

void insert(lnode **node, char *value){
	// LinkedList insert - inserts in the end
	if(*node==NULL){
		*node = newNode(value);
		(*node)->next = NULL;
	}
	else{
		insert(&(*node)->next, value);
	}
}

void printlist(lnode *node){
	/*A helper function to print all the nodes in the given linked list*/
	if(node!=NULL) {
		printf("%s ", node->dir);
		printlist(node->next);
	}
	else
		printf("NULL");
}

/*int main(int argc, char **argv) {
	lnode *head = NULL;
	int count = 5;
	//Testing
	//Insertion and printlist working completely fine
	//Search Working Completely Fine
	char * temp = "VIKas";
	pushd(&head, temp);
	temp = "test1";
	pushd(&head, temp);
	temp = "test2";
	pushd(&head, temp);
	temp = "test3";
	pushd(&head, temp);
	printlist(head);
	printf("\n");

	printf("popd: %s\n", popd(&head));
	printf("popd: %s\n", popd(&head));
	printf("popd: %s\n", popd(&head));
	printf("popd: %s\n", popd(&head));
	count --;

	return 1;
}*/


