
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>

#include "resdata/AnimTreeData.h"

#include "ResEditCtrl.h"

#include "GObjGrid.h"



class CResEditPanel;
struct ResEditPanelState;

struct Reps_AnimTree;
struct AnimTreeData;

class CAnimTreeGrid:public CGObjGrid,public CResEditCtrl
{
public:
	CAnimTreeGrid()
	{
	}

	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_AnimTree *GetState()	{		return (Reps_AnimTree *)CResEditCtrl::_state;	}
	AnimTreeData *GetResData()	{		return (AnimTreeData*)_GetResData();	}

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	
	virtual void EnableCtrl(BOOL bActive=TRUE);

	virtual CXTPPropertyGridItem *InsertVar(void *var,const char *cap,const char *desc,GVarType vt,GSem &sem);

protected:
	DECLARE_MESSAGE_MAP()

};
