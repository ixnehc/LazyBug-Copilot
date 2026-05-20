#include "stdh.h"
#include ".\dummiesselectagent.h"
#include "GuiEditor_res.h"
#include "resdata/DummiesData.h"
#include "DummiesEditPanel.h"
#include "AxisArrow.h"
#include "spatialtester/spatialtester.h"

extern BOOL ProjectScaleMask(ICamera *camer,i_math::matrix43f &matTranf);

struct ZSort
{
	ZSort(){}
	ZSort(float z_,int idx_)
	{
		z = z_;
		idx = idx_;
	}
	float z;
	int idx;
};

CDummiesSelectAgent::CDummiesSelectAgent(void)
{
}

CDummiesSelectAgent::~CDummiesSelectAgent(void)
{
}
BOOL CDummiesSelectAgent::OnLButtonDown(int x,int y,DWORD flag)
{
	CGuiView * view = GetGuiView();
	
	CGuiData_Res * data =(CGuiData_Res *)view->FindData("resource");
	ResEditPanelState *state = data->FindState("Res_Dummies");
	CAxisArrow * arrow = ((Reps_Dummies*)state)->arrowMesh;
	DummiesData * dummiesData = (DummiesData*)state->resdata;
	
	i_math::matrix43f * matDefs = NULL;
	dummiesData->skeletonInfo.GetDefMatrix(matDefs);
	
	//////////////////////////////////////////////////////////////////////////
//	IRenderSystem *pRS = state->panel->GetRS();
//	IDummies * pDummies = pRS->GetDummiesMgr()->Create(dummiesData);
//	IMatrice43 * pMatrics = pRS->GetMatrice43Mgr()->Create();
//	pMatrics->Set(matDefs,dummiesData->skeletonInfo.size());
//	pDummies->Bind(pMatrics);
//	
//	IRenderPort * rp = GetRP();
//	HitProbe probe;
//	rp->CalcHitProbe(x,y,probe);
//	i_math::f64 len;
//	IDummy * pDummy= pDummies->HitTest(probe,len);
//
//	if(pDummy)
//	{
//		int idx = pDummy->GetIndex();
//		const char * name = pDummy->GetName();
//		int r =234;
//
//		if(idx!=-1) // if a dummy is selected
//		{
//			Reps_Dummies * stateDummies = (Reps_Dummies *)state;
//			stateDummies->dummyIdx = idx;
//			stateDummies->panel->ApplyStateMod(FALSE);
//		}
//	}
//
//	SAFE_DELETE(matDefs);
//	SAFE_RELEASE(pMatrics);
//	SAFE_RELEASE(pDummies);
	//////////////////////////////////////////////////////////////////////////
	i_math::matrix43f * matDummies =  new i_math::matrix43f[dummiesData->dummies.size()];
	i_math::matrix44f matView;
	ICamera * camer = GetRP()->GetCamera();
	camer->GetView(matView);

	for(int i=0;i<dummiesData->dummies.size();i++)
	{
		i_math::matrix43f mattemp =dummiesData->dummies[i].matOff*matDefs[dummiesData->dummies[i].idxBone];
		matDummies[i] = mattemp;
	}
	
	std::vector<ZSort> zsorts;
	HitProbe probe;
	IRenderPort * rp = GetRP();
	rp->CalcHitProbe(x,y,probe);
	i_math::line3df line;
	line.setLine(probe.start,probe.end);
	int idxSel = -1;
	SpacialTester spacialTester;
	spacialTester.Set(line);
	
	BOOL ret = FALSE;
	for(int i =0 ;i< dummiesData->dummies.size();i++)
	{
		i_math::matrix43f & mat = matDummies[i];
		i_math::matrix43f  matScale = mat;
		ProjectScaleMask(camer,matScale);
		ZSort sort;
		switch(dummiesData->dummies[i].bt)
		{
		case DummyInfo::BoundType_Point:
			{	
				ret = arrow->HitTest(line,&matScale);
				
				if(TRUE == ret)
				{
					i_math::vector3df pos(0,0,0);
					matDummies[i].transformVect(pos,pos);
					matView.transformVect(pos,pos);
					sort.z = pos.z;
					sort.idx = i;
					zsorts.push_back(sort);
				}
				break;
			}
		case DummyInfo::BoundType_AABB:
			{
				i_math::vector3df interEnter,interLeave;
				ret = _HitTestAbb(line,mat,dummiesData->dummies[i].aabb,interEnter,interLeave);
				if(ret&&interEnter.z!=interLeave.z)
				{
					matView.transformVect(interEnter,interEnter);
					matView.transformVect(interLeave,interLeave);					
					sort.z = (interEnter.z<interLeave.z)?interEnter.z:interLeave.z ;
					sort.idx = i;
					zsorts.push_back(sort);
				}
				break;
			}
		case DummyInfo::BoundType_Sphere:
			{
				i_math::vector3df pos(0,0,0),scale(1.0f,0.0f,0.0f);
				i_math::spheref sph;
		
				mat.transformVect(pos,pos);
				mat.transformVect(scale,scale);
				scale -= pos;
				float  s =(float)scale.getLength();

				sph.set(pos,dummiesData->dummies[i].sphere.radius*s);
		
				i_math::f64 dist;	
				ret = sph.getIntersectionWithLine(line,dist);
				pos = line.end - line.start;
				pos.setLength((float)dist);
				pos -= line.start;
				
				if(TRUE == ret)	
				{
					matView.transformVect(pos,pos);
					sort.z = pos.z;
					sort.idx = i;
					zsorts.push_back(sort);
				}

				break;
			}
		}
	}

	int zt = zsorts.size();
	if(zt)
	{
		float z = zsorts[0].z;
		int ii = zsorts[0].idx;	
		if(zt>1)
			for(int iz =1;iz<zt;iz++)
			{
				if(zsorts[iz].z<z)
				{
					z = zsorts[iz].z;
					ii = zsorts[iz].idx;
				}
			}	
		idxSel = ii;
	}

	if(idxSel!=-1) // if a dummy is selected
	{
		Reps_Dummies * stateDummies = (Reps_Dummies *)state;
		stateDummies->dummyIdx = idxSel;
		stateDummies->panel->ApplyStateMod(FALSE);
	}

	SAFE_DELETE(matDummies);
	SAFE_DELETE(matDefs);
	return TRUE;
}

BOOL CDummiesSelectAgent::OnRButtonClick(int x,int y,DWORD flag)
{
	return TRUE;
}

BOOL CDummiesSelectAgent::OnCommand(DWORD idCmd)
{
	return TRUE;
}

void CDummiesSelectAgent::OnDrag(int x,int y)
{
	
}
BOOL CDummiesSelectAgent::_HitTestAbb(i_math::line3df &line,i_math::matrix43f &mat,i_math::aabbox3df &aabb,i_math::vector3df &interEnter,i_math::vector3df &interLeave)
{
	i_math::matrix43f matInverse;
	matInverse = mat;
	matInverse.makeInverse();

	i_math::line3df lineInverse;
	i_math::vector3df vecEnter,vecLeave,vecLine;
	matInverse.transformVect(line.start,lineInverse.start);
	matInverse.transformVect(line.end,lineInverse.end);			
	
	vecLine = lineInverse.end - lineInverse.start;
	bool ret = aabb.calcIntersectionWithLine(lineInverse.start,vecLine,vecEnter,vecLeave);
	if(ret)
	{
		interEnter = vecEnter;
		interLeave = vecLeave;
		return TRUE;
	}
	return FALSE;
}


