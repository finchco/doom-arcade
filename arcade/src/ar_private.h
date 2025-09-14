#ifndef _AR_PRIVATE_H_
#define _AR_PRIVATE_H_
#include "sc_score.h"

#include <SDL_stdinc.h>

enum ar_state_e
{
    ARS_ATTRACT,
    ARS_BEGIN_PLAY,
    ARS_PLAY,
    ARS_DEAD,
    ARS_BEGIN_LOAD_CHECKPOINT,
    ARS_END_LOAD_CHECKPOINT,
    ARS_BEGIN_ENTER_NAME,
    ARS_ENTER_NAME,
    ARS_END_ENTER_NAME,
    ARS_REBOOT,
    ARS_COUNT
};

struct ar_arcade_s
{
    enum ar_state_e state;
    boolean isnightmare;
    Uint64 game_start_time;
    int coins;
    int lives;
    int timer;
    int counter;
    sc_record_t record;
};

extern struct ar_arcade_s arcade;

#endif
