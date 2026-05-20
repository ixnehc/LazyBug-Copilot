#include "stdh.h"

#include "Deal_Teleport.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

#include "Buff_Teleport.h"
#include "LevelUtil.h"

BIND_DEAL(Deal_Teleport);


void Deal_Teleport::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (idBuff!=RecordID_Invalid)
		{
			extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
			if (!LevelUtil_FindBuffByRecordID(loTarget,idBuff))
			{
				CLevelObj *loSrc=osbSrc.GetRootOwner();
				if (loSrc)
				{
					LevelPos posCenter=loSrc->GetFramePos();

					extern BOOL LevelUtil_FindNearbyPos(CLevel *level,LevelPos &posCenter,float radiusMin,float radiusMax,BOOL bWalkable,BOOL bReachable,DWORD nTry,LevelPos &posResult,FindNearbyPosCallBack dlgt);
					LevelPos posTeleport=posCenter;
					if (!LevelUtil_FindNearbyPos(level,posCenter,radiusMin,radiusMax,TRUE,FALSE,10,posTeleport,NULL))
					{
						LevelFace face=LevelUtil_GenRandomFace();
						LevelPos dir=LevelFaceToDir(face);

						CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
						if (unitmgr)
						{
							posTeleport=posCenter;
							unitmgr->MoveTo(UnitFindPath_Walkable,posTeleport,dir*radiusMin);
						}
					}

					BuffArg_Teleport argTeleport;
					argTeleport.pos=posTeleport;
					argTeleport.face=LevelUtil_GenRandomFace();
					level->GetDecider()->MakeBuff(osbSrc,loTarget,idBuff,0,&argTeleport,arg.link);
				}
			}
		}
	}

}
