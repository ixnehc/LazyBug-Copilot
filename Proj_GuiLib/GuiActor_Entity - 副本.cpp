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
#include "WMGuiLib.h"

#include "stringparser/stringparser.h"
#include "commondefines/general_stl.h"

#include "GuiData_entitymap.h"

 

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IBehaviorGraph.h"
#include "RenderSystem/IRecords.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/stubparams/param_sys.h"
#include "WorldSystem/assetcore/asset.h"
#include "WorldSystem/IAssetBodyMap.h"
#include "WorldSystem/IAssetShell.h"
#include "FileSystem/IMapFile.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"
#include "GuiActor_Entity.h"

#include "Log/LogDump.h"

#include "timer/profiler.h"
#include "rasterize/rasterize.h"

#include "Random/Random.h"

#include "records/recordsdefine.h"
#include "records/records.h"


#include "GuiAgent_MatSet.h"
#include "GuiAgent_AstUIDSet.h"
#include "GuiData_RichGrids.h"

#include "SscUID.h"

#include "ximage.h"


//////////////////////////////////////////////////////////////////////////
//CMod_ChangeEntityMap

CMod_ChangeEntityMap::CMod_ChangeEntityMap(CGeView * view):
							CModBlockBack(view)
{
	_bFirstTime=FALSE;
}

void CMod_ChangeEntityMap::BackupSelection()
{
	OnBackup();
}

void CMod_ChangeEntityMap::OnBackup()
{
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");

	CDataPacket dp;
	std::vector<BYTE>buf;
	DP_BeginSave(dp,buf);
		DP_WriteVector(dp,data->selections);
	DP_EndSave();

	SetAddOnData(&buf[0],buf.size());
}

void CMod_ChangeEntityMap::OnRestore()
{
	GuiData_EntityMap *dataMap=(GuiData_EntityMap *)_view->FindData("entitymap");

	int size;
	void *data=GetAddOnData(size);

	CDataPacket dp;
	dp.SetDataBufferPointer((BYTE*)data);
	DP_ReadVector(dp,dataMap->selections);
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ResideEntity

BOOL FindStaticResidePos(i_math::line3df &line,i_math::vector3df &posHit,BOOL bGoundOnly)
{
	IAssetBodyMap *bodymap=g_ssGuiLib.pES->GetAS()->GetBodyMap();
	BOOL bHit=FALSE;
	if (g_ssGuiLib.pES->FindTrrn())
	{
		if (bodymap->TrrnHitTest(line,posHit))
		{
			line.end=posHit;
			bHit=TRUE;
		}
	}
	if (!bGoundOnly)
	{
		if(g_ssGuiLib.pES->HitTestOnMap(line))
			bHit=TRUE;
	}

	if (!bHit)
	{
		i_math::plane3df pl(0,0,0,0,1,0);

		i_math::vector3df pos;
		if (pl.getIntersectionWithLine(line.start,line.getVector(),pos))
		{
			line.end=pos;
			bHit=TRUE;
		}
	}

	if (bHit)
		posHit=line.end;
	return bHit;
}

BOOL CGuiAgent_ResideEntity::_FindResidePos(i_math::pos2di &ptCursor,
																				i_math::matrix43f &matReside)
{
	matReside.makeIdentity();
	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)FindData("terrain");
	if (!dataTrrn)
		return FALSE;

	i_math::recti rc;
	_GetClientRect(rc);

	BOOL bCanReside=FALSE;
	if (TRUE)
	{
		IRenderPort *rp=GetRP();
		if (!rp)
			return FALSE;

		HitProbe probe;
		if (rp->CalcHitProbe(ptCursor.x,ptCursor.y,probe))
		{
			i_math::vector3df vReside;
			if (FindStaticResidePos(probe,vReside,_bResideOnGround))
			{
				matReside.setTranslation(vReside);
				bCanReside=TRUE;
			}
		}
	}
	return bCanReside;
}

void CGuiAgent_ResideEntity::_ModResideMat(i_math::matrix43f &mat)
{
	i_math::xformf xfm;
	if (!_bRandomRotate)
	{
		xfm.rot.fromAngleAxis(_angle,_rotAxis);
		xfm.rot=_rotCache*xfm.rot;
	}
	else
		xfm.rot.fromAngleAxis(_angleRandom,i_math::vector3df(0,1,0));
	xfm.scale_=_scale;
	xfm.pos.y=_offVer;


	mat=xfm.getMatrix()*mat;
}

IEntity* CGuiAgent_ResideEntity::_TryAlign(i_math::matrix43f &mat,IEntity *enSrc)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	assert(data);

	IAsset *aligners[16];
	DWORD nAligners=enSrc->FindAsset("AstAligner",aligners,ARRAY_SIZE(aligners));

	if (nAligners<=0)
		return enSrc;//不需要align

	i_math::matrix43f matSrc[16];
	DWORD nSrcMat;
	for (int i=0;i<nAligners;i++)
		aligners[i]->GetXForm(matSrc[i]);
	nSrcMat=nAligners;

	IEntity *ens[1024];
	DWORD nEns;
	nEns=data->mp->EnumEntities(mat.getTranslationP()->x,mat.getTranslationP()->z,80.0f,ens,ARRAY_SIZE(ens));

	BOOL bFound=FALSE;
	i_math::matrix43f matFrom,matTo;
	float distMin=100000000.0f;

	for (int i=0;i<nEns;i++)
	{
		IEntity *enTarget=ens[i];
		if (enSrc==enTarget)
			continue;

		DWORD nAligners=enTarget->FindAsset("AstAligner",aligners,ARRAY_SIZE(aligners));

		for (int j=0;j<nSrcMat;j++)
		{
			for (int k=0;k<nAligners;k++)
			{
				i_math::matrix43f mat2;
				aligners[k]->GetXForm(mat2);

				float dist=matSrc[j].getTranslationP()->getDistanceFrom(*mat2.getTranslationP());
				if (dist<4.0f)
				{
					if (dist<distMin)
					{
						distMin=dist;
						matFrom=matSrc[j];
						matTo=mat2;
						bFound=TRUE;
					}
				}
			}
		}
	}

	if (!bFound)
		return enSrc;//不需要align

	i_math::matrix43f matTrans;
	if (TRUE)
	{
		matTrans=matFrom;
		matTrans.makeInverse();
		matTrans*=matTo;
	}

	i_math::matrix43f matNew;
	matNew=mat;
	matNew*=matTrans;

	enSrc->Destroy();

	enSrc=data->pES->CreateEntity(matNew,_protoid);

	mat=matNew;

	return enSrc;
}


BOOL CGuiAgent_ResideEntity::OnTimer(int dt,DWORD flag)
{
	GuiData_System *dataSystem=(GuiData_System*)FindData("system");
	assert(dataSystem);

	if (_nIgnore>0)
		_nIgnore--;
	if (_nIgnore>0)
		return TRUE;

	SAFE_DESTROY(_entity);

	i_math::pos2di pt;
	_GetCursorPos(pt);

	i_math::vector3df rotAxis(0,1,0);

	if (!_bHold)
	{
		if ((flag&CtrlOpFlag_CtrlDown)||(flag&CtrlOpFlag_ShiftDown))
		{
			_bHold=TRUE;
			if (flag&CtrlOpFlag_CtrlDown)
				_bCtrlOrShift=TRUE;
			else
				_bCtrlOrShift=FALSE;
			_ptHoldStart=pt;
			_angleStart=_angle;
			_scaleStart=_scale;

			_bMoingVer=FALSE;

			GetCursorPos((LPPOINT)&_ptCursorPosStart);
			GetWnd()->SetFocus();
		}
	}
	else
	{
		if (!((flag&CtrlOpFlag_CtrlDown)||(flag&CtrlOpFlag_ShiftDown)))
		{
			_bHold=FALSE;
//			if (_bCtrlOrShift)
			{
				SetCursorPos(_ptCursorPosStart.x,_ptCursorPosStart.y);
				pt=_ptHoldStart;
				i_math::quatf rot;
				rot.fromAngleAxis(_angle,_rotAxis);
				_rotCache=_rotCache*rot;
				_angle=0.0f;
			}
		}
		else
		{
			if (_bCtrlOrShift)
			{
				_angle=-((float)(pt.x-_ptHoldStart.x))*0.01f;
				_rotAxis.set(0,1,0);
			}
			else
			{
				i_math::pos2di off=pt-_ptHoldStart;
				float dist=sqrt((float)(off.x*off.x+off.y*off.y));
				_angle=-dist*0.01f;
				_rotAxis.set(0,1,0);
				if (dist>=0.01f)
				{
					IRenderPort *rp=GetRP();
					if(rp)
					{
						i_math::recti rc;
						rp->GetRect(rc);
						HitProbe v1,v2;
						rp->CalcHitProbe(rc.getCenter().x,rc.getCenter().y,v1);
						rp->CalcHitProbe(rc.getCenter().x+off.x,rc.getCenter().y+off.y,v2);

						_rotAxis=v1.getVector().crossProduct(v2.getVector());
						_rotAxis.normalize();
					}
				}
				if (_bMoingVer)
				{
					_angle=0.0f;
					_rotAxis.set(0,1,0);
				}

			}
		}
	}

	if (_bHold)
		pt=_ptHoldStart;


	i_math::matrix43f matReside;
	if ((_FindResidePos(pt,matReside))&&(_protoid!=ProtoID_Null))
	{
		_ModResideMat(matReside);
		//Create new one
		_entity=dataSystem->pES->CreateEntity(matReside,_protoid);

		if (_bAutoAlign)
			_entity=_TryAlign(matReside,_entity);
	}

	_Redraw(FALSE);

	return TRUE;
}

// void DoTrrnImprint(EntityAddress *addrs,DWORD nAddrs)
// {
// 	IEntityMap *mp=g_gs.pES->GetMap();
// 
// 	IAsset *asts[256];
// 
// 	for (int i=0;i<nAddrs;i++)
// 	{
// 		IEntity *en=mp->ToEntity(addrs[i]);
// 		if (en)
// 		{
// 			DWORD c=en->FindAsset("AstTrrnImprint",asts,ARRAY_SIZE(asts));
// 			for (int i=0;i<c;i++)
// 			{
// 				g_gs.pES->
// 			}
// 		}
// 	}
// 
// }

extern BOOL RepairAssetUID(IEntity *en,BOOL bForceGen);

BOOL CGuiAgent_ResideEntity::OnLButtonDown(int x,int y,DWORD flag)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	assert(data);
	i_math::matrix43f matReside;
	i_math::pos2di pt=i_math::pos2di(x,y);
	if (_bHold&&_bCtrlOrShift)
		pt=_ptHoldStart;

	SAFE_DESTROY(_entity);

	if ((_FindResidePos(pt,matReside))&&(_protoid!=ProtoID_Null))
	{
		_ModResideMat(matReside);

		if (_bAutoAlign)
		{
			IEntity *en=data->pES->CreateEntity(matReside,_protoid);
			en=_TryAlign(matReside,en);
			SAFE_DESTROY(en);
		}

		_angleRandom=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		IEntityParam *ep=data->pES->CreateEntityParam();
		ep->SetProtoID(_protoid);
		EntityAddress addr=data->mp->Reside(matReside,ep);

		//修补AssetUID
		if (addr!=EntityAddress_Null)
		{
			IEntity *en=data->mp->ToEntity(addr);
			if (en)
				RepairAssetUID(en,FALSE);
		}

		if (addr!=EntityAddress_Null)
		{
			CMod_ChangeEntityMap *mod=NULL;

			if (_GetModMgr())
			{
				i_math::pos2di ptBlk;
				data->mp->ResolveBlockPos(addr,ptBlk);

				mod=new CMod_ChangeEntityMap(GetView());
				mod->BackupBlocks(&ptBlk,1);
				mod->BackupSelection();
			}

			//do the actual modifying
			data->pES->SaveToMap();
			data->ClearSelection();
			data->AddSelection(addr);

			if (_GetModMgr())
			{
				_GetModMgr()->NewModGroup();
				_GetModMgr()->PushBack(mod,FALSE);
			}
		}
		else
		{
			SAFE_RELEASE(ep);
		}
	}

	return TRUE;
}

BOOL CGuiAgent_ResideEntity::OnMouseWheel(int delta,DWORD flag)
{
	if (_bHold)
	{
		if (_bCtrlOrShift)
		{
			_scale=_scale+_scaleStart*((float)delta)/1000.0f;
			if (_scale<_scaleStart*0.01f)
				_scale=_scaleStart*0.01f;
		}
		else
		{
			_offVer+=((float)delta)/1000.0f;
			_bMoingVer=TRUE;
		}

		return FALSE;//无需后续处理
	}

	return TRUE;
}

BOOL CGuiAgent_ResideEntity::OnKeyDown(char c,DWORD flag)
{
	if (c==27)
	{
		_scale=1.0f;
		_angle=0.0f;
		_offVer=0.0f;
		_rotCache.set(0,0,0,1);
		_Redraw(FALSE);
		return FALSE;
	}

	if (c==32)
	{
		_angleRandom=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		_Redraw(FALSE);
		return FALSE;
	}

	return TRUE;
}



BOOL CGuiAgent_ResideEntity::OnRButtonClick(int x,int y,DWORD flag)
{
	_DetachActor();
	return FALSE;
}



void CGuiAgent_ResideEntity::OnDetachView(CGeView *view,DWORD iLevel)
{
	if (_entity)
		_entity->Destroy();
	_entity=NULL;
	_Redraw(FALSE);
}


BOOL CGuiAgent_ResideEntity::OnDraw()
{
	return TRUE;
}

BOOL CGuiAgent_ResideEntity::OnSetCursor(int x,int y,DWORD flag)
{
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_OperateEntity
GuiLib_Api RecordID SeekMapRecordID(IWorldSystem *pWS,IMapFile *mf,std::string &nmMap)
{
	std::string pathRoot=pWS->GetPath(WSPath_Map);
	std::string path=mf->GetPath();
	std::string nm=CutHeadPath(path.c_str(),pathRoot.c_str());
	RemoveFileSuffix(nm);

	IRecords *records=(IRecords *)pWS->GetRS()->GetRecordsMgr()->ObtainRes("maps.rcs");
	if (records)
	{
		CRecords *recs=records->GetRecords();
		if (recs)
		{
			GElemBase *elemName=recs->FindElem("Name");
			GElemBase *elem=recs->FindElem("path");
			if (elem)
			{
				DWORD c;
				RecordID *ids=recs->GetRecords(c);
				for (int i=0;i<c;i++)
				{
					CRecord *rec=recs->GetRecord(ids[i]);
					if (rec)
					{
						void *var;
						if (elem->GetVar(rec->GetGObj()->GetOwner(),&var))
						{
							std::string *p=(std::string*)var;
							if (p)
							{
								if ((*p)==nm)
								{
									SAFE_RELEASE(records);

									if (elemName->GetVar(rec->GetGObj()->GetOwner(),&var))
									{
										std::string *p=(std::string*)var;
										nmMap=*p;
										return ids[i];
									}
								}
							}
						}
					}
				}
			}
		}
	}
	SAFE_RELEASE(records);

	return RecordID_Invalid;
}

RecordID SeekMapRecordIDFromLevelAI(StringID nmAI,std::string &nmMap)
{
	IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes("maps.rcs");
	if (records)
	{
		CRecords *recs=records->GetRecords();
		if (recs)
		{
			GElemBase *elemName=recs->FindElem("Name");
			GElemBase *elem=recs->FindElem("ais");
			if (elem)
			{
				DWORD c;
				RecordID *ids=recs->GetRecords(c);
				for (int i=0;i<c;i++)
				{
					CRecord *rec=recs->GetRecord(ids[i]);
					if (rec)
					{
						void *var;
						DWORD nSub;
						if (elem->GetSubCount(rec->GetGObj()->GetOwner(),&nSub))
						{
							for (int j=0;j<nSub;j++)
							{
								if (elem->GetSubVar(rec->GetGObj()->GetOwner(),j,&var))
								{
									StringID*p=(StringID*)var;
									if (*p!=nmAI)
										continue;

									SAFE_RELEASE(records);

									if (elemName->GetVar(rec->GetGObj()->GetOwner(),&var))
									{
										std::string *p=(std::string*)var;
										nmMap=*p;
										return ids[i];
									}
								}
							}
						}
					}
				}
			}
		}
	}
	SAFE_RELEASE(records);

	return RecordID_Invalid;
}


BOOL CGuiAgent_OperateEntity::_GetAgentInfo(RecordID &idMap,std::string &nmMap,
												RecordID &idAgent,std::string &nmAgent,DWORD &guid)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return FALSE;
	if (data->selections.size()==1)
	{
		EntityAddress addr=data->selections[0];
		if (addr!=EntityAddress_Null)
		{
			IEntity *en=data->mp->ToEntity(addr);
			if (en)
			{
				IProto *proto=en->GetProto();
				extern DWORD SeekAgentGUID(IEntity *en);
				extern RecordID SeekAgentRecordID(IProto *proto);
				extern BOOL SeekAgentName(IProto *proto,std::string &nm);

				idMap=SeekMapRecordID(data->pES->GetWS(),data->mf,nmMap);
				idAgent=SeekAgentRecordID(proto);

				if ((idAgent!=RecordID_Invalid)&&(idMap!=RecordID_Invalid))
				{
					SeekAgentName(proto,nmAgent);
					guid=SeekAgentGUID(en);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


BOOL CGuiAgent_OperateEntity::OnRButtonClick(int x,int y,DWORD flag)
{
	CGuiAgent_3DNodeOperate::OnRButtonClick(x,y,flag);

	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return TRUE;

	std::string nmMap;
	RecordID idMap;
	RecordID idAgent;
	std::string nmAgent;
	DWORD guid;

	if (_GetAgentInfo(idMap,nmMap,idAgent,nmAgent,guid))
	{
		_AddMenuSep();
		std::string s;
		FormatString(s,"复制[%s,%s(%08X)]",nmMap.c_str(),nmAgent.c_str(),guid);
		_AddMenu(s.c_str(),ID_AGENT_COPY_AGENTREF);
	}

	return TRUE;
}

BOOL CGuiAgent_OperateEntity::OnCommand(DWORD idCmd)
{

	if (idCmd==ID_AGENT_COPY_AGENTREF)
	{
		std::string nmMap;
		RecordID idMap;
		RecordID idAgent;
		std::string nmAgent;
		DWORD guid;

		if (_GetAgentInfo(idMap,nmMap,idAgent,nmAgent,guid))
		{
			std::string s;
			FormatString(s,"LoAgentRef||%s||%s||%d||%d||%d",nmMap.c_str(),nmAgent.c_str(),guid,idMap,idAgent);
			extern void CopyToClipboard(CWnd *wnd,const char *str);
			CopyToClipboard(GetWnd(),s.c_str());
		}
		return FALSE;
	}
	return CGuiAgent_3DNodeOperate::OnCommand(idCmd);
}


void*CGuiAgent_OperateEntity::_GetSelBuf()
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return NULL;
	return (void*)&data->selections;

}

i_math::pos2di *CGuiAgent_OperateEntity::_GetBlock(H3DNode node)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			if (data->mp->ResolveBlockPos((EntityAddress)node,_ptTemp))
				return &_ptTemp;
		}
	}

	return NULL;
}

EntityAddress HitTestOnMap(IEntitySystem *pES,i_math::line3df &ray,IRenderPort *rp)
{
	EntityAddress addr=pES->HitTestOnMap(ray);

	if (addr!=EntityAddress_Null)
		return (H3DNode)addr;

// 	if (rp)
// 	{
// 		int x,y;
// 		rp->TransPos(ray.end,x,y);
// 		ShellPos pos;
// 		pos.x=x;
// 		pos.y=y;
// 		IAsset *ast=pES->GetAS()->GetShell()->HitTest(pos);
// 		if (ast)
// 		{
// 			addr=pES->FindAssetOwnerOnMap(ast);
// 			if (addr!=EntityAddress_Null)
// 				return (H3DNode)addr;
// 		}
// 	}
	return EntityAddress_Null;
}

H3DNode CGuiAgent_OperateEntity::_HitTest(i_math::line3df &ray)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		EntityAddress addr=HitTestOnMap(data->pES,ray,GetRP());

		if (addr!=EntityAddress_Null)
			return (H3DNode)addr;
	}
	return H3DNode_Invalid;
}

BOOL CGuiAgent_OperateEntity::_Remove(H3DNode node)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			IEntityParam *ep;
			i_math::matrix43f mat;
			ep=data->mp->UnReside(mat,(EntityAddress)node);
			if (ep)
			{
				SAFE_RELEASE(ep);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CGuiAgent_OperateEntity::_CollectEnvelope(H3DNode *nodes,DWORD nNodes,Envelope &evlp)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			for (int i=0;i<nNodes;i++)
				data->pES->CollectEnvelopeOnMap((EntityAddress)nodes[i],evlp);
		}
	}
}

H3DNode CGuiAgent_OperateEntity::_Clone(H3DNode node)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			EntityAddress addrClone=data->mp->Clone((EntityAddress)node,i_math::vector3df(0.5f,0.0f,0.5f));
			if (addrClone!=EntityAddress_Null)
			{
				//修补AssetUID
				IEntity *en=data->mp->ToEntity(addrClone);
				if (en)
					RepairAssetUID(en,TRUE);
			}
			return addrClone;
		}
	}

	return H3DNode_Invalid;

}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_EntityRectSel

void CGuiAgent_EntityRectSel::_Sel(EntityAddress*inrects,DWORD c)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");

	std::vector<EntityAddress>sels;
	data->selections=_initials;

	for (int i=0;i<c;i++)
	{
		EntityAddress id=inrects[i];

		int idx;
		VEC_FIND(data->selections,id,idx);
		if (idx==-1)
		{//原来没有,我们添加
			data->selections.push_back(id);
		}
		else
		{//原来有的,我们删除
			if (!_bAccum)
				data->selections.erase(data->selections.begin()+idx);
		}
	}
}


BOOL CGuiAgent_EntityRectSel::OnBeginDrag(int x,int y,DWORD flag)
{

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe,2000.0f))
		return TRUE;

	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (!(flag&CtrlOpFlag_ShiftDown))
		{
			EntityAddress addr=HitTestOnMap(data->pES,probe,GetRP());
			if (addr!=EntityAddress_Null)
				return FALSE;
		}
	}

	_bAccum=FALSE;
	if (flag&CtrlOpFlag_CtrlDown)
		_initials=data->selections;
	else
	{
		if (flag&CtrlOpFlag_ShiftDown)
		{
			_bAccum=TRUE;
			_initials=data->selections;
		}
		else
		{
			data->selections.clear();
			_initials.clear();
			_Redraw(FALSE);
		}
	}


	_start.set(x,y);
	_rcDraw.set(_start.x,_start.y,_start.x,_start.y);

	return TRUE;
}

void CGuiAgent_EntityRectSel::OnDrag(int x,int y,DWORD flag)
{
	IRenderPort *rp=GetRP();
	if (!rp)
		return;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe))
		return;

	i_math::recti rc;
	rc.set(_start.x,_start.y,x,y);
	rc.repair();

	i_math::volumeCvxf vol;
	if (rp->CalcHitVolume(rc,vol))
	{
		GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
		if (data)
		{
			DWORD c;
			EntityAddress *addr=data->pES->VolumeHitTestOnMap(vol,c);
			_Sel(addr,c);
		}
	}

	_rcDraw=rc;

	_Redraw(TRUE);
}


void CGuiAgent_EntityRectSel::OnEndDrag(int x,int y,DWORD flag)
{
	OnDrag(x,y,flag);
}

BOOL CGuiAgent_EntityRectSel::OnDraw()
{
	if (!_bInDrag)
		return TRUE;

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;
	rp->FrameRect(_rcDraw,ColorAlpha(0x00ff00,0x7f));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_EntityMatrixEdit

void*CGuiAgent_EntityMatrixEdit::_GetSelBuf()
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return NULL;
	return (void*)&data->selections;
}

i_math::matrix43f *CGuiAgent_EntityMatrixEdit::_GetMat(H3DNode node)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
			return data->mp->GetEntityMat((EntityAddress)node);
	}
	return NULL;
}

i_math::pos2di *CGuiAgent_EntityMatrixEdit::_GetBlock(H3DNode node)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			if (data->mp->ResolveBlockPos((EntityAddress)node,_ptTemp))
				return &_ptTemp;
		}
	}
	return NULL;
}

void CGuiAgent_EntityMatrixEdit::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (data)
	{
		if (data->mp)
		{
			EntityAddress addr=(EntityAddress)node;
			i_math::matrix43f t;
			IEntityParam *ep=data->mp->UnReside(t,addr);
			if(ep)
			{
				addr=data->mp->Reside(mat,ep);
				if (addr==EntityAddress_Null)
				{//无法移到新的位置,reside回原位
					addr=data->mp->Reside(t,ep);
					assert(addr!=EntityAddress_Null);
				}
			}
			node=(H3DNode)addr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_EntityTrrnImprint

BOOL CGuiAgent_EntityTrrnImprint::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return TRUE;

	if (data->selections.size()<=0)
		return TRUE;

	BOOL bFound=FALSE;
	for (int i=0;i<data->selections.size();i++)
	{
		EntityAddress addr=data->selections[i];
		IEntity *en=g_ssGuiLib.pES->GetMap()->ToEntity(addr);

		if (en)
		{
			IAsset *asts[64];
			DWORD c=en->FindAsset("AstTrrnImprint",asts,ARRAY_SIZE(asts));
			if (c>0)
			{
				bFound=TRUE;
				break;
			}
		}
	}
	if (!bFound)
		return TRUE;

	_PushMenu("地表修改");
	_AddMenu("挖洞",ID_AGENT_TRRNIMPRINT_ADDHOLE);
	_AddMenu("印刻",ID_AGENT_TRRNIMPRINT_IMPRINT);
	_PopMenu();

	return TRUE;
}

static void InflateMods(std::set<i_math::pos2di>&mods)
{
	std::set<i_math::pos2di> modsNew;

	int range=3;
	std::set<i_math::pos2di>::iterator it;
	for(it=mods.begin();it!=mods.end();it++)
	{
		i_math::pos2di pos=(*it);
		for (int i=pos.x-range;i<=pos.x+range;i++)
		for (int j=pos.y-range;j<=pos.y+range;j++)
			modsNew.insert(i_math::pos2di(i,j));
	}
	mods.swap(modsNew);
}

void CGuiAgent_EntityTrrnImprint::_DoImprint(ITrrnMapEditor *editor,IMeshSnapshot *ms,i_math::vector3df *vtxs,DWORD nVtx,WORD *idxs,DWORD nIdx,const char *mode)
{
	DWORD lvl=1;
	if (std::string(mode)=="AddHole")
		lvl=3;
	if (std::string(mode)=="Imprint")
		lvl=3;

	float tilelen=((float)BLOCK_LENGTH)/(float)(1<<lvl);

	std::vector<i_math::pos2di>tilesTotal;
	if (TRUE)
	{
		std::vector<i_math::pos2di>tiles;
		for (int i=0;i<nIdx;i+=3)
		{
			float xyz[6];
			xyz[0]=vtxs[idxs[i]].x;
			xyz[1]=vtxs[idxs[i]].z;
			xyz[2]=vtxs[idxs[i+1]].x;
			xyz[3]=vtxs[idxs[i+1]].z;
			xyz[4]=vtxs[idxs[i+2]].x;
			xyz[5]=vtxs[idxs[i+2]].z;
			TileByTriangle(xyz,tilelen,tiles);
			for (int j=0;j<tiles.size();j++)
			{
				UNIQUE_VEC_ADD(tilesTotal,tiles[j]);
			}
		}
	}

	TrrnSeedMap seedmp;
	if (std::string(mode)=="AddHole")
	{
		seedmp.lvl=lvl;
		for (int i=0;i<tilesTotal.size();i++)
		{
			TrrnSeedMap::SeedPoint sp;
			sp.x=tilesTotal[i].x;
			sp.y=tilesTotal[i].y;
			sp.wt=1.0f;
			sp.flag=TrrnSeedMap::SeedPointF_None;

			seedmp.points.push_back(sp);
			seedmp.rcAllowed.merge(sp.x,sp.y);


			i_math::pos2di ptAffect=tilesTotal[i];
			ptAffect.scale_signed((1<<lvl));
			_mods.insert(ptAffect);
		}

		InflateMods(_mods);
		//严重注意:目前没有判断要修改的block是否可以修改,要补上

		editor->ModOpaque(seedmp,TRUE);
	}

	if (std::string(mode)=="Imprint")
	{
		seedmp.lvl=lvl;
		for (int i=0;i<tilesTotal.size();i++)
		{
			TrrnSeedMap::SeedPoint sp;
			sp.x=tilesTotal[i].x;
			sp.y=tilesTotal[i].y;
			sp.flag=TrrnSeedMap::SeedPointF_None;

			if (TRUE)
			{
				i_math::line3df line;
				line.start.x=0.5f*tilelen+tilelen*(float)tilesTotal[i].x;
				line.start.z=0.5f*tilelen+tilelen*(float)tilesTotal[i].y;
				line.start.y=1000.0f;
				line.end=line.start;
				line.end.y=-1000.0f;

				float dist;
				if(!ms->HitTest(line,dist))
					continue;
				sp.wt=line.start.y-dist;
				sp.v=line.start;
				sp.v.y=sp.wt;
			}

			seedmp.points.push_back(sp);
			seedmp.rcAllowed.merge(sp.x,sp.y);

			i_math::pos2di ptAffect=tilesTotal[i];
			ptAffect.scale_signed((1<<lvl));
			_mods.insert(ptAffect);
		}

		InflateMods(_mods);
		//严重注意:目前没有判断要修改的block是否可以修改,要补上

		editor->AddHeight(seedmp,FALSE,1.0f,0.0f,TRUE);
	}
}


BOOL CGuiAgent_EntityTrrnImprint::OnCommand(DWORD idCmd)
{
	GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
	if (!data)
		return TRUE;

	if ((idCmd==ID_AGENT_TRRNIMPRINT_ADDHOLE)||
		(idCmd==ID_AGENT_TRRNIMPRINT_IMPRINT))
	{
		if (data->selections.size()<=0)
			return TRUE;

		ITrrnMap *trrn=g_ssGuiLib.pES->FindTrrn();
		if (!trrn)
			return TRUE;

		std::string modeWork;
		if (idCmd==ID_AGENT_TRRNIMPRINT_ADDHOLE)
			modeWork="AddHole";
		if (idCmd==ID_AGENT_TRRNIMPRINT_IMPRINT)
			modeWork="Imprint";

		ITrrnMapEditor *editor=trrn->GetEditor();

		editor->BeginModify();
		_mods.clear();

		for (int i=0;i<data->selections.size();i++)
		{
			EntityAddress addr=data->selections[i];
			IEntity *en=g_ssGuiLib.pES->GetMap()->ToEntity(addr);
			if (en)
			{
				IAsset *asts[64];
				DWORD c=en->FindAsset("AstTrrnImprint",asts,ARRAY_SIZE(asts));
				for (int j=0;j<c;j++)
				{
					IAsset *ast=asts[j];

					std::string path;
					std::string mode;
					i_math::matrix43f mat;
					if (TRUE)
					{
						Prop_Void t;
						GProperty *prop;
						prop=GCall(ast,"GetMeshPath",t);
						if (prop->GetGVT()==GVT_String)
							path=((Prop_String*)prop)->v;
						prop=GCall(ast,"GetMode",t);
						if (prop->GetGVT()==GVT_String)
							mode=((Prop_String*)prop)->v;
						ast->GetXForm(mat);
					}

					if (path.empty()||(mode.empty()))
						continue;

					if (!(mode==modeWork))
						continue;

					IMesh *msh=(IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(path.c_str());
					if (msh)
					{
						IMeshSnapshot *ms=msh->ObtainSnapshot();

						MeshSnapshotArg arg;
						if (ms->TakeSnapshot(mat,arg))
						{
							i_math::vector3df *pos=ms->GetPos();
							WORD *indices=ms->GetIndices(0);

							_DoImprint(editor,ms,pos,ms->GetVBCount(),indices,ms->GetIBCount(0),modeWork.c_str());
						}

						SAFE_RELEASE(ms);
						SAFE_RELEASE(msh);
					}
				}
			}
		}

		editor->EndModify();

		if (_mods.size()>0)
		{
			CGeActor * actor = _GetActor();
			CModManager * modmgr = actor->GetModMgr();
			if (modmgr)
			{
				CGuiView * view = GetGuiView();
				CModBlockBack * mod = new CModBlockBack(view);
				std::vector<i_math::pos2di> blocks;

				std::set<i_math::pos2di>::iterator it;
				for(it = _mods.begin();it!=_mods.end();it++)
					blocks.push_back((*it));
				mod->BackupBlocks(&blocks[0],blocks.size());

				trrn->SaveModified();
				blocks.clear();
				Mod_New(modmgr,(CModBase *&)mod);
			}
		}

		_Redraw();

		return FALSE;
	}

	return TRUE;

}



//////////////////////////////////////////////////////////////////////////
//CMod_ForceBind
class CMod_ForceBind:public CModBase
{
public:
	CMod_ForceBind(CGuiPanel_Entity *panel)
	{
		_panel=panel;
	}
	virtual BOOL IsEmpty()	{		return FALSE;	}

	virtual BOOL Undo()
	{
		_panel->SetForceBind();
		return TRUE;
	}
	virtual BOOL Redo()
	{
		return Undo();
	}

public:
	CGuiPanel_Entity*_panel;
};

//////////////////////////////////////////////////////////////////////////
//CProtoThumbnailsList

BEGIN_MESSAGE_MAP(CProtoThumbnailsList, CXTListCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()

//should be sychronized with the value in protolib.cpp
#define NODETYPE_FOLDER 1
#define NODETYPE_PROTO 2

int CProtoThumbnailsList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CXTListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	_il.Create(ProtoThumbnailWidth,ProtoThumbnailHeight,ILC_COLOR24,128,8);
	SetImageList(&_il,LVSIL_NORMAL);

	CFont* pFont = GetFont(); 
	if (pFont != NULL) 
	{
		LOGFONT lf;
		pFont->GetLogFont(&lf);
		lf.lfHeight=-1;
		_font.CreateFontIndirect(&lf);
	 	SetFont(&_font);
	}

	SetIconSpacing(ProtoThumbnailWidth+2,40);

	return 0;
}

void CProtoThumbnailsList::_ClearContent()
{
	DeleteAllItems();
	_il.SetImageCount(0);
}

NodeHandle CProtoThumbnailsList::_GetCurSel()
{
	int nSelect=GetSelectedCount();
	if (nSelect==1)
	{
		int idxItem=GetNextItem(-1,LVNI_SELECTED);
		if (idxItem!=-1)
			return (NodeHandle)GetItemData(idxItem);
	}
	return NodeHandle_Null;
}

void CProtoThumbnailsList::_SetCurSel(NodeHandle h)
{
	DWORD c=GetItemCount();
	for (int i=0;i<c;i++)
	{
		if (h==(NodeHandle)GetItemData(i))
		{
			SetItemState(-1, 0, LVIS_SELECTED);
			SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			EnsureVisible(i,FALSE);
			break;
		}
	}
}


void CProtoThumbnailsList::Update()
{
	CGuiPanel_Entity* panel=((CGuiPanel_Entity*)GetParent());

	GuiData_EntityMap *data=(GuiData_EntityMap *)panel->FindData("entitymap");
	
	NodeHandle hSel=panel->GetPrlTree().GetCurSel();
	if(hSel)
	{
		IProtoLib*lib=data->pES->GetProtoLib();
		if (lib)
		{
			CNodeTree *ntree=lib->GetNodeTree()->GetTree();
			if (ntree->GetType(hSel)==NODETYPE_FOLDER)
			{
				if (hSel!=_hFolder)
				{
					_ClearContent();

					CxImage img;
					std::string path;
					DWORD c;
					NodeHandle *handles=ntree->Enum(hSel,NODETYPE_PROTO,c);
					for (int i=0;i<c;i++)
					{
						NodeHandle h=handles[i];

						path=lib->MakeProtoPath(ntree->GetPath(h));
						RemoveFileSuffix(path);
						MakeFileSuffix(path,"tbn");

						if (img.Load(path.c_str()))
						{
							if ((img.GetWidth()!=ProtoThumbnailWidth)||(img.GetHeight()!=ProtoThumbnailHeight))
								img.Resample(ProtoThumbnailWidth,ProtoThumbnailHeight,2);
							HBITMAP hBmp=img.MakeBitmap();
							_il.Add(CBitmap::FromHandle(hBmp),(CBitmap*)NULL);

							int idxItem=InsertItem(GetItemCount(),"",_il.GetImageCount()-1);
							if (idxItem>=0)
								SetItemData(idxItem,(DWORD_PTR)h);
						}
					}

					_hFolder=hSel;
				}
// 				std::string path=ntree->GetPath(hSel);
// 

			}
			else
			{
				if (ntree->GetType(hSel)==NODETYPE_PROTO)
				{
					if (ntree->CheckDescendant(_hFolder,hSel))
					{
						if (_GetCurSel()!=hSel)
						{
							_SetCurSel(hSel);
						}
					}
				}
				
			}
		}

	}


// 	if (GetItemCount()<=0)
// 	{
// 		if (data)
// 		{
// 			if (data->pES)
// 			{
// 				IProtoLib *lib=data->pES->GetProtoLib();
// 				if (lib)
// 				{
// 					std::string path=data->pES->GetWS()->GetPath(WSPath_ProtoLib);
// 					path+="\\__thumbnaildump__.tga";
// 
// 					CxImage img;
// 					img.Load(path.c_str());
// 					HBITMAP hBmp=img.MakeBitmap();
// 					_il.Add(CBitmap::FromHandle(hBmp),(CBitmap*)NULL);
// 
// 					SetIconSpacing(ProtoThumbnailWidth,ProtoThumbnailHeight+16);
// 					for (int i=0;i<20;i++)
// 						InsertItem(0,"test",0);
// 				}
// 			}
// 		}
// 	}
}



//////////////////////////////////////////////////////////////////////////
//CEntityPage
BEGIN_MESSAGE_MAP(CEntityPage, CGObjGrid)
END_MESSAGE_MAP()

void CEntityPage::Reset()
{
	CGObjGrid::Bind(NULL);
	_ClearCache();
	Zero();
}

BOOL CEntityPage::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CGObjGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	ShowToolBar(FALSE);
	return TRUE;

}

void CEntityPage::_ClearCache()
{
	for (int i=0;i<_cache.size();i++)
	{
		GProp_SafeDeleteThis(_cache[i]);
	}

	_cache.clear();
}


void CEntityPage::Bind(EntityAddress addr,BOOL bForceRebind)
{
	_util.Init(g_ssGuiLib.pRS->GetPath(Path_BehaviorGraph),(BgpClasses*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());

	BOOL bUpdating=FALSE;
	if (addr==_addr)
	{
		if (!bForceRebind)
			return;
		bUpdating=TRUE;//if force bind,and the asset address is not changed,we should 
										//updat the content(not clean and reset)
	}

	_addr=addr;

	IEntityParam *ep=_mp->ToEntityParam(addr);
	IEntity *en=_mp->ToEntity(addr);

	if ((!ep)||(!en))
	{
		ResetContent();
		return;
	}

	for (int i=0;i<_cachesBhvValues.size();i++)
		_cachesBhvValues[i].Clear();
	_cachesBhvValues.clear();

	RGState state;
	if (bUpdating)
	{//record the original item state
		CGObjGrid::RecordState(state);
		CGObjGrid::LockPaint();
	}

	//Bind 数据
	if (TRUE)
	{
		ResetContent();
		_ClearCache();

		BeginInsert();

		InsertCategory("Properties","",NULL);
		PushInsert();


		IProto *proto=en->GetProto();

		DWORD c=proto->GetStubCount();
		_cache.resize(c);
		VEC_SET(_cache,0);
		for (int i=0;i<c;i++)
		{
			const char *name;
			proto->GetStub(i,name);
			GProperty *t=proto->FindPropertyData(name);
			if (!t)
				continue;
			GProperty *t2=ep->FindProp(name);
			if (!t->IsRef())
			{
				if (t2)
					_cache[i]=t2->Clone();
				else
					_cache[i]=t->Clone();
			}
			else
			{
				if (t2)
					_cache[i]=((PropRef*)t2)->CloneDeep();
				else
					_cache[i]=((PropRef*)t)->CloneDeep();
			}

			GStubBase *stb=proto->FindStub(name);

			CXTPPropertyGridItem * item=InsertProp(_cache[i],name,stb->sem,stb->desc.c_str());

			if (t2)
			{
				item->GetValueMetrics()->m_clrFore = 0xff7f00;
				item->GetValueMetrics()->m_clrBack = RGB(215, 215, 215);
				item->GetCaptionMetrics()->m_clrFore = 0xff7f00;
				item->GetCaptionMetrics()->m_clrBack = RGB(215, 215, 215);
			}

		}

		PopInsert();

		EndInsert();
		
	}

	if (bUpdating)
	{//recover the original state
		CGObjGrid::RestoreState(state);
		CGObjGrid::UnLockPaint();
	}
	else
		CGObjGrid::ExpandAll();
}




void CEntityPage::OnBeginItemChange(CXTPPropertyGridItem *item)
{
}

void CEntityPage::OnItemChange(CXTPPropertyGridItem *item)
{
}


void CEntityPage::OnEndItemChange(CXTPPropertyGridItem *item)
{
	extern void BehaviorValueCache_PreSave(BhvValuesCache*cache);
	for (int i=0;i<_cachesBhvValues.size();i++)
		BehaviorValueCache_PreSave(&_cachesBhvValues[i]);

	_ApplyMod();

	((CGuiPanel_Entity*)(GetParent()))->SetForceBind();

}


void CEntityPage::_ApplyMod()
{
	CGuiPanel_Entity* panel=((CGuiPanel_Entity*)GetParent());
	CGuiView *view=(CGuiView *)panel->FindView("perspective");
	GuiData_EntityMap *data=(GuiData_EntityMap *)panel->FindData("entitymap");
	if (TRUE)
	{
		IProto *proto;
		IEntityParam *ep;
		if (TRUE)
		{
			IEntity *en=_mp->ToEntity(_addr);
			proto=en->GetProto();
			ep=_mp->ToEntityParam(_addr);
		}
		if ((!proto)||(!ep))
			return;

		i_math::pos2di ptBlk;
		_mp->ResolveBlockPos(_addr,ptBlk);

		ep->ClearProp();

		for (int i=0;i<_cache.size();i++)
		{
			GProperty *p=_cache[i];
			if (!p)
				continue;

			const char *name;
			proto->GetStub(i,name);
			GProperty *t=proto->FindPropertyData(name);
			if (t->Equals(p))
				continue;

			ep->AddProp(name,p);
		}


		CMod_ChangeEntityMap *mod=NULL;

		//do the backup
		if (_modmgr)
		{
			mod=new CMod_ChangeEntityMap(view);
			mod->BackupBlocks(&ptBlk,1);
			mod->BackupSelection();
		}

		//do the actual modifying
		_mp->SetBlockModified(ptBlk);
		data->pES->SaveToMap();

		//update the block on the map
		data->pES->ReloadMap(&ptBlk,1);

		//add the mods
		if (_modmgr)
		{
			_modmgr->NewModGroup();
			_modmgr->PushBack(mod,FALSE);
			_modmgr->PushBack(new CMod_ForceBind(panel),FALSE);
		}

	}

}

BOOL CEntityPage::_InsertElem(GObjBase *obj,GElemBase *elem)
{
	GObjBase *objSub;
	if (elem->GetObj(obj->GetOwner(),&objSub))
	{
		if (!(std::string("BhvValues")==objSub->GetName()))
			return FALSE;
	}
	else
		return FALSE;

	IEntity*en=_mp->ToEntity(_addr);
	if (!en)
		return FALSE;

	IProto *proto=en->GetProto();
	if (proto)
	{
		GProperty *propToFind=NULL;
		for (int i=0;i<_cache.size();i++)
		{
			extern GObjBase *FindOnlySubObj(GObjBase *obj);
			if (!_cache[i])
				continue;
			if (FindOnlySubObj(_cache[i]->GetGObj())==obj)
			{
				const char *name;
				proto->GetStub(i,name);
				propToFind=proto->FindRawPropertyData(name);
				break;
			}
		}
		if (propToFind)
		{
			extern StringID SeekBehaviorGraphName(IProto *proto,GProperty *propToFind);
			StringID nmBG=SeekBehaviorGraphName(proto,propToFind);
			if (nmBG!=StringID_Invalid)
			{
				CBehaviorGraphPads pads;
				if (_util.LoadBGPads(nmBG,pads))
				{
					_cachesBhvValues.resize(_cachesBhvValues.size()+1);
					BhvValuesCache*cache=&_cachesBhvValues[_cachesBhvValues.size()-1];

					_util.ResolveBGPads(pads);
					extern void BehaviorValueCache_PreLoad(BhvValuesCache *cache,BhvValues *values,CBehaviorGraphPads &pads);
					BehaviorValueCache_PreLoad(cache,(BhvValues *)objSub->GetOwner(),pads);
					extern void BehaviorValueCache_InsertItems(CGObjGrid *grid,BhvValuesCache *cache);
					BehaviorValueCache_InsertItems(this,cache);
				}

	// 			IBehaviorGraph *bg=(IBehaviorGraph*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->ObtainRes(nmBG);
	// 			if (bg)
	// 			{
	// 				BehaviorGraphData *dataBg=bg->GetData();
	// 				_cachesBhvValues.resize(_cachesBhvValues.size()+1);
	// 				BhvValuesCache*cache=&_cachesBhvValues[_cachesBhvValues.size()-1];
	// 
	// 				extern void BehaviorValueCache_PreLoad(BhvValuesCache *cache,BhvValues *values,CBehaviorGraphPads &pads);
	// 				BehaviorValueCache_PreLoad(cache,(BhvValues *)objSub->GetOwner(),dataBg->pads);
	// 				extern void BehaviorValueCache_InsertItems(CGObjGrid *grid,BhvValuesCache *cache);
	// 				BehaviorValueCache_InsertItems(this,cache);
	// 			}
			}
		}
	}

	return TRUE;
}





//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Entity
#define ID_SYNC 50


BEGIN_MESSAGE_MAP(CGuiPanel_Entity, CGuiPanel)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_RESIDE,OnReside)
	ON_COMMAND(IDC_RELOADPROTOLIB,OnReloadProtoLib)
	ON_MESSAGE(GLM_PrlTree_DblClick,OnPrlTreeDblClk)
	ON_COMMAND(IDC_REPAIRGUID,OnRepairGUID)
	ON_COMMAND(ID_SYNC,OnSync)
END_MESSAGE_MAP()


CGuiPanel_Entity::CGuiPanel_Entity(CWnd* pParent):
							CGuiPanel(IDD_EDITPANEL_ENTITY, pParent)
{
	_bResiding=FALSE;
	_bForceBind=FALSE;

	_mateditor=NULL;

	_bLibDirty=FALSE;
}

BOOL CGuiPanel_Entity::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_ENTITY,pParent);	
}


BOOL CGuiPanel_Entity::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;

	GET_CONTROL_RECT(this,IDC_TREE,rc);
	HIDE_CONTROL(this,IDC_TREE);
	_tree.Create(this,rc,1);
	_tree.SetOwner(m_hWnd);

	GET_CONTROL_RECT(this,IDC_PAGE,rc);
	HIDE_CONTROL(this,IDC_PAGE);
	_page.Create(rc,this,2);
	_page.SetWindowText("Entity");

	GET_CONTROL_RECT(this,IDC_THUMBNAILLIST,rc);
	HIDE_CONTROL(this,IDC_THUMBNAILLIST);
	_tbns.Create(LVS_ICON|WS_BORDER|WS_VISIBLE|WS_CHILD,rc,this,3);
//	_tbns.ShowWindow(SW_SHOW);

	GET_CONTROL_RECT(this,IDC_SYNC,rc);
	HIDE_CONTROL(this,IDC_SYNC);
	_btnSync.Create("",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,rc,this,ID_SYNC);
	_btnSync.SetBitmap(CSize(16,16),IDB_SYNCPROTO);

	_resider.AddRef();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CGuiPanel_Entity::OnDestroy()
{
	_watcher.Stop();
	_tree.SetNodeTree(NULL);
	_page.Reset();

	CGuiPanel::OnDestroy();



	// TODO: Add your message handler code here
}

void CGuiPanel_Entity::Reset()
{
	EnableWindow(FALSE);
	GuiData_System *dataSys=(GuiData_System*)FindData("system");
	if (!dataSys)
		return;
	GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");
	if (!dataMap)
		return;


	_tree.SetES(dataMap->pES);
	_tree.SetLib(dataMap->pES->GetProtoLib());
	_tree.EnableEdit(FALSE);
	_tree.SetNodeTree(dataMap->pES->GetProtoLib()->GetNodeTree());

	_page.SetModMgr(_modmgr);
	_page.SetEntityMap(dataMap->mp);

	_watcher.Start(dataSys->pWS->GetPath(WSPath_ProtoLib),WNF_CHANGE_FILE_NAME|WNF_CHANGE_CREATION|WNF_CHANGE_LAST_WRITE);
	_bLibDirty=FALSE;

	EnableWindow(TRUE);

}

void CGuiPanel_Entity::_OccupyActor()
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

		view->AddAgent(0,new CGuiAgent_OperateEntity,AGENTPRIORITY_STANDARD+10);
		view->AddAgent(0,new CGuiAgent_EntityRectSel,AGENTPRIORITY_STANDARD+15);
		if (TRUE)//the matrix editor agent
		{
			_mateditor=new CGuiAgent_EntityMatrixEdit;
			_mateditor->SetWorkable(EditMode_All,TRUE);
			view->AddAgent(0,_mateditor,AGENTPRIORITY_STANDARD+20);
		}
		view->AddAgent(0,new CGuiAgent_EntityTrrnImprint,AGENTPRIORITY_STANDARD+5);

		view->AddAgent(0,new CGuiAgent_MatSet,AGENTPRIORITY_STANDARD+30);
		view->AddAgent(0,new CGuiAgent_AstUIDSet,AGENTPRIORITY_STANDARD+30);

		GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
		if (dataRG)
			dataRG->RegisterRichGrid("EntityPage",&_page);


	}
}

void CGuiPanel_Entity::OnEnterActivity()
{
	_OccupyActor();
}

void CGuiPanel_Entity::OnDetachView(CGeView *view,DWORD iLevel)
{
	if (view->CheckName("perspective")&&(iLevel==1))
		_bResiding=FALSE;

	if (view->CheckName("perspective")&&(iLevel==0))
		_mateditor=NULL;
}

ProtoID CGuiPanel_Entity::_GetCurSelProto()
{
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	IProtoLib*lib=dataSys->pES->GetProtoLib();
	ProtoID protoid=ProtoID_Null;
	CNodeTree *ntree=lib->GetNodeTree()->GetTree();
	if(ntree)
	{
		NodeHandle hSel=_tree.GetCurSel();
		if(hSel)
		{
			std::string path=ntree->GetPath(hSel);
			RemoveFileSuffix(path);
			protoid=lib->FindProto(path.c_str());
		}
	}

	return protoid;
}



void CGuiPanel_Entity::UpdateUI()
{
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	assert(dataSys);
	GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");
	assert(dataMap);

	//过滤掉那些不在map范围内的selections
	if (TRUE)
	{
		i_math::recti rcMap=dataMap->mp->GetMapRect();

		DWORD c=0;
		for (int i=0;i<dataMap->selections.size();i++)
		{
			i_math::pos2di pos;
			if (FALSE==dataMap->mp->ResolveBlockPos(dataMap->selections[i],pos))
				continue;
			if (!rcMap.isPointInside(pos))
				continue;
			dataMap->selections[c]=dataMap->selections[i];
			c++;
		}

		dataMap->selections.resize(c);
	}


	CHECK_BUTTON(this,IDC_RESIDE,_bResiding);

	IProtoLib*lib=dataSys->pES->GetProtoLib();

	if (_bResiding)
	{
		ProtoID protoid=_GetCurSelProto();
		_resider.SetProtoID(protoid);
	}

	//update the page
	if (TRUE)
	{
		if (dataMap->selections.size()!=1)
			_page.Bind(0,TRUE);
		else
			_page.Bind(dataMap->selections[0],_bForceBind);

		_bForceBind=FALSE;
	}

	//Update the title
	if (TRUE)
	{
		std::string title="n/a";
		if (dataMap->selections.size()==1)
		{
			EntityAddress addr=dataMap->selections[0];
			IEntityParam *ep=dataMap->mp->ToEntityParam(addr);
			if (ep)
			{
				ProtoID id=ep->GetProtoID();
				title=lib->FindPath(id);
			}
		}

		CWnd *wnd=GetDlgItem(IDC_TITLE);
		CString s;
		wnd->GetWindowText(s);
		if (title!=(LPCTSTR)s)
			wnd->SetWindowText(title.c_str());
	}

	//Random Rotate
	if (TRUE)
	{
		CButton*p=(CButton*)GetDlgItem( IDC_RANDOMROTATE);
		if (p->GetCheck()==BST_CHECKED)
			_resider.SetRandomRotate(TRUE);
		else
			_resider.SetRandomRotate(FALSE);
	}
	
	//Reside On Ground
	if (TRUE)
	{
		CButton*p=(CButton*)GetDlgItem( IDC_RESIDEONGROUND);
		if (p->GetCheck()==BST_CHECKED)
			_resider.SetResideOnGround(TRUE);
		else
			_resider.SetResideOnGround(FALSE);
	}

	//Auto Align
	if (TRUE)
	{
		CButton*p=(CButton*)GetDlgItem( IDC_AUTOALIGN);
		if (p->GetCheck()==BST_CHECKED)
			_resider.SetAutoAlign(TRUE);
		else
			_resider.SetAutoAlign(FALSE);
	}

	if (_mateditor)
		_mateditor->UpdateBind();

	//更新_bLibDirty
	if (TRUE)
	{
		ChangedFileInformation *info=NULL;
		DWORD c=_watcher.FetchChangedFiles((const ChangedFileInformation *&)info);
		for (int i=0;i<c;i++)
		{
			ChangedFileAction action=info[i].action;

			if (!CheckFileSuffix(info[i].name,"prt"))
				continue;
			
			_bLibDirty=TRUE;
		}
	}

	if (_bLibDirty)
	{
		ENABLE_CONTROL(this,IDC_RELOADPROTOLIB);
		CWnd *wnd=GetDlgItem(IDC_RELOADPROTOLIB);
		wnd->FlashWindow(TRUE);
	}
	else
	{
		DISABLE_CONTROL(this,IDC_RELOADPROTOLIB);
	}

	_tbns.Update();


}

void CGuiPanel_Entity::OnReside()
{
	_OccupyActor();

	CGuiView *view=(CGuiView *)FindView("perspective");

	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");

	if (!_bResiding)
	{
		view->AttachActor(1,static_cast<CGeActor*>(this));
		view->AddAgent(1,&_resider,AGENTPRIORITY_STANDARD+10);
		view->AddAgent(1,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
		_bResiding=TRUE;
	}
}

LRESULT CGuiPanel_Entity::OnPrlTreeDblClk(WPARAM wParam,LPARAM lParam)
{
	CButton*p=(CButton*)GetDlgItem( IDC_REPLACEMODE);
	if (p->GetCheck()!=BST_CHECKED)
		OnReside();
	else
	{//替换模式

		GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");

		if (dataMap->selections.size()==1)
		{
			ProtoID protoid=_GetCurSelProto();

			i_math::matrix43f mat;
			if (TRUE)
			{
				IEntityParam *ep;
				ep=dataMap->mp->UnReside(mat,dataMap->selections[0]);
				if (ep)
				{
					SAFE_RELEASE(ep);

					ep=dataMap->pES->CreateEntityParam();
					ep->SetProtoID(protoid);
					EntityAddress addr=dataMap->mp->Reside(mat,ep);

					//修补AssetUID
					if (addr!=EntityAddress_Null)
					{
						IEntity *en=dataMap->mp->ToEntity(addr);
						if (en)
							RepairAssetUID(en,FALSE);
					}

					if (addr!=EntityAddress_Null)
					{
						CMod_ChangeEntityMap *mod=NULL;

						if (GetModMgr())
						{
							i_math::pos2di ptBlk;
							dataMap->mp->ResolveBlockPos(addr,ptBlk);

							mod=new CMod_ChangeEntityMap(FindView("perspective"));
							mod->BackupBlocks(&ptBlk,1);
							mod->BackupSelection();
						}

						//do the actual modifying
						dataMap->pES->SaveToMap();
						dataMap->ClearSelection();
						dataMap->AddSelection(addr);

						if (GetModMgr())
						{
							GetModMgr()->NewModGroup();
							GetModMgr()->PushBack(mod,FALSE);
						}
					}
				}
			}
		}
	}
	return 0;
}


void CGuiPanel_Entity::OnReloadProtoLib()
{
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	assert(dataSys);
	GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");
	assert(dataMap);

	CGuiView *view=(CGuiView *)FindView("perspective");
	view->DetachActor(1,static_cast<CGeActor*>(this));

	dataSys->pES->ReloadProtoLib();         

	//update the tree
	_tree.SetNodeTree(dataSys->pES->GetProtoLib()->GetNodeTree());

	SetForceBind();//force to update the page

	view->Invalidate();

	_bLibDirty=FALSE;

}

BOOL NeedRepairGUID(GObjBase *gobj)
{
	GElemBase *elems=gobj->GetElems();
	void *owner=gobj->GetOwner();

	DWORD *guid;
	while(elems)
	{
		if (elems->GetSem().code==GSem_GUID)
		{
			if (elems->GetVar(owner,(void**)&guid))
			{
				if ((*guid)==0)
					return TRUE;
			}
		}
		GObjBase *objSub;
		if (elems->GetObj(owner,&objSub))
		{
			if (NeedRepairGUID(objSub))
				return TRUE;
		}
		elems=elems->next;
	}

	return FALSE;
}

extern SscUID SscUID_SafeGen();

//返回是否发生了修补
BOOL RepairGUID(GObjBase *gobj)
{

	GElemBase *elems=gobj->GetElems();
	void *owner=gobj->GetOwner();

	DWORD *guid;
	while(elems)
	{
		if (elems->GetSem().code==GSem_GUID)
		{
			if (elems->GetVar(owner,(void**)&guid))
			{
				if ((*guid)==0)
				{
					*guid=SscUID_SafeGen();
					return TRUE;
				}
			}
			continue;
		}
		GObjBase *objSub;
		if (elems->GetObj(owner,&objSub))
		{
			if (RepairGUID(objSub))
				return TRUE;
		}
		elems=elems->next;
	}

	return FALSE;

}

//如果bForceGen为TRUE,表示会强制生成所有Asset的AssetUID,否则只对AssetUID为空的Asset进行
BOOL RepairAssetUID(IEntity *en,BOOL bForceGen)
{
	IAsset *asts[512];
	DWORD c=en->EnumAllAsset(asts,sizeof(asts))	;

	BOOL bRepaired=FALSE;
	for (int i=0;i<c;i++)
	{
		IAsset *ast=asts[i];
		if (ast)
		{
			if (ast->SupportUID())
			{
				if ((ast->GetUID()==AssetUID_Null)||(bForceGen))
				{
					ast->SetUID(SscUID_SafeGen());
					bRepaired=TRUE;
				}
			}
		}
	}
	return bRepaired;
}

AssetUID GetAssetUID(IEntity *en)
{
	IAsset *asts[512];
	DWORD c=en->EnumAllAsset(asts,sizeof(asts))	;

	for (int i=0;i<c;i++)
	{
		IAsset *ast=asts[i];
		if (ast)
		{
			if (ast->SupportUID())
				return ast->GetUID();
		}
	}
	return AssetUID_Null;
}




void CGuiPanel_Entity::OnRepairGUID()
{
	if (IDOK!=AfxMessageBox("为地图上所有需要GUID的对象自动生成一个(如果还没有生成的话),该操作会清楚UndoBuffer,确认吗?",MB_OKCANCEL))
		return;
	GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");
	IEntitySystem *pES=dataMap->pES;
	IEntityMap *mp=dataMap->mp;
	IMapFile *mf=dataMap->mf;

	if (TRUE)
	{
		i_math::vector3df centerOld=pES->GetAS()->GetCenter();

		i_math::recti rc=mf->GetRect();
		rc*=BLOCK_LENGTH;

		int step=128;

		static std::vector<EntityAddress> addrs;
		addrs.resize(4096);

		for (int i=rc.Left();i<rc.Right();i+=step)
		for (int j=rc.Top();j<rc.Bottom();j+=step)
		{
			i_math::recti rcCur;
			rcCur.set(i,j,i+step,j+step);

			i_math::pos2di center=rcCur.getCenter();

			pES->Locate(i_math::vector3df((float)center.x,0,(float)center.y));

			DWORD sz=mp->EnumEntities((float)center.x,(float)center.y,4.0f+(float)step,&addrs[0],addrs.size());

			if (sz>=addrs.size())
			{
				LOG_DUMP("RepairGuid",Log_Error,"枚举Entity时,数量可能超出Buffer大小!");
			}

			for (int k=0;k<sz;k++)
			{
				EntityAddress addr=addrs[k];
				IEntityParam *ep;
				IEntity *en;
				ep=mp->ToEntityParam(addr);
				en=mp->ToEntity(addr);

				if ((!ep)||(!en))
					continue;

				i_math::pos2di ptBlk;
				mp->ResolveBlockPos(addr,ptBlk);

				BOOL bRepaired=FALSE;

				IProto *proto=en->GetProto();

				DWORD c=proto->GetStubCount();
				for (int i=0;i<c;i++)
				{
					const char *name;
					proto->GetStub(i,name);
					GProperty *t=proto->FindPropertyData(name);
					if (!t)
						continue;
					GProperty *t2=ep->FindProp(name);
					if (t2)
					{
						GObjBase *gobj=t2->GetGObj();
						if (!NeedRepairGUID(gobj))
							continue;
						bRepaired=RepairGUID(gobj);
						continue;
					}

					GObjBase *gobj=t->GetGObj();
					if (!NeedRepairGUID(gobj))
						continue;

					GProperty *propNew=NULL;
					if (!t->IsRef())
						propNew=t->Clone();
					else
						propNew=((PropRef*)t)->CloneDeep();

					bRepaired=RepairGUID(propNew->GetGObj());
					ep->AddProp(name,propNew);
					GProp_SafeDeleteThis(propNew);
				}

				mp->SetBlockModified(ptBlk);
			}
		}

		//归位
		pES->Locate(i_math::vector3df((float)centerOld.x,0,(float)centerOld.y));
	}

	mp->SaveModified();

	GetModMgr()->Clear();
}

void CGuiPanel_Entity::OnSync()
{
	if (!_mgr)
		return;
	GuiData_System *dataSys=(GuiData_System*)_mgr->FindData("system");
	assert(dataSys);
	GuiData_EntityMap*dataMap=(GuiData_EntityMap*)FindData("entitymap");
	assert(dataMap);

	std::string title="n/a";
	if (dataMap->selections.size()==1)
	{
		EntityAddress addr=dataMap->selections[0];
		IEntityParam *ep=dataMap->mp->ToEntityParam(addr);
		if (ep)
		{
			ProtoID id=ep->GetProtoID();

			std::string path=dataSys->pES->GetProtoLib()->FindPath(id);
			MakeFileSuffix(path,"prt");

			HTREEITEM hItem=_tree.ItemFromPath(path);
			if (hItem!=NULL)
			{
				_tree.EnsureVisible(hItem);
				_tree.SelectItem(hItem);
			}
		}
	}
}
