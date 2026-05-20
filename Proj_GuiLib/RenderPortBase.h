#pragma once

#include "GuiLib.h"

#include "RenderSystem/IRenderSystem.h"
#include "spatialtester/spatialtester.h"

#pragma warning(disable:4091)

typedef i_math::matrix43f;
GuiLib_Api BOOL DrawDefSkeleton(IRenderPort *rp,ISkeleton *skeleton,i_math::matrix43f *xfm=NULL,DWORD col=0xffffffff);

GuiLib_Api BOOL DrawMeshSnapshot(IRenderPort *rp,IMesh *mesh,DWORD col);

GuiLib_Api BOOL DrawGrid(IRenderPort *rp,DWORD d,DWORD gap);

GuiLib_Api BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df &aabb,DWORD col);

struct ProfilerMgr;
GuiLib_Api BOOL DrawProfile(IRenderPort *rp,ProfilerMgr *mgr,i_math::pos2di &pt);

GuiLib_Api BOOL DrawCvxVolume(IRenderPort *rp,ICvxVolume * pVol);

GuiLib_Api BOOL DrawCapsule(IRenderPort * rp,i_math::capsulef & cap);


GuiLib_Api void CalShinesColor(i_math::vector3df &eyePoint,i_math::vector3df &point,i_math::vector3df &normal,DWORD &col);

BOOL ProjectScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf);

BOOL GetProjScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf,i_math::matrix43f &matScale);

GuiLib_Api void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);

struct MeshHitTestResult
{
	MeshHitTestResult()
	{
		indices = NULL;
		nNum = 0;
	}
	WORD * indices;
	SpacialTester::Result testResult;
	DWORD nNum;
	void Release() 
	{
		SAFE_DELETE(indices);
	}
};

MeshHitTestResult HitTest(IMeshSnapshot * meshSnap,SpacialTester & spacialTester);

