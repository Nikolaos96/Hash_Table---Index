/* File: main.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "HT.h"
#include "BF.h"
#include "sht_functions.h"
#include "Recordd.h"

#define NUM_BUCKET 10

int main(int argc, char const *argv[])
{
    BF_Init();

    if (HT_CreateIndex("Id_index",'c',"id",10,NUM_BUCKET) < 0 ){
	printf("Error on create primary inedx at id. \n");
	return -1;
    }
    HT_info* header2 = HT_OpenIndex("Id_index");
    if( header2 == NULL ){
	printf("Error open Primary index at id. \n");
	return -1;
    }


    if( SHT_CreateSecondaryIndex("PIndex_address","address",15,NUM_BUCKET,"Id_index") < 0 ){
	printf("Error on crate secondary index at address. \n");
	return -1;
    }
    SHT_info* sht = SHT_OpenSecondaryIndex("PIndex_address");
    if (sht == NULL) {
	printf("Error open secondary index at address. \n");
	return -1;
    }

    char str[30];
    Record rec;
    SecondaryRecord sr;

    FILE *f;
    char *strr=NULL;
	int i = 5,a;
	size_t len=0;
    if ((f = fopen("records5K.txt","r")) == NULL){
        printf("file doesnt exist?!\n");
        return -11;
    }
    while(!feof(f)){
        strr = NULL;
        len=0;
        if(fgetc(f)==EOF) break;
        fgetc(f);
		if(getdelim(&strr,&len,',',f) == -1) break;
        strr[strlen(strr)-1] ='\0';
		a = atoi(strr);
        rec.id = a;
		for(int j=0;j<3;j++){
			strr = NULL;
			fgetc(f);
			getdelim(&strr,&len,'"',f);
			strr[strlen(strr)-1] ='\0';
            if(j==0) strcpy(rec.name,strr);
            else if(j==1) strcpy((rec.surname),strr);
            else strcpy(rec.address,strr);
			if(fgetc(f)==EOF) break;
		}
        //printf("id: %d name: %s surname: %s address: %s\n",rec.id,rec.name,rec.surname,rec.address);
        memcpy(&sr.record,&rec,sizeof(Record));
        sr.blockId=HT_InsertEntry(*header2,rec);// Insert record in primary index at id
        SHT_SecondaryInsertEntry(*sht,sr);	// Insert record in secondary index at address
    }

    fclose(f);

    printf("\n			Search records based id :   \n\n");
    int mm;
    int ii = 60;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);
    ii = 61;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);
    ii = 62;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);
    ii = 63;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);
    ii = 64;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);

    printf("\n");
    printf("			Search records based address :    \n\n");
    if( (mm = SHT_SecondaryGetAllEntries(*sht,*header2,"address_60")) >= 0) printf("Were read %d blocks \n", mm);

    printf("\n");
    if( (mm = SHT_SecondaryGetAllEntries(*sht,*header2,"address_62")) >= 0) printf("Were read %d blocks \n", mm);

    printf("			Search record based id before and after delete :     \n\n");
    ii = 60;
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);

    HT_DeleteEntry(*header2,(void*)&ii);
    if( (mm = HT_GetAllEntries(*header2,(void*)&ii)) > 0) printf("Were read %d blocks \n", mm);
    else printf("Record does not exist. \n");

    if ( HT_CloseIndex(header2) < 0 ) printf("ERR close");
    if ( SHT_CloseSecondaryIndex(sht) < 0 ) printf("ERR close");


    printf("\n			HashStatistics		\n\n");
    HashStatistics("Id_index");
    printf("\n");
    HashStatistics("PIndex_address");

    return 0;
}
