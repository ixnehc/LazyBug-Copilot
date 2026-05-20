
#include "stdh.h"

#include "resource.h"

#include "TransformSettingsDlg.h"

#include "matrixedit_base.h"

TransformSettings & GetTransformSettings()
{
	static TransformSettings sTransformSettings;
	return sTransformSettings;
} 

CTransformSettingsDlg * GetTranformSettingsDlg()
{
	static CTransformSettingsDlg s_SettingDlg;
	
	CWnd * pWnd = AfxGetMainWnd();
	if(!s_SettingDlg.GetSafeHwnd()&&pWnd)
		s_SettingDlg.Create(IDD_DIALOG_TRANSSETTING,pWnd);

	return &s_SettingDlg;
}
float TransformSettings::_SnapValue(float v,float grid)
{
	//如果grid太小将失去snap作用
	if(grid<=0.005f)
		return v;

	float snapValue = v;
	float leftGrid = 0;

	leftGrid = (int(v/grid))*grid;

	if(v>0){
		float r = v - leftGrid;
		snapValue = r>(grid/2.0f)? (leftGrid+grid):leftGrid;
	}
	else{
		float r = leftGrid - v;
		snapValue = r>(grid/2.0f)?(leftGrid-grid):leftGrid;
	}

	return snapValue;
}
float TransformSettings::SnapMove(float v)
{
	if(bSnapMove)
		return _SnapValue(v,moveSnap);;
	return v;	
}

float TransformSettings::SnapScale(float v)
{
	if(bSnapScale)
		return  _SnapValue(v,scaleSnap/100.0f);
	return v;
}
float TransformSettings::SnapRotate(float v)
{
	if(bSnapRotate)
		return _SnapValue(v,rotateSnap);
	return v;
}

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTransformSettingsDlg,CXTPDialog)
	ON_EN_CHANGE(IDC_EDIT_SNAPROTATE,OnSettingChange)
	ON_EN_CHANGE(IDC_EDIT_SNAPMOVE,OnSettingChange)
	ON_EN_CHANGE(IDC_EDIT_SNAPSCALE,OnSettingChange)
	ON_BN_CLICKED(IDC_CHECK_SNAPMOVE,OnSettingChange)
	ON_BN_CLICKED(IDC_CHECK_ROTATE,OnSettingChange)
	ON_BN_CLICKED(IDC_CHECK_SCALE,OnSettingChange)
END_MESSAGE_MAP()


CTransformSettingsDlg::CTransformSettingsDlg()
:CXTPDialog(IDD_DIALOG_TRANSSETTING,NULL)
{
}
void CTransformSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	TransformSettings & settingsTransform = GetTransformSettings();

	DDX_Text(pDX,IDC_EDIT_SNAPMOVE,settingsTransform.moveSnap);
	DDX_Text(pDX,IDC_EDIT_SNAPSCALE,settingsTransform.scaleSnap);
	DDX_Text(pDX,IDC_EDIT_SNAPROTATE,settingsTransform.rotateSnap);
	DDX_Check(pDX,IDC_CHECK_SNAPMOVE,settingsTransform.bSnapMove);
	DDX_Check(pDX,IDC_CHECK_ROTATE,settingsTransform.bSnapRotate);
	DDX_Check(pDX,IDC_CHECK_SCALE,settingsTransform.bSnapScale);
}
void CTransformSettingsDlg::OnSettingChange()
{
	UpdateData(TRUE); //更新数据	
}





