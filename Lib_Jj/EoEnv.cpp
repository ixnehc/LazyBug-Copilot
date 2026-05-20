
#include "stdh.h"

#include "Level.h"
#include "LevelUtil.h"

#include "EoEnv.h"

#include "LevelRecords.h"

#include "LevelOp.h"
#include "LevelOSB.h"
#include "LevelDeal.h"


BIND_EOPARAM(EoEnv,EoParamEnv);


EoEnvLichenHandle EoEnv::StartLichen(LevelPos &pos,float radius,BOOL bDispel,float radiusDispelCore,float durFI,float durFO)
{
	if (_bPendingDestroy)
		return EoEnvLichenHandle_Invalid;
	if (!_level)
		return EoEnvLichenHandle_Invalid;
	EoEnvLichenHandle h=_level->_GenEnvLichenHandle();

	LichenEntry &e=_lichens[h];
	e.tStart=_level->GetT_();
	e.durFI=ANIMTICK_FROM_SECOND(durFI);
	e.durFO=ANIMTICK_FROM_SECOND(durFO);
	e.pos=pos;
	e.radius=radius;
	e.handle=h;
	e.bTrail=FALSE;
	e.bDispel=bDispel;
	e.radiusDispelCore=radiusDispelCore;

	if (TRUE)
	{
		EoEnvOp_LichenStart *op=Class_New2(EoEnvOp_LichenStart);
		op->hLichen=h;
		op->pos=pos;
		op->radius=radius;
		op->durFI=e.durFI;
		op->durFO=e.durFO;

		_opsEnv.push_back(op);
	}

	return h;
}

EoEnvLichenHandle EoEnv::StartLichen(LevelObjID idHost,float radius,BOOL bDispel,float radiusDispelCore,float durFI,float durFO)
{
	if (_bPendingDestroy)
		return EoEnvLichenHandle_Invalid;
	if (!_level)
		return EoEnvLichenHandle_Invalid;
	EoEnvLichenHandle h=_level->_GenEnvLichenHandle();

	LichenEntry &e=_lichens[h];
	e.tStart=_level->GetT_();
	e.durFI=ANIMTICK_FROM_SECOND(durFI);
	e.durFO=ANIMTICK_FROM_SECOND(durFO);
	e.idHost=idHost;
	e.radius=radius;
	e.handle=h;
	e.bTrail=FALSE;
	e.bDispel=bDispel;
	e.radiusDispelCore=radiusDispelCore;

	if (TRUE)
	{
		EoEnvOp_LichenStart *op=Class_New2(EoEnvOp_LichenStart);
		op->hLichen=h;
		op->idHost=idHost;
		op->radius=radius;
		op->durFI=e.durFI;
		op->bTrail=FALSE;
		op->bDispel=bDispel;
		op->radiusDispelCore=radiusDispelCore;

		_opsEnv.push_back(op);
	}

	return h;
}

EoEnvLichenHandle EoEnv::StartLichenTrail(LevelObjID idHost,float radius)
{
	if (_bPendingDestroy)
		return EoEnvLichenHandle_Invalid;
	if (!_level)
		return EoEnvLichenHandle_Invalid;

	EoParamEnv *param=GetParam<EoParamEnv>();

	EoEnvLichenHandle h=_level->_GenEnvLichenHandle();

	LichenEntry &e=_lichens[h];
	e.tStart=_level->GetT_();
	e.durFI=0;
	e.durFO=0;
	e.idHost=idHost;
	e.radius=radius;
	e.handle=h;
	e.bTrail=TRUE;
	e.bTrail=FALSE;

	if (TRUE)
	{
		EoEnvOp_LichenStart *op=Class_New2(EoEnvOp_LichenStart);
		op->hLichen=h;
		op->idHost=idHost;
		op->radius=radius;
		op->durFI=e.durFI;
		op->durFO=e.durFO;
		op->bTrail=TRUE;

		_opsEnv.push_back(op);
	}

	return h;

}


void EoEnv::StopLichen(EoEnvLichenHandle hLichen)
{
	if (_bPendingDestroy)
		return;

	std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it=_lichens.find(hLichen);
	if (it!=_lichens.end())
	{
		LichenEntry &e=(*it).second;

		if (e.tStop==ANIMTICK_INFINITE)
		{
			e.tStop=_level->GetT_();

			if (TRUE)
			{
				EoEnvOp_LichenStop *op=Class_New2(EoEnvOp_LichenStop);
				op->hLichen=(*it).first;

				_opsEnv.push_back(op);
			}
		}
	}
}

void EoEnv::UpdateLichenPos(EoEnvLichenHandle hLichen,LevelPos &posNew)
{
	if (_bPendingDestroy)
		return;

	std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it=_lichens.find(hLichen);
	if (it!=_lichens.end())
	{
		LichenEntry &e=(*it).second;
		e.pos=posNew;
	}
}

void EoEnv::_CollectLichenAffects(LevelPos &pos,float radius0,float wt)
{
	float radius=radius0*wt;
	float radius2=radius*radius;
	DWORD c=0;
	CLevelObj **los=NULL;

	los=_DetectRange(pos,radius,c);

	LevelPlayerID idPlayerMe=GetPlayerID();

	LevelRelationMatrix *matRelation=_level->GetRelationMatrix();
	if (matRelation)
	{
		for (int i=0;i<c;i++)
		{
			CLevelObj *lo=los[i];
			float dist2=lo->GetFramePos().getDistanceSQFrom(pos);
			if (dist2>radius2)
				continue;

			float wtAffect=wt*(radius2-dist2)/radius2;


			LevelRelation relation=LevelUtil_CalcPlayerRelation(matRelation,idPlayerMe,lo->GetPlayerID());
			if ((relation!=LevelRelation_Native)&&(relation!=LevelRelation_Enemy))
				continue;

			std::map<CLevelObj *,float>&affects=(relation==LevelRelation_Native)?_affectsLichenNative:_affectsLichenEnemy;

			std::map<CLevelObj *,float>::iterator itAffect=affects.find(lo);

			if (itAffect==affects.end())
				affects[lo]=wtAffect;
			else
			{
				if (wtAffect>(*itAffect).second)
					(*itAffect).second=wtAffect;
			}
		}
	}
}


void EoEnv::_UpdateLichenDeal()
{
	EoParamEnv *param=GetParam<EoParamEnv>();

	AnimTick t=_level->GetT_();
	AnimTick tAge=ANIMTICK_SAFE_MINUS(t,_tCreate);

	if (TRUE)
	{
		int nLichenTrailToSample=tAge/param->durLichenTrailSampleCycle;
		if(nLichenTrailToSample>_nLichenSamples)
		{
			std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it=_lichens.begin();
			while(it!=_lichens.end())
			{
				std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator itCur=it;
				it++;

				LichenEntry &e=(*itCur).second;
				if (e.bTrail)
				{
					if (!e.IsStopped())
					{
						if(e.idHost!=LevelObjID_Invalid)
						{
							CLevelObj *lo=LevelUtil_GetAliveLo(_level,e.idHost);
							if (lo)
							{
								LichenEntry::TrailSample sample;
								sample.pos=lo->GetFramePos();
								sample.tStart=t;
								e.trail.push_back(sample);
							}
						}
					}
				}
			}
		}
		_nLichenSamples=nLichenTrailToSample;
	}

	int nLichenToDeal=tAge/param->durLichenDealCycle;
	while (nLichenToDeal>_nLichenDeals)
	{
		_affectsLichenEnemy.clear();
		_affectsLichenNative.clear();

		LichenEntry *entryDispel=NULL;

		if (TRUE)
		{
			std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it=_lichens.begin();
			while(it!=_lichens.end())
			{
				std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator itCur=it;
				it++;

				LichenEntry &e=(*itCur).second;

				if (e.tStart==ANIMTICK_INFINITE)
					continue;//尚未开始

				//清除过期的
				if (!e.bTrail)
				{
					if (e.IsStopped())
					{
						if (e.tStop+e.durFO<t)
						{
							_lichens.erase(itCur);
							continue;
						}
					}
				}
				else
				{
					if (e.trail.empty())
					{
						_lichens.erase(itCur);
						continue;
					}
				}

				if (e.bDispel)
				{
					if (!entryDispel)
						entryDispel=&e;
					continue;
				}

				if ((!e.bTrail)&&(!e.bDispel))
				{
					float wt=calc_faded_weight(e.tStart,e.durFI,e.tStop,e.durFO,t);
					if (t<e.tStart+e.durFI)
						wt=pow(wt,0.1f);

					if (TRUE)
					{
						if (e.idHost==LevelObjID_Invalid)
							_CollectLichenAffects(e.pos,e.radius,wt);
						else
						{
							CLevelObj *lo=LevelUtil_GetAliveLo(_level,e.idHost);
							if (lo)
								_CollectLichenAffects(lo->GetFramePos(),e.radius,wt);
						}
					}
				}
				if (e.bTrail)
				{
					while(e.trail.size()>0)
					{
						LichenEntry::TrailSample &sample=e.trail[0];

						if (sample.tStart+param->durLichenTrailFI+param->durLichenTrailFO+param->durLichenTrail<t)
						{
							e.trail.pop_front();
							continue;
						}
						else
							break;
					}

					for (int i=0;i<e.trail.size();i++)
					{
						LichenEntry::TrailSample &sample=e.trail[i];

						float wt=calc_faded_weight(sample.tStart,
												param->durLichenTrailFI,
												sample.tStart+param->durLichenTrailFI+param->durLichenTrail,
												param->durLichenTrailFO,
												t);
						_CollectLichenAffects(sample.pos,e.radius,wt);
					}
				}
			}
		}

		if (TRUE)
		{
			DealArg arg;

			float radiusDispel=0.0f;
			float radiusDispelCore=0.0f;
			LevelPos posDispel;
			if (entryDispel)
			{
				float wt=calc_faded_weight(entryDispel->tStart,entryDispel->durFI,entryDispel->tStop,entryDispel->durFO,t);
				radiusDispel=entryDispel->radius*wt;
				radiusDispelCore=entryDispel->radiusDispelCore*wt;
				posDispel=entryDispel->pos;
			}


			LevelOSB osb(this);
			std::map<CLevelObj *,float>::iterator it;
			for (it=_affectsLichenEnemy.begin();it!=_affectsLichenEnemy.end();it++)
			{
				CLevelObj *loTarget=(*it).first;
				arg.multiply=(*it).second;
				if (entryDispel)
				{
					float dist=loTarget->GetFramePos().getDistanceFrom(posDispel);
					arg.multiply*=i_math::clamp_f((dist-radiusDispelCore)/(radiusDispel-radiusDispelCore),0.0f,1.0f);
					if (arg.multiply<=0.0f)
						continue;
				}

				MakeDeals(param->dealsLichenEnemy,osb,loTarget,arg,NULL);
			}

			for (it=_affectsLichenNative.begin();it!=_affectsLichenNative.end();it++)
			{
				CLevelObj *loTarget=(*it).first;
				arg.multiply=(*it).second;
				if (entryDispel)
				{
					float dist=loTarget->GetFramePos().getDistanceFrom(posDispel);
					arg.multiply*=i_math::clamp_f((dist-radiusDispelCore)/(radiusDispel-radiusDispelCore),0.0f,1.0f);
					if (arg.multiply<=0.0f)
						continue;
				}
				MakeDeals(param->dealsLichenNative,osb,loTarget,arg,NULL);
			}

		}

		_nLichenDeals++;
	}

	_affectsLichenEnemy.clear();
	_affectsLichenNative.clear();
}


void EoEnv::_OnUpdate()
{
	EoParamEnv *param=GetParam<EoParamEnv>();
	if (!param)
		return;

	_UpdateLichenDeal();
	_UpdateSporeDeal();

	if (_bPendingDestroy)
	{
		if(!IsDeferDestroy())
		{
			if (_lichens.empty())
			{
				if (_level)
					_level->UnRegisterEoEnv(this);
				DeferDestroy();
			}
		}
	}
}

void EoEnv::_ClearEnvOps()
{
	for (int i=0;i<_opsEnv.size();i++)
	{
		Safe_Class_Delete(_opsEnv[i]);
	}
	_opsEnv.clear();
}

void EoEnv::_SaveEnvOps(CBitPacket &bp,BOOL &bContent)
{
	if (_opsEnv.empty())
	{
		bp.Bit_Write_0();
		return;
	}

	bContent=TRUE;
	bp.Bit_Write_1();
	bp.Data_EncodeDword(_opsEnv.size());
	for(int i=0;i<_opsEnv.size();i++)
	{
		bp.Bits_Write(_opsEnv[i]->GetType(),3);
		_opsEnv[i]->Save(bp);
	}

}

void EoEnv::_SaveLichenEntry(CBitPacket &bp,LichenEntry &e)
{
	bp.Data_WriteSimple(e.handle);
	bp.Data_WriteSimpleR(e.pos);
	bp.Data_WriteSimple(e.idHost);
	bp.Data_WriteSimple(e.radius);
	bp.Data_WriteSimple(e.durFO);
	bp.Bit_Write(e.bTrail);
}

void EoEnv::_SaveLichens(CBitPacket &bp,BOOL &bContent)
{
	std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it;
	for (it=_lichens.begin();it!=_lichens.end();it++)
	{
		LichenEntry &e=(*it).second;
		if (e.IsStarted()&&e.IsStopped())
		{
			bp.Bit_Write_1();
			bContent=TRUE;

			_SaveLichenEntry(bp,e);
		}
	}
	bp.Bit_Write_0();//结束标志
}

void EoEnv::_ClearLichens()
{
	_lichens.clear();
}

void EoEnv::_OnDetroy()
{
	_ClearLichens();
	_ClearEnvOps();
}

void EoEnv::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_SaveLichens(*bp,bContent);

	_SaveEnvOps(*bp,bContent);
}

void EoEnv::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_SaveEnvOps(*bp,bContent);
}

void EoEnv::_OnPostWriteSync()
{
	_ClearEnvOps();
}

void EoEnv::RequestDestroy()
{
	if (!_bPendingDestroy)
	{
		//Stop all lichens
		std::unordered_map<EoEnvLichenHandle,LichenEntry>::iterator it;
		for (it=_lichens.begin();it!=_lichens.end();it++)
		{
			EoEnvLichenHandle h=(*it).first;
			StopLichen(h);
		}

		if (TRUE)
		{
			EoEnvOp_RequestDestroy *op=Class_New2(EoEnvOp_RequestDestroy);

			_opsEnv.push_back(op);
		}

		_bPendingDestroy=TRUE;
	}
}

void EoEnv::_ClearSpores()
{
	_mpSpores.Reset();
	_poolSpores.Reset(FALSE);
}

static float GetSporeMapCell()
{
	return 2.0f;
}

void EoEnv::SpawnSpore(LevelPos &pos,LevelOpLink &link)
{
	EoEnvSporeHandle h=_level->_GenEnvSporeHandle();

	const float wCell=GetSporeMapCell();

	int x,y;
	x=FloatToNearestInt(floor(pos.x/wCell));
	y=FloatToNearestInt(floor(pos.y/wCell));

	SporesEntry *spores=_mpSpores.Obtain(x,y);

	SporeEntry *spore=_poolSpores.Alloc();
	spore->pos=pos;
	spore->handle=h;
	spore->tStart=_level->GetT_();

	spore->next=spores->head;
	spores->head=spore;

	LevelOp_Spore *op=NewOp<LevelOp_Spore>(link);
	op->op=LevelOp_Spore::Spawn;
	op->handle=spore->handle;
	op->pos=pos;

	GetOps()->AddOp(op);
	
}

void EoEnv::DetonateSpore(LevelOSB &osb,LevelPos &pos,float radius,LevelOpLink &link)
{
	EoParamEnv *param=GetParam<EoParamEnv>();
	AnimTick tCur=_level->GetT_();

	const float wCell=GetSporeMapCell();

	i_math::rectf rc;
	rc.set(pos.x,pos.y,pos.x,pos.y);
	rc.inflate(radius);

	rc.scale_signed(wCell);
	i_math::recti rc2;
	rc2=rc.convert<int>();

	DealArg arg;
	arg.link=link;

	float radius2=radius*radius;

	for (int i=rc2.Left();i<rc2.Right();i++)
	for (int j=rc2.Top();j<rc2.Bottom();j++)
	{
		SporesEntry *spores=_mpSpores.Get(i,j);
		if (spores)
		{
			SporeEntry ** p=&spores->head;

			while(*p)
			{
				if ((*p)->pos.getDistanceSQFrom(pos)>radius2*radius2)
				{
					p=&((*p)->next);
					continue;
				}

				if (TRUE)
				{
					LevelTick tAge=ANIMTICK_SAFE_MINUS(tCur,(*p)->tStart);
					if (tAge<param->durSporeUndetonatable)
					{
						p=&((*p)->next);
						continue;
					}
				}


				SporeEntry *pSpore=(*p);
				(*p)=pSpore->next;

				if (TRUE)
				{
					LevelOp_Spore *op=NewOp<LevelOp_Spore>(link);
					op->op=LevelOp_Spore::Detonate;
					op->handle=pSpore->handle;

					GetOps()->AddOp(op);
				}

				//根据毒孢子的寿命计算伤害的比例
				if (TRUE)
				{
					AnimTick tAge=ANIMTICK_SAFE_MINUS(tCur,pSpore->tStart);
					float scaleMin=0.05f;
					float scaleMax=1.0f;

					float ratio=i_math::clamp_f(((float)tAge)/(float)param->durSporeGrow,0.0f,1.0f);
					arg.multiply=scaleMin+(scaleMax-scaleMin)*ratio;
				}

				if (TRUE)
				{
					DWORD c;
					CLevelObj **los=_DetectRange(pSpore->pos,param->radiusSporeExplode,c);
					for (int i=0;i<c;i++)
						MakeDeals(param->dealsSpore,osb,los[i],arg,NULL);
				}

				_poolSpores.Free(pSpore);
			}
		}

	}

}

void EoEnv::_UpdateSporeDeal()
{
	_temp.clear();

	EoParamEnv *param=GetParam<EoParamEnv>();

	DWORD c;
	CLevelObj **los=_DetectInAll(c);
	for (int i=0;i<c;i++)
	{
		CLevelObj *lo=los[i];
		if (lo)
			_temp.push_back(lo->GetFramePos());
	}

	LevelOpLink link;
	link.id=GetLevel()->GenOpLinkID();

	for (int i=0;i<_temp.size();i++)
	{
		DetonateSpore(LevelOSB(this),_temp[i],param->radiusSporeExplode*0.8f,link);
	}

	_temp.clear();
}

void EoEnv::SetArea(BccArea &area)
{
	_area=area;
}
