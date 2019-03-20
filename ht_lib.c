#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "HT.h"


uint hashInt ( int x, int numBuckets )
{   // universal hash function for int
    uint a = rand();
    uint b = rand();
    a = 123;
    b = 432;

	uint p = 179426549;
	return ( (a*x + b) % p ) % numBuckets;
}


int HT_CreateIndex ( char *fileName,
		char attrType,
		char *attrName,
		int attrLength,
		int buckets
		)
{

    /*    Create BF file    */
    if ( BF_CreateFile(fileName) < 0 ) {
        BF_PrintError("Error on BF_CreateFile");
        return -1;
    }
    /*    Open BF file and allocate first block to write usefull info    */
    int fd = BF_OpenFile(fileName);
    if ( fd < 0 ) {
        BF_PrintError("Error on BF_Openfile");
        return -1;
    }
    if ( BF_AllocateBlock(fd) < 0 ) {
        BF_PrintError("Error on BF_AllocateBlock");
        return -1;
    }

    /*    Read block and store block data at block variable    */
    void *block;
    if ( BF_ReadBlock(fd,BF_GetBlockCounter(fd)-1,&block) < 0) {
        BF_PrintError("Error on Read");
        return -1;
    }
    /* Create an HT_info struct to store info and write it to the file */
    HT_info header;
    header.fileDesc = fd;
    header.attrLength = attrLength;
    strcpy(header.attrName,attrName);
    header.attrType = attrType;
    header.numBuckets = buckets;
    memcpy(block,"HT",3);
    memcpy(block+3,&header,sizeof(HT_info));
    if ( BF_WriteBlock(fd,BF_GetBlockCounter(fd)-1) < 0 ) {
        BF_PrintError("Error on Write");
        return -1;
    }

    Block bl;
    bl.number_records = 0;
    bl.next_block = -1;

    // Dimiourgise ta arxika block osa kai ta bucket
    // Arxika einai adeia.
    //To prwto block kathe buckt einai o arithmos tou ucket + 1
    int i;
    for(i = 0 ; i < buckets ; i++){
        if ( BF_AllocateBlock(fd) < 0 ) {
            BF_PrintError("Error on BF_AllocateBlock - bucket block");
            return -1;
        }
        if ( BF_ReadBlock(fd,BF_GetBlockCounter(fd)-1,&block) < 0) {
            BF_PrintError("Error on Read - bucket block");
            return -1;
        }
        memcpy(block,&bl,sizeof(Block));
        if ( BF_WriteBlock(fd,BF_GetBlockCounter(fd)-1) < 0 ) {
            BF_PrintError("Error on Write - buctet block");
            return -1;
        }
    }

    if ( BF_CloseFile(fd) < 0 ) {
        BF_PrintError("Error on CLose");
        return -1;
    }
    return 0;
}


HT_info * HT_OpenIndex( char *fileName ){

	HT_info *header = malloc(sizeof(HT_info));
	int fd = BF_OpenFile(fileName);
	if ( fd < 0 ) {
		BF_PrintError("Error on Open");
		return NULL;
	}
	void *block;
	if ( BF_ReadBlock(fd,0,&block) < 0){
		BF_PrintError("Error on Read");
		return NULL;
	}
	char str[3];
	memcpy(str,block,3);
	if (strcmp(str,"HT")) {
		BF_CloseFile(fd);
		return NULL;
	}
	memcpy(header,block+3,sizeof(HT_info));
	header->fileDesc = fd;
	return header;
}


int HT_CloseIndex( HT_info * header_info ){

	if ( BF_CloseFile(header_info->fileDesc) < 0 ){
		BF_PrintError("Error on Close");
		return -1;
	}
	free(header_info);
	return 0;
}

int HT_InsertEntry ( HT_info header_info, Record record)
{
	/* Briskw se poio bucket paei i eggrafi, to +1 epeidi gia to bucket 0 einai to block 1 klp  */
	uint bucket = hashInt( record.id, header_info.numBuckets ) + 1;
	int fd = header_info.fileDesc;
	Block *block;
	if ( BF_ReadBlock(fd, bucket,(void**) &block) < 0) {
		BF_PrintError("Error on Read");
		return -1;
	}
	while(block->next_block!= -1) {
		bucket = block->next_block;
		if ( BF_ReadBlock(fd, block->next_block,(void**) &block) < 0) {
			BF_PrintError("Error on Read");
			return -1;
		}
	}
	if (block->number_records == 6) {
		if ( BF_AllocateBlock(fd) < 0 ) {
			BF_PrintError("Error on BF_AllocateBlock - insert new block");
			return -1;
		}
		block->next_block = BF_GetBlockCounter(fd)-1;
		/* Sto trexon block allakse to next_block apo -1 se BF_GetBlockCounter(fd)-1 ara prepei na to grapsw */
		if ( BF_WriteBlock(fd,bucket) < 0 ) {
			BF_PrintError("Error on Write - insert write old block after new");
			return -1;
		}
		/* meta koita to epomeno block, poy molis kaname allocate */
		bucket = block->next_block;

		/* Arxikopoihisi tou kainouriou block me next_block=-1 kai number_records = 0 */
		/* Stin epomeni epanalipsi 8a brei xwro na graftei i eggrafi sto kainourio block */
		if ( BF_ReadBlock(fd,bucket,(void**) &block) < 0) {
			BF_PrintError("Error on Read - insert new block");
			return -1;
		}
		block->next_block = -1;
		block->number_records = 0;
		if ( BF_WriteBlock(fd,bucket) < 0 ) {
			BF_PrintError("Error on Write - insert write old block after new");
			return -1;
		}
	}
	if ( BF_ReadBlock(fd,bucket,(void**) &block) < 0) {
		BF_PrintError("Error on Read - insert new block");
		return -1;
	}

	/* An yparxei xwros grapsto */
	memcpy(&(block->Records_in_Block[block->number_records]),&record,sizeof(Record));
	block->number_records++;
	if ( BF_WriteBlock(fd,bucket) < 0 ) {
		BF_PrintError("Error on Write");
		return -1;
	}
	//break;
	return bucket;
}


int HT_DeleteEntry ( HT_info header_info, void *value){
	int key = *(int*)value;
	uint bucket = hashInt(key, header_info.numBuckets) + 1;
	int fd = header_info.fileDesc;

	Block* block;
	int stop = 0;
	while(1){
		if ( BF_ReadBlock(fd, bucket,(void**) &block) < 0) {
			BF_PrintError("Error on Read");
			return -1;
		}
		for(int i = 0 ; i < block->number_records ; i++){ // Gia kathe mia apo tis 6 eggrafes koita an yparxei kapoia me ayto to id (key == id)
			if( key == block->Records_in_Block[i].id ){ // h eggrafi yparxei
				// Antikatestise tin eggrafi pou thes na diagrapseis me thn teleutaia tou block
				block->Records_in_Block[i] = block->Records_in_Block[block->number_records - 1];
				// midenise tin teleutaia eggrafi tou block efoson tin exeis metaferei ekei pou esvises
				Record rec; // ========================= maybe memset 0 here ========================= //
				block->Records_in_Block[block->number_records - 1] = rec;
				block->number_records--;

				if ( BF_WriteBlock(fd,bucket) < 0 ) {
					BF_PrintError("Error on Write - insert write old block after new");
					return -1;
				}

				stop = 1;
				break;
			}
		}
		if(stop == 1) break;
		// Den vrethike sto block phgaine sto epomeno
		if (block->next_block > 0) bucket = block->next_block;
		else return 0;	// Den exei epomeno block ara h egrafi den yparxei
	}

	return 0;
}



int HT_GetAllEntries ( HT_info header_info, void *value){
    int id = * (int *) value;       // id gia anazitisi
    int fd = header_info.fileDesc;  // fd tou BF arxeio
    int bucket = hashInt(id,header_info.numBuckets) + 1;    // prwto block tou bucket pou anikei i eggrafi
    Block *block;
    Record rec;
    int num_blocks = 0;
    while(1) {
        // diabasma block
        if ( BF_ReadBlock(fd, bucket,(void**) &block) < 0) {
            BF_PrintError("Error on Read");
            return -1;
        }
	num_blocks++;
        // Gia ka8e eggrafi elegxos id
        for (int i=0 ; i < block->number_records ; i++) {
            memcpy(&rec,&block->Records_in_Block[i],sizeof(Record));
            if ( rec.id == id ) {
                printf("{%d,%s,%s,%s}\n",rec.id,rec.name,rec.surname,rec.address);
                return num_blocks + 1;
            }
        }
        // An den bre8ike proxwra sto epomeno block tou bucket an yparxei
        if (block->next_block > 0) bucket = block->next_block;
        else break;
    }
    return -1;
}
