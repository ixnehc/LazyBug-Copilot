
#include "stdh.h"
#include "TransformInputDlg.h"
#include "resource.h"
#include "matrixedit_base.h"

#define MIN_SCALE 0.05f
#define MAX_SCALE 20.0f

CTransformInputDlg * GetTransformDlg()
{
	static CTransformInputDlg gTransformDlg;
	return &gTransformDlg;
}

CTransformInputDlg::CTransformInputDlg(CWnd * pParent/* = NULL*/)
:CXTPDialog(IDD_DIALOG_TRANSFORM,pParent)
{
	_typeEdit = TYPE_Move;
	_changeType = Change_Begin;
//	_bNeedShow = FALSE;
//	_bClosed = FALSE;
}
CTransformInputDlg::~CTransformInputDlg()
{
}
BEGIN_MESSAGE_MAP(CTransformInputDlg,CXTPDialog)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_ABX,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_ABY,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_ABZ,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_ABXYZ,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_OFFX,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_OFFY,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_OFFZ,BeginChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_TRANS_OFFXYZ,BeginChange)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_ABX,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_ABY,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_ABZ,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_ABXYZ,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_OFFX,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_OFFY,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_OFFZ,OnChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_TRANS_OFFXYZ,OnChange)

	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_ABX,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_ABY,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_ABZ,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_ABXYZ,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_OFFX,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_OFFY,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_OFFZ,EndChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_TRANS_OFFXYZ,EndChange)

	ON_BN_CLICKED(IDC_BUTTON_RESETXF,OnReset)
END_MESSAGE_MAP()

void CTransformInputDlg::BeginChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	NMHDR_PB * pStruct = (NMHDR_PB *)pNotifyStruct;
	_changeType = Change_Begin;
	_DoChange(pStruct->idFrom,pStruct->pinboard);
	*pResult = S_OK;
}

void CTransformInputDlg::OnChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{	
	NMHDR_PB * pStruct = (NMHDR_PB *)pNotifyStruct;
	_changeType = Change_On;
	_DoChange(pStruct->idFrom,pStruct->pinboard);
	*pResult = S_OK;
}
void CTransformInputDlg::EndChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	NMHDR_PB * pStruct = (NMHDR_PB *)pNotifyStruct;
	_changeType = Change_End;
	_DoChange(pStruct->idFrom,pStruct->pinboard);
	*pResult = S_OK;
}

float CTransformInputDlg::_ToDegree(float v)
{
	//将弧度转换到0－2pi之间
	while(v<0)
		v += 2*M_PI_F;

	while(v>2*M_PI_F)
		v -= 2*M_PI_F;

	v = v*180.0f/M_PI_F;

	return v;
}
void CTransformInputDlg::OnReset()
{
	if(_typeEdit==TYPE_Rotate&&!_funMatrixChange.empty()){
		i_math::matrix43f matLocal;
		_funMatrixChange(FALSE,Change_Begin,i_math::vector3df(0,0,0),matLocal);
		_funMatrixChange(FALSE,Change_On,i_math::vector3df(0,0,0),matLocal);
		_funMatrixChange(FALSE,Change_End,i_math::vector3df(0,0,0),matLocal);
		_matWorld = matLocal*_matParent;
	
		_HandleBind(); //更新界面
	}
}
void CTransformInputDlg::_HandleBind()
{
	i_math::vector3df vec;
	std::string caption;
		
	switch(_typeEdit){
		case TYPE_Move:
			{
				vec = _matWorld.getTranslation();
				_editAbXYZ.Enable(FALSE);
				_editOffXYZ.Enable(FALSE);
				caption = "  Move Transform Type-In";
				break;
			}
		case TYPE_Rotate:
			{
				_matWorld.getRotationXYZ(&vec);
				
				if(!_matParent.equalsIdentity())
				{
					_editAbX.Enable(FALSE);
					_editAbY.Enable(FALSE);
					_editAbZ.Enable(FALSE);

					_editOffX.Enable(FALSE);
					_editOffY.Enable(FALSE);
					_editOffZ.Enable(FALSE);
				}

				_editAbXYZ.Enable(FALSE);
				_editOffXYZ.Enable(FALSE);

				CWnd * pWnd = GetDlgItem(IDC_BUTTON_RESETXF);
				if(pWnd)
					pWnd->ShowWindow(SW_SHOW);

				caption = "  Rotate Transform Type-In";
				break;
			}
		case TYPE_Scale:
			{
				vec = _matWorld.getScale();		
				caption = "  Scale Transform Type-In";
				break;
			}
		default:
			break;
	}
	
	//弧度转换为角度
	if(_typeEdit==TYPE_Rotate){
		vec.x = _ToDegree(vec.x);
		vec.y = _ToDegree(vec.y);
		vec.z = _ToDegree(vec.z);
	}

	_editAbX.SetValue(vec.x);
	_editAbY.SetValue(vec.y);
	_editAbZ.SetValue(vec.z);

	_vecInit.x = _editAbX.GetFVal();
	_vecInit.y = _editAbY.GetFVal();
	_vecInit.z = _editAbZ.GetFVal();

	//只有在放缩的时候使用1.0f;
	float defaultValue = 0.0f;
	if(_typeEdit==TYPE_Scale){
		defaultValue = 1.0f;
		_editAbXYZ.SetValue(vec.x);
		_editOffXYZ.SetValue(1.0f);
	}

	_editOffX.SetValue(defaultValue);
	_editOffY.SetValue(defaultValue);
	_editOffZ.SetValue(defaultValue);

	SetWindowText(fromMBCS(caption.c_str()));
}

void CTransformInputDlg::Create(CWnd * pParent)
{
	if(!GetSafeHwnd())
		CXTPDialog::Create(IDD_DIALOG_TRANSFORM,pParent);	
}

void CTransformInputDlg::_DoChange(DWORD_PTR ctrID,CPinboard * pinboard)
{
	CPinboard * pEdit = pinboard;
	
	switch(ctrID)
	{
	case IDC_EDIT_TRANS_ABXYZ:
		{
			float v = pEdit->GetFVal();
			_editAbX.SetValue(v);
			_editAbY.SetValue(v);
			_editAbZ.SetValue(v);
			break;
		}
	case IDC_EDIT_TRANS_OFFXYZ:
		{
			float v = pEdit->GetFVal();
			_editOffX.SetValue(v);
			_editOffY.SetValue(v);
			_editOffZ.SetValue(v);
			break;
		}
	default :break;
	}

	i_math::vector3df vecChange;
	BOOL bRalative = TRUE;

	switch(_typeEdit){
		case TYPE_Move:
			_OnMatrixMoveChange((DWORD)ctrID,vecChange);
			break;
		case TYPE_Rotate:
			bRalative = _OnMatrixRoateChange((DWORD)ctrID,vecChange);
			break;
		case TYPE_Scale:
			_OnMatrixScaleChange((DWORD)ctrID,vecChange);
			break;
		default:
			break;
	}
	
	//通知MatrixEditor改变发生了并更新修改后的矩阵
	if(!_funMatrixChange.empty()){
		i_math::matrix43f matLocal;
		_funMatrixChange(bRalative,_changeType,vecChange,matLocal);
		_bLock = TRUE;
		if(_changeType!=Change_Begin){
			_matWorld = matLocal*_matParent;
			_HandleBind(); //更新界面
		}
		
		i_math::vector3df vec;
		_matWorld.getRotationXYZ(&vec);
		_bLock = FALSE;
	}
}
void CTransformInputDlg::_OnMatrixMoveChange(DWORD idCmd,i_math::vector3df &vecChange)
{
	vecChange = _GetChange(idCmd,_vecInit);
}
BOOL CTransformInputDlg::_OnMatrixRoateChange(DWORD idCmd,i_math::vector3df &vecChange)
{
	BOOL bRelative = TRUE;
	vecChange.set(0,0,0);
	float v = 0;

	switch(idCmd)
	{
		case IDC_EDIT_TRANS_ABX:
		case IDC_EDIT_TRANS_ABY:
		case IDC_EDIT_TRANS_ABZ:
			{
				v = _editAbX.GetFVal();
				vecChange.x = _AddChange(v,0);

				v = _editAbY.GetFVal();
				vecChange.y = _AddChange(v,0);

				v = _editAbZ.GetFVal();
				vecChange.z = _AddChange(v,0);
				
				bRelative = FALSE;
				break;
			}
		default:
			{
				v = _editOffX.GetFVal();
				vecChange.x = _AddChange(v,0);
				
				v = _editOffY.GetFVal();
				vecChange.y = _AddChange(v,0);

				v = _editOffZ.GetFVal();
				vecChange.z = _AddChange(v,0);
			
				bRelative = TRUE;
				break;
			}
	}

	vecChange.x = vecChange.x*M_PI_F/180.0f;
	vecChange.y = vecChange.y*M_PI_F/180.0f;
	vecChange.z = vecChange.z*M_PI_F/180.0f;

	return bRelative;
}
void CTransformInputDlg::_OnMatrixScaleChange(DWORD idCmd,i_math::vector3df &vecChange)
{
	//得到某个方向的变化
	vecChange = _GetChange(idCmd,_vecInit);
}

void CTransformInputDlg::_SetDefaultRange()
{

	CPinboardEdit * pEdits[] = {&_editOffX,&_editOffY,&_editOffZ,&_editOffXYZ,
							&_editAbX,&_editAbY,&_editAbZ,&_editAbXYZ};

	CPinSpinner * pSpins[] = { &_spinOffX,&_spinOffY,&_spinOffZ,&_spinOffXYZ,
								&_spinAbX,&_spinAbY,&_spinAbZ,&_spinAbXYZ};

	for(int i = 0;i<8;i++)
	{
		CPinboardEdit * pE = pEdits[i];
		CPinSpinner * pSpin = pSpins[i];
		
		pE->Enable(TRUE);
		switch(_typeEdit){
			case TYPE_Move:
				{
					pE->SetLimits(-500.0f,500.0f);
					break;
				}
			case TYPE_Rotate:
				{
					pE->SetLimits(0,360.0f);
					break;
				}
			case TYPE_Scale:
				{
					pE->SetLimits(MIN_SCALE,MAX_SCALE);
					pE->SetValue(1.0f);
					break;
				}
			default: break;
		}
		pSpin->SetAccel(0.1f,false);
	}

	
	CWnd * pWnd = GetDlgItem(IDC_BUTTON_RESETXF);
	if(pWnd)
		pWnd->ShowWindow(SW_HIDE);
	
}

float CTransformInputDlg::_GetSafeChange(float v,float oldValue)
{
	float vChange = 0;
	
	switch(_typeEdit){
		case TYPE_Scale:
			{
				if(oldValue<MIN_SCALE)
					oldValue = MIN_SCALE;
				vChange = v/oldValue;
				break;
			}
		case TYPE_Move:
			{
				vChange = v - oldValue;
				break;
			}
		case TYPE_Rotate:
			{		
				vChange = v - oldValue;
				break;
			}
		default: break;
	}
	
	return vChange;
}
float CTransformInputDlg::_AddChange(float v,float oldChange)
{
	float vChange = 0;
	
	switch(_typeEdit){
		case TYPE_Scale:
			{
				vChange = v*oldChange;
				if(vChange<MIN_SCALE)
					vChange = MIN_SCALE;
				break;
			}
		case TYPE_Move:
			{
				vChange = v +oldChange;
				break;
			}
		case TYPE_Rotate:
			{		
				vChange = v + oldChange;
				break;
			}
		default: break;
	}
	
	return vChange;
}
i_math::vector3df CTransformInputDlg::_GetChange(DWORD idCmd,const i_math::vector3df &oldVec)
{
	float defaultValue = (_typeEdit==TYPE_Scale)?1.0f:0.0f;

	i_math::vector3df vecChange(defaultValue,defaultValue,defaultValue);	

	float v = 0;
	
	//收集绝对变化
	if(TRUE){
		v = _editAbX.GetFVal();
		vecChange.x = _GetSafeChange(v,oldVec.x);

		v = _editAbY.GetFVal();
		vecChange.y = _GetSafeChange(v,oldVec.y);

		v = _editAbZ.GetFVal();
		vecChange.z = _GetSafeChange(v,oldVec.z);
	}
				
	//收集相对变化
	if(TRUE){
		v = _editOffX.GetFVal();
		vecChange.x = _AddChange(v,vecChange.x);

		v = _editOffY.GetFVal();
		vecChange.y = _AddChange(v,vecChange.y);

		v = _editOffZ.GetFVal();
		vecChange.z = _AddChange(v,vecChange.z);
	}

	return vecChange;
}
void CTransformInputDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_EDIT_TRANS_ABX,_editAbX);
	DDX_Control(pDX,IDC_EDIT_TRANS_ABY,_editAbY);
	DDX_Control(pDX,IDC_EDIT_TRANS_ABZ,_editAbZ);
	DDX_Control(pDX,IDC_EDIT_TRANS_ABXYZ,_editAbXYZ);
	
	DDX_Control(pDX,IDC_EDIT_TRANS_OFFX,_editOffX);
	DDX_Control(pDX,IDC_EDIT_TRANS_OFFY,_editOffY);
	DDX_Control(pDX,IDC_EDIT_TRANS_OFFZ,_editOffZ);
	DDX_Control(pDX,IDC_EDIT_TRANS_OFFXYZ,_editOffXYZ);

	DDX_Control(pDX,IDC_SPIN_TRANS_ABX,_spinAbX);
	DDX_Control(pDX,IDC_SPIN_TRANS_ABY,_spinAbY);
	DDX_Control(pDX,IDC_SPIN_TRANS_ABZ,_spinAbZ);
	DDX_Control(pDX,IDC_SPIN_TRANS_ABXYZ,_spinAbXYZ);

	DDX_Control(pDX,IDC_SPIN_TRANS_OFFX,_spinOffX);
	DDX_Control(pDX,IDC_SPIN_TRANS_OFFY,_spinOffY);
	DDX_Control(pDX,IDC_SPIN_TRANS_OFFZ,_spinOffZ);
	DDX_Control(pDX,IDC_SPIN_TRANS_OFFXYZ,_spinOffXYZ);

	CXTPDialog::DoDataExchange(pDX);
}
BOOL CTransformInputDlg::OnInitDialog()
{
	if(FALSE==CXTPDialog::OnInitDialog())
		return FALSE;

	_editOffX.SetValue(0.0f);
	_editOffY.SetValue(0.0f);
	_editOffZ.SetValue(0.0f);
	_editOffXYZ.SetValue(0.0f);

	_editAbX.SetValue(0.0f);
	_editAbY.SetValue(0.0f);
	_editAbZ.SetValue(0.0f);
	_editAbXYZ.SetValue(0.0f);

	_spinOffX.LinkTo(&_editOffX);
	_spinOffY.LinkTo(&_editOffY);
	_spinOffZ.LinkTo(&_editOffZ);
	_spinOffXYZ.LinkTo(&_editOffXYZ);
	
	_spinAbX.LinkTo(&_editAbX);
	_spinAbY.LinkTo(&_editAbY);
	_spinAbZ.LinkTo(&_editAbZ);
	_spinAbXYZ.LinkTo(&_editAbXYZ);

	_SetDefaultRange();
	_HandleBind();

	return TRUE;
}








