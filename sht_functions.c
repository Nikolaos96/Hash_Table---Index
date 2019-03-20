#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "sht_functions.h"

uint hashString(char* x, int p)
{ // universal hash function for string
	uint h = 12,a=rand();
        a=3243;
	for (uint i=0 ; i < strlen(x) ; ++i)
		h = ((h*a) + x[i]) % p;
	return h%p;
}


int SHT_CreateSecondaryIndex ( char *sfileName,
                     char *attrName,
                     int attrLength,
                     int buckets,
		     char *Primary_file
)
{

    /*    Create BF file    */
    if ( BF_CreateFile(sfileName) < 0 ) {
        BF_PrintError("Error on BF_CreateFile");
        return -1;
    }
    /*    Open BF file and allocate first block to write usefull info    */
    int fd = BF_OpenFile(sfileName);
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
    SHT_info header;
    header.fileDesc = fd;
    header.attrLength = attrLength;
    strcpy(header.attrName,attrName);
    strcpy(header.fileName,Primary_file);
    header.numBuckets = buckets;
    memcpy(block,"SHT",4);
    memcpy(block+4,&header,sizeof(SHT_info));
    if ( BF_WriteBlock(fd,BF_GetBlockCounter(fd)-1) < 0 ) {
        BF_PrintError("Error on Write");
        return -1;
    }

    Block_s bl;
    bl.number_records = 0;	// arxikopoiw to block - arxika exei 0 egrafes
    bl.next_block = -1;		// epeidi den exei epomeno exw valei -1

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
        memcpy(block,&bl,sizeof(Block_s));	////////////////////
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


SHT_info * SHT_OpenSecondaryIndex( char *sfileName ){

    SHT_info *header = malloc(sizeof(SHT_info));
    int fd = BF_OpenFile(sfileName);
    if ( fd < 0 ) {
        BF_PrintError("Error on Open");
        return NULL;
    }
    void *block;
    if ( BF_ReadBlock(fd,0,&block) < 0){
        BF_PrintError("Error on Read");
        return NULL;
    }
    char str[4];
    memcpy(str,block,4);
    if (strcmp(str,"SHT")) {
        BF_CloseFile(fd);
        return NULL;
    }

    memcpy(header,block+4,sizeof(SHT_info));
    header->fileDesc = fd;

    return header;
}


int SHT_CloseSecondaryIndex( SHT_info * header_info ){

    if ( BF_CloseFile(header_info->fileDesc) < 0 ){
        BF_PrintError("Error on Close");
        return -1;
    }
    free(header_info);
    return 0;
}


int SHT_SecondaryInsertEntry ( SHT_info header_info, SecondaryRecord rec)
{
    uint bucket;

    int fd = header_info.fileDesc;
    Block_s *block;
    Sec_record s_rec;
    s_rec.block_id = rec.blockId;
    s_rec.id = rec.record.id;

    /* Briskw se poio bucket paei i eggrafi, to +1 epeidi gia to bucket 0 einai to block 1 klp  */

    if( !strcmp(header_info.attrName, "name") ){
        bucket = hashString( rec.record.name, header_info.numBuckets ) + 1;
        strcpy(s_rec.NamSurrAdd,rec.record.name);
    }else if( !strcmp(header_info.attrName, "surname") ){
	bucket = hashString( rec.record.surname, header_info.numBuckets ) + 1;
        strcpy(s_rec.NamSurrAdd,rec.record.surname);
    }else{
	bucket = hashString( rec.record.address, header_info.numBuckets ) + 1;
	strcpy(s_rec.NamSurrAdd,rec.record.address);
    }

    while(1) {
        if ( BF_ReadBlock(fd, bucket,(void**) &block) < 0) {
            BF_PrintError("Error====== on Read");
            return -1;
        }
        /* An yparxei xwros grapsto */
        if (block->number_records < 10) {
            memcpy(&(block->Records_in_Block[block->number_records]),&s_rec,sizeof(Sec_record));//////
            block->number_records++;
            if ( BF_WriteBlock(fd,bucket) < 0 ) {
                BF_PrintError("Error on Write");
                return -1;
            }
            return bucket;
        }
        else {
            /* An yparxei epomeno block pigaine se auto */
            if (block->next_block > 0) {
                bucket = block->next_block;
                continue;
            }
            /* Alliws allocate kainourio */
            else {
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
        }
    }
    return -1;
}

int getRec (int fd,int block_id, int id)
{
    Block *b;
    Record rec;
    if ( BF_ReadBlock(fd, block_id,(void**) &b) < 0) {
            BF_PrintError("Error on Read");
            return -1;
    }
    for (int i=0 ; i < b->number_records ; i++) {
        memcpy(&rec,&b->Records_in_Block[i],sizeof(Record));
        if ( rec.id == id ) {
                printf("{%d,%s,%s,%s}\n",rec.id,rec.name,rec.surname,rec.address);
                return 0;
        }
    }
}


int SHT_SecondaryGetAllEntries ( SHT_info header_info_sht, HT_info header_info_ht, void *value)
{
    int k;
    if( !strcmp(header_info_sht.attrName, "name") ){
	    k = 1;
    }else if( !strcmp(header_info_sht.attrName, "surname") ){
	    k = 2;
    }else{
	    k = 3;
    }
    int fd = header_info_sht.fileDesc;  // fd tou BF arxeio
    int bucket = hashString((char *)value, header_info_sht.numBuckets) + 1;    // prwto block tou bucket pou anikei i eggrafi
    Block_s *block;
    Sec_record rec;
    int num_read_blocks = 0,stop = 0;

        while(1) {
            // diabasma block
            if ( BF_ReadBlock(fd, bucket,(void**) &block) < 0) {
                BF_PrintError("Error on Read");
                return -1;
            }
            for (int i=0 ; i < block->number_records ; i++) {
                memcpy(&rec,&block->Records_in_Block[i],sizeof(Sec_record));
                if ( (k==1 && !strcmp(rec.NamSurrAdd,(char*)value))  || \
                     (k==2 && !strcmp(rec.NamSurrAdd,(char*)value)) || \
                     (k==3 && !strcmp(rec.NamSurrAdd,(char*)value)) ) {
                    getRec(header_info_ht.fileDesc,rec.block_id,rec.id);
                    if(stop == 0){
                        num_read_blocks++;
                        stop = 1;
                    }
                }
            }
	        stop = 0;
            // An den bre8ike proxwra sto epomeno block tou bucket an yparxei
            if (block->next_block > 0)
                bucket = block->next_block;
            else break;
        }
    return num_read_blocks + 1;
}

void stat(HT_info* header)
{

    int num_blocks;
    num_blocks = BF_GetBlockCounter(header->fileDesc);
    int min=100000000,max=-1,bk[header->numBuckets],total=0;
    double mean=0,meanR=0;
    Block *b;
    for (int i=0;i<header->numBuckets;i++) {
        int tmp=1,tempR=0;
        BF_ReadBlock(header->fileDesc,i+1,(void**)&b);
        tempR += b->number_records;
        while(b->next_block != -1) {
            BF_ReadBlock(header->fileDesc,b->next_block,(void**)&b);
            tempR += b->number_records;
            tmp++;
        }
        bk[i] = tmp-1;
        if (tmp > 1) total++;
        if (tempR < min) min = tempR;
        if (tempR > max) max = tempR;
        mean += tmp;
        meanR += tempR;
    }
    mean /= (double) header->numBuckets;
    meanR /= (double) header->numBuckets;
    printf("Arithos block arxeiou: %d\n",num_blocks);
    printf("min: %d, max: %d, mean: %.2f\n",min,max,meanR);
    printf("Meso pli8os block: %.2f\n",mean);
    printf("Buckets me blocks yperxeilisis: %d\n",total);
    for (int i=0 ; i < header->numBuckets;i++) {
        if (bk[i]>=1)
            printf("Bucket %d: %d blocks yperxeilisi\n",i,bk[i]);
    }
}
void stat2(SHT_info* header)
{
    printf("========= SHT =========\n");
    int num_blocks;
    num_blocks = BF_GetBlockCounter(header->fileDesc);
    int min=100000000,max=-1,bk[header->numBuckets],total=0;
    double mean=0,meanR=0;
    Block *b;
    for (int i=0;i<header->numBuckets;i++) {
        int tmp=1,tempR=0;
        BF_ReadBlock(header->fileDesc,i+1,(void**)&b);
        tempR += b->number_records;
        while(b->next_block != -1) {
            BF_ReadBlock(header->fileDesc,b->next_block,(void**)&b);
            tempR += b->number_records;
            tmp++;
        }
        bk[i] = tmp-1;
        if (tmp > 1) total++;
        if (tempR < min) min = tempR;
        if (tempR > max) max = tempR;
        mean += tmp;
        meanR += tempR;
    }
    mean /= (double) header->numBuckets;
    meanR /= (double) header->numBuckets;
    printf("Arithos block arxeiou: %d\n",num_blocks);
    printf("min: %d, max: %d, mean: %.2f\n",min,max,meanR);
    printf("Meso pli8os block: %.2f\n",mean);
    printf("Buckets me blocks yperxeilisis: %d\n",total);
    for (int i=0 ; i < header->numBuckets;i++) {
        if (bk[i]>=1)
            printf("Bucket %d: %d blocks yperxeilisi\n",i,bk[i]);
    }
}

int HashStatistics ( char *filename)
{
    HT_info* header = HT_OpenIndex(filename);
    SHT_info* sh_header = SHT_OpenSecondaryIndex(filename);

    if (header != NULL){
        stat(header);
        HT_CloseIndex(header);
    }
    else if (sh_header != NULL){
        stat2( sh_header) ;
        SHT_CloseSecondaryIndex(sh_header);
    }
    return 0;
}
