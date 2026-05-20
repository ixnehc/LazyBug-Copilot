#pragma once

#include "GuiLib.h"
#include "SscBtn.h"



class CBrushLibSscBtn:public CSscBtn
{
public:

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRefRes();

protected:
	virtual void _OnCustomizeMenu(CMenu *menu);
};


