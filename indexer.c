
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
char s[5000];

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

	return strcmp(s2, s1);
}



int recordCompare( void* a, void* b )
{
	int aa = ((RecordPtr)a)->occurences;
	int bb = ((RecordPtr)b)->occurences;

	return aa-bb;
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
		printf("error: %s \n", newRecord->file_path);
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


	if (response)
	{
		hash->num_records++;
		SLInsert(tokens, token);       //insert token into master sorted LL of all gathered tokens
		HASH_ADD_KEYPTR( hh,hashtable,hash->word,strlen(hash->word), hash);	

		return 1;
	}

	else
	{
		return 0;
	}
}

char* FiletoString(char* file_path)
{


	memset(&s[0], 0, sizeof(s));

	FILE *file = fopen(file_path, "r");

	if (file == NULL)
	{
		printf("Error opening file/dir: %s \n", file_path);
		printf("Indexer will now exit \n");
		exit(EXIT_FAILURE);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	//char *s = (char *) malloc(fileSize);
	int result = fread(s, 1, fileSize, file);
	//bzero(s,500);

	fclose(file);

	if (result == 0) {
		return NULL;
	} 

	return s;
}

void indexify(char* entpath)
{
				char* ss = FiletoString(entpath);

				TokenizerT* tokenizer = TKCreate(ss);
				char * token;
				token = TKGetNextToken(tokenizer);
				
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
					else
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


void walkDir(char *basedir)
{


	DIR *dir;
	//char b[512];
	struct dirent *ent;
	char filenames[250][100];
	int fileindex = 0;
	int fileflag = 0;
	char* entpath;
	dir = opendir(basedir);
	if (!dir)
	{
		indexify(basedir);
	}

	
	if(dir != NULL)
	{	
		while((ent = readdir(dir)) != NULL)
		{

			if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			{
				continue;
			} 
			entpath = filenames[fileindex];
			strcat(entpath, basedir);
			strcat(entpath, "/");
			strcat(entpath, ent->d_name); 

			fileindex++; 
			DIR* dirtest = opendir(entpath);
			if(dirtest) // directory
			{
				walkDir(entpath);
				closedir(dirtest);
			}
			else // file
			{
				indexify(entpath);
			}
		}
	closedir(dir);	
	}
}

int checkOverWrite (char* filename)
{
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if (strcmp(dir->d_name, filename) == 0)
      {
      	printf("Safety trigger: Attempting to overwrite existing file in this directory. \n");
      	printf("Indexer will now exit \n");
      	exit(EXIT_FAILURE);
      	return 0;
      }
    }
    closedir(d);
  }

  return 1;
}



void writeFile( char* file_path )
{
	SortedListIteratorPtr iter = SLCreateIterator(tokens);;
	FILE* new_file = fopen( file_path, "w" );
	char* token = (char*) SLGetItem(iter);
	HashNodePtr hashy = Hasher(token);
	int i = 0;
	printf("\n");
		
		while (token)
		{

		hashy = Hasher(token);
		fputs("<list> ", new_file);
		fputs(token, new_file);
		fputs("\n", new_file);
		SortedListIteratorPtr recorditer = SLCreateIterator(hashy->records);
		RecordPtr record = SLGetItem(recorditer);


			while (record)
			{
				if (i == 5)
				{
					fputs("\n", new_file);	
					i = 0;
				}
				fputs( record->file_path, new_file );
				fputs( " ", new_file );
				char occ[15];
				snprintf(occ, 20, "%zd", record->occurences );
				fputs( occ, new_file );
				fputs( " ", new_file );
				record = SLNextItem(recorditer);
				i++;
			}
		fputs("\n", new_file);
		fputs("</list>", new_file);
		fputs("\n", new_file);
		i = 0;	

		token = (char*) SLNextItem(iter); 
		}

		fclose(new_file);
}


int main (int argc, char** argv)
{
	
	tokens = SLCreate(compareStrings, basicDestructor);
	hashtable = NULL;
	
	checkOverWrite(argv[1]);
	walkDir(argv[2]);
	writeFile(argv[1]);
	printf("\n");
	printf("File generated successfully \n");


}	
