#include "sc_score.h"

#include <assert.h>
#include <stdlib.h>

#include "../../src/i_system.h"
#include "../../src/m_misc.h"
#include "../../src/doom/p_mobj.h"

#define SC_RECORD_FILENAME "ardata/arcade_records.txt"

#define SSCANF_FORMAT_STRING_LEN_(S) "%" #S "s"
#define SSCANF_FORMAT_STRING_LEN(S) SSCANF_FORMAT_STRING_LEN_(S)

#define SC_RECORD_VERSION 1

static sc_record_t sc_records[SC_NUM_RECORDS];

typedef struct
{
    boolean is_nightmare;
    int maxkills, maxitems, maxsecrets;
    int score;
} sc_score_t;

static sc_score_t sc_active_score;
static sc_score_t sc_checkpoint_score;

static sc_record_t sc_default_records[SC_NUM_RECORDS] = {
    {"REG", 5625, 113},
    {"DMK", 4922, 96},
    {"REG", 4543, 81},
    {"REG", 4041, 80},
    {"REG", 4025, 79},
    {"REG", 2395, 66},
    {"REG", 1433, 44},
    {"REG", 617, 15},
};

static void SC_LoadRecords(void)
{
    char line[128];
    int fileversion;
    FILE *f;
    sc_record_t *r;

    f = fopen(SC_RECORD_FILENAME, "rb");
    if (!f)
    {
        memcpy(sc_records, sc_default_records, sizeof(sc_records));
        return;
    }

    // check file version
    // TODO migration
    if (!fgets(line, sizeof(line), f)) { I_Error("Failed to read records"); }
    fileversion = atoi(line);
    if (fileversion != SC_RECORD_VERSION)
    {
        fclose(f);
        I_Error(SC_RECORD_FILENAME " is version %i, expected version %i\n",
                fileversion, SC_RECORD_VERSION);
    }

    for (int i = 0; i < SC_NUM_RECORDS; ++i)
    {
        r = &sc_records[i];
        if (!fgets(line, sizeof(line), f)) { I_Error("Failed to read recoreds"); }
        if (sscanf(line, SSCANF_FORMAT_STRING_LEN(SC_MAX_NAME_LEN) " %u %u",
                   r->name, &r->score, &r->duration_sec) != 3)
        {
            fclose(f);
            I_Error(SC_RECORD_FILENAME " line %i parse error\n", i);
        }
        r->name[SC_MAX_NAME_LEN] = 0;
    }

    fclose(f);
}

void SC_Init(void)
{
    memset(&sc_active_score, 0, sizeof(sc_active_score));
    memset(&sc_checkpoint_score, 0, sizeof(sc_checkpoint_score));
    memset(&sc_records[0], 0, sizeof(sc_records));
    SC_LoadRecords();
}

void SC_BeginNewRecord(boolean is_nightmare)
{
    memset(&sc_active_score, 0, sizeof(sc_active_score));
    sc_active_score.is_nightmare = is_nightmare;
    sc_active_score.score = 0;
}

int SC_FinalizeRecord(char *player_name)
{
    // find the rank
    FILE *f;
    int rank = 0;

    for (; rank < SC_NUM_RECORDS; ++rank)
    {
        if (sc_active_score.score > sc_records[rank].score)
        {
            break;
        }
    }

    if (rank == SC_NUM_RECORDS)
    {
        // did not place
        return -1;
    }

    // insert a record
    memmove(&sc_records[rank + 1], &sc_records[rank],
            sizeof(sc_record_t) * (SC_NUM_RECORDS - rank - 1));
    sc_records[rank].score = sc_active_score.score;
    sc_records[rank].duration_sec = 0; // TODO
    M_StringCopy(sc_records[rank].name, player_name, SC_MAX_NAME_LEN);

    // save
    f = fopen(SC_RECORD_FILENAME, "wb");
    if (!f)
    {
        I_Error(SC_RECORD_FILENAME " failed to open for write");
    }
    fprintf(f, "%i\n", SC_RECORD_VERSION);
    for (int i = 0; i < SC_NUM_RECORDS; ++i)
    {
        sc_record_t *r = &sc_records[i];
        fprintf(f, "%s %u %u\n", r->name, r->score, r->duration_sec);
    }
    fclose(f);

    return rank;
}

int SC_GetCurrentScore(void)
{
    return sc_active_score.score;
}

void SC_GetRecords(sc_record_t out[8])
{
    memcpy(out, sc_records, sizeof(sc_records));
}

void AR_OnNextMap(int maxkills, int maxitems, int maxsecrets)
{
    sc_active_score.maxkills = maxkills;
    sc_active_score.maxitems = maxitems;
    sc_active_score.maxsecrets = maxsecrets;
}

void AR_OnGetAmmo(ammotype_t ammo, int amount)
{
    sc_active_score.score += amount;
}

void AR_OnGetArmor(int amount)
{
    sc_active_score.score += amount;
}

void AR_OnGetBackpack(void)
{
    sc_active_score.score += 500;
}

void AR_OnGetHealth(int amount)
{
    sc_active_score.score += amount;
}

void AR_OnGetKey(int type)
{
    sc_active_score.score += 1000;
}

void AR_OnGetPowerup(int type)
{
    sc_active_score.score += 1000;
}

void AR_OnGetWeapon(weapontype_t weapon, boolean was_dropped)
{
    sc_active_score.score += 1000;
}

void AR_OnMappedWall(boolean is_boundary, boolean is_secret)
{
    sc_active_score.score += is_boundary + (is_secret * 100);
}

void AR_OnMobjDamaged(mobj_t *target, mobj_t *inflictor, mobj_t *source,
                      int damage, fixed_t thrust)
{
    sc_active_score.score += damage / 10;
}

static int SC_PointsForKill(mobj_t *killed)
{
    switch (killed->type)
    {
        case MT_POSSESSED:
            return 100;
        case MT_SHOTGUY:
            return 250;
        case MT_TROOP: // imp
            return 400;
        case MT_SERGEANT: // pinky
            return 650;
        case MT_HEAD: // cacodemon
            return 850;
        case MT_SKULL: // lost soul
            return 225;
        case MT_BRUISER: // baron
            return 1000;
        case MT_SPIDER: // mastermind
            return 6500;
        case MT_CYBORG: // cyberdemon
            return 7500;

        case MT_CHAINGUY:
        case MT_KNIGHT:
        case MT_BABY:
        case MT_PAIN:
        case MT_UNDEAD:
        case MT_FATSO:
        case MT_VILE:
            assert(false);
            return 0;

        default:
            return 0;
    }
}

void AR_OnMobjKilled(mobj_t *target, mobj_t *inflictor, mobj_t *source)
{
    int points = SC_PointsForKill(target);
    if (!points)
    {
        return;
    }
    if (inflictor && inflictor->type == MT_BARREL)
    {
        points *= 2;
    }
    sc_active_score.score += points;
}

void AR_OnTouchSecretSector(void)
{
    sc_active_score.score += 1000;
}

void SC_OnLoadCheckpoint(void)
{
    memcpy(&sc_active_score, &sc_checkpoint_score, sizeof(sc_score_t));
}

void SC_OnSaveCheckpoint(void)
{
    memcpy(&sc_checkpoint_score, &sc_active_score, sizeof(sc_score_t));
}
