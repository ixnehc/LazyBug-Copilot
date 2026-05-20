#pragma once

#include "GuiLib.h"

#include "EditorBase.h"

#include "ArcBall.h"

class CWEA_CameraRotater:public CGEA_Dragger<FALSE,0>
{
public:
	virtual void OnBeginDrag(int x,int y);
	virtual void OnEndDrag(int x,int y);
	virtual void OnDrag(int x,int y);
	virtual BOOL OnMouseWheel(int delta,DWORD flag);

protected:
};

class CWEA_CameraMover:public CGEA_Dragger<TRUE,0>
{
public:
	virtual void OnBeginDrag(int x,int y);
	virtual void OnDrag(int x,int y);

	virtual BOOL OnKeyDown(char c,DWORD flag);

protected:
};


class CWEAgent_BaseMenu:public CGuiEditorAgent
{
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
};

//A base editor
class CWEditor_Base:public CGuiEditor
{
public:
	CCameraController *GetCameraController()	{		return &_camctrl;	}
	virtual void OnInitAgent();
	virtual void OnEnable();
protected:
	CCameraController _camctrl;


};


struct WEditorEnv;
class GuiLib_Api CWEditorMgr:public CGuiEditorMgr
{
public:
	CWEditorMgr()
	{
		AddEditor(&_base,"");
	}

	void SetEnv(WEditorEnv &env);

	CWEditor_Base _base;

};

