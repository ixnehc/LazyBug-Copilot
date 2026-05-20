/********************************************************************
	created:	2012/11/21 
	author:		cxi
	
	purpose:	Level Behavior
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include "Behavior.h"

#include "BehaviorGraphs.h"
#include "BehaviorGraphPads.h"
#include "BehaviorDebug.h"

#include "BgnHelper.h"

#include "../Random/Random.h"

#include "../Log/LogDump.h"



void _SaveSet(CDataPacket *dp,std::unordered_set<PadID> &set)
{
	dp->Data_NextWord()=set.size();
	std::unordered_set<PadID>::iterator it;
	for (it=set.begin();it!=set.end();it++)
		dp->Data_WriteSimple(*it);
}
void _LoadSet(CDataPacket *dp,std::unordered_set<PadID> &set)
{
	set.clear();
	WORD sz=dp->Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		PadID id;
		dp->Data_ReadSimple(id);
		set.insert(id);
	}
}


////////////////////////////////////////////////////////////////////////
//CBehaviorGraphNode

CBehaviorGraphNode::~CBehaviorGraphNode()
{
	Safe_Class_Delete(_padCache);
}

CBehaviorGraphPad *CBehaviorGraphNode::_GetPad()
{
	if (_lpad)
		return (CBehaviorGraphPad *)(_lpad->pad);
	return NULL;
}

CBehaviorMem*CBehaviorGraphNode::_GetMem()
{
	if (!_lpad)
		return NULL;
	if (_bhv)
		return _bhv->GetMem((DWORD)_lpad->idxMem);
	return NULL;
}

CBehaviorMemDesc*CBehaviorGraphNode::_GetMemDesc()
{
	CBehaviorGraph *bg=_GetBg();
	if (!bg)
		return NULL;
	return bg->GetMemDesc((DWORD)_lpad->idxMem);
}


CBehaviorGraph *CBehaviorGraphNode::_GetBg()
{		
	return _bhv?_bhv->GetBg():NULL;	
}



BOOL CBehaviorGraphNode::_GetCOut(DWORD iStb)
{
	if (_lpad)
	{
		if (iStb<_lpad->stbOthers.size())
		{
			StbOther stbOther=_lpad->stbOthers[iStb];
			BGPad *lpadOther=stbOther.pad;
			if (lpadOther)
			{
				if (lpadOther->clssNode)
				{
					CBehaviorGraphNode *node=(CBehaviorGraphNode *)lpadOther->clssNode->New();
					BOOL ret=node->GetCIn();
					Safe_Class_Delete(node);
					return ret;
				}
			}
		}
	}
	return TRUE;
}

BOOL CBehaviorGraphNode::_TestStbLink(DWORD iStb)
{
	if (_lpad)
	{
		if (iStb<_lpad->stbOthers.size())
			return _lpad->stbOthers[iStb].pad!=NULL;
	}
	return FALSE;
}


void CBehaviorGraphNode::_VerifyStbName(DWORD iStb,const char *nm)
{
#ifdef _DEBUG
	BOOL bError=FALSE;
	CBehaviorGraphPad *pad=_GetPad();
	if (pad)
	{
		if (iStb>=pad->GetStubCount())
			bError=TRUE;
		else
		{
			PadStub stub=pad->GetStub(iStb);
			if (strcmp(stub.name,nm)!=0)
				bError=TRUE;
		}
	}

	assert(!bError);

#endif
}

void CBehaviorGraphNode::_SetResult(AResult result)	
{		
	_result=result;
	switch(result)
	{
		case A_Ok:
		{
			switch(_lpad->pad->_styleRet)
			{
				case CBehaviorGraphPad::Not:
				case CBehaviorGraphPad::AlwaysFalse:
					_result=A_Fail;	
					break;
			}
			break;
		}
		case A_Fail:
		{
			switch(_lpad->pad->_styleRet)
			{
				case CBehaviorGraphPad::Not:
				case CBehaviorGraphPad::AlwaysTrue:
					_result=A_Ok;	
					break;
			}
			break;
		}
	}
}


void CBehaviorGraphNode::_OutputOk(BGNOutputs &outputs,DWORD iStb,const char *nm)
{
	_VerifyStbName(iStb,nm);
	outputs.Add(iStb,_thrd);
	_SetResult(A_Ok);
}

void CBehaviorGraphNode::_OutputFail(BGNOutputs &outputs,DWORD iStb,const char *nm)
{
	_VerifyStbName(iStb,nm);
	outputs.Add(iStb,_thrd);
	_SetResult(A_Fail);
}


BOOL CBehaviorGraphNode::_SetBit(StringID nmVar,BOOL b)
{
	if (!_bhv)
		return FALSE;

	if (nmVar==StringID_Invalid)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->SetBit(nmVar,b))
			return TRUE;
	}
	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_SetNumber(StringID nmVar,short n)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->SetNumber(nmVar,n))
			return TRUE;
	}
	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_SetFloat(StringID nmVar,float f)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->SetFloat(nmVar,f))
			return TRUE;
	}
	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_SetID(StringID nmVar,BehaviorMemType tp,DWORD id)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->SetID(nmVar,tp,id))
			return TRUE;
	}
	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_GetBit(StringID nmVar,BOOL &b)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->GetBit(nmVar,b))
			return TRUE;
	}
// 	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中读出变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_GetNumber(StringID nmVar,short &n)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->GetNumber(nmVar,n))
			return TRUE;
	}
// 	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_GetFloat(StringID nmVar,float&f)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->GetFloat(nmVar,f))
			return TRUE;
	}
	return FALSE;
}


BOOL CBehaviorGraphNode::_GetID(StringID nmVar,BehaviorMemType tp,DWORD &id)
{
	id=0;
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->GetID(nmVar,tp,id))
			return TRUE;
	}
// 	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_SetPos(StringID nmVar,i_math::vector2df &pos)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->SetPos(nmVar,pos))
			return TRUE;
	}
	LOG_DUMP_2P("BehaviorGraph",Log_Error,"行为图(%s)中写入变量(%s)失败!",StrLib_GetStr(_bhv->GetName()),StrLib_GetStr(nmVar));
	return FALSE;
}

BOOL CBehaviorGraphNode::_GetPos(StringID nmVar,i_math::vector2df &pos)
{
	if (!_bhv)
		return FALSE;

	CBehaviorMem *mem=_GetMem();
	if (mem)
	{
		if (TRUE==mem->GetPos(nmVar,pos))
			return TRUE;
	}

	return FALSE;
}



AnimTick CBehaviorGraphNode::_GetT()
{
	if (_bhv)
		return _bhv->GetT();
	return 0;
}


////////////////////////////////////////////////////////////////////////
//CBehavior
BehaviorDebugFrameData CBehavior::_dataFrameTemp;


void CBehavior::Init(CBehaviorGraph *bg,DWORD objOwner)
{
	_bg=bg;
	_objOwner=objOwner;

	//创建Counters
	if (TRUE)
	{
		std::unordered_map<StringID,CBgp_Counter*>::iterator it;
		for (it=bg->_counters.begin();it!=bg->_counters.end();it++)
		{
			StringID nm=(*it).first;

			CBgp_Counter*pad=(*it).second;

			BehaviorCounter counter;
			counter.v=CSysRandom::RandVaryUInt(pad->_init,pad->_vary);
			_counters[nm]=counter;
		}
	}

	//创建Timers
	if (TRUE)
	{
		std::unordered_map<StringID,CBgp_Timer*>::iterator it;
		for (it=bg->_timers.begin();it!=bg->_timers.end();it++)
		{
			StringID nm=(*it).first;

			CBgp_Timer*pad=(*it).second;

			BehaviorTimer timer;
			timer.tExpect=_GetT()+CSysRandom::RandVaryUInt(pad->_init,pad->_vary);
			_timers[nm]=timer;
		}
	}

	_mems.resize(_bg->_mems.size());
	VEC_SET(_mems,0);
	for (int i=0;i<_bg->_mems.size();i++)
	{
		if (!_bg->_mems[i]->IsEmpty())
		{
			_mems[i]=Class_New2(CBehaviorMem);
			_mems[i]->Init(_bg->_mems[i]);
		}
	}
}

void CBehavior::Clear()
{
	if (_bg)
	{
		CBehaviorGraphs *bgs = _bg->GetOwner();
		if (bgs)
		{
			CBehaviorDebug *dbg = bgs->GetDebug();
			if (dbg)
				dbg->ClearFrameData(_objOwner,_bg->GetName());
		}
	}

	for (int i=0;i<_pendings.size();i++)
	{
		_DestroyNode(_pendings[i]);
		Safe_Class_Delete(_pendings[i]);
	}
	_pendings.clear();

	_counters.clear();
	_timers.clear();

	for (int i=0;i<_mems.size();i++)
	{
		if (_mems[i])
		{
			_mems[i]->Clear();
			Safe_Class_Delete(_mems[i]);
		}
	}
	_mems.clear();

	_dataNodes.clear();

	Zero();
}


CBehaviorGraphNode *CBehavior::_CreateNode(BGPad *lpad,BgnThread thrd)
{
	if (!lpad)
		return NULL;

	if (!lpad->clssNode)
		return NULL;
	CBehaviorGraphNode *node=(CBehaviorGraphNode *)lpad->clssNode->New();
	node->_lpad=lpad;
	node->_bhv=this;
	node->_thrd=thrd;
	node->_id=++_seedBgnID;
	node->Create();

	return node;
}


void CBehavior::_DestroyNode(CBehaviorGraphNode *node)
{
	node->Destroy();

	//标记为空
	node->_bhv=NULL;
	node->_lpad=NULL;
}

void CBehavior::_BreakNode(CBehaviorGraphNode *node,BGNOutputs &outputs)
{
	node->Break(outputs);
//	_DestroyNode(node);
}


void CBehavior::Start()
{
	if (!_bg)
		return;

	_tStart=_GetT();
	_iFrame=0;

	_nops.clear();

	CBehaviorMem *mem=GetMem(0);
	if (mem)
	{
		assert(mem->_states.size()<=0);
	}
	if (TRUE)
	{
		BGPad *lpadDef=_bg->_def;

		CBehaviorGraphNode *node=_CreateNode(lpadDef,BgnThread());
		if (!node)
			return;
		_nops.push_back(BGNop(node,BGNop::Start));
	}

	_FlushOps(_nops);
}

AResult CBehavior::StartRelay(StringID nmRelay)
{
	if (!_bg)
		return A_Fail;

	_nops.clear();

	if (TRUE)
	{
		BGPad *lpadRelay=_bg->LPadFromRelayName(nmRelay);
		CBehaviorGraphNode *node=_CreateNode(lpadRelay,BgnThread());
		if (!node)
			return A_Fail;
		_nops.push_back(BGNop(node,BGNop::Start));
	}

	return _FlushOps(_nops);
}


CBehaviorGraphNode *CBehavior::NodeFromNodeID(BgnID idNode)
{
	if (idNode==BgnID_Invalid)
		return NULL;
	for (int i=0;i<_pendings.size();i++)
	{
		CBehaviorGraphNode *node=_pendings[i];
		if (!node)
			continue;
		if (!node->IsValid())
			continue;
		if (node->_id==idNode)
			return node;
	}

	return NULL;
}

BOOL CBehavior::_MakeOutputs(CBehaviorGraphNode *node,BGNOutputs &outputs,std::deque<BGNop> &nops)
{
	BOOL bAnyOk=FALSE;
	for (int i=outputs.nOutputs-1;i>=0;i--)
	{
		BYTE stb=outputs.stbs[i];//从这个node的哪个stb发出
		StbOther stbOther;
		if (stb<node->_lpad->stbOthers.size())
			stbOther=node->_lpad->stbOthers[stb];

		if (TRUE)
		{
			CBehaviorGraphNode *nodeOther=_CreateNode(stbOther.pad,outputs.thrds[i]);
			if (nodeOther)
			{
				BGNop nopNew(nodeOther,BGNop::Start);
				nops.push_front(nopNew);
				bAnyOk=TRUE;
			}
		}
	}
	return bAnyOk;
}

void CBehavior::_MakeRewind(CBehaviorGraphNode *node,BOOL bOk,std::deque<BGNop> &nops)
{
	if (node->_thrd.keyRewind!=BGNTHREAD_INVALID_REWINDKEY)
	{//有Rewind信息

		CBehaviorGraphNode *nodeRewind=NodeFromNodeID(node->_thrd.idNode);
		if (nodeRewind)
		{
			BGNop nop(nodeRewind,bOk?BGNop::RewindOk:BGNop::RewindFail);
			nop.keyRewind=node->_thrd.keyRewind;
			nops.push_front(nop);//还是push_back(..)?
		}
	}
}

void CBehavior::_DeleteNotInPending(CBehaviorGraphNode *node)
{
	if (node)
	{
		if (!node->_bInPending)
			Safe_Class_Delete(node);
	}
}


AResult CBehavior::_FlushOps(std::deque<BGNop> &nops)
{
	_statesSwitched.clear();

	_outputs.Clear();

	AResult ret=A_Ok;

	while(nops.size()>0)
	{
		BGNop nop=nops[0];
		nops.pop_front();

		if (nop.op==BGNop::None)
			continue;

		_outputs.Clear();

		CBehaviorGraphNode *node=nop.node;
		BOOL bFinalizing=node->_IsFinalizing();

		//根据op执行对node的操作
		switch(nop.op)
		{
			case BGNop::Start:
			{
				_DebugStep(node,FALSE);

				CBehaviorGraphPad *pad=node->_lpad->pad;
				if (node->_lpad->refs.size()>0)
					node->_ResolvePad(pad);
				node->Start(0,_outputs);
				if (node->_result==A_Pending)
				{
					//Pending的话需要把resolve过的pad复制一份
					if (node->_lpad->refs.size()>0)
					{
						node->_padCache=(CBehaviorGraphPad*)pad->GetClass()->New();
						node->_padCache->GetGObj()->Copy(pad->GetGObj());
					}
				}
				break;
			}
			case BGNop::Break:
			{
				_DebugStep(node,TRUE);
				_BreakNode(node,_outputs);
				node->_result=A_Ok;//强制为A_Ok
				break;
			}
			case BGNop::StartPending:
				node->StartPending(0);
				assert(node->_result==A_Pending);
				break;
			case BGNop::Update:
				node->Update(_outputs);
				break;
			case BGNop::RewindOk:
				node->RewindOk(nop.keyRewind,_outputs);
				break;
			case BGNop::RewindFail:
				node->RewindFail(nop.keyRewind,_outputs);
				break;
		}

		//看看有没有要开始的新的Relay
		BOOL bRelay=FALSE;
		if (nop.op!=BGNop::Break)
		{
			if (_outputs.idRelay!=PadID_Null)
			{
				BGPad *lpadNew=_bg->LPadFromPadID(_outputs.idRelay);
				if (lpadNew)
				{
					CBehaviorGraphNode *nodeNew=_CreateNode(lpadNew,_outputs.thrdRelay);
					if (nodeNew)
					{
						BGNop nopNew(nodeNew,BGNop::Start);
						nops.push_front(nopNew);
						bRelay=TRUE;
					}
				}
			}
		}


		//看看有没有要开始的新的state
		if ((nop.op!=BGNop::Break)&&(!bFinalizing))
		{
			for (int i=_outputs.idsNewState.size()-1;i>=0;i--)
			{
				BGPad *lpadNew=_bg->LPadFromPadID(_outputs.idsNewState[i]);
				if (lpadNew)
				{
					BgnThread thrd;
					thrd.padState=lpadNew->pad->GetID();
					thrd.idNode=BgnID_Invalid;
					CBehaviorGraphNode *nodeNew=_CreateNode(lpadNew,thrd);
					if (nodeNew)
					{
						//我们看看这个state是不是在同一帧里已经被切换过了,如果已经切换了
						//我们要让这个state开始后处于pending状态,这样做是为了避免同一帧内在state
						//之间反复切换造成的死循环

						BOOL bSwitched=FALSE;
						if (TRUE)
						{
							std::unordered_set<PadID>::iterator it=_statesSwitched.find(_outputs.idsNewState[i]);
							if (it!=_statesSwitched.end())
								bSwitched=TRUE;
						}

						if (!bSwitched)
						{
							BGNop nopNew(nodeNew,BGNop::Start);
							nops.push_back(nopNew);
							_statesSwitched.insert(_outputs.idsNewState[i]);//记录为已经Switch过了
						}
						else
						{
							BGNop nopNew(nodeNew,BGNop::StartPending);
							nops.push_back(nopNew);
						}
					}
				}
			}
		}

		//outputs
		if (TRUE)
		{
			AResult result=(AResult)node->_result;
			ret=result;
			if ((result==A_Fail)||(result==A_Ok))
			{//如果node执行结束了(成功or失败)

				BOOL bOk=(result==A_Ok);
				_DebugStep(node,FALSE,result);

				if (nop.op!=BGNop::Break)
				{
					if (_outputs.nOutputs>0)
					{//有Output,继续执行
						if (!_MakeOutputs(node,_outputs,nops))
							_MakeRewind(node,bOk,nops);//Output上没有链接,要尝试Rewind
					}
					else
					{//没有Output,看看要不要Rewind
						if (!bRelay)
							_MakeRewind(node,bOk,nops);
					}
				}
				else
				{
					//这个node被break了,不需要Rewind
					_MakeOutputs(node,_outputs,nops);
				}

				//Destroy这个node
				_DestroyNode(node);
				_DeleteNotInPending(node);
				node=NULL;
			}
			if (result==A_Pending)
			{//node还在执行中

				//先加到队列中去
				if (!node->_bInPending)
				{
					_pendings.push_back(node);
					node->_bInPending=1;
				}

				if (_outputs.nOutputs>0)
				{
					if (!_MakeOutputs(node,_outputs,nops))
					{
						//Outputs 上没有链接,看看要不要直接Rewind
						for (int i=0;i<_outputs.nOutputs;i++)
						{
							if (_outputs.thrds[i].keyRewind!=BGNTHREAD_INVALID_REWINDKEY)
							{//有Rewind信息
								CBehaviorGraphNode *nodeRewind=NodeFromNodeID(_outputs.thrds[i].idNode);
								if (nodeRewind)
								{
									BGNop nop(nodeRewind,BGNop::RewindOk);
									nop.keyRewind=_outputs.thrds[i].keyRewind;
									nops.push_front(nop);//还是push_back(..)?
								}
							}
						}
					}
				}
			}
		}

		//打断
		if (!bFinalizing)
		{
			if (_outputs.thrdsBreak.size()>0)
			{
				for (int j=_outputs.thrdsBreak.size()-1;j>=0;j--)
				{
					for (int i=0;i<_pendings.size();i++)
					{
						CBehaviorGraphNode *node=_pendings[i];
						if (!node)
							continue;
						if (node->_thrd.Equals(_outputs.thrdsBreak[j]))
						{
							BGNop nop(node,BGNop::Break);
							nops.push_front(nop);

							//remove from pending
							node->_bInPending=0;
							_pendings[i]=NULL;
						}
					}

					//将nop转换成break,从头部加入
					int nNops=nops.size();
					for (int i=nNops-1;i>=0;i--)
					{
						int idx=nops.size()-1-i;
						BGNop &nopCur=nops[idx];
						if (nopCur.op==BGNop::None)
							continue;

						CBehaviorGraphNode *node=nopCur.node;
						if (node->_thrd.Equals(_outputs.thrdsBreak[j]))
						{
							switch(nopCur.op)
							{
								case BGNop::Start:
								case BGNop::StartPending:
								{
									//拥有node,需要删除node
									_DeleteNotInPending(node);
									nopCur.op=BGNop::None;
									nopCur.node=NULL;
									break;
								}
								case BGNop::RewindOk:
								case BGNop::RewindFail:
								{
									//不拥有node,不需要删除node

									nopCur.op=BGNop::None;
									nopCur.node=NULL;
									break;
								}
								case BGNop::Update:
								{
									//转换成break,从头部加入
									nopCur.op=BGNop::None;
									nopCur.node=NULL;

									BGNop nop(node,BGNop::Break);
									nops.push_front(nop);//从头部加入

									break;
								}
							}
						}
					}

				}
			}
		}
	}

	//清除掉那些已经无效的node
	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_pendings.size();i++)
		{
			if (_pendings[i]&&_pendings[i]->IsValid())
			{
				if (_pendings[i]->_IsFinalizing())
				{
					assert(FALSE);
					LOG_DUMP_1P("BehaviorGraph",Log_Error,"行为图(%s)中发现Pending的Finalizing的Node!",StrLib_GetStr(GetName()));
				}
				_pendings[c]=_pendings[i];
				c++;
			}
			else
			{
				Safe_Class_Delete(_pendings[i]);
			}
		}
		_pendings.resize(c);
	}

	_outputs.Clear();

	return ret;
}


void CBehavior::Update()
{
	if (!_bg)
		return;

	_iFrame++;

	static std::deque<BGNop>nops;
	for (int i=0;i<_pendings.size();i++)
	{
		_pendings[i]->_bInPending=FALSE;
		nops.push_back(BGNop(_pendings[i],BGNop::Update));
	}
	_pendings.clear();

	_FlushOps(nops);

	if (TRUE)
	{
		CBehaviorGraphs *bgs = _bg->GetOwner();
		if (bgs)
		{
			CBehaviorDebug *dbg = bgs->GetDebug();
			if (dbg)
			{
				_dataFrameTemp.key.nmBG= _bg->GetName();
				_dataFrameTemp.key.obj=_objOwner;
				_dataFrameTemp.pendings.resize(_pendings.size());
				for (int i = 0;i < _pendings.size();i++)
				{
					PadID idPad = PadID_Null;;
					CBehaviorGraphPad *pad = _pendings[i]->GetPad();
					if (pad)
						idPad = pad->GetID();
					_dataFrameTemp.pendings[i] = idPad;
				}
				dbg->SetFrameData(_dataFrameTemp);
			}
		}
	}



}

BOOL CBehavior::IsPadLocked(PadID idPad)
{
	std::unordered_set<PadID>::iterator it=_locks.find(idPad);
	if (it!=_locks.end())
		return TRUE;
	return FALSE;
}

BOOL CBehavior::LockPad(PadID idPad)
{
	std::unordered_set<PadID>::iterator it=_locks.find(idPad);
	if (it!=_locks.end())
		return FALSE;
	_locks.insert(idPad);
	return TRUE;
}

void CBehavior::UnLockPad(PadID idPad)
{
	std::unordered_set<PadID>::iterator it=_locks.find(idPad);
	if (it==_locks.end())
		return;
	_locks.erase(it);
}


StringID CBehavior::GetName()
{
	if (_bg)
		return _bg->GetName();
	return StringID_Invalid;
}


void CBehavior::_DebugStep(CBehaviorGraphNode *node,BOOL bBreaking,AResult result)
{
	if (_bg)
	{
		if (_bg->GetName()!=StringID_Invalid)
		{
			CBehaviorGraphs *bgs=_bg->GetOwner();
			if (bgs)
			{
				CBehaviorDebug *dbg=bgs->GetDebug();
				if (dbg)
				{
					BehaviorDebugStep step;
					step.nmBG=_bg->GetName();
					step.result=result;
					CBehaviorGraphPad *pad=node->_GetPad();
					if (pad)
					{
						step.idPad=pad->GetID();
						step.bBreaking=bBreaking;
						dbg->Step(step,_objOwner);
					}
				}
			}
		}
	}
}


void CBehavior::SetNodeData(PadID id,unsigned __int64 data)
{
	_dataNodes[id]=data;
}

BOOL CBehavior::GetNodeData(PadID id,unsigned __int64 &data)
{
	data=0;
	std::unordered_map<PadID,unsigned __int64>::iterator it=_dataNodes.find(id);
	if (it!=_dataNodes.end())
	{
		data=(*it).second;
		return TRUE;
	}

	return FALSE;
}
