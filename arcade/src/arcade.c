#include "../include/arcade.h"

#include "ar_fonts.h"
#include "ar_private.h"
#include "m_controls.h"
#include "sc_score.h"
#include "../../src/doomkeys.h"
#include "../../src/i_swap.h"
#include "../../src/i_system.h"
#include "../../src/i_video.h"
#include "../../src/m_misc.h"
#include "../../src/v_video.h"
#include "../../src/w_wad.h"
#include "../../src/z_zone.h"
#include "../../src/doom/d_main.h"
#include "../../src/doom/doomstat.h"
#include "../../src/doom/g_game.h"
#include "../../src/doom/p_saveg.h"
#include "../../src/doom/s_sound.h"

#include <SDL_assert.h>
#include <SDL_timer.h>

struct ar_arcade_s arcade;

extern void M_ClearMenus(void);

static void ResetArcadeState(void)
{
    SDL_zerop(&arcade);
    SDL_memcpy(arcade.record.name, "AAA", 3);
}

void AR_Init(void)
{
    ResetArcadeState();
    SC_Init();
    AR_InitFonts();
}

static void Reboot(void)
{
    ResetArcadeState();
    arcade.timer = TICRATE * 5;
    arcade.state = ARS_REBOOT;
}

static void DrawHud(void)
{
    char s[10] = {0};
    SDL_itoa(SC_GetCurrentScore(), s, 9);
    AR_DrawStringRightAlign(ARCADE_FONT_BIG, SCREENWIDTH, -4, s);
}

static void DrawLeaderboard(void)
{
    sc_record_t records[SC_NUM_RECORDS];
    char buf[16] = {0};
    int y = 40;

    SC_GetRecords(records);

    for (int i = 0; i < SC_NUM_RECORDS; ++i)
    {
        const int startx = 80;
        AR_DrawString(ARCADE_FONT_BIG, startx, y, records[i].name);

        SDL_itoa(records[i].score, buf, 10);
        AR_DrawStringRightAlign(ARCADE_FONT_BIG, startx + 160, y, buf);
        y += 16;
    }
}

static void DrawCoinPrompt(void)
{
    if (arcade.coins == 0)
    {
        if (I_GetTime() & 16)
        {
            V_DrawPatchDirect(80, 8, W_CacheLumpName("INCOIN", PU_CACHE));
        }
    }
    else
    {
        if (I_GetTime() & 4)
        {
            V_DrawPatchDirect(80, 8, W_CacheLumpName("PRSTART", PU_CACHE));
        }
    }
}

static void DrawNameEntry(void)
{
    AR_DrawStringCentered(ARCADE_FONT_BIG, 16, "NEW HIGH SCORE!");
    AR_DrawStringCentered(ARCADE_FONT_SMALL, 32, "ENTER YOUR NAME");
    AR_DrawStringCentered(ARCADE_FONT_BIG, 64, arcade.record.name);
}

void AR_Drawer(void)
{
    switch (arcade.state)
    {
        case ARS_ATTRACT:
            if (gamestate == GS_LEVEL)
            {
                DrawLeaderboard();
                DrawCoinPrompt();
            }
            break;

        case ARS_BEGIN_PLAY:
            break;

        case ARS_PLAY:
            if (!automapactive)
            {
                DrawHud();
            }
            break;

        case ARS_DEAD:
            if (arcade.lives > 1)
            {
                char s[128] = {0};
                SDL_snprintf(s, 127, "%d LIVES LEFT", arcade.lives);
                AR_DrawString(ARCADE_FONT_BIG, 64, 64, s);
            }
            else
            {
                AR_DrawString(ARCADE_FONT_BIG, 64, 64, "LAST LIFE");
            }
            break;

        case ARS_BEGIN_LOAD_CHECKPOINT:
            break;

        case ARS_END_LOAD_CHECKPOINT:
            break;

        case ARS_BEGIN_ENTER_NAME:
            break;

        case ARS_ENTER_NAME:
            DrawNameEntry();
            break;

        case ARS_END_ENTER_NAME:
            break;

        case ARS_REBOOT:
            AR_DrawString(ARCADE_FONT_BIG, 64, 64, "GAME OVER");
            break;

        default:
            break;
    }
}

static void LoadCheckpoint(void);
void AR_Ticker(void)
{
    arcade.timer = SDL_max(0, arcade.timer - 1);

    switch (arcade.state)
    {
        case ARS_ATTRACT:
            break;

        case ARS_BEGIN_PLAY:
            arcade.state = ARS_PLAY;
            arcade.lives = arcade.coins * 3;
            arcade.coins = 0;
            arcade.game_start_time = SDL_GetTicks64();
            G_DeferedInitNew(arcade.isnightmare ? sk_nightmare : sk_hard, 1, 1);
            M_ClearMenus();
            SC_BeginNewRecord(arcade.isnightmare);
            break;

        case ARS_PLAY:
            break;

        case ARS_DEAD:
            break;

        case ARS_BEGIN_LOAD_CHECKPOINT:
            arcade.state = ARS_END_LOAD_CHECKPOINT;
            LoadCheckpoint();
            break;

        case ARS_END_LOAD_CHECKPOINT:
            break;

        case ARS_BEGIN_ENTER_NAME:
            if (arcade.timer == 0)
            {
                arcade.state = ARS_ENTER_NAME;
                arcade.record.duration_sec = (int)((SDL_GetTicks64() - arcade.game_start_time) / 1000ull);
                arcade.record.score = SC_GetCurrentScore();
            }
            break;

        case ARS_ENTER_NAME:
            break;

        case ARS_END_ENTER_NAME:
            SC_FinalizeRecord(&arcade.record);
            Reboot();
            break;

        case ARS_REBOOT:
            if (arcade.timer == 0)
            {
                arcade.state = ARS_ATTRACT;
                M_ClearMenus();
                D_StartTitle();
            }
            break;

        default:
            break;
    }
}

static void AdjustNameChar(int ofs, char* p)
{
    char c = SDL_toupper(*p);
    c += ofs;
    if (c < 'A') c = 'Z';
    else if (c > 'Z') c = 'A';
    *p = c;
}

boolean AR_Responder(event_t *ev)
{
    int key = ev->type == ev_keydown ? ev->data1 : 0;
    int button = ev->type == ev_joystick ? ev->data1 : 0;
    int mouse = ev->type == ev_mouse ? ev->data1 : 0;

    if (key == KEY_ESCAPE)
    {
        I_Quit();
        // unreachable
    }

    switch (arcade.state)
    {
        case ARS_ATTRACT:
            if (key == 'q')
            {
                ++arcade.coins;
                S_StartSound(NULL, sfx_brssit);
            }
            else if (key == '\\' || key == KEY_ENTER)
            {
                arcade.isnightmare = key == '\\';
                arcade.state = ARS_BEGIN_PLAY;
            }
            return true;

        case ARS_BEGIN_PLAY:
            return false;

        case ARS_PLAY:
            return false;

        case ARS_DEAD:
            if (arcade.timer == 0 && (key || button || mouse))
            {
                arcade.state = ARS_BEGIN_LOAD_CHECKPOINT;
            }
            return true;

        case ARS_BEGIN_LOAD_CHECKPOINT:
            return true;

        case ARS_END_LOAD_CHECKPOINT:
            return true;

        case ARS_BEGIN_ENTER_NAME:
            return true;

        case ARS_ENTER_NAME:
            if (key == key_up)
            {
                AdjustNameChar(1, &arcade.record.name[arcade.counter]);
            }
            else if (key == key_down)
            {
                AdjustNameChar(-1, &arcade.record.name[arcade.counter]);
            }
            else if (key == key_strafeleft)
            {
                arcade.counter = SDL_max(0, arcade.counter - 1);
            }
            else if (key == key_straferight)
            {
                arcade.counter = SDL_min(SC_NAME_LEN - 1, arcade.counter + 1);
            }
            else if (key == KEY_ENTER)
            {
                arcade.state = ARS_END_ENTER_NAME;
            }
            return true;

        case ARS_END_ENTER_NAME:
            return true;

        case ARS_REBOOT:
            return true;

        default:
            return false;
    }

    SDL_assert(false); // unreachable
}

static void SaveCheckpoint()
{
    SDL_assert(!demoplayback);
    SDL_assert(gameaction == ga_nothing);
    G_QuickSaveImmediate("arcade");
    SC_SaveCheckpoint();
}

static void LoadCheckpoint()
{
    SDL_assert(!demoplayback);
    SDL_assert(gameaction == ga_nothing);
    G_LoadGame(P_SaveGameFile(0));
    SC_LoadCheckpoint();
}

void AR_OnPlayDemo(void)
{
    arcade.state = ARS_ATTRACT;
}

void AR_OnLevelLoaded(void)
{
    if (arcade.state == ARS_ATTRACT)
    {
        return;
    }
    else if (arcade.state == ARS_END_LOAD_CHECKPOINT)
    {
        arcade.state = ARS_PLAY;
    }
    else
    {
        SDL_assert(arcade.state == ARS_PLAY || arcade.state == ARS_BEGIN_PLAY);
        SaveCheckpoint();
    }
}

void AR_OnDeath(void)
{
    if (arcade.state != ARS_PLAY)
    {
        return;
    }
    else if (--arcade.lives == 0)
    {
        if (SC_GetCurrentRank() >= 0)
        {
            arcade.timer = TICRATE * 2;
            arcade.state = ARS_BEGIN_ENTER_NAME;
        }
        else
        {
            Reboot();
        }
    }
    else
    {
        arcade.timer = TICRATE * 2;
        arcade.state = ARS_DEAD;
    }
}
