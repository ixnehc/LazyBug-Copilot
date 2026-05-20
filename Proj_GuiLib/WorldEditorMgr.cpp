/********************************************************************
	created:	2007/2/14   15:03
	filename: 	e:\IxEngine\Proj_GuiLib\WorldEditorMgr.cpp
	author:		cxi
	
	purpose:	an EditorMgr for world editor
*********************************************************************/
#include "stdh.h"

#include "TreeCtrlBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "WorldEditorMgr.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldEditorDefines.h"


//////////////////////////////////////////////////////////////////////////
//CWEA_CameraRotater

void CWEA_CameraRotater::OnBeginDrag(int x,int y)
{
	((CWEditor_Base*)GetEditor())->GetCameraController()->DragBegin(x,y);
}

void CWEA_CameraRotater::OnEndDrag(int x,int y)	
{
}

void CWEA_CameraRotater::OnDrag(int x,int y)	
{
	((CWEditor_Base*)GetEditor())->GetCameraController()->DragRotate(x,y);
	((CWEditor_Base*)GetEditor())->GetCameraController()->UpdateCamera(GetRP()->QueryCamera());
//	_Redraw();
}

BOOL CWEA_CameraRotater::OnMouseWheel(int delta,DWORD flag)
{
	((CWEditor_Base*)GetEditor())->GetCameraController()->ZoomIn(-delta);
	((CWEditor_Base*)GetEditor())->GetCameraController()->UpdateCamera(GetRP()->QueryCamera());
//	_Redraw();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//

void CWEA_CameraMover::OnBeginDrag(int x,int y)
{
	((CWEditor_Base*)GetEditor())->GetCameraController()->DragBegin(x,y);
}
void CWEA_CameraMover::OnDrag(int x,int y)
{
	((CWEditor_Base*)GetEditor())->GetCameraController()->DragMove(x,y);
	((CWEditor_Base*)GetEditor())->GetCameraController()->UpdateCamera(GetRP()->QueryCamera());
//	_Redraw();
}

BOOL CWEA_CameraMover::OnKeyDown(char c,DWORD flag)
{
	if (!_bInDrag)
	{
		CCameraController *ctrl=((CWEditor_Base*)GetEditor())->GetCameraController();
		switch(c)
		{
			case 37:
				ctrl->ShiftHor(-5);
				break;
			case 39:
				ctrl->ShiftHor(5);
				break;
			case 38:
				ctrl->Forward(500);
				break;
			case 40:
				ctrl->Forward(-500);
				break;
			default:
				return TRUE;
		}

		ctrl->UpdateCamera(GetRP()->QueryCamera());

//		_Redraw();
	}

	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CWEAgent_BaseMenu
BOOL CWEAgent_BaseMenu::OnRButtonClick(int x,int y,DWORD flag)
{
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CWEditor_Base
void CWEditor_Base::OnInitAgent()
{
	DefineEditorAgent(CWEA_CameraRotater,"CameraCtrl","",0);
	DefineEditorAgent(CWEA_CameraMover,"CameraCtrl","",0);
	DefineEditorAgent(CWEAgent_BaseMenu,"BaseMenu","",0);

	EnableAgentGroup("",TRUE);
}

void CWEditor_Base::OnEnable()
{
	ICamera *cam=GetRP()->GetCamera();
	_camctrl.SyncFromCamera(cam);
}



//////////////////////////////////////////////////////////////////////////
//CWEditorMgr
void CWEditorMgr::SetEnv(WEditorEnv &env)
{
	CEditorMgr::SetEnv((EditorEnv&)env);
}
