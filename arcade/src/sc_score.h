#ifndef _SC_SCORE_
#define _SC_SCORE_

#include "../../src/doomtype.h"

#define SC_NUM_RECORDS 8
#define SC_MAX_NAME_LEN 3

typedef struct
{
    char name[SC_MAX_NAME_LEN + 1];
    int score;
    int duration_sec;
} sc_record_t;

void SC_Init(void);
void SC_BeginNewRecord(boolean is_nightmare);
int SC_FinalizeRecord(char *player_name); // return leaderboard spot or -1

int SC_GetCurrentScore(void);
void SC_GetRecords(sc_record_t out[SC_NUM_RECORDS]);

void SC_OnSaveCheckpoint(void);
void SC_OnLoadCheckpoint(void);


#endif
