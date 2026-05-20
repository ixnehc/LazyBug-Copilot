
#include "stdh.h"

#include "commondefines/general_stl.h"
 
#include "Level.h"
#include "LevelRecords.h"

#include "LoMagicBoard.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"

#include "Random/Random.h"

#include "LevelRecordMagicTile.h"

#include "LevelBehavior.h"

#include "Buff_Birth.h"


#include "Protocal.h"



void CMagicBoardAI::Init(CLoMagicBoard *loMB)
{
	_ctx.level=loMB->GetLevel();
	_ctx.lo=loMB;
	_ctx.idPlayer=MBUtil_GetAIPlayer();

	_ctx.attr.Init(NULL);

}

void CMagicBoardAI::Clear()
{
	if (_bhv)
	{
		_bhv->Clear();
		Safe_Class_Delete(_bhv);
	}

}

void CMagicBoardAI::Update()
{
	//初始化AI
	if (!_bhv)
	{
		if (_ctx.lo)
		{
			LosMagicBoard *los=(LosMagicBoard *)_ctx.lo->GetLos();
			if (los)
			{
				if(_ctx.level)
				{
					LevelRecordMagicBoard *rec=_ctx.level->GetRecords()->GetMagicBoard(los->idBoard);
					if (rec)
					{
						if (rec->nmBg!=StringID_Invalid)
						{
							LevelBehaviorContext ctx;
							ctx.lo=_ctx.lo;
							ctx.ctxMB=&_ctx;
							_bhv=_ctx.level->CreateBehavior(rec->nmBg,ctx);
							_bhv->Start();
							return;
						}
					}
				}
			}
		}
	}

	if (_bhv)
		_bhv->Update();

// 
// 
// 
// 	extern int GenPrimeStep();
// 	int step=GenPrimeStep();
// 
// 	int nTotal=_ctx.unseals.size()+_ctx.seals.size();
// 	if (nTotal>0)
// 	{
// 		int idx=0;
// 		for (int i=0;i<nTotal;i++)
// 		{
// 			idx=(idx+step)%nTotal;
// 
// 			DWORD idxTile;
// 			if (idx<_ctx.seals.size())
// 				idxTile=(DWORD)_ctx.seals[idx];
// 			else
// 				idxTile=(DWORD)_ctx.unseals[idx-_ctx.seals.size()];
// 
// 			MagicTileInfo *ti=_ctx.lo->GetTileInfo(idxTile);
// 			if (ti)
// 			{
// 				if (MBUtil_CheckTileReady(_ctx.level,_ctx.idPlayer,ti))
// 				{
// 					MagicBoardInvoke invoke;
// 					invoke.id=_ctx.lo->GetID();
// 					invoke.x=ti->x;
// 					invoke.y=ti->y;
// 					_ctx.lo->Invoke(_ctx.idPlayer,invoke);
// 					break;
// 				}
// 			}
// 		}
// 	}

}
