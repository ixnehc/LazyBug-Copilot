/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:Make Buff
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
  
#include "BgnTroop_MakeBuff.h"

#include "LevelObj.h"
#include "LevelBGs.h"
#include "LevelTroops.h"

#include "LoGeneralAgent.h"

#include "LoUnit.h"

#include "Buff_Dead.h"
#include "Buff_General.h"

#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_MakeBuffTroop

void CBgnTroop_MakeBuff::_DoMakeBuff(MakeBuffInfo &info,LevelBuffArg *arg,AnimTick dur)
{
	CBgpTroop_MakeBuff*pad=_GetPad<CBgpTroop_MakeBuff>();
	if (_idBuff!=RecordID_Invalid||_specialBuff!=0)
	{
		CLevel *level=_GetLevel();
		if (level)
		{
			if (info.idUnit!=LevelObjID_Invalid)
			{
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *lo=LevelUtil_GetAliveLo(level,info.idUnit);
				if (lo)
				{
					if (_specialBuff==0)
						level->GetDecider()->MakeBuff(lo,_idBuff,dur,arg,TRUE);
					else
					{
						if (_specialBuff==1)
						{
							RecordID idBuff_Ash=lo->GetBuffID_Ash();
							if (idBuff_Ash!=RecordID_Invalid)
								level->GetDecider()->MakeBuff(lo,idBuff_Ash,dur,arg,TRUE);
						}
					}
				}
			}
		}
	}
}


void CBgnTroop_MakeBuff::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_MakeBuff*pad=_GetPad<CBgpTroop_MakeBuff>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BP_MakeBuff *param=&pad->_param;
	CLevelTroop *troop=_GetTroop(pad->_troop);

	_idBuff=param->idBuff;
	_specialBuff=param->special;

	LevelRecordBuff *recBuff=level->GetRecords()->GetBuff(_idBuff);

	BOOL bInstantly=TRUE;
	if (param)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		MakeBuffInfo info;

		if (troop)
		{
			if (param->speed>0.0f)
				bInstantly=FALSE;


			DWORD nFrames=troop->GetFrameCount();
			for (int i=0;i<nFrames;i++)
			{
				LevelTroopFrame *frm=troop->GetFrame(i);
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *lo=LevelUtil_GetAliveLo(level,frm->idUnit);
				if (!lo)
					continue;

				extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
				if (LevelUtil_CheckDead(lo))
					continue;

				if (!frm->CheckRank(pad->_flagsRank))
					continue;

				info.idUnit=frm->idUnit;

				if (bInstantly)
					_MakeBuff(info);
				else
					_infos.push_back(info);
			}
		}
// 		else
// 		{
// 			bInstantly=TRUE;
// 			LevelObjID id;
// 			if (_GetID(pad->_lo,BehaviorMemType_ObjID,id))
// 			{
// 				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
// 				CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
// 				if (lo)
// 				{
// 					extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
// 					if (!LevelUtil_CheckDead(lo))
// 					{
// 						info.idUnit=id;
// 						_MakeBuff(info);
// 					}
// 				}
// 			}
// 		}
	}

	if ((bInstantly)||(_infos.size()<=0))
	{
		_OutputOk(outputs,1,"结束");
		return;
	}

	CSysRandom::GenRandomIndices(_indices,_infos.size());

	_tStart=_GetT();

	Update(outputs);
	return;
}

void CBgnTroop_MakeBuff::Update(BGNOutputs &outputs)
{
	CBgpTroop_MakeBuff*pad=_GetPad<CBgpTroop_MakeBuff>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();
	BP_MakeBuff *param=&pad->_param;

	AnimTick tCur=_GetT();
	tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

	DWORD nToSpawn=0;
	if (param->speed<=0.0f)
		nToSpawn=100000;
	else
		nToSpawn=(DWORD)(param->speed*ANIMTICK_TO_SECOND(tCur));

	if (nToSpawn>_indices.size())
		nToSpawn=_indices.size();

	for (int i=_nMade;i<nToSpawn;i++)
		_MakeBuff(_infos[_indices[i]]);

	_nMade=nToSpawn;

	if (_nMade>=_indices.size())
	{
		_OutputOk(outputs,1,"结束");
	}
	else
		_SetResult(A_Pending);
}


//////////////////////////////////////////////////////////////////////////
//CBgpGA_MakeBuffTroop_Dead

class CBgpGA_MakeBuffTroop_Dead:public CBgpTroop_MakeBuff
{
public:
	DEFINE_CLASS(CBgpGA_MakeBuffTroop_Dead);
	virtual const char *GetTypeName()	{		return "杀死Troop所有单位";	}
};

class CBgnGA_MakeBuffTroop_Dead:public CBgnTroop_MakeBuff
{
	DEFINE_CLASS(CBgnGA_MakeBuffTroop_Dead);

	virtual void _MakeBuff(MakeBuffInfo &info)
	{
		BuffArg_Dead arg;
		_DoMakeBuff(info,&arg,ANIMTICK_INFINITE);
	}

};

BIND_BGN_CLASS(CBgnGA_MakeBuffTroop_Dead,CBgpGA_MakeBuffTroop_Dead);

//////////////////////////////////////////////////////////////////////////
//CBgpGA_MakeBuffTroop_General

class CBgpGA_MakeBuffTroop_General:public CBgpTroop_MakeBuff
{
public:
	DEFINE_CLASS(CBgpGA_MakeBuffTroop_General);
	virtual const char *GetTypeName()	{		return "为Troop添加通用Buff";	}
};

class CBgnGA_MakeBuffTroop_General:public CBgnTroop_MakeBuff
{
	DEFINE_CLASS(CBgnGA_MakeBuffTroop_General);

	virtual void _MakeBuff(MakeBuffInfo &info)
	{
		BuffArg_General arg;
		_DoMakeBuff(info,&arg,ANIMTICK_INFINITE);
	}

};

BIND_BGN_CLASS(CBgnGA_MakeBuffTroop_General,CBgpGA_MakeBuffTroop_General);
