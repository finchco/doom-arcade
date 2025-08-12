#ifndef _SC_SCORE_
#define _SC_SCORE_

#include "../src/doom/doomdef.h"
#include "m_fixed.h"
typedef struct mobj_s mobj_t;

void SC_Init(void);
void SC_BeginNewRecord(boolean is_nightmare);
int SC_FinalizeRecord(char *player_name); // return leaderboard spot or -1
int SC_GetCurrentScore(void);

void SC_OnNextMap(int maxkills, int maxitems, int maxsecrets);
void SC_OnGetAmmo(ammotype_t ammo, int amount);
void SC_OnGetArmor(int amount);
void SC_OnGetBackpack(void);
void SC_OnGetHealth(int amount);
void SC_OnGetKey(int type);     // it_redcard etc
void SC_OnGetPowerup(int type); // pw_invisibility etc
void SC_OnGetWeapon(weapontype_t weapon, boolean was_dropped);
void SC_OnMappedWall(boolean is_boundary, boolean is_secret);
void SC_OnMobjDamaged(mobj_t *target, mobj_t *inflictor, mobj_t *source,
                      int damage, fixed_t thrust);
void SC_OnMobjKilled(mobj_t *target, mobj_t *inflictor, mobj_t *source);
void SC_OnTouchSecretSector(void);

void SC_OnLoadCheckpoint(void);
void SC_OnSaveCheckpoint(void);

#endif
