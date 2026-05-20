#include "stdh.h"
#include ".\GuiLib.h"

#include "GuiData_ProtoLogic.h"
#include "GuiData_proto.h"
#include "GuiData_debugger.h"

#include "GuiView_protologic.h"

#include "RenderPortBase.h"
 
#include "stringparser/stringparser.h"

#include "MultiTree/NodeTree.h"

#include "graphicsgraph.h"

#include "WMGuiLib.h"




//////////////////////////////////////////////////////////////////////////
//CGuiView_ProtoLogic

void CGuiView_ProtoLogic::_OnDraw(GraphicsGraph *gg)
{
	GuiData_ProtoLogic *data=(GuiData_ProtoLogic *)FindData("proto_logic");

	i_math::recti rc(-20000,-20000,20000,20000);
	gg->FillHatchRect(rc,0x7f7f7f,0x6f6f6f,4);

	_DrawStuff(gg);
}


void CGuiView_ProtoLogic::_DrawStuff(GraphicsGraph *gg)
{
	GuiData_ProtoLogic *dataLogic=(GuiData_ProtoLogic *)FindData("proto_logic");
	GuiData_Proto *dataProto=(GuiData_Proto*)FindData("proto");

	IProto *proto=dataProto->proto();

	if (!proto)
	{
		dataLogic->graph.Clear();
		return;
	}

	dataLogic->graph.Load(proto,gg,dataProto->pES);

	dataLogic->graph.Draw(gg,&dataProto->sels[0],dataProto->sels.size());


}

BOOL CGuiView_ProtoLogic::Respond(CtrlOp &co)
{
	GuiData_Proto *dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

//	if (dataProto)
//	{
//		if (dataProto->IsReadOnly())
//			return FALSE;
//	}
//	if (dataDebugger)
//	{
//		if (dataDebugger->context->IsRunning())
//			return FALSE;
//	}
	return CGuiView::Respond(co);
}


BOOL CGuiView_ProtoLogic::RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret)
{
	if ((msg==GLM_Proto_DragOver)||(msg==GLM_Proto_DragDrop))
	{
		GuiData_Proto *dataProto=(GuiData_Proto*)FindData("proto");
		GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");
		BOOL bCanDrop=TRUE;
		if (dataProto)
		{
			if (dataProto->IsReadOnly())
				bCanDrop=FALSE;
		}
		if (dataDebugger)
		{
			if (dataDebugger->context->IsRunning())
				bCanDrop=FALSE;
		}
		if (bCanDrop)
		{
			if (msg==GLM_Proto_DragOver)
			{
				ret=1;
				return TRUE;
			}
			if (msg==GLM_Proto_DragDrop)
			{
				GuiData_ProtoLogic *data=(GuiData_ProtoLogic *)FindData("proto_logic");
				if (data)
				{
					data->drops=(const char*)wParam;
					data->ptDrop.set(ptCursor.x,ptCursor.y);
					data->bAssetOrProto=(BOOL)lParam;
					_ScreenToGG(data->ptDrop.x,data->ptDrop.y);
				}

				ret=0;
				return TRUE;
			}
		}
		ret=0;
		return TRUE;
	}
	return CGuiView::RespondMsg(ptCursor,msg,wParam,lParam,ret);
}
