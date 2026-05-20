/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "BgnGA_RegisterTeleportTarget.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_RegisterTeleportTarget
BIND_BGN_CLASS(CBgnGA_RegisterTeleportTarget,CBgpGA_RegisterTeleportTarget);

void CBgnGA_RegisterTeleportTarget::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RegisterTeleportTarget*pad=_GetPad<CBgpGA_RegisterTeleportTarget>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level)
	{
		LevelTeleportTarget target;
		target.tp=pad->tpTarget;
		target.idMap=level->GetMapID();
		target.posSite=lo->GetFramePos();

		if (lo->GetType()==LevelObjType_Agent)
		{
			i_math::matrix43f *mat=((CLoAgent*)lo)->GetMat();
			if (mat)
			{
				i_math::vector3df pos(pad->xOff,0.0f,pad->zOff);
				mat->transformVect(pos,pos);
				target.posSite=pos.getXZ();
			}
		}
		level->GetWorld()->RegisterTeleportTarget(target);
	}

	_OutputOk(outputs,1,"结束");
	return;

}
