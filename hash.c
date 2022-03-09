#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities.h"

#include "hash.h"


struct Hash_t{
	int size;
	char** keys;
	char** values;
};


//private methods
bool getTextd(char , char , char* , char* , unsigned int);



Hash_t* hash_new(int size)
{
	int i;
	Hash_t* H;
	H = malloc(sizeof(Hash_t));
	H->size=size;
	H->keys=NULL;
	H->values=NULL;
	
	if(size>0)
	{
		H->keys =   (char**)calloc(size, sizeof(char*));
		H->values = (char**)calloc(size,sizeof(char*));

		for(i=0;i<size;i++)
		{
			H->keys[i] = (char*)calloc(MAX_KEY_SIZE,sizeof(char));
			H->values[i] = (char*)calloc(MAX_VALUE_SIZE,sizeof(char));	
		}		
	}
		
	return H;
}

void hash_delete(Hash_t* H)
{
	int i,L;
	L = H->size;
	
	for(i=0;i<L;i++)
	{
		free(H->keys[i]);
		free(H->values[i]);
	}

	if(L>0)
	{
		free(H->keys);
		free(H->values);
	}

	//delete hasht object itself
	free(H);

}

void hash_insert(Hash_t* H, char* key, char* val)
{
	int i;
	
	//Check that key val are correct size
	if(! (strlen(key)<MAX_KEY_SIZE  && strlen(val) < MAX_VALUE_SIZE) )
	{
		
		errorMsg("hash_insert\n key/value exceed max size!");
		printf("key %s \t\t value %s \n",key,val);
		return;
	}
	
	
	//overwrite if existing
	for(i=0;i<H->size;i++)
	{
		if(strcmp(H->keys[i],key)==0)
		{	strcpy(H->values[i],val);
			return;
		}

	}

	//write into space if free	
	for(i=0;i<H->size;i++)
	{
		if(!(H->keys[i][0]))
		{
			strcpy(H->keys[i],key);	
			strcpy(H->values[i],val);
			
			return;
		}
	}
	
	
	warningMsg("Table full! Key/Value NOT allocated!");
	printf("key:%s \t\t value:%s \n",key,val);	
	
}

char* hash_lookup(Hash_t* H, char* key)
{
	int i=0;	
	for(i=0;i<H->size;i++)
	{	
		if(strcmp(H->keys[i],key)==0)
			return H->values[i];	
	}
	
	return NULL;	
}

void hash_print(Hash_t* H)
{
	int i;
	
	for(i=0;i<H->size;i++)
	{
		if((H->keys[i][0]))//if Key exists
		printf("\nkey: %s\t\t value: %s",H->keys[i],H->values[i]);
	}
	printf("\n");
}



bool checkParameterList(Hash_t* H, char** str, int L)
{
	int i;
	char* s = NULL;
	for(i=0; i<L; i++)
	{
	   	s = hash_lookup(H,str[i]); 
		if(s==NULL)
		return false;
	}

	return true;
}



bool readConfigurationFile(char* filename, Hash_t* H)
{
	FILE* fp;	
	int LINE_SIZE = MAX_KEY_SIZE+MAX_VALUE_SIZE;
	char line[LINE_SIZE];

	char key[MAX_KEY_SIZE];	
	char value[MAX_VALUE_SIZE];

	unsigned long n;
	unsigned int line_number=0;

	fp = fopen(filename, "r");
	if (!(fp)) { errorMsg("Could not open config file!"); return false;  }

	//reads a maximum of LINE_SIZE
	while (fgets(line, LINE_SIZE, fp) != NULL) //NULL is end of line or eof. Uses null character terminator  
	{
		line_number++;
		//line includes \n character  at end so replace it with NULL
		n = strnlen(line,LINE_SIZE);
		line[n-1]='\0';
		
		//Note empty lines (i.e. LF=10,CR=13) will now be null

		//Ignore comments, newlines->which have been converted to NULL
		if(line[0]!='#'  && line[0]!='\0' ){ 
			
			//Check for a key and value (anything else equals syntax error!)
			if(getTextd(DELIM1,DELIM2, line, key,0)){
				if(getTextd(DELIM2,'\0',line, value,0))
				{
					//if key/value are non-null - insert
					if(key[0]!='\0' && value[0]!='\0')
						hash_insert(H, key, value);		
					else{
						warningMsg("readConfig - empty values!");
						printf("key: %s \tor\t value:%s \n",key,value);
					}
						
			
				}
			}else{//improper key/value line detected
				
				errorMsg("Failed to read config: key/value syntax/size limit error at:");
				printf("\tline %d: %s \n",line_number,line);
				warningMsg("check LF/CR ? i.e. blank lines should have nothing but LF (i.e. no empty spaces)");
				printf("\t Also note: max KEY size:%d, max VAlUE size:%d",MAX_KEY_SIZE,MAX_VALUE_SIZE);
				fclose(fp);
				return false;
			}  
	
		}
	}
	
	fclose(fp);
	return true;
}


//Search string for deliminators and get key 
bool getTextd(char delim1, char delim2, char* line, char* out, unsigned int skip)
{
	unsigned int n=0;
	unsigned int i=0;
	bool copy = false;
	bool success = false;
	
	unsigned int MX = MAX_VALUE_SIZE; //looking for a value
	if(delim1 == DELIM1)  //looking for a key
	MX = MAX_KEY_SIZE;
	
	
	if(delim1 == '\0')  //start copying right away!
	copy=true;
	
	if(line){
	while(line[n]!='\0' && !success )
	{
		if(copy && line[n]!=32 && line[n]!=9) //ascii code for spaces/tabs (ignore spaces!)
		{
			out[i] = line[n];
			i++;
			
			if(i==MX)
			{  //buffer is now full
				out[i-1]='\0'; //overwrite last character to end string
				warningMsg("(getTextd): max key/value size reached!");
				printf("%s\n",out);
				return false;
				
			}
					
		}				

		if(line[n]==delim1)
		{
			if(!skip)
			copy=true;
			else
			skip--;
		}	
		
		n++;
		
		if(line[n]==delim2 && copy)
		{copy=false; success =true;}	
		
	}}  //line == null
	
	out[i]='\0';
	
		
	return success; //i.e. success

}


bool hash_lookup_csv(Hash_t* H, char* key, unsigned int n, char* out)
{
	int i=0;	
	char* csv;
	
	csv = NULL;
	
	for(i=0;i<H->size;i++)
	{	
		if(strcmp(H->keys[i],key)==0)
			csv =  H->values[i];	
	}
	
	if(csv)
	{
		if(n==0)
		{
			if ( !getTextd('\0',',',csv,out,0) ){
				if( !getTextd('\0','\0',csv,out,0) )
				return false;
			}	
				
				return true;
			
			
		}else{
			
			if ( !getTextd(',',',',csv,out,n-1) ){
				if( !getTextd(',','\0',csv,out,n-1) )
				return false;
			}	
				return true;
			
		}
		
	}
	

	return false;		
}


