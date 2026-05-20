/********************************************************************
	created:	2006/11/1   15:09
	filename: 	e:\IxEngine\Proj_GuiLib\MtrlGrid.cpp
	author:		cxi
	
	purpose:	a grid to edit the content of a MtrlData
*********************************************************************/
#include "stdh.h"

#include "MtrlGrid.h"

#include "RenderSystem/IShader.h"

#include "RenderSystem/IUtilRS.h"

#include "RichGridButtonItem.h"
#include "RichGridComboItem.h"
#include "RichGridTexItem.h"
#include "RichGridResItem.h"

#include "RichGridIntItem.h"

#include "resdata/MtrlData.h"
#include "resdata/MtrlExtData.h"

#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include "MtrlEditPanel.h"

#include "shaderlib/SLDefines.h"
#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_FP_ANIMNONE 1576
#define ID_FP_ANIMVALUESET 1577
#define ID_FP_ANIMRES 1578


//////////////////////////////////////////////////////////////////////////
//CMtrlGrid
BEGIN_MESSAGE_MAP(CMtrlGrid,CRichGrid)
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
	ON_COMMAND(ID_FP_ANIMNONE,OnAnimNone)
	ON_COMMAND(ID_FP_ANIMVALUESET,OnAnimValueSet)
	ON_COMMAND(ID_FP_ANIMRES,OnAnimRes)
END_MESSAGE_MAP()

BOOL CMtrlGrid::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CRichGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	SetHelpHeight(40);

	return TRUE;
}

RGState &CMtrlGrid::_GetRGState()
{		
	return ((Reps_Mtrl*)_state)->stateRG;	
}



i_math::rectf GetSemLimitRect(GSem &sem)
{
	switch(sem.code)
	{
		case GSem_Shiness:
			return i_math::rectf(0,0,-1.0f,300.0f);
		case GSem_ShineStr:
			return i_math::rectf(0,0,-1.0f,10.0f);
		case GSem_Alpha:
			return i_math::rectf(0,0,-1.0f,1.0f);
		case GSem_Float:
		{
			std::vector<std::string> pieces;
			SplitStringBy(",",sem.constraint,&pieces);
			float vmin = (float)atof(pieces[0].c_str());
			float vmax = (float)atof(pieces[1].c_str());
			return i_math::rectf(0,vmin,-1.0f,vmax);
		}
	}
	return i_math::rectf(0,0,-1.0f,100.0f);
}

void InsertMtrlDemand(CRichGrid *grid,MtrlDemand &demand)
{
	std::vector<std::string> modes;
	modes.push_back(std::string("没有特殊效果:0"));
	modes.push_back(std::string("(单层)扭曲效果:1"));
	modes.push_back(std::string("(多层)扭曲效果:2"));

	grid->InsertComboItem<DWORD>("Demand","材质的特殊需求",&demand.flags,modes);
}

extern KeyType EPInfo_GetKT(DWORD idx);
extern ResType ResTypeFromKeyType(KeyType kt);



void CMtrlGrid::_InsertLod(MtrlData::Lod&lod,DWORD iLod)
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

		if (_NeedUniFeature())//the unifeature
		{
			std::vector<std::string> constaints;
			std::string names=slmgr->EnumUniFeatures(lod.slib.c_str());
			SplitStringBy(",",names,&constaints);
			constaints.insert(constaints.begin(),std::string(""));
			InsertComboItem("Uni Feature","The unique feature being used",&lod.unifeature,constaints);
		}

		if (_NeedMtrlExt())
		{
			CXTPPropertyGridItem *item=InsertResItem("材质扩展","材质扩展资源的路径",&lod.mte,Res_MtrlExt);
		}

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
							if (item)
							{
								if (_NeedRefPath())
									((CRichGrid_ResItem*)item)->SetOwnerPath(_pathOwner.c_str());
							}
						}
					}
					if (lod.fps[i].at==FeatureParamA::Anim_ValueSet)
					{
						i_math::rectf rc=GetSemLimitRect(sem);

						item=InsertValueSetItem(showname.c_str(),desc.c_str(),&lod.fps[i].vs,rc);
					}
					if (!item)
					{
						item=InsertGVar(showname.c_str(),desc.c_str(),lod.fps[i].var,sem,g_ssGuiLib.pRS);
						if (item)
						{
							if (sem.code==GSem_TexturePath)
							{
								if (_NeedRefPath())
									((CRichGrid_TexItem *)item)->SetOwnerPath(_pathOwner.c_str());
							}
						}
					}

					if (item)
					{
						if (kt!=KT_None)
						{
							MtrlDataItemInfo data;
							data.fp=fp;
							data.kt=kt;
							data.sem=sem;
							_itemdata[item]=data;
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
	
	ExpandAll();
}

void CMtrlGrid::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);

	_pathOwner=state->panel->GetAnchor()->GetRelativePath();

	if (bUpdateCtrl)
	{
		LockPaint();
		ResetContent();
		_itemdata.clear();
		MtrlData *data=_GetMtrlData();
		if (!data)
		{
			UnLockPaint();
			return;
		}

		BeginInsert();
		InsertCategory(_GetGridName(),_GetGridName());

		PushInsert();

			InsertVectorItem<MtrlData::Lod>("LODs","all the level of details",
											&data->lods,ID_RGIB_New|ID_RGIB_Clear);

			PushInsert();
			for (int j=0;j<data->lods.size();j++)
			{
				std::string s;
				FormatString(s,"Lod %02d",j+1);
				InsertVectorElemItem<MtrlData::Lod>(s.c_str(),s.c_str(),&data->lods,j,
					ID_RGIB_MoveUp|ID_RGIB_MoveDown|ID_RGIB_Clone|ID_RGIB_Remove);
				_InsertLod(data->lods[j],j);
			}
			PopInsert();

		PopInsert();

		RestoreState(_GetRGState());
		UnLockPaint();
	}
}

void CMtrlGrid::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	LockPaint();
	RecordState(_GetRGState());//record the grid state before any change occurs
//	RefreshMod();
	
}

void CMtrlGrid::OnItemChange(CXTPPropertyGridItem *item)
{
//	RefreshMod();
}
void CMtrlGrid::EnableCtrl(BOOL bActive)
{
	if(bActive)
		SetReadOnly(FALSE);
	else
		SetReadOnly(TRUE);
}

void CMtrlGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	_Repair();//the data modified by CRichGrid may have error data combo,we should repair them
	RefreshMod(FALSE);

	UnLockPaint();
}


void CMtrlGrid::_Repair()
{
	ResData*data=_GetResData();
	if (!data)
		return;
	g_ssGuiLib.pUtilRS->RepairResData(data);
}

MtrlDataItemInfo *CMtrlGrid::_FindItemData(CXTPPropertyGridItem *item)
{
	std::map<CXTPPropertyGridItem *,MtrlDataItemInfo>::iterator it=_itemdata.find(item);
	if (it==_itemdata.end())
		return NULL;
	return &((*it).second);
}


LRESULT CMtrlGrid::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_PGN_CONTEXTMENU)
	{
		CXTPPropertyGridItem *item=GetSelectedItem();
		MtrlDataItemInfo *data=_FindItemData(item);
		FeatureParamA *fp=NULL;
		if (data)
			fp=data->fp;
		if (fp)
		{
			CMenu menu;	
			menu.CreatePopupMenu();
			int idx=0;
			DWORD flags[3];
			for (int i=0;i<3;i++)
				flags[i]=MF_ENABLED|MF_STRING;
			flags[fp->at]|=MF_CHECKED;
			menu.InsertMenu(idx++, flags[0], ID_FP_ANIMNONE, _T("无动画"));
			menu.InsertMenu(idx++,flags[1],ID_FP_ANIMVALUESET, _T("编辑动画"));
			menu.InsertMenu(idx++,flags[2],ID_FP_ANIMRES, _T("使用动画资源"));
			CPoint point;
			GetCursorPos(&point);
			menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x,point.y,this,NULL );
		}
	}

	return 0;
}

void CMtrlGrid::_SwitchAnimType(int at)
{
	CXTPPropertyGridItem *item=GetSelectedItem();
	MtrlDataItemInfo *data=_FindItemData(item);
	FeatureParamA *fp=NULL;
	if (data)
		fp=data->fp;

	if (fp)
	{
		LockPaint();
		RecordState(_GetRGState());//record the grid state before any change occurs

		fp->at=(FeatureParamA::AnimType)at;
		if (fp->at==FeatureParamA::Anim_ValueSet)
		{
			//重置这个ValueSet的初始值
			KeyType kt=data->kt;
			if (kt!=fp->vs.GetKeyType())
			{
				i_math::rectf rc=GetSemLimitRect(data->sem);
				if (kt==KT_Float)
					fp->vs.ResetFloat(rc.Bottom());
				if (kt==KT_Color)
					fp->vs.ResetColor(0xffffffff);
			}
		}

		RefreshMod(FALSE);
		UnLockPaint();
	}
}


void CMtrlGrid::OnAnimNone()
{
	_SwitchAnimType(FeatureParamA::Anim_None);

}

void CMtrlGrid::OnAnimValueSet()
{
	_SwitchAnimType(FeatureParamA::Anim_ValueSet);
}

void CMtrlGrid::OnAnimRes()
{
	_SwitchAnimType(FeatureParamA::Anim_Res);
}
