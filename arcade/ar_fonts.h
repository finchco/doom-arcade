#ifndef _FON2_H_
#define _FON2_H_

typedef enum
{
    ARCADE_FONT_BIG,
    ARCADE_FONT_SMALL,
    NUM_ARCADE_FONTS
} ar_font_e;

void AR_InitFonts(void);
void AR_DrawString(ar_font_e fontid, unsigned startx, unsigned starty,
                   const char *str);

#endif
