/********************************************************************
	created:	2022/3/5 
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelRecordAgent.h"


#include "BgnGA_Brief.h"

#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_Brief
BIND_BGN_CLASS(CBgnGA_Brief,CBgpGA_Brief);


void CBgnGA_Brief::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Brief*pad=_GetPad<CBgpGA_Brief>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelPlayer *player=_GetTalkPlayer();
	if (!player)
	{
		extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
		player=LevelUtil_GetFirstPlayer(level);
	}
	

	if (player->GetLPS())
	{
		LevelGUID guid=ctx->lo->GetGUID();
		if (guid!=LevelGUID_Invalid)
		{
			BOOL bNewEntry=TRUE;
			if (LPS_FindPersistEntry_AgentBrief(player->GetLPS(),level->GetMapID(),guid))
				bNewEntry=FALSE;
			BOOL bModified=FALSE;
			LevelAgentBrief *brief=LPS_QueryPersistEntry_AgentBrief(player->GetLPS(),level->GetMapID(),guid);
			if(brief)
			{
				if (bNewEntry)
					bModified=TRUE;
				if (bNewEntry)
				{
					if (ctx->lo->GetType()==LevelObjType_Agent)
						brief->pos=((CLoAgent*)ctx->lo)->GetBriefCenter();
					else
						brief->pos=ctx->lo->GetFramePos3D();
					extern RecordID LevelUtil_GetAgentRecID(CLevelObj *lo);
					brief->idAgent=LevelUtil_GetAgentRecID(ctx->lo);

					brief->initial.tp=LevelAgentBrief::Default;
				}

				LevelAgentBrief::Brief *brBase=&brief->initial;
				if (!pad->bInitial)
				{
					if(!brief->cur.IsEmpty())
						brBase=&brief->cur;
				}
				LevelAgentBrief::Brief br;
				br.CopyFrom(*brBase);

				//作实质的修改
				switch(pad->op)
				{
					case 2:
					{
						br.tp=pad->tp;
						break;
					}
					case 0:
					case 3:
					{
						BOOL bNeedMod=FALSE;
						if (pad->op==0)
							bNeedMod=TRUE;
						else
						{
							if (!br.rcOnIconTex.isValid())
								bNeedMod=TRUE;
						}
						if (bNeedMod)
						{
							char buf[256];
							strcpy(buf,pad->pathIcon.c_str());
							char *p=buf;
							char *str=SeperateStringBy(',',p);
							if (strcmp(str,AgentBriefIcon_TexPath)==0)
							{
								br.rcOnIconTex.Left()=(short)IntFromString(SeperateStringBy(',',p));
								br.rcOnIconTex.Top()=(short)IntFromString(SeperateStringBy(',',p));
								br.rcOnIconTex.Right()=(short)IntFromString(SeperateStringBy(',',p));
								br.rcOnIconTex.Bottom()=(short)IntFromString(p);
								br.ptIconAnchor.x=i_math::clamp_i(FloatToNearestInt(pad->ptIconAnchor.x*100.0f),0,100);
								br.ptIconAnchor.y=i_math::clamp_i(FloatToNearestInt(pad->ptIconAnchor.y*100.0f),0,100);
							}
						}
						break;
					}
					case 1:
					{
						br.tip=pad->tip;
						break;
					}
				}

				if (!br.Equals(*brBase))
				{
					bModified=TRUE;

					LevelAgentBrief::Brief &brToMod=pad->bInitial?brief->initial:brief->cur;
					brToMod.CopyFrom(br);
				}
			}

			if (bModified)
				player->AddPendingAgentBriefEntry(guid);
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}


