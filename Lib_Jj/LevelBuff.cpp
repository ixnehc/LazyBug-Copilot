/********************************************************************
	created:	2012/01/02
	file base:	Buff
	author:		cxi
	
	purpose:	LevelBuff
*********************************************************************/

#include "stdh.h"

#include "LevelBuff.h"

#include "Level.h"

#include "LevelRecordBuff.h"
#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorMem.h"

#include "Log/LogDump.h"

#include "LevelUtil.h"

//////////////////////////////////////////////////////////////////////////
//LevelBuffData
void LevelBuffData::Save(CBitPacket *bp)
{
	bp->Bits_Write(bClassUID,1);
	bp->Data_WriteSimple(id);
	bp->Data_WriteSimple(uid);
	bp->Data_WriteSimple(dur);
	bp->Bits_Write(szData,5);
	bp->Data_WriteData(data,szData);
	bp->Bits_Write(szBitsData,5);
	bp->Data_WriteData(bits,szBitsData);
}

void LevelBuffData::Load(CBitPacket *bp)
{
	bClassUID=bp->Bit_Read();
	bp->Data_ReadSimple(id);
	bp->Data_ReadSimple(uid);
	bp->Data_ReadSimple(dur);
	szData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(data,szData);
	szBitsData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(bits,szBitsData);
}



//////////////////////////////////////////////////////////////////////////
//CLevelBuff

CLevelOp *CLevelBuff::NewOp(CClass *clssOp,LevelOpLink &link)
{
	extern CLevelOp*NewLevelOp(ClassUID uid);
	CLevelOp *op=NewLevelOp(clssOp->GetUID());
	if (!op)
		return NULL;

	LevelOpDesc &desc=op->GetDesc();
	desc.uid=(WORD)clssOp->GetUID();
	desc.idOwner=_id;
	desc.tpOwner=LevelOpDesc::Buff;
	desc.link=link;

	return op;
}

void CLevelBuff::ToData(LevelBuffData &data)
{
	data.id=_id;
	if (_rec)
	{
		data.bClassUID=0;
		data.uid=(WORD)_rec->GetSimpleID();
	}
	else
	{
		data.bClassUID=1;
		data.uid=(WORD)GetClass()->GetUID();
	}
	if (_dur!=ANIMTICK_INFINITE)
		data.dur=(WORD)(_dur*10/ANIMTICK_PER_SECOND);
	else
		data.dur=0xffff;

	CBitPacket bp;
	bp.SetBufferPointer(data.data,data.bits);
	_WriteData(&bp);
	DWORD szData,szBitsInByte;
	bp.GetBufferSize(szData,szBitsInByte);
	assert(szData<=MAX_BUFF_DATA);
	assert(szBitsInByte<=MAX_BUFF_DATA);
	data.szData=(BYTE)szData;
	data.szBitsData=(BYTE)szBitsInByte;
}


void CLevelBuff::Create(CLevelBuffs *buffs,LevelBuffID id,LevelRecordBuff *rec,AnimTick dur,LevelBuffArg *arg)
{
	_buffs=buffs;
	_id=id;
	SAFE_REPLACE(_rec,rec);
	if (_rec)
		_SetParam(_rec->param);
	_dur=dur;

	BuffFlag flag=GetFlags();
	if (flag!=0)
		buffs->AddFlag(flag);

	_tUpdate=buffs->GetOwner()->GetT();
	_tAge=0;

	//创建Behavior
	if (_rec)
	{
		if (_rec->idBG!=StringID_Invalid)
		{
			LevelBehaviorContext ctx;
			ctx.lo=_GetOwner();
			_bhv=_GetLevel()->CreateBehavior(_rec->idBG,ctx);
		}
	}

	if (_rec)
	{
		if (_rec->paramsReactor.size()>0)
		{
			_reactors=Class_New2(CLevelReactors);
			_reactors->Init(this);
		}
	}

	_OnCreate(arg);

	AddRef();
}

void CLevelBuff::Create_Teleport(CLevelBuffs *buffs,LevelBuffID id,LevelRecordBuff *rec,AnimTick dur,CLevelBuff *buffOrg)
{
	_buffs=buffs;
	_id=id;
	SAFE_REPLACE(_rec,rec);
	if (_rec)
		_SetParam(_rec->param);
	_dur=dur;

	BuffFlag flag=GetFlags();
	if (flag!=0)
		buffs->AddFlag(flag);

	_tUpdate=buffs->GetOwner()->GetT();
	_tAge=0;

	//把buffOrg的Behavior拿过来
	if (buffOrg->_bhv)
	{
		_bhv=buffOrg->_bhv;
		LevelBehaviorContext *ctx=_bhv->GetContext();
		ctx->lo=_GetOwner();
		buffOrg->_bhv=NULL;
	}

	//把buffOrg的reactors拿过来
	if (buffOrg->_reactors)
	{
		_reactors=buffOrg->_reactors;
		_reactors->SetOwner(this);
		buffOrg->_reactors=NULL;
	}

	LoadTeleport(buffOrg);

	AddRef();
}


void CLevelBuff::Destroy()
{
	if (IsAlive())
	{
		if (_bhv)
		{
			_bhv->Clear();
			Safe_Class_Delete(_bhv);
		}
		if (_reactors)
		{
			_reactors->Clear();
			Safe_Class_Delete(_reactors);
		}
		BuffFlag flag=GetFlags();

		_OnDestroy();

		_buffs->GetIDPool()->Free(_id);

		SAFE_RELEASE(_rec);

		if (flag!=0)
			_buffs->MarkFlagsDirty();

		Zero();
	}

	Release();

}

CLevelBuff::ConflictResult CLevelBuff::CheckConflict(CLevelBuff *buffExist)
{
	LevelBuffMask maskExist=(((LevelBuffMask)1)<<buffExist->GetClass()->GetUID());
	if (GetForbiddingBuffs()&maskExist)
		return Conflict_Forbid;
	if (GetReplaceBuffs()&maskExist)
		return Conflict_Replace;

	return Conflict_None;
}

void CLevelBuff::MergeDur(AnimTick durNew)
{
	if (durNew==ANIMTICK_INFINITE)
		_dur=ANIMTICK_INFINITE;
	if (_dur!=ANIMTICK_INFINITE)
	{
		if (durNew>_dur)
			_dur=durNew;
	}
}


void CLevelBuff::Update(AnimTick t)
{
	AnimTick dt=t-_tUpdate;
	_tUpdate=t;
	_tAge+=dt;
	if (_dur==ANIMTICK_INFINITE)
	{
		_OnUpdate(dt);
		if (_bhv)	
			_bhv->Update();
		if (_reactors)
			_reactors->Update(t);
	}
	else
	{
		if (_dur>dt)
		{
			_dur-=dt;
			_OnUpdate(dt);
			if (_bhv)
				_bhv->Update();
			if (_reactors)
				_reactors->Update(t);
		}
		else
		{
			//时间到,Destroy自己
			if (NeedSyncTimeUp())
			{
				CLevelOp *op=NewOp<LevelOp_BuffTimeUp>();
				_GetOwner()->AddOp(op);
			}
			else
			{
				if (NeedSyncTimeUpIP())
				{
					if (_GetOwner()->IsPlayer())
					{
						CLevelOp *op=NewOp<LevelOp_BuffTimeUp>();
						_GetOwner()->AddOp(op);
					}
				}
			}
			_dur=0;
			AddRef();
			Destroy();
		}
	}
}

CLevelObj*CLevelBuff::_GetOwner()
{
	if (_buffs)
		return _buffs->GetOwner();
	return NULL;
}

CLevel*CLevelBuff::_GetLevel()
{
	CLevelObj *owner=_GetOwner();
	if (owner)
		return owner->GetLevel();
	return NULL;
}


void CLevelBuff::_AddSyncDataOp()
{
	LevelOp_SyncBuffData*op=NewOp<LevelOp_SyncBuffData>();

	CBitPacket bp;
	bp.SetBufferPointer(op->data,op->bits);
	_WriteData(&bp);
	DWORD szData,szBitsInByte;
	bp.GetBufferSize(szData,szBitsInByte);
	assert(szData<=MAX_BUFF_DATA);
	assert(szBitsInByte<=MAX_BUFF_DATA);
	op->szData=(BYTE)szData;
	op->szBitsData=(BYTE)szBitsInByte;

	_GetOwner()->AddOp(op);

}

CBehaviorMem *CLevelBuff::GetMem()
{
	if (!_bhv)
		return NULL;
	CBehaviorMem *mem=_bhv->GetMem(0);
	if (!mem)
		return NULL;
	return mem;
}

void CLevelBuff::HandleEvent(LevelEvent &e)
{
	if (_reactors)
		_reactors->HandleEvent(e);
}

BOOL CLevelBuff::CanTeleport()
{
	if (!_rec)
		return FALSE;
	return _rec->bCanTeleport;
}


void CLevelBuff::_MakeDeals(LevelPos3D &pos,DealArg&arg)
{
	LevelOSB osbSrc(this);
	if (_rec)
	{
		_rec->deal->Make(osbSrc,pos,arg,NULL);
		MakeDeals(_rec->deals,osbSrc,pos,arg,NULL);
	}
}

void CLevelBuff::_MakeDeals(CLevelObj *loTarget,DealArg&arg)
{
	LevelOSB osbSrc(this);
	if (_rec)
	{
		if (_GetLevel()->GetDecider()->MakeHit(osbSrc,loTarget,_rec->hit.Get(),arg.link))
		{
			_rec->deal->Make(osbSrc,loTarget,arg,NULL);
			MakeDeals(_rec->deals,osbSrc,loTarget,arg,NULL);
		}
	}
}


CLevelObj **CLevelBuff::_DetectRange(LevelPos &pos,float radius,DWORD &c)
{
	c=0;
	if (_GetOwner())
	{
		if (_rec)
		{
			LevelUtilDetectParam paramDetect;
			paramDetect.loSrc=_GetOwner();
			paramDetect.pos=pos;
			paramDetect.rangeMin=0.0f;
			paramDetect.rangeMax=radius;
			paramDetect.flags=&_rec->detects[0];
			paramDetect.nFlags=_rec->detects.size();
			paramDetect.requires=&_rec->requires[0];
			paramDetect.nRequires=_rec->requires.size();

			return LevelUtil_Detect(paramDetect,NULL,c);
		}
	}

	return NULL;
}


void CLevelBuff::_MakeRangeDeals(float radius)
{
	if (!_GetOwner())
		return;
	DWORD c;
	CLevelObj **los=_DetectRange(_GetOwner()->GetFramePos(),radius,c);

	for (int i=0;i<c;i++)
	{
		CLevelObj *loTarget=los[i];
		LevelPos dir=loTarget->GetFramePos()-_GetOwner()->GetFramePos();

		DealArg arg;
		arg.dir.setXZ(dir.safe_normalize());
		arg.link.id=_GetLevel()->GenOpLinkID();
		arg.grd=0;

		_MakeDeals(loTarget,arg);
	}
}




//////////////////////////////////////////////////////////////////////////
//CBuffIDPool
LevelBuffID CLevelBuffIDPool::Alloc()
{
	LevelBuffID ret;
	if (_frees.size())
	{
		if (_frees[0].t+ANIMTICK_FROM_SECOND(20.0f)<_level->GetT_())
		{
			ret=_frees[0].id;
			_frees.pop_front();
			return ret;
		}
	}
	ret=_seed;
	_seed++;
	return ret;
}

void CLevelBuffIDPool::Free(LevelBuffID id)
{
	FreeNode node;
	node.t=_level->GetT_();
	node.id=id;
	_frees.push_back(node);
}



//////////////////////////////////////////////////////////////////////////
//CBuffs
void CLevelBuffs::Init(CLevelObj*owner,CLevelBuffIDPool *idpool)
{
	CBuffFormular::SetBuffs((std::vector<CBuffFactor*>*)&_buffs);
	_idpool=idpool;
	_owner=owner;
}

void CLevelBuffs::Init_Teleport(CLevelObj*owner,CLevelBuffIDPool *idpool,CLevelBuffs *buffs)
{
	Init(owner,idpool);

	DWORD c;
	CLevelBuff **buf=buffs->GetBuffs(c);
	for (int i=0;i<c;i++)
	{
		CLevelBuff *buff=buf[i];

		CLevelBuff *buffNew=CreateBuff_Teleport(buff);
		SAFE_RELEASE(buffNew);
	}

}


void CLevelBuffs::Clear()
{
	for (int i=0;i<_buffs.size();i++)
		SAFE_DESTROY(_buffs[i]);
	_buffs.clear();

	Zero();
}

class CLevelBuff;
CLevelBuff*NewLevelBuff(LevelRecordBuff *rec)
{
	if (!rec->param)
		return NULL;
	CClass *clss=rec->param->GetBuffClass();
	return clss?(CLevelBuff*)clss->New():NULL;
}

CLevelBuff*NewLevelBuff(CClass*clss)
{
	return clss?(CLevelBuff*)clss->New():NULL;
}



CLevelBuff *CLevelBuffs::CreateBuff(LevelRecordBuff *rec,AnimTick dur,LevelBuffArg *arg)
{
	CLevelBuff *buff=NewLevelBuff(rec);
	if (!buff)
	{
		LOG_DUMP_1P("CLevelBuffs",Log_Error,"无效的Buff:%s",rec->Name.c_str());
		return NULL;
	}

	if (arg)
	{
		if (rec->param)
		{
			if (!rec->param->GetArgClass()->IsSameWith(arg->GetClass()))
			{
				LOG_DUMP_2P("CLevelBuffs",Log_Error,"Buff的参数不匹配:使用%s创建%s",arg->GetClass()->GetName(),buff->GetClass()->GetName());
				Safe_Class_Delete(buff);
				return NULL;
			}
		}
	}


	LevelBuffID id=_idpool->Alloc();
	buff->Create(this,id,rec,dur,arg);
	_buffs.push_back(buff);
	MarkFlagsDirty();
	buff->AddRef();
	return buff;
}

CLevelBuff *CLevelBuffs::CreateBuff(CClass *clss,AnimTick dur,LevelBuffArg *arg)
{
	CLevelBuff *buff=NewLevelBuff(clss);

	LevelBuffID id=_idpool->Alloc();
	buff->Create(this,id,NULL,dur,arg);
	_buffs.push_back(buff);
	MarkFlagsDirty();
	buff->AddRef();
	return buff;
}

CLevelBuff *CLevelBuffs::CreateBuff_Teleport(CLevelBuff *buffOrg)
{
	if (!buffOrg->CanTeleport())
		return NULL;
	CLevelBuff *buff=NULL;
	if (buffOrg->GetRec())
		buff=NewLevelBuff(buffOrg->GetRec());
	else
		buff=NewLevelBuff(buffOrg->GetClass());
	if (!buff)
		return NULL;

	LevelBuffID id=_idpool->Alloc();
	buff->Create_Teleport(this,id,buffOrg->GetRec(),buffOrg->GetDur(),buffOrg);
	_buffs.push_back(buff);
	MarkFlagsDirty();
	buff->AddRef();
	return buff;
}



void CLevelBuffs::_FlushDead()
{
	DWORD c=0;
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *p=_buffs[i];
		if (p->IsAlive())
		{
			_buffs[c]=_buffs[i];
			c++;
		}
		else
			SAFE_RELEASE(p);
	}
	_buffs.resize(c);
	_bNeedFlushDead=FALSE;
}

void CLevelBuffs::Update(AnimTick t)
{
	if (_bNeedFlushDead)
		_FlushDead();

	int nDead=0;
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *p=_buffs[i];
		if (!p->IsAlive())
		{
			nDead++;
			continue;
		}

		p->Update(t);
	}

	if (nDead*3>_buffs.size())//超过1/3的buff已经dead了
		_bNeedFlushDead=TRUE;
}

void CLevelBuffs::_WriteBigData(CLevelBuff *buff,CBitPacket *bp)
{
	if (buff->NeedSyncBigData())
	{
		bp->Bit_Write_1();

		LevelBuffID id= buff->GetID();
		bp->Data_WriteSimple(id);

		CDataPacket *dp=bp->GetDP();
		DP_PreSafeSave(*dp);
		buff->_WriteBigData(dp);
		DP_PostSafeSave();
	}
	else
		bp->Bit_Write_0();

}

void CLevelBuffs::WriteFirstSync(CBitPacket *bp)
{
	LevelBuffData data;
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *p=_buffs[i];
		if (!p->IsAlive())
			continue;
		if (!p->NeedSync())
			continue;

		bp->Bit_Write_1();

		//某些Buff会通过传递一个Op来同步,是因为这样可以通过op的LinkID来进行联动播放
		//(就是说这个Buff会由另一个Op播放时联动的加到LevelObj上,典型的用于一个召唤生物的技能,
		//生物在召唤出来时会有一个Buff_Birth,这个Buff将由技能来决定什么时候可以加到生物身上)
		LevelOp_AddBuff *op=p->AccuireSyncOp();
		if (op)
		{
			bp->Bit_Write_0();
			op->GetDesc().Save(bp);
			op->Save(bp);
		}
		else
		{
			bp->Bit_Write_1();
			p->ToData(data);
			data.Save(bp);
		}

		_WriteBigData(p,bp);

		CBehaviorMem *mem=p->GetMem();
		if (mem)
		{
			bp->Bit_Write_1();
			LevelBuffID id= p->GetID();
			bp->Data_WriteSimple(id);
			mem->SaveSync(bp->GetDP());
		}
		else
		{
			bp->Bit_Write_0();
		}
	}
	bp->Bit_Write_0();//终止标志
}

void CLevelBuffs::WriteSyncL(CBitPacket *bp,BOOL &bContent)
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *p=_buffs[i];
		if (!p->IsAlive())
			continue;
		if (!p->NeedSync())
			continue;

		bp->Bit_Write_1();

		_WriteBigData(p,bp);

		CBehaviorMem *mem=p->GetMem();

		if (mem)
		{	
			if (!mem->IsSyncDirty())
				mem=NULL;
		}
		if (mem)
		{
			bContent=1;
			bp->Bit_Write_1();
			LevelBuffID id= p->GetID();
			bp->Data_WriteSimple(id);
			mem->SaveSync(bp->GetDP());
		}
		else
			bp->Bit_Write_0();
	}
	bp->Bit_Write_0();//终止标志
}

void CLevelBuffs::PostWriteSync()
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *p=_buffs[i];
		if (!p->IsAlive())
			continue;
		if (!p->NeedSync())
			continue;

		CBehaviorMem *mem=p->GetMem();
		if (mem)
			mem->ClearSyncDirty();
	}
}




void CLevelBuffs::HandleEvent(LevelEvent &e)
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *buff=_buffs[i];
		if (!buff->IsAlive())
			continue;
		buff->HandleEvent(e);
		if (e.bHandled)
			return;
	}
}

CLevelBuff *CLevelBuffs::FindBuffByID(LevelBuffID idBuff)
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *buff=_buffs[i];
		if (!buff->IsAlive())
			continue;
		if (buff->GetID()==idBuff)
			return buff;
	}
	return NULL;
}


CLevelBuff *CLevelBuffs::FindBuff(CClass *clssBuff)
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *buff=_buffs[i];
		if (!buff->IsAlive())
			continue;
		if (buff->GetClass()->IsSameWith(clssBuff))
			return buff;
	}
	return NULL;
}

CLevelBuff *CLevelBuffs::FindBuffByRecordID(RecordID idBuff)
{
	for (int i=0;i<_buffs.size();i++)
	{
		CLevelBuff *buff=_buffs[i];
		if (!buff->IsAlive())
			continue;
		if (buff->_rec)
		{
			if (buff->_rec->GetID()==idBuff)
				return buff;
		}
	}
	return NULL;
}

