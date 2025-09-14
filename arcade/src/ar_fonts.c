#include "ar_fonts.h"

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"

#include <SDL_stdinc.h>
#include <SDL_assert.h>

// TODO: none of this is endian-safe

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t w;
    uint8_t h;
    int8_t advance;
    uint8_t page;
} glyph_t;

typedef struct
{
    char name[32];
    uint16_t w;
    uint16_t h;
    uint8_t lineheight;
    uint8_t baseheight;
    uint8_t numpages;
    glyph_t glyphs[256];
    uint8_t *pixels;
} font_t;

typedef PACKED_STRUCT({
    uint8_t id_length;
    uint8_t colormap_type;
    uint8_t image_type;

    uint16_t colormap_start_index;
    uint16_t colormap_num_entries;
    uint8_t colormap_bpp;

    uint16_t origin_x;
    uint16_t origin_y;
    uint16_t w;
    uint16_t h;
    uint8_t bpp;
    uint8_t descriptor;
}) tga_header_t;


static font_t fonts[NUM_ARCADE_FONTS] = {(font_t) {"debug"},
                                         (font_t) {"debug"}};

static void ReadLabeledLine(FILE *fp, const char *first_token,
                            char outbuf[1024])
{
    SDL_assert(first_token && *first_token);
    outbuf[1023] = 0;
    if (!fgets(outbuf, 1022, fp))
    {
        I_Error("ExpectLine: no line");
    }
    else if (strncmp(first_token, outbuf, strlen(first_token)) != 0)
    {
        I_Error("ExpectLine: expected %s, got:\n%s", first_token, outbuf);
    }
}

static void LoadBMFontMeta(font_t *font)
{
    static char buf[1024];
    FILE *fp;
    int skip, count, numchars, ichar;

    SDL_snprintf(buf, sizeof(buf), "ardata/%s.fnt", font->name);
    fp = fopen(buf, "rt");
    if (!fp)
    {
        I_Error("LoadBMFont: file not found: %s", buf);
    }

    ReadLabeledLine(fp, "info", buf);

    ReadLabeledLine(fp, "common", buf);
    count = sscanf(
        buf,
        "common lineHeight=%hhi base=%hhi scaleW=%i scaleH=%i pages=%hhu "
        "packed=%i alphaChnl=%i redChnl=%i greenChnl=%i blueChnl=%i",
        &font->lineheight, &font->baseheight, &skip, &skip, &font->numpages,
        &skip, &skip, &skip, &skip, &skip);
    if (count != 10)
    {
        I_Error("LoadBMFont: parse error on 'common...'");
    }
    else if (font->numpages != 1)
    {
        I_Error("LoadBMFont: only 1 page is supported");
    }

    ReadLabeledLine(fp, "page", buf);

    ReadLabeledLine(fp, "chars", buf);
    count = sscanf(buf, "chars count=%i", &numchars);
    if (count != 1)
    {
        I_Error("LoadBMFont: parse error on 'chars...'");
    }
    else if (numchars > SDL_arraysize(font->glyphs))
    {
        I_Error("LoadBMFont: too many chars (%d)", numchars);
    }
    else if (!numchars)
    {
        I_Error("LoadBMFont: no chars");
    }

    for (int i = 0; i < numchars; ++i)
    {
        glyph_t g = {0};
        ReadLabeledLine(fp, "char", buf);
        count =
            sscanf(buf,
                   "char id=%i x=%hu y=%hu width=%hhu height=%hhu xoffset=%i "
                   "yoffset=%i xadvance=%hhi page=%hhu chnl=%i",
                   &ichar, &g.x, &g.y, &g.w, &g.h, &skip, &skip, &g.advance,
                   &g.page, &skip);
        if (count != 10)
        {
            I_Error("LoadBMFont: parse error on 'char...'");
        }
        else if (ichar < 0 || ichar > SDL_arraysize(font->glyphs))
        {
            I_Error("LoadBMFont: invalid char index %d", ichar);
        }

        SDL_copyp(&font->glyphs[ichar], &g); // NOLINT(*-security.ArrayBound)
    }

    fclose(fp);
}

static void LoadBMFontPixelsTGA(font_t *font)
{
    static char buf[1024];
    tga_header_t header;
    FILE *fp;
    int count;

    SDL_assert(font && font->numpages == 1 && !font->pixels);

    SDL_snprintf(buf, sizeof(buf), "ardata/%s_0.tga", font->name);
    fp = fopen(buf, "rb");
    if (!fp)
    {
        I_Error("LoadBMFont: file not found: %s", buf);
    }

    if (!fread(&header, sizeof(header), 1, fp))
    {
        I_Error("LoadBMFont: short file");
    }
    else if (header.w < 8 || header.h < 8 || header.w > 8192 || header.h > 8192)
    {
        I_Error("LoadBMFont: weird size");
    }
    else if (header.bpp != 8)
    {
        I_Error("LoadBMFont: not 8bpp");
    }

    font->w = header.w;
    font->h = header.h;

    // skip id field
    if (fseek(fp, header.id_length, SEEK_CUR))
    {
        I_Error("LoadBMFont: short file");
    }

    // skip colormap
    count = header.colormap_bpp / 8 * header.colormap_num_entries;
    if (fseek(fp, count, SEEK_CUR))
    {
        I_Error("LoadBMFont: short file");
    }

    // read pixels
    count = header.w * header.h;
    font->pixels = Z_Malloc(count, PU_STATIC, NULL);
    if (fread(font->pixels, 1, count, fp) != count)
    {
        I_Error("LoadBMFont: short file");
    }

    // mirror if necessary
    if ((header.descriptor & 4) == 4)
    {
        I_Error("Right-to-left TGA not supported");
    }
    if ((header.descriptor & 5) == 5)
    {
        I_Error("Bottom-up TGA not supported");
    }

    fclose(fp);
}

static void LoadBMFont(font_t *font)
{
    SDL_assert(font && font->name[0]);
    SDL_assert(!font->pixels);
    printf("Loading font: %s\n", font->name);
    LoadBMFontMeta(font);
    LoadBMFontPixelsTGA(font);

    for (int i = 0; i < 256; ++i)
    {
        if (font->glyphs[i].w == 0)
        {
            // temp hack for missing space glyph
            font->glyphs[i].w = font->glyphs['W'].w;
        }
    }
}

static int CalcStringWidth(font_t *font, const char *str)
{
    int w = 0;
    while (*str)
    {
        w += font->glyphs[(uint8_t) *str].w;
        ++str;
    }
    return w;
}

void AR_InitFonts(void)
{
    for (int i = 0; i < NUM_ARCADE_FONTS; ++i)
    {
        LoadBMFont(&fonts[i]);
    }
}

void AR_DrawString(ar_font_e fontid, // NOLINT(*-easily-swappable-parameters)
                   int outx, int outy, const char *str)
{
    font_t *font;
    uint8_t c = (uint8_t) *str;

    if (fontid >= NUM_ARCADE_FONTS)
    {
        I_Error("AR_DrawString: bad font");
    }
    else if (!c || outx >= SCREENWIDTH || outy >= SCREENHEIGHT)
    {
        return;
    }

    // The big trouble with dumb bastards is that they are too dumb to believe
    // there is such a thing as being smart.
    // TODO: write a blit function that doesn't shame my ancestors

    font = &fonts[fontid];
    while (c)
    {
        glyph_t *g = &font->glyphs[c];

        for (int y = 0; y < g->h; ++y)
        {
            if (y + outy < 0)
            {
                continue;
            }
            else if (y + outy + g->h >= SCREENHEIGHT)
            {
                break;
            }

            for (int x = 0; x < g->w; ++x)
            {
                int i;
                uint8_t p;

                if (x + outx < 0)
                {
                    continue;
                }
                else if (x + outx >= SCREENWIDTH)
                {
                    break;
                }

                i = (x + g->x) + (y + g->y) * font->w;
                SDL_assert(i >= 0 && i < font->w * font->h);
                p = font->pixels[i];
                if (p)
                {
                    I_VideoBuffer[(x + outx) + (y + outy) * SCREENWIDTH] = p;
                }
            }
        }

        c = (uint8_t) *++str;
        outx += g->w;
    }
}

void AR_DrawStringRightAlign(ar_font_e fontid, int startx, int starty,
                             const char *str)
{
    if (fontid >= NUM_ARCADE_FONTS)
    {
        I_Error("AR_DrawStringRightAlign: bad font");
    }

    startx -= CalcStringWidth(&fonts[fontid], str);
    AR_DrawString(fontid, startx, starty, str);
}

void AR_DrawStringCentered(ar_font_e fontid, int starty, const char *str)
{
    int w;
    if (fontid >= NUM_ARCADE_FONTS)
    {
        I_Error("AR_DrawStringRightAlign: bad font");
    }

    w = CalcStringWidth(&fonts[fontid], str);
    w = (SCREENWIDTH - w) / 2;
    AR_DrawString(fontid, w, starty, str);
}
