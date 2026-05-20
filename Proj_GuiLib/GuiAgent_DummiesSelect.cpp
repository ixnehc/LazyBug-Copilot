#include "stdh.h"
#include "GuiAgent_DummiesSelect.h"
#include "GuiEditor_res.h"
#include "resdata/DummiesData.h"
#include "DummiesEditPanel.h"
#include "AxisArrow.h"
#include "spatialtester/spatialtester.h"

#include <set>

extern BOOL ProjectScaleMask(IRenderPort *camer,i_math::matrix43f &matTranf);

struct ZSort
{
	ZSort(){}
	ZSort(float z_,int idx_){z = z_;idx = idx_;	}
	bool operator < (const ZSort & oth) const {return z<oth.z;}
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
	if(!data) 
		return TRUE;

	ResEditPanelState *state = data->GetState();
	CAxisArrow * arrow = ((Reps_Dummies*)state)->arrowMesh;
	DummiesData * dummiesData = (DummiesData*)state->resdata;
	
	i_math::matrix43f * matDefs = NULL;
	dummiesData->skeletonInfo.GetDefMatrix(matDefs);

	i_math::matrix43f * matDummies =  new i_math::matrix43f[dummiesData->dummies.size()];
	i_math::matrix44f matView;
	ICamera * camer = GetRP()->GetCamera();
	camer->GetView(matView);

	for(int i=0;i<dummiesData->dummies.size();i++)
	{
		i_math::matrix43f mattemp =dummiesData->dummies[i].matOff*matDefs[dummiesData->dummies[i].idxBone];
		matDummies[i] = mattemp;
	}
	
	std::set<ZSort> zDepth;
	
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
		ProjectScaleMask(rp,matScale);
		ZSort sort;
		
		DWORD btype = dummiesData->dummies[i].getBoundType();
		
		switch(btype)
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
					zDepth.insert(sort);
				}
				break;
			}
		case DummyInfo::BoundType_AABB:
			{
				i_math::vector3df interEnter,interLeave;

				i_math::aabbox3df * abb = dummiesData->dummies[i].getAAbb();
				assert(abb);

				ret = _HitTestAbb(line,mat,*abb,interEnter,interLeave);

				if(ret&&interEnter.z!=interLeave.z)
				{
					matView.transformVect(interEnter,interEnter);
					matView.transformVect(interLeave,interLeave);					
					sort.z = (interEnter.z<interLeave.z)?interEnter.z:interLeave.z ;
					sort.idx = i;
					//zsorts.push_back(sort);
					zDepth.insert(sort);
				}
				break;
			}
		case DummyInfo::BoundType_Sphere:
			{
				i_math::spheref * pSph = dummiesData->dummies[i].getSphere();
				
				i_math::spheref sph;
				i_math::vector3df pos;
				mat.transformSphere(*pSph,sph);
		
				i_math::f64 dist;	
				ret = sph.getIntersectionWithLine(line,dist);
				pos = line.end - line.start;
				pos.setLength((float)dist);
				pos += line.start;
				if(TRUE == ret)	
				{
					matView.transformVect(pos,pos);
					sort.z = pos.z;
					sort.idx = i;
					zDepth.insert(sort);
				}

				break;
			}
		}
	}

	if(zDepth.size()>0)
	{
		std::set<ZSort>::iterator it = zDepth.begin();
		idxSel =(*it).idx;
	}
	
	if(idxSel!=-1) // if a dummy is selected
	{
		Reps_Dummies * stateDummies = (Reps_Dummies *)state;
		stateDummies->dummyIdx = idxSel;
		stateDummies->panel->RefreshStateMod();
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

void CDummiesSelectAgent::OnDrag(int x,int y,DWORD flag)
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
		
	mat.transformVect(vecEnter,interEnter);
	mat.transformVect(vecLeave,interLeave);

	return (ret==true);
}


