#pragma once

#include "GuiLib.h"
#include "GuiEditor.h"

//#include "Changelists.h"

#include "GuiData_Changelists.h"


struct Changelists;
struct ChangelistsSnapshot;
struct Key_f;
class CGuiView_Changelists : public CGuiView
{
public:
	CGuiView_Changelists();
	~CGuiView_Changelists();
	
	virtual const char*	GetName()	{		return "Changelists";	}
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;	}

	virtual void _OnDraw( GraphicsGraph *gg );

	void SetReadOnly(BOOL bReadOnly)
	{
		_bReadOnly=bReadOnly;
	}

	void SetNeedReCenterCur()
	{
		_needReCenterCur = true;
	}

	virtual BOOL Respond(CtrlOp &co);

protected:

	bool _needReCenterCur;

	void _RefreshSnapshot(GraphicsGraph* gg);

	void _DrawChangelists(GraphicsGraph *gg);

	void _DrawNode(GraphicsGraph* gg, const ChangelistsSnapshot::Node &node);

	void _DrawChangelistFilePathes(GraphicsGraph* gg, const i_math::recti& rc, const ChangelistsSnapshot &snapshot, const ChangelistsSnapshot::Node& node, CChangelists& changelists,bool isSelected);

	void _ReCenterCur(GraphicsGraph* gg);

	BOOL _bReadOnly;

};
