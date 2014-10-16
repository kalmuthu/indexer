
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "tokenizer.h"
#include "sorted-list.h"
#include <unistd.h>
#include <stdlib.h>
#include "indexer.h"
#include <dirent.h>
#include "uthash.h"

HashNodePtr hashtable;
SortedListPtr tokens;


void basicDestructor(void* v){
	return;
}
void mallocDestructor(void *p){
	free(p);
}

void HashDestroyer(HashNodePtr ptr){
	free(ptr);
	free(ptr->records);
	return;
}

int compareStrings(void *p1, void *p2)
{
	char *s1 = p1;
	char *s2 = p2;

	return strcmp(s1, s2);
}



int recordCompare( void* a, void* b )
{
	int aa = ((RecordPtr)a)->occurences;
	int bb = ((RecordPtr)b)->occurences;

	return aa - bb;
}


HashNodePtr Hasher( char* key )
{
	HashNodePtr hashNode = NULL;
	//HASH_FIND(hh, hashtable, key, strlen(key), hashNode);
	HASH_FIND_STR( hashtable, key, hashNode );
	return hashNode;
}

HashNodePtr createHashNode()
{
	HashNodePtr hash =  malloc(sizeof(struct HashNode));
	hash->word = NULL;
	hash->records = NULL;
	hash->num_records = 0;

	return hash;
}

RecordPtr createRecordPtr()
{
	RecordPtr record =  malloc(sizeof(struct Record));
	record->file_path = NULL;
	record->occurences = 0;

	return record;
}

int insertNewRecord(HashNodePtr hashptr, char* file_path)
{


	RecordPtr newRecord = createRecordPtr();
	
	newRecord->file_path = file_path;
	newRecord->occurences++;

	int response = SLInsert(hashptr->records, newRecord);
	

	if(response)
	{
		return 1;

	}

	else
	{
		free(newRecord);
		return 0;
	}

}


int addRecord(char* file_path, HashNodePtr hash )
{
	struct Node * target;
	int cmp = 0;
	RecordPtr curr = NULL;

	for(target = hash->records->head; target!=NULL; target = target->next )
	{
		curr = (RecordPtr)(target->data);
		cmp = strcmp( file_path, curr->file_path );

		if( cmp == 0 )
		{
			curr->occurences++;
			return 1;
		}
	}

	// no existing record had that file_path, create a new record and insert it

	int response = insertNewRecord( hash, file_path );


	if(response)      // new Record was successfully inserted
	{	
		hash->num_records++;
	}

	return response;
}



int addHashNode(char* token, char* file_path)
{
	HashNodePtr hash = NULL;
	hash = createHashNode();

	
	hash->word = token;
	SortedListPtr newList =  SLCreate(recordCompare, mallocDestructor); 
	hash->records = newList;
	hash->num_records = 0;


	int response = insertNewRecord(hash, file_path); //insert head record to list since it's a new Node

//	printf("%p \n", hash->records->head);

	//RecordPtr r = hash->records->head->data;
	//printf("%s, ", r->file_path);


	if (response)
	{
		hash->num_records++;
		SLInsert(tokens, token);       //insert token into master sorted LL of all gathered tokens

		RecordPtr r = hash->records->head->data;
		printf("%s \n", r->file_path);

	//printf("word before: %s \n", hash->word);
	//printf("head before: %s \n", hash->records->head);


		HASH_ADD_KEYPTR( hh,hashtable,hash->word,strlen(hash->word), hash);	

	//	printf("word after: %s \n", hash->word);
	//	printf("head after: %s \n", hash->records->head);

		HashNodePtr h = NULL;
		h = Hasher(token);
		//printf("%s \n", h->word);
		//printf("%p \n", h->records->head);
		//printf("%d \n", h->num_records);
		//exit(EXIT_FAILURE);
		//RecordPtr rr = h->records->head->data;
		//printf("%s \n", rr->file_path);


		return 1;
	}

	else
	{
		HashDestroyer(hash);      //may be wonky :0
		return 0;
	}
}

char* FiletoString(char* file_path)
{

	FILE *file = fopen(file_path, "r");


	if (file == NULL)
	{
		printf("Error opening file, %s \n", file_path);
		return NULL;
	}



	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file); 
	rewind(file);

	char *s = (char *) malloc(sizeof(fileSize));
	int result = fread(s, 1, fileSize, file);
	fclose(file);

	if (result == 0) {
		return NULL;
	} 

	return s;
}


void walkDir(char *basedir)
{

	DIR *dir;
	char b[512];
	struct dirent *ent;
	
	dir = opendir(basedir);
	
	if(dir != NULL)
	{
		printf("\n\tWalking \"%s\"\n", basedir);
		//ent = readdir(dir);

		
		while((ent = readdir(dir)) != NULL)
		{

			printf("%s \n", ent->d_name);
			if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			{
				continue;
			} 

			DIR* dirtest = opendir(ent->d_name);
			
			char entpath[] = "";
			strcat(entpath, basedir);
			strcat(entpath, "/");
			strcat(entpath, ent->d_name);  
			
			if(dirtest) // directory
			{
				printf("\n\tDIR: %s\n", ent->d_name);
				walkDir(ent->d_name);
				closedir(dirtest);
			}
			else // file
			{
				char* s = FiletoString(entpath);
				TokenizerT* tokenizer = TKCreate(s);
				char* token = TKGetNextToken( tokenizer );
				

				while (token)  // loop through all tokens, inserting new Records into hashnodes / creating new Hashnodes / updating existing records
				{
					HashNodePtr h = NULL;
					h = Hasher(token);

					if (!h)
					{
						int Nodegen = addHashNode(token, entpath);

						if (!Nodegen)
						{
							printf("Error creating new hash node \n");
							return;
						}

					}

					if (h)
					{
						int Fileinsert = addRecord(entpath, h);

						if (!Fileinsert)
						{
							printf("Error adding new Record to existing Node \n");
							return;
						}
					}
					token = TKGetNextToken(tokenizer);
				}
			}
		
		}
	closedir(dir);	
	}
}



int main (int argc, char** argv)
{

	tokens = SLCreate(compareStrings, basicDestructor);
	hashtable = NULL;

	walkDir(argv[1]);

	SortedListIteratorPtr iter = SLCreateIterator(tokens);

	

	char* token = (char*) SLGetItem(iter);
	HashNodePtr hashy = Hasher(token);
	
	SortedListIteratorPtr recorditer = SLCreateIterator(hashy->records);
	RecordPtr record = SLGetItem(recorditer);



	while (token != NULL)
	{
		hashy = Hasher(token);
		RecordPtr record = (RecordPtr) hashy->records->head->data;
		printf("word: %s, file: %s, occ: %d \n", hashy->word, record->file_path, record->occurences);
		token = (char*) SLNextItem(iter);
	} 


}	