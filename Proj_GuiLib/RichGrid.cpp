/********************************************************************
	created:	2006/10/31   17:32
	filename: 	e:\IxEngine\Proj_GuiLib\RichGrid.cpp
	author:		cxi
	
	purpose:	a property grid for easy use
*********************************************************************/

#include "stdh.h"

#include "RichGrid.h"

#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include "resdata/ResDataDefines.h"

#include "resdata/MtrlData.h"
  
#include "gds/GVar.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IEntitySystem.h"

#include "SscBase.h"

#include <assert.h>

#include "RichGridButtonItem.h"
#include "RichGridBoolItem.h"
#include "RichGridColorAlphaItem.h"
#include "RichGridFloatItem.h"
#include "RichGridIntItem.h"
#include "RichGridXformItem.h"
#include "RichGridComboItem.h"
#include "RichGridTexItem.h"
#include "RichGridResItem.h"
#include "RichGridFlagItem.h"
#include "RichGridNameItem.h"
#include "RichGridFolderItem.h"
#include "RichGridFileItem.h"
#include "RichGridVec3Item.h"
#include "RichGridAbbItem.h"
#include "RichGridRectItem.h"
#include "RichGridSizeItem.h"
#include "RichGridRangeItem.h"
#include "RichGridIntRangeItem.h"
#include "RichGridProtoItem.h"
#include "RichGridValueSetItem.h"
#include "RichGridStrIDItem.h"
#include "RichGridRecordIDItem.h"
#include "RichGridDynObjItem.h"
#include "RichGridMatSetItem.h"
#include "RichGridAstUIDSetItem.h"
#include "RichGridRandomItem.h"
#include "RichGridSscUIDItem.h"

#include "log/LogDump.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRichGrid *GetRichGrid(CXTPPropertyGridItem *item)
{
	return (CRichGrid *)(item->GetGrid()->GetPropertyGrid());
}

//////////////////////////////////////////////////////////////////////////
//CRichGrid
RichGridHook *CRichGrid::_hook=NULL;



BOOL CRichGrid::Create(const RECT& rc, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CXTPPropertyGrid::Create(rc,pParentWnd,nID,dwListStyle))
		return FALSE;

	SetPropertySort(xtpGridSortCategorized);
	ShowToolBar(FALSE);
	ShowHelp(TRUE);
	SetTheme(xtpGridThemeOffice2003);

	return TRUE;

}

RichGridHook *CRichGrid::_GetHook()
{
	if (!_NeedHook())
		return NULL;
	if (_hook2)
		return _hook2;
	return _hook;
}

void CRichGrid::ResetContent()
{
	CXTPPropertyGrid::ResetContent();
	_binds.clear();
	if (_GetHook())
		_GetHook()->PostResetContent(this);
}


void CRichGrid::_SetReadOnly(CXTPPropertyGridItems *items,BOOL bReadOnly)
{
	for (int nItem = 0; nItem < items->GetCount(); nItem++)
	{
		CXTPPropertyGridItem* item= items->GetAt(nItem);
		item->SetReadOnly(bReadOnly);
		_SetReadOnly(item->GetChilds(),bReadOnly);
	}

}


void CRichGrid::SetReadOnly(BOOL bReadOnly)
{
	_bReadOnly=bReadOnly;
	_SetReadOnly(GetCategories(),bReadOnly);
}

void CRichGrid::_CollapseItemR(CXTPPropertyGridItem*item)
{
	item->Collapse();

	CXTPPropertyGridItems *ca=item->GetChilds();

	DWORD sz=ca->GetCount();
	for (int i=0;i<sz;i++)
		_CollapseItemR(ca->GetAt(i));
}


void CRichGrid::_ExpandItemR(CXTPPropertyGridItem*item)
{
	item->Expand();

	CXTPPropertyGridItems *ca=item->GetChilds();

	DWORD sz=ca->GetCount();
	for (int i=0;i<sz;i++)
		_ExpandItemR(ca->GetAt(i));
}

void CRichGrid::ExpandAll()
{
	CXTPPropertyGridItems *ca=GetCategories();

	DWORD sz=ca->GetCount();
	for (int i=0;i<sz;i++)
		_ExpandItemR(ca->GetAt(i));
}

void CRichGrid::Expand()
{
	CXTPPropertyGridItems *ca=GetCategories();

	DWORD sz=ca->GetCount();
	for (int i=0;i<sz;i++)
		ca->GetAt(i)->Expand();
}

CXTPPropertyGridItem *CRichGrid::SeekItem(const char *cap)
{
	CXTPPropertyGridItems*cs=NULL;
	if (!_GetStackTop())
		cs=GetCategories();
	else
		cs=_GetStackTop()->GetChilds();
	CXTPPropertyGridItem* p = cs->FindItem(fromMBCS(cap));
	if (p)
	{
		_itemLast=p;
		return _itemLast;
	}
	return NULL;
}



CXTPPropertyGridItem *CRichGrid::InsertCategory(const char *cap,const char *desc,CXTPPropertyGridItem *item)
{
	if (!_GetStackTop())
		_itemLast=GetGridView().AddCategory(fromMBCS(cap),item);
	else
	{
		if (item)
		{
			_itemLast=_GetStackTop()->AddChildItem(item);
			_itemLast->SetCaption(fromMBCS(cap));
		}
		else
			_itemLast=_GetStackTop()->AddChildItem(new CXTPPropertyGridItemCategory(fromMBCS(cap)));
	}
	_itemLast->SetDescription(fromMBCS(desc));
	return _itemLast;
}

CXTPPropertyGridItem *CRichGrid::InsertItem(const char *cap,const char *desc,const char *value)
{
	RG_ADDCHILDITEM(item,CXTPPropertyGridItem,cap,desc);
	item->SetValue(fromMBCS(value));
	return item;
}




CXTPPropertyGridItem *CRichGrid::InsertBoolItem(const char *cap,const char *desc,BOOL *b,const char *constraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_BoolItem,cap,desc);
	item->Bind(b,constraint);
	_AddItemBind(item,b);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertBoolItem(const char *cap,const char *desc,BYTE*b,const char *constraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_BoolItem,cap,desc);
	item->Bind(b,constraint);
	_AddItemBind(item,b);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertBoolItem(const char *cap,const char *desc,DWORD *flag,DWORD mask,const char *constraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_BoolItem,cap,desc);
	item->Bind(flag,mask,constraint);
	_AddItemBind(item,flag);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertNumItem(const char *cap,const char *desc,int*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_IntItem<int>,cap,desc);
	item->Bind(v,-0x7fffffff,0x7fffffff);
	_AddItemBind(item,v);
	return item;
}
CXTPPropertyGridItem *CRichGrid::InsertNumItem(const char *cap,const char *desc,short*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_IntItem<short>,cap,desc);
	item->Bind(v,-30000,30000);
	_AddItemBind(item,v);
	return item;
}
CXTPPropertyGridItem *CRichGrid::InsertNumItem(const char *cap,const char *desc,WORD*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_IntItem<WORD>,cap,desc);
	item->Bind(v,0,60000);
	_AddItemBind(item,v);
	return item;
}
CXTPPropertyGridItem *CRichGrid::InsertNumItem(const char *cap,const char *desc,BYTE*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_IntItem<BYTE>,cap,desc);
	item->Bind(v,0,255);
	_AddItemBind(item,v);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertRandomItem(const char *cap,const char *desc,DWORD*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_RandomItem,cap,desc);
	item->Bind(v);
	_AddItemBind(item,v);
	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertFloatItem(const char *cap,const char *desc,float *f,float min,float max,float slideSpeed)
{
	RG_ADDCHILDITEM(item,CRichGrid_FloatItem,cap,desc);
	item->Bind(f,min,max);
	_AddItemBind(item,f);
	
	if(slideSpeed==0.0f)
		slideSpeed = (max-min)/100.0f;

	item->SetSlideSpeed(slideSpeed);
	
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertAnimTickItem(const char *cap,const char *desc,AnimTick*v,float min,float max,float slideSpeed)
{
	RG_ADDCHILDITEM(item,CRichGrid_AnimTickItem,cap,desc);
	item->Bind(v,min,max);
	_AddItemBind(item,v);

	if(slideSpeed==0.0f)
		slideSpeed = (max-min)/100.0f;

	item->SetSlideSpeed(slideSpeed);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertVec4Item(const char *cap,const char *desc,i_math::vector4df *vec)
{
	RG_ADDCHILDITEM(item,CRichGridVec4Item,cap,desc);
	item->Bind(vec);
	_AddItemBind(item,vec);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertVec3Item(const char *cap,const char *desc,i_math::vector3df *vec)
{
	RG_ADDCHILDITEM(item,CRichGridVec3Item,cap,desc);
	item->Bind(vec);
	_AddItemBind(item,vec);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertVec2Item(const char *cap,const char *desc,i_math::vector2df *vec)
{
	RG_ADDCHILDITEM(item,CRichGridVec2Item,cap,desc);
	item->Bind(vec);
	_AddItemBind(item,vec);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertSizeItem(const char *cap,const char *desc,i_math::vector2di*sz)
{
	RG_ADDCHILDITEM(item,CRichGridSizeItem,cap,desc);
	item->Bind(sz);
	_AddItemBind(item,sz);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertSizeItem(const char *cap,const char *desc,i_math::vector2db*sz)
{
	RG_ADDCHILDITEM(item,CRichGridSizeItem,cap,desc);
	item->Bind(sz);
	_AddItemBind(item,sz);
	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertPointItem(const char *cap,const char *desc,i_math::vector2di*sz)
{
	RG_ADDCHILDITEM(item,CRichGridPointItem,cap,desc);
	item->Bind(sz);
	_AddItemBind(item,sz);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertPointItem(const char *cap,const char *desc,i_math::vector2db*sz)
{
	RG_ADDCHILDITEM(item,CRichGridPointItem,cap,desc);
	item->Bind(sz);
	_AddItemBind(item,sz);
	return item;
}


CXTPPropertyGridItem * CRichGrid::InsertAbbItem(const char *cap,const char *desc,i_math::aabbox3df *aabb)
{
	RG_ADDCHILDITEM(item,CRichGridAbbItem,cap,desc);
	item->Bind(aabb);
	_AddItemBind(item,aabb);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertRectItem(const char *cap,const char *desc,i_math::recti*rc,BOOL bShowSize,BOOL bNoRepair,int gran)
{
	if (bShowSize)
	{
		RG_ADDCHILDITEM(item,CRichGridRectItem_ShowSize,cap,desc);
		if (bNoRepair)
			item->SetNoRepair();
		item->SetGran(gran);
		item->Bind(rc);
		_AddItemBind(item,rc);
		return item;
	}
	else
	{
		RG_ADDCHILDITEM(item,CRichGridRectItem,cap,desc);
		if (bNoRepair)
			item->SetNoRepair();
		item->SetGran(gran);
		item->Bind(rc);
		_AddItemBind(item,rc);
		return item;
	}
}


CXTPPropertyGridItem *CRichGrid::InsertRangeItem(const char *cap,const char *desc,i_math::rangef*range)
{
	RG_ADDCHILDITEM(item,CRichGridRangeItem,cap,desc);
	item->Bind(range);
	_AddItemBind(item,range);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertRangeItem(const char *cap,const char *desc,i_math::rangei*range)
{
	RG_ADDCHILDITEM(item,CRichGridIntRangeItem,cap,desc);
	item->Bind(range);
	_AddItemBind(item,range);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertUVXformItem(const char *cap,const char *desc,float *f)
{
	RG_ADDCHILDITEM(item,CRichGrid_XformItem,cap,desc);
	item->Bind((i_math::matrix43f *)f,RGXI_Edit_2D);
	_AddItemBind(item,f);
	return item;
}
CXTPPropertyGridItem *CRichGrid::InsertXformItem(const char * cap,const char * desc,i_math::matrix43f * matrix)
{
	RG_ADDCHILDITEM(item,CRichGrid_XformItem,cap,desc);
	item->Bind(matrix);
	_AddItemBind(item,matrix);
	return item;
}
CXTPPropertyGridItem *CRichGrid::InsertUVAddrItem(const char *cap,const char *desc,int *v)
{
	RG_ADDCHILDITEM(item,CRichGrid_ButtonItem,cap,desc);

	std::vector<std::string>constaints;
	std::vector<int>remap;
	constaints.push_back("Wrap");
	constaints.push_back("Mirror");
	constaints.push_back("Clamp");
	remap.push_back(1);
	remap.push_back(2);
	remap.push_back(3);
	PushInsert();

	std::string s;
	for (int i=0;i<2;i++)
	{
		FormatString(s,"%c-AddressMode",'U'+i);
		RG_ADDCHILDITEM(item,CRichGrid_ComboItem<int>,s.c_str(),"");
		item->Bind(&v[i],constaints,&remap);
		_AddItemBind(item,&v[i]);
	}

	PopInsert();

	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertColorItem(const char *cap,const char *desc,float *col)
{
	RG_ADDCHILDITEM(item,CRichGrid_ColorAlphaItem,cap,desc);
	item->Bind(col);
	_AddItemBind(item,col);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertColorItem(const char *cap,const char *desc,DWORD*col)
{
	RG_ADDCHILDITEM(item,CRichGrid_ColorAlphaItem,cap,desc);
	item->Bind(col);
	_AddItemBind(item,col);
	return item;
}



CXTPPropertyGridItem *CRichGrid::InsertTexItem(const char *cap,const char *desc,std::string *s,BOOL bSelPart)
{
	RG_ADDCHILDITEM(item,CRichGrid_TexItem,cap,desc);

	item->Bind(s,bSelPart);
	_AddItemBind(item,s);
	return item; 
}

CXTPPropertyGridItem *CRichGrid::InsertResItem(const char *cap,const char *desc,std::string *s,int restype)
{
	RG_ADDCHILDITEM(item,CRichGrid_ResItem,cap,desc);

	item->Bind(s,restype);
	_AddItemBind(item,s);
	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertNameItem(const char *cap,const char *desc,std::string *s)
{
	RG_ADDCHILDITEM(item,CRichGrid_NameItem,cap,desc);

	item->Bind(s);
	_AddItemBind(item,s);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertComboItem(const char *cap,const char *desc,
												 std::string *str,std::vector<std::string>&constraints)
{
	RG_ADDCHILDITEM(item,CRichGrid_ComboItem<int>,cap,desc);

	item->Bind(str,constraints);
	_AddItemBind(item,str);
	return item;
}



CXTPPropertyGridItem *CRichGrid::InsertButtonItem(const char *cap,const char *desc,DWORD IDs)
{
	RG_ADDCHILDITEM(item,CRichGrid_ButtonItem,cap,desc);

	item->AddButtonMask(IDs);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertFlagItem(const char *cap,const char *desc,std::string *sFlag,const char *sConstraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_FlagItem,cap,desc);

	item->Bind(sFlag,sConstraint);
	_AddItemBind(item,sFlag);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertFlagItem(const char *cap,const char *desc,DWORD *flag,const char *sConstraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_FlagItem,cap,desc);

	item->Bind(flag,sConstraint);
	_AddItemBind(item,flag);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertFlagItem(const char *cap,const char *desc,WORD*flag,const char *sConstraint)
{
	RG_ADDCHILDITEM(item,CRichGrid_FlagItem,cap,desc);

	item->Bind(flag,sConstraint);
	_AddItemBind(item,flag);

	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertFolderItem(const char *cap,const char *desc,std::string*s,const char *pathRoot)
{
	RG_ADDCHILDITEM(item,CRichGrid_FolderItem,cap,desc);

	item->SetRootPath(pathRoot);
	item->Bind(s);
	_AddItemBind(item,s);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertFileItem(const char *cap,const char *desc,std::string*v,const char *pathRoot,const char *suffix,const char *filter)
{
	RG_ADDCHILDITEM(item,CRichGrid_FileItem,cap,desc);

	item->SetRootPath(pathRoot);
	item->SetFilter(suffix,filter);
	item->Bind(v);
	_AddItemBind(item,v);

	return item;

}

CXTPPropertyGridItem *CRichGrid::InsertSscUIDItem(const char *cap,const char *desc,DWORD*v)
{
	RG_ADDCHILDITEM(item,CRichGrid_SscUIDItem,cap,desc);

	item->Bind(v);
	_AddItemBind(item,v);

	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertProtoItem(const char *cap,const char *desc,std::string*v,IProtoLib *protolib,BOOL bLuaOnly)
{
	RG_ADDCHILDITEM(item,CRichGrid_ProtoItem,cap,desc);

	item->SetProtoLib(protolib);

	item->Bind(v,bLuaOnly);
	_AddItemBind(item,v);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertProtoItem(const char *cap,const char *desc,unsigned __int64*v,IProtoLib *protolib,BOOL bLuaOnly)
{
	RG_ADDCHILDITEM(item,CRichGrid_ProtoItem,cap,desc);

	item->SetProtoLib(protolib);

	item->Bind(v,bLuaOnly);
	_AddItemBind(item,v);

	return item;
}


// add by yuyang for RichGrid
CXTPPropertyGridItem *CRichGrid::InsertValueSetItem( const char* cap, const char* desc, ValueSet*p,i_math::rectf &rcLimit)
{
	RG_ADDCHILDITEM( item, CRichGrid_ValueSetItem, cap, desc );

	item->Bind(p,rcLimit);
	_AddItemBind(item,p);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::matrix43f> *mats,BOOL bLS,const char *mode)
{
	RG_ADDCHILDITEM( item, CRichGrid_MatSetItem, cap, desc );

	item->Bind(mats);
	_AddItemBind(item,mats);
	item->SetLS(bLS);
	item->SetMode(mode);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::vector3df> *vecs,BOOL bLS,const char *mode)
{
	RG_ADDCHILDITEM( item, CRichGrid_MatSetItem, cap, desc );

	item->Bind(vecs);
	_AddItemBind(item,vecs);
	item->SetLS(bLS);
	item->SetMode(mode);

	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::spheref> *sphs,BOOL bLS,const char *mode)
{
	RG_ADDCHILDITEM( item, CRichGrid_MatSetItem, cap, desc );

	item->Bind(sphs);
	_AddItemBind(item,sphs);
	item->SetLS(bLS);
	item->SetMode(mode);

	return item;
}

// end

CXTPPropertyGridItem *CRichGrid::InsertAstUIDSetItem( const char* cap, const char* desc, std::vector<DWORD> *uids)
{
	RG_ADDCHILDITEM( item, CRichGrid_AstUIDSetItem, cap, desc );

	item->Bind(uids);
	_AddItemBind(item,uids);

	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertStringIDItem(const char *cap,const char *desc,StringID*v,const char *grp,DWORD iCategory)
{
	RG_ADDCHILDITEM(item,CRichGrid_StrIDItem,cap,desc);

	item->Bind(v,grp,iCategory);
	_AddItemBind(item,v);
	return item;
}

CXTPPropertyGridItem *CRichGrid::InsertRecordIDItem(const char *cap,const char *desc,DWORD*v,const char *nameRecords)
{
	RG_ADDCHILDITEM(item,CRichGrid_RecordIDItem,cap,desc);

	item->Bind(v,nameRecords);
	_AddItemBind(item,v);
	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertDynObjItem(const char *cap,const char *desc,void**v,std::unordered_map<std::string,CClass*>&classes,std::unordered_map<std::string,std::string>&names)
{
	RG_ADDCHILDITEM(item,CRichGrid_DynObjItem,cap,desc);

	item->Bind(v,classes,names);
	_AddItemBind(item,v);
	return item;
}


CXTPPropertyGridItem *CRichGrid::InsertGVar(const char *cap,const char *desc,GVar &var,GSem &sem,IRenderSystem *pRS)
{
	assert(VerifySemVarType(sem,var.Type()));

	if (var.type==GVT_String)
		return InsertVar(&var.Str(),cap,desc,var.type,sem);
	else
	{
		DWORD szData;
		return InsertVar((void*)var.GetData(szData),cap,desc,var.type,sem);
	}
}



CXTPPropertyGridItem *CRichGrid::InsertVar(void *var,const char *cap,
										   const char *desc,GVarType vt,GSem &sem)
{
	assert(VerifySemVarType(sem,vt));

	CXTPPropertyGridItem *item=NULL;
	if (sem.code==GSem_ColorAlpha)
	{
		if (sem.constraint=="NumericInput")
			item=InsertVec4Item(cap,desc,(i_math::vector4df*)var);
		else
			item=InsertColorItem(cap,desc,(float*)var);
	}
	if (sem.code==GSem_ColorAlphaU)
		item=InsertColorItem(cap,desc,(DWORD*)var);
	if (sem.code==GSem_Shiness)
	{
		item=InsertFloatItem(cap,desc,(float*)var,1,500);
		((CRichGrid_FloatItem*)item)->SetSlideSpeed(0.5);
	}
	if (sem.code==GSem_ShineStr)
	{
		item=InsertFloatItem(cap,desc,(float*)var,0,10);
		((CRichGrid_FloatItem*)item)->SetSlideSpeed(0.05);
	}

	if ((sem.code==GSem_Alpha)||(sem.code==GSem_TexelLength))
	{
		item=InsertFloatItem(cap,desc,(float*)var,0,1);
		((CRichGrid_FloatItem*)item)->SetSlideSpeed(0.005);
	}
	if (sem.code==GSem_UVXform)
		item=InsertUVXformItem(cap,desc,(float*)var);
	if (sem.code==GSem_UVAddr)
		item=InsertUVAddrItem(cap,desc,(int*)var);
	if (sem.code==GSem_Boolean)
	{
		switch(vt)
		{
			case GVT_S:
			case GVT_U:
				item=InsertBoolItem(cap,desc,(BOOL*)var,sem.constraint.c_str());
				break;
			case GVT_B:
				item=InsertBoolItem(cap,desc,(BYTE*)var,sem.constraint.c_str());
				break;
		}
	}

	if (sem.code==GSem_MeshPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Mesh);
	if (sem.code==GSem_MtrlPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Mtrl);
	if (sem.code==GSem_XformAnimPath)
		item=InsertResItem(cap,desc,(std::string *)var,ResA_XForm);
	if (sem.code==GSem_MtrlColorAnimPath)
		item=InsertResItem(cap,desc,(std::string *)var,ResA_MtrlColor);
	if (sem.code==GSem_UvAnimPath)
		item=InsertResItem(cap,desc,(std::string *)var,ResA_MapCoord);
	if (sem.code==GSem_DummiesPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Dummies);
	if (sem.code==GSem_SptPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Spt);
	if (sem.code==GSem_MoppPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Mopp);
	if (sem.code==GSem_SpgPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Spg);
	if (sem.code==GSem_AnimTreePath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_AnimTree);
	if (sem.code==GSem_BoneAnim2Path)
		item=InsertResItem(cap,desc,(std::string *)var,ResA_Bones2);
	if (sem.code==GSem_MtrlExtPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_MtrlExt);
	if (sem.code==GSem_SoundPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Sound);
	if (sem.code==GSem_RecordsPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Records);
	if (sem.code==GSem_RagdollPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Ragdoll);
	if (sem.code==GSem_DtrPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_Dtr);
	if (sem.code==GSem_BehaviorGraphPath)
		item=InsertResItem(cap,desc,(std::string *)var,Res_BehaviorGraph);
	//XXXXX:more res type

	if (sem.code==GSem_Name)
		item=InsertNameItem(cap,desc,(std::string *)var);
	if (sem.code==GSem_AnimTick)
	{
		std::vector<std::string> pieces;
		SplitStringBy(",",sem.constraint,&pieces);
		if (pieces.size()==3)
		{
			float vmin = -10000.0f;
			float vmax = 10000.0f;
			float slideSpeed = 0.0f;
			vmin = (float)atof(pieces[0].c_str());
			vmax = (float)atof(pieces[1].c_str());
			slideSpeed = (float)atof(pieces[2].c_str());

			item=InsertAnimTickItem(cap,desc,(AnimTick*)var,vmin,vmax,slideSpeed);
		}
	}
	if (sem.code==GSem_Interger)
	{
		if (sem.constraint=="")
		{
			switch(vt)
			{
				case GVT_S:
				case GVT_U:
					item=InsertNumItem(cap,desc,(int*)var);
					break;
				case GVT_SS:
				case GVT_SU:
					item=InsertNumItem(cap,desc,(short*)var);
					break;
				case GVT_B:
					item=InsertNumItem(cap,desc,(BYTE*)var);
					break;
			}
		}
		else
		{
			if (sem.constraint=="RandomSeed")
			{
				if (vt==GVT_U)
					item=InsertRandomItem(cap,desc,(DWORD*)var);
			}
			else
			{
				std::vector<std::string> pieces;
				SplitStringBy(",",sem.constraint,&pieces);
				switch(vt)
				{
					case GVT_S:
					case GVT_U:
						item=InsertComboItem(cap,desc,(int*)var,pieces);
						break;
					case GVT_SS:
					case GVT_SU:
						item=InsertComboItem(cap,desc,(short*)var,pieces);
						break;
					case GVT_B:
						item=InsertComboItem(cap,desc,(char*)var,pieces);
						break;
				}
			}
		}
	}
	if (sem.code==GSem_Flags)
	{
		if (sem.constraint=="")
			item=InsertNumItem(cap,desc,(int*)var);
		else
			item=InsertFlagItem(cap,desc,(DWORD*)var,sem.constraint.c_str());
	}

	if (sem.code==GSem_Float)
	{
		float vmin = -10000.0f;
		float vmax = 10000.0f;
		float slideSpeed = 0.0f;

		if(!sem.constraint.empty())
		{
			std::vector<std::string> pieces;
			SplitStringBy(",",sem.constraint,&pieces);
			vmin = (float)atof(pieces[0].c_str());
			vmax = (float)atof(pieces[1].c_str());
			slideSpeed = (float)atof(pieces[2].c_str());
		}

		item=InsertFloatItem(cap,desc,(float*)var,vmin,vmax,slideSpeed);
	}
	if (sem.code==GSem_FolderPath)
		item=InsertFolderItem(cap,desc,(std::string*)var);
	if (sem.code==GSem_MapFilePath)
		item=InsertFolderItem(cap,desc,(std::string*)var,g_ssGuiLib.pWS->GetPath(WSPath_Map));
	if (sem.code==GSem_TrrnBrushLibPath)
		item=InsertFileItem(cap,desc,(std::string*)var,g_ssGuiLib.pWS->GetPath(WSPath_TrrnBrushLib),"tblib","Terrain Brush Lib File(*.tblib)|*.tblib|All Files (*.*)|*.*||");
	if (sem.code==GSem_BrushLibPath)
	{
		std::string root=g_ssGuiLib.pWS->GetPath(WSPath_BrushLib);
		if (-1==Enums_FindValue(BRLIB_TYPE,sem.constraint.c_str()))
			LOG_DUMP_1P("RichGrid",Log_Error,"无效的BrushLib子目录:%s",sem.constraint.c_str());
		else
			root=root+"\\"+sem.constraint;
		
		item=InsertFileItem(cap,desc,(std::string*)var,root.c_str(),"brlib","BrushLib文件(*.brlib)|*.brlib|All Files (*.*)|*.*||");
	}
	if (sem.code==GSem_TexturePath)
		item=InsertTexItem(cap,desc,(std::string*)var);
	if (sem.code==GSem_TexturePartPath)
		item=InsertTexItem(cap,desc,(std::string*)var,TRUE);
	if (sem.code==GSem_Rect)
	{
		BOOL bShowSize=sem.constraint.find("ShowSize")!=-1;
		BOOL bNoRepair=sem.constraint.find("NoRepair")!=-1;
		int gran=1;
		if (TRUE)
		{
			int idx=sem.constraint.find("Gran:");
			if (idx!=-1)
			{
				std::string s;
				const char *p=&sem.constraint.c_str()[idx];
				p+=5;
				while((*p)&&((*p)!=','))
				{
					s+=*p;
					p++;
				}
				gran=IntFromString(s.c_str());
			}
		}
		item=InsertRectItem(cap,desc,(i_math::recti *)var,bShowSize,bNoRepair,gran);
	}
	if (sem.code==GSem_Aabb)
		item=InsertAbbItem(cap,desc,(i_math::aabbox3df*)var);
	if (sem.code==GSem_Size)
	{
		if ((vt==GVT_Sx2)||(vt==GVT_Ux2))
			item=InsertSizeItem(cap,desc,(i_math::vector2di *)var);
		if (vt==GVT_Bx2)
			item=InsertSizeItem(cap,desc,(i_math::vector2db *)var);
	}
	if (sem.code==GSem_Point)
	{
		if (vt==GVT_Fx2)
			item=InsertVec2Item(cap,desc,(i_math::vector2df *)var);
		else
		{
			if ((vt==GVT_Sx2)||(vt==GVT_Ux2))
				item=InsertPointItem(cap,desc,(i_math::vector2di *)var);
			else
				item=InsertPointItem(cap,desc,(i_math::vector2db *)var);
		}
	}
	if (sem.code==GSem_Range)
	{
		if (vt==GVT_Fx2)
			item=InsertRangeItem(cap,desc,(i_math::rangef *)var);
		else
			item=InsertRangeItem(cap,desc,(i_math::rangei *)var);
	}
	if ((sem.code==GSem_Normal)||(sem.code==GSem_Pos))
		item=InsertVec3Item(cap,desc,(i_math::vector3df *)var);
	if (sem.code==GSem_ProtoPath)
	{
		if (vt==GVT_String)
		{
			if (sem.constraint=="LuaOnly")
				item=InsertProtoItem(cap,desc,(std::string*)var,g_ssGuiLib.pES->GetProtoLib(),TRUE);
			else
				item=InsertProtoItem(cap,desc,(std::string*)var,g_ssGuiLib.pES->GetProtoLib(),FALSE);
		}
		else
		{
			if (sem.constraint=="LuaOnly")
				item=InsertProtoItem(cap,desc,(unsigned __int64*)var,g_ssGuiLib.pES->GetProtoLib(),TRUE);
			else
				item=InsertProtoItem(cap,desc,(unsigned __int64*)var,g_ssGuiLib.pES->GetProtoLib(),FALSE);
		}
	}
	if (sem.code==GSem_StringID)
	{
		if(!sem.constraint.empty())
		{
			std::vector<std::string> pieces;
			SplitStringBy(":",sem.constraint,&pieces);
			if (pieces.size()<=1)
				item=InsertStringIDItem(cap,desc,(StringID*)var,pieces[0].c_str(),STRLIB_CATEGORY_DEFAULT);
			else
			{
				DWORD iCategory=(DWORD)IntFromString(pieces[1].c_str());
				item=InsertStringIDItem(cap,desc,(StringID*)var,pieces[0].c_str(),iCategory);
			}
		}
		else
			item=InsertStringIDItem(cap,desc,(StringID*)var,"",STRLIB_CATEGORY_DEFAULT);
	}
	if (sem.code==GSem_RecordID)
		item=InsertRecordIDItem(cap,desc,(RecordID*)var,sem.constraint.c_str());

	if (sem.code==GSem_Xform)
		item=InsertXformItem(cap,desc,(i_math::matrix43f *)var);
	if (sem.code==GSem_FilePath)
	{
		std::vector<std::string>temp;
		SplitStringBy(":",sem.constraint,&temp);
		std::string sRoot;
		if (temp.size()>0)
		{
			if (temp[0]=="[RESROOT]")
				sRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
			else
				sRoot=temp[0];
		}
		InsertFileItem(cap,desc,(std::string*)var,
			sRoot.c_str(),
			temp.size()>1?temp[1].c_str():"",
			temp.size()>2?temp[2].c_str():"");
	}
	if (sem.code==GSem_GUID)
		item=InsertSscUIDItem(cap,desc,(DWORD*)var);
	if (sem.code==GSem_ObjID)
	{
		switch(vt)
		{
			case GVT_S:
			case GVT_U:
				item=InsertNumItem(cap,desc,(int*)var);
				break;
		}
	}

	return item;
}



void CRichGrid::BeginInsert(CXTPPropertyGridItem *item)
{
	_stackItem.clear();
	if (item)
		_stackItem.push_back(item);

	_stackCaptionHides.push_back(std::set<std::string>());
	_stackCaptionShows.push_back(std::set<std::string>());

	_itemLast=NULL;
}
void CRichGrid::EndInsert()
{
	_stackItem.clear();
	_stackCaptionHides.clear();
	_stackCaptionShows.clear();
	_itemLast=NULL;
}

void CRichGrid::PushInsert()
{
	if (_itemLast)
		_stackItem.push_back(_itemLast);

	_stackCaptionHides.push_back(std::set<std::string>());
	_stackCaptionShows.push_back(std::set<std::string>());
}
void CRichGrid::PopInsert()
{
	if (_stackItem.size()>0)
	{
		_itemLast=_stackItem[_stackItem.size()-1];
		_stackItem.pop_back();
	}

	_stackCaptionHides.pop_back();
	_stackCaptionShows.pop_back();
}

void CRichGrid::AddCaptionHide(const char *caption)
{
	if (_stackCaptionHides.size()>0)
		_stackCaptionHides[_stackCaptionHides.size()-1].insert(std::string(caption));
}

void CRichGrid::AddCaptionShow(const char *caption)
{
	if (_stackCaptionShows.size()>0)
		_stackCaptionShows[_stackCaptionShows.size()-1].insert(std::string(caption));
}


BOOL CRichGrid::_IsCaptionHide(const char *caption)
{
	if (_stackCaptionHides.size()>0)
	{
		if (_stackCaptionHides[_stackCaptionHides.size()-1].find(std::string(caption))!=_stackCaptionHides[_stackCaptionHides.size()-1].end())
			return TRUE;
	}
	return FALSE;
}

BOOL CRichGrid::_IsCaptionShow(const char *caption)
{
	if (_stackCaptionShows.size()>0)
	{
		if (_stackCaptionShows[_stackCaptionShows.size()-1].size()>0)
		{
			if (_stackCaptionShows[_stackCaptionShows.size()-1].find(std::string(caption))==_stackCaptionShows[_stackCaptionShows.size()-1].end())
				return FALSE;
		}
	}
	return TRUE;
}


const char *CRichGrid::PathFromItem(CXTPPropertyGridItem *item)
{
	static std::string path;
	path="";
	while(item)
	{
		path+=_GetPathSep();
		path += toMBCS(item->GetCaption());
		item=item->GetParentItem();
	}
	return path.c_str();
}

CXTPPropertyGridItem *CRichGrid::_ItemFromPath(CXTPPropertyGridItems*items,const char *path)
{
	for (int nItem = 0; nItem < items->GetCount(); nItem++)
	{
		CXTPPropertyGridItem* item= items->GetAt(nItem);
		if (strcmp(PathFromItem(item),path)==0)
			return item;

		item=_ItemFromPath(item->GetChilds(),path);
		if (item)
			return item;
	}
	return NULL;
}

CXTPPropertyGridItem *CRichGrid::ItemFromPath(const char *path)
{
	return _ItemFromPath(GetCategories(),path);
}

CXTPPropertyGridItem *CRichGrid::GetPrevItem(CXTPPropertyGridItem *item)
{
	CXTPPropertyGridItem *parent=item->GetParentItem();
	if (!parent)
		return NULL;

	DWORD idx=parent->GetChilds()->Find(item);
	if (idx==0)
		return NULL;
	return parent->GetChilds()->GetAt(idx-1);
}
CXTPPropertyGridItem *CRichGrid::GetNextItem(CXTPPropertyGridItem *item)
{
	CXTPPropertyGridItem *parent=item->GetParentItem();
	if (!parent)
		return NULL;

	DWORD idx=parent->GetChilds()->Find(item);
	if (idx>=parent->GetChilds()->GetCount()-1)
		return NULL;
	return parent->GetChilds()->GetAt(idx+1);
}



void CRichGrid::_RecordItemState(CXTPPropertyGridItems*items,RGState &stateRG)
{
	for (int nItem = 0; nItem < items->GetCount(); nItem++)
	{
		CXTPPropertyGridItem* item= items->GetAt(nItem);

		std::string path;
		RGItemState state;
		state.bExpand=item->IsExpanded();
		state.bSel=item->IsSelected();
		state.bReadOnly=item->GetReadOnly();
		path=PathFromItem(item);

		stateRG[path]=state;
		_RecordItemState(item->GetChilds(),stateRG);
	}
}

void CRichGrid::_RestoreItemState(CXTPPropertyGridItems*items,RGState &stateRG)
{
	for (int nItem = 0; nItem < items->GetCount(); nItem++)
	{
		CXTPPropertyGridItem* item= items->GetAt(nItem);
		std::string path;
		RGItemState state;
		path=PathFromItem(item);
		RGState::iterator it;
		it=stateRG.find(path);
		if (it!=stateRG.end())
		{
			state=(*it).second;
			if (state.bExpand)
				item->Expand();
			else
				item->Collapse();
			if (state.bSel)
				item->Select();
			item->SetReadOnly(state.bReadOnly);
		}
		else
			item->Expand();

		_RestoreItemState(item->GetChilds(),stateRG);
	}
}


void CRichGrid::RecordState(RGState &stateRG)
{
	stateRG.clear();
	_RecordItemState(GetCategories(),stateRG);

	stateRG.iTopLine=GetGridView().GetTopIndex();
}

void CRichGrid::RestoreState(RGState &stateRG)
{
	_RestoreItemState(GetCategories(),stateRG);

	GetGridView().SetTopIndex(stateRG.iTopLine);
}

void CRichGrid::LockPaint()
{
	GetGridView().LockPaint();
}
void CRichGrid::UnLockPaint()
{
	GetGridView().UnLockPaint();
}

void CRichGrid::_AddItemBind(CXTPPropertyGridItem*item,void *ptr)
{
	_binds[ptr]=item;
}
