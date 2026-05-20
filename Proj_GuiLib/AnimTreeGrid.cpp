/********************************************************************
	created:	2010/4/19   14:06
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	a grid to edit the content of a AnimTreeData
*********************************************************************/
#include "stdh.h"

#include "AnimTreeGrid.h"

#include "RenderSystem/IAnim.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderSystem.h"

#include "RenderSystem/IUtilRS.h"

#include "RichGridComboItem.h"


#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include "AnimTreeEditPanel.h"

#include "resdata/MeshData.h"

#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//CAnimTreeGrid
BEGIN_MESSAGE_MAP(CAnimTreeGrid,CGObjGrid)
END_MESSAGE_MAP()


void CAnimTreeGrid::Bind(ResEditPanelState *state0,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state0,bUpdateCtrl);

	if (bUpdateCtrl)
	{
		AnimTreeData *data=GetResData();
		if (!data)
		{
			UnLockPaint();
			return;
		}

		_objs.clear();
		LockPaint();
		RecordState(CGObjGrid::_state);
		ResetContent();
		BeginInsert();

		CXTPPropertyGridItem*item;

		item=InsertCategory("Preview","",NULL);
		PushInsert();
		_Bind(data->preview.GetGObj(),GSem(GSem_Unknown));
		_objs.push_back(data->preview.GetGObj());
		PopInsert();
		item->Expand();

		item=InsertCategory("Parameters","",NULL);
		PushInsert();
		_Bind(data->params.GetGObj(),GSem(GSem_Unknown));
		_objs.push_back(data->params.GetGObj());
		PopInsert();
		item->Expand();

		Reps_AnimTree *state=(Reps_AnimTree *)state0;

		if (state->sels.size()==1)
		{
			CLinkPad *pad=data->pads.FindPad(state->sels[0]);
			if (pad)
			{
				item=InsertCategory(pad->GetTypeName(),"",NULL);
				PushInsert();
				_Bind(pad->GetGObj(),GSem(GSem_Unknown));
				_objs.push_back(pad->GetGObj());
				PopInsert();
				_ExpandItemR(item);
			}
		}

		EndInsert();
		RestoreState(CGObjGrid::_state);
		UnLockPaint();
	}
}

void CAnimTreeGrid::OnBeginItemChange(CXTPPropertyGridItem *item)
{
//	CGObjGrid::OnBeginItemChange(item);
	Reps_AnimTree *state=GetState();
	if (state->sels.size()==1)
	{
		AnimTreeData *data=GetResData();
		data->pads.PreModify(state->sels[0]);
	}
	
}

void CAnimTreeGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	Reps_AnimTree *state=GetState();
	if (state->sels.size()==1)
	{
		AnimTreeData *data=GetResData();
		data->pads.PostModify();
	}

	RefreshMod(FALSE);

// 	CGObjGrid::OnEndItemChange(item);
}



void CAnimTreeGrid::EnableCtrl(BOOL bActive)
{
	if(bActive)
		SetReadOnly(FALSE);
	else
		SetReadOnly(TRUE);
}


CXTPPropertyGridItem *CAnimTreeGrid::InsertVar(void *var,const char *cap,const char *desc,GVarType vt,GSem &sem)
{
	AnimTreeData *data=(AnimTreeData *)GetState()->resdata;
	if (data)
	{
		if (vt==GVT_String)
		{
			std::vector<std::string> constraints;
			if (sem.code==GSem_Name)
			{
				if (sem.constraint=="BoneName")
				{
					IBoneAnim *anim=(IBoneAnim*)g_ssGuiLib.pRS->GetBoneAnimMgr()->ObtainRes(data->preview.anim.c_str());
					if (anim)
					{
						if (anim->ForceTouch())
						{
							ISkeleton *skl=anim->GetSkeleton();
							if (skl)
							{
								SkeletonInfo*info=skl->GetSkeletonInfo();
								constraints.resize(info->size());
								for (int i=0;i<constraints.size();i++)
									constraints[i]=(*info)[i].name;
							}
						}
						SAFE_RELEASE(anim);
					}
				}
			}
			if (constraints.size()>0)
				return InsertComboItem(cap,desc,(std::string*)var,constraints);
		}
		if (vt==GVT_U)
		{
			std::vector<std::string> constraints;
			std::string s;
			if (sem.code==GSem_StringID)
			{
				if ((sem.constraint=="骨骼动画AnimPiece"))
				{
					for (int i=0;i<data->preview.animsAddon.size()+1;i++)
					{
						IBoneAnim *anim=NULL;
						if (i==0)
							anim=(IBoneAnim*)g_ssGuiLib.pRS->GetBoneAnimMgr()->ObtainRes(data->preview.anim.c_str());
						else
							anim=(IBoneAnim*)g_ssGuiLib.pRS->GetBoneAnimMgr()->ObtainRes(data->preview.animsAddon[i-1].c_str());
						if (anim)
						{
							if (anim->ForceTouch())
							{
								DWORD c=anim->GetAnimPieceCount();
								for (int i=0;i<c;i++)
								{
									StringID name=anim->GetAnimPieceName(i);
									FormatString(s,"%s:%d",StrLib_GetStr(name),(int)name);
									constraints.push_back(s);
								}
							}
							SAFE_RELEASE(anim);
						}
					}
				}
				if (sem.constraint=="SyncGroup")
				{
					std::string s;
					constraints.push_back(std::string(" :0"));
					for (int i=0;i<data->params.grps.size();i++)
					{
						StringID name=data->params.grps[i].name;
						FormatString(s,"%s:%d",StrLib_GetStr(name),(int)name);
						constraints.push_back(s);
					}
				}
			}
			if (constraints.size()>0)
				return InsertComboItem(cap,desc,(DWORD*)var,constraints);
		}
	}
	return CGObjGrid::InsertVar(var,cap,desc,vt,sem);
}
