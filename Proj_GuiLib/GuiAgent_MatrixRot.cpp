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

#include "RenderSystem/IFont.h"
#include "RenderSystem/IRenderPort.h"


#include ".\GuiAgent_MatrixRot.h"

#include "stringparser/stringparser.h"

#include "GuiEditor_res.h"

#include "DummiesEditPanel.h"

#include "Log/LogFile.h"

#include "TransformInputDlg.h"

#include "TransformSettingsDlg.h"

#define M_PI       3.14159265358979323846f
extern float DrawCircleWire(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius ,DWORD col,DWORD nSeg=100,i_math::vector3df *cilpVec=NULL);
extern void DrawCirclePlane(IRenderPort * rp,i_math::vector3df &center,i_math::vector3df &normal,float radius,i_math::vector3df *start,float angle,DWORD col,DWORD nSeg);
extern void DrawDirLine(IRenderPort *rp ,i_math::vector3df &normal,i_math::vector3df &nvec,i_math::vector3df & point,float len ,float dirLen,DWORD col);
extern BOOL GetProjScaleMask(IRenderPort *rp,i_math::matrix43f &matTrans,i_math::matrix43f &matScale);
extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);

void CGuiAgent_MatrixRot::SetUIArg(RotCtrlArg arg)
{
	_arg = arg;
}

BOOL CGuiAgent_MatrixRot::OnDraw()
{
	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Rot)
		return TRUE;	

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

	return TRUE;
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
	i_math::vector3df dir = (_angleRot<0)?-_dirLine.nvec:_dirLine.nvec; //切线方向

	DrawDirLine(rp,_dirLine.normal,dir,_dirLine.start,len,0.2f,_arg.colActive);
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

	GetProjScaleMask(rp,matTrans,matScale);
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
		rp->DrawText("{F:1}{S:12}{C:255,64,0}x",fontArg);
	}
	else
		Draw_InvalidCross(rp,vx,"{F:1}{S:12}{C:64,64,64}x");

	rp->TransPos(vy,x,y);
	fontArg.SetLocation(x+4,y-4);
	if(_flag&Active_Y)
	{
		rp->Line(v0,vy,ColorAlpha(0x7fff00,0xff));
		rp->DrawText("{F:1}{S:12}{C:128,255,0}",fontArg);
	}
	else
		Draw_InvalidCross(rp,vy,"{F:1}{S:12}{C:64,64,64}y");

	rp->TransPos(vz,x,y);
	fontArg.SetLocation(x+4,y-4);
	if(_flag&Active_Z)
	{
		rp->Line(v0,vz,ColorAlpha(0x007fff,0xff));
		rp->DrawText("{F:1}{S:12}{C:0,128,255}z",fontArg);
	}
	else
		Draw_InvalidCross(rp,vz,"{F:1}{S:12}{C:64,64,64}z");
}
void CGuiAgent_MatrixRot:: _DrawCenterArc()
{
	IRenderPort * rp = GetRP();
	assert(rp);

	i_math::vector3df eyeDir;
	ICamera * cam = rp->GetCamera();
	cam->GetEyeDir(eyeDir);

	_innerdius = DrawCircleWire(rp,_center,-eyeDir,_arg.radius,_arg.colCenterLine,200);

	if(_flag==Active_XY)
		DrawCirclePlane(rp,_center,-eyeDir,_arg.radius,NULL,360.0f,_arg.colSel,200);
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

	i_math::vector3df eyeDir;
	ICamera *cam = rp->GetCamera();
	cam->GetEyeDir(eyeDir);
	_outdius = DrawCircleWire(rp,_center,-eyeDir,1.4f*_arg.radius,colOZ,200);//绘制平行与屏幕的外圆
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
void CGuiAgent_MatrixRot::_GetExTestPos(i_math::vector3df * testPos,i_math::vector3df &center,ICamera *cam,float len)
{
	assert(cam);
	i_math::matrix44f matView,matInv;
	cam->GetView(matView);
	matView.getInverse(matInv);

	i_math::vector3df cen = center;
	matView.transformVect(cen);

	const float pi = 3.14159265358979323846f/4.0f;

	float angle = 0;
	for(int i = 0;i<8;i++)
	{
		testPos[i].x = len*cosf(angle);
		testPos[i].y = len*sinf(angle);
		testPos[i].z = 0;
		testPos[i] += cen;
		matInv.transformVect(testPos[i]);

		angle += pi;
	}
}
BOOL CGuiAgent_MatrixRot::_HitCircleTest(HitProbe & probe,i_math::vector3df &center ,i_math::vector3df &normal,float radius,BOOL bUpdate,BOOL bClip/* = TRUE*/)
{
	IRenderPort *rp = GetRP();
	assert(rp);

	const float tolerance = 0.001f;
	
	float rangeActive = _arg.activeRange;
	ScaleLen(rp,_center,rangeActive);// 相对屏幕空间的长度
	normal.normalize();

	i_math::plane3df planeClip;
	i_math::vector3df eyeDir;
	ICamera *cam = rp->GetCamera();
	cam->GetEyeDir(eyeDir);
	planeClip.setPlane(center,-eyeDir);// 从屏幕空间的长度得到世界空间的长度
	
	i_math::vector3df interCen;
	planeClip.getIntersectionWithLine(probe.start,probe.end-probe.start,interCen);
	

	i_math::vector3df testPos[8],cv,argCv;
	_GetExTestPos(testPos,interCen,cam,rangeActive);
	
	// 测试8个点是否与圆形线框的相交情况
	i_math::plane3df  plane;
	plane.setPlane(center,normal);
	int n = 0;
	int n1 = 0;
	for(int i = 0;i<8;i++)
	{
		i_math::vector3df  t1; 
		if(plane.getIntersectionWithLine(probe.start,probe.start-testPos[i],cv))
		{
			float dist = (float)cv.getDistanceFrom(center);
			if(abs(dist-radius)< rangeActive)
				continue;
			n1++;
			t1 = cv - center;
			t1.setLength(radius);
			t1 = t1 + center;  //得到边界的相交点位置
			if(bClip==FALSE||bClip&&planeClip.getDistanceTo(t1)>0){
				argCv += t1;
				n++;
			}
		}
	}

	if(n>0&&n1<8)
	{
		_dirLine.start = argCv/(float)n;  //得到边界的相交点位置
		if(bUpdate) //是否需要更新由于绘制的切线。
		{
			i_math::vector3df  t1;
			t1 = _dirLine.start - center;
			_dirLine.nradius = t1;     // 副法线向量
			_dirLine.normal = normal;  // 法线向量
			_dirLine.nvec = t1.crossProduct(normal);//中心位置
			_dirLine.center = center;  // 切线向量
		}

		return TRUE;
	}
	
	return FALSE;

}

BOOL CGuiAgent_MatrixRot::OnLButtonUp(int x,int y,DWORD flag)
{	
	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);

	if(!_bSel)
		return TRUE;
	
	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Rot)
		return TRUE;	

	return !(_bSel);
}

BOOL CGuiAgent_MatrixRot::OnLButtonDown(int x,int y,DWORD flag)
{
	//Added By Chenxi
	if (flag)
		return TRUE;
	//

	if(_bInDrag)	// for mouse lost
	{
		_HitTest(x,y);
		_bSel = (_flag!=0);
	}
	if(!_bSel)
	{
		_bInDrag = FALSE;
		return TRUE;
	}

	PreEdit(EditMode_Rot);

	IRenderPort *rp = GetRP();
	assert(rp);

	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	i_math::vector3df  eyeDir;
	ICamera * cam = rp->GetCamera();
	cam->GetEyeDir(eyeDir);

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
	else if(_HitCircleTest(probe,_center,-eyeDir,_outdius,TRUE,FALSE))
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

	if(_bSel&&!flag)
	{
		_bInDrag=TRUE;
		OnBeginDrag(x,y,flag);
		OccupyFocus(OpType_Mouse);
	}

	return !_bSel;
}
void CGuiAgent_MatrixRot::_HitTest(int x,int y)
{
	IRenderPort * rp = GetRP();
	assert(rp);

	DWORD oldflag  = _flag;

	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	
	i_math::vector3df  eyeDir;
	ICamera * cam = rp->GetCamera();
	cam->GetEyeDir(eyeDir);

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
	else if(_HitCircleTest(probe,_center,-eyeDir,_outdius,FALSE,FALSE))
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

	if(oldflag!=_flag)
		_Redraw(FALSE);

}
BOOL CGuiAgent_MatrixRot::OnMouseMove(int x,int y,DWORD flag)
{
	static int oldx,oldy;
	if(oldx==x&&oldy==y)
		return TRUE;
	else
	{
		oldx = x;
		oldy = y;
	}

	if(!IsBind()||!_bWorkable||_data.modeedit!=EditMode_Rot)
		return TRUE;	


	if(_bInDrag)
		OnDrag(x,y,flag);
	else
	{
		_HitTest(x,y);
	}

	if(_flag!=0)
		_bSel = TRUE;
	else
		_bSel = FALSE;
	
	return !_bSel;
}

void CGuiAgent_MatrixRot::OnEndDrag(int x,int y,DWORD flag)	
{
	_angleRot = 0;
	_angleRotOther =0;
	_matValue = (*_data.matrix);

	_Redraw(FALSE);

	_funEndEditListener(this);	
}
void CGuiAgent_MatrixRot::OnDlgEdit(BOOL bRelative,DWORD changeType,const i_math::vector3df& vecChange,i_math::matrix43f &matLocal)
{
	if(!_data.matrix)
		return;

	i_math::matrix43f matRot;
	matRot.setRotationXYZ(vecChange);
	
	i_math::vector3df vecRot;
	matRot.getRotationXYZ(&vecRot);

	i_math::vector3df trans = _data.matrix->getTranslation();
	i_math::vector3df scale = _data.matrix->getScale();
	
	_Redraw(FALSE);
	
	if(changeType==CTransformInputDlg::Change_Begin&&!_funPerEditListener.empty()){
		matLocal = *(_data.matrix);
		_funPerEditListener(this);
	}
	else{
		if(bRelative){
			_matValue = (*_data.matrix)*matRot;
		}
		else{
			_matValue.setScale(scale);
			_matValue = matRot;
		}
		
		i_math::vector3df vecRot;
		_matValue.getRotationXYZ(&vecRot);

		//添加偏移量
		_matValue.m03 = trans.x;
		_matValue.m13 = trans.y;
		_matValue.m23 = trans.z;
		matLocal = _matValue;
		*_data.matrix = matLocal;

		if(changeType==CTransformInputDlg::Change_On&&!_funOnEditListener.empty())
			_funOnEditListener(this);

		if(changeType==CTransformInputDlg::Change_End&&!_funEndEditListener.empty())
			_funEndEditListener(this);
	}
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
	
	if(_flag&Active_Z)
	{
		FormatString(s,"{F:1}{S:12}{C:255,255,0} [0.00, 0.00, %.02f]",_angleRot);
	}
	else if(_flag&Active_X)
	{	
		FormatString(s,"{F:1}{S:12}{C:255,255,0} [%.02f, 0.00, 0.00]",_angleRot);
	}
	else if(_flag&Active_Y)
	{	
		FormatString(s,"{F:1}{S:12}{C:255,255,0} [0.00, %.02f, 0.00]",_angleRot);
	}
	else if(_flag&Active_OZ)
	{
		FormatString(s,"{F:1}{S:12}{C:255,255,0} [0.00, 0.00, %.02f]",_angleRot);
	}
	else if(_flag&Active_XY)
	{	
		FormatString(s,"{F:1}{S:12}{C:255,255,0} [%.02f, %.02f, 0.00]",_angleRot,_angleRotOther);
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
BOOL CGuiAgent_MatrixRot::Bind(MatrixEditData &data)
{
	BOOL ret =  CMatrixEditBase::Bind(data);
	if(!ret)
		return FALSE;

	_matValue = (*_data.matrix);
	_matInit = (*_data.matrix);

	return TRUE;
}
void CGuiAgent_MatrixRot::OnDrag(int x,int y,DWORD flag)
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

	float ox ,oy ;
	ox = (float)(x-_x+_scrX);
	oy = (float)(y-_y+_scrY);

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

		vecMove.set((float)ox,(float)oy);
		float effect = vecMove.dotProduct(vecCross);   
		_angleRot = effect*_arg.speed;
	}
	else
	{
		_angleRot      = ox * _arg.speed;
		_angleRotOther = oy * _arg.speed;
	}	
	
	_angleRot = GetTransformSettings().SnapRotate(_angleRot);
	_angleRotOther = GetTransformSettings().SnapRotate(_angleRotOther);

	IRenderPort *rp = GetRP();
	assert(rp);

	rp->TransPos(_center,x,y);
	i_math::vector3df v0,nx ,ny ,nz,nsx,nsy,nsz;
	i_math::matrix43f matWld;

	matWld = _matValue*_data.matParent;
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

	if(_flag&Active_Z)    // y-->x
	{
		quatRot.fromAngleAxis(radian0,nz);
		mat43from44(matTrans,quatRot.getMatrix());
	}
	else if(_flag&Active_X)//z-->y
	{	
		quatRot.fromAngleAxis(radian0,nx);
		mat43from44(matTrans,quatRot.getMatrix());
	}
	else if(_flag&Active_Y)//x-->z 
	{	
		quatRot.fromAngleAxis(radian0,ny);
		mat43from44(matTrans,quatRot.getMatrix());
	}
	else if(_flag&Active_OZ)
	{
		quatRot.fromAngleAxis(-radian0,nsz);
		mat43from44(matTrans,quatRot.getMatrix());
	}
	else if(_flag&Active_XY)
	{	
		i_math::quatf q0,q1;
		q0.fromAngleAxis(radian0,nsy);
		q1.fromAngleAxis(radian1,nsx);
		quatRot = q0*q1;
		mat43from44(matTrans,quatRot.getMatrix());
	}

	if(!matTrans.equalsIdentity())
	{	
		*_data.matrix = matTrans*_matInit;
	}
	
	if(radian0 != 0||radian1!=0)
	{
		_funOnEditListener(this);
//		GetTransformDlg()->UpdateMatrix(*_data.matrix);
		_Redraw(FALSE);
	}
}


