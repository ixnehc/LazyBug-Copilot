/********************************************************************
	created:	2012/11/20 
	author:		cxi
	
	purpose:	a grid to edit the content of a BehaviorGraphData
*********************************************************************/
#include "stdh.h"

#include "BehaviorGraphGrid.h"

#include "RenderSystem/IBehaviorGraph.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderSystem.h"

#include "RenderSystem/IUtilRS.h"

#include "RichGridComboItem.h"
#include "RichGridMatSetItem.h"


#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include "BehaviorGraphEditPanel.h"

#include "resdata/MeshData.h"
#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BgnHelper.h"
#include "behaviorgraph/BgnState.h"
#include "behaviorgraph/BgpFunc.h"
#include "behaviorgraph/BehaviorParam.h"

#include "LoAgentRef.h"

#include "RichGridButtonItem.h"

#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//BhvValuesCache

void BhvValuesCache::Clear()
{
	declares.clear();
	valuesDef.clear();
	for (int i=0;i<binds.size();i++)
	{
		Safe_Class_Delete(binds[i].pad);
	}
	binds.clear();
	items.clear();
}

void BhvValuesCache::LoadBind(CBehaviorGraphPads &pads)
{
	valuesDef.resize(declares.size());
	for (int i=0;i<declares.size();i++)
		declares[i]->AssignDefault(valuesDef[i]);

	//生成bind
	CDataPacket dp;
	binds.resize(valuesDef.size());
	for (int i=0;i<valuesDef.size();i++)
	{
		BhvVal *value=&valuesDef[i];
		BOOL bDef=TRUE;
		if (TRUE)
		{
			BhvVal *valueOverride=values->Find(value->nm);
			if (valueOverride)
			{
				if (valueOverride->IsCompatible(*value))
				{
					value=valueOverride;
					bDef=FALSE;
				}
			}
		}

		BhvValuesCache::Bind bind;
		bind.nm=value->nm;
		bind.bDef=bDef;
		bind.value=value;
		bind.tp=value->tp;
		bind.nmRef=value->nmRef;
		if (bind.nmRef==StringID_BhvValInvalidRef)
		{
			CClass *clss;
			GElemBase *elem;
			if (pads.ResolveBhvValType(value->tp,clss,elem))
			{
				bind.pad=(CBehaviorGraphPad *)clss->New();
				bind.elem=elem;
				dp.SetDataBufferPointer(&value->data[0]);
				elem->Load(bind.pad,dp,TRUE);
			}
		}
		binds[i]=bind;
	}
}

void BhvValuesCache::SaveBind()
{
	std::deque<BhvVal>updates;
	BhvVal value;
	for (int i=0;i<binds.size();i++)
	{
		value=valuesDef[i];
		BhvValuesCache::Bind &bind=binds[i];
		value.nmRef=bind.nmRef;
		if (bind.nmRef==StringID_BhvValInvalidRef)
		{
			if (bind.elem)
			{
				DP_BeginSave(dp,value.data);
				bind.elem->Save(bind.pad,dp,TRUE);
				DP_EndSave();
			}
		}

		if (!(value.Equals(valuesDef[i])))
		{
			updates.resize(updates.size()+1);
			updates[updates.size()-1]=value;
		}
	}

	std::deque<BhvVal> entries;
	entries.resize(updates.size());
	for (int i=0;i<updates.size();i++)
		entries[i]=updates[i];

	values->entries.swap(entries);
	values->lookup.clear();

}


//////////////////////////////////////////////////////////////////////////
//CBehaviorGraphGrid

#define ID_ELEM_REF 8132
#define ID_BHVVAL_REF 8133
#define ID_ELEM_ADDCONST 8137
#define ID_ELEM_ADDPARAM 8138

//////////////////////////////////////////////////////////////////////////
//CBehaviorGraphGrid
BEGIN_MESSAGE_MAP(CBehaviorGraphGrid,CGObjGrid)
	ON_COMMAND(ID_ELEM_REF,OnElemRef)
	ON_COMMAND(ID_ELEM_ADDCONST,OnElemAddConst)
	ON_COMMAND(ID_ELEM_ADDPARAM,OnElemAddParam)
	ON_COMMAND(ID_BHVVAL_REF,OnBhvValRef)
END_MESSAGE_MAP()


void CBehaviorGraphGrid::Bind(ResEditPanelState *state0,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state0,bUpdateCtrl);

	if (bUpdateCtrl)
	{
		BehaviorGraphData *data=GetResData();
		if (!data)
		{
			UnLockPaint();
			return;
		}

		for (int i=0;i<_cachesBhvValues.size();i++)
			_cachesBhvValues[i].Clear();
		_cachesBhvValues.clear();
		_cachesBhvValDeclare.Clear();

		_objs.clear();
		LockPaint();
		RecordState(CGObjGrid::_state);
		ResetContent();
		BeginInsert();

		CXTPPropertyGridItem*item;


		Reps_BehaviorGraph *state=(Reps_BehaviorGraph*)state0;

		if (state->sels.size()==1)
		{
			CLinkPad *pad=data->pads.FindPad(state->sels[0]);
			if (pad)
			{
				_padBind=pad;
				item=InsertCategory(pad->GetTypeName(),"",NULL);
				PushInsert();
				_Bind(pad->GetGObj(),GSem(GSem_Unknown));
				_objs.push_back(pad->GetGObj());
				PopInsert();
				_ExpandItemR(item);
				_padBind=NULL;
			}
		}

		EndInsert();
		RestoreState(CGObjGrid::_state);

		if (state->sels.size()==1)
		{
			std::vector<void*> *ptrsDelta=data->pads.FindDeltaPtrs(state->sels[0]);
			if (ptrsDelta)
				RenderDelta(*ptrsDelta);
		}
		
		UnLockPaint();
	}
}

void CBehaviorGraphGrid::OnBeginItemChange(CXTPPropertyGridItem *item)
{
//	CGObjGrid::OnBeginItemChange(item);
	Reps_BehaviorGraph*state=GetState();
	if (state->sels.size()==1)
	{
		BehaviorGraphData *data=GetResData();
		data->pads.PreModify(state->sels[0]);
	}
	
}

void CBehaviorGraphGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	_BhvValDeclareCache_PreSave(item);

	for (int i=0;i<_cachesBhvValues.size();i++)
		_BhvValueCache_PreSave(&_cachesBhvValues[i]);

	Reps_BehaviorGraph*state=GetState();
	if (state->sels.size()==1)
	{
		BehaviorGraphData *data=GetResData();
		data->pads.PostModify();
	}

	RefreshMod(FALSE);

// 	CGObjGrid::OnEndItemChange(item);
}



void CBehaviorGraphGrid::EnableCtrl(BOOL bActive)
{
	if(bActive)
		SetReadOnly(FALSE);
	else
		SetReadOnly(TRUE);
}

void CBehaviorGraphGrid::_CollectRefConstraints(const char *prefix,BhvValDeclare *declare,BhvValType &tp,std::vector<std::string> &constraints)
{
	if (declare->nm==StringID_Invalid)
		return;

	static std::string s;

	if (!declare->tp.IsCompatible(tp))
		return;

	FormatString(s,"[%s]    %s:%d",prefix,StrLib_GetStr(declare->nm),(int)declare->nm);
	constraints.push_back(s);
}


void CBehaviorGraphGrid::_CollectRefConstraints(const char *prefix,BhvValDeclare *declare,BehaviorMemType tpMem,std::vector<std::string> &constraints)
{
	if (declare->nm==StringID_Invalid)
		return;

	static std::string s;

	if (!BehaviorMemType_IsNumber(tpMem))
	{
		if (declare->tp.tpMem!=tpMem)
			return;
	}
	else
	{
		if (!BehaviorMemType_IsNumber((BehaviorMemType)declare->tp.tpMem))
			return;
	}

	FormatString(s,"[%s]    %s:%d",prefix,StrLib_GetStr(declare->nm),(int)declare->nm);
	constraints.push_back(s);
}

void CBehaviorGraphGrid::_CollectRefConstraints(const char *prefix,BhvVarDeclare *declare,BehaviorMemType tpMem,std::vector<std::string> &constraints)
{
	if (declare->nm==StringID_Invalid)
		return;

	static std::string s;

	if (tpMem!=BehaviorMemType_None)
	{
		if (!BehaviorMemType_IsNumber(tpMem))
		{
			if (declare->tp!=tpMem)
				return;
		}
		else
		{
			if (!BehaviorMemType_IsNumber(declare->tp))
				return;
		}
	}

	FormatString(s,"[%s]    %s:%d",prefix,StrLib_GetStr(declare->nm),(int)declare->nm);
	constraints.push_back(s);
}




void CBehaviorGraphGrid::_CollectRefConstraints(BhvValType &tp,std::vector<std::string> &constraints)
{
	constraints.clear();
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (!data)
		return;

	std::string s;

	if(TRUE)
	{
		if (tp.tpMem==BehaviorMemType_Bit)
		{
//			constraints.push_back(std::string("[Check Day]:1"));
			constraints.push_back(std::string("[FLAG0]:4"));
			constraints.push_back(std::string("[FLAG1]:5"));
			constraints.push_back(std::string("[FLAG2]:6"));
		}

		if (tp.tpMem==BehaviorMemType_Integer)
		{
			constraints.push_back(std::string("[BYTE]:2"));
			constraints.push_back(std::string("[WORD]:3"));
		}
		//XXXXX:more simple var
	}

	CBgp_Func *padFunc=data->pads.FindOwnerFunc((CBehaviorGraphPad*)_padBind);
	if (padFunc)
	{
		for (int i=0;i<padFunc->_declares2.size();i++)
		{
			BhvParamDeclare *declare=&padFunc->_declares2[i];
			_CollectRefConstraints("参数",declare,tp,constraints);
		}
	}

	for (int i=0;i<data->pads.GetPadCount();i++)
	{
		CLinkPad *pad=data->pads.GetPad(i);
		if (!pad)
			continue;
		if(pad->GetClass()->CheckName("CBgp_Vars"))
		{
			CBgp_Vars *vars=(CBgp_Vars*)pad;

			for (int i=0;i<vars->_declares2.size();i++)
			{
				BhvVarDeclare *declare=&vars->_declares2[i];
				_CollectRefConstraints("变量",declare,(BehaviorMemType)tp.tpMem,constraints);
			}
		}

		if(pad->GetClass()->CheckName("CBgp_Consts"))
		{
			CBgp_Consts*var=(CBgp_Consts*)pad;
			for (int i=0;i<var->_declares2.size();i++)
			{
				BhvConstDeclare *declare=&var->_declares2[i];
				_CollectRefConstraints("常量",declare,tp,constraints);
			}
		}
	}
}

void CBehaviorGraphGrid::_CollectRefConstraints(BehaviorMemType tpMem,std::vector<std::string> &constraints)
{
	constraints.clear();
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (!data)
		return;

	std::string s;

	if(TRUE)
	{
		if (BehaviorMemType_IsNumber(tpMem)||(tpMem==BehaviorMemType_None))
		{
			constraints.push_back(std::string("[FLAG0]:4"));
			constraints.push_back(std::string("[FLAG1]:5"));
			constraints.push_back(std::string("[FLAG2]:6"));
			constraints.push_back(std::string("[BYTE]:2"));
			constraints.push_back(std::string("[WORD]:3"));
		}
	}

	for (int i=0;i<data->pads.GetPadCount();i++)
	{
		CLinkPad *pad=data->pads.GetPad(i);
		if (!pad)
			continue;
		if(pad->GetClass()->CheckName("CBgp_Vars"))
		{
			CBgp_Vars *vars=(CBgp_Vars*)pad;

			for (int i=0;i<vars->_declares2.size();i++)
			{
				BhvVarDeclare *declare=&vars->_declares2[i];
				_CollectRefConstraints("变量",declare,tpMem,constraints);
			}
		}
	}
}


void CBehaviorGraphGrid::_CollectStateConstraints(BehaviorGraphData *data,std::vector<std::string> &constraints)
{
	constraints.clear();
	if (!data)
		return;

	std::string s;
	s="[空]:0";
	constraints.push_back(s);

	for (int i=0;i<data->pads.GetPadCount();i++)
	{
		CLinkPad *pad=data->pads.GetPad(i);
		if (!pad)
			continue;
		if(pad->GetClass()->CheckName("CBgp_State"))
		{
			CBgp_State *state=(CBgp_State*)pad;
			if (state->_nm==StringID_Invalid)
				continue;

			FormatString(s,"%s:%d",StrLib_GetStr(state->_nm),(int)state->_nm);
			constraints.push_back(s);
		}
	}
}


void CBehaviorGraphGrid::_CollectStateConstraints(std::vector<std::string> &constraints)
{
	constraints.clear();
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	_CollectStateConstraints(data,constraints);
}

void CBehaviorGraphGrid::_CollectFuncConstraints(std::vector<std::string> &constraints)
{
	constraints.clear();
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;

	if (!data)
		return;

	std::string s;
	s="[空]:0";
	constraints.push_back(s);

	for (int i=0;i<data->pads.GetPadCount();i++)
	{
		CLinkPad *pad=data->pads.GetPad(i);
		if (!pad)
			continue;
		if(pad->GetClass()->CheckName("CBgp_Func"))
		{
			CBgp_Func *func=(CBgp_Func*)pad;
			if (func->_nm==StringID_Invalid)
				continue;

			FormatString(s,"%s:%d",StrLib_GetStr(func->_nm),(int)func->_nm);
			constraints.push_back(s);
		}
	}


}


LoAgentRef *CBehaviorGraphGrid::_FindAgentRef(GObjBase *gobj)
{
	LoAgentRef *refAgent=NULL;
	GElemBase *elem=gobj->GetElems();
	while(elem)
	{
		GObjBase *objSub;
		if (elem->GetObj(gobj->GetOwner(),&objSub))
		{
			if ( ( GSem_Unknown == elem->sem.code) && ( std::string("LoAgentRef") == objSub->GetName() ) )
			{
				return static_cast<LoAgentRef*>( objSub->GetOwner() );
			}

			if (std::string("BPR_CustomObjBase")==objSub->GetName())
			{
				BPR_CustomObjBase*p=static_cast<BPR_CustomObjBase*>( objSub->GetOwner() );
				refAgent=_FindAgentRef(p->GetEmbedGObj());
				if (refAgent)
					return refAgent;
			}
		}
		refAgent=_FindAgentRef(objSub);
		if (refAgent)
			return refAgent;

		elem=elem->next;
	}

	return NULL;
}




void CBehaviorGraphGrid::_CollectAgentStateConstraints(std::vector<std::string> &constraints)
{
	constraints.clear();

	if (!_padBind)
		return;

	GObjBase *gobj=_padBind->GetGObj();

	LoAgentRef *refAgent=_FindAgentRef(gobj);
	if (!refAgent)
		return;

	if (refAgent->idAgent==RecordID_Invalid)
		return;

	extern StringID SeekBehaviorGraphName(RecordID idAgent);
	StringID nmAI=SeekBehaviorGraphName(refAgent->idAgent);
	if (nmAI==StringID_Invalid)
		return;

	IBehaviorGraph *bg=(IBehaviorGraph *)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->ObtainRes(nmAI);

	if (bg)
	{
		BehaviorGraphData *data=bg->GetData();
		_CollectStateConstraints(data,constraints);
		SAFE_RELEASE(bg);
	}

}


void CBehaviorGraphGrid::_CollectTroopConstraints(std::vector<std::string> &constraints)
{
	constraints.clear();
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (!data)
		return;

	std::string s;
	s="[空]:0";
	constraints.push_back(s);

	for (int i=0;i<data->pads.GetPadCount();i++)
	{
		CLinkPad *pad=data->pads.GetPad(i);
		if (!pad)
			continue;
		if(pad->GetClass()->CheckName("CBgp_Troops"))
		{
			for (int i=0;i<((CBgp_Troops*)pad)->_declares.size();i++)
			{
				StringID id=((CBgp_Troops*)pad)->_declares[i];
				if (id!=StringID_Invalid)
				{
					FormatString(s,"%s:%d",StrLib_GetStr(id),(int)(id));
					constraints.push_back(s);
				}
			}
			continue;
		}
	}
}


CXTPPropertyGridItem *CBehaviorGraphGrid::InsertVar(void *var,const char *cap,const char *desc,GVarType vt,GSem &sem)
{
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (data)
	{
		if (vt==GVT_String)
		{
			std::vector<std::string> constraints;
			if (sem.code==GSem_Name)
			{
				if (sem.constraint=="BehaviorValue字符串ID类型")
				{
					constraints.push_back(std::string(""));
					constraints.push_back(std::string("Troop名称引用"));
					constraints.push_back(std::string("行为图状态名称引用"));
					constraints.push_back(std::string("Agent行为图状态名称引用"));
					return InsertComboItem(cap,desc,(std::string*)var,constraints);
				}
			}
		}

		if (vt==GVT_U)
		{
			if (sem.code==GSem_StringID)
			{
				if (sem.constraint.find("行为图内存变量名称:")==0)
				{
					std::string sType=sem.constraint;
					std::string sHead;
					SeperateStringBy(":",sType,sHead);
					BehaviorMemType tp=BehaviorMemType_None;//Collect all
					BOOL bAllNumber=FALSE;
					if (sType=="Bit")
						tp=BehaviorMemType_Bit;
					else if (sType=="Interger")
						tp=BehaviorMemType_Integer;
					else if (sType=="AllNumber")
					{
						tp=BehaviorMemType_Integer;
						bAllNumber=TRUE;
					}
					else if (sType=="SkillRecord")
						tp=BehaviorMemType_SkillRecord;
					else if (sType=="BuffRecord")
						tp=BehaviorMemType_BuffRecord;
					else if (sType=="ItemRecord")
						tp=BehaviorMemType_ItemRecord;
					else if (sType=="UnitRecord")
						tp=BehaviorMemType_UnitRecord;
					else if (sType=="ResRecord")
						tp=BehaviorMemType_ResourceRecord;
					else if (sType=="Pos")
						tp=BehaviorMemType_Pos;
					else if (sType=="ObjID")
						tp=BehaviorMemType_ObjID;
					else if (sType=="GUID")
						tp=BehaviorMemType_GUID;
					else if (sType=="StringID")
						tp=BehaviorMemType_StringID;
					else if (sType=="Obj")
						tp=BehaviorMemType_Obj;
					//XXXXX:more BehaviorMemType

					if (TRUE)
					{
						static std::vector<std::string> constraints;
						_CollectRefConstraints(tp,constraints);
						return InsertComboItem(cap,desc,(DWORD*)var,constraints);
					}
				}

				if (sem.constraint.find("行为图函数名称引用")==0)
				{
					static std::vector<std::string> constraints;
					_CollectFuncConstraints(constraints);
					return InsertComboItem(cap,desc,(DWORD*)var,constraints);
				}
				

				if (sem.constraint.find("Agent行为图状态名称引用")==0)
				{
					static std::vector<std::string> constraints;
					_CollectAgentStateConstraints(constraints);
					return InsertComboItem(cap,desc,(DWORD*)var,constraints);
				}
				if (sem.constraint.find("行为图状态名称引用")==0)
				{
					static std::vector<std::string> constraints;
					_CollectStateConstraints(constraints);
					return InsertComboItem(cap,desc,(DWORD*)var,constraints);
				}
				if (sem.constraint=="Troop名称引用")
				{
					static std::vector<std::string> constraints;
					_CollectTroopConstraints(constraints);
					return InsertComboItem(cap,desc,(DWORD*)var,constraints);
				}

			}
		}
	}
	return CGObjGrid::InsertVar(var,cap,desc,vt,sem);
}

void CBehaviorGraphGrid::_BhvValueCache_PreLoad(BhvValuesCache *cache,BhvValues *values,GObjBase *obj)
{
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;

	//填写原始信息:values和declares
	cache->values=values;
	//先从CBgp_Func里找到缺省参数
	if (std::string("CBgp_Call")==obj->GetName())
	{
		if (data)
		{
			CBgp_Call *bgp=(CBgp_Call *)obj->GetOwner();
			CBgp_Func *bgpFunc=data->pads.FindFunc(bgp->_nm);
			if (bgpFunc)
			{
				cache->declares.resize(bgpFunc->_declares2.size());
				for (int i=0;i<bgpFunc->_declares2.size();i++)
					cache->declares[i]=&bgpFunc->_declares2[i];
			}
		}
	}

	//Load Bind
	cache->LoadBind(data->pads);
}

void CBehaviorGraphGrid::_BhvValueCache_PreSave(BhvValuesCache *cache)
{
	cache->SaveBind();
}


void CBehaviorGraphGrid::_BhvValueCache_InsertItems(BhvValuesCache *cache)
{
	CXTPPropertyGridItem *item=InsertButtonItem("参数","参数",0);
// 	_AddItemBind(item,cache->values);
	PushInsert();

	cache->items.resize(cache->binds.size());
	for (int i=0;i<cache->binds.size();i++)
	{
		BhvValuesCache::Bind &bind=cache->binds[i];
		BhvValDeclare *declare=cache->declares[i];
		cache->items[i]=NULL;

		if (bind.nm==StringID_Invalid)
			continue;

		BOOL bDef=bind.bDef;

		CXTPPropertyGridItem *item=NULL;

		if (bind.nmRef==StringID_BhvValInvalidRef)
		{
			if (bind.elem)
			{
				std::unordered_map<std::string,std::string> overrides;
				_BindElem(bind.pad->GetGObj(),bind.elem,StrLib_GetStr(bind.nm),overrides);
				item=_itemLast;
			}
		}
		else
		{
			std::vector<std::string> constraints;
			_CollectRefConstraints(bind.tp,constraints);
			item=InsertComboItem(StrLib_GetStr(bind.nm),"",(DWORD*)&bind.nmRef,constraints);
		}

		if (item)
			_AddItemBind(item,bind.value);
		cache->items[i]=item;

		if (item)
		{
			extern void MakeItemColor(CXTPPropertyGridItem *item,DWORD col);
			if (bDef)
				MakeItemColor(item,0x9f9f9f);
			else
				MakeItemColor(item,0);
		}
	}

	PopInsert();
}

BhvValuesCache::Bind* CBehaviorGraphGrid::_BhvValueCache_BindFromItem(CXTPPropertyGridItem *item)
{
	for (int i=0;i<_cachesBhvValues.size();i++)
	{
		BhvValuesCache::Bind *bind=_cachesBhvValues[i].BindFromItem(item);
		if (bind)
			return bind;
	}
	return NULL;
}


BhvValDeclareCache *CBehaviorGraphGrid::_BhvValDeclareCache_Preload(BhvValDeclare *declare)
{
	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (data)
	{
		CClass *clss;
		GElemBase *elem;
		if (data->pads.ResolveBhvValType(declare->tp,clss,elem))
		{
			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)clss->New();

			BhvValDeclareCache cache;
			cache.declare=declare;
			cache.pad=pad;
			cache.elem=elem;

			CDataPacket dp;
			dp.SetDataBufferPointer(&declare->dataDef[0]);
			GObjBase *gobj=pad->GetGObj();
			elem->Load(pad,dp,TRUE);
			_cachesBhvValDeclare.entries.push_back(cache);

			cache.Zero();
			return &_cachesBhvValDeclare.entries[_cachesBhvValDeclare.entries.size()-1];
		}
	}

	return NULL;
}

void CBehaviorGraphGrid::_BhvValDeclareCache_InsertItem(BhvValDeclareCache *cache)
{
	std::unordered_map<std::string,std::string> overrides;
	_BindElem(cache->pad->GetGObj(),cache->elem,StrLib_GetStr(cache->declare->nm),overrides);
	cache->item=_itemLast;
}

void CBehaviorGraphGrid::_BhvValDeclareCache_PreSave(CXTPPropertyGridItem *item)
{
	for (int i=0;i<_cachesBhvValDeclare.entries.size();i++)
	{
		BhvValDeclareCache *cache=&_cachesBhvValDeclare.entries[i];
		if (cache->item==item)
			return;//删除/Clone/MoveUp/MoveDown这个declare

		if (cache->item->HasParent(item))
			return;//高层的修改
	}

	for (int i=0;i<_cachesBhvValDeclare.entries.size();i++)
	{
		BhvValDeclareCache *cache=&_cachesBhvValDeclare.entries[i];
		DP_BeginSave(dp,cache->declare->dataDef);
		cache->elem->Save(cache->pad,dp,TRUE);
		DP_EndSave();
	}
}


extern GElemBase *GetBVRElem(GElemBase *elem);

BOOL CBehaviorGraphGrid::_InsertElem(GObjBase *obj,GElemBase *elem)
{
	if (std::string("CBgp_ModVar")==obj->GetName())
	{
		if (elem->elemname=="mode")
		{
			BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
			if (data)
			{
				GElemBase *elemNM=obj->FindElem("nm");
				if (elemNM)
				{
					StringID nm=*(StringID*)elemNM->GetPtr(obj->GetOwner());
					extern BehaviorMemType ResolveSimpleVarType(StringID nmVar);
					BehaviorMemType tp=ResolveSimpleVarType(nm);
					if (tp==BehaviorMemType_None)
						tp=data->pads.GetVarMemType(nm);

					switch(tp)
					{
						case BehaviorMemType_Bit:
							elem->sem.constraint="设置:2,取反:4";
							break;
						case BehaviorMemType_Integer:
						case BehaviorMemType_Float:
							elem->sem.constraint="增加:0,减少:1,缩放:3,设置:2";
							break;
						case BehaviorMemType_StringID:
						case BehaviorMemType_SkillRecord:
						case BehaviorMemType_BuffRecord:
						case BehaviorMemType_ItemRecord:
						case BehaviorMemType_UnitRecord:
						case BehaviorMemType_ResourceRecord:
						case BehaviorMemType_ObjID:
						case BehaviorMemType_GUID:
							elem->sem.constraint="设置:2";
							break;
							//XXXXX:more BehaviorMemType
					}
				}
			}
		}

		if (elem->elemname=="nm")
		{
			BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
			if (data)
			{
				StringID nm=*(StringID*)elem->GetPtr(obj->GetOwner());
				extern BehaviorMemType ResolveSimpleVarType(StringID nmVar);
				BehaviorMemType tp=ResolveSimpleVarType(nm);
				if (tp==BehaviorMemType_None)
					tp=data->pads.GetVarMemType(nm);
				std::string captionShow="";
				switch(tp)
				{
					case BehaviorMemType_Bit:
						captionShow="布尔值";
						break;
					case BehaviorMemType_Integer:
						captionShow="整数值";
						break;
					case BehaviorMemType_Float:
						captionShow="浮点数值";
						break;
					case BehaviorMemType_StringID:
						captionShow="字符串ID";
						break;
					case BehaviorMemType_SkillRecord:
						captionShow="Skill ID";
						break;
					case BehaviorMemType_BuffRecord:
						captionShow="Buff ID";
						break;
					case BehaviorMemType_ItemRecord:
						captionShow="Item ID";
						break;
					case BehaviorMemType_UnitRecord:
						captionShow="Unit ID";
						break;
					case BehaviorMemType_ResourceRecord:
						captionShow="Resource ID";
						break;
					case BehaviorMemType_ObjID:
						captionShow="游戏对象ID";
						break;
					case BehaviorMemType_GUID:
						captionShow="GUID";
						break;
						//XXXXX:more BehaviorMemType
				}
				const char *hides[]=
				{
					"布尔值","整数值","浮点数值","字符串ID","Skill ID","Buff ID","Item ID","Unit ID","Resource ID","游戏对象ID","GUID"
					//XXXXX:more BehaviorMemType
				};
				for (int i=0;i<ARRAY_SIZE(hides);i++)
				{
					if (captionShow==hides[i])
						continue;
					AddCaptionHide(hides[i]);
				}
			}
		}
	}
	if (TRUE)
	{
		if (((elem->subtype=="BhvConstDeclare")||(elem->subtype=="BhvParamDeclare"))
			&&(elem->GetTypeID()==6))//ObjVector
		{
			void *owner=obj->GetOwner();
			CXTPPropertyGridItem *item=InsertGObjVectorItem(elem->GetEditName(),elem->GetEditDesc(),
				elem,obj->GetOwner(),ID_RGIB_New|ID_RGIB_Clear,ID_RGIB_Remove|ID_RGIB_Clone|ID_RGIB_MoveUp|ID_RGIB_MoveDown);
			_RecordElemEntry(elem,owner);

			PushInsert();

			DWORD nSubs=0;
			elem->GetSubCount(obj->GetOwner(),&nSubs);
			std::string s;
			for (int i=0;i<nSubs;i++)
			{
				GObjBase *objSub;
				if (elem->GetSubObj(obj->GetOwner(),i,&objSub))
				{
					BhvValDeclare *declare=NULL;
					if (elem->subtype=="BhvConstDeclare")
						declare=(BhvValDeclare *)(BhvConstDeclare *)objSub->GetOwner();
					if (elem->subtype=="BhvVarDeclare")
						declare=(BhvValDeclare *)(BhvVarDeclare *)objSub->GetOwner();
					if (elem->subtype=="BhvParamDeclare")
						declare=(BhvValDeclare *)(BhvParamDeclare *)objSub->GetOwner();

					FormatString(s,"%03d",i+1);
					CXTPPropertyGridItem *item=InsertGObjSubItem(s.c_str(),"",elem,owner,i,
											ID_RGIB_Remove|ID_RGIB_Clone|ID_RGIB_MoveUp|ID_RGIB_MoveDown);
					_RecordElemEntry(elem,owner,i);
					_AddItemBind(item,objSub->GetOwner());

					PushInsert();

					BhvValDeclareCache *cache=_BhvValDeclareCache_Preload(declare);
					if (cache)
					{
						_BhvValDeclareCache_InsertItem(cache);
						cache->item=item;
						_AddItemBind(cache->item,objSub->GetOwner());

					}

					if (TRUE)
					{
						GElemBase *elemFlag=objSub->FindElem("flags");
						if (elemFlag)
						{
							std::unordered_map<std::string,std::string>overrides;
							_BindElem(objSub,elemFlag,"",overrides);
						}
					}

					if (TRUE)
					{
						GElemBase *elemFlag=objSub->FindElem("tpShow");
						if (elemFlag)
						{
							std::unordered_map<std::string,std::string>overrides;
							_BindElem(objSub,elemFlag,"",overrides);
						}
					}

 					PopInsert();
				}

			}

			PopInsert();

			return TRUE;
		}
	}

	if (TRUE)
	{
		GObjBase *objSub;
		if (elem->GetObj(obj->GetOwner(),&objSub))
		{
			if ((std::string("BhvValues")==objSub->GetName()))
			{
				_cachesBhvValues.resize(_cachesBhvValues.size()+1);
				BhvValuesCache *cache=&_cachesBhvValues[_cachesBhvValues.size()-1];

				_BhvValueCache_PreLoad(cache,(BhvValues*)objSub->GetOwner(),obj);
				_BhvValueCache_InsertItems(cache);

				return TRUE;
			}
		}
	}

	if (TRUE)
	{
		GElemBase *elemBVR=GetBVRElem(elem);
		if (elemBVR)
		{
			StringID *pID=(StringID*)elemBVR->GetPtr(obj->GetOwner());
			if (pID)
			{
				if ((*pID)!=StringID_BhvValInvalidRef)
				{
					BhvValType tp;
					tp.From(elem);
					std::vector<std::string> constraints;
					_CollectRefConstraints(tp,constraints);
					static std::string nm;
					nm=elem->name+"  [引用]";
					CXTPPropertyGridItem *item=InsertComboItem(nm.c_str(),elem->desc.c_str(),(DWORD*)pID,constraints);

					_AddItemBind(item,pID);
					_RecordElemEntry(elem,obj->GetOwner());

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CBehaviorGraphGrid::_IsDeclarePad()
{
	if (_objs.size()==1)
	{
		GObjBase *gobj=_objs[0];
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)gobj->GetOwner();

		if (!pad)
			return FALSE;
		if (pad->GetClass()->CheckName("CBgp_Func"))
			return TRUE;
		if (pad->GetClass()->CheckName("CBgp_Consts"))
			return TRUE;
		if (pad->GetClass()->CheckName("CBgp_Vars"))
			return TRUE;
	}

	return FALSE;
}


void CBehaviorGraphGrid::_OnContextMenu(CXTPPropertyGridItem *item,CMenu &menu)
{

	if (TRUE)
	{
		BhvValuesCache::Bind* bind=_BhvValueCache_BindFromItem(item);
		if (bind)
		{
			int idxMenu=menu.GetMenuItemCount();

			menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING | MF_SEPARATOR, 0, _T(""));

			if (bind->nmRef!=StringID_BhvValInvalidRef)
				menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING | MF_CHECKED, ID_BHVVAL_REF, _T("引用参数"));
			else
				menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_BHVVAL_REF, _T("引用参数"));
			_itemBehaviorValue=item;
			return;
		}
	}

	int idx;
	VEC_FIND_BY_ELEMENT(_elems,item,item,idx);
	if (idx==-1)
		return;

	int idxMenu=menu.GetMenuItemCount();

	GElemBase *elem=_elems[idx].elem;

	//Add Const/Param
	if (elem->bEditable)
	{
		BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
		if (data)
		{
			BOOL bValidElem=FALSE;
			if (TRUE)
			{
				BhvValType tp;
				tp.From(elem);
				CClass *clss;
				GElemBase *elemT;
				if (data->pads.ResolveBhvValType(tp,clss,elemT))
					bValidElem=TRUE;
			}

			if (bValidElem)
			{
				BOOL bSep=FALSE;

				if (_objs.size()==1)
				{
					GObjBase *gobj=_objs[0];
					CBehaviorGraphPad *padFrom=(CBehaviorGraphPad *)gobj->GetOwner();
					CBgp_Consts *padConsts=data->pads.FindConsts(padFrom);
					if (padConsts)
					{
						if (!bSep)
						{
							menu.InsertMenu(idxMenu++,MF_SEPARATOR,0, _T(""));
							bSep=TRUE;
						}

						idxMenu+=10;//不知道为什么,需要加一些值,不然菜单Item的位置不对.
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_ADDCONST, _T("添加该类型常量"));
						_iCurElemEntry=idx;
					}
				}

				if (TRUE)
				{
					if (_objs.size()==1)
					{
						GObjBase *gobj=_objs[0];
						CBehaviorGraphPad *pad=(CBehaviorGraphPad *)gobj->GetOwner();

						CBgp_Func *padFunc=data->pads.FindOwnerFunc(pad);
						if (padFunc)
						{
							if (!bSep)
							{
								menu.InsertMenu(idxMenu++,MF_SEPARATOR,0, _T(""));
								bSep=TRUE;
							}

							menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_ADDPARAM, _T("添加该类型参数"));
							_iCurElemEntry=idx;
						}
					}
				}

			}
		}
	}


	//可以切换引用的elem
	if (!_IsDeclarePad())
	{
		GElemBase *elemBVR=GetBVRElem(elem);
		if (elemBVR)
		{
			StringID *pID=(StringID*)elemBVR->GetPtr(_elems[idx].owner);
			if (pID)
			{
				menu.InsertMenu(idxMenu++,MF_SEPARATOR,0, _T(""));
				if ((*pID)!=StringID_BhvValInvalidRef)
					menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING|MF_CHECKED,ID_ELEM_REF, _T("引用参数"));
				else
					menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_REF, _T("引用参数"));
				_iCurElemEntry=idx;
			}
		}

	}
}

void CBehaviorGraphGrid::OnElemRef()
{
	if ((DWORD)_iCurElemEntry>=_elems.size())
		return;

	GElemBase *elem=_elems[_iCurElemEntry].elem;
	void *owner=_elems[_iCurElemEntry].owner;
	CXTPPropertyGridItem *item=_elems[_iCurElemEntry].item;

	GElemBase *elemBVR=GetBVRElem(elem);
	if (elemBVR)
	{
		StringID *pID=(StringID*)elemBVR->GetPtr(owner);
		if (pID)
		{
			OnBeginItemChange(item);
			if (*pID==StringID_BhvValInvalidRef)
				*pID=StringID_Invalid;
			else
				*pID=StringID_BhvValInvalidRef;
			OnItemChange(item);
			OnEndItemChange(item);
		}
	}
}

void CBehaviorGraphGrid::OnBhvValRef()
{
	if (!_itemBehaviorValue)
		return;

	BhvValuesCache::Bind*bind=_BhvValueCache_BindFromItem(_itemBehaviorValue);
	if (!bind)
		return;

	OnBeginItemChange(_itemBehaviorValue);
	if (bind->nmRef==StringID_BhvValInvalidRef)
		bind->nmRef=StringID_Invalid;
	else
		bind->nmRef=StringID_BhvValInvalidRef;
	OnItemChange(_itemBehaviorValue);
	OnEndItemChange(_itemBehaviorValue);
}



void CBehaviorGraphGrid::OnElemAddConst()
{
	if ((DWORD)_iCurElemEntry>=_elems.size())
		return;

	GElemBase *elem=_elems[_iCurElemEntry].elem;

	if (_objs.size()==1)
	{
		GObjBase *gobj=_objs[0];
		CBehaviorGraphPad *padFrom=(CBehaviorGraphPad *)gobj->GetOwner();

		BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
		if (data)
		{
			BOOL bSep=FALSE;
			CBgp_Consts *padConsts=data->pads.FindConsts(padFrom);
			if (padConsts)
			{
				StringID nm;
				extern BOOL StrLibDlg_Browse(DWORD iCategory,StringID &id,const char *grp);
				if (StrLibDlg_Browse(0,nm,"行为图常量名称"))
				{
					BhvValType tp;
					tp.From(elem);
					CClass *clss;
					GElemBase *elemT;
					if(data->pads.ResolveBhvValType(tp,clss,elemT))
					{
						std::vector<BhvConstDeclare> declares;
						declares.resize(padConsts->_declares2.size()+1);
						for (int i=0;i<padConsts->_declares2.size();i++)
							declares[i]=padConsts->_declares2[i];

						BhvConstDeclare *declare=&declares[declares.size()-1];
						declare->nm=nm;
						declare->tp=tp;

						//缺省值
						if (TRUE)
						{
							CBehaviorGraphPad *pad=(CBehaviorGraphPad *)clss->New();

							DP_BeginSave(dp,declare->dataDef);
							elemT->Save(pad,dp,TRUE);
							DP_EndSave();

							Safe_Class_Delete(pad);
						}

						padConsts->_declares2.swap(declares);

						RefreshMod(FALSE);
					}
				}
			}
		}
	}

}

void CBehaviorGraphGrid::OnElemAddParam()
{
	if ((DWORD)_iCurElemEntry>=_elems.size())
		return;

	GElemBase *elem=_elems[_iCurElemEntry].elem;

	BehaviorGraphData *data=(BehaviorGraphData *)GetState()->resdata;
	if (data)
	{
		BOOL bSep=FALSE;

		if (_objs.size()==1)
		{
			GObjBase *gobj=_objs[0];
			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)gobj->GetOwner();

			CBgp_Func *padFunc=data->pads.FindOwnerFunc(pad);
			if (padFunc)
			{
				StringID nm;
				extern BOOL StrLibDlg_Browse(DWORD iCategory,StringID &id,const char *grp);
				if (StrLibDlg_Browse(0,nm,"行为图参数名称"))
				{
					BhvValType tp;
					tp.From(elem);
					CClass *clss;
					GElemBase *elemT;
					if(data->pads.ResolveBhvValType(tp,clss,elemT))
					{
						std::vector<BhvParamDeclare> declares;
						declares.resize(padFunc->_declares2.size()+1);
						for (int i=0;i<padFunc->_declares2.size();i++)
							declares[i]=padFunc->_declares2[i];

						BhvParamDeclare *declare=&declares[declares.size()-1];
						declare->nm=nm;
						declare->tp=tp;

						//缺省值
						if (TRUE)
						{
							CBehaviorGraphPad *pad=(CBehaviorGraphPad *)clss->New();

							DP_BeginSave(dp,declare->dataDef);
							elemT->Save(pad,dp,TRUE);
							DP_EndSave();

							Safe_Class_Delete(pad);
						}

						padFunc->_declares2.swap(declares);

						RefreshMod(FALSE);
					}
				}
			}
		}
	}
}



BOOL CBehaviorGraphGrid::_OnDblClickOnItem(CXTPPropertyGridItem *item)
{
	if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
	{
		if (_owner)
		{
			extern BOOL ExistWorldEditor();
			extern void KillWorldEditor();
			extern void OpenWorldEditor(const char *pathMap);
			extern BOOL VerifyMapID(RecordID idMap);
			std::string nmMap;
			RecordID idMap;
			if (_owner->GetMapInfo(idMap,nmMap))
			{
				if (ExistWorldEditor())
				{
					if (!VerifyMapID(idMap))
					{
						KillWorldEditor();
						OpenWorldEditor(nmMap.c_str());
					}
				}
				else
					OpenWorldEditor(nmMap.c_str());
			}
			else
			{
				std::string s;
				StringID nmAI=StringID_Invalid;
				if ((!_owner->GetAIName(nmAI))||(nmAI==StringID_Invalid))
				{
					AfxMessageBox(_T("该行为图未指定名称!"),MB_OK);
				}
				else
				{
					FormatString(s,"该行为图(%s)未被任何地图使用!",StrLib_GetStr(nmAI));
					AfxMessageBox(fromMBCS(s.c_str()), MB_OK);
				}
				return TRUE;//不需要后续处理
			}
		}
	}

	return FALSE;//Not handled

}
