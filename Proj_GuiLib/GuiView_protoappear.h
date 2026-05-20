#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

#include "cursors/Cursors.h"

class GuiLib_Api CGuiView_ProtoAppearance :public CGuiView
{
public:
	CGuiView_ProtoAppearance()
	{
	}
	virtual const char*GetName()	{		return "proto_appear";	}
	virtual void _OnPreDraw(IRenderPort *rp);
	virtual void _OnDraw(IRenderPort *rp);
	virtual void _OnDrawNoDepth(IRenderPort *rp);
	virtual void _OnPostDraw(IRenderPort *rp);

	virtual BOOL Respond(CtrlOp &co);
	virtual BOOL RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret);

protected:

	BOOL OnProgressDraw();

	void _RecordMatNodes();

	CCursors _cursors;


};
