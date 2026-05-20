/********************************************************************
	created:	2008/04/01
	created:	1:4:2008   10:04
	filename: 	e:\IxEngine\Proj_GuiLib\Agent_MatrixMove.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	Agent_MatrixMove
	file ext:	cpp
	author:		star
	purpose:    matrix edit, move.	
*********************************************************************/
#include "stdh.h"
#include ".\agent_matrixmove.h"
#include "spatialtester/spatialtester.h"

CAgent_MatrixMove::CAgent_MatrixMove(void)
{
	_flag = 0;
}

CAgent_MatrixMove::~CAgent_MatrixMove(void)
{

}
BOOL  CAgent_MatrixMove::OnLButtonUp(int x,int y,DWORD flag)
{
	PreEdit();

	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);

	return FALSE;
}
void CAgent_MatrixMove::SetUIArg(MoveCtrlArg arg)
{
	_arg = arg;
}
BOOL CAgent_MatrixMove::Bind(MatrixEditData data)
{
	BOOL ret = CMatrixEditBase::Bind(data);
	if(!ret)
		return FALSE;
	_matValue = (*data.matrix);
	
	return TRUE;
}

BOOL CAgent_MatrixMove::OnDraw()
{
	PreEdit();
	IRenderPort *rp = GetRP();
	rp->ClearBuffer(ClearBuffer_Depth);
	_DrawAxisLine();
	_DrawAxisQuad();
	return FALSE;
}

BOOL CAgent_MatrixMove::OnMouseMove(int x,int y,DWORD flag)
{
	PreEdit();

	if(_bInDrag)
		OnDrag(x,y);
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

void CAgent_MatrixMove::OnEndDrag(int x,int y)
{
	_funEndEditListener(this);
}

void CAgent_MatrixMove::OnDrag(int x,int y)
{
	i_math::matrix43f matTrans;
	i_math::matrix43f matInverse,matSpace,matWorld;
	
	matWorld = _matValue*_data.matParent;
	matWorld.getInverse(&matInverse);

	float ex,ey,ez;
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	_CalInitPos(probe,ex,ey,ez);
	
	i_math::vector3df vs,ve,off;	
	vs.set(_ix,_iy,_iz);
	ve.set(ex,ey,ez);

	off = ve - vs;
	i_math::vector3df v0(0,0,0),vx,vy,vz,pos(0,0,0);
	vx.set(1.0f,0.0f,0.0f);
	vy.set(0.0f,1.0f,0.0f);
	vz.set(0.0f,0.0f,1.0f);
	matInverse.transformVect(v0,v0);
	matInverse.transformVect(vx,vx);
	matInverse.transformVect(vy,vy);
	matInverse.transformVect(vz,vz);
	
	vx -=v0;
	vy -=v0;
	vz -=v0;

	vx.normalize();
	vy.normalize();
	vz.normalize();
	matSpace.set(vx.x,vx.y,vx.z,
				 vy.x,vy.y,vy.z,
				 vz.x,vz.y,vz.z,
				 0,0,0);
	
	i_math::vector3df ivx,ivy,ivz;
	if(TRUE)
	{
		float md;
		md = _nx.dotProduct(off);
		ivx = _nx;
		ivx.setLength(md);
		
		md = _ny.dotProduct(off);
		ivy = _ny;
		ivy.setLength(md);

		md = _nz.dotProduct(off);
		ivz = _nz;
		ivz.setLength(md);
	}	

	switch(_flag)
	{
	case Active_X:
		{
			off -= ivy;
			off -= ivz;
			break;
		}
	case Active_Y:
		{
			off -= ivx;
			off -= ivz;
			break;
		}
	case Active_Z:
		{
			off -= ivx;
			off -= ivy;
			break;
		}
	case Active_XY:
		{
			off -= ivz;
			break;
		}
	case Active_YZ:
		{
			off -= ivx;
			break;
		}
	case Active_ZX:
		{
			off -= ivy;
			break;
		}
	default:
		{
			break;
		}
	}
	
	i_math::vector3df ts,org(0,0,0);
	matWorld.transformVect(org,org);

#define CAgent_MatrixMove_OnDrag_CALSCALE(t0,t1,t2,scale)\
	{\
		ts.set(t0,t1,t2);			\
		matSpace.transformVect(ts,ts);	\
		matWorld.transformVect(ts,ts);	\
		ts -=org;\
		scale = (float)ts.getLength();\
	}
	
	float s[3];
	CAgent_MatrixMove_OnDrag_CALSCALE(1.0f,0.0f,0.0f,s[0]);
	CAgent_MatrixMove_OnDrag_CALSCALE(0.0f,1.0f,0.0f,s[1]);
	CAgent_MatrixMove_OnDrag_CALSCALE(0.0f,0.0f,1.0f,s[2]);

	off.x /= s[0];
	off.y /= s[1];
	off.z /= s[2];

	matTrans.setTranslation(off);
	matTrans.transformVect(pos,pos);
	matSpace.transformVect(pos,pos);

	matTrans.setTranslation(pos); 
	(*_data.matrix) = matTrans*_matInit;
}
BOOL CAgent_MatrixMove::OnLButtonDown(int x,int y,DWORD flag)
{
	PreEdit();

	IRenderPort * rp = GetRP();
	assert(rp);
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	_CalInitPos(probe,_ix,_iy,_iz,TRUE);

	_matInit = (*_data.matrix);
	
	if(!flag)
	{
		_bInDrag=TRUE;
		OnBeginDrag(x,y);
	}

	return TRUE;
}

void CAgent_MatrixMove::_DrawAxisLine()
{
	i_math::line3df lineX,lineY,lineZ;
	
	IRenderPort * rp = GetRP();
	assert(rp);
    
	_center.set(0,0,0);
	i_math::matrix43f matWorld;
	matWorld = _matValue*_data.matParent;
	matWorld.transformVect(_center,_center);
	
	SpaceData data;
	data.mode = _data.modespace;
	data.matOffset = &_matValue;
	data.matParent = &_data.matParent;
	data.rp = rp;
	GetSpace(data,_nx,_ny,_nz,_matSpace,&_matFromWld);

	
	DWORD colX ,colY ,colZ ,colQx,colQy,colQz;
	colX = (_flag&Active_X||_flag&Active_XY||_flag&Active_ZX)?_arg.colActive:_arg.colX;
	colY = (_flag&Active_Y||_flag&Active_XY||_flag&Active_YZ)?_arg.colActive:_arg.colY;
	colZ = (_flag&Active_Z||_flag&Active_YZ||_flag&Active_ZX)?_arg.colActive:_arg.colZ;
	
	float lenAxis = _arg.lenAxis,lenTaper;
	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
	ScaleLen(rp,_center,lenAxis);
	
	extern float DrawDirLineTaper(IRenderPort * rp, const i_math::vector3df & point,const i_math::vector3df & dir,float len,DWORD colLine,DWORD colTaper);
	lenTaper = DrawDirLineTaper(rp,_center,_nx,lenAxis,colX,_arg.colX);
	lenTaper = DrawDirLineTaper(rp,_center,_ny,lenAxis,colY,_arg.colY);
	lenTaper = DrawDirLineTaper(rp,_center,_nz,lenAxis,colZ,_arg.colZ);
	
	_lenAxis = lenAxis + lenTaper;

	extern void DrawQuadWire(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD *cols/*in out*/,float & width/* in out*/,float & height /* in out*/);
	
	colQx = (_flag&Active_XY)?_arg.colActive:_arg.colX;
	colQy = (_flag&Active_XY)?_arg.colActive:_arg.colY;
	float w ,h;
	DWORD  cols[4];
	DWORD colNone = ColorAlpha(0x000000,0x00);
	const float ratio = 0.4f;
	w = _lenAxis*ratio;
	h = - _lenAxis*ratio;
	cols[0] = colX;
	cols[1] = colQy;
	cols[2] = colY;
	cols[3] = colQx;
	DrawQuadWire(rp,_nz,_nx,_center,cols,w,h);

	colQy = (_flag&Active_YZ)?_arg.colActive:_arg.colY;
	colQz = (_flag&Active_YZ)?_arg.colActive:_arg.colZ;
	w = _lenAxis*ratio;
	h = -_lenAxis*ratio;
	cols[0] = colY;
	cols[1] = colQz;
	cols[2] = colZ;	
	cols[3] = colQy;
	DrawQuadWire(rp,_nx,_ny,_center,cols,w,h);
	
	colQx = (_flag&Active_ZX)?_arg.colActive:_arg.colX;
	colQz = (_flag&Active_ZX)?_arg.colActive:_arg.colZ;
	w =  _lenAxis*ratio;
	h = - _lenAxis*ratio;
	cols[0] = colZ;
	cols[1] = colQx;
	cols[2] = colX;
	cols[3] = colQz;
	DrawQuadWire(rp,_ny,_nz,_center,cols,w,h);

	_w = w;
}
void CAgent_MatrixMove::_HitTest(int x,int y)
{	
	
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	i_math::line3df line = (i_math::line3df)probe;
	_flag = 0;

	if(TRUE)
	{
		DWORD flag[3];
		i_math::vector3df posinter[3];
		int num = 0;
		
		i_math::vector3df nx,ny,nz;
		i_math::vector3df pos[4];
		BOOL ret = FALSE;
		nx = _nx;
		ny = _ny;
		nz = _nz;	
		nx.setLength(_w);
		ny.setLength(_w);
		nz.setLength(_w);
		//xy
		pos[0] = _center;
		pos[1] = _center + nx;
		pos[3] = _center + ny;
		pos[2] = pos[3] + nx;	
		ret = _HitTestQuad(line,pos,&posinter[num]);
		if(TRUE==ret)
			flag[num++] = Active_XY;
		//zx
		pos[1] = _center + nx;
		pos[3] = _center + nz;
		pos[2] = pos[3]  + nx;
		ret = _HitTestQuad(line,pos,&posinter[num]);
		if(TRUE == ret)
			flag[num++] = Active_ZX;

		//yz
		pos[1] = _center + ny;
		pos[3] = _center + nz;
		pos[2] = pos[3] + ny;
		ret = _HitTestQuad(line,pos,&posinter[num]);
		if(TRUE == ret)
			flag[num++] = Active_YZ;
		
		if(num>0)
		{
			i_math::matrix43f matViewProj;
			i_math::matrix44f matViewProj0;
			DWORD flagActive;
			ICamera * camer = rp->GetCamera();
			camer->GetViewProj(matViewProj0);
			mat43from44(matViewProj,matViewProj0);	
			matViewProj.transformVect(posinter[0],posinter[0]);
			float z = posinter[0].z,tmp;
			flagActive = flag[0];
			for(int iz =0; iz<num;iz++)
			{
				matViewProj.transformVect(posinter[iz],posinter[iz]);
				tmp = posinter[iz].z;
				if(tmp<z)
				{
					flagActive = flag[iz];
					z = tmp;
				}
			}
			_flag |= flagActive;
			return;
		}
	}


	i_math::plane3df plane;
	plane.setPlane(_center,_ny);
	float dist,rangeActive;

	rangeActive = _arg.activeRange;
	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
	ScaleLen(rp,_center,rangeActive);

	dist = _getDistDir(probe,_center,_nx,_lenAxis,plane);
	if(dist>=0&&dist<rangeActive)
	{
		_flag |= Active_X;
		return;
	}
	
	dist = _getDistDir(probe,_center,_nz,_lenAxis,plane);
	if(dist>=0&&dist<rangeActive)
	{
		_flag |= Active_Z;
		return;
	}

	plane.setPlane(_center,_nx);
	dist = _getDistDir(probe,_center,_ny,_lenAxis,plane);
	if(dist>=0&&dist<rangeActive)
	{
		_flag |=Active_Y;
		return;
	}

}
void CAgent_MatrixMove::_DrawAxisQuad()
{	
	IRenderPort * rp = GetRP();
	extern	void DrawQuad(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD col,float & width/* in out*/,float & height /* in out*/);
	float w,h;
	const float ratio = 0.4f;
	w = _lenAxis*ratio;
	h = -_lenAxis*ratio;
	
	if(_flag&Active_XY)
		DrawQuad(rp,_nz,_nx,_center,_arg.colQuad,w,h);
	else if(_flag&Active_YZ)
		DrawQuad(rp,_nx,_ny,_center,_arg.colQuad,w,h);
	else if(_flag&Active_ZX)
		DrawQuad(rp,_ny,_nz,_center,_arg.colQuad,w,h);
}

void CAgent_MatrixMove::_CalInitPos(HitProbe &vecHit,float &x,float &y,float &z,BOOL bInit )
{	
	i_math::plane3df pZX ,pXY ,pYZ,pS;
	i_math::vector3df vecInter,vecNs;
	
	i_math::vector3df vx,vy,vz,vs,start;
	i_math::vector3df center;

	int xs,ys;
	IRenderPort * rp = GetRP();
	HitProbe probeS;
	rp->TransPos(_center,xs,ys);
	rp->CalcHitProbe(xs,ys,probeS);
	vs = probeS.start - probeS.end;
	vs.normalize();
	pS.setPlane(_center,vs);

	vx = _nx;
	vy = _ny;
	vz = _nz;
	center = _center;

	pXY.setPlane(center,vz);
	pYZ.setPlane(center,vx);
	pZX.setPlane(center,vy);

	vecNs = vecHit.start - vecHit.end;
	vecNs.normalize();

	start = vecHit.start;

	if(bInit)
	{
		_ix = center.x;
		_iy = center.y;
		_iz = center.z;
	}
	
	bool ret = false;
	if(_flag&Active_X)
	{	
	    ret = false;
		ret = pS.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}
	else if(_flag&Active_Y)
	{
	    ret = false;
		ret = pS.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}
	else if(_flag&Active_Z)
	{
		ret = false;
		ret = pS.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}
	else if(_flag&Active_XY)
	{
		ret =false;
		ret =pXY.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}
	else if(_flag&Active_YZ)
	{
		ret =false;
		ret = pYZ.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}
	else if(_flag&Active_ZX)
	{
		ret = false;
		ret = pZX.getIntersectionWithLine(start,vecNs,vecInter);
		assert(ret);
	}

	if(((abs(vecInter.x-_ix)>_arg.mx)||(abs(vecInter.y-_iy)>_arg.my)||(abs(vecInter.z-_iz)>_arg.mz)))
	{
		*_data.matrix = _matInit;
		ReleaseCapture();
		OnEndDrag(0,0);
		_bInDrag = FALSE;
	}
	else
	{
		x = vecInter.x;
		y = vecInter.y;
		z = vecInter.z; 
	}
}
