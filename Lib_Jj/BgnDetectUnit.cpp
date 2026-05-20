/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelTroops.h"

#include "LoUnit.h"
#include "LevelRecordUnit.h"


#include "LevelUtil.h"

#include "BgnDetectUnit.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectUnit


CLevelObj *DetectFirstTarget(CLevelObj *lo,float rangeMin,float rangeMax,std::vector<LevelDetectTargetFlag>&flags,
							 std::vector<LevelObjRequire>*requires,LevelObjMapEnumCallBack dlgt=NULL,CLevelObj **loIgnore=NULL,DWORD nIgnores=0)
{
	extern CLevelObj *LevelUtil_DetectFirst(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
	LevelUtilDetectParam param;
	param.loSrc=lo;
	param.toIgnores=loIgnore;
	param.nIgnores=nIgnores;
	param.flags=flags.data();
	param.nFlags=flags.size();
	if (requires)
	{
		param.requires=&(*requires)[0];
		param.nRequires=requires->size();
	}
	param.rangeMin=rangeMin;
	param.rangeMax=rangeMax;
	param.pos=lo->GetFramePos();

	return LevelUtil_DetectFirst(param,dlgt);
}

CLevelObj *DetectFirstTarget(CLevelObj *lo,float rangeMin,float rangeMax,std::vector<LevelDetectTargetFlag>&flags,
							 std::vector<LevelObjRequire>*requires,CLevelObj **loIgnore=NULL,DWORD nIgnores=0)
{
	return DetectFirstTarget(lo,rangeMin,rangeMax,flags,requires,NULL,NULL,0);
}


CLevelObj *DetectFirstTarget(CLevelObj *lo,float rangeMin,float rangeMax,LevelDetectTargetFlag flags,
							 std::vector<LevelObjRequire>*requires,std::vector<RecordID> *idAgents,LevelObjMapEnumCallBack dlgt=NULL,CLevelObj **loIgnore=NULL,DWORD nIgnores=0,RecordID idAgent=RecordID_Invalid)
{

	extern CLevelObj *LevelUtil_DetectFirst(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
	LevelUtilDetectParam param;
	param.loSrc=lo;
	param.toIgnores=loIgnore;
	param.nIgnores=nIgnores;
	param.flags=&flags;
	param.nFlags=1;
	if (requires)
	{
		param.requires=&(*requires)[0];
		param.nRequires=requires->size();
	}
	if (idAgents)
	{
		param.idAgents=&(*idAgents)[0];
		param.nAgents=idAgents->size();
	}
	param.rangeMin=rangeMin;
	param.rangeMax=rangeMax;
	param.pos=lo->GetFramePos();


	return LevelUtil_DetectFirst(param,dlgt);

}



CLevelObj *DetectBestTarget(CLevelObj *lo,float rangeMin,float rangeMax,std::vector<LevelDetectTargetFlag>&flags,
							   std::vector<LevelObjRequire>&requires,LevelDetectWeights &weights,std::vector<CLevelObj *> &detects)
{
	extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
	CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,std::vector<CLevelObj *>&candidates);
	LevelUtilDetectParam param;
	param.loSrc=lo;
	param.pos=lo->GetFramePos();
	param.toIgnores=NULL;
	param.nIgnores=0;
	param.flags=flags.data();
	param.nFlags=flags.size();
	param.requires=requires.data();
	param.nRequires=requires.size();
	param.rangeMin=rangeMin;
	param.rangeMax=rangeMax;
	param.weights.AddFlag(LevelDetectWeights_Dist);
	param.weights.wtDist=100.0f;
	param.weights.OverrideFrom(weights);

	if (detects.size()<=0)
		return LevelUtil_DetectBest(param,NULL);
	else
		return LevelUtil_DetectBest(param,NULL,detects);
}


// CLevelObj *DetectBestTarget(CLevelObj *lo,float rangeMin,float rangeMax,LevelDetectTargetFlag flags,
// 							   LevelObjRequire *requires,DWORD nRequires,
// 							   CLevelObj **loIgnore=NULL,DWORD nIgnores=0,LevelObjMapEnumCallBack dlgt=NULL)
// {
// 	extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
// 	LevelUtilDetectParam param;
// 	param.loSrc=lo;
// 	param.pos=lo->GetFramePos();
// 	param.toIgnores=loIgnore;
// 	param.nIgnores=nIgnores;
// 	param.flags=&flags;
// 	param.nFlags=1;
// 	param.requires=requires;
// 	param.nRequires=nRequires;
// 	param.rangeMin=rangeMin;
// 	param.rangeMax=rangeMax;
// 	param.weights.AddFlag(LevelDetectWeights_Dist);
// 	param.weights.wtDist=100.0f;
// 
// 	return LevelUtil_DetectBest(param,dlgt);
// }





BIND_BGN_CLASS(CBgn_DetectUnit,CBgp_DetectUnit);


void CBgn_DetectUnit::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DetectUnit*pad=_GetPad<CBgp_DetectUnit>();

	CLevelObj *loDetect=NULL;
	if ((pad->tpSight==DetectSightType_Me)||(pad->tpSight==DetectSightType_Owner)||(pad->tpSight==DetectSightType_Custom))
	{
		CLevelObj *lo=_GetLo();
		extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
		if (pad->tpSight==DetectSightType_Owner)
		{
			CLevelObj *loOwner=LevelUtil_GetOwnerLo(lo);
			if (!loOwner)
				lo=loOwner;
		}
		if (pad->tpSight==DetectSightType_Custom)
		{
			if (pad->nmCustomSrc!=StringID_Invalid)
			{
				LevelObjID idCustom;
				if (_GetID(pad->nmCustomSrc,BehaviorMemType_ObjID,idCustom))
				{
					CLevelObj *loCustom=LevelUtil_GetAliveLo(lo->GetLevel(),idCustom);
					if (loCustom)
						lo=loCustom;
				}
			}
		}
		if (lo)
		{
			CLevelObjMap *om=lo->GetLevel()->GetObjMap();
			if (!pad->bClosest)
				loDetect=DetectFirstTarget(lo,pad->rangeMin,pad->range,pad->flagsDetect,&pad->requires,NULL,NULL,0);
			else
			{
				std::vector<CLevelObj*> detects;
				loDetect=DetectBestTarget(lo,pad->rangeMin,pad->range,pad->flagsDetect,pad->requires,pad->weights,detects);
			}
		}
	}
	if (pad->tpSight==DetectSightType_Troop)
	{
		CLevelObj *lo=_GetLo();
		TroopCombatContext *tcc=_GetTcc();
		if (tcc)
			loDetect=DetectBestTarget(lo,pad->rangeMin,1000000.0f,pad->flagsDetect,pad->requires,pad->weights,tcc->detects);
	}

	if (loDetect)
	{
		if (pad->nmVar!=StringID_Invalid)
			_SetID(pad->nmVar,BehaviorMemType_ObjID,loDetect->GetID());
		_OutputOk(outputs,1,"侦测到");
	}
	else
	{
		if (pad->nmVar!=StringID_Invalid)
			_SetID(pad->nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
		_OutputFail(outputs,2,"未侦测到");
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectSpecifiedUnit
BIND_BGN_CLASS(CBgn_DetectSpecifiedUnit,CBgp_DetectSpecifiedUnit);
void CBgn_DetectSpecifiedUnit::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DetectSpecifiedUnit*pad=_GetPad<CBgp_DetectSpecifiedUnit>();

	CLevelObj *loDetect=NULL;
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelObjMapEnumCallBack dlgt;
		dlgt.bind(this,&CBgn_DetectSpecifiedUnit::_EnumCallBack);
		CLevelObjMap *om=lo->GetLevel()->GetObjMap();
		LevelDetectTargetFlag flag=pad->flagsDetect;
		flag=(LevelDetectTargetFlag)((DWORD)flag|LevelDetectTarget_Unit|LevelDetectTarget_Player|(DWORD)LevelDetectTargetFlag_Method);
		loDetect=DetectFirstTarget(lo,pad->rangeMin,pad->range,flag,NULL,NULL,dlgt);
	}

	if (loDetect)
	{
		if (pad->nmVar!=StringID_Invalid)
			_SetID(pad->nmVar,BehaviorMemType_ObjID,loDetect->GetID());
		_OutputOk(outputs,1,"侦测到");
		return;
	}

	if (pad->nmVar!=StringID_Invalid)
		_SetID(pad->nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
	_OutputFail(outputs,2,"未侦测到");
}

BOOL CBgn_DetectSpecifiedUnit::_EnumCallBack(CLevelObj *lo,float dist2)
{
	if(lo->GetType()!=LevelObjType_Unit)
		return FALSE;
	LevelRecordUnit *rec=((CLoUnit *)lo)->GetRec();
	if (!rec)
		return FALSE;

	if (lo==_GetLo())
		return FALSE;
	CBgp_DetectSpecifiedUnit*pad=_GetPad<CBgp_DetectSpecifiedUnit>();
	if (pad->idUnit!=RecordID_Invalid)
	{
		if (rec->GetID()!=pad->idUnit)
			return FALSE;
	}

	if(pad->idBuff!=RecordID_Invalid)
	{
		extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
		if (!LevelUtil_FindBuffByRecordID(lo,pad->idBuff))
			return FALSE;
	}

	if (pad->idItem!=RecordID_Invalid)
	{
		extern BOOL LevelUtil_CheckOwningItem(CLevelObj *lo,RecordID idItem);
		if (!LevelUtil_CheckOwningItem(lo,pad->idItem))
			return FALSE;
	}

	return TRUE;
}
