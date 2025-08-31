#ifndef _AR_PRIVATE_H_
#define _AR_PRIVATE_H_

typedef struct
{
    boolean is_playing;
    boolean is_loading_checkpoint;
    int coins;
    int lives;
} ar_state_t;

extern ar_state_t ar_state;

#endif
