
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoPushAdv.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LevelUtil.h"

#include "Skill_GeneralS.h"


BIND_EOPARAM(EoPushAdv,EoParamPushAdv);

void EoPushAdv::_OnDetroy()
{
	_handled.clear();
}

void EoPushAdv::_OnPostCreate()
{
	_tStart=_GetSkillCastingTime();

	EoParamPushAdv *param=GetParam<EoParamPushAdv>();
	CLevelObj *owner=_GetOwner();

	_level->RegisterSubframeUpdate(this);

}


LevelAttr_AttackMods*EoPushAdv::GetAttr_AttackMods()
{
	if (_GetOwner())
		return _GetOwner()->GetAttr_AttackMods();
	return NULL;
}

void EoPushAdv::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LevelSkillID idSkill=LevelSkillID_Invalid;
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
		idSkill=skill->GetID();

	bp->Data_WriteSimple(idSkill);
	bContent=TRUE;
}


void EoPushAdv::_OnUpdate()
{
	EoParamPushAdv *param=GetParam<EoParamPushAdv>();
	if (!param)
		return;

	CLevelObj *owner=_GetOwner();
	CLevelSkill *skillOwner=_GetOwnerSkill();
	if (_eZone)
	{
		if (skillOwner&&owner)
		{
			CLevel *level=owner->GetLevel();
			AnimTick tCur=_GetSkillCastingTime();
			if (tCur!=ANIMTICK_INFINITE)
			{
				tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

				if (tCur<=_eZone->GetDur())
				{
					i_math::xformf xfm;
					if (_GetSkillCastingXfm(xfm))
					{
						LevelUtilDetectParam paramDetect;
						paramDetect.loSrc=this;
						paramDetect.flags=&_rec->detects[0];
						paramDetect.nFlags=_rec->detects.size();
						paramDetect.requires=&_rec->requires[0];
						paramDetect.nRequires=_rec->requires.size();
						paramDetect.bTouching=TRUE;

						DWORD c;
						CLevelObj **los=LevelUtil_DetectInZone(paramDetect,*_eZone,xfm,tCur+_eZone->t,c);

						if (TRUE)
						{
							AnimEventZone::KeyFan k;
							if (_eZone->CalcKeyFan(tCur+_eZone->t,k))
							{
								k.xfmCenter.applyBase(xfm);
								level->GetDbgDraw().DrawFan(0,k,ColorAlpha(0xff0000,0xff),0.3f);
							}
						}

						if (_rec->deals.size()<=0)
						{
							LOG_DUMP_1P("EoPushAdv",Log_Error,"Eo(%s)里结算列表为空",_rec->Name.c_str());
						}

						for (int i=0;i<c;i++)
						{
							CLevelObj *loTarget=los[i];
							if (_handled.find(loTarget->GetID())!=_handled.end())
								continue;//已经处理过了

							level->GetDbgDraw().DrawSphere(GetID(),loTarget->GetFramePos3D(),0.2f,ColorAlpha(0xff0000,0xff),0.3f);

							DealArg arg;
							arg.dir.setXZ((loTarget->GetFramePos()-xfm.pos.getXZ()).safe_normalize());
							arg.grd=1;
							arg.link.id=level->GenOpLinkID();
							arg.link.t=tCur;
							_MakeDeals(loTarget,arg);

							_handled.insert(loTarget->GetID());
						}

						return;
					}
				}
			}
		}
	}

	DeferDestroy();
	return;
}


void EoPushAdv::UpdateSubframe()
{
	_OnUpdate();
}
