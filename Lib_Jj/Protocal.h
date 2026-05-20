
#pragma once

#include "LevelDefines.h"
#include "LevelSlateDefines.h"
#include "LevelRtnuDefines.h"

#include "netmsg/netmsg.h"
#include "gds/GObj.h"

#include "strlib/strlibdefines.h"
#include "records/recordsdefine.h"

#include "math/circle.h"
#include "math/sphere.h"

#include "anim/animbase.h"

#define MATCH_SERVER_PORT (6667)
#define MATCH_SERVER_PORT_BS (6668) //为BattleServer打开的端口

#define BATTLE_SERVER_PORT_START (6680) //BattleServer的起始Port


//////////////////////////////////////////////////////////////////////////
//Messages

enum MsgType
{
	NetMsg_Start=8,//更小的值被系统使用

	NetMsg_CSAuthorize=NetMsg_Start,
	NetMsg_SCAuthorize,

	NetMsg_CBEnterWorld,
	NetMsg_BCEnterWorld,
	NetMsg_CBStartDay,
	NetMsg_BCStartDay,
	NetMsg_CBEndDay,
	NetMsg_BCEndDay,
	NetMsg_CBRestartGame,
	NetMsg_BCRestartGame,

	NetMsg_CBLeaveWorld,

	NetMsg_BCLevelFrame,

	NetMsg_CBPlayerMove,
	NetMsg_BCPlayerMoveReply,

	NetMsg_BCLevelObjsSync,

	NetMsg_CBPlayerSkill,
	NetMsg_CBPlayerSkill_MultiObj,
	NetMsg_CBPlayerSkill_MultiSite,
	NetMsg_CBPlayerSkill_MultiObjAndSite,
	NetMsg_CBPlayerSkill_RawData,
	NetMsg_BCPlayerSkillReply,
	NetMsg_CBPlayerSkillCasted,
	NetMsg_CBPlayerSkillCombine,
	NetMsg_CBPlayerSkillStopCasting,

	NetMsg_CSPing,
	NetMsg_SCPing,

	NetMsg_CSQueryPlayerStates,//请求PlayerStates的一个field
	NetMsg_SCPlayerStates,//返回PlayerStates的一个field的数据

	NetMsg_CSBagOp,
	NetMsg_CSEquipOp,

	NetMsg_CSDiscardPickUp,

	NetMsg_CBTalkOp,

	NetMsg_BCPreTeleport,
	NetMsg_CBAcceptTeleport,
	NetMsg_BCTeleport,

	NetMsg_SCExploreMapData,

	NetMsg_CBGatherItem,
	NetMsg_CBGatherResPile,

	NetMsg_CSSetSkillFast,
	NetMsg_CSSetRtnuSkill,

	NetMsg_CSPause,

	NetMsg_CBRtnuCmd,
	NetMsg_CBRtnuHint,

	NetMsg_CBInvokeMagicBoard,

	NetMsg_SSRegisterSlave,
	NetMsg_SSAcceptSlave,

	NetMsg_SSRequestMigrate,
	NetMsg_SSAcceptMigrate,

	NetMsg_BCAbility,

	NetMsg_CSSetFast,

	NetMsg_BCDebugRunning,
	NetMsg_CBDebugRunning,

	NetMsg_CBSwitchWpn,

	NetMsg_CBFlipSlate,
	NetMsg_CBIncSlateButtonChip,

    NetMsg_CBToeStoneThrust,
	NetMsg_CBToggleAbility,

	NetMsg_CBConsumeAbility,
	NetMsg_CBUtumReturn,

	NetMsg_CBAddService,
	NetMsg_CBSignal,

	NetMsg_CBConfirmAbsorb,

	NetMsg_CBSlidewayReached,

	NetMsg_SCAgentBrief,

	NetMsg_BCDebugDraw,

	NetMsg_CBShardsReady,

	NetMsg_CBEelString,


	//XXXXX:More MsgType

	NetMsg_End,
};


struct CSAuthorize:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSAuthorize);
	char account[MAX_ACCOUNT_NAME];
	char password[MAX_ACCOUNTPASSWORD_NAME];
	char nmPlayer[MAX_PLAYER_NAME];
	BOOL bMigrating;//是否在进行Server之间的迁移
};

struct SCAuthorize:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(SCAuthorize);
	BYTE result;//0表示失败,1表示成功
};


struct CBEnterWorld:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBEnterWorld);
	CBEnterWorld()
	{
	}
};

struct CBLeaveWorld:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBLeaveWorld);
};

struct CBStartDay:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBStartDay);
	CBStartDay()
	{
	}
};


struct CBEndDay:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBEndDay);
	CBEndDay()
	{
	}
};

struct CBRestartGame:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBRestartGame);
	CBRestartGame()
	{
		bNewDay=FALSE;
	}
	BOOL bNewDay;
};


struct BCRestartGame:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCRestartGame);
	BCRestartGame()
	{
	}
};


struct BCEnterWorld:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCEnterWorld);
	BCEnterWorld()
	{
	}
};

struct BCStartDay:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCStartDay);
	BCStartDay()
	{
		id=LevelObjID_Invalid;
		idPlayer=LevelPlayerID_Invalid;
	}
	
	LevelPlayerID idPlayer;//主角的player的id
	LevelObjID id;//主角unit的id
	LevelPos pos;//主角的位置
};


struct BCEndDay:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCEndDay);
	BCEndDay()
	{
		result=0;
	}
	BYTE result;//0:失败,1:成功
};

#define LEVELFRAME_NULL ((BYTE)0)
#define LEVELFRAME_LEAVESIGHT ((BYTE)1)
#define LEVELFRAME_ENTERSIGHT ((BYTE)2)
#define LEVELFRAME_SYNC_H ((BYTE)3)
#define LEVELFRAME_SYNC_L ((BYTE)4)
//这个消息通知Client: Server端的Level进入下一帧了,同时会将整个Level的下一帧的内容更新给客户端
struct BCLevelFrame:public NetMsgSC
{
	BEGIN_GOBJ_MSG_PURE(BCLevelFrame);
		GELEM_VAR(DWORD,tServer);
		GELEM_VARVECTOR(BYTE,data);
		GELEM_VARVECTOR(BYTE,bits);
	END_GOBJ_MSG();

	std::vector<BYTE>data;
	std::vector<BYTE>bits;

};

//注意:这个消息和BCLevelObjSync功能一样,只是会传递多个LevelObj的同步数据
struct BCLevelObjsSync:public NetMsgSC
{

	BEGIN_GOBJ_MSG_PURE(BCLevelObjsSync);
		GELEM_VAR(DWORD,tServer);
		GELEM_VAR(WORD,count);
		GELEM_VARVECTOR(BYTE,data);
		GELEM_VARVECTOR(BYTE,bits);
	END_GOBJ_MSG();
	WORD count;
	std::vector<BYTE>data;
	std::vector<BYTE>bits;
};


struct PlayerMove
{
	PlayerMove()
	{
		memset(this,0,sizeof(*this));
	}
	DWORD tStart;//AnimTick,起始时间
	LevelPos posExpect;//预期要走到的位置
	LevelPos pos1;
	LevelPos pos2;
	BYTE nStep:5;//如果nStep<=4,pos2无效,pos1代表了nStep后的位置
							//如果nStep>4,pos1代表了4个step后的位置,pos2代表了nStep后的位置
							//1个step目前为1/4个LEVEL_UPDATE_INTERVAL
	BYTE bReaching:1;//走完后是否到达
	BYTE bTeleport:1;//是否为Teleport的路点,如果为1,pos1为要teleport的位置,pos2无效,但pos2的前两个字节保存了一个LevelTeleportID
	BYTE bFace:1;//face是否有效
	LevelFaceInt face;//第一个step时的face
};

struct CBPlayerMove:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBPlayerMove);

	PlayerMove move;
};

struct PlayerMoveReply
{
	int tDiff;//AnimTick,Server收到PlayerMove消息的时间-PlayerMove里的tStart

};
struct BCPlayerMoveReply:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCPlayerMoveReply);
	PlayerMoveReply reply;
};

struct PlayerSkill
{
	PlayerSkill()
	{
//		tidSkill=0;
		idClient=ClientSkillID_Invalid;
	}
	LevelSkillTarget target;
//	DWORD tidSkill;
	LevelSkillType tpSkill;
	ClientSkillID idClient;
};

struct CBPlayerSkill:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBPlayerSkill);
	PlayerSkill skill;
};

struct CBPlayerSkill_MultiX:public NetMsg
{
	CBPlayerSkill_MultiX()
	{
		seedRnd=LevelRandomSeed_Invalid;
	}
	PlayerSkill skill;
	LevelRandomSeed seedRnd;
};

//带附加的多个LevelObjID的释放技能消息
#define MAX_PLAYERSKILL_IDS 16
struct CBPlayerSkill_MultiObj:public CBPlayerSkill_MultiX
{
	DEFINE_SIMPLE_MSG(CBPlayerSkill_MultiObj);
	CBPlayerSkill_MultiObj()
	{
		nIDs=0;
	}
	LevelObjID ids[MAX_PLAYERSKILL_IDS];
	DWORD nIDs;
};

//带附加的多个LevelPos的释放技能消息
#define MAX_PLAYERSKILL_SITES 16
struct CBPlayerSkill_MultiSite:public CBPlayerSkill_MultiX
{
	DEFINE_SIMPLE_MSG(CBPlayerSkill_MultiSite);
	CBPlayerSkill_MultiSite()
	{
		nSites=0;
	}
	LevelPos sites[MAX_PLAYERSKILL_SITES];
	DWORD nSites;
};

struct CBPlayerSkill_MultiObjAndSite:public CBPlayerSkill_MultiX
{
	DEFINE_SIMPLE_MSG(CBPlayerSkill_MultiObjAndSite);
	CBPlayerSkill_MultiObjAndSite()
	{
		nIDs=0;
		nSites=0;
	}
	LevelObjID ids[MAX_PLAYERSKILL_IDS];
	DWORD nIDs;

	LevelPos sites[MAX_PLAYERSKILL_SITES];
	DWORD nSites;
};


//带附加的多个angle的释放技能消息
#define MAX_PLAYERSKILL_RAWDATA 1024
struct CBPlayerSkill_RawData:public CBPlayerSkill_MultiX
{
	DEFINE_SIMPLE_MSG(CBPlayerSkill_RawData);
	CBPlayerSkill_RawData()
	{
		sz=0;
	}
	BYTE data[MAX_PLAYERSKILL_RAWDATA];
	DWORD sz;
};


struct CBPlayerSkillCasted:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBPlayerSkillCasted);
	ClientSkillID idClient;
};

struct CBPlayerSkillCombine:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBPlayerSkillCombine);
	LevelSkillTarget target;
	ClientSkillID idClient;
};

struct CBPlayerSkillStopCasting:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBPlayerSkillStopCasting);
	AnimTick tCasting;
	ClientSkillID idClient;
};


struct BCPlayerSkillReply:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCPlayerSkillReply);

	BCPlayerSkillReply()
	{
		result=0;
		idClient=ClientSkillID_Invalid;
	}
	BYTE result;
	ClientSkillID idClient;
};


struct CSPing:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSPing);

};
struct SCPing:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(SCPing);

	unsigned __int64 cycle;//Server收到的时间

};

struct CSQueryPlayerStates:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSQueryPlayerStates);
	CSQueryPlayerStates()
	{
		nFields=0;
	}
	WORD nFields;
	WORD fields[4];
};

struct SCPlayerStates:public NetMsgSC
{
	BEGIN_GOBJ_MSG_PURE(SCPlayerStates);
		GELEM_VAR(DWORD,tServer);
		GELEM_VAR(WORD,nFields);
		GELEM_VARVECTOR(BYTE,data);
	END_GOBJ_MSG();
	WORD nFields;
	std::vector<BYTE>data;

};

struct CSBagOp:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSBagOp);
	enum Type
	{
		None,
		PickUp,
		DropDown,
		Equip,
		UnEquip,
	};
	CSBagOp()
	{
		tp=(BYTE)None;
		iBag=0;
	}
	BYTE tp;
	BYTE iBag;
	BYTE part;//tp为UnEquip时有效
	i_math::rect_c rc;
};

struct CSEquipOp:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSEquipOp);
	enum Type
	{
		None,
		PickUp,
		DropDown,
	};

	CSEquipOp()
	{
		part=0;
	}
	BYTE tp;
	BYTE part;//EquipPart_XXXXX

};

//丢弃PickUp的道具
struct CSDiscardPickUp:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSDiscardPickUp);
	LevelPos pos;//丢在哪
};

struct CBTalkOp:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBTalkOp);

	enum Op
	{
		None,
		Query,
		Accept,
		DialogCmd,
		Break,//中断这个LevelObj与这个Player的所有Talk
	};

	CBTalkOp()
	{
		op=None;
		id=LevelObjID_Invalid;
		choose=StringID_Invalid;
	}

	DWORD op;

	LevelObjID id;

	//仅在op为Accept时有效
	StringID choose;//选择哪一句话

	//仅在op为DialogCmd时有效
	LevelTalkDlgCmd cmdDlg;//
};



//这个消息让Client为接下来一小段时间后会发生的真正的Teleport作准备
struct BCPreTeleport:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCPreTeleport);

	BCPreTeleport()
	{
		idMap=RecordID_Invalid;
	}

	RecordID idMap;
	LevelPos pos;
};

//客户端通知Server已经准备好了
struct CBAcceptTeleport:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBAcceptTeleport);

};

//真正的Telepot通知
struct BCTeleport:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCTeleport);

	BCTeleport()
	{
		idMap=RecordID_Invalid;
		idPlayerUnit=LevelObjID_Invalid;
		idPlayer=LevelPlayerID_Invalid;
	}

	RecordID idMap;
	LevelPos pos;
	LevelPlayerID idPlayer;
	LevelObjID idPlayerUnit;
};

//Explore Map Data
struct SCExploreMapData:public NetMsgSC
{
	BEGIN_GOBJ_MSG_PURE(SCExploreMapData);
		GELEM_VAR(DWORD,tServer);
		GELEM_VAR(RecordID,idMap);
		GELEM_VARVECTOR(BYTE,dataEMs);
	END_GOBJ_MSG();

	RecordID idMap;
	std::vector<BYTE> dataEMs;//ExploreMap的数据
};

struct CBGatherItem:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBGatherItem);
	CBGatherItem()
	{
		idItem=LevelObjID_Invalid;
	}

	LevelObjID idItem;
};

struct CBGatherResPile:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBGatherResPile);
	CBGatherResPile()
	{
		idOwner=LevelObjID_Invalid;
		tp=LevelResource_None;
		amount=0;
	}

	LevelObjID idOwner;
	LevelResourceType tp;
	BYTE amount;
};

struct CSSetSkillFast:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSSetSkillFast);
	CSSetSkillFast()
	{
		idxFast=0;
		bLorR=0;
		idSkill=RecordID_Invalid;
	}

	BYTE idxFast;//如果idxFast为0xff,表示直接设置左键/右键技能(而不是绑定热键)
	BYTE bLorR;//左键技能还是右键技能
	RecordID idSkill;
};

struct CSSetRtnuSkill:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSSetRtnuSkill);
	CSSetRtnuSkill()
	{
		idRtnu=RecordID_Invalid;
		idSkill=RecordID_Invalid;
	}
	RecordID idRtnu;
	RecordID idSkill;

};

struct CSPause:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSPause);
	CSPause()
	{
		bPause_=TRUE;
		durResume=0.0f;
	}
	BOOL bPause_;
	float durResume;
};

struct CBRtnuCmd:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBRtnuCmd);

	LevelRtnuCmd cmd;
};


struct CBRtnuHint:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBRtnuHint);
	LevelRtnuHint hint;
};

struct MagicBoardInvoke
{
	LevelObjID id;
	short x,y;

};

struct CBInvokeMagicBoard:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBInvokeMagicBoard);
	MagicBoardInvoke invoke;
};

//Server2Server

struct SSRegisterSlave:public NetMsg
{
	DEFINE_SIMPLE_MSG(SSRegisterSlave);
	int tp;
};

struct SSAcceptSlave:public NetMsg
{
	DEFINE_SIMPLE_MSG(SSAcceptSlave);
	DWORD port;
};

struct SSRequestMigrate:public NetMsg
{
	DEFINE_SIMPLE_MSG(SSRequestMigrate);
	char nmPlayer[MAX_PLAYER_NAME];
};

struct SSAcceptMigrate:public NetMsg
{
	DEFINE_SIMPLE_MSG(SSAcceptMigrate);
	char nmPlayer[MAX_PLAYER_NAME];
};

struct BCAbility:public NetMsgSC
{
	BEGIN_GOBJ_MSG_PURE(BCAbility);
		GELEM_VAR(DWORD,tServer);
		GELEM_VAR(LevelAbilityType,tp);
		GELEM_VARVECTOR(BYTE,data);
	END_GOBJ_MSG();

	LevelAbilityType tp;
	std::vector<BYTE> data;

};

struct CSSetFast:public NetMsg
{
	DEFINE_SIMPLE_MSG(CSSetFast);
	CSSetFast()
	{
		op=Bind;
		key=0;
		tp=0;
	}

	enum Op
	{
		Bind,
		SetSelectedSkill,//将target设为右键技能
	};

	Op op;
	char key;
	BYTE tp;
	LevelFastTarget target;
};

struct BCDebugRunning:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCDebugRunning);
};

struct CBDebugRunning:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBDebugRunning);
};

struct CBSwitchWpn:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBSwitchWpn);
	CBSwitchWpn()
	{
		wpnActive=0;
	}
	char wpnActive;
};

struct CBFlipSlate:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBFlipSlate);
	CBFlipSlate()
	{
		idSlates=LevelObjID_Invalid;
		idxSlate=LevelSlateIdx_Invalid;
	}
	LevelObjID idSlates;
	LevelSlateIdx idxSlate;
};

struct CBIncSlateButtonChip:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBIncSlateButtonChip);
	CBIncSlateButtonChip()
	{
		idSlates=LevelObjID_Invalid;
		idxSlate=LevelSlateIdx_Invalid;
	}
	LevelObjID idSlates;
	LevelSlateIdx idxSlate;
};


#define MAX_TOESTONE_THRUST_SITES 48
struct CBToeStoneThrust:public NetMsg
{
    DEFINE_SIMPLE_MSG(CBToeStoneThrust);
    CBToeStoneThrust()
    {
        nSites = 0;
    }
    i_math::pos2d_sh base;
    i_math::pos2db sites[MAX_TOESTONE_THRUST_SITES];
    DWORD nSites;
};

struct CBToggleAbility:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBToggleAbility);
	CBToggleAbility()
	{
		tpAbility=LevelAbilityType_None;
		bOn=TRUE;
	}
	LevelAbilityType tpAbility;
	BOOL bOn;
};

struct CBConsumeAbility:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBConsumeAbility);
	CBConsumeAbility()
	{
		tpAbility=LevelAbilityType_None;
	}
	LevelAbilityType tpAbility;
};

struct CBUtumReturn:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBUtumReturn);
	CBUtumReturn()
	{
		idEo = LevelObjID_Invalid;
	}
	LevelObjID idEo;
};

struct CBAddService:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBAddService);
	CBAddService()
	{
		idServer = LevelObjID_Invalid;
		tp=LevelServiceType_None;
		nQuota=0;
		durTimeOut=ANIMTICK_INFINITE;
	}
	LevelServiceType tp;
	short nQuota;
	short methodPriority;//CLevelService::ClientPriorityMethod
	LevelObjID idServer;
	AnimTick durTimeOut;

};

struct CBSignal:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBSignal);
	CBSignal()
	{
		idTarget=LevelObjID_Invalid;
		idSrc=LevelObjID_Invalid;
		nm=StringID_Invalid;
		radius=0.0f;
	}
	StringID nm;
	LevelObjID idTarget;
	LevelObjID idSrc;
	LevelPos posSrc;
	float radius;

};

struct CBConfirmAbsorb:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBConfirmAbsorb);
	CBConfirmAbsorb()
	{
		id=LevelObjID_Invalid;
	}
	LevelObjID id;
};

struct CBSlidewayReached:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBSlidewayReached);
	CBSlidewayReached()
	{
		id=LevelObjID_Invalid;
	}
	LevelObjID id;
};

struct SCAgentBrief:public NetMsgSC
{
	BEGIN_GOBJ_MSG_PURE(SCAgentBrief);
		GELEM_VAR(LevelGUID ,guid);
		GELEM_VARVECTOR(BYTE,data);
	END_GOBJ_MSG();

	LevelGUID guid;
	std::vector<BYTE> data;

};

struct BCDebugDraw:public NetMsgSC
{
	DEFINE_SIMPLE_MSG(BCDebugDraw);

	BCDebugDraw()
	{
		tp=None;
		uid=0;
		col=0;
		dur=0.0f;
	}

	enum Type
	{
		None,
		Circle,
		Line,
		Fan,
		Sphere
	};

	i_math::circlef &GetCircle()
	{
		return *(i_math::circlef*)data;
	}
	i_math::line2df &GetLine()
	{
		return *(i_math::line2df*)data;
	}
	AnimEventZone::KeyFan &GetFan()
	{
		return *(AnimEventZone::KeyFan*)data;
	}
	i_math::spheref &GetSphere()
	{
		return *(i_math::spheref*)data;
	}


	DWORD uid;
	Type tp;
	DWORD col;
	float dur;
	BYTE data[80];

};

struct CBShardsReady:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBShardsReady);
	CBShardsReady()
	{
		id=LevelObjID_Invalid;
	}
	LevelObjID id;
};

struct CBEelString:public NetMsg
{
	DEFINE_SIMPLE_MSG(CBEelString);

	enum Type
	{
		None,
		Touch,
		Untouch,
		Broken,
	};

	CBEelString()
	{
		idOwner=LevelObjID_Invalid;
		tp=None;
		rateTouch=0;
		idTarget=LevelObjID_Invalid;
	}
	BOOL Equal(CBEelString &other)
	{
		return idOwner==other.idOwner&&tp==other.tp&&rateTouch==other.rateTouch&&idTarget==other.idTarget;
	}

	LevelObjID idOwner;
	BYTE tp;//Type
	BYTE rateTouch;//0..100,在什么位置Touch
	LevelObjID idTarget;
};


//XXXXX:More MsgType
