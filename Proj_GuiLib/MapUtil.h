#pragma once
#include "ToolBase.h"

#include "GuiEditor.h"

#include "GuiAgent_Draw2D.h"

class CMapUtil :public CToolBase
{
public:
	CMapUtil(void);
	~CMapUtil(void);

	virtual const char * GetTypeName();
	virtual DWORD GetType();
	virtual void RegisterAgent();

	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual BOOL InitDlg(CWnd * pParent);

	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView);
	
	virtual DWORD SelectMode(){ return CGuiAgent_Draw2D::Select_Multi;}
protected:
	class _CGuiAgent_MapUtil :public CGuiAgent
	{
	public:
		virtual BOOL OnCommand(DWORD idCmd);
		void SetTool(CMapUtil * owner){_owner = owner;}
		BOOL OnRButtonClick(int x,int y,DWORD flag);
	protected:
		CMapUtil * _owner;
	};

	class _CGuiAgentMinMap :public CGuiAgent 
	{
	public:
		_CGuiAgentMinMap(){}
		~_CGuiAgentMinMap(){}
		virtual BOOL OnDraw();
		BOOL _Draw();
	};
	friend class _CGuiAgentMinMap;

protected:
	CGuiAgent_Draw2D  _agentDraw;
	_CGuiAgent_MapUtil _agentMapUitl;
	_CGuiAgentMinMap _agentMinMap;
};
