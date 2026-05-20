#include "stdh.h"
#include ".\Agent_MatrixSimpleScale.h"

CAgent_MatrixSimpleScale::CAgent_MatrixSimpleScale(void)
{
	_flag = 0;
}

CAgent_MatrixSimpleScale::~CAgent_MatrixSimpleScale(void)
{
}


extern float DrawDirLineTaper(IRenderPort * rp, const i_math::vector3df & point,const i_math::vector3df & dir,float len,DWORD colLine,DWORD colTaper);

BOOL CAgent_MatrixSimpleScale::Bind(MatrixEditData data)
{
	BOOL ret =  CMatrixEditBase::Bind(data);
	return ret;
}
BOOL CAgent_MatrixSimpleScale::OnDraw()
{
	PreEdit();
	IRenderPort *rp = GetRP();
	rp->ClearBuffer(ClearBuffer_Depth);
	_DrawAxisWire();
	_DrawTriAreaWire();
	_DrawActiveArea();
	return FALSE;
}
BOOL CAgent_MatrixSimpleScale::OnMouseMove(int x,int y,DWORD flag)
{	
	PreEdit();
	if(_bInDrag)
		OnDrag(x,y);
	else
	{
		_HitTest(x,y);
		_bSel = _bInDrag;
	}
	_bSel = (_bInDrag||_HitTest(x,y));
	return !_flag;
}
void CAgent_MatrixSimpleScale::OnEndDrag(int x,int y)
{
	_matScaleEdit.makeIdentity();
	_funEndEditListener(this);
}
void CAgent_MatrixSimpleScale::OnDrag(int x,int y)
{
	const interval = 10;
	int totalX,totalY;

	if (TRUE)  // reset cursor position
	{
		POINT  point;
		GetCursorPos(&point);
		totalX = GetSystemMetrics(SM_CXFULLSCREEN);
		totalY = GetSystemMetrics(SM_CYFULLSCREEN);

		BOOL bChange = FALSE;
		if(point.x < interval)
		{
			_scrX = _scrX - totalX - interval-1 -point.x;
			point.x = totalX - interval-1;		
			bChange = TRUE;
		}
		else if(point.x > totalX - interval)
		{
			_scrX += point.x - interval +1;  
			point.x = interval + 1;	
			bChange = TRUE;
		}

		if(point.y < interval)
		{
			_scrY = _scrY - totalY - interval-1 -point.y;
			point.y = totalY - interval-1;		
			bChange = TRUE;
		}
		else if(point.y > totalY - interval)
		{
			_scrY += point.y - interval +1;
			point.y = interval + 1;
			bChange = TRUE;
		}

		if(bChange)
			SetCursorPos(point.x,point.y);
	}

	float nx ,ny ;
	nx = (float)(x-_ix+_scrX);
	ny = (float)(y-_iy+_scrY);

	i_math::matrix43f matScale;
	i_math::vector2df curOff,curNs;
	curOff.set((float)(nx),(float)(ny));
	curNs.set(0,-1.0f);
	float idot =(float)(curNs.dotProduct(curOff)); 
	float scaleValue =  1.0f+idot*_arg.speed;
	if(scaleValue<=0)
		scaleValue = 1.0f;

	matScale.setScale(scaleValue,scaleValue,scaleValue);
	
	_matScaleEdit = matScale;
	*_data.matrix = matScale*_matInit;
}

BOOL CAgent_MatrixSimpleScale::OnLButtonDown(int x,int y,DWORD flag)
{
	PreEdit();

	if(!flag)
	{
		_bInDrag = TRUE;
		OnBeginDrag(x,y);
		OccupyFocus(OpType_Mouse);
	}
	_ix = x;
	_iy = y;
	_scrX = 0;
	_scrY = 0;
	_matInit = *_data.matrix;
	
	return TRUE;
}
BOOL CAgent_MatrixSimpleScale::OnLButtonUp(int x,int y,DWORD flag)
{
	PreEdit();
	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);

	return FALSE;
}

BOOL CAgent_MatrixSimpleScale::_HitTest(int x,int y)
{
	i_math::triangle3df tri;
	tri.set(_x0,_y0,_z0);

	IRenderPort *rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	i_math::vector3df inter;
	bool ret = tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter);

	if(ret)
		return TRUE;
	else
		return FALSE;
}
void CAgent_MatrixSimpleScale::_DrawActiveArea()
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

	DrawActiveArea_FILLTRI(_x0,_y0,_z0);

	if(idx)
		rp->Triangles(tri,idx/3,_arg.colSel);

}
void CAgent_MatrixSimpleScale::_DrawAxisWire()
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

	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len); 
	float lenAxis = _arg.lenAxis; 
	ScaleLen(rp,_center,lenAxis);

	i_math::vector3df sv;
	float scale;
#define MatrixSimpleScale_DrawAxisWire_CALSCALE(t0,t1,t2,len)\
	{\
	sv.set(t0,t1,t2);						\
	_matScaleEdit.transformVect(sv,sv);		\
	scale = (float)sv.getLength();		\
	len = scale*lenAxis; \
	}

	MatrixSimpleScale_DrawAxisWire_CALSCALE(1.0f,0.0f,0.0f,_lenAxisX);	
	MatrixSimpleScale_DrawAxisWire_CALSCALE(0.0f,1.0f,0.0f,_lenAxisY);
	MatrixSimpleScale_DrawAxisWire_CALSCALE(0.0f,0.0f,1.0f,_lenAxisZ);

	i_math::vector3df lineEnd;
#define MatrixSimpleScale_DrawAxisWire_DRAWLINE(n,len)\
	{\
		lineEnd = n;							\
		lineEnd.setLength(len);			\
		lineEnd += _center;						\
		rp->Line(_center,lineEnd,_arg.colActive);\
	}
	
	MatrixSimpleScale_DrawAxisWire_DRAWLINE(_nx,_lenAxisX);
	MatrixSimpleScale_DrawAxisWire_DRAWLINE(_ny,_lenAxisY);
	MatrixSimpleScale_DrawAxisWire_DRAWLINE(_nz,_lenAxisZ);
}
void CAgent_MatrixSimpleScale::_DrawTriAreaWire()
{
	i_math::vector3df mxy0,mxy1,myz0,myz1,mz_x0,mzx1;

	IRenderPort *rp = GetRP();
	assert(rp);

	_x0 = _nx;
	_x0.setLength(_lenAxisX);
	_y0 = _ny;
	_y0.setLength(_lenAxisY);
	_z0 = _nz;
	_z0.setLength(_lenAxisZ);
	_x0 += _center;
	_y0 += _center;
	_z0 += _center;

	i_math::vector3df lines[6];
	DWORD cols[6];
	int idx = 0;
#define DrawTriArea_FILLLINE(t0,t1,col)\
	{	\
	cols[idx] = col;		\
	lines[idx++] = t0;		\
	cols[idx] = col;		\
	lines[idx++] = t1;		\
	}
	DrawTriArea_FILLLINE(_x0,_y0,_arg.colActive);
	DrawTriArea_FILLLINE(_y0,_z0,_arg.colActive);
	DrawTriArea_FILLLINE(_z0,_x0,_arg.colActive);

	rp->Lines(lines,3,cols);
}	
