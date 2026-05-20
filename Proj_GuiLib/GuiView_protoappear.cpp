#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IFont.h"


#include "GuiData_Proto.h"

#include "GuiData.h"
#include "GuiData_debugger.h"
#include "GuiData_ProtoLogic.h"


#include "GuiView_protoappear.h"

#include "RenderPortBase.h"

#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetRenderer.h"
#include "WorldSystem/IAssetShell.h"
#include "WorldSystem/IAnimNodes.h"
#include "WorldSystem/IAssetEventer.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IGlobalRenv.h"

#include "WMGuiLib.h"

#include "timer/profiler.h"
#include "Log/LogDump.h"
#include "MultiTree/NodeTree.h"

#include "Registry/Registry.h"


//////////////////////////////////////////////////////////////////////////
//CGuiView_ProtoAppearance
void CGuiView_ProtoAppearance::_OnPreDraw(IRenderPort *rp)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	if (!dataProto)
		return;

	ICamera *cam=rp->QueryCamera();
	cam->Clone(dataProto->cam);
	float n,f;
	extern void CalcDefaultNearFar(CConfig *cfg,float &n,float &f);
	CalcDefaultNearFar(g_ssGuiLib.cfg,n,f);
	cam->SetNearFar(n,f);

	int fov=75;
	if (g_ssGuiLib.reg)
		fov=g_ssGuiLib.reg->ReadInt("General","EditorCamFov",75);
	cam->SetFov(((float)fov)*(float)i_math::GRAD_PI2);

}

void CGuiView_ProtoAppearance::_RecordMatNodes()
{
	DEFINE_GUIDATA_PROTO(dataProto);

	dataProto->nodemats.clear();

	IProto *proto=dataProto->proto();
	if (!proto)
		return;

	CNodeTree *ntree=proto->GetNodeTree()->GetTree();
	if (!ntree)
		return;//not ready to change

	DWORD c;
	NodeHandle *nodes;
	nodes=ntree->Enum(NodeHandle_Root,NodeType_None,c);
	for (int i=0;i<c;i++)
	{
		const char *path=ntree->GetPath(nodes[i]);
		ProtoNodeID id=proto->FindNodeID(path);
		IProtoNode *node=proto->GetNode(id);
		if (!node)
			continue;

		ProtoNodeType type=node->GetType();
		if (type==PN_LuaObj)
			continue;

		GuiData_Proto::NodeMat matnode;
		matnode.base.makeIdentity();
		matnode.local=node->GetLocalMat();

		if (type==PN_Entity)
		{
			if (dataProto->entityView->GetEntity(id))//确认这个node对应的entity此时被创建出来了
				dataProto->nodemats[id]=matnode;
		}

		if (type==PN_Asset)
		{//对于Asset我们要确定它的base

			IAsset *ast=dataProto->entityView->GetAsset(id);
			if (ast)
			{//只有当这个asset此时被创建出来时,我们才会记录它的matnode
				if (ast->GetBaseXform(ANIMTICK_FROM_SECOND(dataProto->tView),matnode.base))
					dataProto->nodemats[id]=matnode;
			}
		}
	}
}


void CGuiView_ProtoAppearance::_OnDraw(IRenderPort *rp)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	if (!dataProto)
		return;
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);
	if (!dataDebugger)
		return;

	rp->ClearBuffer(ClearBuffer_All,ColorAlpha(0x7f7f7f,0xff));

	if (dataDebugger->context->IsBreak())
	{
		DrawFontArg arg;
		arg.SetLocation(100,100);
		rp->DrawText("Debug Break...,Nothing to Present!",arg);
		return;
	}

	if (dataDebugger->context->IsRunning())
	{//处于测试模式

		dataProto->pES->GetWS()->GetRS()->FlushCommand();
		dataProto->pES->Render(rp,AdrPart_All);

		GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");


// 		if (dataViewSwitch)
// 		{
// 			if (dataViewSwitch->bShow)
// 				DrawProfile(rp,dataViewSwitch->mgr,i_math::pos2di(10,10));
// 		}


// 		ITexture *tex=dataProto->pES->GetWS()->GetRS()->GetFontMgr()->GetFontTexture(0);

// 		DrawTextureArg arg;
// 		arg.SetDest(200,200);
// 
// 		rp->DrawTexture(tex,arg);
		return;
	}

	//画编辑状态的entity

	if (dataProto->proto())
	{
		if (!dataProto->proto()->ContainGlobalAsset())
		{
			//更新光源的方向
			if (TRUE)
			{
				IAssetRenderer *adr=dataProto->pES->GetAS()->GetRenderer();
				GlobalRenv *grenv=adr->GetGlobalRenv();
				grenv->lgt.dir.set(0,-1,-1);
				grenv->lgt.dir.normalize();
				grenv->lgt.colDifDL.set(0.7f,0.7f,0.7f);
				grenv->lgt.colSpecDL.set(0.7f,0.7f,0.7f);
				grenv->lgt.colAmbDL.set(0.3f,0.3f,0.3f);
			}

			assert(!dataProto->entityView);
			dataProto->entityView=dataProto->pES->CreateEntity(i_math::matrix43f(),dataProto->protoid,FALSE,TRUE);

			EntitySystemInput in;
			i_math::recti rc;
			rp->GetRect(rc);
			in.SetRPSize(rc.getSize());
			srand(7177);

			in.dt=0.0f;
			dataProto->pES->Update(in);//
			in.dt=dataProto->tView;
			dataProto->pES->Update(in);

			//先画非界面部分
			dataProto->pES->Render(rp,AdrPart_NotShell);

			_RecordMatNodes();
		}
	}
	DrawGrid(rp,10,1);

}

void CGuiView_ProtoAppearance::_OnDrawNoDepth(IRenderPort *rp)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	if (!dataProto)
		return;
	if (dataProto->entityView)
	{
		//画界面部分
		dataProto->pES->Render(rp,AdrPart_Shell);
	}

}

void CGuiView_ProtoAppearance::_OnPostDraw(IRenderPort *rp)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	if (!dataProto)
		return;
	if (dataProto->entityView)
	{
		//清除为了绘制而创建的entity

		dataProto->entityView->Destroy();
		dataProto->entityView=NULL;

		dataProto->pES->GetGlobal()->ClearDynEntities();

		EntitySystemInput in;
		i_math::recti rc;
		rp->GetRect(rc);
		in.SetRPSize(rc.getSize());

		dataProto->pES->GetAS()->GetSS()->pAS->GetEventer()->ClearAllSignals();
		if (dataProto->pES->GetAS()->GetSS()->pLabAS)
			dataProto->pES->GetAS()->GetSS()->pLabAS->GetEventer()->ClearAllSignals();

		dataProto->pES->Update(in);//之所以还要update一下,是为了flush那些defered destroyed 的entity

		dataProto->pES->SwitchEditMode(TRUE);//把时间归0

	}

	//显示profiler 结果
	if (!dataProto->pES->IsInProgress())
	{
		GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
		if (dataViewSwitch)
		{
			if (dataViewSwitch->bShowProfiler)
				DrawProfile(rp,dataViewSwitch->mgr,i_math::pos2di(10,10));
			if(dataViewSwitch)
				dataViewSwitch->mgr->Dump();
		}
	}

}


BOOL CGuiView_ProtoAppearance::Respond(CtrlOp &co)
{

	CGeView::Respond(co);

	DEFINE_GUIDATA_PROTO(dataProto);
	if (!dataProto)
		return TRUE;
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);
	if (!dataDebugger)
		return TRUE;

	if (TRUE)
	{
		ESProgressCallBack dlgt;
		dlgt.bind(this,&CGuiView_ProtoAppearance::OnProgressDraw);
		dataDebugger->context->SetProgressDrawCallBack(dlgt);
	}



	if (dataDebugger->context->IsRunning())
	{
		if (!_cursors.IsInit())
			_cursors.Init(dataProto->pES->GetAS());

		_hCursor=_cursors.GetActive();
	}

	return TRUE;
}

BOOL CGuiView_ProtoAppearance::OnProgressDraw()
{
	Draw();
	return TRUE;
}

BOOL CGuiView_ProtoAppearance::RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret)
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

					IProto *proto=dataProto->proto();
					if (proto)
					{
						data->drops=(const char*)wParam;
						data->bAssetOrProto=(BOOL)lParam;

						data->ptDrop.set(ptCursor.x,ptCursor.y);
						_ScreenToGG(data->ptDrop.x,data->ptDrop.y);
					}
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
