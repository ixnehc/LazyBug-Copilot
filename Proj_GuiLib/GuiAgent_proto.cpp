/********************************************************************
created:	2008/5/20   18:14
file path:	d:\IxEngine\Proj_GuiLib
author:		cxi

purpose:	the gui agent used in editing proto
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IFont.h"


#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/ILuaMachine.h"
#include "WorldSystem/client/IClient.h"

#include "GuiAgent_proto.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "ruler/ruler.h"

#include "EditPopup.h"
#include "StubPopup.h"

#include "ProtoSelectDlg.h"

#include "GuiData.h"

#include "GuiData_frameproxy.h"

#include "GuiData_proto.h"
#include "GuiData_protologic.h"
#include "GuiData_debugger.h"

#include "commondefines/general_stl.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetRenderer.h"

#include "GuiActor_proto.h"

#include "timer/profiler.h"

#include "AgentCmdID.h"

#include "graphicsgraph.h"

#include "ximage.h"

#include "config/config.h"

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_PNMatrixEdit
BOOL CGuiAgent_PNMatrixEdit::Respond(CtrlOp &co)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

	if (!dataDebugger->context->IsRunning())
	{
		if (!dataProto->IsReadOnly())
			CGuiAgent_3DNodeMatEdit::Respond(co);
	}
	return TRUE;
}

BOOL CGuiAgent_PNMatrixEdit::OnDraw()
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

	if (!dataDebugger->context->IsRunning())
	{
		if (!dataProto->IsReadOnly())
			CGuiAgent_3DNodeMatEdit::OnDraw();
	}
	return TRUE;

}



void*CGuiAgent_PNMatrixEdit::_GetSelBuf()
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		return &dataProto->sels;
	return NULL;
}

i_math::matrix43f *CGuiAgent_PNMatrixEdit::_GetMat(H3DNode node)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
	{
		if (dataProto->GetNodeMat((ProtoNodeID)node,_matTemp))
			return &_matTemp;
	}
	return NULL;
}

i_math::matrix43f *CGuiAgent_PNMatrixEdit::_GetLocalMat(H3DNode node,i_math::matrix43f &matParent)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
	{
		if (dataProto->GetNodeLocalMat((ProtoNodeID)node,_matTemp,matParent))
			return &_matTemp;
	}
	return NULL;
}


void CGuiAgent_PNMatrixEdit::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		dataProto->SetNodeMat((ProtoNodeID)node,mat);
}

void CGuiAgent_PNMatrixEdit::_MoveLocal(H3DNode &node,i_math::matrix43f &mat)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		dataProto->SetNodeLocalMat((ProtoNodeID)node,mat);
}


void CGuiAgent_PNMatrixEdit::_BeginMatrixEdit(i_math::matrix43f *mat)
{
	CGuiAgent_3DNodeMatEdit::_BeginMatrixEdit(mat);
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		dataProto->bChanging=TRUE;

}

void CGuiAgent_PNMatrixEdit::_EndMatrixEdit(i_math::matrix43f *mat)
{
	CGuiAgent_3DNodeMatEdit::_EndMatrixEdit(mat);
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		dataProto->bChanging=FALSE;
}


BOOL CGuiAgent_PNMatrixEdit::OnCommand(DWORD idCmd)
{
	if (idCmd==ID_AGENT_3DNODEEDIT_ALIGNTOSURF)
	{
		extern IEntity *CreateGuiDataProto(CGuiAgent *agent);
		extern void DestroyGuiDataProto(IEntity *en,CGuiAgent *agent);
		IEntity *entity=CreateGuiDataProto(this);
		BOOL bRet=CGuiAgent_3DNodeMatEdit::OnCommand(idCmd);
		DestroyGuiDataProto(entity,this);

		return FALSE;
	}

	return CGuiAgent_3DNodeMatEdit::OnCommand(idCmd);
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_OperatePN

static BOOL _AgentCanOp(CGuiAgent *agent)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)agent->FindData("proto");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)agent->FindData("debugger");

	if (!dataDebugger->context->IsRunning())
	{
		if (!dataProto->IsReadOnly())
			return TRUE;
	}
	return FALSE;

}

void*CGuiAgent_OperatePN::_GetSelBuf()
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
		return &dataProto->sels;
	return NULL;
}

IEntity *CreateGuiDataProto(CGuiAgent *agent)
{
	GuiData_Proto *dataProto=(GuiData_Proto *)agent->FindData("proto");
	if (dataProto)
	{
		if (dataProto->proto())
		{
			if (TRUE)
			{
				if (!dataProto->proto()->ContainGlobalAsset())
				{
					IEntity *entity;
					EntityCreateArg arg;
					entity=dataProto->pES->CreateEntity(i_math::matrix43f(),dataProto->protoid,FALSE,TRUE);

					EntitySystemInput in;

					i_math::recti rc;
					agent->GetClientRect(rc);
					in.SetRPSize(rc.getSize());

					srand(7177);
					in.dt=0.0f;
					dataProto->pES->Update(in);
					in.dt=dataProto->tView;
					dataProto->pES->Update(in);

					return entity;
				}
			}
		}
	}
	return NULL;
}

void DestroyGuiDataProto(IEntity *entity,CGuiAgent *agent)
{
	GuiData_Proto *dataProto=(GuiData_Proto *)agent->FindData("proto");
	if (dataProto)
	{
		if (dataProto->proto())
		{
			if (TRUE)
			{
				if (!dataProto->proto()->ContainGlobalAsset())
				{
					if (entity)
						entity->Destroy();
					dataProto->pES->GetGlobal()->ClearDynEntities();
					dataProto->pES->SwitchEditMode(TRUE);//使时间归0
				}
			}
		}
	}

}


H3DNode CGuiAgent_OperatePN::_HitTest(i_math::line3df &ray)
{
	extern IEntity *CreateGuiDataProto(CGuiAgent *agent);
	extern void DestroyGuiDataProto(IEntity *en,CGuiAgent *agent);
	IEntity *entity=CreateGuiDataProto(this);
	ProtoNodeID idHit=ProtoNodeID_Null;
	if (entity)
		idHit=entity->ProtoNodeHitTest(ray);
	DestroyGuiDataProto(entity,this);

	if (idHit!=ProtoNodeID_Null)
	{
		GuiData_Proto *dataProto=(GuiData_Proto *)FindData("proto");
		if (dataProto)
		{
			IProtoNode *node=dataProto->proto()->GetNode(idHit);
			if (!node->IsEditHelper())
				return (H3DNode)idHit;
		}
	}

	return H3DNode_Invalid;
}

void CGuiAgent_OperatePN::_CollectEnvelope(H3DNode *nodes,DWORD nNodes,Envelope &evlp)
{
	if (nNodes<=0)
		return;
	GuiData_Proto *dataProto=(GuiData_Proto *)FindData("proto");
	if (dataProto)
	{
		if (dataProto->entityView)
		{
			for (int i=0;i<nNodes;i++)
				dataProto->entityView->CollectEnvelope((ProtoNodeID)nodes[i],evlp);
		}
	}
}


BOOL CGuiAgent_OperatePN::OnRButtonClick(int x,int y,DWORD flag)
{
	if (_AgentCanOp(this))
		return __super::OnRButtonClick(x,y,flag);
	return TRUE;

}

BOOL CGuiAgent_OperatePN::OnCommand(DWORD idCmd)
{
	if (_AgentCanOp(this))
		return __super::OnCommand(idCmd);
	return TRUE;
}

BOOL CGuiAgent_OperatePN::OnDraw()
{
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

	if (!dataDebugger->context->IsRunning())
		return __super::OnDraw();
	return TRUE;
}

BOOL CGuiAgent_OperatePN::_Remove(H3DNode node)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ViewTimeCtrl

#define VTC_RULER_SCALE 200.0f


float CGuiAgent_ViewTimeCtrl::_TimeFromX(int x)
{
	i_math::recti rc;
	_GetClientRect(rc);

	CRuler ruler;
	ruler.SetLength(rc.getWidth());
	ruler.SetScale(VTC_RULER_SCALE);

	return ruler.ToWS(x);
}

void CGuiAgent_ViewTimeCtrl::_UpdateViewTime(int x,int y)
{
	DEFINE_GUIDATA_PROTO(dataProto);

	dataProto->tView=_TimeFromX(x);
	if (dataProto->tView<0.0f)
		dataProto->tView=0.0f;

	_Redraw(FALSE);

}

BOOL CGuiAgent_ViewTimeCtrl::OnBeginDrag(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);

	if(dataDebugger->context->IsRunning())
		return FALSE;

	i_math::recti rc,rc2;
	_GetClientRect(rc);
	rc.cutout(3,32,rc2);
	if (!rc2.isPointInside(x,y))
		return FALSE;

	_bLoop=FALSE;
	_UpdateViewTime(x,y);

	return TRUE;
}

void CGuiAgent_ViewTimeCtrl::OnEndDrag(int x,int y,DWORD flag)
{

}

void CGuiAgent_ViewTimeCtrl::OnDrag(int x,int y,DWORD flag)
{
	_UpdateViewTime(x,y);
}

BOOL CGuiAgent_ViewTimeCtrl::OnDraw()
{
	DEFINE_GUIDATA_PROTO(dataProto);
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);

	if(dataDebugger->context->IsRunning())
		return TRUE;
	IRenderPort *rp=GetRP();

	i_math::recti rc,rc2;
	_GetClientRect(rc);
	rc.cutout(3,32,rc2);

	rp->FillRect(rc2,ColorAlpha(0x7f7f7f,0xff));


	CRuler ruler;
	ruler.SetLength(rc.getWidth());
	ruler.SetScale(VTC_RULER_SCALE);

	if (_bLoop)
	{
		int x=ruler.ToRS(_durLoop);
		i_math::recti rcDur;
		rcDur=rc2;
		rcDur.Right()=rc.Left()+x;
		rp->FillRect(rcDur,ColorAlpha(0x3f7fff,0x5f));
	}

	rp->Line(rc2.Left(),rc2.Top(),rc2.Right(),rc2.Top(),ColorAlpha(0xdfdfdf,0xff));
	rp->Line(rc2.Left(),rc2.Top(),rc2.Left(),rc2.Bottom(),ColorAlpha(0xdfdfdf,0xff));


	std::string s;

	//绘制ruler
	if (TRUE)
	{

		float steps[]={0.01f,0.02f,0.05f,0.1f,0.2f,0.5f,1.0f,2.0f,5.0f,10.0f,0.0f};
		CRuler::WorldSpaceUnit units[100];
		int nMarks=ruler.BuildMarks(units,steps,60);

		std::vector<i_math::pos2di> lines;

		DrawFontArg arg;

		for (int i=0;i<nMarks;i++)
		{
			CRuler::RulerSpaceUnit x=ruler.ToRS(units[i]);

			lines.push_back(i_math::pos2di(x,rc2.Bottom()-8));
			lines.push_back(i_math::pos2di(x,rc2.Bottom()-1));

			FormatString(s,"{F:1}{S:10}{C:31,31,31}%g 秒",units[i]);
			arg.SetLocation(x+4,rc2.Bottom()-16);
			rp->DrawText(s.c_str(),arg);
		}

		rp->Lines(lines.data(),lines.size()/2,ColorAlpha(0x1f1f1f,0xff));

	}

	if (TRUE)
	{
		int cur=ruler.ToRS(dataProto->tView);
		rc2.Left()=cur-1;
		rc2.Right()=cur+2;
		rc2.Top()++;

		rp->FillRect(rc2,ColorAlpha(0x00ff00,0xff));

		FormatString(s,"{F:1}{S:14}{C:0,255,0}{Shd:2,4,4,0}{SC:0,0,0}%.3f 秒",dataProto->tView);

		DrawFontArg arg;
		arg.SetLocation(rc2.Left()+6,rc2.Top()+4);

		rp->DrawText(s.c_str(),arg);
	}


	return TRUE;
}

BOOL CGuiAgent_ViewTimeCtrl::OnLButtonDblClk(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);

	if(dataDebugger->context->IsRunning())
		return TRUE;

	i_math::recti rc,rc2;
	_GetClientRect(rc);
	rc.cutout(3,32,rc2);
	if (!rc2.isPointInside(x,y))
		return TRUE;

	_bLoop=TRUE;
	_tLoopStart=GetTickCount();
	_durLoop=_TimeFromX(x);

	return FALSE;
}

BOOL CGuiAgent_ViewTimeCtrl::OnTimer(int dt,DWORD flag)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);
	if(dataDebugger->context->IsRunning())
		return TRUE;

	if (!_bLoop)
		return TRUE;

	DWORD t=GetTickCount();
	if (t>_tLoopStart)
		t-=_tLoopStart;
	else
		t=0;
	float t2=((float)t)/1000.0f;
	t2=fmod(t2,_durLoop);

	dataProto->tView=t2;
	if (dataProto->tView<0.0f)
		dataProto->tView=0.0f;

	_Redraw(FALSE);
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_TestProto

BOOL CGuiAgent_TestProto::Respond(CtrlOp &op)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);
	GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");

	if (!dataDebugger->context->IsRunning())
		return CGuiAgent::Respond(op);

	if (dataDebugger->context->IsBreak())
		return FALSE;

	if (op.op==CtrlOp::Op_Timer)
	{
		_Redraw(TRUE);
	}
	else
	{
		BOOL bFiltered=FALSE;
// 		if ((op.op==CtrlOp::Op_Click)&&(op.vk==VK_RBUTTON))
// 			bFiltered=TRUE;
		if (op.op==CtrlOp::Op_Cmd)
			bFiltered=TRUE;
		if ((op.op==CtrlOp::Op_Down)&&((op.vk==VK_UP)||(op.vk==VK_DOWN))&&(op.flag&CtrlOpFlag_CtrlDown))
			bFiltered=TRUE;

		if (!bFiltered)
			dataDebugger->context->AddOp(op);//累积op
		else
			CGuiAgent::Respond(op);
	}

	return FALSE;
}

BOOL CGuiAgent_TestProto::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

	if (!dataDebugger->context->IsRunning())
	{
		_AddMenu("Enter Test Mode",ID_AGENT_EnterTestMode);

		if (!dataProto->IsReadOnly())
		{
			IProto *proto=dataProto->proto();
			if (proto&&dataProto->lib)
			{
				ProtoID id=dataProto->idGE;
				std::string s;
				if (id!=ProtoID_Null)
					FormatString(s,"_ge: %s",dataProto->lib->FindPath(id));
				else
					s="_ge: <None>";
				_AddMenu(s.c_str(),ID_AGENT_BrowseGE);

				id=dataProto->idGT;
				if (id!=ProtoID_Null)
					FormatString(s,"_gt: %s",dataProto->lib->FindPath(id));
				else
					s="_gt: <None>";
				_AddMenu(s.c_str(),ID_AGENT_BrowseGT);
			}
		}

		_AddMenuSep();

// 		if (dataViewSwitch)
// 		{
// 			if (dataViewSwitch->bShowProfiler)
// 				_AddMenu("View Profiler",ID_AGENT_ViewProfiler,MF_CHECKED);
// 			else
// 				_AddMenu("View Profiler",ID_AGENT_ViewProfiler);
// 		}

	}
// 	else
// 	{
// 		_AddMenu("Leave Test Mode",ID_AGENT_LeaveTestMode);
// 	}


	return TRUE;
}

BOOL CGuiAgent_TestProto::OnCommand(DWORD idCmd)
{
	if (idCmd==ID_AGENT_EnterTestMode)
	{
		Run();
		return FALSE;
	}
	if (idCmd==ID_AGENT_LeaveTestMode)
	{
		Stop();
		return FALSE;
	}

	if (idCmd==ID_AGENT_BrowseGE)
	{
		BrowseGlobal(TRUE);
		return FALSE;
	}
	if (idCmd==ID_AGENT_BrowseGT)
	{
		BrowseGlobal(FALSE);
		return FALSE;
	}

	if (idCmd==ID_AGENT_ViewProfiler)
	{
		GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
		if (dataViewSwitch)
			dataViewSwitch->bShowProfiler=!dataViewSwitch->bShowProfiler;
	}

	return TRUE;
}

void CGuiAgent_TestProto::Run()
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	if (dataProto)
	{
		if (dataProto->proto())
		{
			dataDebugger->context->Run(Break_PointCheck,dataProto->protoid,dataProto->idGE,dataProto->idGT,TRUE);
		}
	}
}

void CGuiAgent_TestProto::Stop()
{
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	if (dataDebugger)
		dataDebugger->context->RequestStop();
}

void CGuiAgent_TestProto::BrowseGlobal(BOOL bGE)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (!dataProto)
		return;
	if ((!dataProto->lib)||(!dataProto->proto()))
		return;

	CProtoSelectDlg dlg;
	dlg.SetProtoLib(dataProto->lib);
	dlg.ShowSelNone();//显示<None>的那个按钮
	if (bGE)
		dlg.SetNoneLuaProtoOnly();
	else
		dlg.SetLuaProtoOnly();


	if (IDCANCEL==dlg.DoModal())
		return;

	ProtoID id=dataProto->lib->FindProto(dlg.GetSelPath());

	if (bGE)
		dataProto->idGE=id;
	else
		dataProto->idGT=id;
}


BOOL CGuiAgent_TestProto::OnKeyDown(char c,DWORD flag)
{
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (c==VK_ESCAPE)
	{
		Stop();
	}
	if (c==VK_SPACE)
	{
		IClient *client=dataProto->pES->GetAS()->GetClient();
		if (dataProto->pES->GetAS()->GetSS()->bPaused)
			client->Resume(0xffffffff);
		else
			client->Pause();
	}
	if (dataDebugger)
	{
		if ((c==VK_UP)&&(flag&CtrlOpFlag_CtrlDown))
		{
			dataDebugger->context->acc++;
			if (dataDebugger->context->acc>3)
				dataDebugger->context->acc=3;
			return FALSE;
		}
		if ((c==VK_DOWN)&&(flag&CtrlOpFlag_CtrlDown))
		{
			dataDebugger->context->acc--;
			if (dataDebugger->context->acc<(MIN_DEBUG_ACC-1))
				dataDebugger->context->acc=(MIN_DEBUG_ACC-1);
			return FALSE;
		}
	}

	return TRUE;
}

void CGuiAgent_TestProto::OnAttachView(CGeView *view,DWORD iLevel)
{
}



void CGuiAgent_TestProto::OnDetachView(CGeView *view,DWORD iLevel)
{
	//Stop();
}

BOOL CGuiAgent_TestProto::OnDraw()
{
	std::string s;
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	if (dataDebugger)
	{
		if (dataDebugger->context->IsRunning())
		{
			int acc=dataDebugger->context->acc;
			if (acc==0)
				s="{O:1}{OC:0,0,0}~ 正常速度 ~";
			else
			{
				if (acc>0)
					FormatString(s,"{O:1}{OC:0,0,0}{C:0,255,0}~ 加速 x %d ~",1<<acc);
				else
				{
					if (acc>=MIN_DEBUG_ACC)
						FormatString(s,"{O:1}{OC:0,0,0}{C:255,128,128}~ 减速 x 1/%d ~",1<<(-acc));
					else
						FormatString(s,"{O:1}{OC:0,0,0}{C:255,128,128}~ 暂停 ~");
				}
			}

			i_math::recti rc;
			GetRP()->GetRect(rc);
			DrawFontArg arg;
			arg.SetLocation(4,rc.Bottom()-18);
			GetRP()->DrawText(s.c_str(),arg);
		}
	}

	if(TRUE)
	{
		extern CCurrentUserRegistry g_reg;
		s="{O:1}{OC:0,0,0}";
		s+=g_reg.ReadString("GameDebugDraw","DebugText");
		if (s.length()>500)
			s.resize(500);

		i_math::recti rc;
		GetRP()->GetRect(rc);
		DrawFontArg arg;
		arg.SetLocation(rc.Right()-300,rc.Top()+80);
		GetRP()->DrawText(s.c_str(),arg);
	}

	if (dataProto)
	{
		if (dataProto->pES->GetAS()->GetSS()->cfg->GetNumber("Engine.Helper.NeverShow")!=1)
		{
			extern CCurrentUserRegistry g_reg;
			s="{O:1}{OC:0,0,0}";

			BYTE *data;
			DWORD szData;
			if (g_reg.ReadData("GameDebugDraw","DebugText3D",(void *&)data,szData))
			{
				CDataPacket dp;
				dp.SetDataBufferPointer(data);

				DWORD c=dp.Data_NextDword();

				std::string ss;
				i_math::vector3df pos;

				DrawFontArg arg;

				i_math::recti rc;
				GetRP()->GetRect(rc);

				for (int i=0;i<c;i++)
				{
					dp.Data_ReadString(ss);
					dp.Data_ReadSimple(pos);

					int x,y;
					if (TRUE)
					{
						ICamera *cam=g_ssGuiLib.pES->GetAS()->GetSS()->adr->GetRecentCamera();
						cam->TransPos(pos);
						x=(int)((pos.x-(-1.0f))*(i_math::f32)rc.getWidth()/2.0f);
						y=rc.getHeight()-(int)((pos.y-(-1.0f))*(i_math::f32)rc.getHeight()/2.0f);
					}

					arg.SetLocation(x,y);

					ss=s+ss;
					GetRP()->DrawText(ss.c_str(),arg);
				}
			}

		}
	}


	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ThumbnailMake
BOOL CGuiAgent_ThumbnailMake::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");

	if (!_bDumping)
	{
		_AddMenu("Make Thumbnail",ID_AGENT_MAKETHUMBNAIL);
		std::string pathThumbnail=_GetThumbnailPath();
		if (!pathThumbnail.empty())
		{
			if (dataProto->pES->GetWS()->GetFS()->ExistFileAbs(pathThumbnail.c_str()))
				_AddMenu("Clear Thumbnail",ID_AGENT_CLEARTHUMBNAIL);
		}
		_AddMenuSep();
	}

	return TRUE;
}

BOOL CGuiAgent_ThumbnailMake::OnCommand(DWORD idCmd)
{
	if (idCmd==ID_AGENT_MAKETHUMBNAIL)
	{
		GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
		if (dataProto)
		{
			if (!_bDumping)
			{
				std::string path=dataProto->pES->GetWS()->GetPath(WSPath_ProtoLib);
				path+="\\__thumbnaildump__.tga";
				if (dataProto->pES->GetAS()->GetRenderer()->BeginDump(path.c_str()))
					_bDumping=TRUE;
			}
		}

		return FALSE;
	}

	if (idCmd==ID_AGENT_CLEARTHUMBNAIL)
	{
		GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
		if (dataProto)
		{
			if (!_bDumping)
			{
				std::string pathThumbnail=_GetThumbnailPath();
				if (!pathThumbnail.empty())
					dataProto->pES->GetWS()->GetFS()->RemoveFileAbs(pathThumbnail.c_str());
			}
		}

		return FALSE;
	}

	return TRUE;
}

const char *CGuiAgent_ThumbnailMake::_GetThumbnailPath()
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (dataProto)
	{
		_pathThumbnailTemp=dataProto->lib->MakeProtoPath(dataProto->pathProtoFile.c_str());
		RemoveFileSuffix(_pathThumbnailTemp);
		MakeFileSuffix(_pathThumbnailTemp,"tbn");
		return _pathThumbnailTemp.c_str();
	}
	return "";
}

BOOL CGuiAgent_ThumbnailMake::OnTimer(int dt,DWORD flag)
{
	if (_bDumping)
	{
		GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
		if (dataProto)
		{
			AResult result=dataProto->pES->GetAS()->GetRenderer()->CheckDump();
			if (result!=A_Pending)
			{
				dataProto->pES->GetAS()->GetRenderer()->EndDump();
				_bDumping=FALSE;

				if (result==A_Ok)
				{
					std::string path=dataProto->pES->GetWS()->GetPath(WSPath_ProtoLib);
					path+="\\__thumbnaildump__.tga";

					CxImage img;
					if (img.Load(fromMBCS(path.c_str())))
					{
						float w=(float)img.GetWidth();
						float h=(float)img.GetHeight();

						const float wT=ProtoThumbnailWidth,hT=ProtoThumbnailHeight;

						if (w/h>wT/hT)
						{
							w=w*hT/h;
							h=hT;
						}
						else
						{
							h=h*wT/w;
							w=wT;
						}

						int ww,hh,wwT,hhT;
						ww=FloatToNearestInt(w);
						hh=FloatToNearestInt(h);
						wwT=FloatToNearestInt(wT);
						hhT=FloatToNearestInt(hT);

						img.Resample2(ww,hh);
						int l,t,r,b;
						if (ww<=wwT)
							l=0;
						else
							l=(ww-wwT)/2;
						r=l+wwT;
						if (hh<=hhT)
							t=0;
						else
							t=(hh-hhT)/2;
						b=t+hhT;

						img.Crop(l,t,r,b);

						img.Save(fromMBCS(_GetThumbnailPath()), CXIMAGE_FORMAT_TGA);
					}
				}

			}
		}

	}

	return TRUE;

}


void CGuiAgent_ThumbnailMake::OnAttachView(CGeView *view,DWORD iLevel)
{
}



void CGuiAgent_ThumbnailMake::OnDetachView(CGeView *view,DWORD iLevel)
{
	//Stop();
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphScroll
void CGuiAgent_GraphScroll::OnUpdateTransform(const i_math::pos2df & pos,const i_math::pos2df &scale)
{
	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	if (!dataProto)
		return;

	dataProto->xlate.set((int)pos.x,(int)pos.y);
	dataProto->scale=scale.x;
}





//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphNodeSel
void CGuiAgent_GraphNodeSel::_SelectProtoNode(ProtoNodeID id)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	if (id==ProtoNodeID_Null)
		data->sels.clear();
	else
	{
		int idx;
		VEC_FIND(data->sels,id,idx);
		if (idx==-1)
		{
			data->sels.resize(1);
			data->sels[0]=id;
			_Redraw(FALSE);
		}
	}
}


BOOL CGuiAgent_GraphNodeSel::OnBeginDrag(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	_sels.clear();
	_starts.clear();

	_nameExpose="";

	GraphHit hit;
	BOOL bHit=logic->graph.HitTest(x,y,hit);
	_SelectProtoNode(hit.id);
	_Redraw(FALSE);
	if (bHit)
	{
		if (hit.id!=ProtoNodeID_Null)
		{

			if (hit.part==GraphHit::Blank)
			{
				_sels=data->sels;
				_starts.resize(data->sels.size());
				for (int i=0;i<data->sels.size();i++)
					_starts[i]=data->proto()->GetNode(_sels[i])->GetGraphPos();

				_pt.set(x,y);

				data->bChanging=TRUE;//在拖动过程中,禁止auto save

				return TRUE;
			}
		}

		if (hit.nameExpose!="")
		{
			ProtoStubInfo info;
			if (data->proto()->FindStubInfo(hit.nameExpose.c_str(),info))
			{
				_nameExpose=hit.nameExpose;
				_startExpose=info.pos;
				_pt.set(x,y);

				data->bChanging=TRUE;//在拖动过程中,禁止auto save
				return TRUE;
			}
		}


	}

	return FALSE;
}

void CGuiAgent_GraphNodeSel::OnDrag(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	_ScreenToGG(x,y);

	assert(data->proto());

	i_math::pos2di off(x,y);
	off-=_pt;

	if (_sels.size())
	{
		for (int i=0;i<_sels.size();i++)
		{
			IProtoNode *node=data->proto()->GetNode(_sels[i]);
			if (node)
				node->SetGraphPos(_starts[i]+off);
		}
	}
	else
	{
		if (_nameExpose!="")
		{
			ProtoStubInfo info;
			if (data->proto()->FindStubInfo(_nameExpose.c_str(),info))
			{
				info.pos=_startExpose+off;
				data->proto()->AddStub(info);
			}
		}
	}

	_Redraw(TRUE);

}


void CGuiAgent_GraphNodeSel::OnEndDrag(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");

	if (data)
		data->bChanging=FALSE;//拖动结束,允许自动save



}

BOOL CGuiAgent_GraphNodeSel::OnRButtonDown(int x,int y,DWORD flag)
{
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");

	if (data->sels.size()>0)
		return TRUE;//对于右键来说,如果原来有选中的东西,我们什么也不能改变

	_ScreenToGG(x,y);

	GraphHit hit;
	logic->graph.HitTest(x,y,hit);
	_SelectProtoNode(hit.id);
	_Redraw(FALSE);

	return TRUE;
}

BOOL CGuiAgent_GraphNodeSel::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");

	_ScreenToGG(x,y);

	GraphHit hit;
	logic->graph.HitTest(x,y,hit);
	_SelectProtoNode(hit.id);
	_Redraw(FALSE);

	return TRUE;

}



BOOL CGuiAgent_GraphNodeSel::OnTimer(int dt,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");

	i_math::pos2di ptCursor;
	_GetCursorPos(ptCursor);

	_ScreenToGG(ptCursor.x,ptCursor.y);

	if (_IsInMenu())
		return TRUE;

	GraphHit hit;
	BOOL bChanged;
	if (logic->graph.HitTest(ptCursor.x,ptCursor.y,hit))
		bChanged=logic->graph.SetFocusItem(hit.item);
	else
		bChanged=logic->graph.SetFocusItem(NULL);

	if (bChanged)
		_Redraw(FALSE);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphNodeRectSel

void CGuiAgent_GraphNodeRectSel::_Sel(ProtoNodeID *inrects,DWORD c)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");

	std::vector<ProtoNodeID>sels;
	data->sels=_initials;

	for (int i=0;i<c;i++)
	{
		ProtoNodeID id=inrects[i];

		int idx;
		VEC_FIND(data->sels,id,idx);
		if (idx==-1)
		{//原来没有,我们添加
			data->sels.push_back(id);
		}
		else
		{//原来有的,我们删除
			data->sels.erase(data->sels.begin()+idx);
		}
	}
}


BOOL CGuiAgent_GraphNodeRectSel::OnBeginDrag(int x,int y,DWORD flag)
{

	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	if (TRUE)
	{
		GraphHit hit;
		BOOL bHit=logic->graph.HitTest(x,y,hit);

		if (bHit)
			return FALSE;
	}

	if (flag&CtrlOpFlag_CtrlDown)
		_initials=data->sels;
	else
		_initials.clear();

	_start.set(x,y);

	return TRUE;
}

void CGuiAgent_GraphNodeRectSel::OnDrag(int x,int y,DWORD flag)
{
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	i_math::recti rc;
	rc.set(_start.x,_start.y,x,y);
	rc.repair();

	DWORD c;
	ProtoNodeID *ids=logic->graph.RectHitTest(rc,c);

	_Sel(ids,c);

	_rcDraw=rc;

	_Redraw(TRUE);
}


void CGuiAgent_GraphNodeRectSel::OnEndDrag(int x,int y,DWORD flag)
{
	OnDrag(x,y,flag);
}

BOOL CGuiAgent_GraphNodeRectSel::OnDraw()
{
	if (!_bInDrag)
		return TRUE;
	GraphicsGraph *gg=GetGG();

	gg->DrawFrameRect(_rcDraw,0x00ff00,1,128);
	return TRUE;
}



void Popup_EditExposeName(CGuiAgent *agent,const char *name)
{
	GuiData_Proto*data=(GuiData_Proto*)agent->FindData("proto");

	ProtoStubInfo info;
	if (data->proto()->FindStubInfo(name,info))
	{
		i_math::pos2di pt=info.pos;
		agent->_GGToScreen(pt.x,pt.y);

		agent->GetWnd()->ClientToScreen((CPoint*)&pt);

		CEditPopup popup;
		while(1)
		{
			info.name=popup.Popup(pt.x,pt.y,name);
			if (data->proto()->AddStub(info))
				break;
		}
	}
}

const char *Popup_NewStub(IProtoNode *node,GStubType type,const char *nameInitial)
{
	static std::string name;
	name="";
	StubArg arg;
	arg.sem=GSem_Unknown;
	arg.type=type;
	switch(type)
	{
	case GStub_Property:
		arg.name="NewProperty";
		break;
	case GStub_Signal:
		arg.name="NewSignal";
		break;
	case GStub_Slot:
		arg.name="NewSlot";
		break;
	case GStub_Call:
		arg.name="NewFunc";
	}

	if (nameInitial!=NULL)
		arg.name=nameInitial;

	while(node->FindStub(arg.name.c_str()))
		IncreaseTailOrdinal(arg.name,3);

	CStubPopup popup;
	if (popup.Popup(&arg))
	{
		node->AddStub(arg);
		name=arg.name;
	}

	return name.c_str();
}

void Popup_EditStub(IProtoLib *lib,IProtoNode *node,const char *nameStub)
{
	GStubBase *stb=node->FindStub(nameStub);
	if (!stb)
		return;

	StubArg arg;
	arg.name=stb->name;
	arg.sem=stb->sem.code;
	arg.constaint=stb->sem.constraint;
	arg.type=stb->type;
	arg.desc=stb->desc;
	arg.bConnectable=stb->IsConnectable();

	arg.nameGVT="未知类型";
	GProperty *prop=stb->GetDefVal();
	if (prop->IsSuperb())
		arg.nameGVT="";//百搭
	else
	{
		GVarType gvt=prop->GetGVT();
		if (gvt<GVT_Max)
			arg.nameGVT=NameFromVarType(gvt);
		else
		{
			CLud *lud=lib->GetLuaMachine()->Find(gvt);
			if (lud)
				arg.nameGVT=lud->GetName();
		}
	}

	CStubPopup popup;
	if (popup.Popup(&arg))
	{
		node->ChangeStub(nameStub,arg);
	}

}





//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphNodeConnect
BOOL CGuiAgent_GraphNodeConnect::OnBeginDrag(int x,int y,DWORD flag)
{
	if (!_AgentCanOp(this))
		return FALSE;

	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	assert(data->proto());

	ConnectDynPG conn;
	GraphHit hit;
	if (logic->graph.HitTest(x,y,hit))
	{
		switch(hit.part)
		{
		case GraphHit::Signal:
		case GraphHit::PropOut:
			{
				conn.type=ConnectDynPG::Connecting;
				conn.AddItem(hit.item);
				conn.pt.set(x,y);
				break;
			}
		case GraphHit::Call:
		case GraphHit::Slot:
		case GraphHit::PropIn:
			{
				DWORD n;
				GraphItem **items=logic->graph.GetConnects(hit.item,FALSE,n);
				if (flag&CtrlOpFlag_CtrlDown)
					n=0;
				if (n>0)
				{
					PNConnect c;
					c.id[1]=hit.id;
					c.name[1]=hit.item->name.c_str();
					data->proto()->RemoveConnect(c);//删除所有连到这个stub上的连接
					for (int i=0;i<n;i++)
						conn.AddItem(items[i]);
					conn.pt=hit.item->GetConnectSpot(FALSE);
					conn.type=ConnectDynPG::Connecting;
				}
				else
				{
					conn.AddItem(hit.item);
					conn.pt.set(x,y);
					conn.type=ConnectDynPG::Connected;
				}
				if (hit.part==GraphHit::Call)
					conn.type=ConnectDynPG::Void;
				break;
			}
		default:
			break;
		}
	}

	if (conn.IsEmpty())
		return FALSE;

	_conn=conn;
	logic->graph.SetFocusConnect(_conn);

	data->bChanging=TRUE;//在拖动过程中,禁止auto save

	_Redraw(FALSE);

	return TRUE;
}


void CGuiAgent_GraphNodeConnect::OnDrag(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	_conn.pt.set(x,y);
	GraphHit hit;
	if (logic->graph.HitTest(x,y,hit))
	{
		if (hit.item)
		{
			if (_conn.type==ConnectDynPG::Connecting)
			{
				if ((hit.part==GraphHit::Slot)||(hit.part==GraphHit::PropIn))
					_conn.pt=hit.item->GetConnectSpot(FALSE);
			}
			if (_conn.type==ConnectDynPG::Connected)
			{
				if ((hit.part==GraphHit::Signal)||(hit.part==GraphHit::PropOut))
					_conn.pt=hit.item->GetConnectSpot(TRUE);
			}
		}
	}

	logic->graph.SetFocusConnect(_conn);

	_Redraw(FALSE);
}

void CGuiAgent_GraphNodeConnect::OnEndDrag(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;
	_ScreenToGG(x,y);

	GraphItem *item=NULL;
	GraphHit hit;
	if (logic->graph.HitTest(x,y,hit))
	{
		if (hit.item)
		{
			if (_conn.type==ConnectDynPG::Connecting)
			{
				if ((hit.part==GraphHit::Slot)||(hit.part==GraphHit::PropIn))
					item=hit.item;
				if (hit.part==GraphHit::CreateSlot)
				{
					if (_conn.items.size()==1)
					{
						std::string name="On";
						name=name+_conn.items[0]->name;

						DiscardFocus(OpType_Mouse);
						IProtoNode *node=data->proto()->GetNode(hit.item->id);
						name=Popup_NewStub(node,GStub_Slot,name.c_str());

						if (name!="")
						{
							PNConnect c;
							c.id[0]=_conn.items[0]->id;
							c.name[0]=_conn.items[0]->name.c_str();
							c.id[1]=hit.item->id;
							c.name[1]=name.c_str();

							data->proto()->AddConnect(c);

							proxy->AddLuaSrcFunc(data->proto()->GetID(),hit.item->id,name.c_str());
						}
					}
				}
			}
			if (_conn.type==ConnectDynPG::Connected)
			{
				if ((hit.part==GraphHit::Signal)||(hit.part==GraphHit::PropOut))
					item=hit.item;
			}
		}
	}
	else
	{
		if (flag&CtrlOpFlag_CtrlDown)
		{//在这里创建一个新的expose
			if (_conn.items.size()==1)
			{
				GraphItem *item=_conn.items[0];

				if (TRUE)
				{
					IProtoNode *node=data->proto()->GetNode(item->id);
					if (node->IsEditHelper()||node->IsVirtual()||node->IsDynamic()||node->GetDeferGrp()!=PNDeferGrp_None)
						AfxMessageBox(_T("Disable,Dynamic,Edit Helper或者属于某个延后组的node不能向外暴露接口!"), MB_OK);
				}
				ProtoStubInfo info;

				info.idInner=item->id;
				info.nameInner=item->name.c_str();
				info.pos.set(x,y);
				std::string name=info.nameInner;
				info.name=name.c_str();

				while(!data->proto()->AddStub(info))
				{
					if (FALSE==IncreaseTailOrdinal(name,3))
						break;
					info.name=name.c_str();
				}

				logic->graph.ClearFocusConnect();
				_Redraw(FALSE);
				_SetCursor(IDC_NORMAL,TRUE);
				Popup_EditExposeName(this,info.name);
			}
		}
	}

	if ((item)&&(!_conn.IsEmpty()))
	{
		PNConnect c;
		assert((_conn.type==ConnectDynPG::Connecting)||(_conn.type==ConnectDynPG::Connected));
		DWORD idx=(_conn.type==ConnectDynPG::Connecting)?0:1;
		for(int i=0;i<_conn.items.size();i++)
		{
			c.id[idx]=_conn.items[i]->id;
			c.name[idx]=_conn.items[i]->name.c_str();
			c.id[1-idx]=item->id;
			c.name[1-idx]=item->name.c_str();
			data->proto()->AddConnect(c);
		}

	}

	logic->graph.ClearFocusConnect();

	data->bChanging=FALSE;

	_Redraw(FALSE);
}


BOOL CGuiAgent_GraphNodeConnect::OnSetCursor(int x,int y,DWORD flag)
{
	if (_bInDrag)
	{
		if (flag&CtrlOpFlag_CtrlDown)
		{
			_SetCursor(IDC_POINTER_COPY);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CGuiAgent_GraphNodeConnect::OnTimer(int dt,DWORD flag)
{
	if (_bInDrag)
	{
		if (flag&CtrlOpFlag_CtrlDown)
			_SetCursor(IDC_POINTER_COPY);
		else
			_SetCursor(IDC_NORMAL);
	}
	else
	{
	}


	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphNodeCommand
#define SHRINKSTUB_COMMAND_START 45000
#define SHRINKSTUB_COMMAND_END 50000
#define ID_REMOVEEXPOSE 41000
#define ID_RENAMEEXPOSE 41001
#define ID_VIEWSTUB 41002
#define ID_EDITSTUB 41003
#define ID_REMOVESTUB 41004
#define ID_GOTOSTUBSOURCE 41006
#define ID_NEWPROP 41007
#define ID_NEWSIGNAL 41008
#define ID_NEWSLOT 41009
#define ID_NEWCALL 41010
#define ID_MOVESTUB_UP 41011
#define ID_MOVESTUB_DOWN 41012


void CGuiAgent_GraphNodeCommand::_ClearCache()
{
	_shrinknames.clear();;
	_nameExpose="";
	_nodeid=ProtoNodeID_Null;

}

BOOL CGuiAgent_GraphNodeCommand::OnLButtonDown(int x,int y,DWORD flag)
{
	if (!_AgentCanOp(this))
		return FALSE;

	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	_ScreenToGG(x,y);

	assert(data->proto());

	GraphHit hit;
	if (logic->graph.HitTest(x,y,hit))
	{
		if (hit.id!=ProtoNodeID_Null)
		{
			switch(hit.part)
			{
			case GraphHit::MoreProp:
			case GraphHit::MoreSignal:
			case GraphHit::MoreSlot:
			case GraphHit::MoreCall:
				{
					IProtoNode *node=data->proto()->GetNode(hit.id);
					if (node)
					{
						GStubType type;
						switch(hit.part)
						{
						case GraphHit::MoreProp:
							type=GStub_Property;
							break;
						case GraphHit::MoreSignal:
							type=GStub_Signal;
							break;
						case GraphHit::MoreSlot:
							type=GStub_Slot;
							break;
						case GraphHit::MoreCall:
							type=GStub_Call;
							break;
						}

						_nodeid=hit.id;
						_shrinknames.clear();

						for (int i=0;i<node->GetStubCount();i++)
						{
							GStubBase *stb;
							const char *name;
							stb=node->GetStub(i,name);
							if (stb->type!=type)
								continue;
							if (!node->StubCanHide(name))
								continue;
							_shrinknames.push_back(std::string(name));
						}

						std::string hides=node->GetGraphStubHides();
						std::vector<std::string>buf;
						SplitStringBy(",",hides,&buf);

						for(int i=0;i<_shrinknames.size();i++)
						{
							std::string name=_shrinknames[i];
							int idx;
							VEC_FIND(buf,name,idx);
							if (idx==-1)
								_AddMenu(name.c_str(),SHRINKSTUB_COMMAND_START+i,MF_CHECKED|MF_ENABLED);
							else
								_AddMenu(name.c_str(),SHRINKSTUB_COMMAND_START+i,MF_UNCHECKED|MF_ENABLED);
						}

						return FALSE;
					}

					break;
				}

			case GraphHit::Shrink:
				{
					IProtoNode *node=data->proto()->GetNode(hit.id);
					if (node)
					{
						if (!node->ShrinkStub(TRUE))
							node->ExpandStub(FALSE);
						else
							node->ShrinkStub(FALSE);
						_Redraw(FALSE);
						return FALSE;
					}
					break;
				}
			default:
				break;
			}
		}
	}

	return TRUE;

}

BOOL CGuiAgent_GraphNodeCommand::OnRButtonClick(int x,int y,DWORD flag)
{
	BOOL bCanOp=_AgentCanOp(this);

	if (bCanOp)
	{
		GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
		GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
		_ScreenToGG(x,y);

		_ptClick.set(x,y);

		assert(data->proto());

		GraphHit hit;
		if (logic->graph.HitTest(x,y,hit))
		{
			if (hit.nameExpose!="")
			{
				_AddMenu("Remove",ID_REMOVEEXPOSE);
				_AddMenu("Rename",ID_RENAMEEXPOSE);
				_nameExpose=hit.nameExpose;
				_nodeid=hit.id;
				return FALSE;
			}

			if (hit.id!=ProtoNodeID_Null)
			{
				_nodeid=hit.id;
				if (hit.part==GraphHit::Blank)
				{
					CGuiActor_Proto *owner=(CGuiActor_Proto *)_GetActor();
					owner->BuildProtoTreeMenu(_GetMenu());
					return FALSE;
				}

				IProtoNode *node=data->proto()->GetNode(hit.id);
				BOOL bScript=(node->GetType()==PN_LuaObj);

				switch(hit.part)
				{
				case GraphHit::PropIn:
				case GraphHit::PropOut:
				case GraphHit::Signal:
				case GraphHit::Slot:
				case GraphHit::Call:
					{
						_nameStub=hit.item->name;
						if (bScript)
						{
							_AddMenu("Edit...",ID_EDITSTUB);
							_AddMenu("Remove",ID_REMOVESTUB);
							_AddMenu("Goto script codes",ID_GOTOSTUBSOURCE);
							_AddMenuSep();
							_AddMenu("Move up",ID_MOVESTUB_UP);
							_AddMenu("Move down",ID_MOVESTUB_DOWN);

						}
						else
							_AddMenu("View Info...",ID_EDITSTUB);
						break;
					}

				case GraphHit::CreateProp:
					_AddMenu("New Property...",ID_NEWPROP);
					break;
				case GraphHit::CreateSignal:
					_AddMenu("New Signal...",ID_NEWSIGNAL);
					break;
				case GraphHit::CreateSlot:
					_AddMenu("New Slot...",ID_NEWSLOT);
					break;
				case GraphHit::CreateCall:
					_AddMenu("New Func...",ID_NEWCALL);
					break;
				}

				return FALSE;
			}
		}
	}

	CGuiActor_Proto *owner=(CGuiActor_Proto *)_GetActor();
	owner->BuildProtoTreeMenu(_GetMenu());


	return FALSE;
}


BOOL CGuiAgent_GraphNodeCommand::OnCommand(DWORD idCmd)
{
// 	if (!_AgentCanOp(this))
// 		return FALSE;

	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;


	BOOL bNodeTreeCmd=FALSE;
	switch(idCmd)
	{
	case ID_COMMON_DELETE:
	case ID_COMMON_RENAME:
		bNodeTreeCmd=TRUE;
	}
	if ((idCmd>=ID_COMMON_NEW_START)&&(idCmd<ID_COMMON_NEW_END))
		bNodeTreeCmd=TRUE;
	if ((idCmd>=ID_NODETREE_CUSTOM_START)&&(idCmd<ID_NODETREE_CUSTOM_END))
		bNodeTreeCmd=TRUE;

	if (bNodeTreeCmd)
	{
		CGuiActor_Proto *owner=(CGuiActor_Proto *)_GetActor();

		if (data->proto())
			data->proto()->SetNextNodePos(_ptClick);//为新建的proto node设置位置
		owner->SendProtoTreeCmd(idCmd);
		if (data->proto())
			data->proto()->SetNextNodePos(i_math::pos2di(-10000,-10000));//恢复
		_Redraw(FALSE);
		return FALSE;
	}


	if (idCmd==ID_REMOVEEXPOSE)
	{
		if (IDYES == AfxMessageBox(_T("警告:删除外连接口将会导致相关的外部连接被清除,确认要删除吗?"), MB_YESNO))
			data->proto()->RemoveStub(_nameExpose.c_str());
		return FALSE;
	}

	if (idCmd==ID_RENAMEEXPOSE)
	{
		if (IDYES==AfxMessageBox(_T("警告:修改外连接口的名称将会导致相关的外部连接被清除,确认要继续吗?"),MB_YESNO))
			Popup_EditExposeName(this,_nameExpose.c_str());
		return FALSE;
	}

	//Now the command on proto node
	IProtoNode *node=data->proto()->GetNode(_nodeid);

	if (TRUE)
	{
		if(idCmd==ID_NEWPROP)
			_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Property,NULL),GStub_Property);
		if(idCmd==ID_NEWSIGNAL)
			_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Signal,NULL),GStub_Signal);
		if(idCmd==ID_NEWSLOT)
			_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Slot,NULL),GStub_Slot);
		if(idCmd==ID_NEWCALL)
			_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Call,NULL),GStub_Call);
	}

	if ((idCmd>=SHRINKSTUB_COMMAND_START)&&(idCmd<SHRINKSTUB_COMMAND_END))
	{
		DWORD idx=idCmd-SHRINKSTUB_COMMAND_START;
		if (idx>=_shrinknames.size())
			return TRUE;

		if (node)
		{
			if (node->IsStubHide(_shrinknames[idx].c_str()))
				node->HideStub(_shrinknames[idx].c_str(),FALSE);//show
			else
				node->HideStub(_shrinknames[idx].c_str(),TRUE);//show
		}
	}


	if (idCmd==ID_REMOVESTUB)
		node->RemoveStub(_nameStub.c_str());

	if (idCmd==ID_EDITSTUB)
		Popup_EditStub(data->lib,node,_nameStub.c_str());

	if (idCmd==ID_GOTOSTUBSOURCE)
		proxy->GotoLuaSrc(data->protoid,node->GetID(),0);

	if (idCmd==ID_MOVESTUB_DOWN)
		node->MoveStub(_nameStub.c_str(),FALSE);

	if (idCmd==ID_MOVESTUB_UP)
		node->MoveStub(_nameStub.c_str(),TRUE);

	_Redraw(FALSE);

	return FALSE;


}

void CGuiAgent_GraphNodeCommand::_AddLuaSrcFunc(IProtoNode *node,const char *name,int tp)
{
	if (name[0]==0)
		return;
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	ProtoID protoid=data->proto()->GetID();
	ProtoNodeID nodeid=node->GetID();

	std::string s;
	if (tp==GStub_Property)
	{
		s=std::string("set_")+name;
		proxy->AddLuaSrcFunc(protoid,nodeid,s.c_str());
		s=std::string("get_")+name;
		proxy->AddLuaSrcFunc(protoid,nodeid,s.c_str());
	}
	else
		proxy->AddLuaSrcFunc(protoid,nodeid,name);

}



BOOL CGuiAgent_GraphNodeCommand::OnLButtonDblClk(int x,int y,DWORD flag)
{
	GuiData_Proto*data=(GuiData_Proto*)FindData("proto");
	GuiData_ProtoLogic *logic=(GuiData_ProtoLogic *)FindData("proto_logic");
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

	BOOL bCanOp=_AgentCanOp(this);

	_ScreenToGG(x,y);

	GraphHit hit;
	if (logic->graph.HitTest(x,y,hit))
	{
		if (hit.id!=ProtoNodeID_Null)
		{
			IProtoNode *node=data->proto()->GetNode(hit.id);
			switch(hit.part)
			{
			case GraphHit::CreateProp:
				if (bCanOp)
					_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Property,NULL),GStub_Property);
				return FALSE;
			case GraphHit::CreateSignal:
				if (bCanOp)
					_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Signal,NULL),GStub_Signal);
				return FALSE;
			case GraphHit::CreateSlot:
				if (bCanOp)
					_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Slot,NULL),GStub_Slot);
				return FALSE;
			case GraphHit::CreateCall:
				if (bCanOp)
					_AddLuaSrcFunc(node,Popup_NewStub(node,GStub_Call,NULL),GStub_Call);
				return FALSE;
			case GraphHit::PropIn:
			case GraphHit::PropOut:
			case GraphHit::Signal:
			case GraphHit::Slot:
			case GraphHit::Call:
				{
					if (node->GetType()==PN_LuaObj)
					{
						std::string name=hit.item->name;
						if ((hit.part==GraphHit::PropIn)||(hit.part==GraphHit::PropOut))
							name=std::string("set_")+name;
						int iLine=proxy->FindLuaSrcFunc(data->protoid,node->GetID(),name.c_str());
						if (iLine<0)
							iLine=0;
						proxy->GotoLuaSrc(data->protoid,node->GetID(),iLine);
					}
					return FALSE;
				}
			case GraphHit::Blank:
				{
					if (node->GetType()==PN_LuaObj)
						proxy->GotoLuaSrc(data->protoid,node->GetID(),0);
					if (node->GetType()==PN_Entity)
						proxy->GotoLogic(node->GetProtoID());
				}

			}
		}
		if (bCanOp)
		if (hit.nameExpose!="")
		{
			Popup_EditExposeName(this,hit.nameExpose.c_str());
			return FALSE;
		}
	}

	//尝试删除connect
	if (bCanOp)
	{
		if (data->proto())
		{
			PNConnect conn;
			if (logic->graph.ConnectHitTest(x,y,conn))
			{
				data->proto()->RemoveConnect(conn);
				_Redraw(FALSE);
			}
		}
	}
	
	return TRUE;
}
