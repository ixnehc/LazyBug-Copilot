
#include "stdh.h"

#include "LevelDefines.h"

#include "LoSpawner.h"
#include "LoStartLoc.h"
#include "LoUnit.h"
#include "LoSPStone.h"
#include "LoLifeFlies.h"
#include "LoItem.h"
#include "LoHole.h"
#include "LoWatchTower.h"
#include "LoNPCLoc.h"
#include "LoLoc.h"
#include "LoRoute.h"
#include "LoTeleportSite.h"
#include "LoTeleporter.h"
#include "LoDormantSpawner.h"
#include "LoGeneralAgent.h"
#include "LoMagicBoard.h"
#include "LoSlate.h"
#include "LoSlatesA.h"
#include "LoSlatesB.h"
#include "LoCentipede.h"
#include "LoSnailP1.h"
#include "LoSlideway.h"
#include "LoStarPlate.h"
#include "LoMagicCircuit.h"
#include "LoBelly.h"

#include "EoBomb.h"
#include "EoAreaDmg.h"
//XXXXX:more level obj related class

#include "LevelItem.h"
//XXXXX:more level item related class

#include "LevelOp.h"

#include "LevelBuff.h"
#include "LevelSkill.h"
 


struct LevelClassEntry
{
	LevelClassEntry()
	{
		memset(this,0,sizeof(*this));
	}
	CClass *clssSrc;
	CClass *clssParam;
	CClass *clssObj;
	CClass *clssItem;
	CClass *clssOp;
	CClass *clssSkill;
	CClass *clssBuff;
};

#define MAX_LEVEL_UID 1000

#define DEFINE_ENTRY(clssname)											\
{																									\
	CClass *clss=Class_Ptr2(clssname);										\
	ClassUID uid=clss->GetUID();												\
	if (uid<MAX_LEVEL_UID)													\
	{																								\
		if (clss->_flag&ClassF_LevelObjSrc)										\
		{																								\
			assert(!entries[uid].clssSrc);																								\
			entries[uid].clssSrc=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelObjParam)										\
		{																								\
			assert(!entries[uid].clssParam);																								\
			entries[uid].clssParam=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelObj)										\
		{																								\
			assert(!entries[uid].clssObj);																								\
			entries[uid].clssObj=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelItem)										\
		{																								\
			assert(!entries[uid].clssItem);																								\
			entries[uid].clssItem=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelOp)										\
		{																								\
			assert(!entries[uid].clssOp);																								\
			entries[uid].clssOp=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelBuff)												\
		{																								\
			assert(!entries[uid].clssBuff);																								\
			entries[uid].clssBuff=clss;												\
		}																								\
		if (clss->_flag&ClassF_LevelSkill)												\
		{																								\
			assert(!entries[uid].clssSkill);																								\
			entries[uid].clssSkill=clss;												\
		}																								\
	}																								\
}


LevelClassEntry *GetEntries();

void InitLevelClasses()
{
	GetEntries();
}


CLevelObjSrc*NewLevelObjSrc(ClassUID uid)
{
	if (uid>=MAX_LEVEL_UID)
		return NULL;
	CClass *clss=GetEntries()[uid].clssSrc;
	return clss?(CLevelObjSrc*)clss->New():NULL;
}

CLevelObjParam*NewLevelObjParam(ClassUID uid)
{
	if (uid>=MAX_LEVEL_UID)
		return NULL;
	CClass *clss=GetEntries()[uid].clssParam;
	return clss?(CLevelObjParam*)clss->New():NULL;
}

CLevelObj*NewLevelObj(ClassUID uid)
{
	if (uid>=MAX_LEVEL_UID)
		return NULL;
	CClass *clss=GetEntries()[uid].clssObj;
	return clss?(CLevelObj*)clss->New():NULL;
}

class CLevelItem;
CLevelItem*NewLevelItem(ClassUID uid)
{
	if (uid>=MAX_LEVEL_UID)
		return NULL;
	CClass *clss=GetEntries()[uid].clssItem;
	return clss?(CLevelItem*)clss->New():NULL;
}

class CLevelOp;
CLevelOp*NewLevelOp(ClassUID uid)
{
	if (uid>=MAX_LEVEL_UID)
		return NULL;
	CClass *clss=GetEntries()[uid].clssOp;
	return clss?(CLevelOp*)clss->New():NULL;
}


const char *GetAgentsSemConstraint()
{
	static std::string s;
	if (s=="")
	{
		s="n/a:0";
		LevelClassEntry *entries=GetEntries();
		for (int i=0;i<MAX_LEVEL_UID;i++)
		{
			if (entries[i].clssObj)
			{
				CLevelObj *lo=(CLevelObj*)entries[i].clssObj->New();
				if (lo->GetType()==LevelObjType_Agent)
					AppendFmtString(s,",%s:%d",lo->GetShowName(),i);
				Safe_Class_Delete(lo);
			}
		}
	}
	return s.c_str();
}

const char *AgentNameFromUID(int uid)
{
	static std::string nm;
	nm="";
	LevelClassEntry *entries=GetEntries();
	if (uid<MAX_LEVEL_UID)
	{
		if (entries[uid].clssObj)
		{
			CLevelObj *lo=(CLevelObj*)entries[uid].clssObj->New();
			if (lo->GetType()==LevelObjType_Agent)
				nm=lo->GetShowName();
			Safe_Class_Delete(lo);
		}
	}
	return nm.c_str();
}


static LevelClassEntry *GetEntries()
{
	static LevelClassEntry entries[MAX_LEVEL_UID];

	static BOOL bInit=FALSE;

	if (!bInit)
	{
		bInit=TRUE;
		memset(entries,0,sizeof(entries));

		DEFINE_ENTRY(LosSpawner);
		DEFINE_ENTRY(LopSpawner);
		DEFINE_ENTRY(CLoSpawner);

		DEFINE_ENTRY(LosStartLoc);
		DEFINE_ENTRY(LosNPCLoc);

		DEFINE_ENTRY(LosLoc);
		DEFINE_ENTRY(LopLoc);

		DEFINE_ENTRY(LosRoute);
		DEFINE_ENTRY(LopRoute);

		DEFINE_ENTRY(CLoUnit);

		DEFINE_ENTRY(LosSPStone);
		DEFINE_ENTRY(LopSPStone);
		DEFINE_ENTRY(CLoSPStone);

		DEFINE_ENTRY(LosLifeFlies);
		DEFINE_ENTRY(LopLifeFlies);
		DEFINE_ENTRY(CLoLifeFlies);

		DEFINE_ENTRY(CLoItem);

		DEFINE_ENTRY(LosHole);
		DEFINE_ENTRY(LopHole);
		DEFINE_ENTRY(CLoHole);

		DEFINE_ENTRY(LosWatchTower);
		DEFINE_ENTRY(LopWatchTower);
		DEFINE_ENTRY(CLoWatchTower);

		DEFINE_ENTRY(LosTeleportSite);
		DEFINE_ENTRY(LopTeleportSite);
		DEFINE_ENTRY(CLoTeleportSite);

		DEFINE_ENTRY(LosTeleporter);
		DEFINE_ENTRY(LopTeleporter);
		DEFINE_ENTRY(CLoTeleporter);

		DEFINE_ENTRY(LosDormantSpawner);
		DEFINE_ENTRY(LopDormantSpawner);
		DEFINE_ENTRY(CLoDormantSpawner);

		DEFINE_ENTRY(LosGeneralAgent);
		DEFINE_ENTRY(LopGeneralAgent);
		DEFINE_ENTRY(CLoGeneralAgent);

		DEFINE_ENTRY(LosMagicBoard);
		DEFINE_ENTRY(LopMagicBoard);
		DEFINE_ENTRY(CLoMagicBoard);

		DEFINE_ENTRY(EoAreaDmg);
		DEFINE_ENTRY(EoBomb);

		DEFINE_ENTRY(LosSlate);
		DEFINE_ENTRY(LopSlate);

		DEFINE_ENTRY(LosSlatesA);
		DEFINE_ENTRY(LopSlatesA);
		DEFINE_ENTRY(CLoSlatesA);

		DEFINE_ENTRY(LosCentipede);
		DEFINE_ENTRY(LopCentipede);
		DEFINE_ENTRY(CLoCentipede);

		DEFINE_ENTRY(LosSnailP1);
		DEFINE_ENTRY(LopSnailP1);
		DEFINE_ENTRY(CLoSnailP1);

		DEFINE_ENTRY(LosSlatesB);
		DEFINE_ENTRY(LopSlatesB);
		DEFINE_ENTRY(CLoSlatesB);

		DEFINE_ENTRY(LosSlideway);
		DEFINE_ENTRY(LopSlideway);
		DEFINE_ENTRY(CLoSlideway);

		DEFINE_ENTRY(LosStarPlate);
		DEFINE_ENTRY(LopStarPlate);
		DEFINE_ENTRY(CLoStarPlate);

		DEFINE_ENTRY(LosMagicCircuit);
		DEFINE_ENTRY(LopMagicCircuit);
		DEFINE_ENTRY(CLoMagicCircuit);

		DEFINE_ENTRY(LosBelly);
		DEFINE_ENTRY(LopBelly);
		DEFINE_ENTRY(CLoBelly);
		//XXXXX:more level obj related class


		DEFINE_ENTRY(CLevelItem);
		//XXXXX:more level item related class

		DEFINE_ENTRY(LevelOp_StartSkill);
		DEFINE_ENTRY(LevelOp_SkillCasted);
		DEFINE_ENTRY(LevelOp_HPMod);
		DEFINE_ENTRY(LevelOp_BuffTimeUp);
		DEFINE_ENTRY(LevelOp_AddBuff);
		DEFINE_ENTRY(LevelOp_ModBuff);
		DEFINE_ENTRY(LevelOp_SkillTeleport);
		DEFINE_ENTRY(LevelOp_SPMod);
		DEFINE_ENTRY(LevelOp_FullSPMod);
		DEFINE_ENTRY(LevelOp_ExprEquip);
		DEFINE_ENTRY(LevelOp_FixPosEuler);
		DEFINE_ENTRY(LevelOp_CancelReside);
		DEFINE_ENTRY(LevelOp_ModBuffDur);
		DEFINE_ENTRY(LevelOp_ItemBirth);
		DEFINE_ENTRY(LevelOp_ResouceMod);
		DEFINE_ENTRY(LevelOp_Revive);
		DEFINE_ENTRY(LevelOp_Miss);
		DEFINE_ENTRY(LevelOp_StartFire);
		DEFINE_ENTRY(LevelOp_EoBirth);
		DEFINE_ENTRY(LevelOp_GradeMod);
		DEFINE_ENTRY(LevelOp_SyncBuffData);
		DEFINE_ENTRY(LevelOp_CombineSkill);
		DEFINE_ENTRY(LevelOp_SyncSkillData);
		DEFINE_ENTRY(LevelOp_CancelMount);
		DEFINE_ENTRY(LevelOp_CancelSkill);
		DEFINE_ENTRY(LevelOp_MBResouceMod);
		DEFINE_ENTRY(LevelOp_Path);
		DEFINE_ENTRY(LevelOp_SpeedMod);
		DEFINE_ENTRY(LevelOp_ShapeMod);
		DEFINE_ENTRY(LevelOp_DmgAbort);
		DEFINE_ENTRY(LevelOp_HonorMod);
		DEFINE_ENTRY(LevelOp_VitaMod);
		DEFINE_ENTRY(LevelOp_ChainedHammer);
		DEFINE_ENTRY(LevelOp_WormMod);
		DEFINE_ENTRY(LevelOp_Spore);
		DEFINE_ENTRY(LevelOp_FireFly);
		DEFINE_ENTRY(LevelOp_TempleMod);
		DEFINE_ENTRY(LevelOp_StrengthMod);
		DEFINE_ENTRY(LevelOp_EnableBody);
		DEFINE_ENTRY(LevelOp_MagicMod);
		DEFINE_ENTRY(LevelOp_PainMod);
		DEFINE_ENTRY(LevelOp_Dummy);
//XXXXX:More LevelOp



	}

	return entries;


}

