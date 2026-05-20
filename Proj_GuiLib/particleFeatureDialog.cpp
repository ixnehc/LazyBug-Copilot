

#include "stdh.h"
#include "particleFeatureDialog.h"
#include "WndBase.h"
#include ".\particlefeaturedialog.h"




CParticleFeatureDialog::CParticleFeatureDialog( CWnd* pParent /* = NULL  */ )
	:CDialog(CParticleFeatureDialog::IDD, pParent ),
	_bCheck( false )
{
	_eDataType = Data_Null;
}

//BOOL CParticleFeatureDialog::OnCreate()
//{
//	int n = 5;
//	CDialog::OnCreate();
//}

BOOL CParticleFeatureDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// 使Reset按钮不能被tab选种
	CWnd *pWnd = GetDlgItem( IDC_BUTTON_RESET_BL );
	DWORD style = pWnd->GetStyle();
	pWnd->ModifyStyle( WS_TABSTOP, 0 );
	
	// 得到edit的位置 隐藏资源里编辑的控件
	RECT rc;
	GET_CONTROL_RECT( this, IDC_PARTICLE_FEATURE_EDIT, rc );
	HIDE_CONTROL( this, IDC_PARTICLE_FEATURE_EDIT );
	
	// 为ComboBox添加选项
	_ComboBox.AddString( "BrokenLine" );
	_ComboBox.SetCurSel( _eDataType );
	_btnCheck.SetCheck( _bCheck );
	_CreateEditor();
	return TRUE;
}


 
void CParticleFeatureDialog::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBO_PARTICLE_FEATURE, _ComboBox );
	DDX_Control( pDX, IDC_CHECK_CONST, _btnCheck );
}

BEGIN_MESSAGE_MAP(CParticleFeatureDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_RESET_BL, OnBnClickedButtonResetBl)
	ON_CBN_SELCHANGE(IDC_COMBO_PARTICLE_FEATURE, OnCbnSelchangeComboParticleFeature)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnOK)
	ON_BN_CLICKED(IDC_BUTTON_CANCLE, OnCancel)
	ON_BN_CLICKED(IDC_CHECK_CONST, OnBnClickedCheckConst)
END_MESSAGE_MAP()



BOOL CParticleFeatureDialog::PreTranslateMessage( MSG* pMsg )
{
	// 不使用esc退出dialog
	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE )    
	{    
		return TRUE;
	}    
	return CDialog::PreTranslateMessage( pMsg );  
}

void CParticleFeatureDialog::OnOK()
{
	if ( _btnCheck.GetCheck() )
	{
		_bCheck = true;
	}
	else
	{
		_bCheck = false;
	}
	_Editor.SetDataConstant( _bCheck );
	SaveData();
	_Editor.Destroy();
	CDialog::OnOK();
}

void CParticleFeatureDialog::OnCancel()
{
	_eDataType = Data_Null;
	_Editor.Destroy();
	CDialog::OnCancel();
}

// 重置
void CParticleFeatureDialog::OnBnClickedButtonResetBl()
{
	// TODO: 在此添加控件通知处理程序代码
	_Editor.ResetContent();
	InvalidateRect( NULL, false );
}


void CParticleFeatureDialog::BindProperty( BrokenLineRef *p,
										 const float fMinX, const float fMaxX, const float fMinY, const float fMaxY )
{
	_Editor.BindProperty( p, fMinX, fMaxX, fMinY, fMaxY );
}

DataType CParticleFeatureDialog::LoadData()
{
	_eDataType = _Editor.LoadData();
	return _eDataType;
}

void CParticleFeatureDialog::SaveData()
{
	_Editor.SaveData();
}

void CParticleFeatureDialog::OnCbnSelchangeComboParticleFeature()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSelect = _ComboBox.GetCurSel();
	ParticleFeatureType type;
	switch ( nSelect )
	{
	case 0:
		{
			type = PARTICLE_BROKENLINE;
			_eDataType = Data_BrokenLine;
		}
		break;
	}
	_Editor.Init();
	_CreateEditor();
	InvalidateRect( NULL, false );
}

void CParticleFeatureDialog::_CreateEditor()
{
	RECT rc;
	GET_CONTROL_RECT( this, IDC_PARTICLE_FEATURE_EDIT, rc );
	switch ( _eDataType )
	{
	case Data_BrokenLine:
		{	
			_Editor.Create( rc, this );
		}
		break;
	}
}

void CParticleFeatureDialog::OnBnClickedCheckConst()
{
	// TODO: 在此添加控件通知处理程序代码
}
