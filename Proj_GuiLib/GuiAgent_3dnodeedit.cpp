#include "stdh.h"
#include "commondefines/general_stl.h"

#include <vector>
#include <string>


#include "AgentCmdID.h"

#include "GuiAgent_3dnodeedit.h"

#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetSystem.h"

#include "WorldSystem/IAssetRenderer.h"

#include "WorldSystem/IAnimNodes.h"

#include "timer/profiler.h"
#include "stringparser/stringparser.h"

#include "shaderlib/SLFeature.h"

#include "ModBlockBack.h"

#include "GuiData.h"

#include "RenderSystem/IMesh.h"

//WS 代表with selections
class CModBlockBack_WS:public CModBlockBack
{
public:
	CModBlockBack_WS(CGeView * view):
		CModBlockBack(view)
	{
		_selbuf=NULL;
		_ver=NULL;
	}

	void SetSelBuf(std::vector<H3DNode> *selbuf)
	{
		_selbuf=selbuf;
	}
	void SetSels(H3DNode *sels,DWORD nSel)
	{
		VEC_SET_BUFFER(_sels,sels,nSel);
	}

	void SetVer(DWORD *ver)
	{
		_ver=ver;
	}

protected:
	virtual void OnRestore()
	{
		if (_selbuf)
		{
			_selbuf->swap(_sels);
		}
		if (_ver)
			(*_ver)++;
	}

	std::vector<H3DNode> *_selbuf;
	std::vector<H3DNode> _sels;

	DWORD *_ver;

};

//sels/nSels为备份的selections
//ver为版本号的指针
void SaveAndBackupBlks(CGuiAgent *agent,i_math::pos2di *blks,DWORD nBlks,std::vector<H3DNode> *selbuf,H3DNode*sels,DWORD nSels,DWORD *ver)
{
	CModBlockBack_WS * mod=NULL;
	CModManager * modmgr = agent->GetActor()->GetModMgr();

	if(modmgr)
	{
		mod=new CModBlockBack_WS(agent->GetGuiView());
		mod->SetSelBuf(selbuf);
		mod->SetSels(sels,nSels);
		mod->SetVer(ver);
		mod->BackupBlocks(blks,nBlks);
	}
	GuiData_System *data=(GuiData_System *)agent->FindData("system");
	data->pES->SaveToMap();

	Mod_New(modmgr,(CModBase *&)mod);

}




BOOL CGuiAgent_3DNodeOperate::_Select(int x,int y,DWORD flag,int mode)
{
	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;
	HitProbe probe;
	if (!rp->CalcHitProbe(x,y,probe,2000.0f))
		return TRUE;

	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return TRUE;

	if (mode==1)
	{
		if (buf->size()>0)
			return TRUE;
	}

	H3DNode hit=_HitTest(probe);
	if (hit!=H3DNode_Invalid)
	{
		if (!(flag&CtrlOpFlag_CtrlDown))
		{
			buf->clear();
			buf->push_back(hit);
		}
		else
		{
			int idx;
			VEC_FIND(*buf,hit,idx);
			if (idx==-1)
				buf->push_back(hit);
			else
				buf->erase(buf->begin()+idx);
		}
		_Redraw(FALSE);
		return FALSE;//no need to continue
	}
	else
	{
		if (!(flag&CtrlOpFlag_CtrlDown))
			buf->clear();
		_Redraw(FALSE);
	}
	return TRUE;
}

BOOL CGuiAgent_3DNodeOperate::OnLButtonDown(int x,int y,DWORD flag)
{
	return _Select(x,y,flag,0);
}

BOOL CGuiAgent_3DNodeOperate::OnRButtonDown(int x,int y,DWORD flag)
{
	return TRUE;
	return _Select(x,y,flag,1);
}


BOOL CGuiAgent_3DNodeOperate::OnRButtonClick(int x,int y,DWORD flag)
{
	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return TRUE;

	if (buf->size()>0)
	{
		if (_NeedRemove())
			_AddMenu("Delete",ID_AGENT_3DNODEEDIT_REMOVE);
		if (_NeedClone())
			_AddMenu("Clone",ID_AGENT_3DNODEEDIT_CLONE);
	}

	return TRUE;

}

BOOL CGuiAgent_3DNodeOperate::OnKeyDown(char c,DWORD flag)
{
	if (_NeedRemove())
	{
		if (c==46)
		{
			OnCommand(ID_AGENT_3DNODEEDIT_REMOVE);
			return FALSE;
		}
	}
	return TRUE;
}



BOOL CGuiAgent_3DNodeOperate::OnCommand(DWORD idCmd)
{
	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return TRUE;

	if (idCmd==ID_AGENT_3DNODEEDIT_REMOVE)
	{
		std::vector<i_math::pos2di>affected;
		std::vector<H3DNode> removed;
		std::vector<H3DNode> backup;
		backup=*buf;
		for (int i=0;i<buf->size();i++)
		{
			if (_Remove((*buf)[i]))
			{
				_IncVer();
				removed.push_back((*buf)[i]);
				i_math::pos2di *ptAffect=_GetBlock((*buf)[i]);
				if (ptAffect)
				{
					UNIQUE_VEC_ADD(affected,*ptAffect);
				}
			}
		}
		if (removed.size()<=0)
			return TRUE;//什么都删不掉

		for (int i=0;i<removed.size();i++)
		{
			int idx;
			VEC_FIND(*buf,removed[i],idx);
			if (idx!=-1)
				buf->erase(buf->begin()+idx);
		}

		if (_NeedBackup())
			SaveAndBackupBlks(this,affected.data(),affected.size(),buf,backup.data(),backup.size(),_GetVer());

		return FALSE;
	}

	if (idCmd==ID_AGENT_3DNODEEDIT_CLONE)
	{
		std::vector<i_math::pos2di>affected;
		std::vector<H3DNode> backup;
		std::vector<H3DNode> clones;
		backup=*buf;
		for (int i=0;i<buf->size();i++)
		{
			H3DNode h=_Clone((*buf)[i]);
			if (h!=H3DNode_Invalid)
			{
				_IncVer();
				clones.push_back(h);
				i_math::pos2di *ptAffect=_GetBlock((*buf)[i]);
				if (ptAffect)
				{
					UNIQUE_VEC_ADD(affected,*ptAffect);
				}
			}
		}

		(*buf)=clones;

		if (_NeedBackup())
			SaveAndBackupBlks(this,affected.data(),affected.size(),buf,backup.data(),backup.size(),_GetVer());

		return FALSE;
	}

	return TRUE;
}

void DrawEnvelope(IRenderPort *rp,Envelope &evlp,DWORD c)
{
	for (int i=0;i<evlp.meshes.size();i++)
	{
		Envelope::Mesh *p=&evlp.meshes[i];

		ShaderState state;
		if (p->b2Sided)
			state.modeFacing=Facing_Both;
		if ((p->mesh->GetFeature().Test(FC_bones1|FC_bones2|FC_bones3|FC_bones4))&&p->matrices)
			rp->SimpleDrawMesh(p->mesh,p->matrices->GetPtr(),p->matrices->GetCount(),ColorAlpha(c,0x3f),FALSE,NULL,NULL,&state);
		else
			rp->SimpleDrawMesh(p->mesh,p->mat,ColorAlpha(c,0x3f),FALSE,NULL,NULL,&state);
	}

	if (evlp.lines.size()>0)
	{
		rp->Lines(&evlp.lines[0],evlp.lines.size()/2,ColorAlpha(c,0x7f));
	}
}

void CGuiAgent_3DNodeOperate::_DrawEnvelope(Envelope &evlp)
{
	IRenderPort *rp=GetRP();
	DrawEnvelope(rp,evlp,0x00ff00);
}


BOOL CGuiAgent_3DNodeOperate::OnDraw()
{
	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (buf)
	{
		Envelope evlp;
		_CollectEnvelope((*buf).data(), buf->size(), evlp);
		_DrawEnvelope(evlp);
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//


CGuiAgent_3DNodeMatEdit::CGuiAgent_3DNodeMatEdit(Matrix_EditMode mode/* = EditMode_All*/):
CGuiAgent_MatrixEdit(mode)
{
	CGuiAgent_MatrixEdit::EventEdit dlgtBeginEdit;
	CGuiAgent_MatrixEdit::EventEdit dlgtEdit;
	CGuiAgent_MatrixEdit::EventEdit dlgtEndEdit;

	dlgtBeginEdit.bind(this,&CGuiAgent_3DNodeMatEdit::_BeginMatrixEdit);
	dlgtEdit.bind(this,&CGuiAgent_3DNodeMatEdit::_MatrixEdit);
	dlgtEndEdit.bind(this,&CGuiAgent_3DNodeMatEdit::_EndMatrixEdit);

	SetEventListener(dlgtBeginEdit,dlgtEdit,dlgtEndEdit);

	ShowSpaceMenu(FALSE);

	_bParentMat=FALSE;
	_bEditing=FALSE;
}

void CGuiAgent_3DNodeMatEdit::UpdateBind()
{
	if(_bEditing)
		return;

	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return;

	CGuiAgent_MatrixEdit::Enable(FALSE);

	BOOL bNeedRedraw=FALSE;
	if (!(*buf==_editings))
		bNeedRedraw=TRUE;//选中的,和当前正在编辑的不一样了,需要重画

	i_math::matrix43f matParent;

	_bParentMat=FALSE;

	ShowSpaceMenu(FALSE);

	if (buf->size()==1)
	{
		i_math::matrix43f *mat=_GetLocalMat((*buf)[0],matParent);
		if (mat)
		{
			_bParentMat=TRUE;

//			if (!_matWork.equals(*mat))
				bNeedRedraw=TRUE;

			CGuiAgent_MatrixEdit::Enable(TRUE);

// 			ShowSpaceMenu(TRUE);

			_matWork=*mat;
			MatrixEditData data;
			data.matrix=&_matWork;
			data.matParent=matParent;
			data.modespace=EditSpace_Parent;
			CGuiAgent_MatrixEdit::Bind(data);
		}
	}

	if (!_bParentMat)
	{
		if (buf->size()>0)
		{
			//求出这些selection的中心
			i_math::vector3df center;
			DWORD sz=0;
			for (int i=0;i<buf->size();i++)
			{
				i_math::matrix43f *mat=_GetMat((*buf)[i]);
				if (mat)
				{
					center+=mat->getTranslation();
					sz++;
				}
			}
			if (sz<=0)
				return;//相当于啥都没选中
			center/=(float)sz;

			CGuiAgent_MatrixEdit::Enable(TRUE);
			i_math::matrix43f matNew;
			if (buf->size()==1)
				matNew=*_GetMat((*buf)[0]);
			*matNew.getTranslationP()=center;
			if (!_matWork.equals(matNew))
				bNeedRedraw=TRUE;
			_matWork=matNew;

			MatrixEditData data;
			data.matrix=&_matWork;
			CGuiAgent_MatrixEdit::Bind(data);
		}
		else
		{
			MatrixEditData data;
			data.matrix=NULL;
			CGuiAgent_MatrixEdit::Bind(data);
		}
	}

	_editings=*buf;

	if (bNeedRedraw)
		_Redraw(FALSE);
}

void CGuiAgent_3DNodeMatEdit::_BeginMatrixEdit(i_math::matrix43f *mat0)
{
	if (_bEditing)
		return;
	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return;

	_backups=*buf;//先把修改前的selections备份下来,因为修改过程中,可能会导致selections中的值发生变化

	_affected.clear();

	if (!_bParentMat)
	{
		_matsLocal.resize(buf->size());
		_editings=*buf;
		i_math::matrix43f matCenterInv=*mat0;
		matCenterInv.makeInverse();

		for (int i=0;i<_editings.size();i++)
		{
			H3DNode node=_editings[i];
			i_math::matrix43f *	mat=_GetMat(node);
			if (mat)
				_matsLocal[i]=(*mat)*matCenterInv;
			else
				_matsLocal[i].makeIdentity();

			i_math::pos2di *ptBlk=_GetBlock(node);

			if (ptBlk)
			{
				UNIQUE_VEC_ADD(_affected,*ptBlk);
			}
		}
	}
	else
	{
		_editings=*buf;
		if (_editings.size()>0)
		{
			H3DNode node=_editings[0];
			i_math::pos2di *ptBlk=_GetBlock(node);

			if (ptBlk)
			{
				UNIQUE_VEC_ADD(_affected,*ptBlk);
			}
		}
	}

	_bEditing=TRUE;
}

void CGuiAgent_3DNodeMatEdit::_MatrixEdit(i_math::matrix43f *mat)
{
	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return;

	if (!_bParentMat)
	{
		assert(_matsLocal.size()==_editings.size());
		assert(_editings==*buf);

		for (int i=0;i<_editings.size();i++)
		{
			H3DNode node=_editings[i];

			i_math::matrix43f matCur;
			matCur=_matsLocal[i]*(*mat);

			_Move(node,matCur);

			_editings[i]=node;

		}
	}
	else
	{
		if (_editings.size()>0)
		{
			H3DNode node=_editings[0];
			_MoveLocal(node,*mat);
			_editings[0]=node;
		}
	}
	*buf=_editings;
}

void CGuiAgent_3DNodeMatEdit::_EndMatrixEdit(i_math::matrix43f *mat)
{
	_MatrixEdit(mat);

	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return;

	for (int i=0;i<buf->size();i++)
	{
		H3DNode node=(*buf)[i];
		i_math::pos2di *ptBlk=_GetBlock(node);

		if (ptBlk)
		{
			UNIQUE_VEC_ADD(_affected, *ptBlk);
		}
	}

	_IncVer();

	if (_NeedBackup())
		SaveAndBackupBlks(this,_affected.data(),_affected.size(),buf,_backups.data(),_backups.size(),_GetVer());

	_bEditing=FALSE;
}


BOOL CGuiAgent_3DNodeMatEdit::OnRButtonClick(int x,int y,DWORD flag)
{
	CGuiAgent_MatrixEdit::OnRButtonClick(x,y,flag)	;
	_AddMenuSep();
	_AddMenu("对齐到表面",ID_AGENT_3DNODEEDIT_ALIGNTOSURF);

	return TRUE;

}

i_math::vector3df getClosestPosOnTriangle(i_math::triangle3df &tri,i_math::vector3df &pos)
{
	i_math::plane3df pl;
	pl=tri.getPlane();
	i_math::vector3df posProj;
	pl.getProjectionOf(pos,posProj);
	if (tri.isPointInside(posProj))
		return posProj;
	return tri.closestPointOnTriangle(posProj);
}

i_math::matrix43f AlignToSurf(i_math::matrix43f &mat,AssetSystemState *ss,i_math::matrix43f *ignores,DWORD cIgnores)
{
	i_math::vector3df posOrg=mat.getTranslation();
	float range=1.0f;

	i_math::aabbox3df aabb;
	aabb.reset(posOrg);
	aabb.inflate(range,range,range);

	DWORD c;
	RatomID *ratoms=ss->adr->EnumRatoms(aabb,c);


	float dist2Min=10000.0f;
	i_math::triangle3df triClosest;
	i_math::vector3df posClosest;
	BOOL bFound=FALSE;

	for (int i=0;i<c;i++)
	{
		RatomID ratom=ratoms[i];
		RagentType tp=ss->adr->RagentFromRatom(ratom);
		if (tp==Ragent_Default)
		{
			if (!ss->adr->CheckRatomMotive(ratom))
			{
				IAnimNode *an=NULL;
				IMesh *msh=ss->ratomsDefault->GetMesh(ratom,an);
				if (msh)
				{
					i_math::matrix43f *matRatom=an->GetMat(ss->t);
					if (matRatom)
					{
						BOOL bIgnored=FALSE;
						for(int i=0;i<cIgnores;i++)
						{
							if (ignores[i]==*matRatom)
							{
								bIgnored=TRUE;
								break;
							}
						}
						if (!bIgnored)
						{
							IMeshSnapshot *snap=msh->ObtainSnapshot();
							MeshSnapshotArg arg;
							if (snap->TakeSnapshot(*matRatom,arg))
							{
								WORD *indices=snap->GetIndices(0);
								DWORD nIndices=snap->GetIBCount(0);
								i_math::vector3df *pos=snap->GetPos();
								i_math::triangle3df tri;

								for (int i=0;i<nIndices;i+=3)
								{
									tri.pointA=pos[indices[i]];
									tri.pointB=pos[indices[i+1]];
									tri.pointC=pos[indices[i+2]];

									i_math::vector3df posOnTri=getClosestPosOnTriangle(tri,posOrg);
									float dist2=posOnTri.getDistanceFromSQ(posOrg);
									if (dist2<range*range)
									{
										if (dist2<dist2Min)
										{
											dist2Min=dist2;
											triClosest=tri;
											posClosest=posOnTri;
											bFound=TRUE;
										}
									}
								}
							}
							SAFE_RELEASE(snap);
						}
					}
				}
			}
		}
	}

	if (!bFound)
		return mat;

	i_math::plane3df pl=triClosest.getPlane();

	i_math::vector3df nml=pl.Normal;

	i_math::matrix43f matRet;
	i_math::xformf xfm;
	xfm.fromMatrix(mat);
	xfm.pos=posClosest;
	xfm.rot.alignY(nml);
	xfm.getMatrix(matRet);

	return matRet;
}

BOOL CGuiAgent_3DNodeMatEdit::OnCommand(DWORD idCmd)
{

	std::vector<H3DNode> *buf=(std::vector<H3DNode> *)_GetSelBuf();
	if (!buf)
		return TRUE;

	if (idCmd==ID_AGENT_3DNODEEDIT_ALIGNTOSURF)
	{
		_backups=*buf;//先把修改前的selections备份下来,因为修改过程中,可能会导致selections中的值发生变化

		_affected.clear();

		std::vector<H3DNode> editing=*buf;

		std::vector<i_math::matrix43f> ignores;
		for (int i=0;i<editing.size();i++)
		{
			H3DNode node=_editings[i];

			i_math::matrix43f *	mat=_GetMat(node);
			if (mat)
				ignores.push_back(*mat);
		}

		for (int i=0;i<editing.size();i++)
		{
			H3DNode node=_editings[i];
			i_math::pos2di *ptBlk=_GetBlock(node);
			if (ptBlk)
			{
				UNIQUE_VEC_ADD(_affected, *ptBlk);
			}

			i_math::matrix43f *	mat=_GetMat(node);
			if (mat)
			{
				i_math::matrix43f mat2=AlignToSurf(*mat,g_ssGuiLib.pES->GetAS()->GetSS(),ignores.data(),ignores.size());
				_Move(node,mat2);
			}

			ptBlk=_GetBlock(node);
			if (ptBlk)
			{
				UNIQUE_VEC_ADD(_affected, *ptBlk);
			}
		}

		_IncVer();

		if (_NeedBackup())
			SaveAndBackupBlks(this,_affected.data(),_affected.size(),buf,_backups.data(),_backups.size(),_GetVer());
		
		return FALSE;
	}

	return CGuiAgent_MatrixEdit::OnCommand(idCmd);

}
