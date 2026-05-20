
#pragma once

#include "engine/Engine.h"

#include "weditor/WEditor.h"

#include "editor/editor.h"
#include "GuiData.h"
#include "GuiData_database.h"
#include "GuiData_RichGrids.h"
// #include "GuiActor_proto.h"

class CWEditor_ChildFrame:public CWorldEditor
{
public:
	CWEditor_ChildFrame()
	{
	}

	virtual BOOL Create(CXTPDockingPaneManager *panemgr)	;
	virtual void Destroy();
	virtual void ResetContent();

//	BOOL LoadContent(ProtoID protoid,CGuiPanel_Proto *panel,DebuggerContext *dc,CPrlFrameProxy *frameproxy);

	void Refresh();

protected:
	GuiData_Changelist _dataCl;
	GuiData_RichGrids _dataRichGrids;

// 	CGuiActor_Proto _actorProto;


	friend class CMainFrame;
	friend class CChildView;
};



