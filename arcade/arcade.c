#include "arcade.h"

#include "sc_score.h"
#include "../src/i_swap.h"
#include "../src/i_video.h"
#include "../src/m_misc.h"
#include "../src/v_patch.h"
#include "../src/v_video.h"

extern patch_t *shortnum[10];

static void DrawScoreRightAlign(int score, int x, int y)
{
    patch_t **p = shortnum;
    if (score == 0)
    {
        V_DrawPatch(x, y, p[0]);
        return;
    }

    for (int i = 0; score && i < 9; ++i)
    {
        int val = score % 10;
        x -= SHORT(p[val]->width);
        V_DrawPatch(x, y, p[val]);
        score /= 10;
    }
}

void AR_DrawHud(void)
{
    DrawScoreRightAlign(SC_GetCurrentScore(), SCREENWIDTH, 0);
}
