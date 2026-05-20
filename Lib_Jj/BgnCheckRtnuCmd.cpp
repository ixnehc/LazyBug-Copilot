/********************************************************************
	created:	2013/6/20 
	author:		cxi
	
	purpose:	监控是否有随从命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "BgnCheckRtnuCmd.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckRtnuCmd
BIND_BGN_CLASS(CBgn_CheckRtnuCmd,CBgp_CheckRtnuCmd);
void CBgn_CheckRtnuCmd::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckRtnuCmd*pad=_GetPad<CBgp_CheckRtnuCmd>();

	CLevelObj *lo=_GetLo();

	if (lo)
	{
		if (lo->IsRetinue())
		{
			CLevelPlayer *player=lo->GetLevel()->GetPlayer(lo->GetPlayerID());
			if (player)
			{
				LevelRtnuCmd *cmds[32];
				DWORD nCmd=player->GetRecentRtnuCmds(0.2f,cmds,ARRAY_SIZE(cmds));

				for (int i=0;i<nCmd;i++)
				{
					BOOL bToMe=FALSE;
					LevelRtnuCmd *cmdRtnu=cmds[i];
					if (cmdRtnu)
					{
						if (cmdRtnu->tp==pad->_tp)
						{
							BOOL bMatch=FALSE;
							if (cmdRtnu->tp==LevelRtnuCmd_CastSkill)
							{
								if (cmdRtnu->idSkill==(RecordSimpleID)pad->_idSkill)
									bMatch=TRUE;
							}
							else
								bMatch=TRUE;
							if (bMatch)
							{
								switch(cmdRtnu->tpRtnu)
								{
									case LevelRtnuCmd::CmdRtnu_All:
									{
										bToMe=TRUE;
										break;
									}
									case LevelRtnuCmd::CmdRtnu_ID:
									{
										if (lo->GetID()==cmdRtnu->id)
											bToMe=TRUE;
									}
								}
							}
						}
					}
					if (bToMe)
					{
						if (pad->_nmTarget!=StringID_Invalid)
						{
							CBehaviorGraph *bg=_GetBg();
							if (bg)
							{
								CBehaviorMemDesc *memdesc=_GetMemDesc();
								if (memdesc)
								{
									BehaviorMemType tp=memdesc->GetVarType(pad->_nmTarget);
									if ((tp==BehaviorMemType_ObjID)||(tp==BehaviorMemType_Pos))
									{
										CBehaviorMem*mem=_GetMem();
										if (mem)
										{
											if (tp==BehaviorMemType_ObjID)
												mem->SetID(pad->_nmTarget,BehaviorMemType_ObjID,cmdRtnu->idTarget);
											if (tp==BehaviorMemType_Pos)
												mem->SetPos(pad->_nmTarget,cmdRtnu->posTarget);
										}
									}
								}
							}
						}

						_OutputOk(outputs,1,"收到");
						return;
					}
				}
			}
		}
	}
	_OutputFail(outputs,2,"未收到");
}
