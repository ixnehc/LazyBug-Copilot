/*!
 * \file Ability_VampireRing.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"
#include "LevelOSB.h"

#include "LevelUtil.h"

#include "LevelAttrs.h"  

#include "Ability_VampireRing.h"

#include "Deal_CreateEo.h"

#include "Buff_Bleeding.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeVampireRing_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeVampireRing_Init);
BOOL CUpgradeVampireRing_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_VampireRing *ability=(CLevelAbility_VampireRing *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_VampireRing

void CLevelAbility_VampireRing::_SaveSync(CDataPacket &dp)
{
}

void CLevelAbility_VampireRing::_LoadSync(CDataPacket &dp,CRecords *records)
{
}

BOOL CLevelAbility_VampireRing::_CanSuck(CLevelObj *lo)
{
	CUpgradeVampireRing_Init *upgradeInitial=(CUpgradeVampireRing_Init *)_upgradeInitial;

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (!attr)
		return FALSE;

	if (attr->hp.GetRatio()>upgradeInitial->ratioHP)
	{
		extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
		if (!LevelUtil_FindBuff(lo,Class_Ptr2(Buff_Bleeding)))
			return FALSE;
	}

	return TRUE;
}


void CLevelAbility_VampireRing::_OnUpdate(LevelTick dt)
{
	CUpgradeVampireRing_Init *upgradeInitial=(CUpgradeVampireRing_Init *)_upgradeInitial;

	if (upgradeInitial->deal)
	{
		DWORD c=0;
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=_owner;
		paramDetect.pos=_owner->GetFramePos();
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=upgradeInitial->radius;
		paramDetect.flags=&upgradeInitial->detects[0];
		paramDetect.nFlags=upgradeInitial->detects.size();
		paramDetect.requires=&upgradeInitial->requires[0];
		paramDetect.nRequires=upgradeInitial->requires.size();

		extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
		CLevelObj **objs=LevelUtil_Detect(paramDetect,NULL,c);

		DealArg arg;

		for (int i=0;i<c;i++)
		{
			CLevelObj *lo=objs[i];
			if (!lo)
				continue;
			if (_links.find(lo->GetID())!=_links.end())
				continue;

			if (!_CanSuck(lo))
				continue;

			Deal_CreateEo * deal=(Deal_CreateEo *)upgradeInitial->deal;
			CLoEffectObj *eo=deal->CreateEo(LevelOSB(_owner),lo,arg);
			if (eo)
				_links[lo->GetID()]=eo->GetID();
			SAFE_RELEASE(eo);
		}
	}

	//删除旧的
	if (TRUE)
	{
		LevelPos posOwner=_owner->GetFramePos();
		float dist2=upgradeInitial->radius+upgradeInitial->tolerance;
		dist2*=dist2;
		std::unordered_map<LevelObjID,LevelObjID>::iterator it=_links.begin();
		while(it!=_links.end())
		{
			std::unordered_map<LevelObjID,LevelObjID>::iterator itCur=it;
			it++;

			LevelObjID id=(*itCur).first;
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,id);
			if (lo)
			{
				if (lo->GetFramePos().getDistanceSQFrom(posOwner)<dist2)
				{
					if (!LevelUtil_CheckDead(lo))
					{
						if (_CanSuck(lo))
							continue;
					}
				}
			}

			CLevelObj *eo=LevelUtil_GetAliveLo(_level,(*itCur).second);
			if (eo)
				eo->DeferDestroy();
			_links.erase(itCur);
		}
	}

}
