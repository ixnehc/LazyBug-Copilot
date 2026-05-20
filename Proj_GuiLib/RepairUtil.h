#pragma once
#include "MapUtil.h"
#include "progress/progress.h"
#include "WorldSystem/IWorldSystem.h"

class CRepairUtil :public CMapUtil
{
public:
	CRepairUtil(void);
	~CRepairUtil(void);

	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void OnInitDlg(CGeActor * actor);
	
	void RegisterMode();
	
protected:
	void _RepairTree(CGeActor * actor,BOOL bRemove);
	void _RepairWater(CGeActor * actor,BOOL bRemove);
	void _RepairVegetable(CGeActor * actor,BOOL bRemove);
	BOOL _MoveTo(CGeActor * actor,i_math::pos2di &ptF); //TRUE : check out success
	void _MoveHome(CGeActor *actor);
	BEGIN_DECLARE_TOOL_CLASS(CRepairUtil,TOOL_MAPCONTROL)
	END_DECLARE_TOOL_CLASS()
private:
	std::vector<i_math::pos2di> _blks;
	i_math::recti _blkRC;	
};
