/********************************************************************
	created:	2006/8/10   11:27
	filename: 	e:\IxEngine\Proj_GuiLib\RenderPortBase.cpp
	author:		cxi
	
	purpose:	useful functions for drawing in a IRenderPort
*********************************************************************/
#include "stdh.h"

#include <vector>
#include <string>


#include "RenderSystem/IMesh.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IFont.h"


#include "RenderPortBase.h"

#include "anim/KeySet.h"

#include "timer/profiler.h"
#include "resdata/MeshData.h"
#include "math/quaternion.h"

#include "Log/LogDump.h"
#include "shaderlib/SLDefines.h"

#include "stringparser/stringparser.h"

#include "WorldSystem/IAssetRenderer.h"

#define M_PI 3.14159265358979323846f

#pragma warning(disable:4018)
//star{
MeshHitTestResult HitTest(IMeshSnapshot * meshSnap,SpacialTester & spacialTester)
{	
	static WORD * _hitTestBuffer; //the buffer for hitTest.
	MeshHitTestResult hitResult;
	std::vector<WORD> indices;

	i_math::vector3df * posBuffer = meshSnap->GetPos();	
	WORD * indicesBuf =meshSnap->GetIndices(0);
	DWORD nNum = meshSnap->GetIBCount(0);

	DWORD ret = SpacialTester::ForceDword;

	for(int i=0;i<nNum/3;i++)
	{
		i_math::triangle3df tri;

		tri.set(posBuffer[indicesBuf[3*i]],posBuffer[indicesBuf[3*i+1]],posBuffer[indicesBuf[3*i+2]]);
		
		DWORD ret = spacialTester.Test(tri);

		if(ret)
		{
			indices.push_back(indicesBuf[3*i]);
			indices.push_back(indicesBuf[3*i+1]);
			indices.push_back(indicesBuf[3*i+2]);
		}
	}

	if(indices.size())
	{
		//SAFE_DELETE(_hitTestBuffer);
		_hitTestBuffer = new WORD[indices.size()];
		memcpy(_hitTestBuffer,indices.data(),indices.size()*sizeof(WORD));
		hitResult.testResult = SpacialTester::Intersect;
		hitResult.nNum = indices.size();
		hitResult.indices = _hitTestBuffer;
	}

	return  hitResult;
}

void DrawCirclePlane(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius,i_math::vector3df *start,float angle,DWORD col,DWORD nSeg)
{	
	const float ic = 2*M_PI/nSeg;
	i_math::vector3df  axisX ,axisY , axisZ ;
	axisY = normal;
	if(axisY.x!=0)
		axisX.set(axisY.y,-axisY.x,0);
	else
		axisX.set(0,axisY.z,-axisY.y);
	axisZ = axisX.crossProduct(normal);
	
	axisY.normalize();
	axisX.normalize();
	axisZ.normalize();

	i_math::matrix43f  matCircle;
	matCircle.set(axisX.x,axisX.y,axisX.z,
				  axisY.x,axisY.y,axisY.z,
			      axisZ.x,axisZ.y,axisZ.z,
			      center.x,center.y,center.z);

//	ProjectScaleMask(rp,matCircle);
	
	ScaleLen(rp,center,radius);
	
	float p,startAngle ,endAngle;
	startAngle = 0;
	if(start)
	{
		i_math::vector3df vecStart = *start;
		vecStart.normalize();
		float cosV,zV;
		zV = vecStart.dotProduct(axisZ);
		cosV= vecStart.dotProduct(axisX);
		startAngle = (zV>0)?acos(cosV):-acos(cosV);
	}
	endAngle = startAngle + (angle*M_PI/180);

	std::vector<i_math::vector3df> points;
	i_math::vector3df curPos;

	p=startAngle;
	while(TRUE)
	{
		curPos.x = radius*cosf(p);
		curPos.y = 0.0f;
		curPos.z = radius*sinf(p);

		matCircle.transformVect(curPos,curPos);
		points.push_back(curPos);
		
		if(angle>0)
		{
			if(p>endAngle) break;
			p +=ic;
		}
		else
		{
			if(p<endAngle) break;
			p -=ic;
		}
	}

	//triangle strip
	std::vector<i_math::vector3df> tris;

	if(points.size()>1)
	for(int i=0 ;i< points.size()-1; i++)
	{
		tris.push_back(points[i]);
		if(angle>0)
		{
			tris.push_back(points[i+1]);
			tris.push_back(center);
		}
		else
		{
			tris.push_back(center);
			tris.push_back(points[i+1]);
		}
	}
	
	ShaderState state;
	state.modeDepth = Depth_NoCmp;
	state.modeFacing = Facing_Both;
	state.modeBlend = Blend_AlphaBlend;
	rp->Triangles(tris.data(),points.size()-1,col,&state);

}
void ScaleMaskPoint(IRenderPort *rp,i_math::vector3df & point)
{
	assert(rp);
	ICamera * camera = rp->GetCamera();
	i_math::matrix43f mat;
	mat.setTranslation(point);
	ProjectScaleMask(rp,mat);
	point.set(0,0,0);
	mat.transformVect(point,point);
}
void DrawDirLine(IRenderPort *rp ,i_math::vector3df &normal,i_math::vector3df &nvec,i_math::vector3df & point,float len ,float scale,DWORD col)
{
#pragma warning(disable:4244)
	if(!rp)
		return;

	ScaleLen(rp,point,len);

	i_math::vector3df dir,end;
	dir = nvec;
	dir.setLength(len);
	end = point + dir;

	ShaderState state;
	state.modeBlend = Blend_AlphaBlend;
	state.modeDepth = Depth_NoCmp;
    rp->Line(point,end,col,&state);
	
	int x0,y0,x1,y1;
	rp->TransPos(point,x0,y0);
	rp->TransPos(end,x1,y1);

	i_math::vector3df p1,p2,p;
	p1.set(x0,y0,0);
	p2.set(x1,y1,0);
	p = p1 - p2;

	float ac;
	ac = 0.2f*M_PI;
	float sinV = sinf(ac/2);
	float cosV = cosf(ac/2);
	float dist = p1.getDistanceFrom(p2);
	
	i_math::vector3df pe1,pe2;
	
	i_math::quatf quate0;
	quate0.set(0,0,sinV,cosV);
    quate0.getMatrix().transformVect(p,pe1);
	quate0.set(0,0,-sinV,cosV);
	quate0.getMatrix().transformVect(p,pe2);

	pe1.setLength(scale*dist);
	pe2.setLength(scale*dist);
	pe1 += p2;
	pe2 += p2;
	
	rp->Line(p2.x,p2.y,pe1.x,pe1.y,col);
	rp->Line(p2.x,p2.y,pe2.x,pe2.y,col);

}
void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len)
{	
	i_math::matrix43f mat;
	i_math::vector3df vecp(1.0f,1.0f,1.0f),org(0,0,0);

	mat.setTranslation(pos);
	ProjectScaleMask(rp,mat);
	
	vecp.setLength(len);
	mat.transformVect(vecp,vecp);
	mat.transformVect(org,org);
	vecp -=org;

	len = (float)vecp.getLength();
}
float DrawCircleWire(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius ,DWORD col,DWORD nSeg=100,i_math::vector3df *cilpVec=NULL)
{
	i_math::vector3df curPos;
	std::vector<i_math::vector3df> points;
	
	const float angle =  2*M_PI/nSeg;
	const DWORD colBack = ColorAlpha(0x666666,0xff);

	i_math::vector3df axisX,axisY,axisZ;
	axisY = normal;
	if(axisY.x!=0)
		axisX.set(axisY.y,-axisY.x,0);
	else
		axisX.set(0,axisY.z,-axisY.y);
	axisZ = axisX.crossProduct(normal);
	
	axisY.normalize();
	axisX.normalize();
	axisZ.normalize();
	
	i_math::matrix43f  matCircle;
	matCircle.set(axisX.x,axisX.y,axisX.z,
				  axisY.x,axisY.y,axisY.z,
				  axisZ.x,axisZ.y,axisZ.z,
				  center.x,center.y,center.z);
		
	if(TRUE) //test
	{
		i_math::vector3df centerMod(0,0,0),comvec;
		matCircle.transformVect(centerMod,centerMod);
		comvec = centerMod - center;
		 int gooooog = 100;
	}
	
	//从屏幕空间的长度到世界空间的长度的转换
	ScaleLen(rp,center,radius);

	float p ;
	for(int i=0;i<2;i++)
	{
		p=i*angle/2;
		while(p<2*M_PI)
		{
			curPos.x = radius*cosf(p);
			curPos.y = 0.0f;
			curPos.z = radius*sinf(p);

			matCircle.transformVect(curPos,curPos);
			points.push_back(curPos);
			p+=angle;
		}
	}	
	
	i_math::plane3df plane;
	if(cilpVec)
		plane.setPlane(center,*cilpVec);

	std::vector<i_math::vector3df> lines,linesBack;
	for(int i=0;i<points.size()-1;i++)
	{
		if(cilpVec)
		{
			float dist = plane.getDistanceTo(points[i]);
			if(dist<0)
			{
              //  linesBack.push_back(points[i]);
			  //  linesBack.push_back(points[i+1]);
				continue;
			}
		}
		lines.push_back(points[i]);
		lines.push_back(points[i+1]);
	}
	
	ShaderState state;
	state.modeDepth = Depth_NoCmp;
	rp->Lines(lines.data(),nSeg,col,&state);
	
	float cr =(float) center.getDistanceFrom(points[0]);
	
	return cr;
}

float DrawTaper(IRenderPort *rp,const i_math::vector3df & dir,const i_math::vector3df &center,const float &radius,float len ,DWORD col,ShaderState * state= NULL)
{
	i_math::matrix43f matCircle ;
	i_math::vector3df axisX,axisY,axisZ;
	float lenMod = len;

	axisZ = dir;
	if(axisZ.x!=0)
		axisX.set(axisZ.y,-axisZ.x,0);
	else
		axisX.set(0,axisZ.z,-axisZ.y);
	axisY = axisX.crossProduct(axisZ);

    axisX.normalize();
	axisY.normalize();
	axisZ.normalize();

	matCircle.set(axisX.x,axisX.y,axisX.z,
				  axisY.x,axisY.y,axisY.z,
				  axisZ.x,axisZ.y,axisZ.z,
				  center.x,center.y,center.z);
	
	ICamera * camera = rp->GetCamera();
	//ProjectScaleMask(camera,matCircle);
	
	std::vector<i_math::vector3df> points;
	const float ter = 0.2f;
	for(float c = 0;c<8*M_PI+ter; c+=ter)
	{
		i_math::vector3df pos;
		pos.x = radius*cosf(c);
		pos.y = radius*sinf(c);
		pos.z = 0;
		matCircle.transformVect(pos,pos);
		points.push_back(pos);	
	}
	
	std::vector<i_math::vector3df> tris;
	i_math::vector3df vpos;
	vpos = dir;
	vpos.setLength(len);
	//ScaleMaskPoint(rp,vpos);

	lenMod = vpos.getLength();

	vpos += center;

	int nNum = points.size();
    for(int i=0;i<points.size();i++)
	{
		tris.push_back(points[i]);
		tris.push_back(center);
		tris.push_back(points[(i+1)%nNum]);
		
		tris.push_back(points[i]);
		tris.push_back(vpos);
		tris.push_back(points[(i+1)%nNum]);
	}

	rp->Triangles(tris.data(),nNum,col,state);

	return lenMod;
}

float DrawDirLineTaper(IRenderPort * rp, const i_math::vector3df & point,const i_math::vector3df & dir,float len,DWORD colLine,DWORD colTaper,ShaderState * state= NULL)
{
	i_math::matrix43f matLine ;
	i_math::vector3df axisX,axisY,axisZ;

	axisZ = dir;
	if(axisZ.x!=0)
		axisX.set(axisZ.y,-axisZ.x,0);
	else
		axisX.set(0,axisZ.z,-axisZ.y);
	axisY = axisX.crossProduct(axisZ);

	axisX.normalize();
	axisY.normalize();
	axisZ.normalize();

	matLine.set(axisX.x,axisX.y,axisX.z,
		axisY.x,axisY.y,axisY.z,
		axisZ.x,axisZ.y,axisZ.z,
		point.x,point.y,point.z);

	ICamera * camera = rp->GetCamera();
	ProjectScaleMask(rp,matLine);
	
	i_math::vector3df lineStart,lineEnd;
	lineStart = point;
	lineEnd = dir;
	lineEnd.setLength(len);
	lineEnd +=point;

	float lenTaper = DrawTaper(rp,dir,lineEnd,0.06f*len,0.3f*len,colTaper,state);
	len += lenTaper;

	rp->Line(lineStart,lineEnd,colLine,state);
	
	return lenTaper;
}
BOOL GetBoneWorldMatrix(i_math::matrix43f &matWorld,SkeletonInfo &skeleton,int idx,i_math::matrix43f * matsKey, i_math::matrix43f * matOffset=NULL)
{
	if(skeleton.size()<idx)  return FALSE;
	int i=idx;
	i_math::matrix43f matInit;
	if(!matOffset) matInit.makeIdentity();
	else matInit=*matOffset;
	while(i!=-1)
	{
		matWorld *= matsKey[i];;
		i=skeleton[i].iParent;
	}
	matWorld*=matInit;
	return TRUE;
}
BOOL GetBoneDefMatrix(i_math::matrix43f &matWorld,SkeletonInfo &skeleton,int idx,i_math::matrix43f * matOffset=NULL)
{
	
	if(skeleton.size()<=idx) return FALSE;
	int i=idx;
	i_math::matrix43f matInit;
	if(!matOffset) matInit.makeIdentity();
	else  matInit=*matOffset;
	while(i!=-1)
	{
		matWorld*=skeleton[i].xformDef.getMatrix();
		i=skeleton[i].iParent;
	}
	matWorld*=matInit;
	
	return TRUE;
}

//wbi : half lenght of bone joint width.
//col:line color
//distSJ: the distance from start point to middle of joint

void SplitMul_Dword(DWORD & dw,float scale)
{
	byte *p=(byte *)&dw;
	int r;
	for(int i=0;i<3;i++)
	{	
		r=(int)(scale*((float)p[i]));
		if(r>255) r=255;
		p[i]=r;
	}
}

#define ASSIGN_TRIANGLE(tri,p0,p1,p2,twist)  \
	tri[r]=p0;							\
	tri[r+1]=p1;						\
	tri[r+2]=p2;						\
	v1=p2-p0;							\
	v2=p1-p2;							\
	v1.normalize();						\
	v2.normalize();						\
	nor=v1.crossProduct(v2);			\
	nor.normalize();					\
	shines=abs(nor.dotProduct(eye));	\
	color[r]=col;						\
	SplitMul_Dword(color[r],shines);    \
	color[r+2]=color[r+1]=color[r];		\
	if(twist)							\
	{									\
		SplitMul_Dword(color[r],1.2f);	\
	}									\
	r+=3;	

BOOL DrawBone(IRenderPort *rp,i_math::matrix43f matParent,i_math::matrix43f matLocal,i_math::vector3df start,i_math::vector3df end,/*out*/i_math::vector3df *tri,DWORD * color,float scaleSJ=0.2f,DWORD col=0xff00ff00,BOOL bDraw=TRUE,BOOL bEnd=FALSE)
{
	i_math::vector3df j[4];
	float distSJ,wbi;
	float len=(float)start.getDistanceFrom(end);
	distSJ=(float)(scaleSJ*len);
	wbi=0.3f*distSJ;
	i_math::vector3df n0, n1;
	i_math::vector3df r0,r1,pos(0,0,0);	
	matLocal.transformVect(pos,pos);
	if(!bEnd)
		n1=n0=pos;
	else
	{
		n1=n0=end-start;
	}
	n0.setLength(len);
	n1.setLength(distSJ);
	r0.set(0,0,wbi);
	j[0]=n1-r0;
	j[2]=n1+r0;

	r1=n1.crossProduct(r0);
	r1.setLength(wbi);
	j[3]=n1+r1;
	j[1]=n1-r1;
	
	for(int i=0;i<4;i++)
	{
		if(!bEnd)
			matParent.transformVect(j[i],j[i]);
		else
		{
			i_math::matrix43f matoffset;
			matoffset.addTranslation(start.x,start.y,start.z);
			matoffset.transformVect(j[i],j[i]);
		}
	}
	i_math::vector3df v1,v2,nor,eye;
	ICamera * camer=rp->GetCamera();
	if(!camer) return FALSE;
	camer->GetEyeDir(eye);
	float shines;
	int r=0;

	ASSIGN_TRIANGLE(tri,end,j[3],j[0],FALSE)
	ASSIGN_TRIANGLE(tri,end,j[0],j[1],FALSE)
	ASSIGN_TRIANGLE(tri,end,j[1],j[2],FALSE)
	ASSIGN_TRIANGLE(tri,end,j[2],j[3],FALSE)

	SplitMul_Dword(col,1.1f);
	ASSIGN_TRIANGLE(tri,start,j[0],j[3],FALSE)
	ASSIGN_TRIANGLE(tri,start,j[1],j[0],FALSE)
	ASSIGN_TRIANGLE(tri,start,j[2],j[1],FALSE)
	ASSIGN_TRIANGLE(tri,start,j[3],j[2],FALSE)

	if(rp&&bDraw)
	rp->Triangles(tri,8,color);
	
	return TRUE;
}

// 抵消，模型因为Project变换导致的引起的放缩。
// 有：向量 a。matProj0: 透视变换 matProj1:正交变换
//  A = a*matTranf*matWorld*matView*matProj0 
//	B = a*matWorld*matView*matProj1
// 使: A == B
BOOL ProjectScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf)
{
	i_math::matrix43f matScale;
	BOOL ret = GetProjScaleMask(rp,matTranf,matScale);
	
	if(!ret)
		return FALSE;
	matTranf=matScale*matTranf;
	return TRUE;
}


BOOL GetProjScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf,i_math::matrix43f &matScale)
{
	ICamera * cam = rp->GetCamera();
	if(!cam) 
		return FALSE;

	i_math::recti rc;
	rp->GetRect(rc);
	return cam->GetProjScaleMask(rc,*matTranf.getTranslationP(),matScale);
}

void CalShinesColor(i_math::vector3df &eyePoint,i_math::vector3df &point,i_math::vector3df &normal,DWORD &col)
{
	i_math::vector3df vecp;
	float shines;
	vecp = eyePoint - point;
	vecp.normalize();
	shines = vecp.dotProduct(normal);

	DWORD dw = col&0xff000000;
	col = col&0x00ffffff;
	SplitMul_Dword(col,shines);
	col +=dw;
}

#define FILL_TRI(i0,i1,i2) \
{\
	n0 = points[i1] - points[i0];				 \
	n0.normalize();								\
	n1 = points[i2] - points[i0];				 \
	n1.normalize();								\
	normal = n0.crossProduct(n1);				 \
	normal.normalize();							 \
	plane.setPlane(points[i0],normal);				 \
	relation = plane.classifyPointRelation(eyePoint);	 \
	if(i_math::ISREL3D_FRONT==relation){\
		tcol = col;			\
		CalShinesColor(eyePoint,points[i0],normal,tcol);	\
		cols[3*idx] = tcol;						\
		tcol = col;			\
		CalShinesColor(eyePoint,points[i1],normal,tcol);	\
		cols[3*idx+1] = tcol;						\
		tcol = col;			\
		CalShinesColor(eyePoint,points[i2],normal,tcol);	\
		cols[3*idx+2] = tcol;						\
		tri[3*idx] = points[i0];				\
		tri[3*idx+1] = points[i1];				\
		tri[3*idx+2] = points[i2];				\
		idx++;									\
	}\
}

BOOL DrawOBBModel(IRenderPort *rp,i_math::matrix43f &mat,i_math::vector3df &eyePoint,i_math::aabbox3df aabb,DWORD col)
{
	i_math::vector3df points[8];
	i_math::vector3df tri[36];
	DWORD  cols[36];
	int idx =0;
		//					/4--------/0
		//                 /  |      / |
		//                /   |     /  |
		//                6---------2  |
		//                |   5- - -| -1
		//                |  /      |  /
		//                |/        | /
		//                7---------3/ 
	
	aabb.getCorners(points);
	
	for(int i=0;i<8;i++)
		mat.transformVect(points[i],points[i]);

	i_math::vector3df normal,n0,n1;
	i_math::plane3df plane;
	DWORD relation;

	int t=0;
	DWORD tcol;
	FILL_TRI(6,2,0);
	FILL_TRI(6,0,4);
	
	FILL_TRI(4,0,5);
	FILL_TRI(5,0,1);
	FILL_TRI(4,5,6);
	FILL_TRI(6,5,7);
	FILL_TRI(7,2,6);
	FILL_TRI(2,7,3);
	FILL_TRI(0,2,1);
	FILL_TRI(1,2,3);

	FILL_TRI(3,7,1);
	FILL_TRI(1,7,5);
	
	rp->Triangles(tri,idx,cols);

	return FALSE;
}

#define FILL_LINE(i0,i1,len) \
{\
	dir = corners[i1] - corners[i0]; \
	dir.setLength(len);				\
	lines[idx++] = corners[i0];		\
	lines[idx++] = corners[i0]+dir;	\
}
BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df &aabb,DWORD col)
{

	i_math::vector3df corners[8];
	float lenW,lenH,lenT;
	i_math::vector3df lines[48],dir;
	int idx =0;
	
	aabb.getCorners(corners);

	for(int i=0;i<8;i++)
		mat.transformVect(corners[i],corners[i]);
	
	const float ratio = 0.33f;
	lenW = ratio*corners[0].getDistanceFrom(corners[4]);
	lenH = ratio*corners[0].getDistanceFrom(corners[2]);
	lenT = ratio*corners[0].getDistanceFrom(corners[1]);
	
	FILL_LINE(0,2,lenH);
	FILL_LINE(0,1,lenT);
	FILL_LINE(0,4,lenW);

	FILL_LINE(1,0,lenT);
	FILL_LINE(1,3,lenH);
	FILL_LINE(1,5,lenW);

	FILL_LINE(2,0,lenH);
	FILL_LINE(2,3,lenT);
	FILL_LINE(2,6,lenW);

	FILL_LINE(3,1,lenH);
	FILL_LINE(3,2,lenT);
	FILL_LINE(3,7,lenW);

	FILL_LINE(4,0,lenW);
	FILL_LINE(4,5,lenT);
	FILL_LINE(4,6,lenH);

	FILL_LINE(5,1,lenW);
	FILL_LINE(5,7,lenH);
	FILL_LINE(5,4,lenT);
	
	FILL_LINE(6,4,lenH);
	FILL_LINE(6,2,lenW);
	FILL_LINE(6,7,lenT);

	FILL_LINE(7,6,lenT);
	FILL_LINE(7,3,lenW);
	FILL_LINE(7,5,lenH);

	rp->Lines(lines,24,col);

	return TRUE;
}

//! center : center position of the sphere.
//! nstep   : angle interval  in axis direction
//! nSeg   : every circle be draw with nSeg line strip, more larger more smooth.
void DrawSphere(IRenderPort * rp,i_math::vector3df &center,float radius,DWORD col,float nStep=10,int nSeg =20)
{
	if(!rp) 
		return;

	const float im = M_PI/nStep ;
	const float arc =2*M_PI/(nSeg+1);
	std::vector<i_math::vector3df * > circles;

	for(float z =im ; z < M_PI ; z+=im )
	{
		i_math::vector3df * circle = new i_math::vector3df[nSeg];
		float p =0;
		for(int i = 0; i<nSeg ; i++)
		{
			circle[i].z= radius*cosf(z)+center.z;
			circle[i].x = radius*sinf(z)*sinf(p)+center.x;
			circle[i].y = radius*sinf(z)*cosf(p)+center.y;
			p += arc;
		}
		circles.push_back(circle);
	}

	std::vector<i_math::vector3df> lines;
	int nLine =0;
	for(int i=0 ;i<circles.size();i++)
	{
		for(int k=0;k<nSeg;k++)
		{
			lines.push_back(circles[i][k]);
			lines.push_back(circles[i][(k+1)%nSeg]);
			nLine++;
		}

		std::vector<int> figure;
		float ev = (float)nSeg/(float)nStep;
		for(float p=0;p<=(nSeg-1);p+=ev)
			figure.push_back((int)p);

		i_math::vector3df upCenter ,lowCenter;
		upCenter.set(center.x,center.y,center.z+radius);
		lowCenter.set(center.x,center.y,center.z-radius);

		for(int v=0;v<figure.size();v++)
		{
			int index = figure[v];
			lines.push_back(upCenter);
			lines.push_back(circles[0][index]);
			lines.push_back(lowCenter);
			lines.push_back(circles[circles.size()-1][index]);
			nLine += 2;

			if(circles.size()>1)
				for(int j=0;j<circles.size()-1;j++)
				{
					lines.push_back(circles[j][figure[v]]);
					lines.push_back(circles[j+1][figure[v]]);
					lines.push_back(circles[j][figure[v]]);
					lines.push_back(circles[j+1][figure[(v+1)%figure.size()]]);
					nLine += 2;
				}
		}
	}

	for(int i=0 ;i<circles.size();i++)
		delete circles[i];

	rp->Lines(lines.data(),nLine,col);
}
// * FALSE ;if the eyepoint is in the back face else return TRUE.
BOOL CalTriColors(i_math::vector3df * tris,DWORD col,i_math::vector3df & eyePoint,/*out*/DWORD *cols)
{
	i_math::vector3df normal,n0,n1,e0,e1,e2;

	n0 = tris[1] - tris[0];
	n1 = tris[2] - tris[0];
	normal = n0.crossProduct(n1);
	normal.normalize();
	
	CalShinesColor(eyePoint,tris[0],normal,cols[0]);
	CalShinesColor(eyePoint,tris[1],normal,cols[1]);
	CalShinesColor(eyePoint,tris[2],normal,cols[2]);

	float d = - tris[0].dotProduct(normal);
	if((d + eyePoint.dotProduct(normal))>0)
		return TRUE;
	else
		return FALSE;
}

void DrawSphereModel(IRenderPort * rp,float radius,i_math::matrix43f * mat,DWORD col,i_math::vector3df &eyePoint,float nStep=10,int nSeg =20)
{
	if(!rp) 
		return;
	
	const float im = M_PI/nStep ;
	const float arc =2*M_PI/(nSeg+1);
	std::vector<i_math::vector3df * > circles;
	i_math::vector3df  center(0,0,0);
	i_math::matrix43f matTrans;
	if(mat)
		matTrans = *mat;
	
	for(float z =im ; z < M_PI ; z+=im )
	{
		i_math::vector3df * circle = new i_math::vector3df[nSeg];
		float p =0;
		for(int i = 0; i<nSeg ; i++)
		{
			circle[i].z= radius*cosf(z);
			circle[i].x = radius*sinf(z)*sinf(p);
			circle[i].y = radius*sinf(z)*cosf(p);
			matTrans.transformVect(circle[i],circle[i]);
			p += arc;
		}
		circles.push_back(circle);
	}

	i_math::vector3df upCenter ,lowCenter;
	upCenter.set(center.x,center.y,center.z+radius);
	lowCenter.set(center.x,center.y,center.z-radius);
	
	matTrans.transformVect(upCenter,upCenter);
	matTrans.transformVect(lowCenter,lowCenter);
	
	int nSize = circles.size();
	std::vector<i_math::vector3df> tris;
	std::vector<DWORD> cols;
	
//////////////////////////////////////////////////////////////////////////
	i_math::vector3df temp[3];
	DWORD colstemp[3];
	BOOL ret = FALSE;
	#define SPFILL_TRI(t0,t1,t2){\
		temp[0] = t0;		\
		temp[1] = t1;		\
		temp[2] = t2;		\
		colstemp[0]= col;	\
		colstemp[1]= col;	\
		colstemp[2]= col;	\
		ret =CalTriColors(temp,col,eyePoint,colstemp);		\
		if(ret){\
			tris.push_back(temp[0]);			\
			tris.push_back(temp[1]);			\
			tris.push_back(temp[2]);			\
			cols.push_back(colstemp[0]);		\
			cols.push_back(colstemp[1]);		\
			cols.push_back(colstemp[2]);		\
		}\
	}
//////////////////////////////////////////////////////////////////////////

	for(int c=0;c<nSeg;c++)
	{
		SPFILL_TRI(circles[0][(c+1)%nSeg],circles[0][c],upCenter);
		SPFILL_TRI(circles[0][c],circles[0][(c+1)%nSeg],circles[1][(c+1)%nSeg]);

		SPFILL_TRI(circles[nSize-1][c],circles[nSize-1][(c+1)%nSeg],lowCenter);
		SPFILL_TRI(circles[nSize-1][c],circles[nSize-2][c],circles[nSize-1][(c+1)%nSeg]);
	}

	for(int i= 1;i<circles.size()-1;i++)
	{
		for(int c=0;c<nSeg;c++)
		{
			SPFILL_TRI(circles[i][(c+1)%nSeg],circles[i][c],circles[i-1][c]);
			SPFILL_TRI(circles[i][c],circles[i][(c+1)%nSeg],circles[i+1][(c+1)%nSeg]);
		}
	}
	for(int i =0 ;i<circles.size();i++)
		SAFE_DELETE(circles[i]);

	rp->Triangles(tris.data(),tris.size()/3,cols.data());
}

void DrawQuad(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD col,float & width/* in out*/,float & height /* in out*/,ShaderState * state= NULL)
{
	i_math::vector3df axisX,axisY,axisZ;
	axisZ = noraml;
	axisX = vecStart;
	axisY = axisX.crossProduct(axisZ);
	
	axisX.normalize();
	axisY.normalize();
	axisZ.normalize();
	
	/*
		0----1  ->x
		|	 |
	  y	|	 |
		2----3
	*/
	i_math::vector3df v0,v1,v2,v3;

	v0.set(0.0f,0.0f,0.0f);
	v1.set(width,0.0f,0.0f);
	v2.set(0.0f,height,0.0f);
	v3.set(width,height,0.0f);
	
	i_math::matrix43f matQuad;
	matQuad.set(axisX.x,axisX.y,axisX.z,
				axisY.x,axisY.y,axisY.z,
				axisZ.x,axisZ.y,axisZ.z,
				pos.x,pos.y,pos.z);

	//ProjectScaleMask(rp->GetCamera(),matQuad);

	matQuad.transformVect(v0,v0);
	matQuad.transformVect(v1,v1);
	matQuad.transformVect(v2,v2);
	matQuad.transformVect(v3,v3);

	i_math::vector3df tris[6];
	tris[0] = v0;
	tris[1] = v2;
	tris[2] = v3;
	tris[3] = v3;
	tris[4] = v1;
	tris[5] = v0;
	
	ShaderState stateDraw;
	if(state)
		stateDraw = *state;

	stateDraw.modeFacing = Facing_Both;
	stateDraw.modeBlend = Blend_AlphaBlend;

	rp->Triangles(tris,2,col,&stateDraw);
	
	width  = v0.getDistanceFrom(v1);
	height = v0.getDistanceFrom(v2);
}
void DrawQuadWire(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD *cols/*in out*/,float & width/* in out*/,float & height /* in out*/,ShaderState * state= NULL)
{
	i_math::vector3df axisX,axisY,axisZ;
	axisZ = noraml;
	axisX = vecStart;
	
	axisY = axisX.crossProduct(axisZ);
	
	axisX.normalize();
	axisY.normalize();
	axisZ.normalize();
	
	/*
	0----1  ->x
	|	 |
	y	 |	 
	2----3
	*/
	i_math::vector3df v0,v1,v2,v3;

	v0.set(0.0f,0.0f,0.0f);
	v1.set(width,0.0f,0.0f);
	v2.set(0.0f,height,0.0f);
	v3.set(width,height,0.0f);

	i_math::matrix43f matQuad;
	matQuad.set(axisX.x,axisX.y,axisX.z,
				axisY.x,axisY.y,axisY.z,
				axisZ.x,axisZ.y,axisZ.z,
				pos.x,pos.y,pos.z);
	matQuad.transformVect(v0,v0);
	matQuad.transformVect(v1,v1);
	matQuad.transformVect(v2,v2);
	matQuad.transformVect(v3,v3);
	
	i_math::vector3df * lines = NULL;
	lines = new i_math::vector3df[8];
	lines[0] = v0;
	lines[1] = v1;
	lines[2] = v2;
	lines[3] = v3;

	lines[4] = v0;
	lines[5] = v2;
	lines[6] = v1;
	lines[7] = v3;
	
	DWORD *colsv = new DWORD[8];
	for(int i =0;i<8;i++)
		colsv[i] = cols[i/2];
		
	rp->Lines(lines,4,colsv,state);
    
	SAFE_DELETE(lines);
	SAFE_DELETE(colsv);

	width  = v0.getDistanceFrom(v1);
	height = v0.getDistanceFrom(v2);	
}
////////////////////////////////////////////////////////////////////////// star }
BOOL DrawAABBox(IRenderPort *rp,i_math::aabbox3df aabb,i_math::matrix43f * mat=NULL,DWORD col=0xffffffff)
{
	if(!rp) return FALSE;
	i_math::matrix43f matTrans;
	if(!mat) matTrans.makeIdentity();
	else  matTrans=*mat;
	i_math::vector3df pos(0,0,0);
	matTrans.transformVect(pos,pos);
	
	aabb.MinEdge+=pos;
	aabb.MaxEdge+=pos;
	i_math::line3df edges[12];
	aabb.getEdges(edges);
	for(int i=0;i<12;i++)
	rp->Line(edges[i].start,edges[i].end,col);

	return TRUE;
}
BOOL DrawDefSkeleton(IRenderPort *rp,ISkeleton *skeleton,matrix43f *xfm,DWORD col)
{
	if (!skeleton)
		return FALSE;
	matrix43f matIdentity;
	if (!xfm)
		xfm=&matIdentity;
	std::vector<matrix43f>vecTemp;
	vecTemp.resize(skeleton->GetBoneCount());


	int i;
	for (i=0;i<vecTemp.size();i++)
	{
		xformf t;
		skeleton->GetBoneDefXform(i,t);
		vecTemp[i]=t.getMatrix();
		int iParent;
		skeleton->GetBoneParent(i,iParent);

		if (iParent==-1)
			vecTemp[i]*=(*xfm);
		else
			vecTemp[i]*=vecTemp[iParent];
	}

	for (i=0;i<vecTemp.size();i++)
	{
		int iParent;
		skeleton->GetBoneParent(i,iParent);

		if(iParent==-1)
			continue;

		vector3df v1(0,0,0),v2(0,0,0);

		vecTemp[i].transformVect(v1,v1);
		vecTemp[iParent].transformVect(v2,v2);

		if (FALSE==rp->Line(v1,v2,col))
			return FALSE;
	}

	return TRUE;
}



BOOL DrawMeshSnapshot(IRenderPort *rp,IMesh *mesh,DWORD col)
{
	return FALSE;
//	IMeshSnapshot *snapshot;
//	snapshot=mesh->GetSnapshot();
//
//	if (!snapshot)
//		return FALSE;
//
//	BOOL bOk=TRUE;
//	DrawMeshArg dmg;
//	DWORD nSurf;
//	nSurf=snapshot->GetSurfCount();
//	for (int i=0;i<nSurf;i++)
//	{
//		DWORD count;
//		vector3df *pos;
//		dmg.SetSurf(i);
//		pos=(vector3df *)snapshot->GetPos(NULL,dmg,count);
//
//		if (!rp->Points(pos,count,col))
//			bOk=FALSE;
//	}
//	return bOk;
}



BOOL DrawGrid(IRenderPort *rp,DWORD d,DWORD gap)
{
	int i;
	int s,e;
	e=(d/2)*gap;
	s=-e;
	std::vector<vector3df>lines;
	std::vector<vector3df>lines2;
	std::vector<vector3df>lines3;
	vector3df v1,v2;
	for (i=s;i<=e;i+=gap)
	{
		v1.set((f32)i,0.0f,(f32)s);
		v2.set((f32)i,0.0f,(f32)e);
		if (i==0)
		{
			lines2.push_back(v1);
			lines2.push_back(v2);
		}
		else
		{
			lines.push_back(v1);
			lines.push_back(v2);
		}
		v1.set((f32)s,0,(f32)i);
		v2.set((f32)e,0,(f32)i);
		if (i==0)
		{
			lines3.push_back(v1);
			lines3.push_back(v2);
		}
		else
		{
			lines.push_back(v1);
			lines.push_back(v2);
		}
	}

	rp->Lines(lines.data(),lines.size()/2,ColorAlpha(0x7f7f7f,0xff));
	rp->Lines(lines3.data(),lines2.size()/2,ColorAlpha(0xff7f00,0xff));
	rp->Lines(lines2.data(),lines3.size()/2,ColorAlpha(0x007fff,0xff));

	if (TRUE)
	{
		int x,y;
		DrawFontArg arg;
		i_math::vector3df v;
		v.set((float)d/2,0,0);
		rp->TransPos(v,x,y);
		arg.SetLocation(x,y);
		rp->DrawText("{C:255,128,0}{S:16}{A:255}X",arg);

		v.set(0,0,(float)d/2);
		rp->TransPos(v,x,y);
		arg.SetLocation(x,y);
		rp->DrawText("{C:0,128,255}{S:16}{A:255}Z",arg);
	}

	return TRUE;
}


GuiLib_Api BOOL DrawProfile(IRenderPort *rp,ProfilerMgr *mgr,i_math::pos2di &pt)
{
	if (!mgr)
		return FALSE;

	const char *name=mgr->GetDump();
	DrawFontArg arg;
	i_math::size2di sz;
	rp->CalcDrawText(name,arg,sz);
	i_math::recti rc;
	rc.set(pt,sz);
	rc.inflate(0,0,16,16);

	rp->FillRect(rc,ColorAlpha(0x0,0x7f));
	rp->FrameRect(rc,ColorAlpha(0xffffff,0xff));

	arg.SetLocation(pt.x+8,pt.y+8);
	rp->DrawText(name,arg);

	return TRUE;
}

BOOL DrawAdrStats(IRenderPort *rp,AdrStats *stats,i_math::pos2di &pt)
{
	std::string s;
	for (int i=0;i<stats->nPasses;i++)
	{
		PassStats *p=&stats->passes[i];
		AppendFmtString(s,"%s--%d个ratoms,%d个occs,裁剪掉:%d(incl)+%d(excl),地表DP:%d次\n",p->name,p->nRatoms,p->nOccs,p->nCullInc,p->nCullExc,p->nTrrnDP);
	}
	AppendFmtString(s,"Def Batched:%d",stats->nDefBatched);

	DrawFontArg arg;
	i_math::size2di sz;
	rp->CalcDrawText(s.c_str(),arg,sz);
	i_math::recti rc;
	rc.set(pt,sz);
	rc.inflate(0,0,16,16);

	rp->FillRect(rc,ColorAlpha(0x0,0x7f));
	rp->FrameRect(rc,ColorAlpha(0xffffff,0xff));

	arg.SetLocation(pt.x+8,pt.y+8);
	rp->DrawText(s.c_str(),arg);

	return TRUE;

}

GuiLib_Api BOOL DrawCvxVolume(IRenderPort *rp,ICvxVolume * pVol)
{
	assert(rp);
	assert(pVol);
	
	i_math::vector3df * corners;
	DWORD  nCorner = 0;
	pVol->GetCorners(corners,nCorner);
   
	std::vector<i_math::vector3df>  lines;

#define DrawCvxVolume_FillLine(n0,n1,n2,n3)\
	{\
		lines.push_back(corners[n0]);		\
		lines.push_back(corners[n1]);		\
		lines.push_back(corners[n1]);		\
		lines.push_back(corners[n2]);		\
		lines.push_back(corners[n2]);		\
		lines.push_back(corners[n3]);		\
		lines.push_back(corners[n3]);		\
		lines.push_back(corners[n0]);		\
	}
	
	DrawCvxVolume_FillLine(0,1,2,3);
	DWORD col = 0xff0000ff;
	rp->Lines(&(lines[0]),lines.size()/2,col);
	return TRUE;
}

GuiLib_Api BOOL DrawCapsule(IRenderPort * rp,i_math::capsulef & capcol)
{
	if(!rp)
		return FALSE;

	i_math::vector3df axisX,axisY,axisZ;
	i_math::capsulef cap = capcol;
	
	axisY = cap.end - cap.start;
	axisY.normalize();
	axisY.findBestAxis(axisX,axisZ);
	
	i_math::matrix43f mat;
	mat.set(axisX.x,axisX.y,axisX.z,
			axisY.x,axisY.y,axisY.z,
			axisZ.x,axisZ.y,axisZ.z,
			cap.start.x,cap.start.y,cap.start.z);

	std::vector<i_math::vector3df> points;
	float step = M_PI/12.0f;
	
	for(float r = 0;r<2*M_PI;r+=step)
	{
		i_math::vector3df point;
		point.x = cap.radius * cosf(r);
		point.z = cap.radius * sinf(r);
		points.push_back(point);
	}
	
	float capheight = (float)cap.start.getDistanceFrom(cap.end);
	step =  capheight/5.0f;
	
	std::vector<i_math::vector3df> lines;	
	for(int i = 0;i<points.size();i++)
	{
		i_math::vector3df ls(points[i].x,0.0f,points[i].z);
		i_math::vector3df le(points[i].x,capheight,points[i].z);

		lines.push_back(ls);
		lines.push_back(le);
	}

	points.push_back(points[0]);
	float height = 0.0f;
	
	for(float i = 0;i<=5;i++)
	{
		for(int r = 0;r<points.size()-1;r++)
		{
			i_math::vector3df ls(points[r].x,height,points[r].z);
			i_math::vector3df le(points[r+1].x,height,points[r+1].z);
			lines.push_back(ls);
			lines.push_back(le);
		}
		height += step;
	}
	
	for(int i = 0;i<lines.size();i++)
		mat.transformVect(lines[i],lines[i]);
		
	rp->Lines(lines.data(),lines.size()/2,0xff00aaaa);
	
	
	DrawSphereModel(rp,cap.radius,&mat,0x8800aaaa,cap.start);
	
	mat.addTranslation(cap.end-cap.start);
	DrawSphereModel(rp,cap.radius,&mat,0x8800aaaa,cap.end);
	
	return TRUE;
}

void DrawSamples(IRenderPort * rp, i_math::vector3df * fPos,IMesh * mesh,IMtrl * mtrl,DWORD nCount)
{
	if(!rp||!mesh||!mtrl||!fPos)
		return;

	IRenderer * render = rp->ObtainRenderer();
	IShader * shader = render->BeginRaw(mtrl);
	if(shader){
		i_math::matrix43f mat;
		i_math::vector3df colShdw;
		for(int i = 0;i<nCount;i++){
			DrawMeshArg arg;
			arg.iLod = 0;
			arg.fillmode = D3DFILL_SOLID;
			arg.fFrame = 0;
			float w = 1.0f;
			colShdw.set(w,w,w);
			shader->SetEP(EPG_fx3_01,fPos[i]);
			shader->SetEP(EPG_fx3_02,colShdw);
			mesh->Draw(shader,mat,arg);		
		}
		render->EndRaw(shader);
	}
}

void DrawLineSamples(IRenderPort * rp,i_math::vector3df * fPos,i_math::vector3df * fNormal,float len,DWORD nCount,DWORD col = 0xffffffff)
{
	if(!rp||!fPos)
		return;

	std::vector<i_math::vector3df> lines(nCount*2);
	
	for(int i = 0;i<nCount;i++){
		i_math::vector3df nor(0,0,1.0f);
		if(fNormal)
			nor = fNormal[i];
		nor.setLength(len);
		lines[2*i + 0] = fPos[i];
		lines[2*i + 1] = fPos[i] + nor;
	}

	rp->Lines(&(lines[0]),nCount,col);
}







