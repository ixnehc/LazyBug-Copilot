#pragma once

#include "GuiEditor.h"

class CGuiAgent_ShellTransform:public CGuiAgent
{
public:
	CGuiAgent_ShellTransform(void);
	~CGuiAgent_ShellTransform(void);

	BOOL OnLButtonClick(int x,int y,DWORD flag);
	void OnAttachView(CGeView *view,DWORD iLevel);


protected:
	void _UpdateTransform(i_math::matrix43f &mat);

private:
	i_math::matrix43f _mat;
};
