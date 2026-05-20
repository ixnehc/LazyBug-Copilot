#pragma once

#include "GuiLib.h"
#include "GuiEditor.h"
#include "RenderSystem/IRenderSystem.h"

#include "ruler/ruler.h"

struct ValueSet;

struct Key_f;
class GuiLib_Api CGuiView_ValueSet : public CGuiView
{
public:
	CGuiView_ValueSet();
	~CGuiView_ValueSet();
	
	virtual const char*	GetName()	{		return "ValueSet";	}
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;	}

	virtual void _OnDraw( GraphicsGraph *gg );

	void SetReadOnly(BOOL bReadOnly)
	{
		_bReadOnly=bReadOnly;
	}

	virtual BOOL Respond(CtrlOp &co);


protected:

	void _DrawGrid(GraphicsGraph *gg);
	void _DrawRuler(GraphicsGraph *gg);
	void _DrawColor(ValueSet *vs,DWORD idx,GraphicsGraph *gg,BOOL bHilight,int iSel);
	void _DrawFloat(ValueSet *vs,GraphicsGraph *gg,DWORD col,BOOL bHilight,int iSel);
	void _DrawLimitRect(i_math::rectf &rcLimit,GraphicsGraph *gg);
	void _DrawValueSets(GraphicsGraph *gg);

	void _DrawKey(i_math::pos2di &pt,GraphicsGraph *gg,BOOL bHilight,Key_f *k);

	//这两个ruler只在_OnDraw(..)函数内部有效
	CRuler _rulerX,_rulerY;

	BOOL _bReadOnly;

};
