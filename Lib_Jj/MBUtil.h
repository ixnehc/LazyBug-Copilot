#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "LevelDefines.h"
#include "MagicBoardDefines.h"

class CLevel;
struct LevelAttr_MagicBoard;
struct MBResCost;
struct MagicTileInfo;
struct MagicBoardAIContext;
struct LevelOSB;

extern LevelPlayerID MBUtil_GetAIPlayer();
extern BOOL MBUtil_IsAIPlayer(LevelPlayerID idPlayer);
extern void MBUtil_MakeResMod(CLevel *level,LevelPlayerID idPlayer,MBResourceType tp,int mod,BOOL bInstant,LevelOSB &osb,LevelOpLink &link);
extern LevelAttr_MagicBoard *MBUtil_GetAttr(CLevel *level,LevelPlayerID idPlayer);

extern void MBUtil_MakeResMod(CLevel *level,LevelPlayerID idPlayer,MBResourceType tp,int mod,BOOL bInstant,LevelOSB &osb,LevelOpLink &link);
extern MBResCost *MBUtil_GetUnsealCost(CLevel *level,MagicTileInfo *ti);
extern BOOL MBUtil_CheckTileReady(CLevel *level,LevelPlayerID idPlayer,MagicTileInfo *ti);

extern MagicTileInfo **MBUtil_EnumUnseals(MagicBoardAIContext *ctx,RecordID idRec,DWORD &n);
extern MagicTileInfo **MBUtil_EnumCommits(MagicBoardAIContext *ctx,RecordID idRec,DWORD &n);