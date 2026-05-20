
#include "stdh.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"

#include "MatEditDlg.h"
#include "WndBase.h"
#include "RenderPortBase.h"

#include "RenderSystem/IRenderSystem.h"

#include "GuiAgent_general.h"
#include "GuiAgent_MatrixEdit.h"

class CMod_Mat:public CModBase
{
public:
	CMod_Mat( CGuiActor_Mat*owner)
	{
		_owner=owner;
	}
	virtual BOOL IsEmpty()
	{
		return FALSE;
	}
	virtual BOOL Undo()
	{
		GuiData_Mat *data=(GuiData_Mat *)_owner->FindData("mat");
		Swap(*data->mat,_mat);
		if (_owner->_handler)
		{
			_owner->_handler(MEDlg_BeginMod);
			_owner->_handler(MEDlg_Mod);
			_owner->_handler(MEDlg_EndMod);
		}
		if (_owner->_matedit)
		{
			MatrixEditData med;
			med.matrix=data->mat;
			_owner->_matedit->Bind(med);
		}
		
		_owner->FindView("mat")->Invalidate();
		return TRUE;
	}
	virtual BOOL Redo()
	{
		return Undo();
	}

protected:
	CGuiActor_Mat*_owner;
	i_math::matrix43f _mat;

	friend class CGuiActor_Mat;
};


//////////////////////////////////////////////////////////////////////////
//CGuiView_Mat
CGuiView_Mat::~CGuiView_Mat()
{
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_mtrl);
	SAFE_RELEASE(_lgt);
}

void CGuiView_Mat::_OnDraw( IRenderPort*rp)
{
	GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");
	GuiData_Mat *data=(GuiData_Mat*)FindData("mat");

	rp->SetCamera(dataCam->cams[Camera_Perspective]);

	rp->ClearBuffer(ClearBuffer_All);

	DrawGrid(rp,10,1);

	if (!_mesh)
	{
		_mesh=(IMesh*)GetRS()->GetMeshMgr()->ObtainRes("_editor\\plane.msh");
		_mesh->ForceTouch();
	}

	if (!_mtrl)
	{
		_mtrl=(IMtrl*)GetRS()->GetMtrlMgr()->ObtainRes("_editor\\plane2.mtl");
		_mtrl->ForceTouch();
	}
	if (!_lgt)
		_lgt=GetRS()->CreateLight();
	//根据renderport camera更新_lgt的方向
	if (TRUE)
	{
		LightInfo *li=_lgt->QueryInfo();
		rp->GetCamera()->GetEyeDir(li->dir);
		li->dir.normalize();
	}


	if (data->mat)
		rp->SimpleDrawMesh(_mesh,*data->mat,0xffffffff,FALSE,_mtrl,_lgt);
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_Undo
BOOL CGuiAgent_Undo::OnKeyDown( char c, DWORD flag )
{
	bool bCtrlDown = ( ( flag & CtrlOpFlag_CtrlDown ) != 0 );
	if ( bCtrlDown )
	{
		CModManager *mgr = _GetModMgr();
		switch ( c )
		{
		case 'Y':
			{
				if ( mgr )
				{
					mgr->Redo();
				}
			}
			break;
		case 'Z':
			{
				if ( mgr )
				{
					mgr->Undo();
				}
			}
			break;
		}
	}
	return CGuiAgent::OnKeyDown( c, flag );
}



//////////////////////////////////////////////////////////////////////////
//CGuiActor_Mat
void CGuiActor_Mat::Reset()
{
	CGuiView *view = (CGuiView *)FindView( "mat" );
	GuiData_Mat*data= (GuiData_Mat*)FindData( "mat" );
	GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");

	if ( data&& view )
	{   
		view->AttachActor(0,this);

		view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
		view->AddAgent(0,new CGuiAgent_Undo);

		_matedit=new CGuiAgent_MatrixEdit(EditMode_All);
		_matedit->ShowSpaceMenu(FALSE);
		_matedit->ShowMoveToCamera(FALSE);
		_matedit->ShowResetPRS(TRUE);
		CGuiAgent_MatrixEdit::EventEdit e0,e1,e2;
		e0.bind(this,&CGuiActor_Mat::_BeginEdit);
		e1.bind(this,&CGuiActor_Mat::_Edit);
		e2.bind(this,&CGuiActor_Mat::_EndEdit);
		_matedit->SetEventListener(e0,e1,e2);

		MatrixEditData med;
		med.matrix=data->mat;
		_matedit->Bind(med);
		view->AddAgent(0,_matedit);

	}
}

void CGuiActor_Mat::_BeginEdit(i_math::matrix43f *mat)
{
	_matBack=*mat;
	if (_handler)
		_handler(MEDlg_BeginMod);
}
void CGuiActor_Mat::_Edit(i_math::matrix43f *mat)
{
	if (_handler)
		_handler(MEDlg_Mod);
}
void CGuiActor_Mat::_EndEdit(i_math::matrix43f *mat)
{
	CModManager *modmgr=GetModMgr();
	if (modmgr)
	{
		modmgr->NewModGroup();
		CMod_Mat *mod=new CMod_Mat(this);
		mod->_mat=_matBack;
		modmgr->PushBack(mod,FALSE);
	}
	if (_handler)
		_handler(MEDlg_EndMod);

}



//////////////////////////////////////////////////////////////////////////
//CMatWnd
BOOL CMatWnd::Create( RECT &rc, CWnd *pParentWnd )
{
	if (FALSE == CWnd::Create(NULL, _T("MatWnd"), WS_CHILD | WS_VISIBLE,
		rc, pParentWnd, IDC_WND))
		return FALSE;

	_idTimer=(UINT)SetTimer(1,10,NULL);

	CGuiViewWnd<CWnd>::SetView( &_view);

	_view.SetRS(g_ssGuiLib.pRS);

	_mgr.AddModMgr( "mat" );
	_mgr.RegisterData( &_data);
	_dataCam.cams[Camera_Perspective]=g_ssGuiLib.pRS->CreateCamera();
	_dataCam.cams[Camera_Perspective]->SetPosTarget(i_math::vector3df(-4,4,-4),i_math::vector3df(0,0,0));
	_mgr.RegisterData(&_dataCam);
	_mgr.RegisterView( &_view);
	_mgr.RegisterActor( &_actor);
	_actor.Reset();

	return TRUE;
}

void	CMatWnd::Bind(i_math::matrix43f *mat)
{
	_data.mat=mat;
}

LRESULT CMatWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message )
	{
		case WM_TIMER:
		{
			if ( CGuiViewWnd<CWnd>::_view )
			{
				_mgr.RedrawView();
			}
			break;
		}
		case WM_MOVE:
		{
			if (CGuiViewWnd<CWnd>::_view)
			{
				CGuiViewWnd<CWnd>::_view->Invalidate();
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			CWnd::SetFocus();
			break;
		}
		case WM_DESTROY:
		{
			KillTimer(_idTimer);
			_dataCam.Clear();
			_view.Clear();
			_mgr.Reset();
			break;
		}
	}
	return CGuiViewWnd<CWnd>::WindowProc( message, wParam, lParam );

}



//////////////////////////////////////////////////////////////////////////
//CMatEditDlg

CMatEditDlg::CMatEditDlg( CWnd* pParent /* = NULL  */ )
	:CDialog( CMatEditDlg::IDD, pParent )
{
}

BOOL CMatEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// 得到edit的位置 隐藏资源里编辑的控件
	RECT rc;
	GET_CONTROL_RECT( this, IDC_WND, rc );
	HIDE_CONTROL( this, IDC_WND);
	
	_wnd.Create( rc, this );
	return TRUE;
}


 
void CMatEditDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CMatEditDlg, CDialog)
END_MESSAGE_MAP()


void CMatEditDlg::Bind(i_math::matrix43f *mat)
{
	_wnd.Bind(mat);
}
