#pragma once
#include "MapUtil.h"
#include "progress/progress.h"
#include "WorldSystem/IWorldSystem.h"




#include "GuiEditor.h"


class CGameRgnUtil;
class CGuiAgent_GameRgnMap :public  CGuiAgent
{
public:
	CGuiAgent_GameRgnMap();
	~CGuiAgent_GameRgnMap();

	void SetOwner(CGameRgnUtil *owner)	{		_owner=owner;	}
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnTimer(int dt,DWORD flag);

protected:
	void _DrawRgns();
	void _EnsurePixels();
	void _LoadPixels(i_math::recti &rc,std::unordered_map<DWORD,DWORD>&cols);

	BOOL _bModified;
	i_math::recti _rcDirty;
	std::vector<DWORD> _pixels;

	BOOL _bDown;

	CGameRgnUtil *_owner;
};




class CGameRgnUtil :public CMapUtil
{
public:
	CGameRgnUtil(void);
	~CGameRgnUtil(void);

	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void OnInitDlg(CGeActor * actor);

	virtual void RegisterAgent();
	void RegisterMode();

	DWORD GetSelID();
	DWORD GetRadius();
protected:
	void _RefreshRegionList();

	CListCtrl _listRgnID;


	CGuiAgent_GameRgnMap _agent;
	
	BEGIN_DECLARE_TOOL_CLASS(CGameRgnUtil,TOOL_MAPCONTROL)
	END_DECLARE_TOOL_CLASS()

	
};
