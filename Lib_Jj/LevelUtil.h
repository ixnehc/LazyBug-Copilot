
#pragma once

#include "LevelDefines.h"

#include "LevelEvents.h"

#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"

#include "unitmgr/UnitMgr.h"
#include "fastdelegate/FastDelegate.h"

#include "LevelObjMap.h"
#include "LevelAgentBrief.h"

class CLevel;
class CLevelObj;
class CLoUnit;
class CLevelBuff;
class CLevelSkill;
class CLevelBehaviorPersist;
struct AgentDistributeInfo;
struct LevelPlayerStates;
struct LevelRecordSkill;
struct LevelRecordEo;
struct LevelRecordUnit;
struct LevelRecordAgent;
class CLevelTalks;
struct KeySet;
class CLevelPlayer;
class CLevelSkillDriver;
struct LevelRelationMatrix;
class CLevelAbilities;
class CLevelRecords;
struct ExprEquips;
struct LevelItemState;
class CLevelRtnu;
class CLevelRtnus;
class CUnit;
class CUnitMgrNavMesh;
class CLevelService;
class CCubicSpline;
class CLoSlatesA;
struct UnitPainInfo;
struct AnimEventZone;
struct SkillParam_GeneralAdvS;
struct BellySetting;


extern BOOL LevelUtil_GetFramePos(CLevel *level,LevelObjID id,LevelPos &pos);
extern LevelPos LevelUtil_FindPosAround(LevelPos &center,float radius0,CLevel *level,DWORD nTry);
extern BOOL LevelUtil_MoveTo(CLevelObj *lo,LevelPos &pos,float rangeFollow=-1.0f);
extern BOOL LevelUtil_IsMoving_(CLevelObj *lo);
extern BOOL LevelUtil_CheckSimilarTarget(LevelPos &posCur,LevelPos &targetOld,LevelPos&targetNew);
extern CLevelObj *LevelUtil_DetectClosestPlayer(CLevelObj *lo,float range);
extern CLevelObj *LevelUtil_DetectPlayer(CLevelObj *lo,float range);
extern CLevelObj *LevelUtil_DetectClosestAgent(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent);
extern CLevelObj *LevelUtil_DetectClosestAgent(CLevel *level,LevelPos pos,float range,CLevelObj *toIgnore,RecordID idAgent);
extern CLevelObj **LevelUtil_DetectEnemies(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods,DWORD &c);

struct LevelUtilDetectParam
{
	LevelUtilDetectParam()
	{
		loSrc=NULL;
		memset(&pos,0,sizeof(pos));
		rangeMin=0.0f;
		rangeMax=0.0f;
		toIgnores=0;
		nIgnores=0;
		flags=0;
		nFlags=0;
		requires=0;
		nRequires=0;
		bTouching=0;
		idAgents=0;
		nAgents=0;
		recent=0;
		fail=0;
	}
	CLevelObj *loSrc;
	LevelPos pos;
	float rangeMin;
	float rangeMax;
	i_math::rectf rc;
	CLevelObj **toIgnores;
	DWORD nIgnores;
	LevelDetectTargetFlag *flags;
	DWORD nFlags;
	LevelObjRequire *requires;
	DWORD nRequires;
	BOOL bTouching;
	RecordID *idAgents;
	DWORD nAgents;
	CLevelObj *recent;
	CLevelObj *fail;
	LevelDetectWeightsBase weights;
};
extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
extern CLevelObj **LevelUtil_DetectInAll(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
extern CLevelObj *LevelUtil_DetectFirst(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);

extern CLevelObj **LevelUtil_DetectInZone(LevelUtilDetectParam &param,AnimEventZone &ezone,i_math::xformf&xfm,LevelTick t,DWORD &c);

extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);

extern BOOL LevelUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
extern AnimTick LevelUtil_GetBuffFlagAge(CLevelObj *lo,DWORD flagBuff);
extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
extern CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff);
extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
extern CLevelBuff* LevelUtil_FindBuffByRecordID(CLevel *level,LevelObjID idLo, RecordID idBuff);
extern void LevelUtil_RemoveBuffByRecordID(CLevelObj *lo,RecordID idBuff);
extern CLevelBuff *LevelUtil_FindBuffByFlag(CLevelObj *lo,CClass *clssBuff);
extern BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius);
extern BOOL LevelUtil_Follow(CLevelObj *lo,CLevelObj *loTarget,float range,BOOL bClosestFollow);//range设成-1表示使用缺省的follow range
extern BOOL LevelUtil_StopMove(CLevelObj *lo);

extern LevelRecordAgent* LevelUtil_GetAgentRec(CLevelObj *lo);
extern RecordID LevelUtil_GetAgentRecID(CLevelObj *lo);

extern CLevelObj *LevelUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent);
extern BOOL LevelUtil_FindLandingSpot(CLevelObj *lo,float fwd,float rangeDescend,float rangeLand,float height,LevelPos3D &posStart,LevelPos3D &posEnd);
extern LevelPlayerStates *LevelUtil_GetLPS(CLevel *lvl,CLevelObj *lo);
extern LevelPlayerStates *LevelUtil_GetLPS(CLevelObj *lo);
extern BOOL LevelUtil_CheckOwningItem(CLevelObj *lo,RecordID idItem);

extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
extern BOOL LevelUtil_CheckDead(CLevel *level,LevelObjID id);
extern BOOL LevelUtil_CheckInvisible(CLevelObj *lo);
extern BOOL LevelUtil_CheckDamageImmune(CLevelObj *lo);//检查lo是否是免疫状态
extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
extern LevelPos3D LevelUtil_GetWalkableGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso,UnitFindPathType tpFindPath=UnitFindPath_Walkable);
extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
extern CLevelObj *LevelUtil_GetOwnerLo(CLevel*lvl,LevelObjID id);
extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
extern CLevelPlayer *LevelUtil_GetOwnerPlayer(CLevelObj *lo);
extern BOOL LevelUtil_CheckLoRequire(CLevelObj *lo,LevelObjRequire *requires,DWORD c);
extern BOOL LevelUtil_CheckRelations(LevelRelationMask require,LevelRelation relation);//判断技能relation是否满足技能的要求
extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);//检查lo是否可以成为技能对象
extern LevelGrade LevelUtil_GetGrade(CLevelObj *lo);
extern float LevelUtil_GetSpeed(CLevelObj *lo);
extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
extern BOOL LevelUtil_CalcTargetAimPos3D(CLevel *level,LevelSkillTarget &target,float htCast,LevelPos3D &pos3D);
extern BOOL LevelUtil_CalcTargetDir(CLevelObj *loSrc,LevelSkillTarget &target,LevelPos&dir);
extern LevelObjID LevelUtil_GetTargetObjID(CLevel* level, LevelSkillTarget& target);
extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
extern void LevelUtil_VerifyTalksPlayer(CLevel *level,CLevelTalks *talks);
extern BOOL LevelUtil_ConvertSkillTarget(CLevel *level,LevelSkillTarget &target,LevelSkillTarget::Type tp);
extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
extern void LevelUtil_BuildPathKeyset(KeySet &ks,std::vector<LevelPos>&sites,float speed);
extern float LevelUtil_GetMountHeight(CLevelObj *lo);
extern LevelRecordUnit *LevelUtil_GetUnitRecord(CLevelObj *lo);
extern BOOL LevelUtil_TestSkillCost(LevelSkillType &tpSkill,CLevelObj *lo);
extern BOOL LevelUtil_CheckNeedAI(CLevelObj *lo);//判断一个LevelObj是否需要AI(比如当这个LevelObj已经死亡了,就不需要AI了)
extern BOOL LevelUtil_SwitchNPCRetinue(CLevelPlayer *player,RecordID idNPC);//把地图中的一个NPC变成某个Player的Retinue NPC
extern BOOL LevelUtil_AddCoSkillCharge(CLevelObj *lo,LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target);//
extern LevelSkillID LevelUtil_CancelSkill(CLevelObj *lo,BOOL bStopAct);
extern BOOL LevelUtil_CanCancelSkill(CLevelObj *lo);

extern LevelRelation LevelUtil_CalcPlayerRelation(LevelRelationMatrix *matRelation,LevelPlayerID idPlayer1,LevelPlayerID idPlayer2);
extern BOOL LevelUtil_ActivateAbility(CLevelAbilities *abilities,LevelAbilityType tp,CLevelRecords *records);
extern BOOL LevelUtil_ActivateAbility(CLevelPlayer *player,LevelAbilityType tp);
extern BOOL LevelUtil_DeactivateAbility(CLevelPlayer *player,LevelAbilityType tp);
extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelPlayer *player,LevelAbilityType tp);
extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
extern void LevelUtil_SaveAbilities(CLevelObj *lo);

extern EquipPart LevelUtil_GetItemEquipPart(CLevel *level,RecordID idItem);
extern EquipPart LevelUtil_GetItemEquipPart(CLevelRecords *records,RecordID idItem);
extern RecordID LevelUtil_ItemFromAbilityType(CLevelRecords *records,LevelAbilityType tp);
extern LevelArtifactType LevelUtil_ArtifactTypeFromAbilityType(CLevelRecords *records,LevelAbilityType tp);
extern BOOL LevelUtil_RemoveEquip(CLevelPlayer *player,EquipPart part);
extern BOOL LevelUtil_RemoveEquip(CLevelPlayer *player,RecordID idItem);
extern BOOL LevelUtil_AddArtifact(CLevelPlayer *player,RecordID idItem,int nStack);
void LevelUtil_RemoveArtifact(CLevelPlayer *player,LevelArtifactType tpArtifact);


extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
extern LevelSkillGrade LevelUtil_GetAbilitySkillGrade(CLevelObj *lo,LevelAbilityType tpAbility);

extern BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit);

struct LevelOSB;
extern BOOL LevelUtil_AddEventSrc(LevelOSB &osb,CLevelObj *target,LevelEventType tp);

struct LevelBehaviorContext;
class CLevelTalks;
extern LevelPlayerID LevelUtil_GetTalkingPlayer(LevelBehaviorContext *ctx,CLevelTalks *talks);

extern float LevelUtil_GenRandomFace();
extern void LevelUtil_RandomOffsetTargetPos2D(i_math::vector3df &posTarget,float radiusMin,float radiusMax);

extern BOOL LevelUtil_CheckPathValidity(CLevelObj *lo,RecordID idPath,LevelXfm &xfmBase);

extern BOOL LevelUtil_UpdateExprEquips(ExprEquips *equips,LevelPlayerStates *lps,CLevelRecords *records);

extern void LevelUtil_SendEvent(CLevelObj *lo,LevelEvent &e);

extern BOOL LevelUtil_FindPosAround(CLevelObj *lo,float radius0,LevelPos &pos,LevelFace &face,DWORD nTry);

extern RecordID LevelUtil_GetEquippingWeapon(CLevelObj *lo,RecordID *id2ndWpn=NULL);
extern RecordID LevelUtil_GetEquippingNonWeapon(CLevelObj *lo,EquipPart part);

extern BOOL LevelUtil_CheckEquipingAbility(CLevelObj *lo,LevelAbilityType tp);
extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);

extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
extern CLoUnit*LevelUtil_GetFirstPlayerLoUnit(CLevel *level);

extern int LevelUtil_GetResCount(CLevelPlayer *player,LevelResourceType tp);
extern BOOL LevelUtil_ExistArtifact(CLevelPlayer *player,LevelArtifactType tp);
extern BOOL LevelUtil_ExistArtifact(CLevelObj *lo,LevelArtifactType tp);
extern LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelPlayer *player,LevelArtifactType tp);
extern LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelObj *lo,LevelArtifactType tp);
extern WORD LevelUtil_GetStrength(CLevelObj *lo);
extern WORD LevelUtil_GetMagic(CLevelObj *lo);
extern WORD LevelUtil_GetHonor(CLevelObj *lo);
extern int LevelUtil_GetArrowCountAddOn(CLevelObj *lo);

extern BOOL LevelUtil_CheckAbilityToggledOn(CLevelObj *lo,LevelAbilityType tpAbility);
extern void LevelUtil_HandleSacredArrowFired(CLevelObj *lo);

extern LevelEoqPower LevelUtil_CalcEoqPower(CLevelObj *lo);
extern BOOL LevelUtil_ShieldAmuletHitTest(i_math::line3df &line,float radius,CLevelObj *loUnit,i_math::vector3df &vHit);

extern CLevelRtnus *LevelUtil_GetOwnerRtnus(CLevelObj *lo);
extern CLevelRtnu *LevelUtil_RtnuFromLo(CLevelObj *lo);

extern void LevelUtil_BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,CLevelObj*owner);

extern void LevelUtil_CalcLoMat(CLevelObj *lo,i_math::matrix43f &mat);
extern LevelFace LevelUtil_CalcTargetFacing(CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode);
extern LevelFace LevelUtil_CalcTargetFacing(CLevelObj *lo,LevelObjID idTarget);
extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);

extern LevelPos LevelUtil_CalcPredictedPos(CLevelObj *loSrc,CLevelObj *loTarget,float dtPredict);

extern LevelObjID LevelUtil_GetLevelObjIDFromVar(CLevelObj *owner,StringID nm);
extern CLevelService *LevelUtil_GetService(CLevelObj *lo,LevelServiceType tp);
extern CLevelService *LevelUtil_ObtainService(CLevelObj *lo,LevelServiceType tp);

extern LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,LevelPos &pos,LevelFace face,LevelPlayerID idPlayer);
extern LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,LevelPos3D &pos3D,LevelFace face,LevelPlayerID idPlayer);
extern LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,i_math::matrix43f &mat,LevelPlayerID idPlayer);
extern void LevelUtil_DestroyLo(CLevel *level,LevelObjID id);  
extern void LevelUtil_DeferDestroyLo(CLevel *level,LevelObjID id);

extern float LevelUtil_GetExaustedSP(CLevelObj *lo);

extern const char *LevelUtil_GetArtifactName(LevelArtifactType tp);
extern LevelAbilityType LevelUtil_AbilityFromArtifact(CLevelPlayer *player,LevelArtifactType tp);

extern DWORD LevelUtil_PlayerIDToUnitCollideAlly(LevelPlayerID idPlayer);

extern BOOL LevelUtil_IsBow(RecordID idItem,CLevelRecords *records);

extern BOOL LevelUtil_AddPathToSpline(CUnitMgr *unitmgr, CCubicSpline &spline,LevelPos3D &posSrc,LevelPos3D &posTarget,BOOL bResample);
extern BOOL LevelUtil_AddPathToSpline(CUnitMgr *unitmgr, CCubicSpline &spline,LevelPos &posSrc,LevelPos &posTarget,BOOL bResample);

typedef fastdelegate::FastDelegate1<LevelPos&,BOOL> FindNearbyPosCallBack;
extern BOOL LevelUtil_FindNearbyPos(CLevel *level,LevelPos &posCenter,float radius,BOOL bWalkable,BOOL bReachable,DWORD nTry,LevelPos &posResult,FindNearbyPosCallBack dlgt=NULL);
extern BOOL LevelUtil_FindNearbyPos(CLevel *level,LevelPos &posCenter,float radiusMin,float radiusMax,BOOL bWalkable,BOOL bReachable,DWORD nTry,LevelPos &posResult,FindNearbyPosCallBack dlgt=NULL);

typedef fastdelegate::FastDelegate1<LevelPos&,BOOL> FindFleePosCallBack;
extern BOOL LevelUtil_FindFleePos(CLevel *level,LevelPos &posCur,LevelPos &posEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius,
						   LevelPos &posResult,FindFleePosCallBack dlgt=NULL);

extern void LevelUtil_ModSPCost(CLevelObj *lo,float &sp,float &spCost);

extern void LevelUtil_UpdateAwardPriceAffordable(LevelPlayerStates *lps,LevelAwardPrice *price);
extern void LevelUtil_UpdateAwardExpendable(LevelPlayerStates *lps,LevelAward *award);
extern BOOL LevelUtil_CheckAwardAvailable(CLevelPlayer *player,RecordID idItem);

extern float LevelUtil_GetCurHP(CLevelObj *lo);
extern float LevelUtil_GetHealthRatio(CLevelObj *lo);
extern float LevelUtil_GetHealthRatio(CLevel *level,LevelObjID id);


extern CLoSlatesA *LevelUtil_GetSlatesAFromEmbed(CLevelObj *lo);
extern void LevelUtil_ChangeInSlatesBuff(CLevelPlayer *player,LevelObjID idSlates,BOOL bAddOrRemove);
extern BOOL LevelUtil_CheckInSlates(CLevelObj *lo);

extern void LevelUtil_AddTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,RecordID idItem,LevelAgentBrief::TreasureInfos::Entry &info);
extern void LevelUtil_RevealTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelResourceType tpRes,RecordID idItem);

extern float LevelUtil_CalcCurPain(LevelPain &pain,UnitPainInfo &infoPain,AnimTick tCur);
extern float LevelUtil_CalcCurPain(CLevelObj *lo);
extern float LevelUtil_CalcCurPainRatio(CLevelObj *lo);

extern BOOL LevelUtil_CalcSkillCastingXfm(CLevelSkill *skill,i_math::xformf &xfm);

extern LevelObjID LevelUtil_CreateEnvEo(CLevelObj *loOwner,RecordID idEo);
extern void LevelUtil_DestroyEnvEo(CLevel* level);

extern BellySetting& LevelUtil_GetBellySetting(CLevel *level);
