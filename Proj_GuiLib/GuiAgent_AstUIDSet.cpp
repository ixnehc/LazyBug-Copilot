/********************************************************************
created:	2008/5/20   18:14
file path:	d:\IxEngine\Proj_GuiLib
author:		cxi

purpose:	the gui agent used in editing proto
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IFont.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/ITools.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/ILuaMachine.h"
#include "WorldSystem/IAssetBodyMap.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"


#include "GuiData.h"

#include "GuiData_proto.h"
#include "GuiData_protologic.h"
#include "GuiData_debugger.h"
#include "GuiData_entitymap.h"

#include "GuiData_RichGrids.h"

#include "GuiAgent_AstUIDSet.h"

#include "commondefines/general_stl.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetRenderer.h"

#include "timer/profiler.h"
#include "Registry/Registry.h"

#include "AgentCmdID.h"

#include "graphicsgraph.h"

#include "RichGrid.h"

#include "RichGridAstUIDSetItem.h"

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_AstUIDSet::Binding
BOOL CGuiAgent_AstUIDSet::Binding::IsValid()
{
	if(grid&&item)
		return TRUE;
	return FALSE;
}

std::vector<DWORD> *CGuiAgent_AstUIDSet::Binding::GetBindUIDs()
{
	if (item)
		return item->GetBindUIDs();
	return NULL;
}


void CGuiAgent_AstUIDSet::Binding::OnBeginChange()
{
	if (item&&grid)
		grid->OnBeginItemChange(item);
}

void CGuiAgent_AstUIDSet::Binding::OnChange()
{
	if (item&&grid)
		grid->OnItemChange(item);
}

void CGuiAgent_AstUIDSet::Binding::OnEndChange()
{
	if (item&&grid)
	{
		item->UpdateValue();
		grid->OnEndItemChange(item);
	}
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_AstUIDSetEdit
void CGuiAgent_AstUIDSet::OnAttachView(CGeView *view,DWORD iLevel)
{
	CGuiAgent::OnAttachView(view,iLevel);
}

void CGuiAgent_AstUIDSet::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiAgent::OnDetachView(view,iLevel);
}


CGuiAgent_AstUIDSet::Binding CGuiAgent_AstUIDSet::_GetCurBinding()
{
	Binding binding;
	binding.grid=_GetCurGrid();
	binding.item=_GetCurItem();

	return binding;
}

BOOL CGuiAgent_AstUIDSet::OnTimer(int dt,DWORD flag)
{
	_UpdateCur();

	CGuiAgent::Enable(TRUE);
	return TRUE;
}


void CGuiAgent_AstUIDSet::_UpdateCur()
{
	GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
	std::string curGrid,curItem;

	CXTPPropertyGridItem *item=NULL;
	if (dataRG)
	{
		curGrid=dataRG->GetCurGrid();

		if (!curGrid.empty())
		{
			CRichGrid *grid=dataRG->FindGrid(curGrid.c_str());
			item=grid->GetSelectedItem();
			if (item)
				curItem=grid->PathFromItem(item);
		}
	}

	if ((curGrid==_curGrid)&&(curItem==_curItem))
		return;//没有任何变化

	if (curGrid=="")
	{//focus不在某一个grid上

		//检查原来的grid是不是还是在选中原来的item
		CRichGrid *grid=dataRG->FindGrid(_curGrid.c_str());
		if (grid)
		{
			std::string s;
			item=grid->GetSelectedItem();
			if (item)
				s=grid->PathFromItem(item);
			if (s==_curItem)
				return;//原来的grid仍然选中原来的item(虽然原来的grid已经失去focus),我们看作没有变化
		}
	}


	//发生了变化
	_curGrid=curGrid;
	_curItem=curItem;

	_bWorking=FALSE;

	if (item)
	{
		if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_AstUIDSetItem)))
			_bWorking=TRUE;
	}
}

CRichGrid *CGuiAgent_AstUIDSet::_GetCurGrid()
{
	if (_curGrid.empty())
		return NULL;
	GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
	return dataRG->FindGrid(_curGrid.c_str());
}

CRichGrid_AstUIDSetItem *CGuiAgent_AstUIDSet::_GetCurItem()
{
	CRichGrid *grid=_GetCurGrid();
	if (grid)
	{
		CXTPPropertyGridItem *item=grid->GetSelectedItem();
		if (item)
		{
			if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_AstUIDSetItem)))
			{
				if (_curItem==grid->PathFromItem(item))
					return (CRichGrid_AstUIDSetItem *)item;
			}
		}
	}
	return NULL;
}


BOOL CGuiAgent_AstUIDSet::OnLButtonClick(int x,int y,DWORD flag)
{
	_UpdateCur();

	if (!_bWorking)
		return TRUE;

	_Select(x,y);
	_Redraw(FALSE);

	return FALSE;
}


extern EntityAddress HitTestOnMap(IEntitySystem *pES,i_math::line3df &ray,IRenderPort *rp);

void CGuiAgent_AstUIDSet::_Select(int x,int y)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return;

	IRenderPort *rp=GetRP();
	if (!rp)
		return;

	HitProbe probe;
	if (rp->CalcHitProbe(x,y,probe))
	{
		EntityAddress addr=HitTestOnMap(data->pES,probe,rp);
		if (addr!=EntityAddress_Null)
		{
			IEntity *en=data->mp->ToEntity(addr);
			if (en)
			{
				extern AssetUID GetAssetUID(IEntity *en);
				AssetUID uid=GetAssetUID(en);
				if (uid!=AssetUID_Null)
				{
					Binding binding=_GetCurBinding();

					binding.OnBeginChange();
					std::vector<DWORD> *uids= binding.GetBindUIDs();
					if (uids)
					{
						int idx;
						VEC_FIND(*uids,uid,idx);
						if (idx>=0)
						{
							VEC_REMOVE(*uids,uid);
						}
						else
						{
							uids->push_back(uid);
						}
						binding.OnChange();
					}
					binding.OnEndChange();
				}
			}
		}
	}
}


BOOL CGuiAgent_AstUIDSet::OnDraw()
{
	if (!_bWorking)
		return TRUE;

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;

	if (TRUE)
	{
		DrawFontArg arg;
		arg.SetLocation(10,10);
		rp->DrawText(" {C:0,255,0}{S:16}[ 选择Entity模式,左键双击退出 ]",arg);
	}

	if (_bInDrag)
		rp->FrameRect(_rcDraw,ColorAlpha(0x7fbfff,0xaf));

	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return TRUE;

	Binding binding=_GetCurBinding();
	std::vector<DWORD> *uids= binding.GetBindUIDs();
	if (!uids)
		return TRUE;

	ICamera *cam=rp->GetCamera();
	if (cam)
	{
		i_math::vector3df pos;
		if (cam->GetEyePos(pos))
		{
			static EntityAddress bufs[1024];
			DWORD c;
			c=data->mp->EnumEntities(pos.x,pos.z,100.0f,bufs,ARRAY_SIZE(bufs));

			Envelope evlp;

			for (int i=0;i<c;i++)
			{
				extern AssetUID GetAssetUID(IEntity *en);

				IEntity *en=data->mp->ToEntity(bufs[i]);
				if (en)
				{
					extern AssetUID GetAssetUID(IEntity *en);
					AssetUID uid=GetAssetUID(en);
					if (uid!=AssetUID_Null)
					{
						int idx;
						VEC_FIND(*uids,uid,idx);
						if (idx>=0)
						{
							data->pES->CollectEnvelopeOnMap(bufs[i],evlp);

							extern void DrawEnvelope(IRenderPort *rp,Envelope &evlp,DWORD c);
							DrawEnvelope(rp,evlp,0x3f7fff);
							evlp.Clear();
						}
					}
				}
			}
		}
	}

	return TRUE;
}

BOOL CGuiAgent_AstUIDSet::OnBeginDrag(int x,int y,DWORD flag)
{
	if (!_bWorking)
		return FALSE;


	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe,2000.0f))
		return TRUE;

	Binding binding=_GetCurBinding();
	std::vector<DWORD> *uids= binding.GetBindUIDs();
	if (!uids)
		return FALSE;

	binding.OnBeginChange();

	_bAccum=FALSE;
	if (flag&CtrlOpFlag_CtrlDown)
	{
		_initials.clear();
		(*uids).clear();
	}
	else
	{
		if (flag&CtrlOpFlag_ShiftDown)
		{
			_bAccum=TRUE;
			_initials=*uids;
		}
		else
		{
			_initials=*uids;
		}
	}


	_start.set(x,y);
	_rcDraw.set(_start.x,_start.y,_start.x,_start.y);


	return TRUE;
}

void CGuiAgent_AstUIDSet::OnEndDrag(int x,int y,DWORD flag)
{
	OnDrag(x,y,flag);
	Binding binding=_GetCurBinding();
	binding.OnEndChange();

}

void CGuiAgent_AstUIDSet::OnDrag(int x,int y,DWORD flag)
{
	IRenderPort *rp=GetRP();
	if (!rp)
		return;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe))
		return;

	Binding binding=_GetCurBinding();

	i_math::recti rc;
	rc.set(_start.x,_start.y,x,y);
	rc.repair();

	i_math::volumeCvxf vol;
	if (rp->CalcHitVolume(rc,vol))
	{
		GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
		if (data)
		{
			DWORD c;
			EntityAddress *addr=data->pES->VolumeHitTestOnMap(vol,c);
			_Sel(addr,c);
			binding.OnChange();
		}
	}

	_rcDraw=rc;

	_Redraw(TRUE);

}

void CGuiAgent_AstUIDSet::_Sel(EntityAddress*inrects,DWORD c)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");

	Binding binding=_GetCurBinding();
	std::vector<DWORD> *uids= binding.GetBindUIDs();
	if (!uids)
		return;

	*uids=_initials;

	for (int i=0;i<c;i++)
	{
		EntityAddress id=inrects[i];

		IEntity *en=data->mp->ToEntity(id);
		if (en)
		{
			extern AssetUID GetAssetUID(IEntity *en);
			AssetUID uid=GetAssetUID(en);

			int idx;
			VEC_FIND(*uids,uid,idx);
			if (idx==-1)
			{//原来没有,我们添加
				(*uids).push_back(uid);
			}
			else
			{//原来有的,我们删除
				if (!_bAccum)
					(*uids).erase((*uids).begin()+idx);
			}

		}
	}
}

BOOL CGuiAgent_AstUIDSet::OnLButtonDblClk(int x,int y,DWORD flag)
{
	_UpdateCur();

	if (!_bWorking)
		return TRUE;

	_curGrid="";
	_curItem="";
	_bWorking=FALSE;

	_UpdateCur();
	return FALSE;
}
