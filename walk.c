#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "tokenizer.c"
#include <unistd.h>


#define DEBUG 1


int isDir(const char *file_path)
{
	struct stat s;
	stat(file_path, &s);
	return S_ISDIR(s.st_mode);
}


char* getFileContents(char* file_path )
{

	FILE *fp = fopen(file_path, "r");


	if (fp == NULL)
	{
		printf("=] \n");
		return NULL;
	}



	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	//fseek(fp, 0, SEEK_SET); 
	rewind(fp);

	char *fileString = (char *) malloc(sizeof(fileSize));
	size_t result = fread(fileString, 1, fileSize, fp);
	fclose(fp);

	if (result == 0) {
		return NULL;
	} 

		

	return fileString;
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

				char* file_contents = getFileContents(entpath);
				TokenizerT* tk = TKCreate( file_contents );
				char* token = TKGetNextToken( tk );
				while( token != NULL )
				{
					printf("%s\n", token);
					token = TKGetNextToken( tk );
				}
			}

		}
		
		closedir(dir);
	}
	/*else
	{
		printf( " %s\n", basedir );
		char* file_contents = getFileContents( basedir );
																		if( file_contents == NULL )
																		{
																			//TODO handle error
																		}
				TokenizerT* tk = TKCreate( file_contents );
				char* token = TKGetNextToken( tk );
				while( token != NULL )
				{
					printf("%s\n", token);
					token = TKGetNextToken( tk );
				}
	} */
}




int main (int argc, char** argv)
{
	walkDir(argv[1]);
	return 0;
}
