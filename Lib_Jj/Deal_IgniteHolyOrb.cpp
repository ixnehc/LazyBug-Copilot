#include "stdh.h"

#include "Deal_IgniteHolyOrb.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"
#include "LoAgent.h"
#include "LevelRecordAgent.h"
#include "LevelRecordEO.h"
#include "LevelRecords.h"
#include "behaviorgraph/BehaviorMem.h"
#include "LevelUtil.h"

#include "Level.h"

#include "EoHolyOrb.h"

BIND_DEAL(Deal_IgniteHolyOrb);

void Deal_IgniteHolyOrb::_DoIgnite(LevelOSB &osbSrc,CLoAgent *loAgent,DealArg&arg)
{
	LevelRecordAgent *rec=loAgent->GetRec();
	if (rec)
	{
		if (rec->GetID()==_idAgent)
		{
			CLevelBehavior *bhv=loAgent->GetBehaviorAI();
			if (bhv)
			{
				CBehaviorMem *mem=bhv->GetMem(0);
				if (mem)
				{
					RecordID idPathRes;
					if (mem->GetID(_nmVarPathRes,BehaviorMemType_ResourceRecord,idPathRes))
					{
						short IsActive;
						if (mem->GetNumber(_nmVarIsActive,IsActive))
						{
							if (IsActive)
							{
								mem->SetNumber(_nmVarIsActive,0);

								LevelPos3D pos=loAgent->GetFramePos3D();

								CLevel *level=osbSrc.GetLevel();
								CLevelObj *owner=osbSrc.GetOwner();
								LevelRecordEo *recEo=NULL;
								recEo=level->GetRecords()->GetEo(_idEo);
								if (recEo)
								{
									if (recEo->GetParam<EoParamHolyOrb>())
									{
										EoHolyOrb*eo=(EoHolyOrb*)level->CreateObj(recEo->param->GetEoClass());
										if (eo)
										{
											eo->SetPath(idPathRes);

											eo->PostCreate(owner->GetPlayerID(),recEo,pos,arg.dir,1,osbSrc,arg.link);
											level->AddToActives(eo);

										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

}



void Deal_IgniteHolyOrb::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	if (loTarget->GetType()==LevelObjType_Agent)
		_DoIgnite(osbSrc,(CLoAgent*)loTarget,arg);
}

BOOL Deal_IgniteHolyOrb::_DetectEnumCallBack(CLevelObj *lo,float dist2)
{
	if (lo->GetFramePos3D().getDistanceFrom(_posSrc)<_radius+lo->GetRadius_())
		return TRUE;
	return FALSE;
}

void Deal_IgniteHolyOrb::Make(LevelOSB &osbSrc,LevelPos3D &pos3D,DealArg&arg,DealResult *result)
{
	LevelUtilDetectParam param;

	CLevel *level=osbSrc.GetLevel();
	CLevelObj *owner=osbSrc.GetOwner();
	param.loSrc=owner;
	param.pos=pos3D.getXZ();
	param.rangeMin=0.0f;
	param.rangeMax=_radius;
	LevelDetectTargetFlag flag;
	flag=(LevelDetectTargetFlag)(LevelDetectTarget_Neutral|LevelDetectTarget_Enemy|LevelDetectTarget_Native|LevelDetectTarget_Ally|LevelDetectTarget_Agent|
				LevelDetectTarget_Ground);
	param.flags=&flag;
	param.nFlags=1;
	param.bTouching=1;
	param.idAgents=&_idAgent;
	param.nAgents=1;

	_posSrc=pos3D;
	LevelObjMapEnumCallBack dlgt;
	dlgt.bind(this,&Deal_IgniteHolyOrb::_DetectEnumCallBack);

	CLevelObj *loTarget=LevelUtil_DetectFirst(param,dlgt);
	if (loTarget)
	{
		if (loTarget->GetType()==LevelObjType_Agent)
			_DoIgnite(osbSrc,(CLoAgent*)loTarget,arg);
	}
}
