#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_KEY_SIZE 80
#define MAX_VALUE_SIZE 280
#define DELIM1 '$'
#define DELIM2 '='

typedef struct Hash_t Hash_t;

Hash_t* hash_new(int);
void hash_delete(Hash_t*);
void hash_insert(Hash_t*, char* , char* );
char* hash_lookup(Hash_t* , char* );
bool hash_lookup_csv(Hash_t* , char* , unsigned int , char* );

void hash_print(Hash_t*);
bool checkParameterList(Hash_t* , char** , int );

bool readConfigurationFile(char* , Hash_t*);


#endif
