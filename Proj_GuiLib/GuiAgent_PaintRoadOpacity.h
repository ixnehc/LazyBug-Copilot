
#pragma  once

#include "GuiAgent_CtrlPoints.h"

#include "WorldSystem/ITrrn.h"

class CGuiAgent_PaintRoadOpacity :public CGuiAgent_Dragger<TRUE,0>
{
public:
    CGuiAgent_PaintRoadOpacity()
    {
    }
    ~CGuiAgent_PaintRoadOpacity()
    {
    }

    virtual BOOL OnBeginDrag(int x, int y, DWORD flag);
    virtual void OnEndDrag(int x, int y, DWORD flag);
    virtual void OnDrag(int x, int y, DWORD flag);
    virtual BOOL OnDraw();
    virtual BOOL OnRButtonClick(int x, int y, DWORD flag);
    virtual BOOL OnCommand(DWORD idCmd);

    virtual BOOL OnMouseMove(int x, int y, DWORD flag);

	virtual BOOL OnMouseWheel(int delta,DWORD flag);

protected:

    TrrnSeedMap _seedmap;

};
