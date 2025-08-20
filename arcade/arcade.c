#include "arcade.h"

#include "ar_fonts.h"
#include "sc_score.h"
#include "../src/i_swap.h"
#include "../src/i_video.h"
#include "../src/m_misc.h"
#include "../src/v_patch.h"
#include "../src/v_video.h"

extern patch_t *shortnum[10];
extern patch_t *tallnum[10];

static void DrawScoreRightAlign(patch_t** patches, int score, int x, int y)
{
    if (score == 0)
    {
        V_DrawPatch(x, y, patches[0]);
        return;
    }

    for (int i = 0; score && i < 9; ++i)
    {
        int val = score % 10;
        x -= SHORT(patches[val]->width);
        V_DrawPatch(x, y, patches[val]);
        score /= 10;
    }
}


void AR_DrawHud(void)
{
    char s[10] = {0};
    SDL_itoa(SC_GetCurrentScore(), s, 9);
    AR_DrawString(ARCADE_FONT_BIG, 0, 0, s);
}

void AR_DrawLeaderboard(void)
{
    sc_record_t records[SC_NUM_RECORDS];
    int x, y;
    int startx = 128;

    SC_GetRecords(records);

    x = startx;
    y = 40;
    for (int i = 0; i < SC_NUM_RECORDS; ++i)
    {
        //DrawName(records[i].name, x, y);
        x += 128;
        DrawScoreRightAlign(tallnum, records[i].score, x, y);
        x = startx;
        y += 16;
    }
}
