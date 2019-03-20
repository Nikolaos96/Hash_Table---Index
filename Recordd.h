#ifndef __REC__
#define __REC__

typedef struct {
    int id;
    char name[15];
    char surname[20];
    char address[40];
} Record;


typedef struct {
    Record record;
    int blockId;
} SecondaryRecord;

#endif
