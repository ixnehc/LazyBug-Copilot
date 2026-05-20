#include "stdh.h"

#include "Deal_CreateEo.h"

#include "LevelOSB.h"
#include "LevelEvents.h"

#include "LevelRecordEO.h"
#include "LevelRecords.h"

#include "Log/LogDump.h"


#include "Level.h"

BIND_DEAL(Deal_CreateEo);

Deal_CreateEo::~Deal_CreateEo()
{
	SAFE_RELEASE(_recOverride);
	GDestructor();
}


void Deal_CreateEo::OverrideRecordEO(LevelRecordEo *rec)
{
	SAFE_REPLACE(_recOverride,rec);
}

class CLoEffectObj;

CLoEffectObj *Deal_CreateEo::CreateEo(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,LevelObjID idHost)
{
	CLevel *level=osbSrc.GetLevel();
	CLevelObj *owner=osbSrc.GetOwner();
	LevelRecordEo *rec=NULL;
	if (_recOverride)
		rec=_recOverride;
	else
		rec=level->GetRecords()->GetEo(_idEo);
	CLoEffectObj *eo=NULL;
	if (rec)
	{
		if (_bSendEvent)
			eo=LePreCreateEo::Send(osbSrc,rec,pos,&arg);

		if (!eo)
		{
			eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
			if (eo)
			{
				eo->SetHost(idHost);
				eo->PostCreate(owner->GetPlayerID(),rec,pos,arg.dir,1,osbSrc,arg.link);
				level->AddToActives(eo);
			}
		}
	}

	if (eo)
	{
		if (_bSendEvent)
			LePostCreateEo::Send(osbSrc,eo,NULL,arg.link);
	}

	return eo;
}

void Deal_CreateEo::Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)
{
	CLoEffectObj *eo=CreateEo(osbSrc,pos,arg,LevelObjID_Invalid);

	if (!eo)
	{
		LOG_DUMP("Deal_CreateEo",Log_Error,"无法创建特效对象(Eo)!");
	}
	SAFE_RELEASE(eo);
}

CLoEffectObj *Deal_CreateEo::CreateEo(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg)
{
	if (!loTarget)
		return NULL;

	CLevel *level=osbSrc.GetLevel();
	CLevelObj *owner=osbSrc.GetOwner();
	LevelRecordEo *rec=NULL;
	if (_recOverride)
		rec=_recOverride;
	else
		rec=level->GetRecords()->GetEo(_idEo);
	CLoEffectObj *eo=NULL;
	if (rec)
	{
		eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
		if (eo)
		{
			LevelPos3D pos=loTarget->GetFramePos3D();
			if (_bUseAimPos)
				pos.y+=loTarget->GetAimHeight();
			eo->SetHost(loTarget->GetID());
			if (arg.argObliterate)
				eo->SetObliterateArg(*arg.argObliterate);
			eo->PostCreate(owner->GetPlayerID(),rec,pos,arg.dir,1,osbSrc,arg.link);
			level->AddToActives(eo);
		}
	}

	if (eo)
	{
		if (_bSendEvent)
			LePostCreateEo::Send(osbSrc,eo,NULL,arg.link);
	}

	return eo;
}


void Deal_CreateEo::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLoEffectObj *eo=CreateEo(osbSrc,loTarget,arg);
	if (!eo)
	{
		LOG_DUMP("Deal_CreateEo",Log_Error,"无法创建特效对象(Eo)!");
	}
	SAFE_RELEASE(eo);
}

