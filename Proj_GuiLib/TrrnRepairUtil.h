#pragma once
#include "MapUtil.h"
#include "progress/progress.h"
#include "WorldSystem/IWorldSystem.h"

class CTrrnRepairUtil :public CMapUtil
{
public:
	CTrrnRepairUtil(void);
	~CTrrnRepairUtil(void);

	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void OnInitDlg(CGeActor * actor);
	
	void RegisterMode();
protected:
	
	BEGIN_DECLARE_TOOL_CLASS(CTrrnRepairUtil,TOOL_MAPCONTROL)
	END_DECLARE_TOOL_CLASS()

	
};
