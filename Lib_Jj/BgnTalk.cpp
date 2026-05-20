/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "commondefines/general_stl.h"
#include "LevelBehavior.h"

#include "LevelTalks.h"
#include "LevelPlayer.h"
#include "LoUnit.h"
#include "Level.h"

#include "BgnTalk.h"

////////////////////////////////////////////////////////////////////////
//CBgn_TalkAddChoice

BIND_BGN_CLASS(CBgn_TalkAddChoice,CBgp_TalkAddChoice);
void CBgn_TalkAddChoice::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_TalkAddChoice *pad=_GetPad<CBgp_TalkAddChoice>();
	LevelBehaviorContext *ctx=_GetCtx();

	if (pad)
	{
		if (pad->_choice!=StringID_Invalid)
		{
			CLevelTalks *talks=_GetTalks();
			if (talks)
			{
				LevelPlayerID idPlayer=_GetTalkPlayerID();
				if (idPlayer!=LevelPlayerID_Invalid)
				{
					talks->AddChoice(idPlayer,pad->_choice);
				}
			}
		}
	}
	_OutputOk(outputs,1,"结束");
}

////////////////////////////////////////////////////////////////////////
//CBgn_TalkSentence

BIND_BGN_CLASS(CBgn_TalkSentence,CBgp_TalkSentence);
void CBgn_TalkSentence::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_TalkSentence*pad=_GetPad<CBgp_TalkSentence>();
	LevelBehaviorContext *ctx=_GetCtx();
	if (pad)
	{
		if (pad->_snt!=StringID_Invalid)
		{
			CLevelTalks *talks=_GetTalks();
			if (talks)
			{
				LevelPlayerID idPlayer=_GetTalkPlayerID();
				if (idPlayer!=LevelPlayerID_Invalid)
				{
					talks->SetSentence(idPlayer,pad->_snt);
				}
			}
		}
	}
	_OutputOk(outputs,1,"结束");
}



////////////////////////////////////////////////////////////////////////
//CBgn_TalkSpeak
BIND_BGN_CLASS(CBgn_TalkSpeak,CBgp_TalkSpeak);

void CBgn_TalkSpeak::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_TalkSpeak*pad=_GetPad<CBgp_TalkSpeak>();
	LevelBehaviorContext *ctx=_GetCtx();
	if (pad)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
		{
			LevelPlayerID idPlayer=_GetTalkPlayerID();
			if (idPlayer!=LevelPlayerID_Invalid)
			{
				_idPlayer=idPlayer;
				if (talks->GetState(_idPlayer)!=LevelTalk_None)
				{
					if (!pad->_bPlayerSpeak)
						talks->SetSpeak(_idPlayer,pad->_content);
					else
						talks->SetPlayerSpeak(_idPlayer,pad->_content);

					talks->Popup(_idPlayer,LevelTalk_Speak);
					return;
				}
			}
		}
	}
	_SetResult(A_Fail);
}

void CBgn_TalkSpeak::Update(BGNOutputs &outputs)
{
	if (_idPlayer!=LevelPlayerID_Invalid)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
		{
			LevelTalkState state=talks->GetState(_idPlayer);
			if (state!=LevelTalk_None)
			{
				if (state!=LevelTalk_Speak)
				{
					_OutputOk(outputs,1,"结束");
					_idPlayer=LevelPlayerID_Invalid;
				}
				return;
			}
		}
	}
	_SetResult(A_Fail);
	return;
}

void CBgn_TalkSpeak::Destroy()
{
	if (_idPlayer!=LevelPlayerID_Invalid)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
			talks->ClearActive(_idPlayer);
		_idPlayer=LevelPlayerID_Invalid;
	}
}


void CBgn_TalkSpeak::Break(BGNOutputs &outputs)
{
	Destroy();
}


////////////////////////////////////////////////////////////////////////
//CBgn_TalkDialog
BIND_BGN_CLASS(CBgn_TalkDialog,CBgp_TalkDialog);

void CBgn_TalkDialog::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_TalkDialog*pad=_GetPad<CBgp_TalkDialog>();
	LevelBehaviorContext *ctx=_GetCtx();
	if (pad)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
		{
			LevelPlayerID idPlayer=_GetTalkPlayerID();
			if (idPlayer!=LevelPlayerID_Invalid)
			{
				if (pad->_dlg!=StringID_Invalid)
				{
					_idPlayer=idPlayer;
					if (talks->GetState(_idPlayer)!=LevelTalk_None)
					{
						talks->SetDlg(_idPlayer,pad->_dlg);

						talks->Popup(_idPlayer,LevelTalk_Dialog);
						return;
					}
				}
			}
		}
	}
	_SetResult(A_Fail);
}

void CBgn_TalkDialog::Update(BGNOutputs &outputs)
{
	if (_idPlayer!=LevelPlayerID_Invalid)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
		{
			if (_bHandling)
				return;//正在执行命令,不能处理另外的命令

			LevelTalkState state=talks->GetState(_idPlayer);
			if (state==LevelTalk_Dialog)
				return;//没有变化
			if (state==LevelTalk_DialogCmd)
			{
				CBgp_TalkDialog*pad=_GetPad<CBgp_TalkDialog>();

				LevelTalkDlgCmd cmd;
				if (talks->GetDlgCmd(_idPlayer,cmd))
				{
					int idx;
					VEC_FIND(pad->_cmds,cmd.cmd,idx);
					if (idx!=-1)
					{
						if (cmd.bClose)
						{
							outputs.Add(1+idx,_thrd);
							talks->Accept(_idPlayer,cmd.cmd);//进入到Accept状态
							_SetResult(A_Ok);
							_idPlayer=LevelPlayerID_Invalid;
						}
						else
						{
							BgnThread thrd;
							thrd=_thrd;
							thrd.idNode=_id;
							thrd.keyRewind=0;
							outputs.Add(1+idx,thrd);
							talks->Popup(_idPlayer,LevelTalk_Dialog);//回复到Dialog的状态
							_bHandling=TRUE;
						}
					}
					return;
				}
			}
		}
	}
	_SetResult(A_Fail);
	return;
}

void CBgn_TalkDialog::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_bHandling=FALSE;
}

void CBgn_TalkDialog::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_bHandling=FALSE;
}


void CBgn_TalkDialog::Destroy()
{
	if (_idPlayer!=LevelPlayerID_Invalid)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
			talks->ClearActive(_idPlayer);
		_idPlayer=LevelPlayerID_Invalid;
	}
}


void CBgn_TalkDialog::Break(BGNOutputs &outputs)
{
	if (_bHandling)
	{
		CBgp_TalkDialog*pad=_GetPad<CBgp_TalkDialog>();
		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;
		outputs.thrdsBreak.push_back(thrd);
		_bHandling=FALSE;
	}

	Destroy();
}

//////////////////////////////////////////////////////////////////////////
//CBgn_GetTalkDialogParam
BIND_BGN_CLASS(CBgn_GetTalkDialogParam,CBgp_GetTalkDialogParam);

void CBgn_GetTalkDialogParam::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetTalkDialogParam*pad=_GetPad<CBgp_GetTalkDialogParam>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelTalks *talks=_GetTalks();
	if (talks)
	{
		LevelPlayerID idPlayer=_GetTalkPlayerID();
		float v;
		talks->GetDlgCmdParam(idPlayer,pad->_idx,v);

		if (pad->_var!=StringID_Invalid)
			_SetFloat(pad->_var,v);
	}

	_OutputOk(outputs,1,"结束");
}


////////////////////////////////////////////////////////////////////\////
//CBgn_TalkChoose
BIND_BGN_CLASS(CBgn_TalkPopup,CBgp_TalkPopup);

void CBgn_TalkPopup::_Prepare(CLevelTalks *talks,BGNOutputs &outputs)
{
	CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();

	assert(talks);

	_stage=Preparing;
	if(pad->_varStop!=StringID_Invalid)
		_SetBit(pad->_varStop,FALSE);
	talks->ClearChoices(_idPlayer);//清除选项
	talks->ClearSentence(_idPlayer);
	talks->ClearDlg(_idPlayer);

	_VerifyStbName(1,"[准备]");

	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	outputs.Add(1,thrd);
}


void CBgn_TalkPopup::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();
	CLevelTalks *talks=_GetTalks();
	LevelBehaviorContext *ctx=_GetCtx();
	if (talks)
	{
		LevelPlayerID idPlayer=_GetTalkPlayerID();
		if (idPlayer!=LevelPlayerID_Invalid)
		{
			_idPlayer=idPlayer;
			if (talks->GetState(_idPlayer)!=LevelTalk_None)
			{
				_Prepare(talks,outputs);
				return;
			}
		}
	}
	_SetResult(A_Fail);
	return;
}

void CBgn_TalkPopup::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();
	assert(pad);
	CLevelTalks *talks=_GetTalks();
	assert(talks);
	if (talks->GetState(_idPlayer)==LevelTalk_None)
	{
		_SetResult(A_Fail);
		return;
	}

	if (_stage==Preparing)
	{
		//Prepare完了的Rewind
		_stage=Waiting;

		CLevelTalks *talks=_GetTalks();
		assert(talks);
		LevelTalkNode *node=talks->GetNode(_idPlayer);
		assert(node);
		if (node->nChoices<=0)
		{//没有选项,没有东西可以弹出来,直接结束
			_SetResult(A_Ok);
			return;
		}
		talks->Popup(_idPlayer,pad->_statePopup);
	}
	if (_stage==Handling)
	{
		//处理选项结束后的Rewind
		_TryRepeat(outputs);
	}
}

void CBgn_TalkPopup::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_SetResult(A_Fail);
	return;
}

BOOL CBgn_TalkPopup::_NeedRepeat()
{
	CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();
	if (pad->_varStop==StringID_Invalid)
		return FALSE;

	BOOL bStop=FALSE;
	_GetBit(pad->_varStop,bStop);
	if (!bStop)
		return TRUE;
	return FALSE;
}

void CBgn_TalkPopup::_TryRepeat(BGNOutputs &outputs)
{
	if (!_NeedRepeat())
	{
		_SetResult(A_Ok);
		return;
	}

	_Prepare(_GetTalks(),outputs);
}


void CBgn_TalkPopup::Update(BGNOutputs &outputs)
{
	if (_stage!=Waiting)
		return;

	CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();
	CLevelTalks *talks=_GetTalks();
	assert(_idPlayer!=LevelPlayerID_Invalid);
	assert(talks);
	LevelTalkState state=talks->GetState(_idPlayer);
	if (state==LevelTalk_None)
	{
		_SetResult(A_Fail);
		return;
	}
	if (state!=pad->_statePopup)
	{
		StringID choose=talks->GetChoose(_idPlayer);
		int idx;
		VEC_FIND(pad->_choices,choose,idx);
		if (idx>=0)
		{
			_stage=Handling;
			//处理选中的
			BgnThread thrd=_thrd;
			thrd.idNode=_id;
			thrd.keyRewind=0;
			outputs.Add(idx+2,thrd);
			return;
		}
		else
		{
			//选中的选项,我们无法处理,直接尝试Repeat
			_TryRepeat(outputs);
		}
	}
}

void CBgn_TalkPopup::Destroy()
{
	if (_idPlayer!=LevelPlayerID_Invalid)
	{
		CLevelTalks *talks=_GetTalks();
		if (talks)
			talks->ClearActive(_idPlayer);
		_idPlayer=LevelPlayerID_Invalid;
	}
}


void CBgn_TalkPopup::Break(BGNOutputs &outputs)
{
	if ((_stage==Preparing)||(_stage==Handling))
	{
		CBgp_TalkPopup*pad=_GetPad<CBgp_TalkPopup>();
		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;
		outputs.thrdsBreak.push_back(thrd);

		_stage=None;
	}

	Destroy();
}


////////////////////////////////////////////////////////////////////////
//CBgn_Break
BIND_BGN_CLASS(CBgn_BreakTalk,CBgp_BreakTalk);
void CBgn_BreakTalk::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelTalks *talks=_GetTalks();
	LevelBehaviorContext *ctx=_GetCtx();
	if (talks)
	{
		LevelPlayerID idPlayer=_GetTalkPlayerID();
		if (idPlayer!=LevelPlayerID_Invalid)
		{
			talks->ClearActive(idPlayer);
//			talks->Enable(idPlayer,FALSE);//Disable
		}
	}

	_OutputOk(outputs,1,"结束");

}


////////////////////////////////////////////////////////////////////////
//CBgn_WaitTalk
BIND_BGN_CLASS(CBgn_WaitTalk,CBgp_WaitTalk);
void CBgn_WaitTalk::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelTalks *talks=_GetTalks();

	//Enable all the talks
	if (talks)
	{
		CLevel *level=_GetLevel();
		for (int i=0;i<LEVEL_MAX_PLAYER;i++)
		{
			LevelPlayerID idPlayer=(LevelPlayerID)i;
			talks->Enable(idPlayer,TRUE);
			//talks->ClearActive(idPlayer); //TODO:加上这句,来清除之前的任何query
		}
	}

	Update(outputs);
}

void CBgn_WaitTalk::Update(BGNOutputs &outputs)
{
	CLevelTalks *talks=_GetTalks();
	LevelBehaviorContext *ctx=_GetCtx();
	if (talks)
	{
		LevelPlayerID idPlayer=_GetTalkPlayerID();
		LevelTalkState state=talks->GetState(idPlayer);
		if (state!=LevelTalk_None)
		{
			_OutputOk(outputs,1,"结束");
		}
	}
}



////////////////////////////////////////////////////////////////////////
//CBgn_RecordTalkPlayer
BIND_BGN_CLASS(CBgn_RecordTalkPlayer,CBgp_RecordTalkPlayer);
void CBgn_RecordTalkPlayer::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_RecordTalkPlayer*pad=_GetPad<CBgp_RecordTalkPlayer>();
	CLevelTalks *talks=_GetTalks();
	if (talks)
	{
		LevelBehaviorContext *ctx=_GetCtx();
		LevelPlayerID idPlayer=_GetTalkPlayerID();
		CLevelPlayer *player=ctx->level->GetPlayer(idPlayer);
		LevelObjID id=LevelObjID_Invalid;
		if (player)
		{
			if (player->GetLoUnit())
				id=player->GetLoUnit()->GetID();
		}

		if (pad->_nmVar!=StringID_Invalid)
			_SetID(pad->_nmVar,BehaviorMemType_ObjID,id);
	}

	_OutputOk(outputs,1,"结束");
}


//////////////////////////////////////////////////////////////////////////
//CBgn_CheckTalk
BIND_BGN_CLASS(CBgn_CheckTalk,CBgp_CheckTalk);
void CBgn_CheckTalk::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelTalks *talks=_GetTalks();

	//Enable all the talks
	if (talks)
	{
		if (talks->IsAnyActive())
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}
