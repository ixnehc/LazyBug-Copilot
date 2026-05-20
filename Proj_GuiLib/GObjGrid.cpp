/********************************************************************
	created:	2007/10/10   11:01
	filename: 	e:\IxEngine\Proj_GuiLib\GObjGrid.cpp
	author:		cxi
	
	purpose:	a property grid to edit a GObj
*********************************************************************/
#include "stdh.h"
#include "tlhelp32.h"
#include "commondefines/general_stl.h"

#include <vector>

#include "SscBase.h"

#include "GObjGrid.h"
#include "GObjItem.h"
#include "RichGridFloatItem.h"
#include "RichGridButtonItem.h"
#include "RichGridComboItem.h"
#include "RichGridTexItem.h"
#include "RichGridResItem.h"
#include "RichGridIntItem.h"
#include "RichGridMatSetItem.h"

#include "GraphBgPads.h"

#include "gds/GObjEx.h"

#include "RenderSystem/IShader.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IUtilRS.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetShell.h"

#include "WorldSystem/stubparams/param_sys.h"

#include "shaderlib/SLDefines.h"

#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include "LoAgentRef.h"

#include <assert.h>

#include "resdata/ResDataDefines.h"
#include "resdata/MtrlData.h"
#include "resdata/MtrlExtData.h"

#include "timer/wuid.h"
#include "Registry/Registry.h"

#include "StrLibDlg.h"

#include "GuiAgent_MatSet.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_ELEM_COPY 7842
#define ID_ELEM_PASTE 7843
#define ID_ELEM_LOCATEFILE 7844
#define ID_ELEM_PASTEAGENT 7845
#define ID_ELEM_OPENRES 7846
#define ID_ELEM_OPENPROTO 7847
#define ID_ELEM_OPENRECORD 7848

#define ID_ITEM_COLLAPSEALL 7860
#define ID_ITEM_EXPANDALL 7861

#define ID_FP_ANIMNONE 1576
#define ID_FP_ANIMVALUESET 1577
#define ID_FP_ANIMRES 1578


//////////////////////////////////////////////////////////////////////////
//CRemoteAgent_MatSet
extern CCurrentUserRegistry g_reg;
void CRemoteAgent_MatSet::Start(const char *pathItem)
{
	extern DWORD CalcHashCode(const char *str);
	DWORD uid=CalcHashCode(pathItem);
// 	if (_uid==uid)
// 		return;//没有变化

	CXTPPropertyGridItem *item=_owner->ItemFromPath(pathItem);
	if (!item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
		item=NULL;

	if (!item)
		uid=0;

	_uid=uid;

	if (_uid!=0)
		_pathItem=pathItem;

	BOOL bCenter=FALSE;
	i_math::vector3df center;

	if (TRUE)
	{
		DWORD ver=g_reg.ReadInt("[RemoteItem]","version",0);
		ver+=10;
		_ver=ver;
		g_reg.WriteInt("[RemoteItem]","uid",_uid);
		g_reg.WriteInt("[RemoteItem]","version",ver);

		CRichGrid_MatSetItem *itemMatSet=(CRichGrid_MatSetItem *)item;
		RemoteItemData_MatSet data;
		if (TRUE)
		{
			std::vector<i_math::matrix43f> *mats=itemMatSet->GetBindMats();
			if (mats)
			{
				data.bMats=TRUE;
				data.mats_=*mats;
				data.mode=itemMatSet->GetMode();
				if (mats->size()>0)
				{
					for (int i=0;i<mats->size();i++)
						center+=(*mats)[i].getTranslation();
					center*=1.0f/(float)mats->size();
					bCenter=TRUE;
				}
			}
			std::vector<i_math::vector3df> *vecs=itemMatSet->GetBindVecs();
			if (vecs)
			{
				data.bVecs=TRUE;
				data.vecs=*vecs;
				data.mode=itemMatSet->GetMode();

				if (vecs->size()>0)
				{
					for (int i=0;i<vecs->size();i++)
						center+=(*vecs)[i];
					center*=1.0f/(float)vecs->size();
					bCenter=TRUE;
				}
			}
			std::vector<i_math::spheref> *sphs=itemMatSet->GetBindSphs();
			if (sphs)
			{
				data.bSphs=TRUE;
				data.sphs=*sphs;
				data.mode=itemMatSet->GetMode();

				if (sphs->size()>0)
				{
					for (int i=0;i<sphs->size();i++)
						center+=(*sphs)[i].center;
					center*=1.0f/(float)sphs->size();
					bCenter=TRUE;
				}
			}
		}

		DP_BeginSave(dp,_bufTemp);
		data.Save(dp);
		DP_EndSave();

		g_reg.WriteData("[RemoteItem]","data",_bufTemp.data(),_bufTemp.size());
	}

	g_reg.SendEvent("[RemoteItem]_Start");
	if (!bCenter)
		center.set(-10000,-10000,-10000);
	g_reg.WriteVar("[RemoteItem]","center",center);

	AfxGetMainWnd()->ShowWindow(SW_SHOWMINIMIZED);
}

void CRemoteAgent_MatSet::Update()
{
	if (_uid==0)
		return;

	DWORD ver=g_reg.ReadInt("[RemoteItem]","version",0);
	if (ver>_ver)
	{
		_ver=ver;
		_uid=g_reg.ReadInt("[RemoteItem]","uid",0);
		if (_uid==0)
			_pathItem="";

		if (_uid!=0)
		{
			RemoteItemData_MatSet data;

			void *pData;
			DWORD szData;
			if (g_reg.ReadData("[RemoteItem]","data",pData,szData))
			{
				CDataPacket dp;
				dp.SetDataBufferPointer((BYTE*)pData);
				data.Load(dp);

				CXTPPropertyGridItem *item=_owner->ItemFromPath(_pathItem.c_str());
				if (item)
				{
					if (!item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
						item=NULL;
				}

				if (item)
				{
					CRichGrid_MatSetItem *itemMatSet=(CRichGrid_MatSetItem *)item;
					_owner->OnBeginItemChange(item);
					if (data.bMats&&itemMatSet->GetBindMats())
						(*itemMatSet->GetBindMats())=data.mats_;
					if (data.bVecs&&itemMatSet->GetBindVecs())
						(*itemMatSet->GetBindVecs())=data.vecs;
					if (data.bSphs&&itemMatSet->GetBindSphs())
						(*itemMatSet->GetBindSphs())=data.sphs;
					_owner->OnItemChange(item);
					itemMatSet->UpdateValue();
					_owner->OnEndItemChange(item);
				}

			}
		}

	}


}




//////////////////////////////////////////////////////////////////////////
//CGObjGrid
BEGIN_MESSAGE_MAP(CGObjGrid,CRichGrid)
	ON_WM_DESTROY()
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
	ON_COMMAND(ID_ELEM_COPY,OnCopyElem)
	ON_COMMAND(ID_ELEM_PASTE,OnPasteElem)
	ON_COMMAND(ID_ELEM_PASTEAGENT,OnPasteAgent)
	ON_COMMAND(ID_ELEM_LOCATEFILE,OnLocateFile)
    ON_COMMAND(ID_ELEM_OPENRES, OnOpenRes)
	ON_COMMAND(ID_ELEM_OPENPROTO, OnOpenProto)
	ON_COMMAND(ID_ELEM_OPENRECORD, OnOpenRecord)
	ON_COMMAND(ID_ITEM_COLLAPSEALL,OnCollapseAll)
	ON_COMMAND(ID_ITEM_EXPANDALL,OnExpandAll)
    ON_COMMAND(ID_RGIB_Clone, OnClone)
	ON_COMMAND(ID_RGIB_MoveUp,OnMoveUp)
	ON_COMMAND(ID_RGIB_MoveDown,OnMoveDown)
	ON_COMMAND(ID_RGIB_Remove,OnRemove)
	ON_COMMAND(ID_FP_ANIMNONE,OnAnimNone)
	ON_COMMAND(ID_FP_ANIMVALUESET,OnAnimValueSet)
	ON_COMMAND(ID_FP_ANIMRES,OnAnimRes)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CGObjGrid::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CRichGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	SetHelpHeight(40);

	_agentRemote.Init(this);

	if (_NeedRemoteMatSetEdit())
	{
		_idTimer=(UINT)SetTimer((UINT_PTR)1,10,NULL);
	}

	return TRUE;
}

void CGObjGrid::OnDestroy()
{
	if (_NeedRemoteMatSetEdit())
		KillTimer(_idTimer);
	__super::OnDestroy();
}

void CGObjGrid::OnTimer(UINT_PTR idEvent)
{
	_agentRemote.Update();
}

void CGObjGrid::ResetContent()
{
	CRichGrid::ResetContent();
	_elems.clear();
	_mds.clear();
	_mditems.clear();
}


CXTPPropertyGridItem *CGObjGrid::InsertGObjVectorItem(const char *cap,const char *desc,
															   GElemBase *elem,void *owner,DWORD IDs,DWORD IDsOfSub)
{
	RG_ADDCHILDITEM(item,CGObjVectorItem,cap,desc);

	_AddItemBind(item,elem->GetPtr(owner));

	item->AddButtonMask(IDs);
	item->Bind(owner,elem,IDsOfSub);
	return item;
}

CXTPPropertyGridItem *CGObjGrid::InsertGObjSubItem(const char *cap,const char *desc,
										GElemBase *elem,void *owner,DWORD iSub,DWORD IDs,const char *brief)
{
	RG_ADDCHILDITEM(item,CGObjSubItem,cap,desc);

	item->SetValue(fromMBCS(brief));
	item->AddButtonMask(IDs);
	item->Bind(owner,elem,iSub);
	return item;
}


CXTPPropertyGridItem *CGObjGrid::InsertMtrlDataItem( const char* cap, const char* desc, MtrlData*data)
{
	g_ssGuiLib.pUtilRS->RepairResData(data);

	CXTPPropertyGridItem*item=InsertVectorItem<MtrlData::Lod>(cap,desc,
		&data->lods,ID_RGIB_New|ID_RGIB_Clear);

	if (!item)
		return NULL;

	PushInsert();
	for (int j=0;j<data->lods.size();j++)
	{
		std::string s;
		FormatString(s,"Lod %02d",j+1);
		InsertVectorElemItem<MtrlData::Lod>(s.c_str(),s.c_str(),&data->lods,j,
			ID_RGIB_MoveUp|ID_RGIB_MoveDown|ID_RGIB_Clone|ID_RGIB_Remove);
		InsertMtrlDataLod(data->lods[j],j);
	}
	PopInsert();

	MtrlDataEntry entry;
	entry.item=item;
	entry.md=data;
	_mds.push_back(entry);

	_ExpandItemR(item);

	return item;
}

extern KeyType EPInfo_GetKT(DWORD idx);
extern ResType ResTypeFromKeyType(KeyType kt);

void CGObjGrid::InsertMtrlDataLod(MtrlData::Lod&lod,DWORD iLod)
{
	IRenderSystem *pRS=g_ssGuiLib.pRS;

	IShaderLibMgr *slmgr=pRS->GetShaderLibMgr();
	PushInsert();
	if (TRUE)
	{

		InsertButtonItem("渲染状态","控制渲染的一些参数",0);
		PushInsert();

		if (TRUE)
		{
			if (TRUE)//the blend mode
			{
				//NOTE: should be synchronized with ShaderBlend
				std::vector<std::string> modes;
				modes.push_back(std::string("不透明模式"));
				modes.push_back(std::string("标准混合模式"));
				modes.push_back(std::string("叠加模式(加法)"));
				modes.push_back(std::string("调和模式(乘法)"));
				modes.push_back(std::string("增量模式"));
				modes.push_back(std::string("反色模式"));
				//XXXXX:more blend mode

				InsertComboItem<BYTE>("混合模式","这个材质与屏幕上的像素的颜色混合方式",
					(&lod.state.modeBlend),modes);
			}


			if (TRUE)
			{
				//NOTE: should be synchronized with StencilMode
				std::vector<std::string> modes;
				modes.push_back(std::string("缺省"));
				modes.push_back(std::string("写入Stencil值"));
				modes.push_back(std::string("写入Stencil值(不写颜色)"));
				modes.push_back(std::string("如果Stencil值等于某个特定值时写入颜色"));
				modes.push_back(std::string("如果Stencil值不等于某个特定值时写入颜色"));
				modes.push_back(std::string("Stencil值加1"));
				modes.push_back(std::string("Stencil值加1(不写颜色)"));
				modes.push_back(std::string("如果Stencil值等于某个特定值时写入颜色及Stencil值"));
				modes.push_back(std::string("如果Stencil值不等于某个特定值时写入颜色及Stencil值"));

				//XXXXX:more stencil mode
				InsertComboItem<BYTE>("Stencil 操作","对Stencil Buffer的操作",
					(&lod.state.modeStencil),modes);
			}

			if (lod.state.modeStencil!=0)//not disabling stencil buffer
			{
				InsertFlagItem("Stencil 参考值","Stencil 操作用到的参考值",
					&lod.state.refStencil,
					"Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7,"
					"Bit8,Bit9,Bit10,Bit11,Bit12,Bit13,Bit14,Bit15"							);
			}

			if (TRUE)
			{
				//NOTE: should be synchronized with DepthMode
				std::vector<std::string> modes;
				modes.push_back(std::string("缺省(比较并写入深度值)"));
				modes.push_back(std::string("不比较深度值"));
				modes.push_back(std::string("不写深度值"));
				modes.push_back(std::string("既不比较也不写深度值"));

				InsertComboItem<BYTE>("深度模式","深度模式",
					(&lod.state.modeDepth),modes);
			}

			if (TRUE)
			{
				//NOTE: should be synchronized with AlphaTestMode
				std::vector<std::string> modes;
				modes.push_back(std::string("禁用"));
				modes.push_back(std::string("标准模式"));

				InsertComboItem<BYTE>("Alpha Test 模式","Alpha Test 模式",
					(&lod.state.modeAlphaTest),modes);
			}

			if (lod.state.modeAlphaTest==AlphaTest_Standard)//for the alpha blend mode,the alpha reference value
			{
				InsertIntItem<WORD>("Alpha Test的参考值","Alpha Test时用于比较的alpha参考值",
					(&lod.state.refAlphaTest),0,255);
			}

			if (TRUE)
			{
				//NOTE: should be synchronized with AlphaTestMode
				std::vector<std::string> modes;
				modes.push_back(std::string("只画正面"));
				modes.push_back(std::string("只画背面"));
				modes.push_back(std::string("两面都画"));

				InsertComboItem<BYTE>("正背面绘制模式","决定正面,背面是否要绘制的参数",
					(&lod.state.modeFacing),modes);
			}

		}

		PopInsert();

		extern void InsertMtrlDemand(CRichGrid *grid,MtrlDemand &demand);
		InsertMtrlDemand(this,lod.demand);

		if (TRUE)//the slib choice combo item
		{
			std::vector<std::string> constaints;
			for (int i=0;i<slmgr->GetLibCount();i++)
				constaints.push_back(std::string(slmgr->GetLibName(i)));

			InsertComboItem("Shader库","使用哪个shader库",&lod.slib,constaints);
		}

		const char *names=slmgr->EnumFeatureNames(lod.slib.c_str(),FF_MtrlEdit);
		InsertFlagItem("Features","The features being used",&lod.features,names);

		if (TRUE)//the unifeature
		{
			std::vector<std::string> constaints;
			std::string names=slmgr->EnumUniFeatures(lod.slib.c_str());
			SplitStringBy(",",names,&constaints);
			constaints.insert(constaints.begin(),std::string(""));
			InsertComboItem("Uni Feature","The unique feature being used",&lod.unifeature,constaints);
		}

		InsertResItem("材质扩展","材质扩展资源的路径",&lod.mte,Res_MtrlExt);

		InsertButtonItem("材质参数","各个Feature用到的参数",0);
		PushInsert();
		if (TRUE)//Now add the params
		{
			for (int i=0;i<lod.fps.size();i++)
			{
				KeyType kt;
				std::string showname;
				std::string desc;
				GSem sem;

				FeatureParamA *fp=&lod.fps[i];

				if (!fp->bMte)
				{
					EffectParam ep=EPfromName(fp->ep_name);
					int idx=EPInfo_IndexFromEP(ep);
					if (idx==-1)
						continue;
					kt=EPInfo_GetKT(idx);
					showname=EPInfo_GetShowName(idx);
					desc=EPInfo_GetDesc(idx);
					sem=EPInfo_GetVarSem(idx);
				}
				else
				{
					MtrlExtData::EPInfo info;
					if (FALSE==ParseMteEPInfo(info,fp->nm))
						continue;
					kt=info.kt;
					showname=info.nameShow;
					desc=showname;
					sem=GSem(fp->code,info.sem.constraint.c_str());
				}

				if (TRUE)
				{
					CXTPPropertyGridItem *item=NULL;
					if (lod.fps[i].at==FeatureParamA::Anim_Res)
					{
						ResType rt=ResTypeFromKeyType(kt);
						if (rt!=Res_None)
						{
							item=InsertResItem(showname.c_str(),desc.c_str(),
								&lod.fps[i].pathAnim,rt);
						}
					}
					if (lod.fps[i].at==FeatureParamA::Anim_ValueSet)
					{
						extern i_math::rectf GetSemLimitRect(GSem &sem);
						i_math::rectf rc=GetSemLimitRect(sem);

						item=InsertValueSetItem(showname.c_str(),desc.c_str(),&lod.fps[i].vs,rc);
					}
					if (!item)
						item=InsertGVar(showname.c_str(),desc.c_str(),lod.fps[i].var,sem,g_ssGuiLib.pRS);

					if (item)
					{
						if (kt!=KT_None)
						{
							MtrlDataItemInfo data;
							data.fp=fp;
							data.kt=kt;
							data.sem=sem;
							_mditems[item]=data;
						}
					}

					if (fp->bMte&&item)
					{
						CXTPPropertyGridItemMetrics*metrics=item->GetCaptionMetrics();
						metrics->m_clrFore=0x006f00;
					}
				}
			}
		}
		PopInsert();

	}	
	PopInsert();

}




//比如{ABC:DEF}GHI处理后,返回TRUE,name为"ABC",value为"DEF",remain为"GHI"
BOOL CullTag(const char *str,const char *&name,const char *&value,const char *&remain)
{
	static std::string name0,value0;
	name0="";
	value0="";
	BOOL bRet=FALSE;
	char *p=(char*)str;
	int state=0;
	while(*p)
	{
		char c=*p;
		switch(state)
		{
			case 0:
			{//期待'{'
				if (c!='{')
					return FALSE;
				state=1;
				break;
			}
			case 1:
			{//期待':'或'}'
				if ((c==':')||(c=='}'))
				{
					if (c=='}')
					{
						remain=p+1;
						bRet=TRUE;
						goto _out;
					}
					if (c==':')
					{
						state=2;
						break;
					}
				}
				else
					name0+=c;
				break;
			}
			case 2:
			{//期待'}'
				if (c=='}')
				{
					remain=p+1;
					bRet=TRUE;
					goto _out;
				}
				else
					value0+=c;
				break;
			}

		}
		p++;

	}

_out:	
	if (!bRet)
		return FALSE;
	name=name0.c_str();
	value=value0.c_str();
	return TRUE;
}

//Label的格式$Lable{L1,L2,L3,...}
//这个函数从constraint里把头部的Label切掉,并把这些切下来的Label填到label里
BOOL CullLabel(std::string &constraint,std::vector<std::string>&labels)
{
	if (constraint.length()<=7)
		return FALSE;
	if (memcmp(constraint.c_str(),"$Lable{",7)!=0)
		return FALSE;

	const char *p=constraint.c_str()+7;
	std::string s;
	while(*p)
	{
		if ((*p)=='}')
		{
			if (s.empty())
				return FALSE;
			SplitStringBy(",",s,&labels);
			s=p+1;
			constraint=s;
			return TRUE;
		}
		s+=*p;
		p++;
	}
	return FALSE;
}

void CGObjGrid::_RecordElemEntry(GElemBase *elem,void *owner,int iSub)
{
	ElemEntry entry;
	if (!_itemLast)
		return;
	entry.elem=elem;
	entry.owner=owner;
	entry.item=_itemLast;
	entry.iSub=iSub;

	_elems.push_back(entry);
}


void CGObjGrid::_BindElem(GObjBase *obj,GElemBase *elem,const char *capOverride,std::unordered_map<std::string,std::string>&overrides)
{
	void *varSub;
	GObjBase *objSub;
	DWORD cSub;
	void *owner=obj->GetOwner();
	static std::vector<std::string> pieces;

	while(elem->bEditable)
	{
		if (_InsertElem(obj,elem))
			break;
		GVarType vt=elem->GetVarType();
		GSem sem=elem->GetSem();
		if (TRUE)//到重载表里寻找，找到就重载
		{
			std::unordered_map<std::string,std::string>::iterator it=overrides.find(std::string(elem->GetElemName()));
			if (it!=overrides.end())
				sem.constraint=(*it).second;
		}

		if (elem->GetObj(owner,&objSub))
		{//It's a single obj
			BOOL bHandled = FALSE;
			if ((sem.code==GSem_Unknown)&&(sem.constraint=="DynObjPtr"))
			{
				void *var;
				if (TRUE==elem->GetVar(owner,&var))
				{
					InsertDynObjItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),(void**)var,
						((GElem_DynObjPtrBase *)elem)->classes,
						((GElem_DynObjPtrBase *)elem)->names);
					_RecordElemEntry(elem,owner);
				}
				else
					assert(FALSE);
				PushInsert();
				_Bind(objSub,sem);
				PopInsert();
				bHandled = TRUE;
			}
			else if ( ( GSem_Unknown == sem.code ) && ( std::string("ValueSet") == objSub->GetName() ) )
			{
				ValueSet* vs = static_cast<ValueSet*>( objSub->GetOwner() );
				assert( vs != NULL );

				i_math::rectf rcLimit;
				SplitStringBy(",", sem.constraint, &pieces);
				float *p=(float*)&rcLimit;
				for (int i=0;i<pieces.size();i++)
					p[i]=(float)DoubleFromString(pieces[i].c_str());
				InsertValueSetItem( capOverride[0]?capOverride:elem->GetEditName(), elem->GetEditDesc(), vs,rcLimit);
				_RecordElemEntry(elem,owner);
				bHandled = TRUE;
			}
			else if ( ( GSem_Unknown == sem.code ) && ( std::string("LoAgentRef") == objSub->GetName() ) )
			{
				LoAgentRef* refAgent = static_cast<LoAgentRef*>( objSub->GetOwner() );
				assert( refAgent != NULL );

				static std::string s;
				s="[Empty]";
				if (refAgent->IsValid())
				{
					extern BOOL SeekAgentName(RecordID idRec,std::string &nm);
					extern BOOL SeekMapName(RecordID idRec,std::string &nm);
					static std::string nmMap,nmAgent;
					if (SeekMapName(refAgent->idMap,nmMap))
					{
						if (SeekAgentName(refAgent->idAgent,nmAgent))
						{
							FormatString(s,"%s,[%s(%08X)]",nmMap.c_str(),nmAgent.c_str(),refAgent->guid);
						}
					}
				}

				InsertItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),s.c_str());
				_RecordElemEntry(elem,owner);
				bHandled = TRUE;
			}
			else if ( ( GSem_Unknown == sem.code ) && ( std::string("MtrlDataObj") == objSub->GetName() ) )
			{
				MtrlDataObj*mdo= static_cast<MtrlDataObj*>( objSub->GetOwner() );
				assert( mdo!= NULL );
				InsertMtrlDataItem( capOverride[0]?capOverride:elem->GetEditName(), elem->GetEditDesc(), &mdo->md);
				_RecordElemEntry(elem,owner);
				bHandled = TRUE;
			}

			if ( !bHandled )
			{
				CXTPPropertyGridItem *item=InsertButtonItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),0);
				_AddItemBind(item,objSub->GetOwner());
				_RecordElemEntry(elem,owner);

				if (item)
				{
					PushInsert();
					_Bind(objSub,sem);
					PopInsert();
				}	
			}

// 			PushInsert();
// 			_Bind(objSub,sem);
// 			PopInsert();

			break;
		}
		if (elem->GetVar(owner,&varSub))
		{//It's a single var
			InsertVar(varSub,capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),vt,sem);
			_RecordElemEntry(elem,owner);
			break;
		}

		if (elem->GetSubCount(owner,&cSub))
		{
			if  ( GSem_Unknown == sem.code )
			{
				if (!sem.constraint.empty())
				{
					static std::string mode;
					SplitStringBy(",", sem.constraint, &pieces);
					if ((pieces[0]=="MatSet")||(pieces[0]=="MatSetLS"))
					{
						BOOL bLS=FALSE;
						if (pieces[0]=="MatSetLS")
							bLS=TRUE;
						mode="";
						if (pieces.size()>1)
							mode=pieces[1];
						if (vt==GVT_Fx12)
						{
							std::vector<i_math::matrix43f> *mats=(std::vector<i_math::matrix43f> *)elem->GetPtr(owner);

							InsertMatSetItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),mats,bLS,mode.c_str());
							_RecordElemEntry(elem,owner);
						}
						if (vt==GVT_Fx3)
						{
							std::vector<i_math::vector3df> *vecs=(std::vector<i_math::vector3df> *)elem->GetPtr(owner);

							InsertMatSetItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),vecs,bLS,mode.c_str());
							_RecordElemEntry(elem,owner);
						}

						break;
					}
					if ((pieces[0]=="SphereSet")||(pieces[0]=="SphereSetLS"))
					{
						BOOL bLS=FALSE;
						if (pieces[0]=="SphereSetLS")
							bLS=TRUE;
						mode="";
						if (pieces.size()>1)
							mode=pieces[1];
						if (vt==GVT_Fx4)
						{
							std::vector<i_math::spheref> *sphs=(std::vector<i_math::spheref> *)elem->GetPtr(owner);

							InsertMatSetItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),sphs,bLS,mode.c_str());
							_RecordElemEntry(elem,owner);
						}

						break;
					}
					if (pieces[0]=="AssetUIDSet")
					{
						if (vt==GVT_U)
						{
							std::vector<DWORD> *uids=(std::vector<DWORD> *)elem->GetPtr(owner);

							InsertAstUIDSetItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),uids);
							_RecordElemEntry(elem,owner);
						}

						break;
					}
				}
			}

			DWORD mask=0,maskSub=0;
			if (elem->NewSub(NULL))
				mask|=ID_RGIB_New;
			if (elem->ClearSub(NULL))
				mask|=ID_RGIB_Clear;
			if (elem->RemoveSub(NULL,0))
				maskSub|=ID_RGIB_Remove;
			if (elem->CloneSub(NULL,0))
				maskSub|=ID_RGIB_Clone;
			if (elem->MoveSub(NULL,0,TRUE))
				maskSub|=ID_RGIB_MoveUp|ID_RGIB_MoveDown;

			std::string constraint=sem.constraint;
			std::vector<std::string>labels;
			CullLabel(constraint,labels);
			sem.constraint=constraint;

			CXTPPropertyGridItem *item=InsertGObjVectorItem(capOverride[0]?capOverride:elem->GetEditName(),elem->GetEditDesc(),
				elem,owner,mask,maskSub);
			_RecordElemEntry(elem,owner);

			PushInsert();
			if (elem->GetSubVar(NULL,0,&varSub))
			{
				for (int i=0;i<cSub;i++)
				{
					elem->GetSubVar(owner,i,&varSub);

					std::string cap;

					const char *name,*value,*remain;
					if (CullTag(sem.constraint.c_str(),name,value,remain))
					{
						if (std::string("Name")==name)
						{
							GSem sem2=sem;
							sem2.constraint=remain;
							void *var=(void*)(((BYTE*)varSub)+sizeof(StringID));
							StringID id=*(StringID*)varSub;
							const char *name="[未命名]--(双击改名)";
							if (id!=StringID_Invalid)
								name=StrLib_GetStr(id);
							CXTPPropertyGridItem *item=InsertVar(var,name,value,vt,sem2);
							item->SetItemData(FORCE_TYPE(DWORD,varSub));//将StringID的指针绑到这个item里面 
							continue;
						}
					}

					FormatString(cap,"[%03d]",i);
					if (labels.size()>0)
					{
						if (i<labels.size())
						{
							const char *p=labels[i].c_str();
							if (p[0]==0)
								continue;
							if (p[0]=='/')
							{
								if (p[1]=='/')
									continue;
							}
							cap=labels[i];
						}
						else
							continue;
					}

					InsertVar(varSub,cap.c_str(),"",vt,sem);
					_RecordElemEntry(elem,owner,i);
				}
			}
			if (elem->GetSubObj(NULL,0,&objSub))
			{
				for (int i=0;i<cSub;i++)
				{
					elem->GetSubObj(owner,i,&objSub);

					std::string cap;
					FormatString(cap,"[%03d]",i);
					if (labels.size()>0)
					{
						if (i<labels.size())
						{
							const char *p=labels[i].c_str();
							if (p[0]==0)
								continue;
							if (p[0]=='/')
							{
								if (p[1]=='/')
									continue;
							}
							cap=labels[i];
						}
						else
							continue;
					}

					if (TRUE)
					{
						//Peek the 1st elem and try to find a name
						GElemBase *elem=objSub->GetElems();
						if (elem)
						{
							if (elem->GetSem().code==GSem_StringID)
							{
								StringID *id;
								if (elem->GetVar(objSub->GetOwner(),(void**)&id))
									cap=StrLib_GetStr(*id);
							}
						}
					}

					BOOL bSingleElemObj=FALSE;
					if (TRUE)
					{
						GElemBase *elem=objSub->GetElems();
						if (elem)
						{
							if (!elem->next)
								bSingleElemObj=TRUE;
						}
					}

					if (!bSingleElemObj)
					{
						FillDescAssist_GuiLib assist;
						CXTPPropertyGridItem *item=InsertGObjSubItem(cap.c_str(),"",elem,owner,i,maskSub,objSub->GetBrief((void *)&assist));
						_RecordElemEntry(elem,owner,i);
						_AddItemBind(item,objSub->GetOwner());

						PushInsert();
						_Bind(objSub,sem);
						PopInsert();
					}
					else
					{
						_Bind(objSub,sem,cap.c_str());
						_RecordElemEntry(elem,owner,i);
					}
				}
			}
			PopInsert();

			break;
		}
		break;
	}
}


//注意:在Bind一个GObj的时候,可以重载这个GObj的各个Element的语义的Constraint字串,
//constraintOverride的格式为: elemname1:constraint1$$elemname2:constraint2$$...
//注意elemname是变量名,不是编辑显示的名字
void CGObjGrid::_Bind(GObjBase *obj,GSem &sem,const char *capOverride)
{
	if(!obj)
		return;

	if (obj->GetBase())
		_Bind(obj->GetBase(),sem);

	void *owner=obj->GetOwner();
	GElemBase *elem=obj->GetElems();

	std::vector<std::string> pieces;
	std::unordered_map<std::string,std::string>overrides;
	const char *constraintOverride=sem.constraint.c_str();
	if (constraintOverride[0])//建立constraints的重载表
	{
		std::string s;
		SplitStringBy("$$",std::string(constraintOverride),&pieces);
		for (int i=0;i<pieces.size();i++)
		{
			if (SeperateStringBy(":",pieces[i],s))
				overrides[s]=pieces[i];
		}
	}

	while(elem)
	{
		_BindElem(obj,elem,capOverride,overrides);
		elem=elem->next;
	}
}

void CGObjGrid::Bind(GObjBase *obj)
{
	Bind(&obj,1);
}

void CGObjGrid::Bind(GObjBase **objs,DWORD c)
{

	LockPaint();

	ResetContent();

	BeginInsert();
	for(int i=0;i<c;i++)
	{
		GObjBase *obj=objs[i];
		if (obj)
		{
			std::string nm=obj->GetName();
			CXTPPropertyGridItem *item=InsertCategory(nm.c_str(),"",NULL);
// 			if (item)
// 			{
// 				CXTPPropertyGridItemMetrics*metrics=item->GetCaptionMetrics();
// 				LOGFONT lf;
// 				metrics->m_fontBold.GetLogFont(&lf);
// 				metrics->m_fontNormal.CreateFontIndirect(&lf);
// 			}

			PushInsert();

			_Bind(obj,GSem(GSem_Unknown));

 			PopInsert();

		}
	}
	EndInsert();


	UnLockPaint();


	_objs.resize(c);
	memcpy(_objs.data(),objs,sizeof(GObjBase*)*c);
}




void CGObjGrid::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	LockPaint();
	RecordState(_state);//record the grid state before any change occurs
}

void CGObjGrid::OnItemChange(CXTPPropertyGridItem *item)
{
	//	RefreshMod();
}

void CGObjGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	_RepairMtrlData(item);


	Bind(_objs.data(),_objs.size());

	RestoreState(_state);

	UnLockPaint();
}

LRESULT CGObjGrid::SendNotifyMessage(WPARAM wParam, LPARAM lParam)
{
	CXTPPropertyGrid::SendNotifyMessage(wParam,lParam);
	return OnGridNotify(wParam,lParam);
}

struct ElemCB
{
	ElemCB()
	{
		elem=NULL;
		iSub=-1;
	}
	GElemBase *elem;
	int iSub;
	std::vector<BYTE> buf;
};

ElemCB *GetElemCB()
{
	static ElemCB cb;
	return &cb;
}

BOOL IsCurrentlyWorldEditor()
{
//	char buffer[512];
	TCHAR buffer[512];
	GetModuleFileName(NULL,buffer,500);
	std::string s = toMBCS(buffer);
	StringLower(s);
	
	int i=s.find("worldeditor");
	if (i>=0)
		return TRUE;
	return FALSE;
}

BOOL ExistExe(TCHAR *nameExe)
{

    PROCESSENTRY32 pe32;//用于存放进程信息的结构体
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//创建进程快照
    pe32.dwSize = sizeof(pe32);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bMore = ::Process32First(hProcessSnap, &pe32);//获取第一个进程信息到pe32结构体中
    while (bMore)
    {
        if (!_tcscmp(nameExe, pe32.szExeFile))//发现要寻找的进程后结束查找
        {
            return TRUE;
        }
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }
    return FALSE;
}

BOOL ExistWorldEditor()
{
    TCHAR exe_name[20] = _T("WorldEditor.exe");//要查询的进程名
    return ExistExe(exe_name);
}

BOOL ExistResEditor()
{
    TCHAR exe_name[20] = _T("ResEditor.exe");//要查询的进程名
    return ExistExe(exe_name);
}

BOOL ExistProtoEditor()
{
	TCHAR exe_name[20] = _T("ProtoEditor.exe");//要查询的进程名
	return ExistExe(exe_name);
}

BOOL ExistRecordEditor()
{
	TCHAR exe_name[20] = _T("RecordEditor.exe");//要查询的进程名
	return ExistExe(exe_name);
}

void KillExe(TCHAR *exe_name)
{
    PROCESSENTRY32 pe32;//用于存放进程信息的结构体
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//创建进程快照
    pe32.dwSize = sizeof(pe32);


    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return;

    BOOL bMore = ::Process32First(hProcessSnap, &pe32);//获取第一个进程信息到pe32结构体中
    while (bMore)
    {
        if (!_tcscmp(exe_name, pe32.szExeFile))//发现要寻找的进程后结束查找
        {
            break;
        }
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }

    if (!_tcscmp(exe_name, pe32.szExeFile))
    {
        HANDLE hprocess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hprocess != NULL)
        {
            ::TerminateProcess(hprocess, 0);//关闭进程
            ::CloseHandle(hprocess);
        }
    }
    CloseHandle(hProcessSnap);
}


void KillWorldEditor()
{
	TCHAR exe_name[20]=_T("WorldEditor.exe");//要查询的进程名
    KillExe(exe_name);
}

void OpenWorldEditor(const char *pathMap)
{
	std::string path;

	path=g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
	path=LEFT_STRING(path,path.length()-4);
#ifdef _DEBUG
	path+="bin\\debug\\WorldEditor.exe";
#else
	path+="bin\\WorldEditor.exe";
#endif

	std::string arg="-OpenMap:";
	arg+=pathMap;
	CString pathS = fromMBCS(path.c_str());
	CString argS = fromMBCS(arg.c_str());
	ShellExecute( NULL, _T("open"), pathS, argS, NULL, SW_SHOWNORMAL );

	while(1)
	{
		if (!g_reg.FetchEvent("[WorldEditorReady]"))
		{
			Sleep(50);
			continue;
		}
		break;
	}
}

void OpenResEditor()
{
    std::string path;

    path = g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
    path = LEFT_STRING(path, path.length() - 4);
#ifdef _DEBUG
    path += "bin\\debug\\ResEditor.exe";
#else
    path += "bin\\ResEditor.exe";
#endif

    std::string arg ;

	CString pathS = fromMBCS(path.c_str());
	CString argS = fromMBCS(arg.c_str());
    ShellExecute(NULL, _T("open"), pathS, argS, NULL, SW_SHOWNORMAL);

    while (1)
    {
        if (!g_reg.FetchEvent("[ResEditorReady]"))
        {
            Sleep(50);
            continue;
        }
        break;
    }
}

void OpenProtoEditor()
{
	std::string path;

	path = g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
	path = LEFT_STRING(path, path.length() - 4);
#ifdef _DEBUG
	path += "bin\\debug\\ProtoEditor.exe";
#else
	path += "bin\\ProtoEditor.exe";
#endif

	std::string arg ;

	CString pathS = fromMBCS(path.c_str());
	CString argS = fromMBCS(arg.c_str());
	ShellExecute(NULL, _T("open"), pathS, argS, NULL, SW_SHOWNORMAL);

	while (1)
	{
		if (!g_reg.FetchEvent("[ProtoEditorReady]"))
		{
			Sleep(50);
			continue;
		}
		break;
	}
}

void OpenRecordEditor()
{
	std::string path;

	path = g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
	path = LEFT_STRING(path, path.length() - 4);
#ifdef _DEBUG
	path += "bin\\debug\\RecordEditor.exe";
#else
	path += "bin\\RecordEditor.exe";
#endif

	std::string arg;

	CString pathS = fromMBCS(path.c_str());
	CString argS = fromMBCS(arg.c_str());
	ShellExecute(NULL, _T("open"), pathS, argS, NULL, SW_SHOWNORMAL);

	while (1)
	{
		if (!g_reg.FetchEvent("[RecordEditorReady]"))
		{
			Sleep(50);
			continue;
		}
		break;
	}
}


BOOL VerifyMapID(RecordID idMap)
{
	g_reg.WriteInt("[VerifyMapID]","MapID",(int)idMap);
	g_reg.SendEvent("[VerifyMapID]_Request");
	while(1)
	{
		if (!g_reg.FetchEvent("[VerifyMapID]_Result"))
		{
			Sleep(50);
			continue;
		}
		break;
	}
	std::string result=g_reg.ReadString("[VerifyMapID]","Result","");
	if (result=="Ok")
		return TRUE;
	return FALSE;
}

LRESULT CGObjGrid::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_PGN_CONTEXTMENU)
	{
		CXTPPropertyGridItem *item=GetSelectedItem();
		CXTPPropertyGridItem *parent=NULL;

		CMenu menu;	
		menu.CreatePopupMenu();
		int idxMenu=0;

		if (item)
		{
			int idx;
			VEC_FIND_BY_ELEMENT(_elems,item,item,idx);
			if (idx!=-1)
			{
				_iCurElemEntry=idx;
				menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_COPY,_T("复制数据"));
				ElemCB *cb=GetElemCB();
				if (cb->elem)
				{
					if (_elems[idx].elem->CheckCompatible(cb->elem))
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_PASTE,_T("粘帖数据"));
				}

				if (TRUE)
				{
					GObjBase *objSub;
					if (_elems[idx].elem->GetObj(_elems[idx].owner,&objSub))
					{
						if ( ( GSem_Unknown == _elems[idx].elem->sem.code) && ( std::string("LoAgentRef") == objSub->GetName() ) )
						{
							std::string strCB;
							extern BOOL PasteFromClipboard(CWnd *wnd,std::string &str);

							if (PasteFromClipboard(this,strCB))
							{
								std::vector<std::string> temp;
								SplitStringBy("||",strCB,&temp);

								if (temp.size()==6)
								{
									if (temp[0]=="LoAgentRef")
									{
										std::string s;
										int guid=IntFromString(temp[3].c_str());
										FormatString(s,"粘帖 [%s,%s(%08X)]",temp[1].c_str(),temp[2].c_str(),guid);
										menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING, ID_ELEM_PASTEAGENT, fromMBCS(s.c_str()));
									}
								}
							}
						}
					}
				}

				if (TRUE)
				{
					static std::string pathAbs;
					extern BOOL GetElemFilePath(GElemBase * elem, void* owner, int iSub, std::string & pathAbs);
					if (GetElemFilePath(_elems[idx].elem, _elems[idx].owner, _elems[idx].iSub, pathAbs))
					{
						menu.InsertMenu(idxMenu++,MF_SEPARATOR,0,_T(""));
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ELEM_LOCATEFILE,_T("打开文件位置"));
					}

                    extern BOOL GetElemOpenableResPath(GElemBase *elem, void *owner, int iSub, std::string &pathAbs);
                    if (GetElemOpenableResPath(_elems[idx].elem, _elems[idx].owner, _elems[idx].iSub, pathAbs))
                    {
                        menu.InsertMenu(idxMenu++, MF_SEPARATOR, 0, _T(""));
                        menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING, ID_ELEM_OPENRES, _T("在ResEditor中打开"));
                    }

					ProtoID idProto = ProtoID_Null;
					extern BOOL GetElemOpenableProtoID(GElemBase * elem, void* owner, int iSub,ProtoID &idProto);
					if (GetElemOpenableProtoID(_elems[idx].elem, _elems[idx].owner, _elems[idx].iSub,idProto))
					{
						menu.InsertMenu(idxMenu++, MF_SEPARATOR, 0, _T(""));
						menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING, ID_ELEM_OPENPROTO, _T("在ProtoEditor中打开"));
					}

					RecordID idRec= RecordID_Invalid;
					std::string nmRecords;
					extern BOOL GetElemOpenableRecordID(GElemBase * elem, void* owner, int iSub,std::string & nmRecords, RecordID & idRec);
					if (GetElemOpenableRecordID(_elems[idx].elem, _elems[idx].owner, _elems[idx].iSub, nmRecords, idRec))
					{
						menu.InsertMenu(idxMenu++, MF_SEPARATOR, 0, _T(""));
						menu.InsertMenu(idxMenu++, MF_ENABLED | MF_STRING, ID_ELEM_OPENRECORD, _T("在RecordEditor中打开"));
					}
                }
			}
		}

		if (item)
			parent=item->GetParentItem();
		if (parent)
		{
			if (parent->IsKindOf(RUNTIME_CLASS(CGObjVectorItem)))
			{
				_cursub.elem=((CGObjVectorItem*)parent)->_elem;
				_cursub.owner=((CGObjVectorItem*)parent)->_owner;
				_cursub.item=item;
				_cursub.iSub=parent->GetChilds()->Find(item);
				_cursub.mask=((CGObjVectorItem*)parent)->_maskSub;
				if (_cursub.iSub!=-1)
				{
					if (idxMenu>0)
						menu.InsertMenu(idxMenu++,MF_SEPARATOR,0,_T(""));
					if (_cursub.mask&ID_RGIB_Clone)
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_RGIB_Clone, _T("Clone"));
					if (_cursub.mask&ID_RGIB_MoveUp)
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_RGIB_MoveUp, _T("上移"));
					if (_cursub.mask&ID_RGIB_MoveDown)
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_RGIB_MoveDown, _T("下移"));
					if (_cursub.mask&ID_RGIB_Remove)
						menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_RGIB_Remove, _T("删除"));

				}
			}
		}

		if (item)
		{
			MtrlDataItemInfo *mditem=NULL;
			if (TRUE)
			{
				std::map<CXTPPropertyGridItem *,MtrlDataItemInfo>::iterator it=_mditems.find(item);
				if (it!=_mditems.end())
					mditem=&((*it).second);
			}

			if (mditem)
			{
				FeatureParamA *fp=mditem->fp;
				if (fp)
				{
					DWORD flags[3];
					for (int i=0;i<3;i++)
						flags[i]=MF_ENABLED|MF_STRING;
					flags[fp->at]|=MF_CHECKED;
					menu.InsertMenu(idxMenu++,flags[0],ID_FP_ANIMNONE, _T("无动画"));
					menu.InsertMenu(idxMenu++,flags[1],ID_FP_ANIMVALUESET, _T("编辑动画"));
					menu.InsertMenu(idxMenu++,flags[2],ID_FP_ANIMRES, _T("使用动画资源"));
				}
			}
		}

// 		if (item)
// 		{
// 			_itemCur=item;			
// 			if (idxMenu>0)
// 				menu.InsertMenu(idxMenu++,MF_SEPARATOR,0,"");
// 			menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ITEM_COLLAPSEALL,"全部折叠");
// 			menu.InsertMenu(idxMenu++,MF_ENABLED|MF_STRING,ID_ITEM_EXPANDALL,"全部展开");
// 		}


		_OnContextMenu(item,menu);

		if (menu.GetMenuItemCount()>0)
		{
			CPoint point;
			GetCursorPos(&point);
			menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x,point.y,this,NULL );
		}

	}
	if (wParam==XTP_PGN_DBLCLICK)
	{
		CXTPPropertyGridItem *item=GetSelectedItem();
		CXTPPropertyGridItem *parent=NULL;

		if (item)
		{
			if (GetKeyState(VK_LCONTROL)&0x8000)
			{
				if (item->IsExpanded())
					_ExpandItemR(item);
				else
					_CollapseItemR(item);
				return 0;
			}
		}

		if (_OnDblClickOnItem(item))
			return 0;

		if (_NeedRemoteMatSetEdit())
		{
			if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
			{
				std::string pathItem=PathFromItem(item);
				_agentRemote.Start(pathItem.c_str());

				return 0;
			}
		}

		if (TRUE)
		{
			int idx;
			VEC_FIND_BY_ELEMENT(_elems,item,item,idx);
			if (idx!=-1)
			{
				GObjBase *objSub;
				if (_elems[idx].elem->GetObj(_elems[idx].owner,&objSub))
				{
					if ( ( GSem_Unknown == _elems[idx].elem->sem.code) && ( std::string("LoAgentRef") == objSub->GetName() ) )
					{
						LoAgentRef* refAgent = static_cast<LoAgentRef*>( objSub->GetOwner() );
						if (refAgent)
						{
							std::string nmMap;
							extern BOOL SeekMapName(RecordID idRec,std::string &nm);
							if (SeekMapName(refAgent->idMap,nmMap))
							{
								BOOL bCurrentlyWorldEditor=IsCurrentlyWorldEditor();

								if (!bCurrentlyWorldEditor)
								{
									if (ExistWorldEditor())
									{
										if (!VerifyMapID(refAgent->idMap))
										{
											KillWorldEditor();
											OpenWorldEditor(nmMap.c_str());
										}
									}
									else
										OpenWorldEditor(nmMap.c_str());

									if (VerifyMapID(refAgent->idMap))
									{
										g_reg.WriteInt("[LocateAgent]","MapID",(int)refAgent->idMap);
										g_reg.WriteInt("[LocateAgent]","GUID",(int)refAgent->guid);
										g_reg.SendEvent("[LocateAgent]_Start");

										while(1)
										{
											if (!g_reg.FetchEvent("[LocateAgent]_Result"))
											{
												Sleep(100);
												continue;
											}
											std::string result=g_reg.ReadString("[LocateAgent]","Result","");
											if (result=="Ok")
												break;
										}

										AfxGetMainWnd()->ShowWindow(SW_SHOWMINIMIZED);
									}
								}
								else
								{
									g_reg.WriteInt("[LocateAgent]","MapID",(int)refAgent->idMap);
									g_reg.WriteInt("[LocateAgent]","GUID",(int)refAgent->guid);
									g_reg.SendEvent("[LocateAgent]_Start");
								}
							}
						}
					}
				}
			}
		}


		if (item)
			parent=item->GetParentItem();
		if (parent)
		{
			if (parent->IsKindOf(RUNTIME_CLASS(CGObjVectorItem)))
			{
				StringID*id=(StringID*)item->GetItemData();
				if (id)
				{
					CStrLibDlg dlg;

					dlg.SetSscSystem(g_ssGuiLib.ssc->GetSS());
					dlg.BindSel(*id);
					
					dlg.SetCurGrp(StrLib_Get()->FindGroup(toMBCS((LPCTSTR)item->GetDescription())));

					if (IDCANCEL!=dlg.DoModal())
					{
						StringID idNew=dlg.GetSel();
						OnBeginItemChange(item);
						(*id)=idNew;
						OnItemChange(item);
						OnEndItemChange(item);
					}
				}
			}
		}
	}

	return 0;
}

extern void DoElemCommand(CRichGrid *grid,CXTPPropertyGridItem *item,void *owner,GElemBase *elem,int iSub,DWORD idButton);

void CGObjGrid::OnClone()
{
	DoElemCommand(this,_cursub.item,_cursub.owner,_cursub.elem,_cursub.iSub,ID_RGIB_Clone);
}

void CGObjGrid::OnMoveUp()
{
	DoElemCommand(this,_cursub.item,_cursub.owner,_cursub.elem,_cursub.iSub,ID_RGIB_MoveUp);
}

void CGObjGrid::OnMoveDown()
{
	DoElemCommand(this,_cursub.item,_cursub.owner,_cursub.elem,_cursub.iSub,ID_RGIB_MoveDown);
}

void CGObjGrid::OnRemove()
{
	DoElemCommand(this,_cursub.item,_cursub.owner,_cursub.elem,_cursub.iSub,ID_RGIB_Remove);
}

void CGObjGrid::OnCollapseAll()
{
	if (!_itemCur)
		return;

	_CollapseItemR(_itemCur);
	_itemCur=NULL;
}

void CGObjGrid::OnExpandAll()
{
	if (!_itemCur)
		return;

	_ExpandItemR(_itemCur);
	_itemCur=NULL;

}


void CGObjGrid::OnCopyElem()
{
	if (_iCurElemEntry<0)
		return;

	if (_iCurElemEntry>=_elems.size())
		return;

	ElemEntry *entry=&_elems[_iCurElemEntry];

	ElemCB *cb=GetElemCB();

	DP_BeginSave(dp,cb->buf);
	if (entry->iSub<0)
		entry->elem->Save(entry->owner,dp,TRUE);
	else
	{
		DWORD c;
		if (entry->elem->GetSubCount(entry->owner,&c))
		{
			if (entry->iSub<c)
			{
				dp.Data_NextByte()=1;
				entry->elem->SaveSub(entry->owner,entry->iSub,dp);
			}
			else
				dp.Data_NextByte()=0;
		}
	}
	DP_EndSave();

	cb->elem=entry->elem;
	cb->iSub=entry->iSub;
}

void CGObjGrid::OnPasteElem()
{
	if (_iCurElemEntry<0)
		return;

	if (_iCurElemEntry>=_elems.size())
		return;

	ElemEntry *entry=&_elems[_iCurElemEntry];

	ElemCB *cb=GetElemCB();

	if (cb->elem)
	{
		if (cb->elem->CheckCompatible(entry->elem))
		{

			OnBeginItemChange(entry->item);

			CDataPacket dp;
			dp.SetDataBufferPointer(&cb->buf[0]);

			if ((entry->iSub<0)&&(cb->iSub<0))
				entry->elem->Load(entry->owner,dp,TRUE);
			if ((entry->iSub>=0)&&(cb->iSub>=0))
			{
				DWORD c;
				if (entry->elem->GetSubCount(entry->owner,&c))
				{
					if (entry->iSub<c)
					{
						if (dp.Data_NextByte()==1)
						{//CB里确实有数据
							entry->elem->LoadSub(entry->owner,entry->iSub,dp);
						}
					}
				}
			}

			OnItemChange(entry->item);
			OnEndItemChange(entry->item);
		}
	}
}

void CGObjGrid::OnPasteAgent()
{
	if (_iCurElemEntry<0)
		return;

	if (_iCurElemEntry>=_elems.size())
		return;

	ElemEntry *entry=&_elems[_iCurElemEntry];

	GObjBase *objSub;
	if (entry->elem->GetObj(entry->owner,&objSub))
	{
		if ( ( GSem_Unknown == entry->elem->sem.code) && ( std::string("LoAgentRef") == objSub->GetName() ) )
		{
			std::string strCB;
			extern BOOL PasteFromClipboard(CWnd *wnd,std::string &str);

			if (PasteFromClipboard(this,strCB))
			{
				std::vector<std::string> temp;
				SplitStringBy("||",strCB,&temp);

				if (temp.size()==6)
				{
					if (temp[0]=="LoAgentRef")
					{
						OnBeginItemChange(entry->item);
						LoAgentRef* refAgent = static_cast<LoAgentRef*>( objSub->GetOwner() );
						refAgent->idMap=(RecordID)IntFromString(temp[4].c_str());
						refAgent->idAgent=(RecordID)IntFromString(temp[5].c_str());
						refAgent->guid=(DWORD)IntFromString(temp[3].c_str());

						OnItemChange(entry->item);
						OnEndItemChange(entry->item);
					}

				}

			}
		}
	}



	ElemCB *cb=GetElemCB();

	if (cb->elem)
	{
		if (cb->elem->CheckCompatible(entry->elem))
		{

			OnBeginItemChange(entry->item);

			CDataPacket dp;
			dp.SetDataBufferPointer(&cb->buf[0]);

			if ((entry->iSub<0)&&(cb->iSub<0))
				entry->elem->Load(entry->owner,dp,TRUE);
			if ((entry->iSub>=0)&&(cb->iSub>=0))
			{
				DWORD c;
				if (entry->elem->GetSubCount(entry->owner,&c))
				{
					if (entry->iSub<c)
					{
						if (dp.Data_NextByte()==1)
						{//CB里确实有数据
							entry->elem->LoadSub(entry->owner,entry->iSub,dp);
						}
					}
				}
			}

			OnItemChange(entry->item);
			OnEndItemChange(entry->item);
		}
	}
}

BOOL GetElemVar(GElemBase* elem, void *owner,int iSub, void* &var)
{
	if (iSub < 0)
		return elem->GetVar(owner, &var);
	return elem->GetSubVar(owner, iSub, &var);
}

BOOL GetElemFilePath(GElemBase* elem, void* owner, int iSub, std::string& pathAbs)
{
	if (elem)
	{
		if (elem->GetVarType()==GVT_String)
		{
			std::string path;
			void *var=NULL;
			if (GetElemVar(elem, owner, iSub, var))
			{
				std::string &s=*(std::string*)var;

				switch(elem->GetSem().code)
				{
					case GSem_TexturePartPath:
					{
						i_math::rect_sh rc;
						ParseShellImageStr(s,path,rc);
						break;
					}
					case GSem_TexturePath:
					case GSem_MeshPath:
					case GSem_MtrlPath:
					case GSem_BoneAnimPath:
					case GSem_XformAnimPath:
					case GSem_MtrlColorAnimPath:
					case GSem_UvAnimPath:
					case GSem_DummiesPath:
					case GSem_SptPath:
					case GSem_MoppPath:
					case GSem_SpgPath:
					case GSem_AnimTreePath:
					case GSem_BoneAnim2Path:
					case GSem_MtrlExtPath:
					case GSem_SoundPath:
					case GSem_RagdollPath:
					case GSem_DtrPath:
					case GSem_BehaviorGraphPath:
						path=s;
						break;
				}

				if (!path.empty())
				{
					pathAbs=g_ssGuiLib.pRS->GetPath(Path_Res);
					pathAbs=pathAbs+"\\"+path;
	// 				pathAbs=GetFileFolderPath(pathAbs);
					return TRUE;
				}
			}
		}
	}

	return FALSE;

}

BOOL GetElemOpenableResPath(GElemBase* elem, void* owner, int iSub, std::string& pathAbs)
{
    if (elem)
    {
        if (elem->GetVarType() == GVT_String)
        {
            std::string path;
            void *var = NULL;
			if (GetElemVar(elem, owner, iSub, var))
            {
                std::string &s = *(std::string*)var;

                switch (elem->GetSem().code)
                {
                case GSem_TexturePath:
                case GSem_MeshPath:
                case GSem_MtrlPath:
//                 case GSem_BoneAnimPath:
//                 case GSem_XformAnimPath:
//                 case GSem_MtrlColorAnimPath:
//                 case GSem_UvAnimPath:
//                 case GSem_DummiesPath:
//                 case GSem_SptPath:
//                 case GSem_MoppPath:
//                 case GSem_SpgPath:
                case GSem_AnimTreePath:
//                 case GSem_BoneAnim2Path:
                case GSem_MtrlExtPath:
//                 case GSem_SoundPath:
//                 case GSem_RagdollPath:
//                 case GSem_DtrPath:
                case GSem_BehaviorGraphPath:
                    path = s;
                    break;
                }

                if (!path.empty())
                {
                    pathAbs = g_ssGuiLib.pRS->GetPath(Path_Res);
                    pathAbs = pathAbs + "\\" + path;
                    // 				pathAbs=GetFileFolderPath(pathAbs);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;

}

BOOL GetElemOpenableProtoID(GElemBase* elem, void* owner, int iSub, ProtoID &idProto)
{
	if (elem)
	{
		if (elem->GetVarType() == GVT_Bx8)
		{
			void* var = NULL;
			if (GetElemVar(elem, owner, iSub, var))
			{
				if (elem->GetSem().code == GSem_ProtoPath)
				{
					idProto = *(ProtoID*)var;
					if (idProto!=ProtoID_Null)
						return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL GetElemOpenableRecordID(GElemBase* elem, void* owner, int iSub,std::string&nmRecords, RecordID& idRec)
{
	if (elem)
	{
		if (elem->GetVarType() == GVT_U)
		{
			void* var = NULL;
// 			if (elem->GetVar(owner, &var))
			if (GetElemVar(elem,owner,iSub,var))
			{
				if (elem->GetSem().code == GSem_RecordID)
				{
					nmRecords = elem->GetSem().constraint;
					idRec = *(RecordID*)var;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


void CGObjGrid::OnLocateFile()
{
	if (_iCurElemEntry<0)
		return;

	if (_iCurElemEntry>=_elems.size())
		return;

	ElemEntry *entry=&_elems[_iCurElemEntry];

	std::string pathAbs;
	if (GetElemFilePath(entry->elem,entry->owner,entry->iSub,pathAbs))
	{
		std::string arg="/select,";
		arg=arg+pathAbs;
		ShellExecute(NULL, _T("open"), _T("explorer.exe"), fromMBCS(arg.c_str()), NULL, SW_SHOWNORMAL);
	}

}

void CGObjGrid::OnOpenRes()
{
    if (_iCurElemEntry < 0)
        return;

    if (_iCurElemEntry >= _elems.size())
        return;

    ElemEntry *entry = &_elems[_iCurElemEntry];

    std::string pathAbs;
	if (GetElemOpenableResPath(entry->elem, entry->owner, entry->iSub, pathAbs))
	{
		if (!ExistResEditor())
	        OpenResEditor();
        g_reg.WriteString("[OpenRes]", "ResPath", pathAbs.c_str());
        g_reg.SendEvent("[OpenRes]_Request");
    }

}

void CGObjGrid::OnOpenProto()
{
	if (_iCurElemEntry < 0)
		return;

	if (_iCurElemEntry >= _elems.size())
		return;

	ElemEntry* entry = &_elems[_iCurElemEntry];

	ProtoID idProto;
	if (GetElemOpenableProtoID(entry->elem, entry->owner, entry->iSub,idProto))
	{
		extern BOOL ExistProtoEditor();
		extern void OpenProtoEditor();
		if (!ExistProtoEditor())
			OpenProtoEditor();
		extern CCurrentUserRegistry g_reg;
		g_reg.WriteData("[OpenProto]", "ProtoID", &idProto, sizeof(idProto));
		g_reg.SendEvent("[OpenProto]_Request");
	}

}

void CGObjGrid::OnOpenRecord()
{
	if (_iCurElemEntry < 0)
		return;

	if (_iCurElemEntry >= _elems.size())
		return;

	ElemEntry* entry = &_elems[_iCurElemEntry];

	RecordID idRec;
	std::string nmRecords;
	if (GetElemOpenableRecordID(entry->elem, entry->owner, entry->iSub, nmRecords, idRec))
	{
		extern BOOL ExistRecordEditor();
		extern void OpenRecordEditor();
		if (!ExistRecordEditor())
			OpenRecordEditor();
		extern CCurrentUserRegistry g_reg;
		g_reg.WriteData("[OpenRecord]", "RecordID", &idRec, sizeof(idRec));
		g_reg.WriteString("[OpenRecord]", "RecordsName", nmRecords.c_str());
		g_reg.SendEvent("[OpenRecord]_Request");
	}
}



MtrlData *CGObjGrid::_MtrlDataFromItem(CXTPPropertyGridItem *item)
{
	while(item)
	{
		for (int i=0;i<_mds.size();i++)
		{
			if (_mds[i].item==item)
				return _mds[i].md;
		}
		item=item->GetParentItem();
	}
	return NULL;
}


void CGObjGrid::_RepairMtrlData(CXTPPropertyGridItem *item)
{
	MtrlData *md=_MtrlDataFromItem(item);
	if (!md)
		return;

	g_ssGuiLib.pUtilRS->RepairResData(md);
}


void CGObjGrid::_SwitchAnimType(int at)
{
	CXTPPropertyGridItem *item=GetSelectedItem();
	MtrlDataItemInfo *mditem=NULL;
	if (TRUE)
	{
		std::map<CXTPPropertyGridItem *,MtrlDataItemInfo>::iterator it=_mditems.find(item);
		if (it!=_mditems.end())
			mditem=&((*it).second);
	}
	FeatureParamA *fp=NULL;
	if (mditem)
		fp=mditem->fp;

	if (fp)
	{

		OnBeginItemChange(item);

		fp->at=(FeatureParamA::AnimType)at;
		if (fp->at==FeatureParamA::Anim_ValueSet)
		{
			//重置这个ValueSet的初始值
			KeyType kt=mditem->kt;
			if (kt!=fp->vs.GetKeyType())
			{
				extern i_math::rectf GetSemLimitRect(GSem &sem);
				i_math::rectf rc=GetSemLimitRect(mditem->sem);
				if (kt==KT_Float)
					fp->vs.ResetFloat(rc.Bottom());
				if (kt==KT_Color)
					fp->vs.ResetColor(0xffffffff);
			}
		}

		OnItemChange(item);
		OnEndItemChange(item);

	}
}


void CGObjGrid::OnAnimNone()
{
	_SwitchAnimType(FeatureParamA::Anim_None);

}

void CGObjGrid::OnAnimValueSet()
{
	_SwitchAnimType(FeatureParamA::Anim_ValueSet);
}

void CGObjGrid::OnAnimRes()
{
	_SwitchAnimType(FeatureParamA::Anim_Res);
}

void CGObjGrid::_RenderDelta(CXTPPropertyGridItem *item)
{
//	item->GetValueMetrics()->m_clrFore = 0xff7f00;
	item->GetValueMetrics()->m_clrBack = RGB(0x9f,0xcf, 0xff);
//	item->GetCaptionMetrics()->m_clrFore = 0xff7f00;
	item->GetCaptionMetrics()->m_clrBack = RGB(0x7f,0xaf, 0xff);
	CXTPPropertyGridItems *ca=item->GetChilds();

	DWORD sz=ca->GetCount();
	for (int i=0;i<sz;i++)
		_RenderDelta(ca->GetAt(i));
}

void CGObjGrid::RenderDelta(std::vector<void*> ptrs)
{
	for (int i=0;i<ptrs.size();i++)
	{
		std::unordered_map<void*,CXTPPropertyGridItem*>::iterator it=_binds.find(ptrs[i]);
		if (it!=_binds.end())
		{
			CXTPPropertyGridItem *item=(*it).second;
			_RenderDelta(item);
		}
	}
}
