/********************************************************************
created:	2008/5/20   18:14
file path:	d:\IxEngine\Proj_GuiLib
author:		cxi

purpose:	the gui agent used in editing proto
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IFont.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/ITools.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/ILuaMachine.h"
#include "WorldSystem/IAssetBodyMap.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"


#include "GuiData.h"

#include "GuiData_proto.h"
#include "GuiData_protologic.h"
#include "GuiData_debugger.h"

#include "GuiData_RichGrids.h"

#include "GuiAgent_MatSet.h"

#include "commondefines/general_stl.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetRenderer.h"

#include "timer/profiler.h"
#include "Registry/Registry.h"

#include "AgentCmdID.h"

#include "graphicsgraph.h"

#include "RichGrid.h"

#include "RichGridMatSetItem.h"

//////////////////////////////////////////////////////////////////////////
//RemoveItemData
void RemoteItemData_MatSet::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=bMats;
	if (bMats)
		DP_WriteVector(dp,mats_);
	dp.Data_NextByte()=bVecs;
	if (bVecs)
		DP_WriteVector(dp,vecs);
	dp.Data_NextByte()=bSphs;
	if (bSphs)
		DP_WriteVector(dp,sphs);
	dp.Data_WriteString(mode);
}

void RemoteItemData_MatSet::Load(CDataPacket &dp)
{
	Zero();
	bMats=dp.Data_NextByte();
	if (bMats)
		DP_ReadVector(dp,mats_);
	bVecs=dp.Data_NextByte();
	if (bVecs)
		DP_ReadVector(dp,vecs);
	bSphs=dp.Data_NextByte();
	if (bSphs)
		DP_ReadVector(dp,sphs);
	dp.Data_ReadString(mode);
}



//////////////////////////////////////////////////////////////////////////
//CRemoteItem
extern CCurrentUserRegistry g_reg;
void CRemoteItem_MatSet::Reset()
{
	_uidItem=0;
	_data.Clear();
	_IncVer();

	DP_BeginSave(dp,_bufTemp);
	_data.Save(dp);
	DP_EndSave();

	g_reg.WriteData("[RemoteItem]","data",_bufTemp.data(),_bufTemp.size());
	g_reg.WriteInt("[RemoteItem]","version",_ver);
	g_reg.WriteInt("[RemoteItem]","uid",0);

}

void CRemoteItem_MatSet::Update()
{
	DWORD ver=g_reg.ReadInt("[RemoteItem]","version",0);
	if (ver<=_ver)
		return;//没有新的内容

	_ver=ver;
	_uidItem=g_reg.ReadInt("[RemoteItem]","uid",0);
	_data.Clear();

	if (_uidItem!=0)
	{
		void *data;
		DWORD szData;
		if (g_reg.ReadData("[RemoteItem]","data",data,szData))
		{
			CDataPacket dp;
			dp.SetDataBufferPointer((BYTE*)data);
			_data.Load(dp);
		}
	}
}

void CRemoteItem_MatSet::OnEndChange()
{
	DP_BeginSave(dp,_bufTemp);
	_data.Save(dp);
	DP_EndSave();

	_IncVer();

	g_reg.WriteData("[RemoteItem]","data",_bufTemp.data(),_bufTemp.size());
	g_reg.WriteInt("[RemoteItem]","version",_ver);
}




//////////////////////////////////////////////////////////////////////////
//CGuiAgent_MatSet::Binding
BOOL CGuiAgent_MatSet::Binding::IsValid()
{
	if (remote)
		return TRUE;
	if(grid&&item)
		return TRUE;
	return FALSE;
}

std::vector<i_math::matrix43f> *CGuiAgent_MatSet::Binding::GetBindMats()
{
	if (remote)
		return remote->GetBindMats();
	if (item)
		return item->GetBindMats();
	return NULL;
}

std::vector<i_math::vector3df> *CGuiAgent_MatSet::Binding::GetBindVecs()
{
	if (remote)
		return remote->GetBindVecs();
	if (item)
		return item->GetBindVecs();
	return NULL;
}

std::vector<i_math::spheref> *CGuiAgent_MatSet::Binding::GetBindSphs()
{
	if (remote)
		return remote->GetBindSphs();
	if (item)
		return item->GetBindSphs();
	return NULL;
}

BOOL CGuiAgent_MatSet::Binding::IsLS()
{
	if (remote)
		return remote->IsLS();
	if (item)
		return item->IsLS();
	return FALSE;
}

const char *CGuiAgent_MatSet::Binding::GetMode()
{
	if (remote)
		return remote->GetMode();
	if (item)
		return item->GetMode();
	return "";
}


void CGuiAgent_MatSet::Binding::OnBeginChange()
{
	if (remote)
	{
		remote->OnBeginChange();
		return;
	}
	if (item&&grid)
		grid->OnBeginItemChange(item);
}

void CGuiAgent_MatSet::Binding::OnChange()
{
	if (remote)
	{
		remote->OnChange();
		return;
	}
	if (item&&grid)
		grid->OnItemChange(item);
}

void CGuiAgent_MatSet::Binding::OnEndChange()
{
	if (remote)
	{
		remote->OnEndChange();
		return;
	}
	if (item&&grid)
	{
		item->UpdateValue();
		grid->OnEndItemChange(item);
	}
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_MatSetEdit
void CGuiAgent_MatSet::OnAttachView(CGeView *view,DWORD iLevel)
{
	CGuiAgent_3DNodeMatEdit::OnAttachView(view,iLevel);
	_lgt=g_ssGuiLib.pRS->CreateLight();
	_lgt->SetDirLight(i_math::vector3df(1,0,0),0,ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff));
	_mesh=(IMesh*)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes("_editor\\plane.msh");
	_meshRadius=(IMesh*)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes("_editor\\sphere.msh");
	_mtrl=(IMtrl*)g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes("_editor\\plane.mtl");
	_mtrlRadius=(IMtrl*)g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes("_editor\\sphereAdd.mtl");

//	_itemRemote.Reset();
}

void CGuiAgent_MatSet::OnDetachView(CGeView *view,DWORD iLevel)
{
	SAFE_RELEASE(_lgt);
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_meshRadius);
	SAFE_RELEASE(_mtrl);
	SAFE_RELEASE(_mtrlRadius);

	CGuiAgent_3DNodeMatEdit::OnDetachView(view,iLevel);

}

#define CP_RADIUS 0.2f

void CGuiAgent_MatSet::_DrawCP(i_math::matrix43f &matBase,i_math::matrix43f &mat0,DWORD col)
{
	i_math::matrix43f mat;
	mat.setScale(CP_RADIUS,CP_RADIUS,CP_RADIUS);
	mat=mat*mat0;
	mat*=matBase;
	IRenderPort *rp=GetRP();
	rp->SimpleDrawMesh(_mesh,mat,col,FALSE,_mtrl,_lgt);
}

void CGuiAgent_MatSet::_DrawRadius(i_math::matrix43f &matBase,i_math::vector3df &pos,float radius)
{
	i_math::matrix43f mat;
	mat.setScale(radius,radius,radius);
	*mat.getTranslationP()=pos;
	mat*=matBase;
	IRenderPort *rp=GetRP();
	rp->SimpleDrawMesh(_meshRadius,mat,ColorAlpha(0xff7f3f,0x10),FALSE,_mtrlRadius,NULL);
}

void CGuiAgent_MatSet::_GetBaseMat(i_math::matrix43f &matBase)
{
	matBase.makeIdentity();
	GuiData_Proto *dataProto=(GuiData_Proto *)FindData("proto");
	if (dataProto)
	{
		ProtoNodeID id=dataProto->GetSelNodeID();
		if (id!=ProtoNodeID_Null)
		{
			if (!dataProto->GetNodeMat(id,matBase))
				matBase.makeIdentity();
		}
	}

}

CGuiAgent_MatSet::Binding CGuiAgent_MatSet::_GetCurBinding()
{
	Binding binding;
	if (_itemRemote.IsValid())
		binding.remote=&_itemRemote;
	else
	{
		binding.grid=_GetCurGrid();
		binding.item=_GetCurItem();
	}
	return binding;
}

BOOL CGuiAgent_MatSet::OnDraw()
{
	if (!_bWorking)
		return TRUE;

	IRenderPort *rp=GetRP();

	if (TRUE)
	{
		DrawFontArg arg;
		arg.SetLocation(10,10);
		rp->DrawText(" {C:0,255,0}{S:16}[ 位点编辑模式,左键双击退出 ]",arg);
	}


	if (TRUE)
	{
		LightInfo *li=_lgt->QueryInfo();
		rp->GetCamera()->GetEyeDir(li->dir);
	}

	if (TRUE)
	{
		Binding binding=_GetCurBinding();
		if (binding.IsValid())
		{
			i_math::matrix43f matBase;
			if (binding.IsLS())
				_GetBaseMat(matBase);

			std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
			std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
			std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
			static std::string mode;
			mode=binding.GetMode();
			if (mats||vecs||sphs)
			{
				int sz=0;
				if (mats)
					sz=mats->size();
				if (vecs)
					sz=vecs->size();
				if (sphs)
					sz=sphs->size();
				i_math::vector3df posLast;
				for (int i=0;i<sz;i++)
				{
					int idx;
					VEC_FIND(_sels,i,idx);
					i_math::matrix43f mat;
					if (mats)
						mat=(*mats)[i];
					if (vecs)
						mat.setTranslation((*vecs)[i]);
					if (sphs)
						mat.setTranslation((*sphs)[i].center);
					if (idx!=-1)
						_DrawCP(matBase,mat,ColorAlpha(0xffff00,0xff));
					else
						_DrawCP(matBase,mat,ColorAlpha(0xff00ff,0xff));
					if (TRUE)
					{
						int x,y;
						rp->TransPos(mat.getTranslation(),x,y);
						DrawFontArg arg;
						arg.SetLocation(x,y);
						std::string s;
						FormatString(s,"{C:0,255,0}{S:12}#%02d",i+1);
						rp->DrawText(s.c_str(),arg);
					}
					if (mode=="Route")
					{
						if (i>0)
							rp->Line(mat.getTranslation(),posLast,ColorAlpha(0x00ff00,0xff));
					}

					posLast=mat.getTranslation();
				}
				if (sphs)
				{
					for (int i=0;i<sz;i++)
						_DrawRadius(matBase,(*sphs)[i].center,(*sphs)[i].radius);
				}
			}

		}

	}

	if (_bNewMat)
	{
		if (_bHitPos)
		{
			i_math::vector3df pos=_posHit;
			pos.y+=_hangHit;

			i_math::matrix43f mat;
			mat.setTranslation(pos);

			_DrawCP(*i_math::matrix43f::identity(),mat,ColorAlpha(0xff00ff,0x7f));
		}
	}



	GuiData_Proto*dataProto=(GuiData_Proto*)FindData("proto");
	GuiData_Debugger *dataDebugger=(GuiData_Debugger*)FindData("debugger");
	if (dataProto&&dataDebugger)
	{
		if (!dataDebugger->context->IsRunning())
		{
			if (!dataProto->IsReadOnly())
				CGuiAgent_3DNodeMatEdit::OnDraw();
		}
	}
	else
		CGuiAgent_3DNodeMatEdit::OnDraw();

	return TRUE;

}

BOOL CGuiAgent_MatSet::OnTimer(int dt,DWORD flag)
{
	if (_bEnableRemote)
		_itemRemote.Update();
	_UpdateCur();


	CGuiAgent_3DNodeMatEdit::UpdateBind();

	CGuiAgent::Enable(TRUE);
	return TRUE;
}


void CGuiAgent_MatSet::_UpdateCur()
{
	if (_itemRemote.IsValid())
	{
		_curGrid="";
		_curItem="";
		_bWorking=TRUE;

		DWORD uidRemote=_itemRemote.GetItemUID();
		if (uidRemote!=_curRemoteUID)
		{
			_curRemoteUID=uidRemote;
			_bNewMat=FALSE;
		}
		return;
	}
	GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
	std::string curGrid,curItem;

	CXTPPropertyGridItem *item=NULL;
	if (dataRG)
	{
		curGrid=dataRG->GetCurGrid();

		if (!curGrid.empty())
		{
			CRichGrid *grid=dataRG->FindGrid(curGrid.c_str());
			item=grid->GetSelectedItem();
			if (item)
				curItem=grid->PathFromItem(item);
		}
	}

	if ((curGrid==_curGrid)&&(curItem==_curItem))
		return;//没有任何变化

	if (curGrid=="")
	{//focus不在某一个grid上

		//检查原来的grid是不是还是在选中原来的item
		CRichGrid *grid=dataRG->FindGrid(_curGrid.c_str());
		if (grid)
		{
			std::string s;
			item=grid->GetSelectedItem();
			if (item)
				s=grid->PathFromItem(item);
			if (s==_curItem)
				return;//原来的grid仍然选中原来的item(虽然原来的grid已经失去focus),我们看作没有变化
		}
	}


	//发生了变化
	_curGrid=curGrid;
	_curItem=curItem;

	_bWorking=FALSE;

	if (item)
	{
		if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
			_bWorking=TRUE;
	}

	_bNewMat=FALSE;
}

CRichGrid *CGuiAgent_MatSet::_GetCurGrid()
{
	if (_curGrid.empty())
		return NULL;
	GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
	return dataRG->FindGrid(_curGrid.c_str());
}

CRichGrid_MatSetItem *CGuiAgent_MatSet::_GetCurItem()
{
	CRichGrid *grid=_GetCurGrid();
	if (grid)
	{
		CXTPPropertyGridItem *item=grid->GetSelectedItem();
		if (item)
		{
			if (item->IsKindOf(RUNTIME_CLASS(CRichGrid_MatSetItem)))
			{
				if (_curItem==grid->PathFromItem(item))
					return (CRichGrid_MatSetItem *)item;
			}
		}
	}
	return NULL;
}

int CGuiAgent_MatSet::_NodeHitTest(i_math::line3df &line)
{
	Binding binding=_GetCurBinding();
	if (!binding.IsValid())
		return -1;

	i_math::matrix43f matBase;
	if (binding.IsLS())
		_GetBaseMat(matBase);

	std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
	std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
	std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
	if (!(mats||vecs||sphs))
		return -1;

	int iHit=-1;

	int sz=0;
	if (mats)
		sz=mats->size();
	if (vecs)
		sz=vecs->size();
	if (sphs)
		sz=sphs->size();
	for (int i=0;i<sz;i++)
	{
		i_math::matrix43f mat;
		if (mats)
			mat=(*mats)[i];
		if (vecs)
			mat.setTranslation((*vecs)[i]);
		if (sphs)
			mat.setTranslation((*sphs)[i].center);

		mat*=matBase;
		i_math::spheref sph;
		sph.set(mat.getTranslation(),CP_RADIUS);

		i_math::vector3df out;
		if (sph.getIntersectionWithLine(line,out))
		{
			iHit=i;
			line.end=out;
		}
	}

	return iHit;
}


BOOL CGuiAgent_MatSet::OnLButtonDown(int x,int y,DWORD flag)
{
	_UpdateCur();

	if (!_bWorking)
		return TRUE;

	if (_bNewMat)
	{
		_MakeHitPos(x,y);

		if (_bHitPos)
		{
			Binding binding=_GetCurBinding();
			if (binding.IsValid())
			{
				i_math::matrix43f matBase;
				if (binding.IsLS())
					_GetBaseMat(matBase);
				i_math::matrix43f matBaseInv;
				matBaseInv=matBase;
				matBaseInv.makeInverse();

				binding.OnBeginChange();

				std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
				std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
				std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
				if (mats)
				{
					i_math::matrix43f mat;
					mat.setTranslation(_posHit);
					mat.getTranslationP()->y+=_hangHit;

					mat*=matBaseInv;
					mats->push_back(mat);
					_sels.clear();
					_sels.push_back(mats->size()-1);
				}
				if (vecs)
				{
					i_math::vector3df vec=_posHit;
					vec.y+=_hangHit;
					matBaseInv.transformVect(vec,vec);
					vecs->push_back(vec);
					_sels.clear();
					_sels.push_back(vecs->size()-1);
				}
				if (sphs)
				{
					i_math::spheref sph;
					sph.center=_posHit;
					sph.radius=1.0f;
					sph.center.y+=_hangHit;
					matBaseInv.transformSphere(sph,sph);
					sphs->push_back(sph);
					_sels.clear();
					_sels.push_back(sphs->size()-1);
				}

				binding.OnChange();
				binding.OnEndChange();
			}
		}

		return FALSE;
	}

	if (TRUE)
	{
		IRenderPort *rp=GetRP();
		_bHitPos=FALSE;
		HitProbe probe;
		if (rp->CalcHitProbe(x,y,probe))
		{
			int iHit=_NodeHitTest(probe);

			BOOL bCtrlDown=flag&CtrlOpFlag_CtrlDown;

			if (!bCtrlDown)
			{
				_sels.clear();
				if (iHit!=-1)
				_sels.push_back(iHit);
			}
			else
			{
				if (iHit!=-1)
				{
					int idx;
					VEC_FIND(_sels,iHit,idx);
					if (idx==-1)
						_sels.push_back(iHit);
					else
						_sels.erase(_sels.begin()+idx);
				}
			}
		}
	}

	return FALSE;
}

BOOL CGuiAgent_MatSet::OnLButtonDblClk(int x,int y,DWORD flag)
{
	_UpdateCur();

	if (!_bWorking)
		return TRUE;

	IRenderPort *rp=GetRP();
	HitProbe probe;
	if (rp->CalcHitProbe(x,y,probe))
	{
		int iHit=_NodeHitTest(probe);
		if (iHit==-1)
		{
			_curGrid="";
			_curItem="";
			_itemRemote.Reset();
			_bWorking=FALSE;

			_UpdateCur();
			return FALSE;
		}
	}

	return CGuiAgent_3DNodeMatEdit::OnLButtonDblClk(x,y,flag);
}


BOOL CGuiAgent_MatSet::OnLButtonUp(int x,int y,DWORD flag)
{
	return TRUE;
}


BOOL CGuiAgent_MatSet::OnRButtonClick(int x,int y,DWORD flag)
{
	if (!_bWorking)
		return TRUE;

	if (_bNewMat)
	{
		_bNewMat=FALSE;
		return FALSE;
	}


	_AddMenu("新建位点",ID_AGENT_MATSET_NEW);

	if (_sels.size()>0)
		_AddMenu("删除位点",ID_AGENT_MATSET_REMOVE);

	_AddMenu("选中全部位点",ID_AGENT_MATSET_SELALL);
	_AddMenu("清除全部位点",ID_AGENT_MATSET_CLEARALL);



	CGuiAgent_3DNodeMatEdit::OnRButtonClick(x,y,flag);

	return FALSE;

}

BOOL CGuiAgent_MatSet::OnCommand(DWORD idCmd)
{
	if (!_bWorking)
		return TRUE;

	if (idCmd==ID_AGENT_MATSET_NEW)
	{
		_bNewMat=TRUE;
		return FALSE;
	}

	if (idCmd==ID_AGENT_MATSET_REMOVE)
	{
		_UpdateCur();

		Binding binding=_GetCurBinding();

		if (binding.IsValid())
		{
			binding.OnBeginChange();

			std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
			std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
			std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
			if (mats)
			{
				VEC_ASCEND(_sels,H3DNode);
				for (int i=_sels.size()-1;i>=0;i--)
				{
					if (((int)_sels[i])<mats->size())
						mats->erase(mats->begin()+((int)_sels[i]));
				}
			}
			if (vecs)
			{
				VEC_ASCEND(_sels,H3DNode);
				for (int i=_sels.size()-1;i>=0;i--)
				{
					if (((int)_sels[i])<vecs->size())
						vecs->erase(vecs->begin()+((int)_sels[i]));
				}
			}
			if (sphs)
			{
				VEC_ASCEND(_sels,H3DNode);
				for (int i=_sels.size()-1;i>=0;i--)
				{
					if (((int)_sels[i])<sphs->size())
						sphs->erase(sphs->begin()+((int)_sels[i]));
				}
			}

			_sels.clear();

			binding.OnChange();
			binding.OnEndChange();
		}
		return FALSE;
	}


	if (idCmd==ID_AGENT_MATSET_SELALL)
	{
		_UpdateCur();
		Binding binding=_GetCurBinding();
		if (binding.IsValid())
		{
			std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
			std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
			std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
			if (mats)
				_sels.resize(mats->size());
			if (vecs)
				_sels.resize(vecs->size());
			if (sphs)
				_sels.resize(sphs->size());

			if (mats||vecs||sphs)
			{
				for (int i=0;i<_sels.size();i++)
					_sels[i]=i;
			}

		}
		return FALSE;
	}
	if (idCmd==ID_AGENT_MATSET_CLEARALL)
	{
		Binding binding=_GetCurBinding();
		if (binding.IsValid())
		{
			binding.OnBeginChange();

			std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
			std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
			std::vector<i_math::spheref> *sphs=binding.GetBindSphs();

			if (mats)
				mats->clear();
			if (vecs)
				vecs->clear();
			if (sphs)
				sphs->clear();

			_sels.clear();

			binding.OnChange();
			binding.OnEndChange();
		}
		return FALSE;
	}

	if (idCmd==ID_AGENT_3DNODEEDIT_ALIGNTOSURF)
	{
		Binding binding=_GetCurBinding();
		if (binding.IsValid())
		{
			binding.OnBeginChange();

			extern IEntity *CreateGuiDataProto(CGuiAgent *agent);
			extern void DestroyGuiDataProto(IEntity *en,CGuiAgent *agent);
			IEntity *en=CreateGuiDataProto(this);
			BOOL bRet=CGuiAgent_3DNodeMatEdit::OnCommand(idCmd);
			DestroyGuiDataProto(en,this);

			binding.OnChange();
			binding.OnEndChange();
			return bRet;
		}

		return FALSE;
	}


	if (FALSE==CGuiAgent_3DNodeMatEdit::OnCommand(idCmd))
		return FALSE;

	return TRUE;
}

void CGuiAgent_MatSet::_MakeHitPos(int x,int y)
{
	IRenderPort *rp=GetRP();
	if (!rp)
		return;

	_bHitPos=FALSE;
	HitProbe probe;
	if (rp->CalcHitProbe(x,y,probe))
	{
		if (TRUE)
		{
			extern IEntity *CreateGuiDataProto(CGuiAgent *agent);
			extern void DestroyGuiDataProto(IEntity *en,CGuiAgent *agent);
			IEntity *entity=CreateGuiDataProto(this);
			ProtoNodeID idHit=ProtoNodeID_Null;
			if (entity)
				idHit=entity->ProtoNodeHitTest(probe);
			DestroyGuiDataProto(entity,this);

			if (idHit!=ProtoNodeID_Null)
			{
				_bHitPos=TRUE;
				_posHit=probe.end;
			}
		}

		if (!_bHitPos)
		{
			i_math::vector3df posHit;
			if (g_ssGuiLib.pES->GetAS()->GetBodyMap()->StaticHitTest(probe,posHit))
			{
				_bHitPos=TRUE;
				_posHit=posHit;
			}
			else
			{
				i_math::plane3df pl;
				if (pl.getIntersectionWithLimitedLine(probe.start,probe.end,posHit))
				{
					_bHitPos=TRUE;
					_posHit=posHit;
				}
			}
		}
	}

}


BOOL CGuiAgent_MatSet::OnMouseMove(int x,int y,DWORD flag)
{
	if (!_bWorking)
		return TRUE;

	if (_bNewMat)
	{
		_MakeHitPos(x,y);
	
		return FALSE;
	}

	return TRUE;
}


i_math::matrix43f *CGuiAgent_MatSet::_GetMat(H3DNode node)
{
	_UpdateCur();
	Binding binding=_GetCurBinding();
	if (binding.IsValid())
	{
		i_math::matrix43f matBase;
		if (binding.IsLS())
			_GetBaseMat(matBase);

		std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
		std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
		std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
		if (mats)
		{
			if (node>=mats->size())
				return NULL;
			_matTemp=(*mats)[(DWORD)node];
			_matTemp*=matBase;
			return &_matTemp;
		}
		if (vecs)
		{
			if (node>=vecs->size())
				return NULL;
			_matTemp.makeIdentity();
			_matTemp.setTranslation((*vecs)[(DWORD)node]);
			_matTemp*=matBase;
			return &_matTemp;
		}
		if (sphs)
		{
			if (node>=sphs->size())
				return NULL;
			_matTemp.makeIdentity();
			_matTemp.setScale((*sphs)[(DWORD)node].radius,(*sphs)[(DWORD)node].radius,(*sphs)[(DWORD)node].radius);
			(*_matTemp.getTranslationP())=((*sphs)[(DWORD)node].center);
			_matTemp*=matBase;
			return &_matTemp;
		}
	}

	return NULL;
}

i_math::matrix43f *CGuiAgent_MatSet::_GetLocalMat(H3DNode node,i_math::matrix43f &matParent)
{
	return NULL;
}


void CGuiAgent_MatSet::_Move(H3DNode &node,i_math::matrix43f &mat0)
{
	_UpdateCur();
	Binding binding=_GetCurBinding();
	if (binding.IsValid())
	{
		i_math::matrix43f mat=mat0;
		i_math::matrix43f matBaseInv;
		if (binding.IsLS())
			_GetBaseMat(matBaseInv);
		matBaseInv.makeInverse();
		mat*=matBaseInv;

		std::vector<i_math::matrix43f> *mats=binding.GetBindMats();
		std::vector<i_math::vector3df> *vecs=binding.GetBindVecs();
		std::vector<i_math::spheref> *sphs=binding.GetBindSphs();
		if (mats)
		{
			if (node<mats->size())
			{
				(*mats)[(DWORD)node]=mat;

				binding.OnChange();
			}
		}
		if (vecs)
		{
			if (node<vecs->size())
			{
				(*vecs)[(DWORD)node]=mat.getTranslation();
				binding.OnChange();
			}
		}
		if (sphs)
		{
			if (node<sphs->size())
			{
				(*sphs)[(DWORD)node].center=mat.getTranslation();
				(*sphs)[(DWORD)node].radius=mat.getScaleX();
				binding.OnChange();
			}
		}
	}

}

void CGuiAgent_MatSet::_MoveLocal(H3DNode &node,i_math::matrix43f &mat)
{
}


void CGuiAgent_MatSet::_BeginMatrixEdit(i_math::matrix43f *mat)
{
	CGuiAgent_3DNodeMatEdit::_BeginMatrixEdit(mat);

	_UpdateCur();
	Binding binding=_GetCurBinding();

	if (binding.IsValid())
		binding.OnBeginChange();

}

void CGuiAgent_MatSet::_EndMatrixEdit(i_math::matrix43f *mat)
{
	CGuiAgent_3DNodeMatEdit::_EndMatrixEdit(mat);

	_UpdateCur();
	Binding binding=_GetCurBinding();

	if (binding.IsValid())
	{
		binding.OnEndChange();
	}
}


