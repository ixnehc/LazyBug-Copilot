#pragma once

#include "GuiEditor.h"
#include "matrixedit_base.h"//#include "resdata/DummiesData.h"


class CDummiesSelectAgent :public  CGuiAgent_Dragger<1,0>
{
public:
	CDummiesSelectAgent(void);
	~CDummiesSelectAgent(void);
	virtual const char * getClassName() {return "CDummiesSelectAgent";}
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual	BOOL OnCommand(DWORD idCmd);
	virtual void OnDrag(int x,int y);
private:
	BOOL _HitTestAbb(i_math::line3df &line,i_math::matrix43f &mat,i_math::aabbox3df &aabb,i_math::vector3df &interEnter,i_math::vector3df &interLeave);
};
