#pragma  once

#include "GuiEditor.h"
#include "GuiAgent_general.h"

class CGuiAgent_WaterPaint :public CGuiAgent_Dragger<DRAG_BUTTON_LEFT,FALSE>
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);

	virtual BOOL OnCommand(DWORD idCmd);
	
	virtual BOOL OnDraw();
protected:
	void _CalQuad(int x,int y,DWORD flag);
	void _Paint();
	void _NotifyPaintFinish();
	BOOL _CheckWorkable();
private:
	std::vector<i_math::pos2di> _blks;
	i_math::vector3df _vintersec;
	i_math::vector3df _vborder[4];
};