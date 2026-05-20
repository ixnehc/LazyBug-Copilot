/********************************************************************
	created:	2008/2/3   12:46
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	the general-usage GuiAgents
*********************************************************************/

#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"
#include "GuiData.h"

#include "AgentCmdID.h"

#include "ArcBall.h"

#include "stringparser/stringparser.h"
#include "Registry/Registry.h"

#include "WorldSystem/IAssetRendererDefines.h"
#include "WorldSystem/IAssetRenderer.h"


class ICamera;


//T_flag是一个CtrlOpFlag_XXXX
template<int T_button,DWORD T_flag>
class CGuiAgent_CameraRotater:public CGuiAgent_Dragger<T_button,T_flag>
{
public:
	CGuiAgent_CameraRotater()
	{
		cam=NULL;
		controller=NULL;
	}
	CGuiAgent_CameraRotater(ICamera *cam_,CCameraController *controller_)
	{
		cam=cam_;
		controller=controller_;
	}
	void BindCamera(ICamera *cam_)
	{
		cam=cam_;
	}
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag)
	{
		if (cam)
		{
			controller->SyncFromCamera(cam);
			controller->DragBegin(x,y);
			return TRUE;
		}
		return FALSE;
	}
	virtual void OnEndDrag(int x,int y,DWORD flag)
	{
	}
	virtual void OnDrag(int x,int y,DWORD flag)
	{
		if (cam)
		{
			controller->DragRotate(x,y);
			controller->UpdateCamera(cam);
			InvalidateView();
		}
	}
	virtual BOOL OnMouseWheel(int delta,DWORD flag)
	{
		if (flag==(CtrlOpFlag_CtrlDown))
		{
			if (g_ssGuiLib.reg)
			{
				int fov=g_ssGuiLib.reg->ReadInt("General","EditorCamFov",75);
				fov-=(int)(((float)delta)*0.03f);
				fov=i_math::clamp_i(fov,10,100);
				g_ssGuiLib.reg->WriteInt("General","EditorCamFov",fov);
				InvalidateView();
			}
			return FALSE;
		}

		if (cam)
		{
			if (!_bInDrag)
				controller->SyncFromCamera(cam);
			controller->ZoomIn(-delta);
			controller->UpdateCamera(cam);
			InvalidateView();
		}
		return TRUE;
	}

	void SetFocusPos(i_math::vector3df &pos)
	{
		if (controller)
			controller->SetFocusPos(pos);
	}
	void ClearFocusPos()
	{
		if (controller)
			controller->ClearFocusPos();
	}

	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag)
	{
		if (controller)
		{
			if (controller->ResetFocus())
			{
				controller->UpdateCamera(cam);
				InvalidateView();
				return FALSE;
			}
		}
		return TRUE;
	}


protected:

	ICamera *cam;
	CCameraController *controller;
};

//T_flag是一个CtrlOpFlag_XXXX
template<int T_button,DWORD T_flag>
class CGuiAgent_CameraMover:public CGuiAgent_Dragger<T_button,T_flag>
{
public:
	CGuiAgent_CameraMover()
	{
		cam=NULL;
	}
	CGuiAgent_CameraMover(ICamera *cam_,CCameraController *controller_)
	{
		cam=cam_;
		controller=controller_;
	}
	void BindCamera(ICamera *cam_)
	{
		cam=cam_;
	}
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag)
	{
		if (cam)
		{
			controller->SyncFromCamera(cam);
			controller->DragBegin(x,y);
			return TRUE;
		}
		return FALSE;
	}
	virtual void OnDrag(int x,int y,DWORD flag)
	{
		if (cam)
		{
			controller->DragMove(x,y);
			controller->UpdateCamera(cam);
			InvalidateView();
		}
	}

	virtual BOOL OnKeyDown(char c,DWORD flag)
	{
		if (cam)
		{
			if (!_bInDrag)
			{
				switch(c)
				{
				case 37:
					controller->ShiftHor(-50);
					break;
				case 39:
					controller->ShiftHor(50);
					break;
				case 38:
					controller->Forward(50);
					break;
				case 40:
					controller->Forward(-50);
					break;
				default:
					return TRUE;
				}

				controller->UpdateCamera(cam);

				InvalidateView();
			}
		}

		return TRUE;

	}

protected:
	ICamera *cam;
	CCameraController *controller;
};

template<int T_movebutton,DWORD T_flagMove,int T_rotatebutton,DWORD T_flagRotate>
class CGuiAgent_CameraController:public CGuiAgent
{
public:
	CGuiAgent_CameraController(ICamera *cam):
		_rotater(cam,&_controller),_mover(cam,&_controller)
	{
		_controller.SetSensitiveRate(1.0f);
		_cam=cam;
	}

	void SetFocusPos(i_math::vector3df &pos)	{		_rotater.SetFocusPos(pos);	}
	void ClearFocusPos()	{		_rotater.ClearFocusPos();	}

	virtual BOOL Respond(CtrlOp &co)
	{
		//trick:assign the view ptr to let the view receive the redraw event from these 2 embedded agents
		_rotater._view=_view;
		_mover._view=_view;

		GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");
		if (dataCam)
			_controller.SetSensitiveRate((float)(1<<dataCam->scaleMove));

		BOOL b1,b2;
		b1=_rotater.Respond(co);
		b2=_mover.Respond(co);
		if ((!b1)||(!b2))
			return FALSE;
		return CGuiAgent::Respond(co);
	}

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag)
	{
		std::string s;
		DWORD scale=0;
		GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");
		if (dataCam)
			scale=dataCam->scaleMove;

		FormatString(s,"Camera移动速度(x%d)",1<<scale);
		_PushMenu(s.c_str());
			if (scale==0)
				_AddMenu("x1",ID_AGENT_CamCtrl_x1,MF_CHECKED|MF_ENABLED|MF_STRING);
			else
				_AddMenu("x1",ID_AGENT_CamCtrl_x1,MF_ENABLED|MF_STRING);
			if (scale==1)
				_AddMenu("x2",ID_AGENT_CamCtrl_x2,MF_CHECKED|MF_ENABLED|MF_STRING);
			else
				_AddMenu("x2",ID_AGENT_CamCtrl_x2,MF_ENABLED|MF_STRING);
			if (scale==2)
				_AddMenu("x4",ID_AGENT_CamCtrl_x4,MF_CHECKED|MF_ENABLED|MF_STRING);
			else
				_AddMenu("x4",ID_AGENT_CamCtrl_x4,MF_ENABLED|MF_STRING);
			if (scale==3)
				_AddMenu("x8",ID_AGENT_CamCtrl_x8,MF_CHECKED|MF_ENABLED|MF_STRING);
			else
				_AddMenu("x8",ID_AGENT_CamCtrl_x8,MF_ENABLED|MF_STRING);
		_PopMenu();

		return TRUE;
	}

	virtual BOOL OnCommand(DWORD idCmd)
	{
		GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");
		if (!dataCam)
			return TRUE;
		switch(idCmd)
		{
			case ID_AGENT_CamCtrl_x1:
			{
				dataCam->scaleMove=0;
				_controller.SetSensitiveRate(1.0f);
				return FALSE;
			}
			case ID_AGENT_CamCtrl_x2:
			{
				dataCam->scaleMove=1;
				_controller.SetSensitiveRate(2.0f);
				return FALSE;
			}
			case ID_AGENT_CamCtrl_x4:
			{
				dataCam->scaleMove=2;
				_controller.SetSensitiveRate(4.0f);
				return FALSE;
			}
			case ID_AGENT_CamCtrl_x8:
			{
				dataCam->scaleMove=3;
				_controller.SetSensitiveRate(8.0f);
				return FALSE;
			}
		}
		return TRUE;
	}

protected:
	ICamera *_cam;
	CCameraController _controller;
	CGuiAgent_CameraRotater<T_rotatebutton,T_flagRotate>_rotater;
	CGuiAgent_CameraMover<T_movebutton,T_flagMove>_mover;

};

class CGuiAgent_ViewSwitcher:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

	virtual BOOL OnKeyDown(char c,DWORD flag);

};


class CGuiAgent_BakeLocal:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	i_math::pos2di _ptBlk;
};

class CGuiAgent_PlayHere:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	i_math::vector3df _pos;
};

class CGuiAgent_CameraFov:public CGuiAgent
{
public:
	virtual BOOL OnMouseWheel(int delta,DWORD flag);

};

class IAnimNodeMatFixed;
class CGuiAgent_CameraLock:public CGuiAgent_Dragger<DRAG_BUTTON_RIGHT,CtrlOpFlag_CtrlDown|CtrlOpFlag_ShiftDown,TRUE>
{
public:
	CGuiAgent_CameraLock()
	{
		_angelCam=-10000.0f;
		_dist=-1.0f;
		_hCT=SeeThruTargetHandle_Null;
		_an=NULL;
	}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

protected:

	void _UpdateCam(int x,int y);

	BOOL _UpdateCamAngle();
	float _fovOrg;

	float _angelCam;
	i_math::vector3df _pos;

	int _xLast,_yLast;

	float _dist;

	IAnimNodeMatFixed *_an;
	SeeThruTargetHandle _hCT;

};
