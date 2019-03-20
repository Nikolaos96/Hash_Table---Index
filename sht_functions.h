#include "Recordd.h"
#include "HT.h"

typedef unsigned int uint;

typedef struct {
    int fileDesc;
    char attrName[20];
    int attrLength;
    long int numBuckets;
    char fileName[128];
} SHT_info;

typedef struct {
    int id;
    int block_id;
    char NamSurrAdd[40];
} Sec_record;

typedef struct {
    Sec_record Records_in_Block[10];
    int number_records;
    int next_block;
} Block_s;

uint hashString(char* x, int p);

int SHT_CreateSecondaryIndex ( char *sfileName,
                     	       char *attrName,
                     	       int attrLength,
                     	       int buckets,
			       char *Primary_file );

SHT_info * SHT_OpenSecondaryIndex( char *sfileName );

int SHT_CloseSecondaryIndex( SHT_info * header_info );

int SHT_SecondaryInsertEntry ( SHT_info header_info, SecondaryRecord rec );

int SHT_SecondaryGetAllEntries ( SHT_info header_info_sht, HT_info header_info_ht, void *value );

int HashStatistics ( char *filename);
