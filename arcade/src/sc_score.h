#ifndef _SC_SCORE_
#define _SC_SCORE_

#include "../../src/doomtype.h"

#define SC_NUM_RECORDS  8
#define SC_NAME_LEN 3

typedef struct
{
    char name[SC_NAME_LEN + 1];
    int score;
    int duration_sec;
} sc_record_t;


void SC_Init(void);
void SC_BeginNewRecord(boolean is_nightmare);
void SC_FinalizeRecord(sc_record_t* newrecord);

int SC_GetCurrentRank(void);
int SC_GetCurrentScore(void);
void SC_GetRecords(sc_record_t out[SC_NUM_RECORDS]);

void SC_SaveCheckpoint(void);
void SC_LoadCheckpoint(void);


#endif
