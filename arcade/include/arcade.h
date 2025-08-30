#ifndef _ARCADE_H_
#define _ARCADE_H_

#include "../../src/doom/doomdef.h"
#include "../../src/doomtype.h"
#include "m_fixed.h"

typedef struct mobj_s mobj_t;

void AR_Init(void);

void AR_DrawHud(void);
void AR_DrawLeaderboard(void);

void AR_LoadCheckpoint(void);
void AR_SaveCheckpoint(void);

void AR_OnNewGameBegin(boolean is_nightmare);
void AR_OnNextMap(int maxkills, int maxitems, int maxsecrets);
void AR_OnGetAmmo(ammotype_t ammo, int amount);
void AR_OnGetArmor(int amount);
void AR_OnGetBackpack(void);
void AR_OnGetHealth(int amount);
void AR_OnGetKey(int type);     // it_redcard etc
void AR_OnGetPowerup(int type); // pw_invisibility etc
void AR_OnGetWeapon(weapontype_t weapon, boolean was_dropped);
void AR_OnMappedWall(boolean is_boundary, boolean is_secret);
void AR_OnMobjDamaged(mobj_t *target, mobj_t *inflictor, mobj_t *source,
                      int damage, fixed_t thrust);
void AR_OnMobjKilled(mobj_t *target, mobj_t *inflictor, mobj_t *source);
void AR_OnTouchSecretSector(void);


#endif
