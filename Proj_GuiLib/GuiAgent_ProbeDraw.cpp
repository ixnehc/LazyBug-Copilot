

#include "stdh.h"
#include "RenderSystem/IVertexBuffer.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IRenderPort.h"
#include "shaderlib/SLEffectParam.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IFont.h"


#include "GuiData_El.h"

#include "GuiAgent_ProbeDraw.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IAssetSystem.h"

#include "FileSystem/IMapFileDefines.h"

#include "stringparser/stringparser.h"

#include "GuiActor_El.h"

#include "fvfex/fvfex.h"

#include "timer/profiler.h"

#define NUM_VB_ELDRAW	3000
#define NUM_IB_ELDRAW	9000
#define FVFEX_VTX	FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_DIFFUSE
CGuiAgent_ElDraw::CGuiAgent_ElDraw()
{
	_pVb = NULL;
	_pIb = NULL;
	_meshSample = NULL;
	_mtrlSample = NULL;
	_snapMsh = NULL;
}
CGuiAgent_ElDraw::~CGuiAgent_ElDraw()
{
	SAFE_RELEASE(_pVb);
	SAFE_RELEASE(_pIb);
	SAFE_RELEASE(_meshSample);
	SAFE_RELEASE(_mtrlSample);
	SAFE_RELEASE(_snapMsh);
}
void CGuiAgent_ElDraw::_EnumProbeCube()
{
	_editor = NULL;
	_hObjs.clear();

	GuiData_El * dataEl = (GuiData_El *)FindData("envlight");	
	i_math::vector3df center=dataEl->pES->GetAS()->GetCenter();
	if(dataEl)
		_editor = dataEl->GetEditor();

	const int rg = 500;

	i_math::pos2di ptCenter;
	ptCenter.x = int(center.x);
	ptCenter.y = int(center.z);

	i_math::pos2di pt0,pt1;
	pt0 = ptCenter - i_math::pos2di(rg,rg);
	pt1 = ptCenter + i_math::pos2di(rg,rg);

	DWORD count = 0;
	HMapObj * pHObj = NULL;
	if(_editor)
		pHObj = _editor->Enum(i_math::recti(pt0,pt1),count);

	_hObjs.resize(count);
	memcpy(_hObjs.data(),pHObj,count*sizeof(HMapObj));
}

void CGuiAgent_ElDraw::_AddAbbToDraw(i_math::aabbox3df &abb)
{
	i_math::vector3df corner[8];

	abb.getCorners(corner);

#define Fill_Line(i0,i1){			\
	_lines.push_back(corner[i0]);	\
	_lines.push_back(corner[i1]);	\
}
	Fill_Line(0,1)
		Fill_Line(0,4)
		Fill_Line(5,1)
		Fill_Line(5,4)

		Fill_Line(2,6)
		Fill_Line(2,3)
		Fill_Line(7,6)
		Fill_Line(7,3)

		Fill_Line(2,0)
		Fill_Line(3,1)
		Fill_Line(7,5)
		Fill_Line(6,4)
}
void CGuiAgent_ElDraw::OnAttachView(CGeView *view,DWORD iLevel)
{
	if(_pVb&&_pIb)
		return;

	IRenderSystem * pRS = NULL;
	IRenderPort * rp = GetRP();
	if(rp)
		pRS = rp->GetRS();
	
	if(!_pVb)
		_pVb = pRS->GetVertexMgr()->CreateVB(NUM_VB_ELDRAW,FVFEX_VTX,1,VBFlag_Dynamic);
	
	if(!_pIb)
		_pIb = pRS->GetVertexMgr()->CreateIB(NUM_IB_ELDRAW,VBFlag_Dynamic);

	if(!_meshSample)
		_meshSample = (IMesh *)pRS->GetMeshMgr()->ObtainRes("_editor\\box.msh");

	if(!_mtrlSample)
		_mtrlSample = (IMtrl *)pRS->GetMtrlMgr()->ObtainRes("_editor\\envSample.mtl");
}

void CGuiAgent_ElDraw::_FillDrawSamp()
{
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data||0==data->hObjSels.size())
		return;
	
	HMapObj hObj = data->hObjSels.back();

	IProbeCubeMapEditor * editor = data->GetEditor();
	if(!editor)
		return;

	DWORD count = 0;
	BOOL * bBaked = NULL;
	const ProbeInfo * pSamples = editor->GetProbes(hObj,&count,NULL,FALSE);

	if(A_Ok!=_meshSample->Touch()||!pSamples)
		return;
	
	if(!_snapMsh){
		_snapMsh = _meshSample->ObtainSnapshot();
		i_math::matrix43f mat;
		mat.setScale(0.2f,0.2f,0.2f);
		MeshSnapshotArg snapArg;
		_snapMsh->TakeSnapshot(mat,snapArg);
	}
	
	DWORD nVtx = _snapMsh->GetVBCount();
	DWORD nIB = _snapMsh->GetIBCount(0);

	i_math::vector3df * vPos = _snapMsh->GetPos();
	i_math::vector3df * vNor = _snapMsh->GetNormal();
	WORD * vIB = _snapMsh->GetIndices(0);

	BYTE * pVB = (BYTE *)_pVb->Lock(TRUE,FVFEX_VTX);
	WORD * pIB = (WORD *)_pIb->Lock(TRUE);
	if(!pVB||!pIB){
		_pVb->Unlock();
		_pIb->Unlock();
		return;
	}
	
	DWORD strideMesh = nVtx*_pVb->GetStride();

	WORD pv = 0;
	DWORD pi = 0;
	std::vector<WORD> indices(nIB);
	std::vector<i_math::vector3df> vtx(nVtx);
	std::vector<DWORD> cols(nVtx);

	for(int i = 0;i<count;i++){
		
		//判断是否会发生溢出
		if(pv+nVtx>NUM_VB_ELDRAW||pi+nIB>NUM_IB_ELDRAW){
			//结束本次填充
			_pVb->Unlock();
			_pIb->Unlock();
			assert(pi!=0);
			//绘制
			_DrawSamp(pi/3);

			//开始下一次填充绘制
			pVB = (BYTE *)_pVb->Lock(TRUE,FVFEX_VTX);
			pIB = (WORD *)_pIb->Lock(TRUE);

			pv = 0;
			pi = 0;
		}

		for(int k = 0;k<nVtx;k++){
			vtx[k] = vPos[k] + pSamples[i].pos;
			cols[k] = (pSamples[i].bBaked)?0xfffff800:0xffffffff;
		}

		fvfCopy(nVtx,pVB ,FVFEX_VTX,&(vtx[0]),FVFEX_XYZ0);
		fvfCopy(nVtx,pVB,FVFEX_VTX,vNor,FVFEX_NORMAL0);
		fvfCopy(nVtx,pVB,FVFEX_VTX,&(cols[0]),FVFEX_DIFFUSE);
		
		for(int k = 0;k<nIB;k++)
			indices[k] = vIB[k] + pv;

		memcpy(pIB,indices.data(),nIB*sizeof(WORD));
		
		pv += WORD(nVtx);
		pi += nIB;
		pVB += strideMesh;
		pIB += nIB;
	}
	
	//最后一次填充 但没有溢出
	if(pi>0){
		_pVb->Unlock();
		_pIb->Unlock();
		_DrawSamp(pi/3);
	}
}
void CGuiAgent_ElDraw::_DrawSHMap()
{
	IRenderPort * rp = GetRP();
	assert(_editor);

	GuiData_El * data = (GuiData_El *) FindData("envlight");
	assert(data);

	std::vector<i_math::vector3df> lines;
	i_math::triangle3df * tris = NULL;
	DWORD nTris = 0;
	DWORD nSamples = 0;
	float * dist = NULL;
	
	if(TRUE){
		IEnvLight * pKL = data->GetIEnvLight();
		i_math::vector3df *p0 = NULL,*p1 = NULL;
		pKL->GetSHMap(p0,p1,dist,nSamples,tris,nTris);
		lines.clear();
		for(int i = 0;i<nSamples;i++){
			lines.push_back(p0[i]);
			lines.push_back(p1[i]);
		}
		if(lines.size()>0)
			rp->Lines(lines.data(),lines.size()/2,0xffffff00);

		//绘制三角形线框
		if(FALSE){
			lines.clear();
			for(int i = 0;i<nTris;i++){
				i_math::triangle3df &tri = tris[i];
				i_math::vector3df vec(0,0,0);
				lines.push_back(tri.pointA);
				lines.push_back(tri.pointB);
				lines.push_back(tri.pointC);
			}
			if(lines.size()>0)
				rp->Triangles(lines.data(),lines.size()/3,0xff00fff0);
		}
	}
}

void CGuiAgent_ElDraw::_DrawSamp(int nPrim)
{
	IRenderPort * rp = GetRP();
	IRenderer * render = rp->ObtainRenderer();
	ICamera * cam = rp->GetCamera();
	
	i_math::vector3df eyeDir;
	cam->GetEyeDir(eyeDir);

	VBBindArg arg;
	arg.primcount = nPrim;
	arg.primstart = 0;
	IShader * shader = render->BeginRaw(_mtrlSample);
	if(shader){
		_mtrlSample->BindEP(shader,0);
		_mtrlSample->BindState(shader,0);
		shader->SetEP(EPG_fx3_01,eyeDir);
		shader->BindVB(_pVb,_pIb,&arg);
		shader->DoShadeRaw(); // do draw
		render->EndRaw(shader);
	}
}

BOOL CGuiAgent_ElDraw::OnDraw()
{
	_EnumProbeCube();

	if(!_editor)
		return TRUE;

	_lines.clear();

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	assert(data);

	ProbeCubeInfo info;
	for(int i = 0;i<_hObjs.size();i++){
		HMapObj &hObj =  _hObjs[i];

		int k = 0;
		for(;k<data->hObjSels.size();k++){
			if(data->hObjSels[k]==hObj)
				break;
		}

		if(k<data->hObjSels.size())
			continue;

		if(_editor->GetInfo(hObj,info)){
			_AddAbbToDraw(info.aabb);
		}
	}

	IRenderPort *rp=GetRP();
	assert(rp);

	//场景中的线框
	if(_lines.size()>0)
		rp->Lines(_lines.data(),_lines.size()/2,0xff0077ff);

	//绘制信息
	if(TRUE){
		int nValid = 0;
		int total = _hObjs.size();
		for(int i = 0;i<_hObjs.size();i++){
			if(_editor->IsValid(_hObjs[i]))
				nValid++;
		}
		std::string msgDump;
		DrawFontArg argFont;
		FormatString(msgDump,"Total:%d Valid: %d",total,nValid);
		rp->DrawText(msgDump.c_str(),argFont);
	}

	CGuiPanel_El * pActor = (CGuiPanel_El *)GetActor();
	assert(pActor);

	//绘制采样点
	if(data->bShowSample)
		_FillDrawSamp();

	//绘制格线
	if(data->bShowGrid)
		_DrawGrid();

	if(TRUE)
		_DrawSHMap();

	return TRUE;
}
void BackClip(i_math::aabbox3df &abb,i_math::vector3df & eyePos,BOOL * visible)
{
	if(abb.isPointInside(eyePos)){
		for(int i = 0;i<6;i++)
			visible[i] = TRUE;
	}
	else
	{
		float x0 = abb.MinEdge.x;
		float x1 = abb.MaxEdge.x;
		float y0 = abb.MinEdge.y;
		float y1 = abb.MaxEdge.y;
		float z0 = abb.MinEdge.z;
		float z1 = abb.MaxEdge.z;

		float nX,nY,nZ; //距离相机近的面
		float fX,fY,fZ; //距离相机远的面
		BOOL pX = FALSE,pY = FALSE,pZ = FALSE;

		i_math::plane3df p0,p1;
		// +X : -X
		{
			p0.setPlane(abb.MinEdge,i_math::vector3df(1.0f,0,0));
			p1.setPlane(abb.MaxEdge,i_math::vector3df(1.0f,0,0));

			float d0 = p0.getDistanceTo(eyePos);
			float d1 = p1.getDistanceTo(eyePos);
			if(abs(d0)<abs(d1)){
				nX = x0;
				fX = x1;
			}
			else{
				nX = x1;
				fX = x0;
				pX = TRUE;
			}
		}

		//+Y : -Y
		{
			p0.setPlane(abb.MinEdge,i_math::vector3df(0,1.0f,0));
			p1.setPlane(abb.MaxEdge,i_math::vector3df(0,1.0f,0));

			float d0 = p0.getDistanceTo(eyePos);
			float d1 = p1.getDistanceTo(eyePos);

			if(abs(d0)<abs(d1)){
				nY = y0;
				fY = y1;
			}
			else{
				nY = y1;
				fY = y0;
				pY = TRUE;
			}
		}

		//+Z : -Z
		{
			p0.setPlane(abb.MinEdge,i_math::vector3df(0,0,1.0f));
			p1.setPlane(abb.MaxEdge,i_math::vector3df(0,0,1.0f));

			float d0 = p0.getDistanceTo(eyePos);
			float d1 = p1.getDistanceTo(eyePos);

			if(abs(d0)<abs(d1)){
				nZ = z0;
				fZ = z1;
			}
			else{
				nZ = z1;
				fZ = z0;
				pZ = TRUE;
			}		
		}

		//可见性检测
		BOOL bX = TRUE ,bY = TRUE ,bZ = TRUE;
		i_math::line3df lineHit;
		i_math::vector3df cor,vecLine;
		i_math::vector3df vecEnter,vecLeave;
		// X
		{
			cor.set(nX,fY,fZ);
			vecLine = cor - eyePos;
			vecLine.normalize();
			if(abb.calcIntersectionWithLine(eyePos,vecLine,vecEnter,vecLeave)){
				float sQ = (float)(vecEnter-cor).getLengthSQ();
				if(sQ>0.01f)
					bX = FALSE;
			}
		}
		//Y
		{
			cor.set(fX,nY,fZ);
			vecLine = cor - eyePos;
			vecLine.normalize();
			if(abb.calcIntersectionWithLine(eyePos,vecLine,vecEnter,vecLeave)){
				float sQ = (float)(vecEnter-cor).getLengthSQ();
				if(sQ>0.01f)
					bY = FALSE;
			}
		}
		//Z
		{
			cor.set(fX,fY,nZ);
			vecLine = cor - eyePos;
			vecLine.normalize();
			if(abb.calcIntersectionWithLine(eyePos,vecLine,vecEnter,vecLeave)){
				float sQ = (float)(vecEnter-cor).getLengthSQ();
				if(sQ>0.01f)
					bZ = FALSE;
			}
		}

		for (int i = 0;i<6;i++)
			visible[i] = FALSE;

		if(bX){
			if(pX)
				visible[0] = TRUE;
			else
				visible[1] = TRUE;
		}

		if(bY){
			if(pY)
				visible[2] = TRUE;
			else 
				visible[3] = TRUE;
		}

		if(bZ){
			if(pZ)
				visible[4] = TRUE;
			else
				visible[5] = TRUE;
		}
	}
}

void CGuiAgent_ElDraw::_DrawGrid()
{
	if(!_editor)
		return;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	assert(data);

	if(!data->hObjSels.size())
		return;

	HMapObj & hObj = data->hObjSels.back();
	ProbeCubeInfo info;

	if(!_editor->GetInfo(hObj,info))
		return;

	IRenderPort * rp = GetRP();
	ICamera * camera = rp->GetCamera();
	i_math::vector3df eyeDir;
	camera->GetEyeDir(eyeDir);

	std::vector<float> xAxis;
	std::vector<float> yAxis;
	std::vector<float> zAxis;

	i_math::vector3df vec = info.aabb.MaxEdge - info.aabb.MinEdge;
	float x0 = info.aabb.MinEdge.x;
	float y0 = info.aabb.MinEdge.y;
	float z0 = info.aabb.MinEdge.z;
	
	int c = int(vec.x/info.density);
	for(int i = 0;i<c;i++){
		x0 += info.density;
		xAxis.push_back(x0);
	}

	c = int(vec.y/info.density);
	for (int i = 0;i<c;i++) {
		y0 += info.density;
		yAxis.push_back(y0);
	}

	c = int(vec.z/info.density);
	for (int i = 0;i<c;i++) {
		z0 += info.density;
		zAxis.push_back(z0);
	}
	
	float x[] = {info.aabb.MaxEdge.x,info.aabb.MinEdge.x};
	float y[] = {info.aabb.MaxEdge.y,info.aabb.MinEdge.y};
	float z[] = {info.aabb.MaxEdge.z,info.aabb.MinEdge.z};

	i_math::vector3df eyePos;
	camera->GetEyePos(eyePos);
	BOOL bv[6] = {FALSE};
	BackClip(info.aabb,eyePos,bv);
	
	_lineGridFore.clear();
	_lineGridBack.clear();
	i_math::plane3df p0,p1;
	
	//+X -X
	for(int k = 0;k<2;k++){
		std::vector<i_math::vector3df> & lineGrid = (bv[k])?_lineGridFore:_lineGridBack;
		for(int i = 0;i<yAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(x[k],yAxis[i],z[0]));
			lineGrid.push_back(i_math::vector3df(x[k],yAxis[i],z[1]));
		}
		for(int i = 0;i<zAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(x[k],y[0],zAxis[i]));
			lineGrid.push_back(i_math::vector3df(x[k],y[1],zAxis[i]));
		}
	}
	
	for(int k = 0;k<2;k++){
		std::vector<i_math::vector3df> & lineGrid = (bv[k+2])?_lineGridFore:_lineGridBack;
		for(int i = 0;i<xAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(xAxis[i],y[k],z[0]));
			lineGrid.push_back(i_math::vector3df(xAxis[i],y[k],z[1]));
		}
		for(int i = 0;i<zAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(x[0],y[k],zAxis[i]));
			lineGrid.push_back(i_math::vector3df(x[1],y[k],zAxis[i]));
		}
	}
	for(int k = 0;k<2;k++){
		std::vector<i_math::vector3df> & lineGrid = (bv[k+4])?_lineGridFore:_lineGridBack;
		for(int i = 0;i<xAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(xAxis[i],y[0],z[k]));
			lineGrid.push_back(i_math::vector3df(xAxis[i],y[1],z[k]));
		}
		for(int i = 0;i<yAxis.size();i++){
			lineGrid.push_back(i_math::vector3df(x[0],yAxis[i],z[k]));
			lineGrid.push_back(i_math::vector3df(x[1],yAxis[i],z[k]));
		}
	}
	
	const DWORD colSelBack = 0xa000a855;
	const DWORD colSelFore = 0xff00ff00;
	if (info.aabb.isPointInside(eyePos))
	{
		if(_lineGridFore.size())
			rp->Lines(_lineGridFore.data(),_lineGridFore.size()/2,colSelBack);
	}
	else
	{
		if(_lineGridFore.size())
			rp->Lines(_lineGridFore.data(),_lineGridFore.size()/2,colSelFore);
		
		if(_lineGridBack.size())
			rp->Lines(_lineGridBack.data(),_lineGridBack.size()/2,colSelBack);
	}
}
BOOL CGuiAgent_ElDraw::OnLButtonDown(int x,int y,DWORD flag)
{
	if(!_editor)
		return TRUE;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	assert(data);

	if(data->bOnAdd)
		return TRUE;
	
	if(!(flag&CtrlOpFlag_CtrlDown))
		data->hObjSels.clear();

	IRenderPort * rp = GetRP();
	assert(rp);

	HitProbe lineHit;
	rp->CalcHitProbe(x,y,lineHit);

	HMapObj hSel = _editor->HitTest(lineHit);
	if(hSel!=INVALID_HMAPOBJ)
		data->hObjSels.push_back(hSel);

	return TRUE;	
}





