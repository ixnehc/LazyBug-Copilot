/********************************************************************
created:	2008/2/21   15:13
file path:	d:\IxEngine\Proj_GuiLib
author:		cxi

purpose:	asset creating/mofifying panel
*********************************************************************/
 
#include "stdh.h"

#include <vector>
#include <string>

#include "resource.h"
#include "AgentCmdID.h"

#include "FileSystem/IFileSystem.h"
#include "FileSystem/IMapFile.h"
#include "RenderSystem/IUtilRS.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IAnim.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IEntitySystem.h"


#include "stringparser/stringparser.h"
#include "commondefines/general_stl.h"

#include "GuiData_Pathes.h"
#include "GuiData.h"

#include "RenderSystem/IRenderSystem.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"

#include "GuiAgent_general.h"
#include "GuiActor_Pathes.h"

#include "Log/LogDump.h"

#include "timer/profiler.h"
#include ".\guiactor_pathes.h"

#include "resdata/AnimData.h"
#include "Registry/Registry.h"

extern CCurrentUserRegistry g_reg;

//显示控制点的使用的半径
#define RADIUS_CP 0.2f


//////////////////////////////////////////////////////////////////////////
//CMod_ReplacePath
BOOL CMod_ReplacePath::Redo()
{
	XFormData *data=(XFormData *)_dataPathes->FindData(_pathRes.c_str());

	if (data)
	{
		//更新/备份
		XFormData t;
		t.Copy(*data);
		data->Copy(_backup);
		_backup.Copy(t);

		//将更新的数据存盘
		std::string path=_dataPathes->pathRoot+"\\"+_pathRes;
		_dataPathes->pUtilRS->SaveRes(path.c_str(),data);

		if (_dataPathes->iSelCP>=data->cps.size())
			_dataPathes->iSelCP=data->cps.size()-1;
		_dataPathes->bLocateCP=_dataPathes->bAddCP=FALSE;
	}

	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CGuiAgent_DrawPath

void CGuiAgent_DrawPath::OnAttachView(CGeView *view,DWORD iLevel)
{
	_meshPlane=(IMesh*)GetRS()->GetMeshMgr()->ObtainRes("_editor\\plane.msh");
	_mtrlPlane=(IMtrl*)GetRS()->GetMtrlMgr()->ObtainRes("_editor\\plane.mtl");
	_mtrlPlane2=(IMtrl*)GetRS()->GetMtrlMgr()->ObtainRes("_editor\\plane2.mtl");

	_meshPlane->ForceTouch();
	_mtrlPlane->ForceTouch();
	_mtrlPlane2->ForceTouch();

	extern ILight *CreateCPLight(IRenderSystem *pRS);
	_lgt=CreateCPLight(GetRS());
}

void CGuiAgent_DrawPath::OnDetachView(CGeView *view,DWORD iLevel)
{
	SAFE_RELEASE(_meshPlane);
	SAFE_RELEASE(_mtrlPlane);
	SAFE_RELEASE(_mtrlPlane2);

	SAFE_RELEASE(_lgt);

}

BOOL CGuiAgent_DrawPath::OnDraw()
{

	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return TRUE;

	if (dataPathes->bRunning)
		return FALSE;

	XFormData *sel=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (!sel)
		return TRUE;


	IRenderPort *rp=GetRP();
	KeySet *keyset=sel->GetKeySet();

	//根据renderport camera更新_lgt的方向
	if (TRUE)
	{
		LightInfo *li=_lgt->QueryInfo();
		rp->GetCamera()->GetEyeDir(li->dir);
		li->dir.normalize();
	}

	i_math::vector3df eye;
	if (TRUE)//得到camera的位置
	{
		rp->GetCamera()->GetEyePos(eye);
	}

	for (int i=0;i<keyset->GetKeyCount();i++)
	{
		Key_xform *key=(Key_xform *)keyset->GetKey(i);

		i_math::xformf xfm=key->v;

		if ((eye-xfm.pos).getLengthSQ()>=1600.0f)//40m以外
			continue;

		xfm.scale_=0.2f;
		i_math::matrix43f mat=xfm.getMatrix();

		rp->SimpleDrawMesh(_meshPlane,mat,0xffffffff,FALSE,_mtrlPlane,_lgt);
	}


	_spln.Reset(sel->bCircular);

	for (int i=0;i<sel->cps.size();i++)
	{

		if (i==dataPathes->iSelCP)
		{
			i_math::xformf xfm=sel->cps[i].xfm;
			xfm.scale_=0.4f;
			i_math::matrix43f mat=xfm.getMatrix();
			rp->SimpleDrawMesh(_meshPlane,mat,ColorAlpha(0xffffff,0xff),FALSE,_mtrlPlane2,_lgt);
		}

		_spln.AddNode(sel->cps[i].xfm.pos,sel->cps[i].xfm.rot);
	}

	_spln.BuildSNS();

	float tGap=0.2f/_spln.GetDistance();

	std::vector<CCubicSpline::Sample>samples;

	samples.resize((int)(1.0f/tGap)+20);
	samples.resize(_spln.GetSamplesByTime(tGap,samples.data()));

	if (samples.size()>1)
	{
		_lines.resize((samples.size()-1)*2);

		for (int i=0;i<_lines.size();i++)
			_lines[i]=samples[(i+1)/2].pos;

		rp->Lines(_lines.data(),_lines.size()/2,ColorAlpha(0xffff00,0xff));
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_RunPath

void CGuiAgent_RunPath::OnAttachView(CGeView *view,DWORD iLevel)
{
	_cam=GetRS()->CreateCamera();
}


void CGuiAgent_RunPath::OnDetachView(CGeView *view,DWORD iLevel)
{
	SAFE_RELEASE(_cam);
	SAFE_RELEASE(_player);
}

BOOL CGuiAgent_RunPath::Respond(CtrlOp &co)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return TRUE;

	if (dataPathes->bRunning)
	{
		CGuiAgent::Respond(co);
		return FALSE;
	}
	return CGuiAgent::Respond(co);

}



BOOL CGuiAgent_RunPath::OnRButtonClick(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	if (dataPathes->bLocateCP)
		return TRUE;
	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (data)
	{
		_AddMenuSep();
		if (!dataPathes->bRunning)
		{
			_AddMenu("开始巡航",ID_AGENT_RUNPATH);
			_AddMenu("开始巡航(从头开始)",ID_AGENT_RUNPATHFROMSTART);
		}
		else
			_AddMenu("停止巡航",ID_AGENT_STOPRUNPATH);

		_AddMenuSep();
	}

	if (dataPathes->bRunning)
		return FALSE;//巡航模式下,不能进行其它操作
	return TRUE;
}

BOOL CGuiAgent_RunPath::OnCommand(DWORD idCmd)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	switch(idCmd)
	{
		case ID_AGENT_RUNPATHFROMSTART:
		case ID_AGENT_RUNPATH:
		{
			IRenderSystem *pRS=dataPathes->pUtilRS->GetRS();

			IAnim *anim=pRS->GetDynAnimMgr()->Create(data);
			if (anim)
			{
				_player=pRS->CreateAnimPlayer();
				_player->Reset(anim,0,0,data->bCircular);

				SAFE_RELEASE(anim);

				_tickStart=GetTickCount();

				if ((dataPathes->iSelCP==-1)||(ID_AGENT_RUNPATHFROMSTART==idCmd))
					_tOff=0;
				else
				{
					XFormData::CtrlPointSampleInfo info;
					data->CalcFromCPs(&info);

					_tOff=ANIMTICK_FROM_SECOND(info.cptimes[dataPathes->iSelCP]);
				}

				dataPathes->bRunning=TRUE;

				Key_xform *key=(Key_xform *)data->keyset.GetKey(0);
				_eye=key->v.pos;


				GuiData_Camera*dataCamera=(GuiData_Camera*)FindData("cameras");
				if (dataCamera)
					_cam->Clone(dataCamera->cams[Camera_Perspective]);//备份
			}

			break;
		}
		case ID_AGENT_STOPRUNPATH:
		{
			dataPathes->bRunning=FALSE;
			SAFE_RELEASE(_player);

// 			GuiData_Camera*dataCamera=(GuiData_Camera*)FindData("cameras");
// 			if (dataCamera)
// 				dataCamera->cams[Camera_Perspective]->Clone(_cam);//恢复

			break;
		}
	}

	return TRUE;
}

BOOL CGuiAgent_RunPath::OnKeyDown(char c,DWORD flag)
{
	if (c==27)
	{
		OnCommand(ID_AGENT_STOPRUNPATH);
		return FALSE;
	}
	return CGuiAgent::OnKeyDown(c,flag);

}


BOOL CGuiAgent_RunPath::OnTimer(int ,DWORD flag)
{
	if (_player)
	{
		DWORD dt=GetTickCount()-_tickStart;
		AnimTick t=ANIMTICK_FROM_SECOND(((float)dt)/1000.0f);
		t+=_tOff;
		ProfilerStart_Recent(RunPath_GetKey);
		Key_xform *key=(Key_xform *)_player->Calc(t);
		ProfilerEnd();

		GuiData_Camera*dataCamera=(GuiData_Camera*)FindData("cameras");
		if (dataCamera)
		{
			ICamera *cam=dataCamera->cams[Camera_Perspective];

			i_math::matrix43f mat=key->v.getMatrix();

			i_math::vector3df eye,at,up;
			if (TRUE)
			{
				eye=mat.getTranslation();
				at.set(0,0,1);
				mat.rotateVect(at,at);
				at+=eye;
				up.set(0,1,0);
				mat.rotateVect(up,up);
			}
			if(FALSE)//丢弃位移
			{
				eye=_eye;
//				eye=mat.getTranslation();
				at.set(0,0,1);
				mat.rotateVect(at,at);
//				at.set(1,0,0);
				at+=eye;
				up.set(0,1,0);
				mat.rotateVect(up,up);

			}
			if(FALSE)//丢弃旋转
			{
				eye=mat.getTranslation();
				at.set(0,0,1);
				at+=eye;
				up.set(0,1,0);
			}



			cam->SetPosTarget(eye,at,up);
		}

	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_LocateCP

BOOL CGuiAgent_LocateCP::Respond(CtrlOp &co)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return TRUE;

	if (!dataPathes->bLocateCP)
		return TRUE;
	if (dataPathes->IsSelReadOnly())
		return TRUE;

	return CGuiAgent::Respond(co);
}

void CGuiAgent_LocateCP::_UpdateCP(int x,int y)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)FindData("terrain");
	if (!dataTrrn)
		return;

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (data)
	{
		i_math::vector3df pos;
		if (dataTrrn->GetHitPos(x,y,GetRP(),pos))
		{
			pos.y+=dataPathes->heightLocate;

			if (dataPathes->iSelCP!=-1)
				data->cps[dataPathes->iSelCP].xfm.pos=pos;
		}
	}

}

BOOL CGuiAgent_LocateCP::OnMouseMove(int x,int y,DWORD flag)
{
	_UpdateCP(x,y);

	_Redraw();

	return FALSE;
}

BOOL CGuiAgent_LocateCP::OnMouseWheel(int delta,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	dataPathes->heightLocate+=0.001f*(float)delta;
	i_math::pos2di pt;
	_GetCursorPos(pt);

	_UpdateCP(pt.x,pt.y);

	_Redraw();

	return FALSE;
}

BOOL CGuiAgent_LocateCP::OnLButtonDown(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	_UpdateCP(x,y);

	if ((dataPathes->bAddCP)&&(dataPathes->bLocateCP))
	{
		XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
		if (data)
		{
			XFormData::CtrlPoint cp;
			cp.bVelocityAligned=1;
			data->cps.push_back(cp);

			dataPathes->iSelCP=data->cps.size()-1;
			_UpdateCP(x,y);
		}
	}

	return FALSE;
}

BOOL CGuiAgent_LocateCP::OnRButtonClick(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	if (dataPathes->bLocateCP)
	{
		dataPathes->bLocateCP=FALSE;

		XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
		if (data)
		{
			data->cps.erase(data->cps.begin()+dataPathes->iSelCP);
			dataPathes->iSelCP--;

			data->CalcFromCPs();

			dataPathes->modified=dataPathes->sel;
		}
	}

	return FALSE;
}

BOOL CGuiAgent_LocateCP::OnDraw()
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (data)
	{
		if (dataPathes->iSelCP!=-1)
		{
			i_math::vector3df line[2];
			line[0]=data->cps[dataPathes->iSelCP].xfm.pos;
			line[1]=line[0];
			line[1].y-=1000.0f;
			IRenderPort *rp=GetRP();
			rp->Line(line[0],line[1],ColorAlpha(0x00ff00,0xff));
		}
	}

	return TRUE;
}

int NextCP(XFormData *data,int iSel)
{
	if (data->cps.size()<=0)
		return iSel;
	iSel++;
	if (data->bCircular)
		iSel%=data->cps.size();
	else
		iSel=i_math::clamp_i(iSel,0,data->cps.size()-1);
	return iSel;
}

int PrevCP(XFormData *data,int iSel)
{
	if (data->cps.size()<=0)
		return iSel;
	iSel--;
	if (data->bCircular)
		iSel=(iSel+data->cps.size())%data->cps.size();
	else
		iSel=i_math::clamp_i(iSel,0,data->cps.size()-1);
	return iSel;
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_OperateCP
BOOL CGuiAgent_OperateCP::OnRButtonClick(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	if (dataPathes->IsSelReadOnly())
		return TRUE;

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (data)
	{
		_AddMenuSep();
		_AddMenu("增补控制点",ID_AGENT_APPENDCP);
		_AddMenu("增补控制点(到当前Camera位置)  ... [F2]",ID_AGENT_APPENDCAMERACP);
		if (PrevCP(data,dataPathes->iSelCP)!=dataPathes->iSelCP)
			_AddMenu("插入控制点(之前)",ID_AGENT_INSERTCP_BEFORE);
		if (NextCP(data,dataPathes->iSelCP)!=dataPathes->iSelCP)
			_AddMenu("插入控制点(之后)",ID_AGENT_INSERTCP_AFTER);
		if (dataPathes->iSelCP>=0)
			_AddMenu("删除控制点",ID_AGENT_REMOVECP);
		if (data->cps.size()>0)
			_AddMenu("清除所有控制点",ID_AGENT_CLEARCP);

		_AddMenuSep();

		if (data->bCircular)
			_AddMenu("循环路径",ID_AGENT_SWITCH_CP_CIRCULAR,MF_ENABLED|MF_STRING|MF_CHECKED);
		else
			_AddMenu("循环路径",ID_AGENT_SWITCH_CP_CIRCULAR,MF_ENABLED|MF_STRING);

		_AddMenuSep();
	}

	return TRUE;

}

BOOL CGuiAgent_OperateCP::OnCommand(DWORD idCmd)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	if (dataPathes->IsSelReadOnly())
		return TRUE;

	XFormData *sel=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (!sel)
		return FALSE;

	switch(idCmd)
	{
		case ID_AGENT_APPENDCAMERACP:
		{
			IRenderPort *rp=GetRP();
			XFormData::CtrlPoint cp;
			i_math::matrix43f mat;
			if (rp->GetCamera()->GetEyeMat(mat))
			{
				cp.xfm.fromMatrix(mat);
				cp.bVelocityAligned=0;
				sel->cps.push_back(cp);
				sel->CalcFromCPs();
				dataPathes->iSelCP=sel->cps.size()-1;
				dataPathes->modified=dataPathes->sel;
			}
			break;
		}
		case ID_AGENT_APPENDCP:
		{
			XFormData::CtrlPoint cp;
			sel->cps.push_back(cp);
			dataPathes->iSelCP=sel->cps.size()-1;
			dataPathes->bLocateCP=TRUE;
			dataPathes->bAddCP=TRUE;
			break;
		}
		case ID_AGENT_CLEARCP:
		{
			if (AfxMessageBox(_T("将会删除所有控制点,确认吗?"), MB_YESNO) == IDYES)
			{
				sel->cps.clear();
				sel->CalcFromCPs();
				dataPathes->iSelCP=-1;
				dataPathes->modified=dataPathes->sel;
			}
			break;
		}
		case ID_AGENT_REMOVECP:
		{
			if (dataPathes->iSelCP>=0)
			{
				sel->cps.erase(sel->cps.begin()+dataPathes->iSelCP);
				sel->CalcFromCPs();
				if (dataPathes->iSelCP>=sel->cps.size())
					dataPathes->iSelCP=sel->cps.size()-1;
				dataPathes->modified=dataPathes->sel;
			}
			break;
		}
		case ID_AGENT_INSERTCP_BEFORE:
		case ID_AGENT_INSERTCP_AFTER:
		{
			int iNew=(idCmd==ID_AGENT_INSERTCP_BEFORE)?PrevCP(sel,dataPathes->iSelCP):NextCP(sel,dataPathes->iSelCP);
			if (iNew!=dataPathes->iSelCP)
			{
				XFormData::CtrlPointSampleInfo info;
				sel->CalcFromCPs(&info);

				XFormData::CtrlPoint cp;
				cp=sel->cps[dataPathes->iSelCP];
				if (TRUE)//计算新加入的控制点的位置
				{
					float t;
					if (idCmd==ID_AGENT_INSERTCP_BEFORE)
						t=(info.cptimes[iNew]+info.cptimes[iNew+1])/2.0f;
					else
						t=(info.cptimes[dataPathes->iSelCP]+info.cptimes[dataPathes->iSelCP+1])/2.0f;

					IAnim *anim=GetRS()->GetDynAnimMgr()->Create(sel);
					IAnimPlayer *player=GetRS()->CreateAnimPlayer();
					player->Reset(anim,0,0,FALSE);
					cp.xfm=((Key_xform*)player->Calc(ANIMTICK_FROM_SECOND(t)))->v;

					SAFE_RELEASE(anim);
					SAFE_RELEASE(player);
				}
				if (idCmd==ID_AGENT_INSERTCP_BEFORE)
                    sel->cps.insert(sel->cps.begin()+dataPathes->iSelCP,cp);
				else
				{
					sel->cps.insert(sel->cps.begin()+iNew,cp);
					dataPathes->iSelCP=iNew;
				}
				sel->CalcFromCPs();
				dataPathes->modified=dataPathes->sel;
			}
			break;
		}
		case ID_AGENT_SWITCH_CP_CIRCULAR:
		{
			sel->bCircular=!sel->bCircular;
			sel->CalcFromCPs();
			dataPathes->modified=dataPathes->sel;
			break;
		}
		default:
			return TRUE;
	}

	return FALSE;
}

BOOL CGuiAgent_OperateCP::OnKeyDown(char c,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	XFormData *sel=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());

	if(c==VK_F2)
	{
		OnCommand(ID_AGENT_APPENDCAMERACP);
		return FALSE;
	}
	if (sel)
	{
		if (c=='A')
			dataPathes->iSelCP=PrevCP(sel,dataPathes->iSelCP);
		if (c=='D')
			dataPathes->iSelCP=NextCP(sel,dataPathes->iSelCP);
	}

	return CGuiAgent::OnKeyDown(c,flag);
}




//////////////////////////////////////////////////////////////////////////
//CGuiAgent_SelCP

BOOL CGuiAgent_SelCP::OnLButtonDown(int x,int y,DWORD flag)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	HitProbe probe;
	if (GetRP()->CalcHitProbe(x,y,probe))
	{
		i_math::spheref sph;
		sph.radius=RADIUS_CP;

		dataPathes->iSelCP=-1;

		for (int j=0;j<2;j++)
		{
			if (j==1)
				dataPathes->sel="";

			double distMin=100000000.0f;
			std::unordered_map<std::string,ResData*>::iterator it;
			for (it=dataPathes->dataes.begin();it!=dataPathes->dataes.end();it++)
			{
				XFormData *pth=(XFormData *)(*it).second;

				if (j==0)
				{
					if (!StringEqualNoCase((*it).first.c_str(),dataPathes->sel.c_str()))
						continue;//优先选择当前path上的cp
				}
				else
				{
					if (StringEqualNoCase((*it).first.c_str(),dataPathes->sel.c_str()))
						continue;//优先选择当前path上的cp
				}

				for (int i=0;i<pth->cps.size();i++)
				{
					sph.center=pth->cps[i].xfm.pos;
					double dist;
					if (sph.getIntersectionWithLine(probe,dist))
					{
						if (dist<distMin)
						{
							dataPathes->sel=(*it).first;
							dataPathes->iSelCP=i;
							distMin=dist;
						}
					}
				}
			}
			if (dataPathes->iSelCP!=-1)
				break;
		}
		_Redraw(FALSE);

		if (dataPathes->sel!="")
			return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_CPMatrixEdit

CGuiAgent_CPMatrixEdit::CGuiAgent_CPMatrixEdit():
												CGuiAgent_MatrixEdit(EditMode_Move|EditMode_Rot)
{
	CGuiAgent_MatrixEdit::EventEdit dlgtBeginEdit;
	CGuiAgent_MatrixEdit::EventEdit dlgtEdit;
	CGuiAgent_MatrixEdit::EventEdit dlgtEndEdit;

	dlgtBeginEdit.bind(this,&CGuiAgent_CPMatrixEdit::_BeginMatrixEdit);
	dlgtEdit.bind(this,&CGuiAgent_CPMatrixEdit::_MatrixEdit);
	dlgtEndEdit.bind(this,&CGuiAgent_CPMatrixEdit::_EndMatrixEdit);

	SetEventListener(dlgtBeginEdit,dlgtEdit,dlgtEndEdit);

	_bChanging=FALSE;

	ShowSpaceMenu(FALSE);
}

void CGuiAgent_CPMatrixEdit::UpdateBind()
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;

	if (_bChanging)
		return;

	i_math::matrix43f mat;

	CGuiAgent_MatrixEdit::Enable(FALSE);

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (!data)
		return;
	if (dataPathes->iSelCP==-1)
		return;
	if (dataPathes->bRunning)
		return;

	if (dataPathes->IsSelReadOnly())
		return;

	_matWork=data->cps[dataPathes->iSelCP].xfm.getMatrix();

	CGuiAgent_MatrixEdit::Enable(TRUE);
	MatrixEditData med;
	med.matrix=&_matWork;
	CGuiAgent_MatrixEdit::Bind(med);
}


void CGuiAgent_CPMatrixEdit::_BeginMatrixEdit(i_math::matrix43f *mat0)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (_bChanging)
		return;

	_bChanging=TRUE;
}

void CGuiAgent_CPMatrixEdit::_MatrixEdit(i_math::matrix43f *mat)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (!data)
		return;
	if (dataPathes->iSelCP==-1)
		return;
	data->cps[dataPathes->iSelCP].xfm.fromMatrix(*mat);

	ProfilerStart_Recent(CalcFromCPs);
	data->CalcFromCPs();

	ProfilerEnd();

	_Redraw();
}

void CGuiAgent_CPMatrixEdit::_EndMatrixEdit(i_math::matrix43f *mat)
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	_MatrixEdit(mat);

	_bChanging=FALSE;

	//标记为modified,触发保存
	dataPathes->modified=dataPathes->sel;

}

BOOL CGuiAgent_CPMatrixEdit::OnRButtonClick(int x,int y,DWORD flag)
{
	CGuiAgent_MatrixEdit::OnRButtonClick(x,y,flag);
	return TRUE;
}

BOOL CGuiAgent_CPMatrixEdit::OnCommand(DWORD idCmd)
{

	return CGuiAgent_MatrixEdit::OnCommand(idCmd);
}

//////////////////////////////////////////////////////////////////////////
//CCPPage

void CCPPage::Bind(XFormData::CtrlPoint *cp)
{
	if (!cp)
	{
		CGObjGrid::Bind(NULL);
		_info.vel=-1.0f;//mark as invalid
	}
	else
	{
		CPInfo t;
		t.bVelocityAlign=cp->bVelocityAligned;
		t.vel=cp->vel;
		if (t==_info)
			return;

		_info=t;

		CGObjGrid::Bind(_info.GetGObj());
		CGObjGrid::ExpandAll();
	}
}

void CCPPage::Fill(XFormData::CtrlPoint &cp)
{
	if (_info.vel>=0.0f)
	{
		cp.bVelocityAligned=_info.bVelocityAlign;
		cp.vel=_info.vel;
	}
}


void CCPPage::OnEndItemChange(CXTPPropertyGridItem *item)
{
	CGObjGrid::OnEndItemChange(item);
	_bModified=TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Pathes


BEGIN_MESSAGE_MAP(CGuiPanel_Pathes, CGuiPanel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnTvnSelchangedTree)

END_MESSAGE_MAP()


CGuiPanel_Pathes::CGuiPanel_Pathes(CWnd* pParent):
CGuiPanel(IDD_EDITPANEL_PATHES, pParent)
{
	_bInUpdateTreeSel=FALSE;
	_matedit=NULL;
}

BOOL CGuiPanel_Pathes::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_PATHES,pParent);	
}


BOOL CGuiPanel_Pathes::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_TREE,rc);
	HIDE_CONTROL(this,IDC_TREE);

	_tree.Create(this,rc,IDC_TREE);

	GET_CONTROL_RECT(this,IDC_PAGE,rc);
	HIDE_CONTROL(this,IDC_PAGE);

	_page.Create(rc,this,IDC_PAGE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGuiPanel_Pathes::OnDestroy()
{
	_tree.DestroyWindow();

	CGuiPanel::OnDestroy();
}
void CGuiPanel_Pathes::Reset()
{
	EnableWindow(FALSE);

	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;

	//	std::string pathRoot=g_reg.ReadString("GuiPanel_Pathes","RootPath","");

	//根据当前地图的名称,来决定路径存放的根目录
#define PATH_FOLDER "地图路径"
	std::string pathRoot;
	pathRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
	pathRoot=pathRoot+"\\"+PATH_FOLDER;
	if (g_ssGuiLib.pES)
	{
		IMapFile *mf=g_ssGuiLib.pES->GetMapFile();
		if (mf)
		{
			std::string s=mf->GetPath();
			std::string pathMapRoot=g_ssGuiLib.pWS->GetPath(WSPath_Map);
			if (CheckPathContaining(pathMapRoot.c_str(),s.c_str()))
			{
				s=CutHeadPath(s.c_str(),pathMapRoot.c_str());
				pathRoot=pathRoot+"\\"+s;
				if (!g_ssGuiLib.pFS->ExistFolderAbs(pathRoot.c_str()))
				{//如果路径不存在,创建一个
					IFolder *folder=g_ssGuiLib.pFS->OpenFolderAbs(pathRoot.c_str(),FileAccessMode_Write);
					if (folder)
						folder->Close();
				}
			}
		}
	}
	_SetRootPath(pathRoot.c_str());

	_tree.SetSsc(g_ssGuiLib.ssc);

	EnableWindow(TRUE);

}

void CGuiPanel_Pathes::_OccupyActor()
{
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);
	if (view->GetCurActor()!=static_cast<CGeActor*>(this))
	{
		view->DiscardLevels(1);
		view->AttachActor(0,static_cast<CGeActor*>(this));

		//一些通用的agent
		view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
		extern void AddGeneralAgents(CGuiView *view);
		AddGeneralAgents(view);
		view->AddAgent(0,new CGuiAgent_BakeLocal);

		view->AddAgent(0,new CGuiAgent_RunPath,AGENTPRIORITY_STANDARD+20);
		view->AddAgent(0,new CGuiAgent_LocateCP,AGENTPRIORITY_STANDARD+10);

		_matedit=new CGuiAgent_CPMatrixEdit;
		view->AddAgent(0,_matedit,AGENTPRIORITY_STANDARD+7);
		view->AddAgent(0,new CGuiAgent_SelCP,AGENTPRIORITY_STANDARD+5);
		view->AddAgent(0,new CGuiAgent_OperateCP,AGENTPRIORITY_STANDARD);
		view->AddAgent(0,new CGuiAgent_DrawPath);
	}

}

void CGuiPanel_Pathes::OnEnterActivity()
{
	_OccupyActor();
	DEFINE_GUIDATA_PATHES(dataPathes);
	dataPathes->bForceShow=TRUE;
}

void CGuiPanel_Pathes::OnLeaveActivity()
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	dataPathes->bForceShow=FALSE;
}


void CGuiPanel_Pathes::OnDetachView(CGeView *view,DWORD iLevel)
{
	_matedit=NULL;
}

void CGuiPanel_Pathes::_SaveModified()
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	CModManager *modmgr=GetModMgr();
	CMod_ReplacePath *mod=NULL;

	if (dataPathes->modified!="")
	{
		XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->modified.c_str());
		if (data)
		{
			std::string path=dataPathes->pathRoot+"\\"+dataPathes->modified;

			if (modmgr)
			{
				//从硬盘上读取备份
				ResData*data=dataPathes->pUtilRS->LoadRes(path.c_str(),FALSE);
				if (data)
				{
					mod=new CMod_ReplacePath;
					mod->_dataPathes=dataPathes;
					mod->_pathRes=dataPathes->modified;
					mod->_backup.Copy(*data);
					ResData_Delete(data);
				}
			}
			data->MakeDefaultAP();
			if (data->animpieces.size()>0)
			{
				data->animpieces[0].iStart=0;
				data->animpieces[0].iEnd=data->keyset.GetKeyCount();
			}
			dataPathes->pUtilRS->SaveRes(path.c_str(),data);
		}

		if (mod)
		{
			modmgr->NewModGroup();
			modmgr->PushBack(mod,FALSE);
		}

		dataPathes->modified="";
	}

}

void CGuiPanel_Pathes::_UpdateCPPage()
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());

	if (data&&(dataPathes->iSelCP>=0))
	{
		if (_page.IsModified())
		{//从_page里得到修改过的数据,保存到当前选中的CP中去
			_page.Fill(data->cps[dataPathes->iSelCP]);
			_page.ClearModified();
			data->CalcFromCPs();
			dataPathes->modified=dataPathes->sel;
		}
		_page.Bind(&data->cps[dataPathes->iSelCP]);
	}
	else
		_page.Bind(NULL);
}

void CGuiPanel_Pathes::_UpdateTreeSel()
{
	DEFINE_GUIDATA_PATHES(dataPathes);

	_bInUpdateTreeSel=TRUE;

	if (dataPathes->sel!="")
	{
		NodeHandle hSel=_tree.GetCurSel();
		BOOL bNeedUpdate=TRUE;
		if (hSel!=NodeHandle_Null)
		if (StringEqualNoCase(_tree.GetPath(hSel),dataPathes->sel.c_str()))
			bNeedUpdate=FALSE;
		if (bNeedUpdate)
		{
			HTREEITEM hItem=_tree.ItemFromPath(dataPathes->sel);
			if (hItem)
			{
				_tree.SelectAll(FALSE);
				_tree.SelectItem(hItem);
			}
		}
	}
	else
		_tree.SelectAll(FALSE);


	_bInUpdateTreeSel=FALSE;
}

void CGuiPanel_Pathes::_UpdateDesc()
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	std::string desc;
	desc="n/a";

	XFormData *data=(XFormData *)dataPathes->FindData(dataPathes->sel.c_str());
	if (data)
		data->CalcContent(desc);

	CString s;
	GetDlgItemText(IDC_PAGE2,s);
	if (s!=desc.c_str())
		SetDlgItemText(IDC_PAGE2, fromMBCS(desc.c_str()));
}



void CGuiPanel_Pathes::UpdateUI()
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;

	if (_matedit)
		_matedit->UpdateBind();

	if (dataPathes->bLocateCP||dataPathes->bRunning)
		_tree.EnableWindow(FALSE);
	else
		_tree.EnableWindow(TRUE);

	if (dataPathes->bLocateCP||dataPathes->bRunning||dataPathes->IsSelReadOnly())
		_page.SetReadOnly(TRUE);
	else
		_page.SetReadOnly(FALSE);

	_UpdateTreeSel();
	_UpdateCPPage();

	_UpdateDesc();

	if (_tree.IsModified())
	{
		dataPathes->LoadData();
		_tree.ClearModified();
	}

	_tree.Update();

	_SaveModified();
}

void CGuiPanel_Pathes::_SetRootPath(const char *path)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;
	dataPathes->pathRoot=path;
	dataPathes->LoadData();
	_tree.SetContent(path);
	std::string s=g_ssGuiLib.pRS->GetPath(Path_Res);
	s=CutHeadPath(dataPathes->pathRoot.c_str(),s.c_str());
	SetDlgItemText(IDC_ROOTPATH, fromMBCS(s.c_str()));

//	g_reg.WriteString("GuiPanel_Pathes","RootPath",dataPathes->pathRoot.c_str());
}



void CGuiPanel_Pathes::OnBnClickedBrowse()
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;

	CXTBrowseDialog dlg;
	dlg.SetOptions(BIF_DONTGOBELOWDOMAIN);
	dlg.SetTitle(_T("Select the root path:"));

	IRenderSystem *pRS=dataPathes->pUtilRS->GetRS();

	if (dataPathes->pathRoot=="")
		dlg.SetSelPath(fromMBCS(pRS->GetPath(Path_Res)));
	else
		dlg.SetSelPath(fromMBCS(dataPathes->pathRoot.c_str()));

	if (dlg.DoModal() == IDOK)
		_SetRootPath(toMBCS(dlg.GetSelPath()));
}

void CGuiPanel_Pathes::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (_bInUpdateTreeSel)
		return;
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;

	std::string selOld=dataPathes->sel;
	dataPathes->sel="";
	NodeHandle hNode=_tree.GetCurSel();
	if (hNode!=NodeHandle_Null)
	{
		CNodeTree *tree=_tree.GetNodeTree()->GetTree();
		if (tree)
			dataPathes->sel=tree->GetPath(hNode);
	}

	if (selOld!=dataPathes->sel)
		dataPathes->iSelCP=-1;

}

