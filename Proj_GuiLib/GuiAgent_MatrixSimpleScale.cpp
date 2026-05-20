
#include "stdh.h"

#include ".\GuiAgent_MatrixSimpleScale.h"

#include "TransformInputDlg.h"

#include <assert.h>

#include "TransformSettingsDlg.h"

extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len); 

CAgent_MatrixSimpleScale::CAgent_MatrixSimpleScale(void)
{
	_flag = 0;
}
CAgent_MatrixSimpleScale::~CAgent_MatrixSimpleScale(void)
{
}
BOOL CAgent_MatrixSimpleScale::Bind(MatrixEditData &data)
{
	BOOL ret = CMatrixEditBase::Bind(data);
	if(!ret)
		return FALSE;
	_matInit = (*_data.matrix);
	_matValue = (*_data.matrix);
	
	return TRUE;
}
void CAgent_MatrixSimpleScale::OnDlgEdit(BOOL bRelative,DWORD changeType,const i_math::vector3df& vecChange,i_math::matrix43f &matLocal)
{
	i_math::vector3df vecChangeLocal = vecChange;
	
	i_math::matrix43f matChange;
	matChange.setScale(vecChangeLocal);
	_CheckScale(matChange); //确保变化后不至于过大或过小

	_Redraw(FALSE);

	if(changeType==CTransformInputDlg::Change_Begin&&!_funPerEditListener.empty())
		_funPerEditListener(this);
	else{
		_matValue = matChange*(*_data.matrix);
		matLocal = _matValue;	
		*_data.matrix = matLocal;
		if(changeType==CTransformInputDlg::Change_On&&!_funOnEditListener.empty())
			_funOnEditListener(this);
		else if(changeType==CTransformInputDlg::Change_End&&!_funEndEditListener.empty())
			_funEndEditListener(this);
	}
}
BOOL CAgent_MatrixSimpleScale::OnDraw()
{
	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Scale)
		return TRUE;	
	IRenderPort *rp = GetRP();

	_DrawAxisWire();
	_DrawTriAreaWire();
	
	if(_bSel)
		_DrawActiveArea();

	return TRUE;
}
BOOL CAgent_MatrixSimpleScale::OnMouseMove(int x,int y,DWORD flag)
{	
	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Scale)
		return TRUE;	

	static int oldx,oldy;
	if(oldx==x&&oldy==y)
		return TRUE;
	else
	{
		oldx = x;
		oldy = y;
	}

	if(_bInDrag&_bSel)
		OnDrag(x,y,flag);
	else
	{
		BOOL oldSel = _bSel;
		_bSel = _HitTest(x,y);
		if(oldSel!=_bSel)
			_Redraw(FALSE);
	}

	return !_bSel;
}
void CAgent_MatrixSimpleScale::OnEndDrag(int x,int y,DWORD flag)
{
	_matScaleEdit.makeIdentity();
	_Redraw(FALSE);
	_funEndEditListener(this);
}
void CAgent_MatrixSimpleScale::OnDrag(int x,int y,DWORD flag)
{
	const int interval = 10;
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
	
	if(TRUE){
		float sSnap = GetTransformSettings().SnapScale(scaleValue);
		scaleValue = sSnap;
	}
			
	matScale.setScale(scaleValue,scaleValue,scaleValue);
	_CheckScale(matScale); //确保变化后不至于过大或过小

	_matScaleEdit = matScale;
	*_data.matrix = matScale*_matInit;
	
	static i_math::matrix43f matold;
	if(!_matScaleEdit.equals(matold))
	{
		_Redraw(FALSE);
		matold = _matScaleEdit;
		_funOnEditListener(this);
//		GetTransformDlg()->UpdateMatrix(*_data.matrix);
	}
}
void CAgent_MatrixSimpleScale::_CheckScale(i_math::matrix43f & matScale)
{
	if(!_data.matrix)
		return;
	
	i_math::matrix43f matWorld = (*_data.matrix)*_data.matParent;
	i_math::vector3df scale    = matScale.getScale();
	i_math::vector3df scaleOld = matWorld.getScale();
	i_math::vector3df scaleNew = scale*scaleOld;

	scaleNew.x = i_math::clamp_f(scaleNew.x,0.05f,20.0f);
	scaleNew.y = i_math::clamp_f(scaleNew.y,0.05f,20.0f);
	scaleNew.z = i_math::clamp_f(scaleNew.z,0.05f,20.0f);

	scale.x = scaleNew.x/scaleOld.x;
	scale.y = scaleNew.y/scaleOld.y;
	scale.z = scaleNew.z/scaleOld.z;

	matScale.setScale(scale);
}
BOOL CAgent_MatrixSimpleScale::OnLButtonDown(int x,int y,DWORD flag)
{
	if(_bInDrag)
		_bSel = _HitTest(x,y);

	if(!_bSel)
	{
		_bInDrag = FALSE;
		return TRUE;
	}

	PreEdit(EditMode_Scale);

	if(!flag)
	{
		_bInDrag = TRUE;
		OnBeginDrag(x,y,flag);
		OccupyFocus(OpType_Mouse);
	}

	_ix = x;
	_iy = y;
	_scrX = 0;
	_scrY = 0;
	_matInit = *_data.matrix;
	
	return FALSE;
}
BOOL CAgent_MatrixSimpleScale::OnLButtonUp(int x,int y,DWORD flag)
{
	if(!_bSel)
		return TRUE;

	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Scale)
		return TRUE;	

	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);

	return !_bSel;
}

BOOL CAgent_MatrixSimpleScale::_HitTest(int x,int y)
{
	IRenderPort *rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	i_math::triangle3df tri;
	tri.set(_x0,_y0,_z0);

	i_math::vector3df inter;
	bool ret = tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter);
	if (!ret)
	{
		tri.set(_x0,_y0,_center);
		ret = tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter);
	}
	if (!ret)
	{
		tri.set(_x0,_z0,_center);
		ret = tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter);
	}
	if (!ret)
	{
		tri.set(_y0,_z0,_center);
		ret = tri.getSafeIntersectionWithLine(probe.start,probe.start - probe.end,inter);
	}
	
	return (ret==true);
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
	{
		ShaderState state=_shaderState;
		state.modeFacing=Facing_Both;
		rp->Triangles(tri,idx/3,_arg.colSel,&state);
	}

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
		rp->Line(_center,lineEnd,_arg.colActive,&_shaderState);\
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
	
	ShaderState state;
	state.modeDepth = Depth_NoCmp;
	rp->Lines(lines,3,cols,&state);
}	





