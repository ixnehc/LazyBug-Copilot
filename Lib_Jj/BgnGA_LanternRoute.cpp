/********************************************************************
	created:	2022/08/07 
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelOSB.h"

#include "BgnGA_LanternRoute.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelRecordEO.h"

#include "LoGeneralAgent.h"

#include "LevelUtil.h"

#include "EoLanternRoute.h"



////////////////////////////////////////////////////////////////////////
//CBgnGA_LanternRoute_Spawn
BIND_BGN_CLASS(CBgnGA_LanternRoute_Spawn,CBgpGA_LanternRoute_Spawn);

void CBgnGA_LanternRoute_Spawn::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_LanternRoute_Spawn*pad=_GetPad<CBgpGA_LanternRoute_Spawn>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	CLoGeneralAgent *loPortal=NULL;

	if (pad->refAgentPortal.guid!=LevelGUID_Invalid)
		loPortal=level->GetIDs()->LoFromGUID<CLoGeneralAgent>(pad->refAgentPortal.guid);
 
	if ((pad->idEo!=RecordID_Invalid)&&(pad->idPathRes!=RecordID_Invalid)&&loPortal)
	{
		CLevelRecords *records=level->GetRecords();
		LevelRecordEo *rec=records->GetEo(pad->idEo);
		if (rec)
		{
			if (rec->param->GetEoClass()->IsSameWith(Class_Ptr2(EoLanternRoute)))
			{
				if (pad->matsLS.size()>0)
				{
					if (lo->GetLos())
					{
						i_math::xformf xfmSpawn;
						if (TRUE)
						{
							i_math::matrix43f &matBase=lo->GetLos()->GetMat();

							i_math::matrix43f mat=pad->matsLS[0]*matBase;
							xfmSpawn.fromMatrix(mat);
						}

						EoLanternRoute*eo=NULL;
						eo=(EoLanternRoute*)level->CreateObj(rec->param->GetEoClass());
						if (eo)
						{
							LevelGrade grd=0;

							eo->PostCreate(lo->GetPlayerID(),rec,xfmSpawn,NULL,grd,LevelOSB(lo),LevelOpLink());
							eo->SetPath(pad->idPathRes);
							eo->SetPortalID(loPortal->GetID());

							level->AddToActives(eo);
							if (pad->nmVar!=StringID_Invalid)
								_SetID(pad->nmVar,BehaviorMemType_ObjID,eo->GetID());
						}
					}
				}
			}
		}

	}


	_OutputOk(outputs,1,"结束");
	return;
}




////////////////////////////////////////////////////////////////////////
//CBgnGA_LanternRoute_Stop
BIND_BGN_CLASS(CBgnGA_LanternRoute_Stop,CBgpGA_LanternRoute_Stop);

void CBgnGA_LanternRoute_Stop::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_LanternRoute_Stop*pad=_GetPad<CBgpGA_LanternRoute_Stop>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelObjID idEo;

	if (_GetID(pad->nmVar,BehaviorMemType_ObjID,idEo))
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(level,idEo);
		if (lo)
		{
			EoLanternRoute *eo=lo->ToPtr<EoLanternRoute>();
			if (eo)
				eo->Stop();
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}



////////////////////////////////////////////////////////////////////////
//CBgnGA_LanternRoute_Check
BIND_BGN_CLASS(CBgnGA_LanternRoute_Check,CBgpGA_LanternRoute_Check);

void CBgnGA_LanternRoute_Check::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_LanternRoute_Check*pad=_GetPad<CBgpGA_LanternRoute_Check>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelObjID idEo;

	if (_GetID(pad->nmVar,BehaviorMemType_ObjID,idEo))
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(level,idEo);
		if (lo)
		{
			EoLanternRoute *eo=lo->ToPtr<EoLanternRoute>();
			if (eo)
			{	
				if (eo->IsOpened())
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"否");
}

