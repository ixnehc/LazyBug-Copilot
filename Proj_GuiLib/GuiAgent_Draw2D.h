#pragma once
#include "GuiEditor.h"

class CGuiAgent_Draw2D :public CGuiAgent_Dragger<1,FALSE>
{
public:
	enum
	{ 
		Select_Multi,
		Select_Sing,
	};

	CGuiAgent_Draw2D();
	~CGuiAgent_Draw2D(void);

	BOOL OnLButtonDown(int x,int y,DWORD flag);
	BOOL OnRButtonClick(int x,int y,DWORD flag);
	BOOL OnCommand(DWORD idCmd);
	BOOL OnDraw();

	void SetSelectMode(DWORD modeSelect){_modeSelcect = modeSelect;}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnTimer(int dt,DWORD flag);
protected:
	void _GetFeildRc(i_math::recti &rc,int &w,int &h);
	void _UpdateSel(int x,int y,DWORD flag);
	void _ToWorld(int &x,int &y);
	void _SelectFld(int x,int y,DWORD flag,i_math::pos2di* fldSels = NULL);
	void _UpdateWorldCenter();
	
	int _x ,_y;
	i_math::pos2di _worldCenter;
	i_math::vector3df _oldEyeDir;
	i_math::pos2di _dirWalk[3];
	i_math::recti _rcSel;
	DWORD _modeSelcect;

	std::vector<i_math::pos2di> _oldSels;
};

