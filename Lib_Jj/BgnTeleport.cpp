/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnTeleport.h"

#include "LevelObj.h"

#include "Level.h"
#include "LevelRecordGlobal.h"
#include "LevelRecords.h"

#include "Buff_Teleport.h"


  

////////////////////////////////////////////////////////////////////////
//CBgn_Teleport

BIND_BGN_CLASS(CBgn_Teleport,CBgp_Teleport);

RecordID CBgn_Teleport::_GetBuffRecID()
{
	RecordID idBuff=RecordID_Invalid;
	CBgp_Teleport*pad=_GetPad<CBgp_Teleport>();
	if (pad)
		idBuff=pad->_idBuff;

	if (idBuff==RecordID_Invalid)
	{
		LevelBehaviorContext *ctx=_GetCtx();
		LevelRecordGlobal *recGlobal=ctx->level->GetRecords()->GetGlobal();
		if (recGlobal)
			idBuff=recGlobal->idDefBuff_Teleport;
	}

	return idBuff;
}

void CBgn_Teleport::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Teleport*pad=_GetPad<CBgp_Teleport>();
	if (pad)
	{
		LevelBehaviorContext *ctx=_GetCtx();
		CLevelObj *lo=ctx->lo;
		if (lo)
		{
			BOOL bPos=FALSE;
			LevelPos pos;

			BOOL bFace=TRUE;
			float face=0.0f;

			if (pad->_pos)
				bPos=_GetPos(pad->_pos,pos);
			else
			{
				if (pad->_loc)
				{
					bPos=_GetLocXfm(pad->_loc,pos,face);
				}
				else
					bPos=_GetRouteStartPos(pad->_route,pos,face);
			}

			if (TRUE)
			{
				if (pad->_modeFacing==0)
				{
					extern float LevelUtil_GenRandomFace();
					face=LevelUtil_GenRandomFace();//随机角度
				}
				if (pad->_modeFacing==1)
				{
					if (pad->_varFace!=StringID_Invalid)
						_GetFloat(pad->_varFace,face);
				}
			}

			if (bPos&&bFace)
			{
				RecordID idBuff=_GetBuffRecID();
				if (idBuff!=RecordID_Invalid)
				{
					BuffArg_Teleport arg;
					arg.pos=pos;
					arg.face=face;

					AnimTick dur=pad->_dur;
					if (dur<=0)
						dur=ANIMTICK_INFINITE;
					_idBuff=ctx->level->GetDecider()->MakeBuff(lo,idBuff,dur,&arg,TRUE);
				}
			}
		}
	}

	if (pad->_bWaitFinish)
	{
		if (_idBuff==LevelBuffID_Invalid)
			_OutputOk(outputs,1,"结束");
	}
	else
		_OutputOk(outputs,1,"结束");
}

void CBgn_Teleport::Update(BGNOutputs &outputs)
{
	CBgp_Teleport*pad=_GetPad<CBgp_Teleport>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevelObj *lo=ctx->lo;

	if (pad->_dur>0)
	{
		extern CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff);
		if (!LevelUtil_FindBuffByID(lo,_idBuff))
			_OutputOk(outputs,1,"结束");
	}
}
