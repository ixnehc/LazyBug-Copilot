/********************************************************************
created:	2008/03/14
created:	14:3:2008   16:33
filename: 	e:\IxEngine\Proj_GuiLib\TransformRotCtrl.cpp
file path:	e:\IxEngine\Proj_GuiLib
file base:	TransformRotCtrl
file ext:	cpp
author:		star
purpose:	edit matrix in rotate part 
*********************************************************************/

#include "stdh.h"
#include ".\GuiAgen_MatrixRot.h"
#include "stringparser/stringparser.h"
#include "GuiEditor_res.h"
#include "DummiesEditPanel.h"

#define M_PI       3.14159265358979323846f
extern float DrawCircleWire(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius ,DWORD col,DWORD nSeg=100,i_math::vector3df *cilpVec=NULL);
extern void DrawCirclePlane(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius,i_math::vector3df *start,float angle,DWORD col,DWORD nSeg);

void CGuiAgent_MatrixRot::SetUIArg(RotCtrlArg arg)
{
	_arg = arg;
}

BOOL CGuiAgent_MatrixRot::OnDraw()
{
	PreEdit();

	_DrawCross();
	_DrawArc();
	_DrawCenterArc();

	if(_bInDrag&&_flag)
	{
		_DrawDirLine();
		_DrawDragInfo();
		if(!(_flag&Active_OZ))
			_DrawDragArc();
	}
	return FALSE;
}
#define Draw_InvalidCross(rp,v,n)	\
	rp->Line(v0,v,uncol);		\
	rp->DrawText(n,fontArg);\


void CGuiAgent_MatrixRot::_DrawDirLine()
{
	IRenderPort * rp = GetRP();
	assert(rp);

	int idx = 0;
	if(_flag&Active_X) idx++;
	if(_flag&Active_Y) idx++;
	if(_flag&Active_Z) idx++;
	if(_flag&Active_OZ)idx++;

	if(idx!=1)
		return;

	float len = _arg.lenDirline;
	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
	ScaleLen(rp,_center,len);
	if(_angleRot<0)
		len = -len;
	extern void DrawDirLine(IRenderPort *rp ,i_math::vector3df &normal,i_math::vector3df &nvec,i_math::vector3df & point,float len ,float dirLen,DWORD col);
	DrawDirLine(rp,_dirLine.normal,_dirLine.nvec,_dirLine.start,len,0.2f*_arg.lenDirline,_arg.colActive);
}

void CGuiAgent_MatrixRot::_DrawCross()
{
	if(!(_data.matrix))
		return;
	
	IRenderPort * rp = GetRP();
	assert(rp);

	i_math::vector3df v0,vx,vy,vz;
	v0.set(0,0,0);
	vx = _nx;
	vy = _ny;
	vz = _nz;
	vx.setLength(_arg.lencs);
	vy.setLength(_arg.lencs);
	vz.setLength(_arg.lencs);

	//view position
	ICamera * camera= rp->GetCamera();
	assert(camera);
	i_math::matrix43f  matTrans,matScale;
	matTrans = _matValue*_data.matParent;
	i_math::vector3df offset = matTrans.getTranslation();
	extern GetProjScaleMask(ICamera *camera,i_math::matrix43f &matTrans,i_math::matrix43f &matScale);
	GetProjScaleMask(camera,matTrans,matScale);
	matScale.addTranslation(offset);
	
	matScale.transformVect(v0,v0);
	matScale.transformVect(vx,vx);
	matScale.transformVect(vy,vy);
	matScale.transformVect(vz,vz);

	int x,y;
	DrawFontArg fontArg;
	const DWORD uncol=ColorAlpha(0x808080,0xff);
	
	rp->TransPos(vx,x,y);
	fontArg.SetLocation(x+4,y-4);
	if(_flag&Active_X)
	{
		rp->Line(v0,vx,ColorAlpha(0xff3f00,0xff));
		rp->DrawText("{F3}{S12}{N1}{C255,64,0}x",fontArg);
	}
	else
		Draw_InvalidCross(rp,vx,"{F3}{S12}{N1}{C64,64,64}x");
	
	rp->TransPos(vy,x,y);
	fontArg.SetLocation(x+4,y-4);
	if(_flag&Active_Y)
	{
		rp->Line(v0,vy,ColorAlpha(0x7fff00,0xff));
		rp->DrawText("{F3{S12}{N1}{C128,255,0}",fontArg);
	}
	else
		Draw_InvalidCross(rp,vy,"{F3}{S12}{N1}{C64,64,64}y");
	
	rp->TransPos(vz,x,y);
	fontArg.SetLocation(x+4,y-4);
	if(_flag&Active_Z)
	{
		rp->Line(v0,vz,ColorAlpha(0x007fff,0xff));
		rp->DrawText("{F3}{S12}{N1}{C0,128,255}z",fontArg);
	}
	else
		Draw_InvalidCross(rp,vz,"{F3}{S12}{N1}{C64,64,64}z");
}
void CGuiAgent_MatrixRot:: _DrawCenterArc()
{
	IRenderPort * rp = GetRP();
	assert(rp);

	i_math::vector3df v0,nx,ny,nz;
	v0.set(0.0f,0.0f,0.0f);
	i_math::matrix43f  matTrans;
	matTrans = _matValue*_data.matParent;
	matTrans.transformVect(v0,v0);
	
	i_math::vector3df normal;
	int x,y;
	rp->TransPos(v0,x,y);
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	normal = probe.start - probe.end;
	
	_innerdius = DrawCircleWire(rp,v0,normal,_arg.radius,_arg.colCenterLine,200);

	if(_flag==Active_XY)
		DrawCirclePlane(rp,v0,normal,_arg.radius,NULL,360.0f,_arg.colSel,200);
}
void CGuiAgent_MatrixRot::_DrawArc()
{
	IRenderPort * rp = GetRP();
	assert(rp);

	i_math::vector3df v0,nx,ny,nz;
	i_math::matrix43f matTrans,mat;
	int x,y;	
	matTrans = _matValue*_data.matParent;
	_center.set(0,0,0);
	matTrans.transformVect(_center,_center);
	rp->TransPos(_center,x,y);
	
	SpaceData data;
	data.matOffset = &_matValue;
	data.matParent = &_data.matParent;
	data.mode = _data.modespace;
	data.rp = rp;
	
	GetSpace(data,_nx,_ny,_nz,mat);	
	data.mode = EditSpace_Screen;
	GetSpace(data,_nsx,_nsy,_nsz,mat);

	DWORD colActive = ColorAlpha(0x00ffff,0xff);

	DWORD colX = (_flag&Active_X)?_arg.colActive:ColorAlpha(0xff0000,0xff);
	DWORD colY = (_flag&Active_Y)?_arg.colActive:ColorAlpha(0x00ff00,0xff);
	DWORD colZ = (_flag&Active_Z)?_arg.colActive:ColorAlpha(0x0000ff,0xff);
	DWORD colOZ = (_flag&Active_OZ)?_arg.colActive:ColorAlpha(0x888888,0xff);

	i_math::vector3df vecClip;
	HitProbe probeVec;
	rp->CalcHitProbe(x,y,probeVec);
	vecClip = probeVec.start - probeVec.end;

	_radius = DrawCircleWire(rp,_center,_nx,_arg.radius,colX,200,&vecClip);
	_radius = DrawCircleWire(rp,_center,_ny,_arg.radius,colY,200,&vecClip);
	_radius = DrawCircleWire(rp,_center,_nz,_arg.radius,colZ,200,&vecClip);

	HitProbe probe;
	rp->TransPos(_center,x,y);
	rp->CalcHitProbe(x,y,probe);
	i_math::vector3df outNoraml = probe.start- probe.end;
	_outdius = DrawCircleWire(rp,_center,outNoraml,1.2f*_arg.radius,colOZ,200);
}
BOOL CGuiAgent_MatrixRot::_HitCenterTest(int x,int y)
{
	IRenderPort * rp = GetRP();
	int x0,y0;
	HitProbe probeVec,probeNormal;;
	rp->TransPos(_center,x0,y0);
	rp->CalcHitProbe(x0,y0,probeNormal);
	rp->CalcHitProbe(x,y,probeVec);

	i_math::plane3df plane;
	i_math::vector3df normal,vecIntersection;

	normal = probeNormal.start - probeNormal.end;
	plane.setPlane(_center,normal);
	bool bInsect =  plane.getIntersectionWithLine(probeVec.start,probeVec.start- probeVec.end,vecIntersection);
	if(bInsect)
	{
		float dist = (float)vecIntersection.getDistanceFrom(_center);
		if(dist<_innerdius)
			return TRUE;
	}

	return FALSE;
}
BOOL CGuiAgent_MatrixRot::_HitCircleTest(HitProbe & probe,i_math::vector3df &center ,i_math::vector3df &normal,float radius,BOOL bUpdate)
{
	IRenderPort *rp = GetRP();
	assert(rp);
	
	const float tolerance = 0.03f;

	float rangeActive = _arg.activeRange;
	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
	ScaleLen(rp,_center,rangeActive);

	i_math::vector3df vn,cv;
	vn = probe.end-probe.start;
	vn.normalize();
	normal.normalize();

	i_math::plane3df planeClip;
	i_math::vector3df vecClip;
	HitProbe  probeClip;
	int x,y;
	rp->TransPos(center,x,y);
	rp->CalcHitProbe(x,y,probeClip);
	vecClip = probeClip.start - probeClip.end;
	planeClip.setPlane(center,vecClip);

	float cox = vn.dotProduct(normal);
	if(abs(vn.dotProduct(normal))>tolerance)
	{
		i_math::plane3df  plane;

		plane.setPlane(center,normal);
		plane.getIntersectionWithLine(probe.start,probe.start-probe.end,cv);
		
		float dist = (float)cv.getDistanceFrom(center);
		if(abs(dist-radius)< rangeActive)
		{
			i_math::vector3df  t1; 
			t1 = cv - center;
			t1.setLength(radius);
			_dirLine.start = t1 + center;
			
			if(planeClip.getDistanceTo(_dirLine.start)<0)
				return FALSE;

			if(bUpdate)
			{
				_dirLine.nradius = t1;
				_dirLine.center = center;
				_dirLine.normal = normal;
				_dirLine.nvec = t1.crossProduct(normal);
			}
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
		i_math::vector3df line = probe.start - center;
		if(abs(line.dotProduct(normal)) < tolerance)
		{
			if(bUpdate)
			{
				line.setLength(radius);
				
				_dirLine.nradius = line;
				_dirLine.center = center;
				_dirLine.normal = normal;
				_dirLine.start = line + center;
				_dirLine.nvec = line.crossProduct(normal);
			}	
			return TRUE;
		}
		else
			return FALSE;
	}
}

BOOL CGuiAgent_MatrixRot::OnLButtonUp(int x,int y,DWORD flag)
{
	PreEdit();

	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);

	return FALSE;
}

BOOL CGuiAgent_MatrixRot::OnLButtonDown(int x,int y,DWORD flag)
{
	PreEdit();

	IRenderPort *rp = GetRP();
	assert(rp);

	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	i_math::vector3df outNormal;
	outNormal = probe.start - probe.end;

	if(_HitCircleTest(probe,_center,_nx,_radius,TRUE))
	{
		_flag = 0;
		_flag |= Active_X;
	}
	else if(_HitCircleTest(probe,_center,_ny,_radius,TRUE))
	{
		_flag = 0;
		_flag |= Active_Y;
	}
	else if(_HitCircleTest(probe,_center,_nz,_radius,TRUE))
	{
		_flag = 0;
		_flag |= Active_Z;
	}
	else if(_HitCircleTest(probe,_center,outNormal,_outdius,TRUE))
	{
		_flag = 0;
		_flag |= Active_OZ;
	}
	else
	{
		_flag = 0;
		if(_HitCenterTest(x,y))
			_flag |= Active_XY;
	}

	if(!_flag)
		return TRUE;
	else
	{
		_x = x;
		_y = y;
		_scrX = 0;
		_scrY = 0;
		_matInit = (*_data.matrix);
	}

	CGuiAgent_Dragger<1,0>::OnLButtonDown(x,y,flag);
	
	return FALSE;
}

BOOL CGuiAgent_MatrixRot::OnMouseMove(int x,int y,DWORD flag)
{
	PreEdit();

	CGuiView * view = GetGuiView();
	CGuiData_Res * data =(CGuiData_Res *)view->FindData("resource");

	if(!(_data.matrix)) 
		return TRUE;

	IRenderPort * rp = GetRP();
	assert(rp);
	
	if(_bInDrag)
		OnDrag(x,y);
	else
	{
		HitProbe probe;
		rp->CalcHitProbe(x,y,probe);
		i_math::vector3df outNormal;
		outNormal = probe.start - probe.end;

		if(_HitCircleTest(probe,_center,_nx,_radius,FALSE))
		{
			_flag = 0;
			_flag |= Active_X;
		}
		else if(_HitCircleTest(probe,_center,_ny,_radius,FALSE))
		{
			_flag = 0;
			_flag |= Active_Y;
		}
		else if(_HitCircleTest(probe,_center,_nz,_radius,FALSE))
		{
			_flag = 0;
			_flag |= Active_Z;
		}
		else if(_HitCircleTest(probe,_center,outNormal,_outdius,FALSE))
		{
			_flag = 0;
			_flag |= Active_OZ;
		}
		else
		{
			_flag = 0;
			if(_HitCenterTest(x,y))
				_flag |= Active_XY;
		}
	}

	if(_flag!=0)
		_bSel = TRUE;
	else
		_bSel = FALSE;

	return !_flag;
}
BOOL CGuiAgent_MatrixRot::Bind(MatrixEditData data)
{
	BOOL ret = CMatrixEditBase::Bind(data);
	
	return ret;
}
void CGuiAgent_MatrixRot::OnEndDrag(int x,int y)	
{
	_angleRot = 0;
	_angleRotOther =0;
	_matValue = (*_data.matrix);

	_funEndEditListener(this);	
}

void CGuiAgent_MatrixRot::_DrawDragInfo()
{
	IRenderPort *rp = GetRP();
	assert(rp);
	
	DrawFontArg fontArg;

	int x,y;
	rp->TransPos(_center,x,y);
	fontArg.SetLocation(x,y+(int)_radius);
	std::string s;
	
	i_math::vector3df axisX(1.0f,0.0f,0.0f);
	
	i_math::vector3df v0,nx ,ny ,nz,nsx,nsy,nsz;
	i_math::matrix43f matWld;

	matWld = _matInit*_data.matParent;
	matWld.makeInverse();
	v0.set(0,0,0);

	matWld.transformVect(_nx,nx);
	matWld.transformVect(_ny,ny);
	matWld.transformVect(_nz,nz);
	matWld.transformVect(v0,v0);
	nx -= v0;
	ny -= v0;
	nz -= v0;
	nx.normalize();
	ny.normalize();
	nz.normalize();
	
	matWld.transformVect(_nsx,nsx);
	matWld.transformVect(_nsy,nsy);
	matWld.transformVect(_nsz,nsz);
	nsx -= v0;
	nsy -= v0;
	nsz -= v0;
	nsx.normalize();
	nsy.normalize();
	nsz.normalize();
	
	i_math::quatf quatRot;
	i_math::matrix43f matTrans;
	float radian0 = _angleRot*(M_PI/180.0f);
	float radian1 = _angleRotOther*(M_PI/180.0f);
	
	if(_flag&Active_Z)
	{
		FormatString(s,"{F3}{S12}{N1}{C255,255,0} [0.00, 0.00, %.02f]",_angleRot);
		quatRot.fromAngleAxis(radian0,nz);
	}
	else if(_flag&Active_X)
	{	
		FormatString(s,"{F3}{S12}{N1}{C255,255,0} [%.02f, 0.00, 0.00]",_angleRot);
		quatRot.fromAngleAxis(radian0,nx);
	}
	else if(_flag&Active_Y)
	{	
		FormatString(s,"{F3}{S12}{N1}{C255,255,0} [0.00, %.02f, 0.00]",_angleRot);
		quatRot.fromAngleAxis(radian0,ny);
	}
	else if(_flag&Active_OZ)
	{
		FormatString(s,"{F3}{S12}{N1}{C255,255,0} [0.00, 0.00, %.02f]",_angleRot);
		quatRot.fromAngleAxis(-radian0,nsz);
	}
	else if(_flag&Active_XY)
	{	
		FormatString(s,"{F3}{S12}{N1}{C255,255,0} [%.02f, %.02f, 0.00]",_angleRot,_angleRotOther);
		i_math::quatf q0,q1;
		q0.fromAngleAxis(radian0,nsy);
		q1.fromAngleAxis(radian1,nsx);
		quatRot = q0*q1;
	}
	
	mat43from44(matTrans,quatRot.getMatrix());
	if(!matTrans.equalsIdentity())
	{	
		*_data.matrix = matTrans*_matInit;
		_funOnEditListener(this);
	}

	rp->DrawText(s.c_str(),fontArg);
}
void CGuiAgent_MatrixRot::_DrawDragArc()
{
	IRenderPort *rp = GetRP();
	assert(rp);

	int idx = 0;
	DWORD col;
	if(_flag&Active_X)
	{
		idx++;
		col = ColorAlpha(0xff0000,0x44);
	}
	if(_flag&Active_Y) 
	{
		idx++;
		col = ColorAlpha(0x00ff00,0x44);
	}
	if(_flag&Active_Z) 
	{
		idx++;
		col = ColorAlpha(0x0000ff,0x44);
	}
	if(idx!=1)
		return;
	
	DrawCirclePlane(rp,_dirLine.center,_dirLine.normal,_arg.radius,&(_dirLine.nradius),_angleRot,col,200);
}
void CGuiAgent_MatrixRot::OnDrag(int x,int y)
{

	CGuiView * view = GetGuiView();
	CGuiData_Res * data =(CGuiData_Res *)view->FindData("resource");
	Reps_Dummies *state =(Reps_Dummies *)data->FindState("Res_Dummies");
	
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
	nx = (float)(x-_x+_scrX);
	ny = (float)(y-_y+_scrY);

	if(_flag!=Active_XY)
	{
		IRenderPort *rp = GetRP();
		assert(rp);

		i_math::vector2df  vecCross,vecMove;
		int x0,y0,x1,y1;
		rp->TransPos(_dirLine.start,x0,y0);
		rp->TransPos(_dirLine.start+_dirLine.nvec,x1,y1);
		vecCross.set((float)(x1-x0),(float)(y1-y0));
		vecCross.normalize();

		vecMove.set((float)nx,(float)ny);
		float effect = vecMove.dotProduct(vecCross);   
		_angleRot = effect*_arg.speed;
	}
	else
	{
		_angleRot      = nx * _arg.speed;
		_angleRotOther = ny * _arg.speed;
	}
}









