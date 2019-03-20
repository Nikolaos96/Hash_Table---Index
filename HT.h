#ifndef __HT__
#define __HT__
#include "Recordd.h"

typedef unsigned int uint;

typedef struct {
    int fileDesc;
    char attrType;
    char attrName[20];
    int attrLength;
    long int numBuckets;
} HT_info;


typedef struct {
    Record Records_in_Block[6]; // kathe block mporei na exei mexri 6 egrafes
    int number_records;		// poses egrefes exei mesa to block (apo 0 mexri 6)
    int next_block;		// arithmos epomenou block
} Block;


uint hashInt(int x, int numBuckets);

int HT_CreateIndex ( char *fileName,
                     char attrType,
                     char *attrName,
                     int attrLength,
                     int buckets ) ;

HT_info * HT_OpenIndex( char *fileName );

int HT_CloseIndex( HT_info * header_info );

int HT_InsertEntry ( HT_info header_info, Record record);

int HT_DeleteEntry ( HT_info header_info, void *value);

int HT_GetAllEntries ( HT_info header_info, void *value);

uint hashInt ( int x, int numBuckets );

uint hashString ( char* x, int numBuckets );

#endif
