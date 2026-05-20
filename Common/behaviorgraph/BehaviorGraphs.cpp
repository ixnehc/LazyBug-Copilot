/********************************************************************
	created:	2012/11/21 
	author:		cxi
	
	purpose:	BehaviorGraph
*********************************************************************/
#include "stdh.h"
#include "BehaviorGraphs.h"


#include "BehaviorGraphPads.h"
#include "BgnRelay.h"
//#include "BgnController.h"
#include "BgnHelper.h"
#include "BgnState.h"
#include "BgpFunc.h"
#include "BgnInclude.h"

#include "../commondefines/general_stl.h"

#include "../log/LogDump.h"

#include "../resdata/ResDataDefines.h"
#include <fstream>

#include "Behavior.h"

#include "BehaviorDebug.h"

//////////////////////////////////////////////////////////////////////////
//BgpClasses
WORD BgpClasses::UIDFromClassName(const char *nmClass)
{
	WORD uid=0;
	CLinkPad *pad=New(nmClass);
	if (pad)
	{
		GObjBase *gobj=pad->GetGObj();
		if (gobj)
		{
			uid=gobj->GetUID();
			if (uid<GOBJ_UID_START)
				uid=0;
		}
		Class_Delete(pad);
	}
	return uid;
}



////////////////////////////////////////////////////////////////////////
//BGPad
void BGPad::Clear()
{
	stbOthers.clear();

	Zero();
}

void BGPad::AddOther(DWORD iStub,BGPad *padOther,DWORD iStubOther)
{
	while(stbOthers.size()<=iStub)
		stbOthers.push_back(StbOther());

	stbOthers[iStub]=StbOther(padOther,iStubOther);
}

////////////////////////////////////////////////////////////////////////
//CBehaviorGraph
void CBehaviorGraph::Clear()
{
	for (int i=0;i<_lpads.size();i++)
	{
		BGPad *lpad=_lpads[i];
		if (lpad)
		{
			lpad->Clear();
			Safe_Class_Delete(lpad);
		}
	}
	_lpads.clear();
	_heapStates.clear();
	_states.clear();
	_relays.clear();
	_counters.clear();
	_regs.clear();

	if (_mems.size()>0)
	{
		Safe_Class_Delete(_mems[0]);//只有第0个是属于这个Behavior的
	}
	_mems.clear();

	if (_pads)
	{
		_pads->Clear();
		Safe_Class_Delete(_pads);
	}
	Zero();
}

BGPad *CBehaviorGraph::LPadFromPadID(PadID id)
{
	std::unordered_map<PadID,BGPad *>::iterator it=_lookupLPad.find(id);
	if (it==_lookupLPad.end())
		return NULL;
	return (*it).second;
}

BGPad *CBehaviorGraph::LPadFromPad(CLinkPad*pad)
{
	return LPadFromPadID(pad->GetID());
}

BGPad *CBehaviorGraph::LPadFromStateName(StringID nmState)
{
	std::unordered_map<StringID,BGPad *>::iterator it=_states.find(nmState);
	if (it==_states.end())
		return NULL;
	return (*it).second;
}

PadID CBehaviorGraph::PadIDFromStateName(StringID nmState)
{
	BGPad *lpad=LPadFromStateName(nmState);
	if (lpad)
	{
		if (lpad->pad)
			return lpad->pad->GetID();
	}
	return PadID_Null;
}

PadID CBehaviorGraph::PadIDFromRelayName(StringID nmRelay)
{
	BGPad *lpad=LPadFromRelayName(nmRelay);
	if (lpad)
	{
		if (lpad->pad)
			return lpad->pad->GetID();
	}
	return PadID_Null;
}



BGPad *CBehaviorGraph::LPadFromRelayName(StringID nmRelay)
{
	std::unordered_map<StringID,BGPad *>::iterator it=_relays.find(nmRelay);
	if (it==_relays.end())
		return NULL;
	return (*it).second;
}

CBgp_Timer*CBehaviorGraph::FindTimer(StringID nm)
{
	std::unordered_map<StringID,CBgp_Timer*>::iterator it=_timers.find(nm);
	if (it==_timers.end())
		return NULL;

	return (*it).second;
}

void ReverseFixStbOther(BGPad *pad,DWORD iStub)
{
	if (pad)
	{
		if (iStub<pad->stbOthers.size())
		{
			BGPad *padOther=pad->stbOthers[iStub].pad;
			DWORD iStubOther=pad->stbOthers[iStub].iStb;
			if (padOther)
				padOther->AddOther(iStubOther,pad,iStub);
		}
	}
}

void CBehaviorGraph::ResolveInclude(PadID id,CBehaviorGraph *bgSub)
{
	int nLPadOld=_lpads.size();
	int nMemOld=_mems.size();

	VEC_APPEND(_mems,bgSub->_mems);

	std::map<BGPad *,BGPad*>remap;
	std::unordered_map<StringID,BGPad*> stubins;//bgSub里的stubin
	std::unordered_map<StringID,BGPad*> stubouts;//bgSub里的stubout
	for (int i=0;i<bgSub->_lpads.size();i++)
	{
		BGPad *lpad=bgSub->_lpads[i];
		if (!lpad)
			continue;

		CBehaviorGraphPad *pad=lpad->pad;
		if (!pad)
			continue;

		PadID idNew=_pads->NewPad(pad->GetClass()->GetName(),i_math::pos2di());
		if (idNew==PadID_Null)
			continue;

		CLinkPad *padNew=_pads->FindPad(idNew);
		padNew->_name=pad->_name;
		padNew->GetGObj()->Copy(pad->GetGObj());

		BGPad *lpadNew=Class_New2(BGPad);
		lpadNew->pad=(CBehaviorGraphPad*)padNew;
		lpadNew->clssNode=lpad->clssNode;
		lpadNew->stbOthers=lpad->stbOthers;
		lpadNew->idxMem=lpad->idxMem;
		if (lpadNew->idxMem!=-1)
			lpadNew->idxMem+=nMemOld;

		remap[lpad]=lpadNew;

		if (TRUE)
		{
			if (padNew->GetClass()->CheckName("CBgp_StubIn"))
			{
				CBgp_StubIn *padStubIn=(CBgp_StubIn *)padNew;
				if (padStubIn->_nm!=StringID_Invalid)
				{
					if (stubins.find(padStubIn->_nm)!=stubins.end())
					{
						LOG_DUMP_2P("CBehaviorGraph",Log_Error,"行为图(%s)中发现重复的StubIn名称:%s",
							StrLib_GetStr(bgSub->GetName()),StrLib_GetStr(padStubIn->_nm));
					}
					else
						stubins[padStubIn->_nm]=lpadNew;
				}
			}
			if (padNew->GetClass()->CheckName("CBgp_StubOut"))
			{
				CBgp_StubOut*padStubOut=(CBgp_StubOut*)padNew;
				if (padStubOut->_nm!=StringID_Invalid)
				{
					if (stubouts.find(padStubOut->_nm)!=stubouts.end())
					{
						LOG_DUMP_2P("CBehaviorGraph",Log_Error,"行为图(%s)中发现重复的StubOut名称:%s",
							StrLib_GetStr(bgSub->GetName()),StrLib_GetStr(padStubOut->_nm));
					}
					else
						stubouts[padStubOut->_nm]=lpadNew;
				}
			}
		}

		_lpads.push_back(lpadNew);
		_lookupLPad[idNew]=lpadNew;
	}

	//修正新加入的lpad里的stbOther里的指针
	for (int i=nLPadOld;i<_lpads.size();i++)
	{
		BGPad *lpad=_lpads[i];
		for (int j=0;j<lpad->stbOthers.size();j++)
		{
			StbOther *p=&lpad->stbOthers[j];
			if(p->pad)
			{
				assert(remap.find(p->pad)!=remap.end());
				p->pad=remap[p->pad];
			}
		}
	}

	//替换bgSub中StubIn/StubOut的链接
	if (TRUE)
	{
		BGPad *lpad=LPadFromPadID(id);
		if (lpad)
		{
			CBehaviorGraphPad *pad=lpad->pad;
			if (pad->GetClass()->CheckName("CBgp_Include"))
			{
				CBgp_Include *padInclude=(CBgp_Include *)pad;
				if (padInclude->_nm!=StringID_Invalid)
				{
					DWORD iStub=0;
					for (int i=0;i<padInclude->_stubin.size();i++)
					{
						if (iStub<lpad->stbOthers.size())
						{
							StbOther &stbOther=lpad->stbOthers[iStub];
							std::unordered_map<StringID,BGPad*>::iterator it=stubins.find(padInclude->_stubin[i]);
							if (it==stubins.end())
							{
								LOG_DUMP_2P("CBehaviorGraph",Log_Error,"行为图(%s)中的Include(%s)接口不匹配!",
									StrLib_GetStr(bgSub->GetName()),StrLib_GetStr(padInclude->_nm));
							}
							else
							{
								BGPad *lpadStubIn=(*it).second;
								if (lpadStubIn)
								{
									if (stbOther.pad)
									{
										if (lpadStubIn->stbOthers.size()>0)
										{
											stbOther.pad->stbOthers[stbOther.iStb]=lpadStubIn->stbOthers[0];
											ReverseFixStbOther(stbOther.pad,stbOther.iStb);
										}
										else
											stbOther.pad->stbOthers[stbOther.iStb].Zero();
									}
								}
							}
						}
						iStub++;
					}
					for (int i=0;i<padInclude->_stubout.size();i++)
					{
						if (iStub<lpad->stbOthers.size())
						{
							StbOther &stbOther=lpad->stbOthers[iStub];
							std::unordered_map<StringID,BGPad*>::iterator it=stubouts.find(padInclude->_stubout[i]);
							if (it==stubouts.end())
							{
								LOG_DUMP_2P("CBehaviorGraph",Log_Error,"行为图(%s)中的Include(%s)接口不匹配!",
									StrLib_GetStr(bgSub->GetName()),StrLib_GetStr(padInclude->_nm));
							}
							else
							{
								BGPad *lpadStubOut=(*it).second;
								if (lpadStubOut)
								{
									if (stbOther.pad)
									{
										if (lpadStubOut->stbOthers.size()>0)
										{
											stbOther.pad->stbOthers[stbOther.iStb]=lpadStubOut->stbOthers[0];
											ReverseFixStbOther(stbOther.pad,stbOther.iStb);
										}
										else
											stbOther.pad->stbOthers[stbOther.iStb].Zero();
									}
								}
							}
						}
						iStub++;
					}
				}
			}
		}
	}

}

BOOL CBehaviorGraph::_CheckAncestorState(BGPad *lpad,BGPad *lpadAncestor)
{
	if (lpadAncestor->idxInStateHeap==-1)
		return FALSE;
	while(lpad->lpadOwnerState)
	{
		if (lpad->lpadOwnerState==lpadAncestor)
			return TRUE;
		if (lpad->lpadOwnerState->idxInStateHeap<lpadAncestor->idxInStateHeap)
			break;
		lpad=lpad->lpadOwnerState;
	}
	return FALSE;
}


BOOL CBehaviorGraph::FindStateSwitch(PadID idSrcState,PadID idDestState,std::vector<PadID>*&breaks,std::vector<PadID>*&starts)
{
	breaks=NULL;
	starts=NULL;
	_breaksSwitchState.clear();
	_startsSwitchState.clear();

	BGPad *lpadSrc,*lpadDest;
	lpadSrc=LPadFromPadID(idSrcState);
	lpadDest=LPadFromPadID(idDestState);

	if (!lpadDest)
		return FALSE;

	if (lpadSrc)
	{
		if (!lpadSrc->pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
			return FALSE;
	}
	if (!lpadDest->pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
		return FALSE;

	if (lpadSrc==lpadDest)
		return FALSE;

	if (lpadSrc)
	{
		if (_CheckAncestorState(lpadSrc,lpadDest))
			return FALSE;//不能切换到祖先
	}

	if (lpadSrc&&(!_CheckAncestorState(lpadDest,lpadSrc)))
	{//两者拥有共同的ancestor
		BGPad *lpadToBreak=lpadSrc;
		while(1)
		{
			if (!lpadToBreak->lpadOwnerState)
				break;
			if (_CheckAncestorState(lpadDest,lpadToBreak->lpadOwnerState))
				break;
			lpadToBreak=lpadToBreak->lpadOwnerState;
		}

		for (int i=_heapStates.size()-1;i>=0;i--)
		{
			BGPad *lpadState=_heapStates[i];
			if (lpadState==lpadToBreak)
			{
				_breaksSwitchState.push_back(lpadState->pad->GetID());
				break;
			}

			if (_CheckAncestorState(lpadState,lpadToBreak))
				_breaksSwitchState.push_back(lpadState->pad->GetID());
		}

		BGPad *lpadToStart=lpadDest;
		_startsSwitchState.push_back(lpadToStart->pad->GetID());

		while(lpadToStart->lpadOwnerState!=lpadToBreak->lpadOwnerState)
		{
			lpadToStart=lpadToStart->lpadOwnerState;
			_startsSwitchState.push_back(lpadToStart->pad->GetID());
		}
	}
	else
	{
		//切换到后代
		for (int i=_heapStates.size()-1;i>=0;i--)
		{
			BGPad *lpadState=_heapStates[i];
			if (lpadState==lpadSrc)
				break;

			if (lpadState==lpadDest)
				continue;

			if ((!lpadSrc)||_CheckAncestorState(lpadState,lpadSrc))
			{//是src的后代
				if (lpadState->idxInStateHeap<lpadDest->idxInStateHeap)
				{//肯定不是Dest的后代
					if (!_CheckAncestorState(lpadDest,lpadState))
					{//不是Dest的祖先
						_breaksSwitchState.push_back(lpadState->pad->GetID());
					}
				}
				else
				{//有可能是Dest的后代
					if (!_CheckAncestorState(lpadDest,lpadState))
					{//不是Dest的祖先
						if (!_CheckAncestorState(lpadState,lpadDest))
						{//不是Dest的后代
							_breaksSwitchState.push_back(lpadState->pad->GetID());
						}
					}
				}
			}
		}

		BGPad *lpadToStart=lpadDest;
		_startsSwitchState.push_back(lpadToStart->pad->GetID());

		while(lpadToStart->lpadOwnerState!=lpadSrc)
		{
			lpadToStart=lpadToStart->lpadOwnerState;
			_startsSwitchState.push_back(lpadToStart->pad->GetID());
		}
	}

	breaks=&_breaksSwitchState;
	starts=&_startsSwitchState;

	return TRUE;
}

BOOL CBehaviorGraph::FindStateActivate(PadID idDestState,std::vector<PadID>*&starts)
{
	starts=NULL;
	_startsSwitchState.clear();

	BGPad *lpadDest;
	lpadDest=LPadFromPadID(idDestState);

	if (!lpadDest)
		return FALSE;

	if (!lpadDest->pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
		return FALSE;

	BGPad *lpadToStart=lpadDest;
	_startsSwitchState.push_back(lpadToStart->pad->GetID());

	while(lpadToStart->lpadOwnerState!=NULL)
	{
		lpadToStart=lpadToStart->lpadOwnerState;
		_startsSwitchState.push_back(lpadToStart->pad->GetID());
	}

	starts=&_startsSwitchState;

	return TRUE;
}


PadID CBehaviorGraph::GetOwnerState(PadID id)
{
	std::unordered_map<PadID,BGPad *>::iterator it=_lookupLPad.find(id);
	if (it==_lookupLPad.end())
		return PadID_Null;

	BGPad *lpad=(*it).second;
	if (lpad)
	{
		if (lpad->lpadOwnerState)
		{
			if (lpad->lpadOwnerState->pad)
				return lpad->lpadOwnerState->pad->GetID();
		}
	}

	return PadID_Null;
}


//////////////////////////////////////////////////////////////////////////
//BgpClasses
struct BgpClasses;
extern BgpClasses *BgpClasses_GetSingleton();

BgnClassRegister::BgnClassRegister(CClass *clssPad,CClass *clssNode)
{
	extern BgpClasses *BgpClasses_GetSingleton();
	BgpClasses_GetSingleton()->Add(clssPad->GetName(),clssPad,clssNode);
	CLinkPad *pad=(CLinkPad *)clssPad->New();
	if (pad)
	{
		GObjBase *base=pad->GetGObj();
		if (base)
		{
			WORD uid=base->GetUID();
			if (uid>=GOBJ_UID_START)
			{
				BgpClasses_GetSingleton()->Add(uid,clssPad,clssNode);
			}
		}
		Class_Delete(pad);
	}
}


////////////////////////////////////////////////////////////////////////
//CBehaviorGraphs
BOOL CBehaviorGraphs::_LoadBGPadsFromData(BYTE *data,CBehaviorGraphPads &pads,LinkPadClasses *clsses)
{
	CDataPacket dp;
	dp.SetDataBufferPointer(data);

	WORD verData=dp.Data_NextWord();

	pads.SetClasses(clsses);
	pads.Load(dp);

	return TRUE;
}


// #define ADD_ENTRY(clssNode)							\
// 	extern CClass *GetClass_##clssNode();				\
// 	extern CClass *GetPadClass_##clssNode();					\
// 	clsses.Add(GetPadClass_##clssNode()->GetName(),GetPadClass_##clssNode(),GetClass_##clssNode());

BgpClasses *BgpClasses_GetSingleton()
{
	static BgpClasses clsses;
	return &clsses;
}


CClass * BGNodeClassFromPad(CLinkPad *pad,LinkPadClasses *clsses)
{
	return ((BgpClasses *)clsses)->FindNodeClass(pad->GetClass()->GetName());
}


BOOL CBehaviorGraphs::_CompileBG(CBehaviorGraphPads *pads)
{
	CBehaviorGraph *bg=Class_New2(CBehaviorGraph);

	bg->_owner=this;
	bg->_pads=pads;

	CBehaviorMemDesc *mem=Class_New2(CBehaviorMemDesc);
	bg->_mems.push_back(mem);

	//先载入所有的Pad
	if (TRUE)
	{
		BOOL bAnyState=FALSE;

		DWORD c;
		CLinkPad **buf=pads->GetPads(c);

		bg->_lpads.reserve(c);
		for (int i=0;i<c;i++)
		{
			CLinkPad *pad=buf[i];

			if (TRUE)
			{
				CBehaviorGraphPad *bgp=(CBehaviorGraphPad *)pad;
				if (!bgp->IsEnabled())
					continue;
			}

			//记录下这个BG的名称
			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Graph)))
			{
				CBgp_Graph *padName=(CBgp_Graph *)pad;
				if (padName->_nm!=StringID_Invalid)
				{
					if (_lookup.find(padName->_nm)!=_lookup.end())
					{
						assert(FALSE);
						LOG_DUMP_1P("LevelBG",Log_Error,"多个BehaviorGraph使用了同一个名称(%s)",StrLib_GetStr(padName->_nm));
						continue;
					}

					_lookup[padName->_nm]=bg;
					bg->_nm=padName->_nm;

					if (TRUE)
					{
						BGPad *lpad=Class_New2(BGPad);
						lpad->pad=(CBehaviorGraphPad *)pad;
						lpad->idxMem=0;
						bg->_lpads.push_back(lpad);
						bg->_lookupLPad[pad->GetID()]=lpad;
						bg->_def=lpad;
					}
				}
				continue;
			}

			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Counter)))
			{
				CBgp_Counter*padCounter=(CBgp_Counter*)pad;
				if (padCounter->_nm!=StringID_Invalid)
				{
					if (bg->_counters.find(padCounter->_nm)!=bg->_counters.end())
					{
						LOG_DUMP_1P("LevelBG",Log_Error,"定义了多个计数器(%s)",StrLib_GetStr(padCounter->_nm));
						continue;
					}

					bg->_counters[padCounter->_nm]=padCounter;
				}
				continue;
			}

			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Register)))
			{
				CBgp_Register*padReg=(CBgp_Register*)pad;
				if (padReg->_nm!=StringID_Invalid)
				{
					if (bg->_regs.find(padReg->_nm)!=bg->_regs.end())
					{
						LOG_DUMP_1P("LevelBG",Log_Error,"定义了多个寄存器(%s)",StrLib_GetStr(padReg->_nm));
						continue;
					}

					bg->_regs[padReg->_nm]=padReg;
				}
				continue;
			}

			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Vars)))
			{
				CBgp_Vars*padVars=(CBgp_Vars*)pad;

				for (int i=0;i<padVars->_declares2.size();i++)
				{
					BhvVarDeclare *declare=&padVars->_declares2[i];
					if (declare->nm!=StringID_Invalid)
					{
						if (mem->Exist(declare->nm))
						{
							LOG_DUMP_2P("LevelBG",Log_Error,"行为图(%s)内变量名称重复(%s)",StrLib_GetStr(bg->_nm),StrLib_GetStr(declare->nm));
							continue;
						}

						switch(declare->tp)
						{
							case BehaviorMemType_Bit:
								mem->AddBit(declare->nm,declare->_flag,declare->b);
								break;
							case BehaviorMemType_Integer:
								mem->AddNumber(declare->nm,declare->_flag,declare->n);
								break;
							case BehaviorMemType_StringID:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idStr);
								break;
							case BehaviorMemType_SkillRecord:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idSkill);
								break;
							case BehaviorMemType_BuffRecord:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idBuff);
								break;
							case BehaviorMemType_ItemRecord:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idItem);
								break;
							case BehaviorMemType_UnitRecord:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idUnit);
								break;
							case BehaviorMemType_ResourceRecord:
								mem->AddID(declare->nm,declare->tp,declare->_flag,declare->idRes);
								break;
							case BehaviorMemType_ObjID:
							case BehaviorMemType_GUID:
								mem->AddID(declare->nm,declare->tp,declare->_flag,0);
								break;
							case BehaviorMemType_Pos:
								mem->AddPos(declare->nm,declare->_flag);
								break;
							case BehaviorMemType_Float:
								mem->AddFloat(declare->nm,declare->_flag,declare->f);
								break;
							case BehaviorMemType_Obj:
								mem->AddObj(declare->nm,declare->_flag);
								break;
								//XXXXX:more BehaviorMemType
						}
					}
				}
				continue;
			}

			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Timer)))
			{
				CBgp_Timer*padTimer=(CBgp_Timer*)pad;
				if (padTimer->_nm!=StringID_Invalid)
				{
					if (bg->_timers.find(padTimer->_nm)!=bg->_timers.end())
					{
						assert(FALSE);
						LOG_DUMP_1P("LevelBG",Log_Error,"定义了多个计时器(%s)",StrLib_GetStr(padTimer->_nm));
						continue;
					}

					bg->_timers[padTimer->_nm]=padTimer;
				}
				continue;
			}

			//修复调用参数
			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Call)))
			{
				CBgp_Call*padCall=(CBgp_Call*)pad;

				CBgp_Func *padFunc=pads->FindFunc(padCall->_nm);
				if (padFunc)
				{
					padCall->_call._paramsRT.clear();
					padCall->_valuesDef.clear();

					padCall->_valuesDef.resize(padFunc->_declares2.size());

					for (int i=0;i<padFunc->_declares2.size();i++)
					{
						padFunc->_declares2[i].AssignDefault(padCall->_valuesDef[i]);

						BhvVal *valueDef=&padCall->_valuesDef[i];
						StringID nm=padFunc->_declares2[i].nm;
						if (nm!=StringID_Invalid)
						{
							BhvVal *valueRT=valueDef;
							BhvVal *value=padCall->_params.Find(nm);
							if (value)
							{
								if (valueDef->IsCompatible(*value))
									valueRT=value;
							}

							padCall->_call._paramsRT[nm]=valueRT;
						}
					}

					padCall->_call._padFunc=padFunc;
				}
			}



			//根据这个pad创建一个BGPad
			BGPad *lpad=Class_New2(BGPad);
			lpad->pad=(CBehaviorGraphPad *)pad;
			lpad->idxMem=0;

			//记录State的查找表
			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
			{
				CBgp_State*pad2=(CBgp_State*)pad;
				if (pad2->_nm==StringID_Invalid)
					continue;

				bg->_states[pad2->_nm]=lpad;
				bAnyState=TRUE;

				if (pad2->_flag!=0)
					mem->AddState(pad->GetID(),pad2->_nm,pad2->_flag);
			}

			//记录Relay的查找表
			if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_Relay)))
			{
				CBgp_Relay*pad2=(CBgp_Relay*)pad;
				if (pad2->_nm==StringID_Invalid)
					continue;

				bg->_relays[pad2->_nm]=lpad;
			}

			bg->_lpads.push_back(lpad);
			bg->_lookupLPad[pad->GetID()]=lpad;
		}

		if (bg->_nm==StringID_Invalid)
		{//没有被人用到
			bg->Clear();
			Safe_Class_Delete(bg);
			return FALSE;
		}

		if (!bAnyState)
		{
//			LOG_DUMP_1P("LevelBG",Log_Warning,"在行为图(%s)中没有发现任何一个State",StrLib_GetStr(bg->_nm));
		}
	}

	//链接它们
	if (TRUE)
	{
		DWORD c;
		CLinkPads::Link*links=pads->GetLinks(c);

		for (int i=0;i<c;i++)
		{
			CLinkPads::Link *link=&links[i];

			CLinkPad *padSrc=pads->GetPad(link->iPad[0]);
			CLinkPad *padTarget=pads->GetPad(link->iPad[1]);

			if (!(padSrc&&padTarget))
				continue;

			BGPad *lpadSrc=bg->LPadFromPad(padSrc);
			BGPad *lpadTarget=bg->LPadFromPad(padTarget);
			if (!(lpadSrc&&lpadTarget))
				continue;

			PadStub stbSrc=padSrc->GetStub(link->iStub[0]);
			PadStub stbTarget=padTarget->GetStub(link->iStub[1]);

			if (TRUE)
			{
				lpadSrc->AddOther(link->iStub[0],lpadTarget,link->iStub[1]);
				lpadTarget->AddOther(link->iStub[1],lpadSrc,link->iStub[0]);
			}
			else
			{
				if (stbSrc.type==PadStub_Out)
					lpadSrc->AddOther(link->iStub[0],lpadTarget,link->iStub[1]);
				else
				{
					if (stbSrc.type==PadStub_COut)
						lpadSrc->AddOther(link->iStub[0],lpadTarget,link->iStub[1]);
					else
					{
						if (stbTarget.type==PadStub_Out)
							lpadTarget->AddOther(link->iStub[1],lpadSrc,link->iStub[0]);
						else
						{
							if (stbTarget.type==PadStub_COut)
								lpadTarget->AddOther(link->iStub[1],lpadSrc,link->iStub[0]);
						}
					}
				}
			}
		}
	}

	//为所有的Pad填充class指针
	if (TRUE)
	{
		for (int i=0;i<bg->_lpads.size();i++)
		{
			BGPad *lpad=bg->_lpads[i];
			lpad->clssNode=BGNodeClassFromPad(lpad->pad,pads->GetClasses());
			if (!lpad->clssNode)
			{
				LOG_DUMP_1P("LevelBG",Log_Error,"无法为Pad(%s)找到对应的Node",lpad->pad->GetClass()->GetName());
			}
		}
	}

	//检查所有的State是不是Folder
	if (TRUE)
	{
		for (int i=0;i<bg->_lpads.size();i++)
		{
			BGPad *lpad=bg->_lpads[i];
			if (lpad->pad)
			{
				if (lpad->pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
				{
					CBgp_State *padState=(CBgp_State *)lpad->pad;
					if (padState->_nm==StringID_Invalid)
						LOG_DUMP_1P("LevelBG",Log_Error,"BehaviorGraph(%s)中存在没有名字的State",StrLib_GetStr(bg->_nm));
					else
					{
						if (!lpad->pad->IsFolder())
						{
							LOG_DUMP_2P("LevelBG",Log_Error,"BehaviorGraph(%s)中的State(%s)没有Fold!",StrLib_GetStr(bg->_nm),StrLib_GetStr(padState->_nm));
						}
					}
				}
			}
		}
	}

	//记录所有的pad属于哪个State
	if (TRUE)
	{
		for (int i=0;i<bg->_lpads.size();i++)
		{
			BGPad *lpad=bg->_lpads[i];
			if (lpad->pad)
			{
				PadID idFolder=lpad->pad->GetFolder();
				PadID idOwnerState=PadID_Null;
				while(idFolder!=PadID_Null)
				{
					CLinkPad *pad=pads->FindPad(idFolder);
					if (!pad)
						break;
					if (pad->GetClass()->IsSameWith(Class_Ptr2(CBgp_State)))
					{
						idOwnerState=idFolder;
						break;
					}

					idFolder=pad->GetFolder();
				}

				lpad->lpadOwnerState=bg->LPadFromPadID(idOwnerState);
			}
		}
	}

	//构建state的heap
	if (TRUE)
	{
		std::unordered_map<StringID,BGPad *>::iterator it;

		int start,end;
		start=end=0;

		while(1)
		{
			for (it=bg->_states.begin();it!=bg->_states.end();it++)
			{
				BGPad *lpad=(*it).second;
				if (start==end)
				{//第0个level
					if (lpad->lpadOwnerState!=NULL)
						lpad=NULL;//不是第0个level的state的child
				}
				else
				{
					int i;
					for (i=start;i<end;i++)
					{
						if (bg->_heapStates[i]==lpad->lpadOwnerState)
							break;
					}
					if (i>=end)
						lpad=NULL;//不是当前这个level的state的child
				}

				if (lpad)
				{
					bg->_heapStates.push_back(lpad);
					lpad->idxInStateHeap=bg->_heapStates.size()-1;
				}
			}

			//下一个level的state
			start=end;
			end=bg->_heapStates.size();
			if (start==end)
				break;//下一个level没有任何state,退出
		}
	}

	//构建Ref信息
	if (TRUE)
	{
		extern GElemBase *GetBVRElem(GElemBase *elem);
		for (int i=0;i<bg->_lpads.size();i++)
		{
			BGPad *lpad=bg->_lpads[i];
			if (lpad->pad)
			{
				GElemBase *elem=lpad->pad->GetGObj()->GetElems();

				while(elem)
				{
					GElemBase *elemBVR=GetBVRElem(elem);
					if (elemBVR)
					{
						BGPad::RefInfo info;
						info.elem=elem;
						info.idRef=*(StringID*)elemBVR->GetPtr(lpad->pad);
						if ((info.idRef!=StringID_BhvValInvalidRef)&&(info.idRef!=StringID_Invalid))
							lpad->refs.push_back(info);

						elem=elemBVR->next;
					}
					else
						elem=elem->next;
				}
			}
		}
	}

	_bgs.push_back(bg);

	return TRUE;
}

void CBehaviorGraphs::Clear()
{
	for (int i=0;i<_bgs.size();i++)
	{
		CBehaviorGraph *bg=_bgs[i];
		bg->Clear();
		Safe_Class_Delete(bg);
	}
	_bgs.clear();

	_lookup.clear();
	_classesNode.clear();

	Zero();
}


CBehaviorGraph *CBehaviorGraphs::FindBG(StringID nm)
{
	std::unordered_map<StringID,CBehaviorGraph *>::iterator it=_lookup.find(nm);
	if (it==_lookup.end())
		return NULL;
	return (*it).second;
}


