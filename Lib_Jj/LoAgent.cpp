
#include "stdh.h"

#include "LevelObjSrc.h"
#include "LoAgent.h"

#include "LevelOp.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordAgent.h"

//////////////////////////////////////////////////////////////////////////
//AgentSrc

void AgentSrc::Write(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(mat);
	bp->Data_WriteSimple(rec->GetID());
}


//////////////////////////////////////////////////////////////////////////
//CLevelAgent
void CLoAgent::PostCreate()
{
	if (_src)
	{
		_pos.x=_src->GetMat().getTranslationP()->x;
		_pos.y=_src->GetMat().getTranslationP()->z;
	}

	if (_param)
		_idPlayer=_param->GetPlayerID();
	else
		_idPlayer=LevelPlayerID_NeutralWild;
}

void CLoAgent::PostCreate(i_math::matrix43f &mat,RecordID idRec,LevelPlayerID idPlayer)
{
	if (!_srcA)
		_srcA=Class_New2(AgentSrc);

	_srcA->mat=mat;
	_srcA->rec=_level->GetRecords()->GetAgent(idRec);

	_pos=mat.getTranslation().getXZ();

	_idPlayer=idPlayer;

	//´´½¨Unit
	if (TRUE)
	{
		CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
		_unitGround=unitmgr->CreateUnit(_pos,0.0f,GetRadius_(),0.0f,NULL);

		//UnitCollide
		if (TRUE)
		{
			extern void UnitCollide_SetAlly(UnitCollide &collide,DWORD ally);
			extern void UnitCollide_SetPlayer(UnitCollide &collide,BOOL bPlayer);
			extern void UnitCollide_SetLayor(UnitCollide &collide,DWORD layor);

			UnitCollide collide=UnitCollide_Empty;

			UnitCollide_SetAlly(collide,GetPlayerID());
			UnitCollide_SetPlayer(collide,IsPlayer());
			if (_srcA->rec)
				UnitCollide_SetLayor(collide,_srcA->rec->layorCollide);
			else
				UnitCollide_SetLayor(collide,0xffffffff);

			_unitGround->SetCollide(collide);
		}
		_unitGround->SetData((void*)this);
	}
}


void CLoAgent::PostCreate(LevelPos &pos,float rad,RecordID idRec,LevelPlayerID idPlayer)
{
	i_math::matrix43f mat;

	extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
	LevelPos3D pos3D=LevelUtil_GetGroundHeight(_level,pos.x,pos.y,FALSE);
	mat.setRotationAxis(i_math::vector3df(0,1,0),rad);
	*mat.getTranslationP()=pos3D;

	PostCreate(mat,idRec,idPlayer);
}

void CLoAgent::OnDestroy()	
{
	SAFE_DESTROY(_unitGround);
	Safe_Class_Delete(_srcA);
}

LevelPos3D CLoAgent::GetFramePos3D()
{
	if (_srcA)
		return _srcA->mat.getTranslation();
	if (_src)
		return _src->GetMat().getTranslation();
	return CLevelObj::GetFramePos3D();
}


BOOL CLoAgent::IsServerOnly()
{
	LevelRecordAgent *rec=_GetRec();
	if (rec)
		return rec->bServerOnly;
	return FALSE;
}



void CLoAgent::WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_srcA)
	{
		bp->Bit_Write_1();
		_srcA->Write(bp);
		bContent=TRUE;
	}
	else
		bp->Bit_Write_0();

	bp->Bits_Write(_idPlayer,5);
	_ops.WriteSync(bp,TRUE,bContent);
	_OnWriteFirstSync(bp,bContent,idPlayer);
}
void CLoAgent::WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_OnWriteSyncH(bp,bContent,idPlayer);
}

void CLoAgent::WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_ops.WriteSync(bp,FALSE,bContent);
	_OnWriteSyncL(bp,bContent,idPlayer);
}

void CLoAgent::PostWriteSync()
{
	_ops.PostWriteSync();
	_OnPostWriteSync();
}


float CLoAgent::GetInvokeRange()
{
	CLevelObjSrc *los=GetLos();
	if (los)
	{
		LevelRecordAgent *rec=los->GetRecord();
		if (rec)
			return rec->InvokeRange;
	}
	return 0.0f;
}

DWORD CLoAgent::GetBuffID_Dead()
{
	CLevelObjSrc *los=GetLos();
	if (los)
	{
		LevelRecordAgent *rec=los->GetRecord();
		if (rec)
			return rec->buffDead;
	}
	return 0;
}


LevelRecordAgent *CLoAgent::_GetRec()
{
	if (_srcA)
		return _srcA->rec;
	CLevelObjSrc *los=GetLos();
	if (los)
		return los->GetRecord();
	return NULL;
}

LevelGUID CLoAgent::_GetGUID()
{
	if (_srcA)
		return LevelGUID_Invalid;
	CLevelObjSrc *los=GetLos();
	if (los)
		return los->GetGUID();
	return LevelGUID_Invalid;
}

i_math::matrix43f *CLoAgent::_GetMat()
{
	if (_srcA)
		return &_srcA->mat;
	CLevelObjSrc *los=GetLos();
	if (los)
		return &los->GetMat();
	return NULL;
}

float CLoAgent::GetRadius_()
{
	LevelRecordAgent *rec=_GetRec();
	if (!rec)
		return 0.0f;
	return rec->Radius;
}

float CLoAgent::GetHeight()
{
	LevelRecordAgent *rec=_GetRec();
	if (!rec)
		return 0.0f;
	return rec->Height;
}


LoMiscFlags*CLoAgent::GetMiscFlags()
{
	LevelRecordAgent *rec=_GetRec();
	if (rec)
		return &rec->flagsMisc;

	return NULL;
}
