#pragma once

#include "GuiLib.h"

#include <map>
#include <string>

#include "NodeTreeCtrl.h"

//VL for Vegetation Lib
class GuiLib_Api CVLContentTree:public CNodeTreeCtrl
{
public:
	CVLContentTree()
	{
	}

	NodePtr GetCurSel();
protected:
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeType type);


};

