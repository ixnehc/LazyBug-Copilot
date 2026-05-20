
#include "stdh.h"
#include "Protocal.h"

#define MSG_CLASS(msg) buf.push_back(Class_Ptr2(msg));

  
CClass **GetMsgClasses(DWORD &c)
{
	static std::vector<CClass *>buf;
	static BOOL bInit=FALSE;
	if (!bInit)
	{
		bInit=TRUE;

		MSG_CLASS(CSAuthorize);
		MSG_CLASS(SCAuthorize);

		MSG_CLASS(CBEnterWorld);
		MSG_CLASS(BCEnterWorld);

		MSG_CLASS(CBStartDay);
		MSG_CLASS(BCStartDay);
		MSG_CLASS(CBEndDay);
		MSG_CLASS(BCEndDay);
		MSG_CLASS(CBRestartGame);
		MSG_CLASS(BCRestartGame);

		MSG_CLASS(CBLeaveWorld);

		MSG_CLASS(BCLevelFrame);

		MSG_CLASS(CBPlayerMove);
		MSG_CLASS(BCPlayerMoveReply);

		MSG_CLASS(BCLevelObjsSync);

		MSG_CLASS(CBPlayerSkill);
		MSG_CLASS(CBPlayerSkill_MultiObj);
		MSG_CLASS(CBPlayerSkill_MultiSite);
		MSG_CLASS(CBPlayerSkill_MultiObjAndSite);
		MSG_CLASS(CBPlayerSkill_RawData);
		MSG_CLASS(BCPlayerSkillReply);

		MSG_CLASS(CBPlayerSkillCasted);
		MSG_CLASS(CBPlayerSkillCombine);
		MSG_CLASS(CBPlayerSkillStopCasting);

		MSG_CLASS(CSPing);
		MSG_CLASS(SCPing);

		MSG_CLASS(CSQueryPlayerStates);
		MSG_CLASS(SCPlayerStates);

		MSG_CLASS(CSBagOp);

		MSG_CLASS(CSEquipOp);

		MSG_CLASS(CSDiscardPickUp);

		MSG_CLASS(CBTalkOp);

		MSG_CLASS(BCPreTeleport);
		MSG_CLASS(CBAcceptTeleport);
		MSG_CLASS(BCTeleport);

		MSG_CLASS(SCExploreMapData);

		MSG_CLASS(CBGatherItem);
		MSG_CLASS(CBGatherResPile);

		MSG_CLASS(CSSetSkillFast);
		MSG_CLASS(CSSetRtnuSkill);

		MSG_CLASS(CSPause);

		MSG_CLASS(CBRtnuCmd);
		MSG_CLASS(CBRtnuHint);

		MSG_CLASS(CBInvokeMagicBoard);

		MSG_CLASS(SSRegisterSlave);
		MSG_CLASS(SSAcceptSlave);

		MSG_CLASS(SSRequestMigrate);
		MSG_CLASS(SSAcceptMigrate);

		MSG_CLASS(BCAbility);

		MSG_CLASS(CSSetFast);

		MSG_CLASS(BCDebugRunning);
		MSG_CLASS(CBDebugRunning);

		MSG_CLASS(CBSwitchWpn);

		MSG_CLASS(CBFlipSlate);
		MSG_CLASS(CBIncSlateButtonChip);

        MSG_CLASS(CBToeStoneThrust);
		MSG_CLASS(CBToggleAbility);

		MSG_CLASS(CBConsumeAbility);

		MSG_CLASS(CBUtumReturn);

		MSG_CLASS(CBAddService);
		MSG_CLASS(CBSignal);

		MSG_CLASS(CBConfirmAbsorb);

		MSG_CLASS(CBSlidewayReached);

		MSG_CLASS(SCAgentBrief);

		MSG_CLASS(BCDebugDraw);

		MSG_CLASS(CBShardsReady);

		MSG_CLASS(CBEelString);
		//XXXXX:More MsgType

		assert(buf.size()==NetMsg_End-NetMsg_Start);

	}


	c=buf.size();
	return buf.data();
}
