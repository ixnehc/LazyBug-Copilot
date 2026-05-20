/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Protocal.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelExploreMap.h"

#include "LevelOSB.h"

#include "BgnGA_RevealMap.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_RevealMap
BIND_BGN_CLASS(CBgnGA_RevealMap,CBgpGA_RevealMap);

void CBgnGA_RevealMap::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RevealMap*pad=_GetPad<CBgpGA_RevealMap>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level)
	{
		RecordID idMap=level->GetMapID();
		CLevelPlayer *player=_GetTalkPlayer();
		if (player)
		{
			extern LevelExploreMaps LPS_QueryExploreMaps(LevelPlayerStates *lps,RecordID idMap);
			LevelExploreMaps mps=LPS_QueryExploreMaps(player->GetLPS(),idMap);
			if (!mps.IsEmpty())
			{
				std::vector<i_math::spheref> *spheres=NULL;
				BP_Area*bcc=&pad->area;
				if (bcc)
					spheres=&bcc->sphereset;
				i_math::vector3df center3D;
				if (spheres)
				{
					if (spheres->size()>0)
					{
						for (int i=0;i<spheres->size();i++)
						{

							i_math::spheref *sph=&(*spheres)[i];

							center3D=sph->center;

							DWORD radius=(DWORD)((sph->radius+(float)EXPLOREMAP_TILE_LEN/2)/(float)EXPLOREMAP_TILE_LEN);
							if (radius<1)
								radius=1;

							mps.sttc->AddExplore(center3D,radius);
							mps.dyn->AddExplore(center3D,radius);
						}

						if (TRUE)
						{
							SCExploreMapData msg;
							msg.idMap=level->GetMapID();
							DP_BeginSave(dp,msg.dataEMs);
							mps.sttc->Save(dp);
							mps.dyn->Save(dp);
							DP_EndSave();

							level->SendNetMsg(player->GetPlayerID(),&msg);
						}
					}
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;

}
