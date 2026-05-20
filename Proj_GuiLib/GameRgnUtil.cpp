#include "stdh.h"
#include "commondefines/general_stl.h"
#include "GameRgnUtil.h"
#include "resource.h"
#include "GuiData.h"
#include "GuiData_OverallMap.h"
#include "GuiData_GameRgnMap.h"
#include "FileSystem/IMapFile.h"
#include "WorldSystem/IEntitySystem.h"

#include "RenderSystem/IRecords.h"
#include "RenderSystem/IRenderSystem.h"
#include "records/records.h"

#include "stringparser/stringparser.h"

#include "graphicsgraph.h"

#include "CommonCtrlBase.h"


//////////////////////////////////////////////////////////////
//CGuiAgent_GameRgnMap
CGuiAgent_GameRgnMap::CGuiAgent_GameRgnMap()
{
	_owner=NULL;
	_bDown = FALSE;
	_bModified=FALSE;
}

CGuiAgent_GameRgnMap::~CGuiAgent_GameRgnMap()
{
}

BOOL CGuiAgent_GameRgnMap::OnMouseMove(int x,int y,DWORD flag)
{
	if (!_owner)
		return TRUE;
	if (!_bDown)
		return TRUE;
	GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)FindData("gamergnmap");
	CGameRgnGrids *grids=data->ObtainGrids();
	if (!grids)
		return FALSE;

	_ScreenToGG(x,y);

	DWORD len=grids->GetGridLen();

	int xGrid,yGrid;
	xGrid=i_math::idiv_signed(x,len);
	yGrid=i_math::idiv_signed(y,len);

	int radius=_owner->GetRadius();

	for (int i=-radius;i<=radius;i++)
	for (int j=-radius;j<=radius;j++)
	{
		if (i*i+j*j>radius*radius)
			continue;

		if (grids->GetRect().isPointInside(xGrid+i,yGrid+j))
		{
			GameRgnGrid *grid=grids->GetGrid(xGrid+i,yGrid+j);

			grid->id=_owner->GetSelID();

			_rcDirty.merge(xGrid+i,yGrid+j);
		}
	}

	_bModified=TRUE;

	_Redraw();
	return TRUE;
}

BOOL CGuiAgent_GameRgnMap::OnLButtonDown(int x,int y,DWORD flag)
{
	_bDown = TRUE;
	OccupyFocus(OpType_Mouse);
	OnMouseMove(x,y,flag);
	return TRUE;
}

BOOL CGuiAgent_GameRgnMap::OnLButtonUp(int x,int y,DWORD flag)
{
	_bDown = FALSE;
	DiscardFocus(OpType_Mouse);
	return TRUE;
}



void CGuiAgent_GameRgnMap::_LoadPixels(i_math::recti &rc0,std::unordered_map<DWORD,DWORD>&cols)
{
	GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)FindData("gamergnmap");
	if (!data)
		return;

	CGameRgnGrids *grids=data->ObtainGrids();
	if (!grids)
		return;


	i_math::recti rc=grids->GetRect();
	rc.clipAgainst(rc0);
	rc0=rc;
	if (!rc0.isValid())
		return;

	rc=grids->GetRect();
	DWORD w=rc0.getWidth();
	DWORD h=rc0.getHeight();
	DWORD pitch=rc.getWidth();


	DWORD *q,*q0=&_pixels[(rc0.Top()-rc.Top())*pitch+(rc0.Left()-rc.Left())];
	GameRgnGrid *p,*p0=grids->GetGrid(rc0.Left(),rc0.Top());


	std::unordered_map<DWORD,DWORD>::iterator it;
	for (int j=0;j<h;j++)
	{
		q=q0;
		q0+=pitch;
		p=p0;
		p0+=pitch;

		for (int i=0;i<w;i++)
		{
			if (p->id==0)
				*q=0;
			else
			{
				it=cols.find(p->id);
				if (it!=cols.end())
				{
					*q=(*it).second;
					((BYTE*)q)[3]=128;
				}
			}
			p++;
			q++;
		}
	}

}


void CGuiAgent_GameRgnMap::_EnsurePixels()
{
	GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)FindData("gamergnmap");
	if (!data)
		return;

	CGameRgnGrids *grids=data->ObtainGrids();
	if (!grids)
		return;

	i_math::recti rc=grids->GetRect();
	if (rc.getArea()!=_pixels.size())
	{
		_pixels.resize(rc.getArea());

		VEC_SET(_pixels,0);
		_rcDirty=rc;
	}
}


BOOL CGuiAgent_GameRgnMap::OnDraw()
{
	GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)FindData("gamergnmap");
	if (!data)
		return TRUE;

	CGameRgnGrids *grids=data->ObtainGrids();
	if (!grids)
		return TRUE;

	std::unordered_map<DWORD,DWORD>cols;
	if (TRUE)
	{
		IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes("regions.rcs");
		CRecords *recs=records->GetRecords();
		if (recs)
		{
			RecordID *ids;
			DWORD c;
			ids=recs->GetRecords(c);

			GElemBase *elem=recs->FindElem("col");
			if (elem)
			{
				DWORD *col;
				for (int i=0;i<c;i++)
				{
					CRecord *record=recs->GetRecord(ids[i]);

					if (elem->GetVar(record,(void**)&col))
						cols[ids[i]]=*col;
				}
			}
		}

		SAFE_RELEASE(records);
	}


	GraphicsGraph *gg=GetGG();

	_EnsurePixels();

	_LoadPixels(_rcDirty,cols);

	_rcDirty.set(0,0,0,0);

	if (TRUE)
	{
		i_math::recti rc=grids->GetRect();
		rc*=grids->GetGridLen();

		gg->DrawImageData(rc.convert<float>(),_pixels.data(),grids->GetWidth(),grids->GetHeight());
	}

	if (TRUE)
	{
		i_math::pos2di pos;
		_GetCursorPos(pos);
		_ScreenToGG(pos.x,pos.y);

		int radius=(_owner->GetRadius()*grids->GetGridLen());
		DWORD idSel=_owner->GetSelID();

		i_math::recti rc;
		rc.set(pos.x,pos.y,pos.x,pos.y);
		rc.inflate(radius,radius,radius,radius);

		if (TRUE)
		{
			DWORD col;
			std::unordered_map<DWORD,DWORD>::iterator it=cols.find(idSel);
			if (it!=cols.end())
			{
				col=(*it).second;
				col=RGB(GetBValue(col),GetGValue(col),GetRValue(col));
				gg->FillCircle(pos,radius,col,col,128);
			}
			else
				gg->FrameCircle(pos,radius,RGB(0,0,0),2,128,TRUE);
		}
	}



	return TRUE;
}

BOOL CGuiAgent_GameRgnMap::OnTimer(int dt,DWORD flag)
{
	if (_bModified)
	{
		if (!_bDown)
		{
			GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)FindData("gamergnmap");
			if (!data)
				return TRUE;

			CGameRgnGrids *grids=data->ObtainGrids();
			if (!grids)
				return TRUE;

			grids->Save(data->GetFullPath());
			_bModified=FALSE;
		}
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////
//CGameRgnUtil

IMPLEMENT_TOOL_CLASS(CGameRgnUtil)

CGameRgnUtil::CGameRgnUtil(void)
{
}

CGameRgnUtil::~CGameRgnUtil(void)
{
}
BOOL CGameRgnUtil::InitDlg(CWnd * pParent)
{

	return DefDialog(pParent,IDD_DIALOG_TOOLGAMERGN);
}

void CGameRgnUtil::RegisterAgent()
{
	CMapUtil::RegisterAgent();
	_agent.SetOwner(this);
	AddAgent(_agent,AGENTPRIORITY_STANDARD+10);
}


void CGameRgnUtil::OnInitDlg(CGeActor * actor)
{
	_listRgnID.SubclassDlgItem(IDC_LIST_MAPREGID,&m_panel);

	DWORD extStyle = LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES;

	_listRgnID.SetExtendedStyle(_listRgnID.GetExtendedStyle()|extStyle);
	_listRgnID.InsertColumn(0,_T("区域名称"),LVCFMT_CENTER,80);

	_RefreshRegionList();

	CComboBox *pCB=(CComboBox *)m_panel.GetDlgItem(IDC_RADIUSCOMBO);
	pCB->SetCurSel(10);

}

DWORD CGameRgnUtil::GetSelID()
{
	POSITION pos = _listRgnID.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return 0;
	else
	{
		int nItem = _listRgnID.GetNextSelectedItem(pos);
		return (DWORD)_listRgnID.GetItemData(nItem);
	}
}

DWORD CGameRgnUtil::GetRadius()
{
	CComboBox *pCB=(CComboBox *)m_panel.GetDlgItem(IDC_RADIUSCOMBO);
	if (!pCB)
		return 1;
	int iSel=ComboBox_GetListSel(pCB);
	if (iSel<0)
		return 1;
	return iSel+1;
}



void CGameRgnUtil::_RefreshRegionList()
{
	CGeActor * actor = GetActor();

	GuiData_GameRgnMap * data = (GuiData_GameRgnMap *)actor->FindData("gamergnmap");

	IRecords *records=(IRecords *)data->pRS->GetRecordsMgr()->ObtainRes("regions.rcs");

	CRecords *rec=records->GetRecords();


	_listRgnID.DeleteAllItems();
	if (rec)
	{
		DWORD c;
		RecordID *ids=rec->GetRecords(c);

		std::string name;
		for (int i=0;i<c;i++)
		{
			name=rec->GetName(ids[i]);
			_listRgnID.InsertItem(i, fromMBCS(name.c_str()));

			_listRgnID.SetItemData(i,ids[i]);
		}
	}

	SAFE_RELEASE(records);
}


BOOL CGameRgnUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
// 	if(ctrlID==IDC_BUTTON_REPAIR)
// 	{
// 		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
// 		GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
// 		GuiData_Trrn * dataTrrn = (GuiData_Trrn *)actor->FindData("terrain");
// 		ITrrnMapEditor * editor = dataTrrn->GetTrrnMapEditor();
// 
// 		DWORD w=dataSys->mf->GetRect().getWidth()/dataSys->mf->GetFieldRect().getWidth();
// 
// 		i_math::vector3df centerOld;
// 		dataTrrn->GetTrrnMap()->GetCenter(centerOld);
// 
// 		editor->BeginUpdateDitherColor();
// 
// 		for (int i=0;i<dataMap->fldSels.size();i++)
// 		{
// 			i_math::pos2di ptFld=dataMap->fldSels[i];
// 			i_math::recti rcBlk;
// 			rcBlk.set(ptFld.x,ptFld.y,ptFld.x+1,ptFld.y+1);
// 			rcBlk*=w;
// 
// 			i_math::vector3df center;
// 			center.set((float)(rcBlk.getCenter().x*BLOCK_LENGTH),0,(float)(rcBlk.getCenter().y*BLOCK_LENGTH));
// 			dataTrrn->GetTrrnMap()->SetCenter(center);
// 
// 			editor->UpdateDitherColor(rcBlk);
// 		}
// 
// 		editor->EndUpdateDitherColor();
// 
// 
// 		dataTrrn->GetTrrnMap()->SetCenter(centerOld);
// 
// 		dataTrrn->GetTrrnMap()->SaveModified();
// 
// 		dataTrrn->GetTrrnMap()->ReloadAll();
// 	}

	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}
void CGameRgnUtil::RegisterMode()
{
	AddMode("Paint Game Region",0);
}



