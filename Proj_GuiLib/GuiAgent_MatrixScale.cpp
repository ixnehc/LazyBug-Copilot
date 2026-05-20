#include "stdh.h"
#include ".\GuiAgent_MatrixScale.h"
#include <assert.h>

extern float DrawDirLineTaper(IRenderPort * rp, const i_math::vector3df & point,const i_math::vector3df & dir,float len,DWORD colLine,DWORD colTaper,ShaderState * state = NULL);
extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);

CAgent_MatrixScale::CAgent_MatrixScale(void)
{
	_flag = 0;
}

CAgent_MatrixScale::~CAgent_MatrixScale(void)
{
}

BOOL CAgent_MatrixScale::OnDraw()
{
	IRenderPort *rp = GetRP();
//	rp->ClearBuffer(ClearBuffer_Depth);
	_DrawAxisWire();
	_DrawTriAreaWire();
	_DrawActiveArea();

	return FALSE;
}
BOOL CAgent_MatrixScale::OnMouseMove(int x,int y,DWORD flag)
{	
	PreEdit(EditMode_Scale);
	if(_bInDrag)
		OnDrag(x,y,flag);
	else
	{
		_HitTest(x,y);
		if(_flag!=0)
			_bSel = TRUE;
		else
			_bSel = FALSE;
	}
	return !_flag;
}
void CAgent_MatrixScale::OnEndDrag(int x,int y,DWORD flag)
{
	_matScaleEdit.makeIdentity();
	_funEndEditListener(this);
}
void CAgent_MatrixScale::OnDrag(int x,int y,DWORD flag)
{
	i_math::matrix43f matInverse;
	matInverse = _matValue*_data.matParent;
	matInverse.makeInverse();
	
	i_math::vector3df v0(0,0,0),vx,vy,vz;
	matInverse.transformVect(v0,v0);
#define CAgent_MatrixScale_OnDrag_NORMALIZE(n,t0)\
	{\
		matInverse.transformVect(n,t0);\
		t0 -= v0;			\
		t0.normalize();		\
	}
	CAgent_MatrixScale_OnDrag_NORMALIZE(_nx,vx);
	CAgent_MatrixScale_OnDrag_NORMALIZE(_ny,vy);
	CAgent_MatrixScale_OnDrag_NORMALIZE(_nz,vz);

	i_math::matrix43f matSpace,matSpaceInverse,matScale;
	matSpace.set(vx.x,vx.y,vx.z,
				 vy.x,vy.y,vy.z,
				 vz.x,vz.y,vz.z,
				 0,0,0);
	matSpace.getInverse(&matSpaceInverse);
	
	float scaleValue = _CalCurrentScaleValue(x,y);

	switch(_flag)
	{
	case Active_X:
		{
			matScale.setScale(scaleValue,1.0f,1.0f);
			break;
		}
	case Active_Y:
		{
			matScale.setScale(1.0f,scaleValue,1.0f);
			break;
		}
	case Active_Z:
		{
			matScale.setScale(1.0f,1.0f,scaleValue);
			break;
		}

	default:
		break;
	}
	
	if(TRUE)
	{
		i_math::vector3df t[3];
		i_math::matrix43f matTest;
		t[0].set(1.0f,0.0f,0.0f);
		t[1].set(0.0f,1.0f,0.0f);
		t[2].set(0.0f,0.0f,1.0f);
		matTest = matSpace*matScale*matSpaceInverse;
		matTest.transformVect(t[0],t[0]);
		matTest.transformVect(t[1],t[1]);
		matTest.transformVect(t[2],t[2]);

	}
	_matScaleEdit = matScale;
	*_data.matrix = matSpace*matScale*matSpaceInverse*_matInit;
}
float CAgent_MatrixScale::_CalCurrentScaleValue(int x,int y)
{
	i_math::vector2df curNs;
	i_math::vector2df curOff;

	curOff.set((float)(x - _ix),(float)(y - _iy));
	switch(_flag)
	{
	case Active_X:
		curNs = _sx;
		break;
	case Active_Y:
		curNs = _sy;
		break;
	case Active_Z:
		curNs = _sz;
		break;
	case Active_XY:
		curNs = _sxy;
		break;
	case Active_YZ:
		curNs = _syz;
		break;
	case Active_ZX:
		curNs = _szx;
		break;
	case Active_XYZ:
		{
			curNs = _sxy;
			float dot = (float)_sxy.dotProduct(curOff);
			float tmp = (float)_syz.dotProduct(curOff);
			if(dot< tmp)
			{
				dot = tmp;
				curNs = _syz;
			}
			tmp = (float)_szx.dotProduct(curOff);
			if(dot< tmp)
			{
				dot = tmp;
				curNs = _szx;
			}
			break;
		}
	default:
		break;
	}
	
	float idot =(float)(curNs.dotProduct(curOff)); 
	return 1.0f+idot*_arg.speed;
}
BOOL CAgent_MatrixScale::OnLButtonDown(int x,int y,DWORD flag)
{
	//Added By Chenxi
	if (flag)
		return TRUE;
	//

	PreEdit(EditMode_Scale);
	
	if(!flag)
	{
		_bInDrag = TRUE;
		OnBeginDrag(x,y,flag);
		OccupyFocus(OpType_Mouse);
	}
	
	_CalScreenNormal();
	
	_ix = x;
	_iy = y;
	
	_matInit = *_data.matrix;

	return TRUE;
}
BOOL CAgent_MatrixScale::OnLButtonUp(int x,int y,DWORD flag)
{
	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);
	return FALSE;
}
BOOL CAgent_MatrixScale::_HitTestArea(int x,int y)
{
	i_math::vector3df quad[4];
	float zpos[3];
	DWORD flags[3];
	flags[0] = flags[1] = flags[2] = 0;
	
	i_math::line3df line;
	IRenderPort * rp = GetRP();
	assert(rp);
	
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	line.setLine(probe.start,probe.end);

	BOOL ret = FALSE;
	int num = 0;
#define HitTestArea_TESTQUAD(t0,t1,t2,t3,flag)\
{	\
	quad[0] = t0;		\
	quad[1] = t1;		\
	quad[2] = t2;		\
	quad[3] = t3;		\
	ret = _HitTestQuad(line,quad,zpos[num]);	\
	if(ret){ \
		flags[num++] = flag;\
	}\
}

	HitTestArea_TESTQUAD(_x0,_y0,_y1,_x1,Active_XY);
	HitTestArea_TESTQUAD(_y0,_z0,_z1,_y1,Active_YZ);
	HitTestArea_TESTQUAD(_z0,_x0,_x1,_z1,Active_ZX);
	
	if(num==0) 
		return FALSE;
	
	i_math::matrix44f matView0;
	i_math::matrix43f matView;
	rp->GetCamera()->GetView(matView0);
	mat43from44(matView,matView0);

	DWORD flagActive;
	flagActive = flags[0];
	float z = zpos[0];
	if(num>1)
	for(int i=1;i<num;i++)
	{
		if(zpos[i]<z)
		{
			z = zpos[i];
			flagActive = flags[i];
		}
	}
	_flag |=flagActive;

	return TRUE;
}

void  CAgent_MatrixScale::_CalScreenNormal()
{
	i_math::vector3df mid;
	IRenderPort *rp = GetRP();
	assert(rp);
	
	int x0,y0,x1,y1;
	rp->TransPos(_center,x0,y0);
#define CalScreenNormal_FILL_P(t0,t1,sn) \
	{\
		mid = t0+t1;				\
		mid -= _center;				\
		rp->TransPos(mid,x1,y1);	\
		sn.set((float)(x1-x0),(float)(y1-y0));	\
		sn.normalize();				\
	}

	CalScreenNormal_FILL_P(_x0,_y0,_sxy);
	CalScreenNormal_FILL_P(_y0,_z0,_syz);
	CalScreenNormal_FILL_P(_z0,_x0,_szx);

#define CalScreenNormal_FILL_A(t0,sa)\
	{\
		rp->TransPos(t0,x1,y1);		\
		sa.set((float)(x1-x0),(float)(y1-y0));	\
		sa.normalize();				\
	}

	CalScreenNormal_FILL_A(_x0,_sx);
	CalScreenNormal_FILL_A(_y0,_sy);
	CalScreenNormal_FILL_A(_z0,_sz);
}

void CAgent_MatrixScale::_HitTest(int x,int y)
{
	_flag = 0;
	
	IRenderPort *rp = GetRP();
	assert(rp);
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	//mid triangle test
	i_math::triangle3df tri;	
	i_math::vector3df inter;
	tri.set(_x0,_y0,_z0);
	if(tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter))
	{
		_flag |= Active_XYZ;
		return;
	}
	
	// plane test
	BOOL ret = _HitTestArea(x,y);
	if(ret)
		return;

	// axis line test
	i_math::plane3df pXY,pYZ,pZX;
	pXY.setPlane(_center,_nz);
	pYZ.setPlane(_center,_nx);
	pZX.setPlane(_center,_ny);

	float distX,distY,distZ,d,rangeActive;
	i_math::vector3df p[3];
	
    rangeActive = _arg.activeRange;
	ScaleLen(rp,_center,rangeActive);

	distX = distY = distZ = rangeActive;

	d = _getDistDir(probe,_center,_nx,_lenAxisX,pXY);
	distX = (d>0&&(d<distX))?d:distX;
	d = _getDistDir(probe,_center,_nx,_lenAxisX,pZX);
	distX = (d>0&&(d<distX))?d:distX;

	d = _getDistDir(probe,_center,_ny,_lenAxisY,pXY);
	distY = (d>0&&(d<distY))?d:distY;
	d = _getDistDir(probe,_center,_ny,_lenAxisY,pYZ);
	distY = (d>0&&(d<distY))?d:distY;

	d = _getDistDir(probe,_center,_nz,_lenAxisZ,pZX);
	distZ = (d>0&&(d<distZ))?d:distZ;
	d = _getDistDir(probe,_center,_nz,_lenAxisZ,pYZ);
	distZ = (d>0&&(d<distZ))?d:distZ;
	
	float nearDist;
	DWORD flag = 0;
	nearDist = min(distX,distY);
	flag = (distX<distY)?Active_X:Active_Y;
	flag = (distZ<nearDist)?Active_Z:flag;
	nearDist = min(distZ,nearDist);

	if(nearDist<rangeActive)
		_flag |= flag;
	else
		_flag |=Active_XYZ;

}
void CAgent_MatrixScale::_DrawActiveArea()
{
	i_math::vector3df tri[6];
	IRenderPort *rp = GetRP();
	assert(rp);

	int idx = 0; 
#define DrawActiveArea_FILLTRI(t0,t1,t2)\
	{\
		if(idx>=6){			\
			idx =0 ;		\
		}					\
		tri[idx++] = t0;	\
		tri[idx++] = t1;	\
		tri[idx++] = t2;	\
	}

	if(_flag&Active_XY)
	{
		DrawActiveArea_FILLTRI(_x0,_y0,_y1);
		DrawActiveArea_FILLTRI(_x0,_x1,_y1);
	}
	else if(_flag&Active_YZ)
	{
		DrawActiveArea_FILLTRI(_y0,_z0,_z1);
		DrawActiveArea_FILLTRI(_y0,_y1,_z1);
	}
	else if(_flag&Active_ZX)
	{
		DrawActiveArea_FILLTRI(_z0,_x0,_x1);
		DrawActiveArea_FILLTRI(_z0,_z1,_x1);
	}
	else if(_flag&Active_XYZ)
	{
		DrawActiveArea_FILLTRI(_x0,_y0,_z0);
	}
	
	if(idx)
		rp->Triangles(tri,idx/3,_arg.colSel);

}
void CAgent_MatrixScale::_DrawAxisWire()
{
	i_math::matrix43f matWld,matSpace;
	matWld = _matValue*_data.matParent;
	_center.set(0,0,0);
	matWld.transformVect(_center,_center);

	IRenderPort * rp = GetRP();
	assert(rp);

	SpaceData data;
	data.matOffset = &_matValue;
	data.matParent = &_data.matParent;
	data.mode = _data.modespace;
	data.rp = rp;
	GetSpace(data,_nx,_ny,_nz,matSpace);
    	
	DWORD colX,colY,colZ;
	colX = ((_flag&Active_X)||(_flag&Active_XY)||(_flag&Active_ZX))?_arg.colActive:_arg.colX;
	colY = ((_flag&Active_Y)||(_flag&Active_XY)||(_flag&Active_YZ))?_arg.colActive:_arg.colY;
	colZ = ((_flag&Active_Z)||(_flag&Active_YZ)||(_flag&Active_ZX))?_arg.colActive:_arg.colZ;
	
	float lenAxis = _arg.lenAxis; 
	ScaleLen(rp,_center,lenAxis);

	i_math::vector3df sv;
	float scale;
#define DrawAxisWire_CALSCALE(t0,t1,t2,len)\
	{\
		sv.set(t0,t1,t2);						\
		_matScaleEdit.transformVect(sv,sv);		\
		scale = (float)sv.getLength();		\
		len = scale*lenAxis; \
	}

	DrawAxisWire_CALSCALE(1.0f,0.0f,0.0f,_lenAxisX);	
	DrawAxisWire_CALSCALE(0.0f,1.0f,0.0f,_lenAxisY);
	DrawAxisWire_CALSCALE(0.0f,0.0f,1.0f,_lenAxisZ);
	
	float lenTaper;
	lenTaper = DrawDirLineTaper(rp,_center,_nx,_lenAxisX,colX,_arg.colX,&_shaderState);
	lenTaper = DrawDirLineTaper(rp,_center,_ny,_lenAxisY,colY,_arg.colY,&_shaderState);
	lenTaper = DrawDirLineTaper(rp,_center,_nz,_lenAxisZ,colZ,_arg.colZ,&_shaderState);
	
	_lenAxisX += lenTaper;
	_lenAxisY += lenTaper;
	_lenAxisZ += lenTaper;

}
void CAgent_MatrixScale::_DrawTriAreaWire()
{
	i_math::vector3df mxy0,mxy1,myz0,myz1,mz_x0,mzx1;
	
	IRenderPort *rp = GetRP();
	assert(rp);
	
	_x0 = _x1 = _nx;
	_x0.setLength(_lenAxisX*_arg.s0);
	_x1.setLength(_lenAxisX*_arg.s1);
	_y0 = _y1 = _ny;
	_y0.setLength(_lenAxisY*_arg.s0);
	_y1.setLength(_lenAxisY*_arg.s1);
	_z0 = _z1 = _nz;
	_z0.setLength(_lenAxisZ*_arg.s0);
	_z1.setLength(_lenAxisZ*_arg.s1);
	_x0 += _center;
	_x1 += _center;
	_y0 += _center;
	_y1 += _center;
	_z0 += _center;
	_z1 += _center;

	mxy0 = 0.5f*(_x0+_y0);
	mxy1 = 0.5f*(_x1+_y1);
	myz0 = 0.5f*(_y0+_z0);
	myz1 = 0.5f*(_y1+_z1);
	mz_x0 = 0.5f*(_z0+_x0);
	mzx1 = 0.5f*(_z1+_x1);
	
	DWORD colXY,colYX,colYZ,colZY,colZX,colXZ;
	colXY = ((_flag&Active_XY)||(_flag&Active_XYZ))?_arg.colActive:_arg.colX;
	colYX = ((_flag&Active_XY)||(_flag&Active_XYZ))?_arg.colActive:_arg.colY;
	colYZ = ((_flag&Active_YZ)||(_flag&Active_XYZ))?_arg.colActive:_arg.colY;
	colZY = ((_flag&Active_YZ)||(_flag&Active_XYZ))?_arg.colActive:_arg.colZ;
	colXZ = ((_flag&Active_ZX)||(_flag&Active_XYZ))?_arg.colActive:_arg.colX;
	colZX = ((_flag&Active_ZX)||(_flag&Active_XYZ))?_arg.colActive:_arg.colZ;
	
	i_math::vector3df lines[24];
	DWORD cols[24];
	int idx = 0;
#define DrawTriArea_FILLLINE(t0,t1,col)\
	{	\
		cols[idx] = col;		\
		lines[idx++] = t0;		\
		cols[idx] = col;		\
		lines[idx++] = t1;		\
	}
	
	DrawTriArea_FILLLINE(_x0,mxy0,colXY);
	DrawTriArea_FILLLINE(_x1,mxy1,colXY);
	DrawTriArea_FILLLINE(_y0,mxy0,colYX);
	DrawTriArea_FILLLINE(_y1,mxy1,colYX);

	DrawTriArea_FILLLINE(_y0,myz0,colYZ);
	DrawTriArea_FILLLINE(_y1,myz1,colYZ);
	DrawTriArea_FILLLINE(_z0,myz0,colZY);
	DrawTriArea_FILLLINE(_z1,myz1,colZY);

	DrawTriArea_FILLLINE(_x0,mz_x0,colXZ);
	DrawTriArea_FILLLINE(_x1,mzx1,colXZ);
	DrawTriArea_FILLLINE(_z0,mz_x0,colZX);
	DrawTriArea_FILLLINE(_z1,mzx1,colZX);

	rp->Lines(lines,12,cols);
}	





