#include "../include/arcade.h"

#include "ar_fonts.h"
#include "sc_score.h"
#include "../../src/i_swap.h"
#include "../../src/i_video.h"
#include "../../src/m_misc.h"
#include "../../src/doom/g_game.h"
#include "../../src/doom/p_saveg.h"

void AR_Init(void)
{
    SC_Init();
    AR_InitFonts();
}

void AR_DrawHud(void)
{
    char s[10] = {0};
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

void AR_SaveCheckpoint()
{
    G_SaveGame(0, "arcade");
    SC_OnSaveCheckpoint();
}

void AR_LoadCheckpoint()
{
    G_LoadGame(P_SaveGameFile(0));
    SC_OnLoadCheckpoint();
}

void AR_OnNewGameBegin(boolean is_nightmare)
{
    // [arcade] save at start of map so it can be loaded on death
    AR_SaveCheckpoint();
    SC_BeginNewRecord(is_nightmare);
}
