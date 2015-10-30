/*
 * This stores struct and headers related to history file
 */

typedef struct history{
	int count; //number of entries
	int size; // total size of history
	historyEntry entries[1000]; //Can store max 1000 entries
}history;


typedef struct historyEntry{
	int id; //Every history entry will have an ID through which it can be identified uniquely
	int timestamp; // When the entry was created
	char command[100]; //Can be of maximum size 100
}historyEntry;
