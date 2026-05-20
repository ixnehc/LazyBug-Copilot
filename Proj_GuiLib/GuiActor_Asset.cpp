/********************************************************************
	created:	2008/2/21   15:13
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	asset creating/mofifying panel
*********************************************************************/

#include "stdh.h"
 
#include <vector>
#include <string>

#include "resource.h"

#include "stringparser/stringparser.h"

#include "GuiActor_Acl.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"
#include "FileSystem/IMapFile.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"
#include "GuiActor_Asset.h"

#define ID_DATALOCKED 0
#define ID_DATAUNLOCKED 1



//////////////////////////////////////////////////////////////////////////
//CMod_ChangeAssetMap

void CMod_ChangeAssetMap::Clear()
{
	_blocks.clear();
	_selections.clear();
	_data=NULL;
}

BOOL CMod_ChangeAssetMap::BackupBlock(i_math::pos2di &ptBlk)
{
	if (TRUE)
	{
		DWORD c=_blocks.size();
		_blocks.resize(256);//reserve a big buffer to avoid re-new the internal buffer during push_back(..)
		_blocks.resize(c);
	}

	//check whether already backed-up
	for (int i=0;i<_blocks.size();i++)
	{
		if (_blocks[i].ptBlk==ptBlk)
			return TRUE;
	}

	BYTE *data;
	DWORD szData;
	if (FALSE==_data->mapfile->Load(ptBlk,MapChannel_Asset,data,szData))
		return FALSE;
	_blocks.resize(_blocks.size()+1);
	_BlockData *blk=&_blocks[_blocks.size()-1];
	if (data)
	{
		blk->data.resize(szData);
		memcpy(&blk->data[0],data,szData);
	}

	blk->ptBlk=ptBlk;

	return TRUE;
}

void CMod_ChangeAssetMap::BackupSelection()
{
	_selections=_data->selections;
}


BOOL CMod_ChangeAssetMap::Undo()
{
	if (!_data)
		return FALSE;
	if (!_data->mapfile)
		return FALSE;

	//first back up the current
	std::vector<_BlockData> blocks;
	blocks=_blocks;
	_blocks.clear();
	for (int i=0;i<blocks.size();i++)
		BackupBlock(blocks[i].ptBlk);

	std::vector<AssetAddress>sels;
	sels=_selections;
	_selections.clear();
	BackupSelection();

	BOOL bRet=TRUE;
	//now do the change

	//change the asset data in the map file
	std::vector<i_math::pos2di>blks;
	for (int i=0;i<blocks.size();i++)
	{
		BYTE *data=NULL;
		DWORD szData=0;
		if (blocks[i].data.size()>0)
		{
			data=&blocks[i].data[0];
			szData=blocks[i].data.size();
		}

		if (FALSE==_data->mapfile->Save(blocks[i].ptBlk,MapChannel_Asset,data,szData))
			bRet=FALSE;
		else
			blks.push_back(blocks[i].ptBlk);
	}

	//reload the map
	if (blks.size()>0)
		_data->assetmap->Reload(&blks[0],blks.size());

	//update the selections
	_data->ClearSelection();
	for (int i=0;i<sels.size();i++)
	{
		IAsset *ast=_data->assetmap->FromAddress(sels[i]);
		_data->AddSelection(ast);
	}

	return bRet;
}

BOOL CMod_ChangeAssetMap::Redo()
{
	return Undo();
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ResideAsset

BOOL CGuiAgent_ResideAsset::OnTimer(int dt,DWORD flag)
{
	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)FindData("terrain");
	GuiData_System *dataSystem=(GuiData_System*)FindData("system");
	assert(dataSystem&&dataTrrn);
	IRenderPort *rp=GetRP();

	i_math::pos2di pt;
	_GetCursorPos(pt);

	i_math::recti rc;
	_GetClientRect(rc);

	BOOL bCanReside=FALSE;
	i_math::vector3df vReside;
	if (TRUE)
	{
		HitProbe probe;
		if (rp->CalcHitProbe(pt.x,pt.y,probe))
		{
			if (dataTrrn->trrnmap)
			{
				ITrrnMapEditor *editor=dataTrrn->trrnmap->GetEditor();
				if (editor->GetHitPos(probe,vReside))
					bCanReside=TRUE;
			}
		}
	}

	if (!bCanReside)
	{
		if (_ast)
			_ast->Destroy();
		_ast=NULL;
		_Redraw();
		return TRUE;
	}

	if (_clssid==AssetClassID_Null)
		return TRUE;

	i_math::matrix43f matReside;
	matReside.setTranslation(vReside);

	if (!_ast)
	{//Create one
		AssetCreateArg arg;
		arg.idClass=_clssid;
		arg.pos=matReside;

		_ast=dataSystem->pAS->CreateAsset(arg);
		if(!_ast)
			return TRUE;
	}

	dataSystem->pAS->GetZoner()->Remove(_ast);
	_ast->SetXForm(matReside);
	dataSystem->pAS->GetZoner()->Add(_ast);

	_Redraw();

	return TRUE;
}


BOOL CGuiAgent_ResideAsset::OnLButtonDown(int x,int y,DWORD flag)
{
	GuiData_AssetMap*data=(GuiData_AssetMap*)FindData("assetmap");
	assert(data);
	if (_ast)
	{
		if (data->assetmap->Reside(_ast))
		{

			CMod_ChangeAssetMap *mod=NULL;

			if (_GetModMgr())
			{
				i_math::pos2di ptBlk;
				data->assetmap->FindAsset(_ast,ptBlk);

				mod=new CMod_ChangeAssetMap(data);
				mod->BackupBlock(ptBlk);
				mod->BackupSelection();
			}

			//do the actual modifying
			data->assetmap->SaveModified();
			data->ClearSelection();
			data->AddSelection(_ast);

			if (_GetModMgr())
			{
				_GetModMgr()->NewModGroup();
				_GetModMgr()->PushBack(mod,FALSE);
				_GetModMgr()->PushBack(new CMod_InvalidateView(GetGuiView()),TRUE);
			}

			SAFE_RELEASE(_ast);
		}
	}

	return TRUE;
}

BOOL CGuiAgent_ResideAsset::OnRButtonClick(int x,int y,DWORD flag)
{

	_DetachActor();
	return TRUE;
}

void CGuiAgent_ResideAsset::OnDetachView(CGeView *view,DWORD iLevel)
{
	if (_ast)
		_ast->Destroy();
	_ast=NULL;
	_Redraw(FALSE);
}


BOOL CGuiAgent_ResideAsset::OnDraw()
{
	return TRUE;
}

BOOL CGuiAgent_ResideAsset::OnSetCursor(int x,int y,DWORD flag)
{
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_DrawSelAsset
BOOL CGuiAgent_DrawSelAsset::OnDraw()
{
	GuiData_AssetMap*data=(GuiData_AssetMap*)FindData("assetmap");
	if (!data)
		return TRUE;

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;

	for (int i=0;i<data->selections.size();i++)
	{
		IAsset *ast=data->assetmap->FromAddress(data->selections[i]);
		if (!ast)
			continue;
		i_math::aabbox3df aabb;
		i_math::matrix43f mat;
		if (FALSE==ast->GetLocalAABB(aabb))
			continue;
		if (FALSE==ast->GetXForm(mat))
			continue;

		extern BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,i_math::aabbox3df aabb,DWORD col);
		DrawOBB(rp,mat,aabb,ColorAlpha(0x00ff00,0xff));
	}

	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_SelectAsset
BOOL CGuiAgent_SelectAsset::_Select(int x,int y,DWORD flag)
{
	GuiData_AssetMap*data=(GuiData_AssetMap*)FindData("assetmap");
	if (!data)
		return TRUE;
	if ((!data->assetzoner)||(!data->assetmap))
		return TRUE;

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe))
		return TRUE;


	SpacialTester tester;
	tester.Set((i_math::line3df&)probe);
	data->assetzoner->EnumHit(tester,ZEnum_AssetObb);

	DWORD c;
	IAsset **asts=	data->assetzoner->GetEnumAsset(c);

	for (int i=0;i<c;i++)
	{
		AssetAddress addr=data->assetmap->ToAddress(asts[i]);
		if (addr!=AssetAddress_Null)
		{
			if (!(flag&CtrlOpFlag_CtrlDown))
			{
				data->ClearSelection();
				data->AddSelection(addr);
			}
			else
				data->SwitchSelection(addr);
			_Redraw(FALSE);
			return TRUE;//no need to continue
		}
	}

	return TRUE;
}

BOOL CGuiAgent_SelectAsset::OnLButtonDown(int x,int y,DWORD flag)
{
	return _Select(x,y,flag);
}

BOOL CGuiAgent_SelectAsset::OnRButtonDown(int x,int y,DWORD flag)
{
	return _Select(x,y,flag);
}


BOOL CGuiAgent_SelectAsset::OnSetCursor(int x,int y,DWORD flag)
{
	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_RemoveAsset
BOOL CGuiAgent_RemoveAsset::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_AssetMap*data=(GuiData_AssetMap*)FindData("assetmap");
	if (!data)
		return TRUE;

	if (data->selections.size()>0)
	{
		_AddMenu("Delete",10001);
	}
	return TRUE;
}

void CGuiAgent_RemoveAsset::_Remove()
{
	GuiData_AssetMap*data=(GuiData_AssetMap*)FindData("assetmap");
	assert(data);
	CMod_ChangeAssetMap *mod=NULL;

	std::vector<AssetAddress>removed;
	for (int i=0;i<data->selections.size();i++)
	{
		i_math::pos2di ptBlk;
		IAsset *ast=data->assetmap->FromAddress(data->selections[i]);
		if (ast)
		{
			if (data->assetmap->FindAsset(ast,ptBlk))
			{
				ast->AddRef();
				if (TRUE==data->assetmap->UnReside(ast))
				{
					ast->Destroy();
					removed.push_back(data->selections[i]);
					if (_GetModMgr())
					{
						if (!mod)
							mod=new CMod_ChangeAssetMap(data);
						mod->BackupBlock(ptBlk);
					}
				}
				else
					ast->Release();
			}
		}
	}
	if (mod)
		mod->BackupSelection();

	//do the actual modifying
	if (removed.size()>0)
	{
		data->assetmap->SaveModified();
		for (int i=0;i<removed.size();i++)
			data->SwitchSelection(removed[i]);
	}

	if (mod)
	{
		_GetModMgr()->NewModGroup();
		_GetModMgr()->PushBack(mod,FALSE);
		_GetModMgr()->PushBack(new CMod_InvalidateView(GetGuiView()),TRUE);
	}

}


BOOL CGuiAgent_RemoveAsset::OnCommand(DWORD idCmd)
{
	if (idCmd==10001)
	{
		_Remove();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CAssetDataCategory
void CAssetDataCategory::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (pButton->GetIconIndex()==ID_DATAUNLOCKED)
	{
		//try to lock it(discard the override)
		_ast->OverrideData(_name.c_str(),FALSE);
	}
	else
	{
		//try to unlock it(override it)
		_ast->OverrideData(_name.c_str(),TRUE);
	}

	((CAssetPage*)GetGrid()->GetPropertyGrid())->_ApplyMod();
	((CGuiPanel_Asset*)(GetGrid()->GetPropertyGrid()->GetParent()))->SetForceBind();
}



//////////////////////////////////////////////////////////////////////////
//CMod_ForceBind
class CMod_ForceBind:public CModBase
{
public:
	CMod_ForceBind(CGuiPanel_Asset *panel)
	{
		_panel=panel;
	}
	virtual BOOL IsEmpty()	{		return FALSE;	}

	virtual BOOL Undo()
	{
		_panel->SetForceBind();
		return TRUE;
	}
	virtual BOOL Redo()
	{
		return Undo();
	}

public:
	CGuiPanel_Asset*_panel;
};


//////////////////////////////////////////////////////////////////////////
//CAssetPage
BEGIN_MESSAGE_MAP(CAssetPage, CGObjGrid)
END_MESSAGE_MAP()

void CAssetPage::Reset()
{
	CGObjGrid::Bind(NULL);
	Zero();
}

BOOL CAssetPage::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CGObjGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	ShowToolBar(FALSE);
	CXTPImageManager *im=new CXTPImageManager;
	UINT commands[2]={ID_DATALOCKED,ID_DATAUNLOCKED};
	im->SetIcons(IDB_LOCK,commands,ARRAY_SIZE(commands),CSize(16,11));
	SetImageManager(im);
	return TRUE;

}



void CAssetPage::Bind(AssetAddress addr,BOOL bForceRebind)
{
	BOOL bUpdating=FALSE;
	if (addr==_addr)
	{
		if (!bForceRebind)
			return;
		bUpdating=TRUE;//if force bind,and the asset address is not changed,we should 
										//updat the content(not clean and reset)
	}

	_addr=addr;

	IAsset *ast=_amap->FromAddress(addr);
	if (!ast)
	{
		ResetContent();
		return;
	}


	std::vector<GObjBase *>objs;
	std::vector<BOOL>overrides;
	DWORD sz=0;
	sz=ast->GetDataCount();
	objs.resize(sz);
	overrides.resize(sz);
	for (int i=0;i<sz;i++)
		objs[i]=ast->GetDataObj(i,overrides[i]);

	RGState state;
	if (bUpdating)
	{//record the original item state
		CGObjGrid::RecordState(state);
		CGObjGrid::LockPaint();
	}

	if (TRUE)
	{
		std::vector<CAssetDataCategory*>categories;
		categories.resize(sz);
		for (int i=0;i<sz;i++)
			categories[i]=new CAssetDataCategory(objs[i]->GetName());
		CGObjGrid::Bind(&objs[0],sz,(CXTPPropertyGridItem **)&categories[0]);
	}

	//set the category items' state
	CXTPPropertyGridItems *categories=GetCategories();
	assert(categories->GetCount()==sz);
	for (int i=0;i<sz;i++)
	{
		CAssetDataCategory *item=(CAssetDataCategory *)categories->GetAt(i);
		if (TRUE)//bind the assetdata
		{
			item->Bind(ast,objs[i]->GetName());
		}
		if (TRUE)//Add an inplace button
		{
			CXTPPropertyGridInplaceButton *btn=new CXTPPropertyGridInplaceButton(i);
			if (!overrides[i])
				btn->SetIconIndex(ID_DATALOCKED);//locked
			else
				btn->SetIconIndex(ID_DATAUNLOCKED);//unlocked

			item->GetInplaceButtons()->AddButton(btn);
		}
 		if (!overrides[i])//set readonly if NOT an overridden data
		{
 			SetReadOnly(item,TRUE);
			SetReadOnly(item,FALSE,FALSE);//except the category item
		}
	}

	if (bUpdating)
	{//recover the original state
		CGObjGrid::RestoreState(state);
		CGObjGrid::UnLockPaint();
	}
	else
		CGObjGrid::ExpandAll();
}




void CAssetPage::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	CGObjGrid::OnBeginItemChange(item);
}

void CAssetPage::OnItemChange(CXTPPropertyGridItem *item)
{
	CGObjGrid::OnItemChange(item);

//	((CGuiPanel_Acl *)GetParent())->SetForceUpdate();
}


void CAssetPage::OnEndItemChange(CXTPPropertyGridItem *item)
{
	CGObjGrid::OnEndItemChange(item);

	_ApplyMod();

	((CGuiPanel_Asset*)(GetParent()))->SetForceBind();

}


void CAssetPage::_ApplyMod()
{
	CGuiPanel_Asset* panel=((CGuiPanel_Asset*)GetParent());
	CGuiView *view=(CGuiView *)panel->FindView("perspective");
	GuiData_AssetMap *data=(GuiData_AssetMap *)panel->FindData("assetmap");
	if (_modmgr)
	{
		IAsset *ast=_amap->FromAddress(_addr);
		assert(ast);
		if (ast)
		{
			CMod_ChangeAssetMap *mod=NULL;
			i_math::pos2di ptBlk;
			_amap->FindAsset(ast,ptBlk);

			//do the backup
			if (_modmgr)
			{
				mod=new CMod_ChangeAssetMap(data);
				mod->BackupBlock(ptBlk);
				mod->BackupSelection();
			}

			//do the actual modifying
			_amap->SetBlockModified(ptBlk);
			_amap->SaveModified();

			//update the block on the map
			_amap->Reload(&ptBlk,1);


			//add the mods
			if (_modmgr)
			{
				_modmgr->NewModGroup();
				_modmgr->PushBack(mod,FALSE);

				_modmgr->PushBack(new CMod_InvalidateView(view),TRUE);
				_modmgr->PushBack(new CMod_ForceBind(panel),FALSE);
			}
		}

	}

}





//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Asset


BEGIN_MESSAGE_MAP(CGuiPanel_Asset, CGuiPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(IDC_RESIDE,OnReside)
END_MESSAGE_MAP()


CGuiPanel_Asset::CGuiPanel_Asset(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_ASSET, pParent)
{
	_bResiding=FALSE;
	_bForceBind=FALSE;
}

BOOL CGuiPanel_Asset::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_ASSET,pParent);	
}


BOOL CGuiPanel_Asset::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	rc.SetRect(0,0,1,1);
	_tree.Create(this,rc,1);

	_page.Create(rc,this,2);

	_RecalcLayout();

	_resider.AddRef();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGuiPanel_Asset::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;
	GetClientRect((LPRECT)&rc);

	if (TRUE)
	{
		i_math::recti rc2;
		rc.cutout(1,300,rc2);

		rc2.inflate(-1,-1,-1,-1);

		SetWindowPos(&_tree,rc2);
	}

	if (TRUE)
	{
		i_math::recti rcCtrl;
		rc.cutout(1,32,rcCtrl);
		rcCtrl.inflate(-4,-2,-4,-2);
		SetWindowPos(GetDlgItem(IDC_RESIDE),rcCtrl);
	}

	if (TRUE)
	{
		i_math::recti rcCtrl;
		rc.cutout(1,320,rcCtrl);
		rcCtrl.inflate(-1,-1,-1,-1);
		SetWindowPos(&_page,rcCtrl);
	}


}


void CGuiPanel_Asset::OnDestroy()
{
	_tree.SetNodeTree(NULL);
	_page.Reset();

	CGuiPanel::OnDestroy();



	// TODO: Add your message handler code here
}

void CGuiPanel_Asset::Reset()
{
	EnableWindow(FALSE);
	GuiData_System *dataSys=(GuiData_System*)FindData("system");
	if (!dataSys)
		return;
	GuiData_AssetMap*dataAmap=(GuiData_AssetMap*)FindData("assetmap");
	if (!dataAmap)
		return;
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	if (!dataCam)
		return;


	_tree.SetNodeTree(dataSys->pAS->GetClassLib()->GetNodeTree());
	_tree.SetModMgr(_modmgr);
	_tree.EnableEdit(FALSE);

	_page.SetModMgr(_modmgr);
	_page.SetAssetMap(dataAmap->assetmap);


	EnableWindow(TRUE);

	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	if (view)
	{
		view->AttachActor(0,static_cast<CGeActor*>(this));
		view->AddAgent(0,new CGuiAgent_CameraMover<TRUE,0>(dataCam->cams[Camera_Perspective]));
		view->AddAgent(0,new CGuiAgent_CameraRotater<FALSE,0>(dataCam->cams[Camera_Perspective]));
		view->AddAgent(0,new CGuiAgent_DrawSelAsset);
		view->AddAgent(0,new CGuiAgent_SelectAsset,AGENTPRIORITY_STANDARD+10);
		view->AddAgent(0,new CGuiAgent_RemoveAsset,AGENTPRIORITY_STANDARD+8);
	}
}

void CGuiPanel_Asset::OnDetachView(CGeView *view,DWORD iLevel)
{
	if (view->CheckName("perspective")&&(iLevel==1))
		_bResiding=FALSE;
}


void CGuiPanel_Asset::UpdateUI()
{
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	assert(dataSys);
	GuiData_AssetMap*dataAssetMap=(GuiData_AssetMap*)FindData("assetmap");
	assert(dataAssetMap);


	CHECK_BUTTON(this,IDC_RESIDE,_bResiding);

	if (_bResiding)
	{
		IAssetClassLib *classlib=dataSys->pAS->GetClassLib();
		CNodeTree *ntree=classlib->GetNodeTree();
		NodeHandle hSel=_tree.GetCurSel();
		AssetClassID clssid=AssetClassID_Null;
		if(hSel)
		{
			std::string path=ntree->GetPath(hSel);
			clssid=classlib->FindClass(path.c_str());
		}

		_resider.SetClassID(clssid);
	}

	//update the page
	if (TRUE)
	{
		if (dataAssetMap->selections.size()!=1)
			_page.Bind(0,TRUE);
		else
			_page.Bind(dataAssetMap->selections[0],_bForceBind);

		_bForceBind=FALSE;
	}

}
void CGuiPanel_Asset::OnSize(UINT nType, int cx, int cy)
{
	CGuiPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}


void CGuiPanel_Asset::OnReside()
{
	CGuiView *view=(CGuiView *)FindView("perspective");

	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");

	if (!_bResiding)
	{
		view->AttachActor(1,static_cast<CGeActor*>(this));
		view->AddAgent(1,&_resider);
		view->AddAgent(1,new CGuiAgent_CameraRotater<FALSE,0>(dataCam->cams[Camera_Perspective]));
		view->AddAgent(1,new CGuiAgent_DrawSelAsset);
		_bResiding=TRUE;
	}
}

//called when asset class lib is modified
BOOL CGuiPanel_Asset::Prop_SetAclMod(Prop_Void &prop)
{
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	assert(dataSys);
	GuiData_AssetMap*dataAssetMap=(GuiData_AssetMap*)FindData("assetmap");
	assert(dataAssetMap);

	CGuiView *view=(CGuiView *)FindView("perspective");
	view->DetachActor(1,static_cast<CGeActor*>(this));

	dataSys->pAS->ReloadClassLib();
	if (TRUE)
	{
		DWORD c;
		AssetClassID *clsses=dataSys->pAS->GetReloadRemoved(c);
		for (int i=0;i<c;i++)
			_acsRemoved.insert(clsses[i]);
	}
	if (TRUE)
	{
		BOOL bNeedReload=FALSE;
		std::set<AssetClassID>::iterator it=_acsRemoved.begin();
		while(it!=_acsRemoved.end())
		{
			std::set<AssetClassID>::iterator itThis=it;
			it++;
			if (dataSys->pAS->GetClassLib()->ObtainClass(*itThis))
			{//a removed asset class appears again,we need re-load the entire asset map
				bNeedReload=TRUE;				
				_acsRemoved.erase(itThis);
			}
		}

		if (bNeedReload)
			dataSys->pAS->GetMap()->ReloadAll();
	}

	//update the tree
	_tree.SetNodeTree(dataSys->pAS->GetClassLib()->GetNodeTree());

	SetForceBind();//force to update the page

	view->Invalidate();

	return TRUE;
}
