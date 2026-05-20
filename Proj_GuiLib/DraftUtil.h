#pragma once

#include "MapUtil.h"
#include "GuiEditor.h"

class CDraftUtil :public CMapUtil
{

public:
	CDraftUtil(void);
	~CDraftUtil(void);

	virtual void RegisterAgent();
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView);
	
	virtual void RegisterMode();

	// declare tool class
	BEGIN_DECLARE_TOOL_CLASS(CDraftUtil,TOOL_MAPCONTROL)

	END_DECLARE_TOOL_CLASS()

protected:	
	void _SetMapData(const char * fileName);
	void _RebuildImage(void * data,DWORD sz);
};

