/********************************************************************
	created:	2007/2/15   13:04
	filename: 	e:\IxEngine\Proj_WorldEditor\WEditors.cpp
	author:		cxi
	
	purpose:	world editor construction
*********************************************************************/

#include "stdh.h"


#include "WEditor_ChildFrame.h"



BOOL CWEditor_ChildFrame::Create(CXTPDockingPaneManager *panemgr)
{
	CWorldEditor::Create(panemgr);


	return TRUE;

}

void CWEditor_ChildFrame::Destroy()
{
	ResetContent();

	_ClearPanels();
}


// BOOL CWEditor_ChildFrame::LoadContent(ProtoID protoid,CGuiPanel_Proto *panel,DebuggerContext *dc,CPrlFrameProxy *frameproxy)
// {
// 	//////////////////////////////////////////////////////////////////////////
// 	//Init the datas
// 	if (TRUE)//frame proxy
// 	{
// 		_dataFrameProxy.proxy=frameproxy;
// 	}
// 
// 	if (TRUE)//debug context
// 	{
// 		_dataDebugger.context=dc;
// 	}
// 
// 
// 	if (TRUE)//proto 
// 	{
// 		extern CWorld g_World;
// 		_dataProto.pES=g_World.GetEntitySystem();
// 		_dataProto.lib=_dataProto.pES->GetProtoLib();
// 		_dataProto.protoid=protoid;
// 		IProto *proto=_dataProto.lib->ObtainProto(protoid);
// 		if (proto)
// 			_dataProto.pathProtoFile=proto->GetFilePath();
// 
// 		_dataProto.cam=g_World.GetWS()->GetRS()->CreateCamera();
// 		_dataProto.cam->SetPosTarget(i_math::vector3df(1,1,1),i_math::vector3df(0,0,0));
// 
// 		_dataProto.idGE=_dataProto.proto()->GetRunGE();
// 		_dataProto.idGT=_dataProto.proto()->GetRunGT();
// 		if (_dataProto.proto())//gg transform
// 			_dataProto.proto()->GetGraphTransform(_dataProto.xlate,_dataProto.scale);
// 	}
// 
// 
// 	if (TRUE)//profiler
// 	{
// 		_dataViewSwitch.mgr=GetProfilerMgr();
// 		_dataViewSwitch.bShowProfiler=FALSE;
// 	}
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	//Init the views
// 	_viewAppearance.SetRS(g_World.GetWS()->GetRS());
//  
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	//register 
// 	_mgr.AddModMgr("proto");
// 
// 	_mgr.RegisterData(&_dataFrameProxy);
// 	_mgr.RegisterData(&_dataDebugger);
// 	_mgr.RegisterData(&_dataProto);
// 	_mgr.RegisterData(&_dataLogic);
// 	_mgr.RegisterData(&_dataShellInfo);
// 	_mgr.RegisterData(&_dataViewSwitch);
// 	_mgr.RegisterData(&_dataRichGrids);
// 
// 	_mgr.RegisterView(&_viewAppearance);
// 	_mgr.RegisterView(&_viewLogic);
// 
// 	_actorProto.SetPanel(panel);
// 	_mgr.RegisterActor(&_actorProto);
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	//Start the actors
// 	_actorProto.Reset();
// 
// 	return TRUE;
// 
// }

void CWEditor_ChildFrame::ResetContent()
{
// 	if (_dataProto.proto())//save something back to the proto
// 	{
// 		IProto *proto=_dataProto.proto();
// 		if (proto)
// 		{
// 			BOOL bCanSave=FALSE;
// 			if (proto->GetNodeTree()->GetTree())
// 			{
// 				if (!proto->GetNodeTree()->GetTree()->IsReadOnly())
// 					bCanSave=TRUE;
// 			}
// 
// 			if (bCanSave)
// 			{
// 				proto->SetGraphTransform(_dataProto.xlate,_dataProto.scale);
// 				proto->SetRunGE(_dataProto.idGE);
// 				proto->SetRunGT(_dataProto.idGT);
// 				proto->Save();
// 			}
// 		}
// 	}
// 
// 	_mgr.Reset();
// 
// 	_dataProto.Clear();
// 
// 	_viewAppearance.Clear();
// 	_viewLogic.Clear();

}

void CWEditor_ChildFrame::Refresh()
{
}
