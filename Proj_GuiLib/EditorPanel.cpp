/********************************************************************
	created:	2007/2/15   17:55
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel.cpp
	author:		cxi
	
	purpose:	Base class for all the Editor Panel
*********************************************************************/
#include "stdh.h"

#include "EditorPanel.h"



BOOL CEditorPanel::Respond(CtrlOp &co)
{
	if (co.op==CtrlOp::Op_Timer)
	{
		OnUpdateUI();
	}

	return CGuiEditor::Respond(co);
}
