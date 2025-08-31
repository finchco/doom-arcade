#include "../include/arcade.h"

#include "ar_fonts.h"
#include "ar_private.h"
#include "doomkeys.h"
#include "i_system.h"
#include "m_controls.h"
#include "sc_score.h"

#include "../../src/i_swap.h"
#include "../../src/i_video.h"
#include "../../src/m_misc.h"
#include "../../src/v_video.h"
#include "../../src/w_wad.h"
#include "../../src/z_zone.h"
#include "../../src/doom/d_main.h"
#include "../../src/doom/doomstat.h"
#include "../../src/doom/g_game.h"
#include "../../src/doom/p_saveg.h"
#include "doom/s_sound.h"

#include <SDL_assert.h>

ar_state_t ar_state;

extern void M_ClearMenus(void);

void AR_Init(void)
{
    SC_Init();
    AR_InitFonts();
}

void AR_DrawHud(void)
{
    char s[10] = {0};

    if (!ar_state.is_playing)
    {
        return;
    }

    SDL_itoa(SC_GetCurrentScore(), s, 9);
    AR_DrawStringRightAlign(ARCADE_FONT_BIG, SCREENWIDTH, -4, s);
}

void AR_DrawLeaderboard(void)
{
    sc_record_t records[SC_NUM_RECORDS];
    int y;
    const int startx = 88;
    char buf[16] = {0};

    SC_GetRecords(records);

    y = 40;
    for (int i = 0; i < SC_NUM_RECORDS; ++i)
    {
        AR_DrawString(ARCADE_FONT_BIG, startx, y, records[i].name);

        SDL_itoa(records[i].score, buf, 10);
        AR_DrawStringRightAlign(ARCADE_FONT_BIG, startx + 128, y, buf);
        y += 16;
    }
}

void AR_Drawer(void)
{
    // flash INSERT COIN or PRESS START during demo playback (attract mode)
    if (demoplayback)
    {
        AR_DrawLeaderboard();

        if (ar_state.coins == 0)
        {
            if ((I_GetTime() & 16) == 0)
            {
                V_DrawPatchDirect(80, 8, W_CacheLumpName("INCOIN", PU_CACHE));
            }
        }
        else
        {
            if ((I_GetTime() & 8) == 0)
            {
                V_DrawPatchDirect(80, 8, W_CacheLumpName("PRSTART", PU_CACHE));
            }
        }
    }
    else if (gamestate == GS_LEVEL && !automapactive)
    {
        AR_DrawHud();
    }
}

boolean AR_Responder(event_t *ev)
{
    if ((gamestate != GS_LEVEL || demoplayback) && ev->type == ev_keydown)
    {
        int key = ev->data1;

        if (key == key_menu_activate)
        {
            I_Quit();
        }
        else if (key == KEYP_ENTER && ar_state.coins > 0)
        {
            // pressing enter starts game on hard
            ar_state.lives = ar_state.coins * 3;
            ar_state.coins = 0;
            G_DeferedInitNew(sk_hard, 1, 1);
            M_ClearMenus();
            return true;
        }
        else if (key == '\\' && ar_state.coins > 0)
        {
            // pressing backslash starts game on nightmare
            ar_state.lives = ar_state.coins * 3;
            ar_state.coins = 0;
            G_DeferedInitNew(sk_nightmare, 1, 1);
            M_ClearMenus();
            return true;
        }
        else if (key == 'q')
        {
            ++ar_state.coins;
            S_StartSound(NULL, sfx_brssit);
            return true;
        }
    }

    return false;
}

void AR_SaveCheckpoint()
{
    if (!ar_state.is_playing)
    {
        return;
    }

    SDL_assert(gameaction == ga_nothing);
    G_QuickSaveImmediate("arcade");
    SC_SaveCheckpoint();
}

void AR_LoadCheckpoint()
{
    if (!ar_state.is_playing)
    {
        return;
    }

    SDL_assert(gameaction == ga_nothing);
    G_LoadGame(P_SaveGameFile(0));
    SC_LoadCheckpoint();
    ar_state.is_loading_checkpoint = true;
}

void AR_OnNewGameBegin(boolean is_nightmare)
{
    SDL_assert(!ar_state.is_playing);
    ar_state.is_playing = true;
    AR_SaveCheckpoint();
    SC_BeginNewRecord(is_nightmare);
}

void AR_OnLevelLoaded(void)
{
    if (!ar_state.is_playing)
    {
        return;
    }

    if (ar_state.is_loading_checkpoint)
    {
        ar_state.is_loading_checkpoint = false;
    }
    else
    {
        AR_SaveCheckpoint();
    }
}

void AR_Respawn(void)
{
    if (--ar_state.lives == 0)
    {
        M_ClearMenus();
        D_StartTitle();
    }
    else
    {
        AR_LoadCheckpoint();
    }
}

void AR_OnGameOver(void)
{
    ar_state.is_playing = false;
}
