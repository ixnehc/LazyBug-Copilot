
#include "stdh.h"
#include "Level.h"

#include "LevelTalks.h"

#include "datapacket/BitPacket.h"

////////////////////////////////////////////////////////////////////////
//LevelTalkNode

void LevelTalkNode::Write(CBitPacket *bp)
{
	bp->Bits_Write(state,4);

	bp->Bits_Write(nChoices,4);
	bp->Data_WriteData(choices,nChoices*sizeof(StringID));

	bp->Data_WriteSimple(snt);

	bp->Bits_Write(tpSpeaker,4);
	if (tpSpeaker==LevelTalkNode::Speaker_Other)
		bp->Data_WriteSimple(idOtherSpeaker);

	bp->Data_WriteSimple(choose);

	bp->Data_WriteSimple(dlg);
	bp->Data_WriteSimple(cmdDlg);

	bp->Data_WriteSimple(ver);
}

void LevelTalkNode::Read(CBitPacket *bp)
{
	state=(LevelTalkState)bp->Bits_Read(4);

	nChoices=bp->Bits_Read(4);
	bp->Data_ReadData(choices,nChoices*sizeof(StringID));

	bp->Data_ReadSimple(snt);

	tpSpeaker=(LevelTalkNode::Speaker)bp->Bits_Read(4);
	if (tpSpeaker==LevelTalkNode::Speaker_Other)
		bp->Data_ReadSimple(idOtherSpeaker);

	bp->Data_ReadSimple(choose);

	bp->Data_ReadSimple(dlg);
	bp->Data_ReadSimple(cmdDlg);

	bp->Data_ReadSimple(ver);
}



//////////////////////////////////////////////////////////////////////////
//CLevelTalks

void CLevelTalks::Create(CLevelObj *owner)
{
	_owner=owner;
	_nodesEnableDirty.resize(LEVEL_MAX_PLAYER);
}

void CLevelTalks::Destroy()
{
	for (int i=0;i<ARRAY_SIZE(_nodes);i++)
	{
		Safe_Class_Delete(_nodes[i]);
	}
	Zero();
}

void CLevelTalks::Enable(LevelPlayerID idPlayer,BOOL bEnable)
{
	BOOL bEnableOld=IsEnabled(idPlayer);
	if (bEnableOld==bEnable)
		return;
	if (bEnable)
	{
		_ObtainNode(idPlayer);
		SetDirty(idPlayer);
	}
	else
	{
		if (idPlayer<ARRAY_SIZE(_nodes))
		{
			Safe_Class_Delete(_nodes[idPlayer]);
		}
	}
	_nodesEnableDirty.set(idPlayer);
}


LevelTalkNode *CLevelTalks::_GetNode(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_nodes))
		return NULL;
	return _nodes[idPlayer];
}


LevelTalkNode *CLevelTalks::FindNode(LevelPlayerID idPlayer,LevelTalkState state)
{
	if (idPlayer>=ARRAY_SIZE(_nodes))
		return NULL;
	LevelTalkNode*p=_nodes[idPlayer];
	if (!p)
		return NULL;
	if (p->state==state)
		return p;
	return NULL;
}


void CLevelTalks::SetDirty(LevelPlayerID idPlayer)
{
	if (idPlayer<ARRAY_SIZE(_nodes))
	{
		LevelTalkNode*node=_nodes[idPlayer];
		if (node)
		{
			node->bNeedSync=TRUE;
			node->IncVer();
		}
	}
}

void CLevelTalks::ClearDirty(LevelPlayerID idPlayer)
{
	if (idPlayer<ARRAY_SIZE(_nodes))
	{
		LevelTalkNode*node=_nodes[idPlayer];
		if (node)
			node->bNeedSync=FALSE;
	}
}


LevelPlayerID CLevelTalks::GetFirstActive()
{
	for (int i=0;i<ARRAY_SIZE(_nodes);i++)
	{
		LevelTalkNode*node=_nodes[i];
		if (!node)
			continue;
		if (node->IsActive())
			return (LevelPlayerID)i;
	}
	return LevelPlayerID_Invalid;
}

LevelTalkNode*CLevelTalks::_ObtainNode(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_nodes))
		return NULL;
	if (!_nodes[idPlayer])
		_nodes[idPlayer]=Class_New2(LevelTalkNode);

	return _nodes[idPlayer];
}

void CLevelTalks::ClearActive(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_nodes))
		return;
	if (_nodes[idPlayer])
	{
		_nodes[idPlayer]->state=LevelTalk_None;
		SetDirty(idPlayer);
	}
}

void CLevelTalks::ClearChoices(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		node->nChoices=0;
		SetDirty(idPlayer);
	}
}


BOOL CLevelTalks::AddChoice(LevelPlayerID idPlayer,StringID nm)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	if (node->nChoices>=ARRAY_SIZE(node->choices))
		return FALSE;

	node->choices[node->nChoices]=nm;
	node->nChoices++;

	SetDirty(idPlayer);

	return TRUE;
}

void CLevelTalks::ClearSentence(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		node->snt=StringID_Invalid;
		SetDirty(idPlayer);
	}
}

void CLevelTalks::SetSentence(LevelPlayerID idPlayer,StringID snt)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return;
	node->snt=snt;
	SetDirty(idPlayer);
}


void CLevelTalks::ClearSpeak(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		node->tpSpeaker=LevelTalkNode::Speaker_None;
		SetDirty(idPlayer);
	}
}


BOOL CLevelTalks::SetSpeak(LevelPlayerID idPlayer,StringID speak)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	node->tpSpeaker=LevelTalkNode::Speaker_Owner;
	node->snt=speak;

	SetDirty(idPlayer);

	return TRUE;
}

BOOL CLevelTalks::SetPlayerSpeak(LevelPlayerID idPlayer,StringID speak)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	node->tpSpeaker=LevelTalkNode::Speaker_Player;
	node->snt=speak;

	SetDirty(idPlayer);

	return TRUE;
}

BOOL CLevelTalks::SetOtherSpeak(LevelPlayerID idPlayer,StringID speak,LevelObjID idOther)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	node->tpSpeaker=LevelTalkNode::Speaker_Other;
	node->snt=speak;
	node->idOtherSpeaker=idOther;

	SetDirty(idPlayer);

	return TRUE;
}

void CLevelTalks::ClearDlg(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		node->dlg=StringID_Invalid;
		SetDirty(idPlayer);
	}
}

void CLevelTalks::SetDlg(LevelPlayerID idPlayer,StringID dlg)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return;
	node->dlg=dlg;
	SetDirty(idPlayer);
}

void CLevelTalks::Popup(LevelPlayerID idPlayer,LevelTalkState state)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return;

	node->state=state;
	node->choose=StringID_Invalid;
//	node->cmdDlg.Zero();
	SetDirty(idPlayer);
}

void CLevelTalks::Query(LevelPlayerID idPlayer)
{
	if (_mode==TalkMode_Exclusive)
	{
		if (IsAnyActive())
			return;
	}

	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (node)
	{
		if (node->state==LevelTalk_None)
		{
			node->state=LevelTalk_Query;
			SetDirty(idPlayer);
		}
		else
		{
			int v=0;
			v++;
		}
	}
}


BOOL CLevelTalks::Accept(LevelPlayerID idPlayer,StringID choose)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	node->state=LevelTalk_Accept;
	node->choose=choose;

	SetDirty(idPlayer);

	return TRUE;
}

BOOL CLevelTalks::DoDlgCmd(LevelPlayerID idPlayer,LevelTalkDlgCmd &cmd)
{
	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (!node)
		return FALSE;

	node->state=LevelTalk_DialogCmd;
	node->cmdDlg=cmd;

	SetDirty(idPlayer);

	return TRUE;
}



LevelTalkState CLevelTalks::GetState(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (!node)
		return LevelTalk_None;
	return node->state;
}

StringID CLevelTalks::GetChoose(LevelPlayerID idPlayer)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (!node)
		return StringID_Invalid;
	if (node->state==LevelTalk_Accept)
		return node->choose;

	return StringID_Invalid;
}

BOOL CLevelTalks::GetDlgCmd(LevelPlayerID idPlayer,LevelTalkDlgCmd &cmd)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		if (node->state==LevelTalk_DialogCmd)
		{
			cmd=node->cmdDlg;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CLevelTalks::GetDlgCmdParam(LevelPlayerID idPlayer,int idx,float &v)
{
	LevelTalkNode *node=_GetNode(idPlayer);
	if (node)
	{
		if (idx==0)
		{
			v=node->cmdDlg.param01;
			return TRUE;
		}
		if (idx==1)
		{
			v=node->cmdDlg.param02;
			return TRUE;
		}
	}

	return FALSE;
}

void CLevelTalks::WriteFirst(CBitPacket *bp,LevelPlayerID idPlayer,BOOL &bContent)
{
	bContent=TRUE;

	LevelTalkNode*node=_GetNode(idPlayer);
	if (!node)
	{
		bp->Bit_Write_0();//没有Node
		return;
	}

	bp->Bit_Write_1();//有Node
	bp->Bit_Write_1();//有变化
	node->Write(bp);
}


void CLevelTalks::Write(CBitPacket *bp,LevelPlayerID idPlayer,BOOL &bContent)
{
	LevelTalkNode*node=_GetNode(idPlayer);
	if (!node)
	{
		bp->Bit_Write_0();//没有Node
		if (_nodesEnableDirty.test(idPlayer))
			bContent=TRUE;
		return;
	}

	bp->Bit_Write_1();//有Node
	if (node->bNeedSync)
	{
		bp->Bit_Write_1();//有变化
		node->Write(bp);
		bContent=TRUE;
	}
	else
		bp->Bit_Write_0();//没有变化
}


void CLevelTalks::Read(CBitPacket *bp,LevelPlayerID idPlayer)
{
	if (!bp->Bit_Read())
	{
		//没有内容
		Safe_Class_Delete(_nodes[idPlayer]);
		return;
	}

	LevelTalkNode *node=_ObtainNode(idPlayer);
	if (bp->Bit_Read())
	{//有变化
		node->Read(bp);
	}
}

void CLevelTalks::PostWrite()
{
	for (int i=0;i<ARRAY_SIZE(_nodes);i++)
	{
		if (_nodes[i])
			_nodes[i]->bNeedSync=FALSE;
	}
	_nodesEnableDirty.resetAll();
}

BOOL CLevelTalks::IsAnyActive()
{
	return GetFirstActive()!=LevelPlayerID_Invalid;
}
