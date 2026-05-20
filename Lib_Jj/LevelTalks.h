#pragma once

#include "LevelDefines.h"

#include "strlib/strlibdefines.h"

#include "bitset/bitset.h"

class CLevelObj;



struct LevelTalkNode
{
	DEFINE_CLASS(LevelTalkNode);

	LevelTalkNode()
	{
		ver=0;
		bNeedSync=0;

		state=LevelTalk_None;
		tpSpeaker=Speaker_None;
		snt=StringID_Invalid;
		nChoices=0;
		choose=StringID_Invalid;

		dlg=StringID_Invalid;
	}

	enum Speaker
	{
		Speaker_None=0,
		Speaker_Owner,
		Speaker_Player,
		Speaker_Other,
	};


	void Write(CBitPacket *bp);
	void Read(CBitPacket *bp);
	BOOL IsActive()
	{
		if (state==LevelTalk_None)
			return FALSE;
		return TRUE;
	}

	void IncVer()
	{
		ver++;
	}

	//版本
	BYTE bNeedSync:1;//是否Dirty,需要同步给client
	DWORD ver;

	//State
	LevelTalkState state;

	//Content
	Speaker tpSpeaker;
	LevelObjID idOtherSpeaker;//只在Speaker_Other时有效
	StringID dlg;//对话框的名字
	StringID snt;//说的句子

	DWORD nChoices;
	StringID choices[MAX_TALK_CHOICES];

	//Result
	StringID choose;
	LevelTalkDlgCmd cmdDlg;//对话框命令,LevelTalk_DialogCmd时有效

};

struct LevelTalkDlgCmd;
class CLevelTalks
{
public:
	DEFINE_CLASS(CLevelTalks);
	CLevelTalks()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		memset(_nodes,0,sizeof(_nodes));
		_mode=TalkMode_Exclusive;
	}

	void Create(CLevelObj *owner);
	void Destroy();

	void SetMode(LevelTalkMode mode)	{		_mode=mode;	}
	LevelTalkMode GetMode()	{		return _mode;	}
	BOOL IsExclusiveMode()	{		return _mode==TalkMode_Exclusive;	}

	BOOL IsEnabled(LevelPlayerID idPlayer)	{		return GetNode(idPlayer)!=NULL;	}
	void Enable(LevelPlayerID idPlayer,BOOL bEnable);

	LevelTalkNode *GetNode(LevelPlayerID idPlayer)	{		return _GetNode(idPlayer);	}

	LevelTalkNode *FindNode(LevelPlayerID idPlayer,LevelTalkState state);

	//Content 
	void ClearChoices(LevelPlayerID idPlayer);
	BOOL AddChoice(LevelPlayerID idPlayer,StringID nm);
	void ClearSentence(LevelPlayerID idPlayer);
	void SetSentence(LevelPlayerID idPlayer,StringID snt);
	void ClearSpeak(LevelPlayerID idPlayer);
	BOOL SetSpeak(LevelPlayerID idPlayer,StringID snt);
	BOOL SetPlayerSpeak(LevelPlayerID idPlayer,StringID snt);
	BOOL SetOtherSpeak(LevelPlayerID idPlayer,StringID snt,LevelObjID idOther);
	void ClearDlg(LevelPlayerID idPlayer);
	void SetDlg(LevelPlayerID idPlayer,StringID dlg);

	//State
	BOOL IsAnyActive();
	LevelPlayerID GetFirstActive();
	LevelTalkState GetState(LevelPlayerID idPlayer);
	void Query(LevelPlayerID idPlayer);
	void Popup(LevelPlayerID idPlayer,LevelTalkState state);//弹出
	BOOL Accept(LevelPlayerID idPlayer,StringID choose);//关闭(正常的关闭)
	BOOL DoDlgCmd(LevelPlayerID idPlayer,LevelTalkDlgCmd &cmd);//处理Dialog命令,bClose指定要不要关闭对话框
	void ClearActive(LevelPlayerID idPlayer);

	//Result
	StringID GetChoose(LevelPlayerID idPlayer);

	BOOL GetDlgCmd(LevelPlayerID idPlayer,LevelTalkDlgCmd &cmd);
	BOOL GetDlgCmdParam(LevelPlayerID idPlayer,int idx,float &v);


	void SetDirty(LevelPlayerID idPlayer);
	void ClearDirty(LevelPlayerID idPlayer);

	void WriteFirst(CBitPacket *bp,LevelPlayerID idPlayer,BOOL &bContent);
	void Write(CBitPacket *bp,LevelPlayerID idPlayer,BOOL &bContent);
	void Read(CBitPacket *bp,LevelPlayerID idPlayer);
	void PostWrite();

protected:
	LevelTalkNode *_GetNode(LevelPlayerID idPlayer);
	LevelTalkNode*_ObtainNode(LevelPlayerID idPlayer);

	CLevelObj *_owner;

	LevelTalkMode _mode;

	LevelTalkNode*_nodes[LEVEL_MAX_PLAYER];
	Bitset<1> _nodesEnableDirty;
};


