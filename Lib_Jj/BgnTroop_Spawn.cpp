/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建单位
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnTroop_Spawn.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoUnit.h"
#include "LevelUtil.h"

#include "Buff_Birth.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_SpawnTroop
BIND_BGN_CLASS(CBgnTroop_Spawn,CBgpTroop_Spawn);

void CBgnTroop_Spawn::_Spawn(CLevel *level,CLevelTroop *troop,SpawnInfo &info)
{
	if (!info.desc)
		return;
	CLoUnit* lo=NULL;
	lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	if (info.hang<=0.0f)
		lo->PostCreate(info.desc->idPlayer,NULL,info.desc->idUnit,info.desc->grd,info.desc->arg,EquipSetPick_None,info.pos,info.face);
	else
	{
		LevelPos3D pos3D;
		extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
		pos3D=LevelUtil_GetGroundHeight(level,info.pos.x,info.pos.y,FALSE);
		pos3D.y+=info.hang;

		lo->PostCreate(info.desc->idPlayer,NULL,info.desc->idUnit,info.desc->grd,info.desc->arg,EquipSetPick_None,pos3D,info.face);
	}

	level->AddToActives(lo);
	if (troop)
		troop->FillUnit(info.desc,lo->GetID());
	if (info.desc->idBirthBuff!=RecordID_Invalid)
	{
		BuffArg_Birth arg;
		arg.eulerX=LevelFaceToEuler(info.face);
		level->GetDecider()->MakeBuff(lo,info.desc->idBirthBuff,0,&arg,FALSE);
	}
	lo->SetAIScenario(info.desc->senarioAI);
	SAFE_RELEASE(lo);
}


void CBgnTroop_Spawn::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Spawn*pad=_GetPad<CBgpTroop_Spawn>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	SpawnTroopParam *param=&pad->_param;

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (!troop)
	{
		LOG_DUMP_2P("Troop",Log_Error,"无法找到名为%s的Troop(%s)",StrLib_GetStr(pad->_troop),lo->GetClass()->GetName());
		_SetResult(A_Fail);
		return;
	}

	troop->FlushDeadUnits();

	BOOL bInstantly=TRUE;
	if (param)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		if (param)
		{
			std::vector<int>indices;

			if (param->speed>0.0f)
				bInstantly=FALSE;

			std::vector<LevelTroopDesc *>descs;
			//搜集所有的desc
			if (TRUE)
			{
				LevelTroopDesc *descLast=NULL;
				for (int i=0;i<troop->GetFrameCount();i++)
				{
					LevelTroopFrame *frm=troop->GetFrame(i);
					if (!frm)
						continue;
					if (frm->desc==descLast)
						continue;

					UNIQUE_VEC_ADD(descs,frm->desc);
					descLast=frm->desc;
				}
			}


			for (int i=0;i<descs.size();i++)
			{
				LevelTroopDesc *descCur=descs[i];

				std::vector<i_math::matrix43f> *mats=NULL;
				if (descCur->mats.size()>0)
					mats=&descCur->mats;
				else
				{
					if (param->mats.size()>0)
						mats=&param->mats;
				}
				if (mats)
				{
					if (troop->GetFrameCount()<=mats->size())
						CSysRandom::GenRandomIndices(indices,mats->size());
					else
						CSysRandom::GenRandomIndices(indices,troop->GetFrameCount());
				}

				int idx=0;
				for (int j=0;j<troop->GetFrameCount();j++)
				{
					LevelTroopFrame *frm=troop->GetFrame(j);
					if (!frm)
						continue;
					if (frm->desc!=descCur)
						continue;

					SpawnInfo info;
					info.desc=descCur;

					if (mats)
					{
						LevelPos pos;
						i_math::matrix43f &mat=(*mats)[indices[idx]%mats->size()];
						pos.x=mat.getTranslationP()->x;
						pos.y=mat.getTranslationP()->z;


						info.pos=pos;
						if (!param->bRandomRot)
						{
							i_math::xformf xfm;
							xfm.fromMatrix(mat);

							info.face=LevelFaceFromQuat(xfm.rot);
						}
						else
							info.face=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
					}
					else
					{
						info.face=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);

						if (!LevelUtil_FindNearbyPos(level,lo->GetFramePos(),param->radiusMin,param->radiusMax,TRUE,FALSE,10,info.pos))
							continue;
					}

					info.hang=descCur->hang;

					if (bInstantly)
						_Spawn(level,troop,info);
					else
						_infos.push_back(info);

					idx++;
				}
			}
		}
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

void CBgnTroop_Spawn::Update(BGNOutputs &outputs)
{
	CBgpTroop_Spawn*pad=_GetPad<CBgpTroop_Spawn>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();
	SpawnTroopParam *param=&pad->_param;

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (!troop)
	{
		_SetResult(A_Fail);
		return;
	}

	AnimTick tCur=_GetT();
	tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

	DWORD nToSpawn=0;
	if (param->speed<=0.0f)
		nToSpawn=100000;
	else
		nToSpawn=(DWORD)(param->speed*ANIMTICK_TO_SECOND(tCur));

	if (nToSpawn>_indices.size())
		nToSpawn=_indices.size();

	for (int i=_nSpawned;i<nToSpawn;i++)
		_Spawn(level,troop,_infos[_indices[i]]);

	_nSpawned=nToSpawn;

	if (_nSpawned>=_indices.size())
	{
		_OutputOk(outputs,1,"结束");
	}
	else
		_SetResult(A_Pending);
}
