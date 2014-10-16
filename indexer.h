#ifndef index_index_h
#define index_index_h

#include "sorted-list.h"
#include "uthash.h"

struct Record
{
	char* file_path;
	int occurences;
};


typedef struct Record* RecordPtr;

struct HashNode
{
	char* word;
	SortedListPtr records;
	int num_records;
	UT_hash_handle hh;
};

typedef struct HashNode* HashNodePtr;

#endif