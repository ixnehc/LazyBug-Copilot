#pragma once

#include "MapUtil.h"
#include "GuiEditor.h"

class CMinMapUtil :public CMapUtil
{

public:
	CMinMapUtil(void);
	~CMinMapUtil(void);

	virtual void RegisterAgent();
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView);
	
	virtual void RegisterMode();

	// declare tool class
	BEGIN_DECLARE_TOOL_CLASS(CMinMapUtil,TOOL_MAPCONTROL)

	END_DECLARE_TOOL_CLASS()

protected:	
	void _SetMapData(const char * fileName);
	void _RebuildImage(void * data,DWORD sz);
};

