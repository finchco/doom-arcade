#include "arcade.h"

#include "ar_fonts.h"
#include "sc_score.h"
#include "../src/i_swap.h"
#include "../src/i_video.h"

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
