
/********************************************************************
	created:	2008/03/31
	created:	31:3:2008   13:22
	filename: 	e:\IxEngine\Proj_GuiLib\Agent_MatrixEdit.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	Agent_MatrixEdit
	file ext:	cpp
	author:		star
	purpose:	control matrix edit
*********************************************************************/

#include "stdh.h"

#include "matrixedit_base.h"

#include "GuiAgent_MatrixEdit.h"

#include "TransformInputDlg.h"

#include "AgentCmdID.h"

#include <assert.h>

#include "TransformSettingsDlg.h"

CGuiAgent_MatrixEdit::CGuiAgent_MatrixEdit(DWORD flag)
{
	_bBind = FALSE;
	_space = EditSpace_World;
	_mode  = EditMode_Select; 
	_bShowSpaceMenu=TRUE;
	_bShowMoveToCamera=TRUE;
	_bShowResetPRS=FALSE;
	_modeSupports = flag;
	_pAgentMove = NULL;
	_pAgentRot = NULL;
	_pAgentScale = NULL;
	_InstanceAgent();

	//创建设置窗口
}
CGuiAgent_MatrixEdit::~CGuiAgent_MatrixEdit(void)
{
}
void CGuiAgent_MatrixEdit::Enable(BOOL bEnable)
{
	CGuiAgent::Enable(bEnable);

	if(_pAgentMove)
		_pAgentMove->Enable(bEnable);
	if(_pAgentScale)
		_pAgentScale->Enable(bEnable);
	if (_pAgentRot)
		_pAgentRot->Enable(bEnable);
}


Matrix_EditMode CGuiAgent_MatrixEdit::GetEditMode()
{
	return _mode;
}
void CGuiAgent_MatrixEdit::_InstanceAgent()
{	
	fastdelegate::FastDelegate1<CMatrixEditBase *,BOOL> e0,e1,e2;
	e0.bind(this,&CGuiAgent_MatrixEdit::PreEditCallBack);
	e1.bind(this,&CGuiAgent_MatrixEdit::OnEditCallBack);
	e2.bind(this,&CGuiAgent_MatrixEdit::EndEditCallBack);

	if(_modeSupports&EditMode_Rot)
	{
		_pAgentRot = new CGuiAgent_MatrixRot;
		_pAgentRot->SetPreEditListener(e0);
		_pAgentRot->SetOnEditListener(e1);
		_pAgentRot->SetEndEditListener(e2);
	}

	if(_modeSupports&EditMode_Move)
	{
		_pAgentMove = new CAgent_MatrixMove;
		_pAgentMove->SetPreEditListener(e0);
		_pAgentMove->SetOnEditListener(e1);
		_pAgentMove->SetEndEditListener(e2);
	}

	if(_modeSupports&EditMode_Scale)
	{
		_pAgentScale = new CAgent_MatrixSimpleScale;
		_pAgentScale->SetPreEditListener(e0);
		_pAgentScale->SetOnEditListener(e1);
		_pAgentScale->SetEndEditListener(e2);
	}
}
void CGuiAgent_MatrixEdit::_AddAgent()
{
	CGuiView *view = GetGuiView();
	assert(view);

	if(_modeSupports&EditMode_Rot)
		view->AddAgent(_iLevelInView,_pAgentRot,_priority);
	
	if(_modeSupports&EditMode_Move)
		view->AddAgent(_iLevelInView,_pAgentMove,_priority);

	if(_modeSupports&EditMode_Scale)
		view->AddAgent(_iLevelInView,_pAgentScale,_priority);
}

void  CGuiAgent_MatrixEdit::SetEventListener(EventEdit e0,EventEdit e1,EventEdit e2)
{
	_funPreEdit = e0;
	_funOnEdit  = e1;
	_funEndEdit = e2;
}

BOOL g_bMatClipboard=FALSE;
i_math::matrix43f g_matClipboard;

BOOL GetMatClipboard(i_math::matrix43f &mat)
{
	if (g_bMatClipboard)
	{
		mat=g_matClipboard;
		return TRUE;
	}
	return FALSE;
}

void SetMatClipboard(i_math::matrix43f &mat)
{
	g_matClipboard=mat;
	g_bMatClipboard=TRUE;
}


BOOL CGuiAgent_MatrixEdit::OnRButtonClick(int x,int y,DWORD flag)
{	
	UINT nFlags = 0;
	_AddMenuSep();

	CGuiView * view = GetGuiView();
	assert(view);

	DWORD flagMenu = (_mode!=EditMode_Select)?MF_CHECKED:MF_UNCHECKED;

	
	if(TRUE){

		if(_pAgentMove&&_pAgentMove->IsWorkable()){
			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_mode==EditMode_Move)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("移动",ID_AGENT_ME_TSMOVE,nFlags);
		}
		
		if (_pAgentRot&&_pAgentRot->IsWorkable()){
			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_mode==EditMode_Rot)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("旋转",ID_AGENT_ME_TSROTATE,nFlags);
		}
		
		if(_pAgentScale&&_pAgentScale->IsWorkable()){
			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_mode==EditMode_Scale)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("缩放",ID_AGENT_ME_TSSCALE,nFlags);
		}

		nFlags = MF_STRING|MF_ENABLED;
		nFlags |=(_mode==EditMode_Select)?MF_CHECKED:MF_UNCHECKED;
		_AddMenu("选择",ID_AGENT_ME_SELECT,nFlags);

		if(_mode!=EditMode_Select){
			_PushMenu("More...");
			nFlags = MF_STRING|MF_ENABLED;
			_AddMenu("Snap Setting",ID_AGENT_ME_SETING,nFlags);
			_AddMenu("Key Input(F2)",ID_AGENT_ME_INPUT,nFlags);
			_PopMenu();
		}
	}

	if(_bShowSpaceMenu)
	{
		_PushMenu("Space");
		{
			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_space==EditSpace_World)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("world",ID_AGENT_ME_WORLD,nFlags);

			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_space==EditSpace_View)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("view",ID_AGENT_ME_VIEW,nFlags);
			
			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_space==EditSpace_Parent)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("parent",ID_AGENT_ME_PARENT,nFlags);

			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_space==EditSpace_Local)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("local",ID_AGENT_ME_LOCAL,nFlags);

			nFlags = MF_STRING|MF_ENABLED;
			nFlags |=(_space==EditSpace_Screen)?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("screen",ID_AGENT_ME_SCREEN,nFlags);
		
		}
		_PopMenu();
	}

	_AddMenuSep();

	if (_bShowMoveToCamera)
		_AddMenu("移至当前camera位置",ID_AGENT_ME_MOVETO_CAMERA);
	if (_bShowResetPRS)
		_AddMenu("归零",ID_AGENT_ME_RESET_PRS);
	_AddMenu("复制位置",ID_AGENT_ME_COPY_MAT);

	if (g_bMatClipboard)
		_AddMenu("粘帖位置",ID_AGENT_ME_PASTE_MAT);


	_AddMenuSep();

	return TRUE;
}

BOOL CGuiAgent_MatrixEdit::PreEditCallBack(CMatrixEditBase * editAgent)
{
	std::string nameClass = editAgent->getClassName();
	BOOL bVaild = FALSE;
	
	if(!_funPreEdit.empty())
		_funPreEdit(_data.matrix);
	
	if(!IsBind())
		return FALSE;

	if(nameClass.compare("CGuiAgent_MatrixRot")==0&&_mode==EditMode_Rot) 
	{	
		_data.modespace = _space;
		_data.modeedit = _mode;
		editAgent->Bind(_data);
		bVaild = TRUE;
	}
	else if(nameClass.compare("CAgent_MatrixMove")==0&&_mode==EditMode_Move)
	{
		_data.modespace = _space;
		_data.modeedit = _mode;
		editAgent->Bind(_data);
		bVaild = TRUE;
	}
	else if(nameClass.compare("CAgent_MatrixScale")==0&&_mode==EditMode_Scale)
	{
		_data.modespace = _space;
		_data.modeedit = _mode;
		editAgent->Bind(_data);
		bVaild = TRUE;
	}

	return bVaild;
}

BOOL CGuiAgent_MatrixEdit::OnEditCallBack(CMatrixEditBase * editAgent)
{
	if(_funOnEdit.empty())
		return FALSE;


	_funOnEdit(_data.matrix);
	 
	 return TRUE;
}

BOOL CGuiAgent_MatrixEdit::EndEditCallBack(CMatrixEditBase * editAgent)
{
	if(_funEndEdit.empty())
		return FALSE;
	
	_funEndEdit(_data.matrix);

	return TRUE;
}
void CGuiAgent_MatrixEdit::OnAttachView(CGeView *view,DWORD iLevel)
{
	_AddAgent();
}
void CGuiAgent_MatrixEdit::SetWorkable(Matrix_EditMode flag,BOOL bWorkable)
{
	if((flag&EditMode_Move)&&_pAgentMove)
		_pAgentMove->SetWorkable(bWorkable);

	if((flag&EditMode_Scale)&&_pAgentScale)
		_pAgentScale->SetWorkable(bWorkable);

	if((flag&EditMode_Rot)&&_pAgentRot)
		_pAgentRot->SetWorkable(bWorkable);
}
BOOL CGuiAgent_MatrixEdit::OnCommand(DWORD idCmd)
{
	BOOL bHandled=TRUE;
	BOOL bNeedRedraw=TRUE;;
	switch(idCmd)
	{
	case ID_AGENT_ME_TSROTATE:
		{
			_mode = EditMode_Rot;
			break;
		}
	case ID_AGENT_ME_TSSCALE:
		{
			_mode = EditMode_Scale;
			break;
		}
	
	case ID_AGENT_ME_SELECT:
		{
			_mode = EditMode_Select;
			break;
		}//////////////////////////////
	case ID_AGENT_ME_TSMOVE:
		{
			_mode = EditMode_Move;
			break;
		}
	case ID_AGENT_ME_WORLD:
		{
			_space = EditSpace_World;
			break;
		}
	case ID_AGENT_ME_VIEW:
		{
			_space = EditSpace_View;
			break;
		}
	case ID_AGENT_ME_PARENT:
		{
			_space = EditSpace_Parent;
			break;
		}
	case ID_AGENT_ME_LOCAL:
		{
			_space = EditSpace_Local;
			break;
		}
	case ID_AGENT_ME_SCREEN:
		{
			_space = EditSpace_Screen;
			break;
		}
	case ID_AGENT_ME_MOVETO_CAMERA:
		{
			IRenderPort *rp=GetRP();
			if (rp)
			{
				i_math::matrix43f mat;
				if (rp->GetCamera()->GetEyeMat(mat))
				{
					if(!_funPreEdit.empty())
						_funPreEdit(_data.matrix);
					*_data.matrix=mat;
					if(!_funOnEdit.empty())
						_funOnEdit(_data.matrix);
					if(!_funEndEdit.empty())
						_funEndEdit(_data.matrix);
				}
			}
			break;
		}
	case ID_AGENT_ME_RESET_PRS:
		{
			if(!_funPreEdit.empty())
				_funPreEdit(_data.matrix);
			(*_data.matrix).makeIdentity();
			if(!_funOnEdit.empty())
				_funOnEdit(_data.matrix);
			if(!_funEndEdit.empty())
				_funEndEdit(_data.matrix);
			break;
		}
	case ID_AGENT_ME_COPY_MAT:
		{
			SetMatClipboard((*_data.matrix));
			break;
		}
	case ID_AGENT_ME_PASTE_MAT:
		{
			i_math::matrix43f mat;
			if (GetMatClipboard(mat))
			{
				if(!_funPreEdit.empty())
					_funPreEdit(_data.matrix);
				(*_data.matrix)=mat;
				if(!_funOnEdit.empty())
					_funOnEdit(_data.matrix);
				if(!_funEndEdit.empty())
					_funEndEdit(_data.matrix);
			}
			break;
		}

	case ID_AGENT_ME_SETING:
		{
			CTransformSettingsDlg dlgSetting;
			dlgSetting.DoModal();
			break;
		}
	case ID_AGENT_ME_INPUT:
		{
			_DoInputDlg();
			break;
		}
	default:
		bNeedRedraw=FALSE;
		bHandled=FALSE;
		break;
	}

	_data.modeedit = _mode;	
	_data.modespace = _space;

	Bind(_data);

	if (bNeedRedraw)
		_Redraw(FALSE);

	if (bHandled)
		return FALSE;

	return TRUE;
}
void CGuiAgent_MatrixEdit::_DoInputDlg()
{
	CTransformInputDlg transDlg;
	
	BOOL bOK = FALSE;
	switch(_mode){
			case EditMode_Move:
			{
				if(_pAgentMove&&_data.matrix){
					transDlg.Bind<CAgent_MatrixMove>(CTransformInputDlg::TYPE_Move,*_data.matrix,
						_data.matParent,_pAgentMove,&CAgent_MatrixMove::OnDlgEdit);
				}
				bOK = TRUE;
				break;
			}
		case EditMode_Rot:
			{
				if(_pAgentRot&&_data.matrix){
					transDlg.Bind<CGuiAgent_MatrixRot>(CTransformInputDlg::TYPE_Rotate,*_data.matrix,
						_data.matParent,_pAgentRot,&CGuiAgent_MatrixRot::OnDlgEdit);
				}
				bOK = TRUE;
				break;
			}
		case EditMode_Scale:
			{
				if(_pAgentScale&&_data.matrix){
					transDlg.Bind<CAgent_MatrixSimpleScale>(CTransformInputDlg::TYPE_Scale,*_data.matrix,
						_data.matParent,_pAgentScale,&CAgent_MatrixSimpleScale::OnDlgEdit);
				}
				bOK = TRUE;
				break;
			}
		default: 
			{	
				break;
			}
	}

	if(bOK)
		transDlg.DoModal();
}
BOOL CGuiAgent_MatrixEdit::OnKeyDown(char c,DWORD flag)
{
	if(c==VK_F2){
		_DoInputDlg();
	}
	return TRUE;
}
BOOL CGuiAgent_MatrixEdit::IsSelected()
{
	switch(_mode)
	{
	case EditMode_Move:
		return _pAgentMove->IsSelected();
	case EditMode_Rot:
		return _pAgentRot->IsSelected();
	case EditMode_Scale:
		return _pAgentScale->IsSelected();
	default:
		break;
	}
	return FALSE;
}
BOOL CGuiAgent_MatrixEdit::Bind(MatrixEditData &data)
{
	BOOL bOk =  CMatrixEditBase::Bind(data);

	data.modeedit = _mode;
	if(_pAgentMove)
		_pAgentMove->Bind(data);
	if(_pAgentScale)
		_pAgentScale->Bind(data);
	if(_pAgentRot)
		_pAgentRot->Bind(data);

	_space = _data.modespace;

	data.modespace = _space;
	
	return	bOk;
}



