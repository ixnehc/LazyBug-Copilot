/********************************************************************
	created:	14:4:2010   12:45
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	edit panel for AnimTree
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IAnim.h"
#include "RenderSystem/IAnimTree.h"
#include "RenderSystem/IFont.h"


#include ".\dummieseditpanel.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IAnimTreeCtrl.h"


#include "RenderPortBase.h"
#include "../Common/resdata/AnimTreePads.h"
#include "WndBase.h"
#include "GuiEditor_res.h"
#include "GuiAgent_general.h"
#include "matrixedit_base.h"

#include "AnimTreeEditPanel.h"

#include "EditPopup.h"

#include "avtrstates/avtrstates.h"

//////////////////////////////////////////////////////////////////////////
//Reps_AnimTree
CGraphATPads *Reps_AnimTree::GetGraph()
{
	if (panel)
		return ((CAnimTreeEditPanel*)panel)->GetGraph();
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//CDataSrc_GraphATPads
CGraphPads *CDataSrc_GraphATPads::GetGraph(CGuiAgent *agent)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_AnimTree *state=(Reps_AnimTree *)data->GetState();
		if (state)
			return state->GetGraph();
	}
	return NULL;
}

std::vector<PadID>*CDataSrc_GraphATPads::GetSelBuf(CGuiAgent *agent)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_AnimTree *state=(Reps_AnimTree *)data->GetState();
		if (state)
			return &state->sels;
	}
	return NULL;
}

void CDataSrc_GraphATPads::NotifyChange(CGuiAgent *agent,BOOL bSave)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_AnimTree *state=(Reps_AnimTree *)data->GetState();
		state->panel->RefreshStateMod(bSave);
	}
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_NewATPads
CGuiAgent_NewATPads::CGuiAgent_NewATPads()
{
	_classes.push_back(std::string("CAtpRoot"));
	_classes.push_back(std::string("CAtpSequence"));
	_classes.push_back(std::string("CAtpSequenceWH"));
	_classes.push_back(std::string("CAtpSequenceSD"));
	_classes.push_back(std::string("CAtpSequenceTSD"));
	_classes.push_back(std::string("CAtpSequenceST"));

	_classes.push_back(std::string("CAtpFloatST"));

	_classes.push_back(std::string("CAtpPath"));
	_classes.push_back(std::string("CAtpPathST"));

	_classes.push_back(std::string("CAtpBlend"));
	_classes.push_back(std::string("CAtpBlendX"));
	_classes.push_back(std::string("CAtpSpeedBlend"));
	_classes.push_back(std::string("CAtpShiftBlend"));
	_classes.push_back(std::string("CAtpMoveRotBlend"));
	_classes.push_back(std::string("CAtpMoveStartRotBlend"));
	_classes.push_back(std::string("CAtpRotOnSpotBlend"));

	_classes.push_back(std::string("CAtpSwitch"));
	_classes.push_back(std::string("CAtpSwitch_TunerString"));
	_classes.push_back(std::string("CAtpSwitch_Move"));
	_classes.push_back(std::string("CAtpSwitch_Fly"));
	_classes.push_back(std::string("CAtpSwitch_Jump"));
	_classes.push_back(std::string("CAtpSwitch_Turn"));
	_classes.push_back(std::string("CAtpSwitch_Slide"));
	_classes.push_back(std::string("CAtpSwitch_Sit"));
	_classes.push_back(std::string("CAtpSwitch_KO"));
	_classes.push_back(std::string("CAtpSwitch_Posture"));
	_classes.push_back(std::string("CAtpSwitch_PostureTrans"));
	_classes.push_back(std::string("CAtpSwitch_Act"));
	_classes.push_back(std::string("CAtpSwitch_ActSub"));
	_classes.push_back(std::string("CAtpSwitch_Auto"));
	_classes.push_back(std::string("CAtpSwitch_AutoX"));
	_classes.push_back(std::string("CAtpSwitch_AvtrLoco"));

	_classes.push_back(std::string("CAtpPartialBlend"));

	_classes.push_back(std::string("CAtpPartialSwitch"));
	_classes.push_back(std::string("CAtpPartialSwitch_Auto"));

	_classes.push_back(std::string("CAtpCombo_Act"));
	_classes.push_back(std::string("CAtpBoneCtrl"));
	_classes.push_back(std::string("CAtpBoneCtrlMerge"));
	_classes.push_back(std::string("CAtpBoneCtrlBlend"));
	_classes.push_back(std::string("CAtpBoneCtrlChainStretch"));
	_classes.push_back(std::string("CAtpBoneCtrlEel"));
	_classes.push_back(std::string("CAtpIKCtrl_Chain"));
	_classes.push_back(std::string("CAtpIKCtrl_Simple"));
	_classes.push_back(std::string("CAtpIKCtrl_Custom"));

	//XXXXX:more AnimTreePad
}

BOOL CGuiAgent_NewATPads::OnRButtonClick(int x,int y,DWORD flag)
{
	std::string s;
	_PushMenu("新增");
	for (int i=0;i<_classes.size();i++)
	{
		CAnimTreePad *p=(CAnimTreePad *)CClass::New(_classes[i].c_str());
		if (!p)
			continue;
		s=p->GetTypeName();
		Class_Delete(p);

		_AddMenu(s.c_str(),ID_AGENT_NEW_ATPAD_BEGIN+i);
	}
	_PopMenu();
	_AddMenuSep();
	_ScreenToGG(x,y);
	_pt.set(x,y);
	return TRUE;
}

BOOL CGuiAgent_NewATPads::OnCommand(DWORD idCmd)
{
	if ((idCmd>=ID_AGENT_NEW_ATPAD_BEGIN)&&(idCmd<ID_AGENT_NEW_ATPAD_END))
	{
		CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
		Reps_AnimTree *state=(Reps_AnimTree *)data->GetState();
		if (state)
		{
			CGraphATPads *graph=state->GetGraph();

			CAnimTreePads *pads=(CAnimTreePads *)graph->GetPads();
			if (pads)
			{
				PadID id=pads->NewPad(_classes[idCmd-ID_AGENT_NEW_ATPAD_BEGIN].c_str(),_pt);

				//如果原来没有任何一个default root的话,我们把新建的root设为default 的
				CLinkPad *pad=pads->FindPad(id);
				if (pad->GetClass()->CheckName("CAtpRoot"))
				{
					if (pads->GetDefRoot()==PadID_Null)
						pads->SetDefRoot(id);
				}

				state->panel->RefreshStateMod();
			}
		}
		_Redraw(FALSE);
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ATPadsCommand
BOOL CGuiAgent_ATPadsCommand::OnRButtonClick(int x,int y,DWORD flag)
{
	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();

	if (sels->size()!=1)
		return TRUE;

	_pt.x=x;
	_pt.y=y;
	GetWnd()->ClientToScreen(&_pt);

	_ScreenToGG(x,y);

	GraphPadHit hit;
	BOOL bHit=graph->HitTest(x,y,hit);
	if (bHit)
	{
		CAnimTreePad*pad=(CAnimTreePad*)(graph->GetPads()->FindPad(hit.id));
		if (pad->CanModifyChild())
		{
			_sel=hit.id;
			_AddMenuSep();
			_AddMenu("新增Child",ID_AGENT_ADD_ATPAD_CHILD);
			if (hit.part==GraphPadHit::In)
			{
				_iStub=hit.item->iStub;
				_name=hit.item->name;
				_AddMenu("删除Child",ID_AGENT_REMOVE_ATPAD_CHILD);
				_AddMenu("对Child改名",ID_AGENT_RENAME_ATPAD_CHILD);
			}
			_AddMenuSep();
		}
		if (pad->GetClass()->CheckName("CAtpRoot"))
		{
			_sel=hit.id;
			CAnimTreePads *pads=(CAnimTreePads *)(graph->GetPads());
			if (pads->GetDefRoot()==pad->GetID())
				_AddMenu("缺省根节点",ID_AGENT_TOGGLE_ATPAD_DEF_ROOT,MF_ENABLED|MF_STRING|MF_CHECKED);
			else
				_AddMenu("缺省根节点",ID_AGENT_TOGGLE_ATPAD_DEF_ROOT,MF_ENABLED|MF_STRING);
			_AddMenuSep();
		}
	}
	return TRUE;
}

BOOL CGuiAgent_ATPadsCommand::OnCommand(DWORD idCmd)
{
	CGraphPads *graph=_GetGraph();

	if (idCmd==ID_AGENT_TOGGLE_ATPAD_DEF_ROOT)
	{
		CAnimTreePad*pad=(CAnimTreePad*)(graph->GetPads()->FindPad(_sel));
		if (pad->GetClass()->CheckName("CAtpRoot"))
		{
			CAnimTreePads *pads=(CAnimTreePads *)graph->GetPads();
			if (pads->GetDefRoot()==pad->GetID())
				pads->SetDefRoot(PadID_Null);
			else
				pads->SetDefRoot(pad->GetID());
			_NotifyChange(TRUE);
			_Redraw(FALSE);
		}
		return FALSE;
	}

	if (idCmd==ID_AGENT_ADD_ATPAD_CHILD)
	{
		CAnimTreePad*pad=(CAnimTreePad*)(graph->GetPads()->FindPad(_sel));
		pad->AddChild(graph->GetPads());
		_NotifyChange(TRUE);
		_Redraw(FALSE);
		return FALSE;
	}
	if (idCmd==ID_AGENT_REMOVE_ATPAD_CHILD)
	{
		CLinkPads *pads=graph->GetPads();
		CAnimTreePad *pad=(CAnimTreePad *)pads->FindPad(_sel);
		if (((DWORD)_iStub)<pad->GetChildCount())
		{
			pad->RemoveChild(_iStub,pads);
			_NotifyChange(TRUE);
			_Redraw(FALSE);
			return FALSE;
		}
	}
	if (idCmd==ID_AGENT_RENAME_ATPAD_CHILD)
	{
		CLinkPads *pads=graph->GetPads();
		CAnimTreePad *pad=(CAnimTreePad *)pads->FindPad(_sel);
		if (((DWORD)_iStub)<pad->GetChildCount())
		{
			_Redraw(TRUE);
			CEditPopup popup;
			std::string s=popup.Popup(_pt.x,_pt.y,_name.c_str());
			if (s!=_name)
			{
				pad->SetChildName(_iStub,s.c_str(),pads);
				_NotifyChange(TRUE);
				_Redraw(FALSE);
			}
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_AtnDbg

void CGuiAgent_AtnDbg::_UpdateCV(int x,int y,BOOL bEnd)
{
	CGraphATPads *graph=(CGraphATPads *)_GetGraph();

	PadDyn *dyn=graph->FindPadDyn(_sel);
	if (!dyn)
		return;

	CLinkPads *pads=graph->GetPads();
	CAnimTreePad *pad=((CAnimTreePad*)pads->FindPad(_sel));
	assert(pad);

	int w,h;
	w=_rc.getWidth()-DVBOX_WIDTH;
	h=_rc.getHeight()-DVBOX_WIDTH;

	x-=_rc.Left()+DVBOX_WIDTH/2;
	y-=_rc.Top()+DVBOX_WIDTH/2;

	if (bEnd)
	{
		if (pad->GetDbgFlag()&CAnimTreePad::DbgF_AutoReset)
			x=0;
	}

	if (w<=0)
		dyn->dv.x=0.0f;
	else
		dyn->dv.x=((float)x)/(float)w;
	if (h<=0)
		dyn->dv.y=0.0f;
	else
		dyn->dv.y=((float)y)/(float)h;
	dyn->dv.x=i_math::clamp_f(dyn->dv.x,0.0f,1.0f);
	dyn->dv.y=i_math::clamp_f(dyn->dv.y,0.0f,1.0f);

	if (pad->GetDbgType()==CAnimTreePad::Dbg_Name)
	{
		DWORD c;
		StringID *ids=pad->GetDbgNames(c);
		if (c>0)
		{
			int idx=i_math::clamp_i((int)(dyn->dv.x/(1.0f/(float)c)),0,c-1);
			dyn->name=ids[idx];
		}
		else
			dyn->name=StringID_Invalid;
	}

	//将计算出的ctrl value更新到具有相同CtrlGroup的pad中去
	if (TRUE)
	{
		std::string ctrlgrp=pad->GetDbgGroup();
		if (ctrlgrp!="")
		{
			DWORD c;
			CLinkPad **buf=pads->GetPads(c);
			for (int i=0;i<c;i++)
			{
				PadID id=buf[i]->GetID();
				if (id==_sel)
					continue;
				if (ctrlgrp==((CAnimTreePad*)buf[i])->GetDbgGroup())
				{
					PadDyn *dyn2=graph->FindPadDyn(id);
					if (dyn2)
					{
						dyn2->dv=dyn->dv;
						dyn2->name=dyn->name;
					}
				}
			}
		}
	}

	_Redraw(FALSE);

}


BOOL CGuiAgent_AtnDbg::OnBeginDrag(int x,int y,DWORD flag)
{
	_ScreenToGG(x,y);

	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();

	GraphPadHit hit;
	BOOL bHit=graph->HitTest(x,y,hit);
	if (bHit)
	{
		if (hit.part==GraphPadHit::Ctrl)
		{
			_sel=hit.id;
			_rc=hit.item->rcFocus;
			_UpdateCV(x,y,FALSE);
			return TRUE;
		}
	}

	return FALSE;

	
}

void CGuiAgent_AtnDbg::OnDrag(int x,int y,DWORD flag)
{
	_ScreenToGG(x,y);
	_UpdateCV(x,y,FALSE);
}


void CGuiAgent_AtnDbg::OnEndDrag(int x,int y,DWORD flag)
{
	_ScreenToGG(x,y);
	_UpdateCV(x,y,TRUE);
	_sel=PadID_Null;
}




//////////////////////////////////////////////////////////////////////////
//CAnimTreePreview

//返回值0: 没有变化,1:有轻微的变化,2:有巨大的变化
static int CheckChange(AnimTreeData *data1,AnimTreeData *data2)
{
	if ((!data1)&&(data2))
		return 2;//巨大变化
	if ((!data2)&&(data1))
		return 2;//巨大变化
	if (data1->Equal(*data2))
		return 0;//没有变化

	//检查是轻微变化还是巨大变化
	AnimTreeData *dataNew=(AnimTreeData *)data1->Clone();
	AnimTreeData *dataLast=(AnimTreeData *)data2->Clone();
	DWORD n,n2;
	CLinkPad **buf=dataLast->pads.GetPads(n);
	CLinkPad **buf2=dataNew->pads.GetPads(n2);

	BOOL bChanged=TRUE;
	if (n==n2)
	{
		//把位置改成同样的
		for (int i=0;i<n;i++)
			buf[i]->SetPos(buf2[i]->GetPos());

		//FolderXfm全部清除
		dataLast->pads.ClearFolderXfm();
		dataNew->pads.ClearFolderXfm();

		if (dataLast->Equal(*dataNew))
			bChanged=FALSE;
	}
	ResData_Delete(dataNew);
	ResData_Delete(dataLast);

	return bChanged?2:1;
}

extern unsigned __int64 GetAbsTick();

BOOL CAnimTreePreview::Reset(AnimTreeData*data,IWorldSystem *pWS)
{
	if (CheckChange(_dataLast,data)<2)
		return FALSE;//没有大变化,我们不重置

	Clear();
	IRenderSystem *pRS=pWS->GetRS();

	for (int i=0;i<data->preview.models.size();i++)
	{
		IMesh *mesh=(IMesh *)pRS->GetMeshMgr()->ObtainRes(data->preview.models[i].mesh.c_str());
		IMtrl*mtrl=(IMtrl*)pRS->GetMtrlMgr()->ObtainRes(data->preview.models[i].mtrl.c_str());
		if (mesh&&mtrl)
		{
			_mesh.push_back(mesh);
			_mtrl.push_back(mtrl);
		}
		else
		{
			SAFE_RELEASE(mesh);
			SAFE_RELEASE(mtrl);
		}
	}

	if (!data->preview.anim.empty())
		_anims.push_back((IBoneAnim *)pRS->GetBoneAnimMgr()->ObtainRes(data->preview.anim.c_str()));
	for (int i=0;i<data->preview.animsAddon.size();i++)
	{
		if (!data->preview.animsAddon[i].empty())
			_anims.push_back((IBoneAnim *)pRS->GetBoneAnimMgr()->ObtainRes(data->preview.animsAddon[i].c_str()));
	}
	_animtree=(IAnimTree*)pRS->GetDynAnimTreeMgr()->Create(data);
	_ctrl=pWS->CreateAnimTreeCtrl(_anims.data(),_anims.size(),_animtree,_t);
	_ctrl->SetIgnoreCalcErr();//不要输出Calc的报错,因为,这是预料中的事情

	_meshCross=(IMesh*)pRS->GetMeshMgr()->ObtainRes("_editor\\axisarrow.msh");
	_mtrlCross=(IMtrl*)pRS->GetMtrlMgr()->ObtainRes("_editor\\axisarrow.mtl");

	//创建dbgs
	if (TRUE)
	{
		CLinkPads *pads=&data->pads;
		DWORD c;
		CLinkPad **p=pads->GetPads(c);
		for (int i=0;i<c;i++)
		{
			PadID id=p[i]->GetID();
			AnimNodeATDbg *t=Class_New2(AnimNodeATDbg);
			t->AddRef();
			_ctrl->BindDbg(id,t);
			_dbgs[id]=t;
		}
	}

	_anIkEffector._mat.setTranslation(0.0f,0.0f,5.0f);
	_ctrl->BindIKEffector(StringID_Invalid,&_anIkEffector);

	_mats=pRS->CreateMatrice43();

	//重置时间
	if (TRUE)
	{
		_timer.Reset();
		_t=ANIMTICK_FROM_SECOND((float)_timer.GetTime());
		_t/=ANIMTICK_FROM_SECOND(0.05f);
		_t*=ANIMTICK_FROM_SECOND(0.05f);
		_t=ANIMTICK_SAFE_MINUS(_t,ANIMTICK_FROM_SECOND(0.05f));
		_ctrl->Tick(_t);
// 		_t+=ANIMTICK_FROM_SECOND(0.05f);
// 		_ctrl->Tick(_t);
	}

	if (!_dataLast)
		_dataLast=(AnimTreeData*)data->Clone();
	else
		_dataLast->Copy(*data);

	return TRUE;
}

void CAnimTreePreview::Clear()
{
	for (int i=0;i<_mesh.size();i++)
		SAFE_RELEASE(_mesh[i]);
	for (int i=0;i<_mtrl.size();i++)
		SAFE_RELEASE(_mtrl[i]);
	_mesh.clear();
	_mtrl.clear();
	for (int i=0;i<_anims.size();i++)
	{
		SAFE_RELEASE(_anims[i]);
	}
	_anims.clear();
	SAFE_RELEASE(_animtree);
	SAFE_RELEASE(_ctrl);
	SAFE_RELEASE(_mats);
	SAFE_RELEASE(_meshCross);
	SAFE_RELEASE(_mtrlCross);

	_xfms.clear();

	std::unordered_map<PadID,AnimNodeATDbg*>::iterator it;
	for (it=_dbgs.begin();it!=_dbgs.end();it++)
		SAFE_RELEASE((*it).second);
	_dbgs.clear();

	if (_dataLast)
		ResData_Delete(_dataLast);
	_dataLast=NULL;

	Zero();
}

void CAnimTreePreview::Draw(IRenderPort *rp,CWnd *wnd)
{
	if (!_ctrl)
		return;
	ISkeleton *skl=NULL;
	if (_anims.size()>0)
	{
		if (SafeForceTouch(_anims[0]))
			skl=_anims[0]->GetSkeleton();
	}

// 	ICamera *cam=rp->QueryCamera();
// 	cam->SetNearFar(0.1f,1500.0f);

	AnimTick tEventDur=ANIMTICK_FROM_SECOND(2.0f);

	AnimTick t=ANIMTICK_FROM_SECOND(((float)_timer.GetTime()));
	while(_t<t)
	{
		_t+=ANIMTICK_FROM_SECOND(0.05f);//50 ms
		_ctrl->Tick(_t);	

		//处理事件
		if (TRUE)
		{
			AnimEvent **events;
			DWORD nEvents;
			events=_ctrl->FetchEvents(nEvents);
//			DWORD t=GetTickCount();


			for(int i=0;i<nEvents;i++)
			{
				Event e;
				e.e=events[i];
				e.t=_t;
				_events.push_back(e);
			}
			//丢弃太旧的事件
			if (TRUE)
			{
				int i=0;
				while(_events.size()>0)
				{
					if (t<_events[i].t)
						break;
					if ((t-_events[0].t)>tEventDur)
					{
						_events.pop_front();
						continue;
					}
					break;
				}
			}
		}

	}

	if (TRUE)
	{
		static i_math::xformf xfms[256];
		if (skl&&_ctrl->CalcXfms(t,xfms,skl->GetBoneCount()))
		{
			if (TRUE)
			{
				BoneCtrls bcs;
				static BoneCtrl buf[256];
				bcs.bcs_=buf;
				bcs.nBC=skl->GetBoneCount();
				bcs.xfms=xfms;
				_ctrl->CalcAndApplyBoneCtrls(t,bcs);
			}

			if (TRUE)
			{
				IKCtrls ikcs;
				ikcs.xfms=xfms;

				_ctrl->CalcAndApplyIKCtrls(t,ikcs);
			}

			_mats->SetCount(skl->GetBoneCount());
			skl->CalcSkeletonMatrice(_mats,xfms,NULL);

			skl->CalcSkinMatrice(_mats);
			for (int i=0;i<_mesh.size();i++)
				rp->SimpleDrawMesh(_mesh[i],_mats->GetPtr(),_mats->GetCount(),0xffffffff,FALSE,_mtrl[i]);
		}
		else
		{
			float v;
			if (_ctrl->CalcFloat(t,v))
			{
				std::string s;
				FormatString(s,"{F:1}{S:16}数值结果: {C:0,255,0}%.3f",v);
				DrawFontArg arg;
				i_math::pos2di pt(4,4);
				i_math::size2di sz;
				rp->CalcDrawText(s.c_str(),arg,sz);
				i_math::recti rc(pt,sz);
				rc.inflate(2,2,2,2);
				rp->FillRect(rc,ColorAlpha(0,128));
				rp->FrameRect(rc,0xffffffff);
				arg.SetLocation(pt.x,pt.y);
				rp->DrawText(s.c_str(),arg);
			}
			else
			{
				i_math::xformf xfm;
				if (_ctrl->CalcXfm(t,xfm))
				{
					i_math::matrix43f mat;
					xfm.getMatrix(mat);
					rp->SimpleDrawMesh(_meshCross,&mat,1,0xffffffff,FALSE,_mtrlCross);
				}
			}

			for (int i=0;i<_mesh.size();i++)
				rp->SimpleDrawMesh(_mesh[i],NULL,0,0xffffffff,FALSE,_mtrl[i]);
		}
	}

	if (wnd)
	{//绘制事件
		AnimTick t=ANIMTICK_FROM_SECOND(((float)_timer.GetTime()));

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		wnd->ScreenToClient(&ptCursor);
		CRect rc;
		wnd->GetClientRect(&rc);
		if (TRUE)
//			if (!rc.PtInRect(ptCursor))
		{
			ptCursor.x=rc.right-10;
			ptCursor.y=10;
		}
		else
		{
			ptCursor.x+=20;
		}
		int x=ptCursor.x;
		int y;
		for (int i=_events.size()-1;i>=0;i--)
		{
			Event &e=_events[i];

			DrawFontArg arg;
			float rate=1.0f-((float)(tEventDur-(t>e.t?t-e.t:0)))/(float)tEventDur;
			y=ptCursor.y+(int)((rate*20.0f)*(rate*20.0f));
			x=ptCursor.x;
			arg.SetLocation(x,y);
			arg.SetAlign(DT_RIGHT);
			arg.SetAlpha(1);

			rp->DrawText(StrLib_GetStr(e.e->name),arg);
		}
	}
}


void CAnimTreePreview::SyncPadDyns(CGraphATPads *graph)
{

	CLinkPads *pads=graph->GetPads();

	std::unordered_map<PadID,AnimNodeATDbg*>::iterator it;
	for (it=_dbgs.begin();it!=_dbgs.end();it++)
	{
		PadID id=(*it).first;
		PadDyn *dyn=graph->FindPadDyn(id);
		if (!dyn)
			continue;

		CAnimTreePad *pad=(CAnimTreePad *)pads->FindPad(id);
		assert(pad);

		//将各个pad里的dv更新到_dbgs中去
		(*it).second->v=dyn->dv;
		(*it).second->idStr=dyn->name;

		if (pads)
		{
			CAnimTreePad *pad=(CAnimTreePad *)pads->FindPad(id);
			if (pad)
				pad->ConvertDV((*it).second->v);//给pad一个机会转换一下
		}
	}
}


//////////////////////////////////////////////////////////////////////////

CAnimTreeEditPanel::CAnimTreeEditPanel()
{
	_anchor.SetResType(Res_AnimTree);
	_anchor.SetLabel("AnimTree");

	_dataLast=NULL;

}

CAnimTreeEditPanel::~CAnimTreeEditPanel(void)
{
	ResData_Delete(_dataLast);

}

void CAnimTreeEditPanel::Init3d()
{
}
void CAnimTreeEditPanel::Clear3d()
{
}

BEGIN_MESSAGE_MAP(CAnimTreeEditPanel,CResEditPanel)
	ON_WM_SIZE()
	ON_WM_TIMER()

END_MESSAGE_MAP()

void CAnimTreeEditPanel::DoDataExchange(CDataExchange* pDX)
{	
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_ANIMTREEANCHOR,_anchor);
}

BOOL CAnimTreeEditPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();

	//grid
	if (TRUE)
	{
		CRect rc;
		GET_CONTROL_RECT(this,IDC_ANIMTREEGRID,rc);
		HIDE_CONTROL(this,IDC_ANIMTREEGRID);
		_grid.Create(rc,this,IDC_ANIMTREEGRID);
	}

	AddCtrl(dynamic_cast<CResEditCtrl*>(&_grid));


	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}



void CAnimTreeEditPanel::OnResDataChange(ResData *dataNew)
{
	_stateToMod->SetData(dataNew);
	if(!dataNew)  return;

	//初始化GG Transform
	if (TRUE)
	{
		AnimTreeData *data=(AnimTreeData *)_stateToMod->resdata;
		i_math::pos2df off,scale;
		if (data->pads.GetFolderXfm(data->pads.GetCurFolder(),off,scale))
			_view2->SetTransformGG(off,scale);
	}



}

void CAnimTreeEditPanel::Draw(IRenderPort *rp)
{
	
	_preview.Draw(rp,_view->GetWnd());
}


void CAnimTreeEditPanel::Draw(GraphicsGraph*gg)
{	
	Reps_AnimTree *state=(Reps_AnimTree *)_stateToMod;
	if (!state)
		return;
	AnimTreeData *data=(AnimTreeData *)state->resdata;

	_preview.SyncPadDyns(state->GetGraph());

	state->GetGraph()->Draw(gg,&state->sels[0],state->sels.size(),_preview.GetCtrl());

}

BOOL CAnimTreeEditPanel::StateToControl(ResEditPanelState *state0)//Update the controls in the panel to reflect the state
{	
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	Reps_AnimTree *state=(Reps_AnimTree *)state0;
	AnimTreeData *data=(AnimTreeData *)state->resdata;

	_preview.Reset(data,g_ssGuiLib.pWS);//有巨大变化时,我们才要重置preview

	if (CheckChange(_dataLast,data)>0)
	{//有一点点变化

		_graph.Clear();
		_graph.Load(&data->pads);
		_graph.RecalcLayout(_view2->GetGG());
		_graph.SyncDyn(&data->pads);

		_preview.SyncPadDyns(&_graph);

		if (_dataLast)
			_dataLast->Copy(*data);
		else
			_dataLast=(AnimTreeData *)data->Clone();
	}

	return TRUE;
}


ResEditPanelState *CAnimTreeEditPanel::_NewState()
{
	 Reps_AnimTree * state =  new Reps_AnimTree;
	 return state;
}

void CAnimTreeEditPanel::_UpdateView2Agent()
{
	static PadsCB cb;
	ClearAgent_View2();
	if (_bEnable)
	{
		_view2->AddAgent(0,new CGuiAgent_GraphPadScroll(&_src),AGENTPRIORITY_STANDARD+1);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadSel(&_src,FALSE),AGENTPRIORITY_STANDARD+12);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadRectSel(&_src),AGENTPRIORITY_STANDARD+4);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadConnect(&_src),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadCommand(&_src,&cb),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_NewATPads(),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_AtnDbg(&_src),AGENTPRIORITY_STANDARD+10);	
		_view2->AddAgent(0,new CGuiAgent_ATPadsCommand(&_src),AGENTPRIORITY_STANDARD+10);	
	}
	else
	{
		_view2->AddAgent(0,new CGuiAgent_GraphPadScroll(&_src),AGENTPRIORITY_STANDARD+1);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadSel(&_src,TRUE),AGENTPRIORITY_STANDARD+12);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadRectSel(&_src),AGENTPRIORITY_STANDARD+4);	
		_view2->AddAgent(0,new CGuiAgent_AtnDbg(&_src),AGENTPRIORITY_STANDARD+10);	
	}
}


void CAnimTreeEditPanel::OnSelect()
{
	ClearAgent();

	_AddCameraController();

	_UpdateView2Agent();
}

void CAnimTreeEditPanel::EnablePanel(BOOL bEnable)
{
	BOOL bChange=(bEnable!=_bEnable);

	CResEditPanel::EnablePanel(bEnable);

	if (bChange)
		_UpdateView2Agent();
}

void CAnimTreeEditPanel::OnSize(UINT nType, int cx, int cy)
{
	CResEditPanel::OnSize(nType, cx, cy);

	if (_grid.GetSafeHwnd())
	{
		CRect rc;
		_grid.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.bottom=rc.top+cy-34;
		rc.right=rc.left+cx-20;
		if (rc.bottom<rc.top+4)
			rc.bottom=rc.top+4;
		if (rc.right<rc.left+4)
			rc.right=rc.left+4;
		_grid.MoveWindow(&rc);
	}
}
