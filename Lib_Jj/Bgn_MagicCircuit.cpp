/********************************************************************
	created:	2023/11/26 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_MagicCircuit.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoMagicCircuit.h"


////////////////////////////////////////////////////////////////////////
//CBgn_MagicCircuitOp
BIND_BGN_CLASS(CBgn_MagicCircuitOp,CBgp_MagicCircuitOp);

void CBgn_MagicCircuitOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MagicCircuitOp*pad=_GetPad<CBgp_MagicCircuitOp>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLoMagicCircuit *loMagicCircuit=(CLoMagicCircuit *)level->GetUniqueObj(LevelUniqueObj_MagicCircuit);
	if (loMagicCircuit)
	{
		if (pad->mode==0)
		{
			loMagicCircuit->ActivateRelay(lo->GetID(),TRUE);
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==1)
		{
			loMagicCircuit->ActivateRelay(lo->GetID(),FALSE);
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==2)
		{
			LevelObjID idCrystal=loMagicCircuit->SpawnCrystal();
			if (pad->nmVarObj)
				_SetID(pad->nmVarObj,BehaviorMemType_ObjID,idCrystal);
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==3)
		{
			LevelObjID id=loMagicCircuit->SpawnRelayBird();
			if (pad->nmVarObj)
				_SetID(pad->nmVarObj,BehaviorMemType_ObjID,id);
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==4)
		{
			LevelPos posTarget;
			if (loMagicCircuit->GetRelayBirdTargetPos(lo->GetFramePos(),posTarget))
			{
				if (pad->nmVarPos)
					_SetPos(pad->nmVarPos,posTarget);
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==5)
		{
			if (loMagicCircuit->CheckRelayBirdAtHome(lo->GetFramePos()))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==15)
		{
			if (loMagicCircuit->CheckRelayBirdAtRest(lo->GetFramePos()))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==6)
		{
			loMagicCircuit->ActivateFocus();
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==7)
		{
			if (loMagicCircuit->CheckFocus())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==8)
		{
			loMagicCircuit->CommitFocus();
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==9)
		{
			if (loMagicCircuit->CanTailOrbsReach())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==10)
		{
			loMagicCircuit->StartTailOrbsReach();
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==11)
		{
			if (loMagicCircuit->CheckTailOrbsReached())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==12)
		{
			LevelObjID idOrbsHome=loMagicCircuit->GetTailOrbsHome();
			if (pad->nmVarObj)
				_SetID(pad->nmVarObj,BehaviorMemType_ObjID,idOrbsHome);
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==13)
		{
			if (loMagicCircuit->CanSpawnCrystal())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==14)
		{
			if (loMagicCircuit->CanActivateFocus())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode == 16)
		{
			loMagicCircuit->SpawnRailGuards();
			_OutputOk(outputs, 1, "成功");
			return;
		}
		if (pad->mode == 17)
		{
			loMagicCircuit->DespawnRailGuards();
			_OutputOk(outputs, 1, "成功");
			return;
		}

	}
	_OutputFail(outputs,2,"失败");
}

