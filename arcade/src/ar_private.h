#ifndef _AR_PRIVATE_H_
#define _AR_PRIVATE_H_

enum ar_state_e
{
    ARS_ATTRACT,
    ARS_START_GAME,
    ARS_PLAY,
    ARS_DEAD,
    ARS_BEGIN_LOAD_CHECKPOINT,
    ARS_END_LOAD_CHECKPOINT,
    ARS_ENTER_NAME,
    ARS_REBOOT,
    ARS_COUNT
};

struct ar_arcade_s
{
    enum ar_state_e state;
    boolean isnightmare;
    int coins;
    int lives;
    int timer;
};

extern struct ar_arcade_s arcade;

#endif
