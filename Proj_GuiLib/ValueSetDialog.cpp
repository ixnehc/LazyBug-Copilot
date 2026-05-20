
#include "stdh.h"
#include "ValueSetDialog.h"
#include "WndBase.h"

#include "TreeCtrlBase.h"

#include "stringparser/stringparser.h"

#include "RichGridValueSetItem.h"

#include "Registry/Registry.h"

//////////////////////////////////////////////////////////////////////////
//CVSColorPage

extern CCurrentUserRegistry g_reg;


BEGIN_MESSAGE_MAP(CVSColorPage, CColorAlphaPage)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CVSColorPage::CVSColorPage(COLORREF clrNew, COLORREF clrCurrent, float alpha,DWORD dwFlags, CWnd* pWndParent)
			:CColorAlphaPage(clrNew,clrCurrent,alpha,dwFlags,pWndParent)
{

}


void CVSColorPage::OnDestroy()
{
	CRect rc;
	GetWindowRect(&rc);

	g_reg.WriteInt(_owner.c_str(),"VSDlg_CP_X",rc.left);
	g_reg.WriteInt(_owner.c_str(),"VSDlg_CP_Y",rc.top);

	CColorAlphaPage::OnDestroy();
}



//////////////////////////////////////////////////////////////////////////
//CValueSetDialog
CValueSetDialog::CValueSetDialog( CWnd* pParent /* = NULL  */ )
	:CXTPDialog( CValueSetDialog::IDD, pParent ),_colpage(0xffffffff,0xffffffff,1.0f)
{
	_bEditModeLast=TRUE;
	_bVisible=FALSE	;
}



void CValueSetDialog::Create(CWnd *parent)
{
	CXTPDialog::Create(CValueSetDialog::IDD,parent);
}


RichGridHook *CValueSetDialog::GetRGHook()
{
	return static_cast<RichGridHook*>(this);
}

BOOL CValueSetDialog::OnInitDialog()
{
	CXTPDialog::OnInitDialog();
	
	HIDE_CONTROL( this, IDC_EDIT );
	HIDE_CONTROL( this, IDC_TREE );

	//创建窗口
	CRect rc(0,0,1,1);
	_editor.Create(rc,this,IDC_EDIT);

	_colpage.Create(this,WS_OVERLAPPED,WS_EX_TOOLWINDOW);


	_colpage.ShowWindow(SW_HIDE);
	CColorAlphaPage::NotifyHandler dlgt;
	dlgt.bind(this,&CValueSetDialog::_NotifyColorChange);
	_colpage.SetNotifyHandler(dlgt);


	_data._colpage=&_colpage;

	//GuiEditor
	_mgr.AddModMgr( "ValueSet" );
	_mgr.RegisterData( &_data);
	_mgr.RegisterView( &_view);
	_mgr.RegisterActor( &_actor);
	_actor.Reset();

	_editor.SetView(&_view);

	_idTimer=(UINT)SetTimer(1,10,NULL);

	return TRUE;
}



 
void CValueSetDialog::DoDataExchange( CDataExchange* pDX )
{
	CXTPDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CValueSetDialog, CXTPDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



void CValueSetDialog::_RecalcLayout()
{
	i_math::recti rc;
	GetClientRect((CRect*)&rc);

	rc.inflate(-2,-2,-2,-2);

	::SetWindowPos(&_editor,rc);
}


void CValueSetDialog::OnSize(UINT nType, int cx, int cy)
{
	CXTPDialog::OnSize(nType,cx,cy);

	_RecalcLayout();
}


void CValueSetDialog::OnDestroy()
{
	KillTimer(_idTimer);
	_data.Clear();
	_mgr.Reset();
}

void CValueSetDialog::_UpdateSel()
{
	if (_data.UpdateSelGrp())
		_view.Invalidate();
	CRichGrid *grid=_data.GetGrid();
	if (!grid)
		return;

	if (TRUE)
	{
		CXTPPropertyGridItem *item=grid->GetSelectedItem();
		if (item)
		{
			std::string s;
			s=grid->PathFromItem(item);
			if (s!=_data._selentry)
			{
				_data._selentry=s;
				_view.Invalidate();
			}
		}
		else
		{
			_data._selentry="";
			_view.Invalidate();
		}
	}

	//检查是不是要Fit某个item
	if (TRUE)
	{
		ValueSetGroup *grp=_data.GetSelGroup();
		if (grp)
		{
			std::string path;
			CRichGrid_ValueSetItem*itemToFit=NULL;
			for (int i=0;i<grp->entries.size();i++)
			{
				Ref *ref=grp->entries[i].ref;
				CRichGrid_ValueSetItem*item=(CRichGrid_ValueSetItem*)ref->GetStuff();
				if (!item)
					continue;
				if (item->NeedFit())
				{
					path=grid->PathFromItem(item);
					itemToFit=item;
				}
			}
			if (path!="")
			{
				if (_dlgtShowMe)
					_dlgtShowMe(TRUE);
				if (_actor.Fit(path.c_str()))
				{
					itemToFit->ClearNeedFit();
					_editor.SetFocus();
				}
			}
		}
	}

}

void CValueSetDialog::SetOwnerName(const char *name)
{

	i_math::pos2di pos;
	pos.x=g_reg.ReadInt(_colpage.GetOwnerName(),"VSDlg_CP_X",100);
	pos.y=g_reg.ReadInt(_colpage.GetOwnerName(),"VSDlg_CP_Y",100);

	_colpage.SetWindowPos(NULL,pos.x,pos.y,0,0,SWP_NOZORDER|SWP_NOSIZE);

}


void CValueSetDialog::UpdateUI()
{
	if (!m_hWnd)
		return;
	_UpdateSel();

	//根据grid的enable与disable状态更新自己的ReadOnly状态
	if (TRUE)
	{
		CRichGrid *grid=_data.GetGrid();
		if (grid)
		{
			BOOL bReadOnly=FALSE;
			if (!grid->IsWindowEnabled())
				bReadOnly=TRUE;
			if (grid->GetReadOnly())
				bReadOnly=TRUE;
			_view.SetReadOnly(bReadOnly);
		}
	}

	BOOL bEditMode=TRUE;
	if (g_ssGuiLib.pES)
		bEditMode=g_ssGuiLib.pES->IsEditMode();

	if (_bEditModeLast!=bEditMode)
	{
		if (bEditMode)
		{//从运行模式切换到编辑模式
			if (_bVisible)
			{
				if (_dlgtShowMe)
					_dlgtShowMe(TRUE);
			}
		}
		else
		{//从编辑模式切换到运行模式
			_bVisible=IsWindowVisible();
			if (_dlgtShowMe)
				_dlgtShowMe(FALSE);
		}
		_bEditModeLast=bEditMode;
	}


	if (TRUE)
	{
		BOOL bNeedColorDlg=FALSE;
		if (bEditMode)
		{
			if (IsWindowVisible())
			{
				ValueSetEntry *entry=_data.GetSelEntry();
				if (entry)
				{
					ValueSet *vs=entry->GetValueSet(NULL);
					if(vs->GetKeyType()==KT_Color)
					{
						if (_data.GetSelKey()!=-1)
							bNeedColorDlg=TRUE;
					}
				}
			}
		}
		if (_colpage.GetSafeHwnd())
		{
			if (bNeedColorDlg)
				_colpage.ShowWindow(SW_SHOW);
			else
				_colpage.ShowWindow(SW_HIDE);
		}
	}

	_mgr.RedrawView();
}

void CValueSetDialog::_NotifyColorChange(int type)
{
	CRichGrid *grid=_data.GetGrid();
	if (!grid)
		return;

	ValueSetEntry *entry=_data.GetSelEntry();
	if (entry)
	{
		ValueSet *vs=entry->GetValueSet(NULL);
		if (vs->GetKeyType()==KT_Color)
		{
			int iSel=_data.GetSelKey();
			if (iSel!=-1)
			{
				if (type==0)
				{
					Key_col *k=(Key_col*)vs->GetKey(iSel);
					k->color=_colpage.GetCurColor();
					grid->OnItemChange(NULL);
					_view.Invalidate();
				}
				if (type==1)
					grid->OnBeginItemChange(NULL);
				if (type==2)
				{
					grid->OnEndItemChange(NULL);
					_view.Invalidate();
				}
			}
		}
	}

}


void CValueSetDialog::PostInsertItem(CRichGrid *grid,CXTPPropertyGridItem *item,const char *clss)
{
	CString s;
	grid->GetWindowText(s);
	if (s=="")
		return;

	_data.SetGrpGrid(toMBCS((LPCTSTR)s), grid);

	_data.AddEntry(toMBCS((LPCTSTR)s),grid,item,clss);
	_view.Invalidate();

}

void CValueSetDialog::PostResetContent(CRichGrid *grid)
{
	CString s;
	grid->GetWindowText(s);
	if (s=="")
		return;

	_data.SetGrpGrid(toMBCS((LPCTSTR)s),grid);

	_data.ClearGroup(toMBCS((LPCTSTR)s));

	_view.Invalidate();
}

BOOL CValueSetDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
		return FALSE;
	return CXTPDialog::PreTranslateMessage(pMsg);
}

