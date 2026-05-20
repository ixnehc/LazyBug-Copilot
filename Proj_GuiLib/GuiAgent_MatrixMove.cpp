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

#include ".\GuiAgent_MatrixMove.h"

#include "spatialtester/spatialtester.h"

#include <assert.h>

#include "TransformInputDlg.h"

#include "TransformSettingsDlg.h"

#include "RenderSystem/IFont.h"

extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
extern float DrawDirLineTaper(IRenderPort * rp, const i_math::vector3df & point,const i_math::vector3df & dir,float len,DWORD colLine,DWORD colTaper,ShaderState * state= NULL);
extern void DrawQuadWire(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD *cols/*in out*/,float & width/* in out*/,float & height /* in out*/,ShaderState * state= NULL);
extern void DrawQuad(IRenderPort * rp, const i_math::vector3df & noraml,const i_math::vector3df &vecStart,const i_math::vector3df &pos ,DWORD col,float & width/* in out*/,float & height /* in out*/,ShaderState * state= NULL);

CAgent_MatrixMove::CAgent_MatrixMove(void)
{
	_flag = 0;
	_xcScreen = 0;
	_ycScreen = 0;
}

CAgent_MatrixMove::~CAgent_MatrixMove(void)
{

}

BOOL  CAgent_MatrixMove::OnLButtonUp(int x,int y,DWORD flag)
{	
	CGuiAgent_Dragger<1,0>::OnLButtonUp(x,y,flag);
	
	if(FALSE==_CheckWorkable())
		return TRUE;
	
	//正在托拽时为独占模式
	return !(_bSel&&_bInDrag);
}

void CAgent_MatrixMove::SetUIArg(MoveCtrlArg arg)
{
	_arg = arg;
}

void CAgent_MatrixMove::OnDlgEdit(BOOL bRelative,DWORD changeType,const i_math::vector3df& vecChange,i_math::matrix43f &matLocal)
{	
	if(!_data.matrix)
		return;

	i_math::matrix43f matInverse = (*_data.matrix)*_data.matParent;
	matInverse.makeInverse();
	
	//将世界空间的移动 反映到Local空间
	i_math::vector3df vecChangeInLocal;
	matInverse.rotateVect(vecChange,vecChangeInLocal);
	
	i_math::matrix43f matChange;
	matChange.setTranslation(vecChangeInLocal);

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
BOOL CAgent_MatrixMove::Bind(MatrixEditData &data)
{
	BOOL ret = CMatrixEditBase::Bind(data);
	if(!ret)
		return FALSE;

	_matValue = (*data.matrix);
	_matInit  = (*data.matrix);
	return TRUE;
}

BOOL CAgent_MatrixMove::OnDraw()
{
	if(FALSE==_CheckWorkable())
		return TRUE;

	_DrawAxisLine();
	_DrawAxisQuad();
	
	return TRUE;
}
/************************************************************************/
/* 没有Bind矩阵                             
/* 工作模式非当前
/* 被禁用
/* 回调函数未被设置
/************************************************************************/
BOOL CAgent_MatrixMove::_CheckWorkable() 
{
	if(!IsBind()||!_bWorkable||
		_data.modeedit!=EditMode_Move||
		_funOnEditListener.empty()||
		_funEndEditListener.empty())

		return FALSE;

	return TRUE;
}

BOOL CAgent_MatrixMove::OnMouseMove(int x,int y,DWORD flag)
{
	if(FALSE==_CheckWorkable())
		return TRUE;

	if(_bInDrag)
		OnDrag(x,y,flag);
	else
	{
		DWORD oldflag = _flag;

		_HitTest(x,y);
		if(_flag!=0)
			_bSel = TRUE;
		else
			_bSel = FALSE;

		if(oldflag!=_flag)
			_Redraw(FALSE);
	}
	return !_flag;
}

void CAgent_MatrixMove::OnEndDrag(int x,int y,DWORD flag)
{
	_funEndEditListener(this);
}
void CAgent_MatrixMove::_SnapCal(i_math::vector3df &off)
{
	i_math::matrix43f matWolrd = _matInit*_data.matParent;
	i_math::vector3df vecOld = matWolrd.getTranslation();
	i_math::vector3df vec = vecOld + off,vecSnap;
	vecSnap.x = GetTransformSettings().SnapMove(vec.x);
	vecSnap.y = GetTransformSettings().SnapMove(vec.y);
	vecSnap.z = GetTransformSettings().SnapMove(vec.z);
	off = vecSnap - vecOld;
}
void CAgent_MatrixMove::OnDrag(int x,int y,DWORD flag)
{
	i_math::matrix43f matTrans;
	i_math::matrix43f matInverse,matSpace,matWorld;
	
	matWorld = _matValue*_data.matParent;
	matWorld.getInverse(&matInverse);

	float ex,ey,ez;
	IRenderPort * rp = GetRP();

	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	_CalInitPos(probe,rp,ex,ey,ez,x,y);
	
	i_math::vector3df vs,ve,off;	
	vs.set(_ix,_iy,_iz);
	ve.set(ex,ey,ez);
	
	// 得到偏移量
	off = ve - vs;

	//计算沿着编辑空间的三轴向的投影距离
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
	
	// 除去沿着某个轴向的移动限制，得到最总的移动量
	if(_flag==Active_YZ||_flag==Active_Y||_flag==Active_Z)
		off -= ivx;
	if(_flag==Active_ZX||_flag==Active_Z||_flag==Active_X)
		off -= ivy;
	if(_flag==Active_XY||_flag==Active_X||_flag==Active_Y)
		off -= ivz;
	
	_SnapCal(off);

	//计算世界向量 在 局部空间中的表达
	matInverse.rotateVect(off,off);

	matTrans.setTranslation(off); 
	(*_data.matrix) = matTrans*_matInit;
	
	_matValue = (*_data.matrix);

	//检测该次与最近一次矩阵编辑是否发生变化 如果发生变化则向外通知
	static i_math::matrix43f matold;
	if(!matTrans.equals(matold))
	{
		_Redraw(FALSE);
		matold = matTrans;
		_funOnEditListener(this);
//		GetTransformDlg()->UpdateMatrix(_matValue);
	}
	

// 向量上的任何变化 与 空间无关。
// 向量上的偏移在任何空间上进行分解，最终的偏移任然不变。
// 放缩，旋转？
}
BOOL CAgent_MatrixMove::OnLButtonDown(int x,int y,DWORD flag)
{
	IRenderPort * rp = GetRP();
	assert(rp);

	//当复合键（Shift Ctrl）按下时不响应消息
	if(flag||FALSE==_CheckWorkable())
		return TRUE;
	
	//测试编辑状态
	_HitTest(x,y);

	//测得中心点的位置,和光标的位置
	if(TRUE){
		rp->TransPos(_center,_xcScreen,_ycScreen);
		_xCursor = x;
		_yCursor = y;
		
		i_math::vector3df to;
		switch(_flag){
			case Active_X:
				to = _center + _nx;
				break;
			case Active_Y:
				to = _center + _ny;
				break;
			case Active_Z:
				to = _center + _nz;
				break;
		}

		int x1 = 0,y1 = 0;
		rp->TransPos(to,x1,y1);
		_vecInitAxisSc.x = float(x1) - float(_xcScreen);
		_vecInitAxisSc.y = float(y1) - float(_ycScreen);
		_vecInitAxisSc.normalize();
		_centerInit = _center;
	}
	
	//如果某一编辑模式被激活，开始工作
	if(_flag)
	{
		//初始化托拽状态
		HitProbe probe;
		rp->CalcHitProbe(x,y,probe);


		_CalInitPos(probe,rp,_ix,_iy,_iz,x,y,TRUE);
		_matInit = (*_data.matrix);
		
		//初始化鼠标控制状态
		_bInDrag=TRUE;
		OnBeginDrag(x,y,flag);
		OccupyFocus(OpType_Mouse);
		_bSel = TRUE;
		
		_funPerEditListener(this);

		return FALSE;
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
	ScaleLen(rp,_center,lenAxis);

	lenTaper = DrawDirLineTaper(rp,_center,_nx,lenAxis,colX,_arg.colX,&_shaderState);
	lenTaper = DrawDirLineTaper(rp,_center,_ny,lenAxis,colY,_arg.colY,&_shaderState);
	lenTaper = DrawDirLineTaper(rp,_center,_nz,lenAxis,colZ,_arg.colZ,&_shaderState);
	
	_lenAxis = lenAxis + lenTaper;
	
	colQx = (_flag&Active_XY)?_arg.colActive:_arg.colX;
	colQy = (_flag&Active_XY)?_arg.colActive:_arg.colY;
	float w ,h;
	DWORD  cols[4];
	DWORD colNone = ColorAlpha(0x000000,0x00);
	const float ratio = 0.3f;
	w = _lenAxis*ratio;
	h = - _lenAxis*ratio;
	cols[0] = colX;
	cols[1] = colQy;
	cols[2] = colY;
	cols[3] = colQx;
	DrawQuadWire(rp,_nz,_nx,_center,cols,w,h,&_shaderState);

	colQy = (_flag&Active_YZ)?_arg.colActive:_arg.colY;
	colQz = (_flag&Active_YZ)?_arg.colActive:_arg.colZ;
	w = _lenAxis*ratio;
	h = -_lenAxis*ratio;
	cols[0] = colY;
	cols[1] = colQz;
	cols[2] = colZ;	
	cols[3] = colQy;
	DrawQuadWire(rp,_nx,_ny,_center,cols,w,h,&_shaderState);
	
	colQx = (_flag&Active_ZX)?_arg.colActive:_arg.colX;
	colQz = (_flag&Active_ZX)?_arg.colActive:_arg.colZ;
	w =  _lenAxis*ratio;
	h = - _lenAxis*ratio;
	cols[0] = colZ;
	cols[1] = colQx;
	cols[2] = colX;
	cols[3] = colQz;
	DrawQuadWire(rp,_ny,_nz,_center,cols,w,h,&_shaderState);

	_w = w;
}

BOOL CAgent_MatrixMove::_HitTestQuad(i_math::line3df &line,i_math::vector3df &nx,i_math::vector3df &ny,float &dist)
{
	/*		ny------nx,ny
			|		|
			|		|
		_center-----nx
	*/
	IRenderPort * rp = GetRP();
	
	i_math::pos2di pt;
	_GetCursorPos(pt);
	i_math::vector3df ptCursor(float(pt.x),float(pt.y),0);

	i_math::vector3df xLen = nx,yLen = ny;
	xLen.setLength(_w);
	yLen.setLength(_w);
	
	//矩形的四个顶点
	i_math::vector3df p[4],activePos;
	p[0] = _center + xLen + yLen;
	p[1] = _center + yLen;
	p[2] = _center;
	p[3] = _center + xLen;
	
	//投影到屏幕空间
	i_math::vector3df sp[4];
	for(int i = 0;i<4;i++){
		int x ,y;
		rp->TransPos(p[i],x,y);
		sp[i].set(float(x),float(y),0);
	}
	activePos = (sp[0]);

	i_math::triangle3df tri;
	i_math::vector3df vecIntersec;
	i_math::vector2di p0,p1;

	bool bIntersec = false;
	tri.set(sp[2],sp[3],sp[0]);
	
	//几乎在一条直线上
	if((tri.getArea()>1.0f)&&!(bIntersec=tri.isPointInside(ptCursor))){
		tri.set(sp[0],sp[1],sp[2]);
		assert(!bIntersec);
		if(tri.getArea()>1.0f)
			bIntersec = tri.isPointInside(ptCursor);
	}

	dist = -1.0f;

	//如果鼠标的位置在四边形到屏幕投影区域内  返回鼠标位置到激活点的距离
	if(bIntersec){
		dist = (float)ptCursor.getDistanceFrom(activePos);
		return TRUE;
	}

	return FALSE;
}

float CAgent_MatrixMove::_DistLine2Pt(i_math::vector2di &sl,i_math::vector2di &el,i_math::vector2di &pt)
{
	i_math::line2df line;
	line.setLine(float(sl.x),float(sl.y),float(el.x),float(el.y));
	i_math::vector2df ptProj(float(pt.x),float(pt.y));

	i_math::vector2df closePt = line.getClosestPoint(ptProj);
	float d = (float)ptProj.getDistanceFrom(closePt);

	return d;
}
DWORD CAgent_MatrixMove::_HitTestAxis(i_math::line3df &line,float &dist)
{
	IRenderPort * rp = GetRP();

	i_math::pos2di pt;
	_GetCursorPos(pt);
	
	i_math::vector2di ax,ay,az,ptCursor(pt.x,pt.y),po;
	i_math::vector3df vx,vy,vz;
	
	vx = _center + _lenAxis*_nx;
	vy = _center + _lenAxis*_ny;
	vz = _center + _lenAxis*_nz;
	
	rp->TransPos(vx,ax.x,ax.y);
	rp->TransPos(vy,ay.x,ay.y);
	rp->TransPos(vz,az.x,az.y);
	rp->TransPos(_center,po.x,po.y);
	
	DWORD flag = 0;
	float distMin = 999999.0f,d = 0;

	if(_DistLine2Pt(po,ax,ptCursor)<_arg.activeRange){
		d = (float)ax.getDistanceFrom(ptCursor);
		if(d<distMin){
			distMin = d;
			flag = Active_X;
		}
	}

	if(_DistLine2Pt(po,ay,ptCursor)<_arg.activeRange){
		d = (float)ay.getDistanceFrom(ptCursor);
		if(d<distMin){
			distMin = d;
			flag = Active_Y;
		}
	}

	if(_DistLine2Pt(po,az,ptCursor)<_arg.activeRange){
		d = (float)az.getDistanceFrom(ptCursor);
		if(d<distMin){
			distMin = d;
			flag = Active_Z;
		}
	}
	
	dist = (flag)?distMin:-1.0f;

	return flag;
}
DWORD CAgent_MatrixMove::_HitTest3Quad(i_math::line3df &line,float &dist)
{
	DWORD flag = 0;

	float distMin = 999999.0f,d;

	if(_HitTestQuad(line,_nx,_ny,d)&&d<distMin){
		distMin = d;
		flag = Active_XY;
	}
	if(_HitTestQuad(line,_ny,_nz,d)&&d<distMin){
		distMin = d;
		flag = Active_YZ;
	}
	if(_HitTestQuad(line,_nz,_nx,d)&&d<distMin){
		distMin = d;
		flag = Active_ZX;
	}

	return flag;
}
void CAgent_MatrixMove::_HitTest(int x,int y)
{		
	//重置编辑状态
	_flag = 0;
	
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	float dist3p = 0, distAxis = 0;

	DWORD flag3p   = _HitTest3Quad(probe,dist3p);  // 测试轴平面
	DWORD flagAxis = _HitTestAxis(probe,distAxis); // 测试轴线段
	
	//根据测试结果 设置编辑状态	:优先选择轴平面 其次选取轴线段
	_flag = (flag3p)?flag3p:flagAxis;
}
void CAgent_MatrixMove::_DrawAxisQuad()
{	
	IRenderPort * rp = GetRP();
	float w,h;
	const float ratio = 0.3f;
	w = _lenAxis*ratio;
	h = -_lenAxis*ratio;
	
	if(_flag&Active_XY)
		DrawQuad(rp,_nz,_nx,_center,_arg.colQuad,w,h,&_shaderState);
	else if(_flag&Active_YZ)
		DrawQuad(rp,_nx,_ny,_center,_arg.colQuad,w,h,&_shaderState);
	else if(_flag&Active_ZX)
		DrawQuad(rp,_ny,_nz,_center,_arg.colQuad,w,h,&_shaderState);
}

void CAgent_MatrixMove::_Cal3DPos(IRenderPort * rp,i_math::vector3df &pos,i_math::vector3df &normal,int x,int y)
{
	i_math::recti rc;
	rp->GetRect(rc);
	float w = float(rc.getWidth());
	float h = float(rc.getHeight());

	ICamera * camera = rp->GetCamera();
	i_math::vector3df center = pos;

	// a position at near view clip plane.
	i_math::matrix44f matViewProj;
	camera->GetViewProj(matViewProj);
	matViewProj.makeInverse();

	i_math::vector3df eyePos;
	camera->GetEyePos(eyePos);

	pos.x = 2.0f*float(x - rc.Left())/w - 1.0f;
	pos.y = 2.0f*float(rc.Bottom() - y)/h - 1.0f;
	pos.z = 0;

	matViewProj.transformVect(pos);
	pos -= eyePos;
	pos.setLength(1000.0f);

	HitProbe probe;
	probe.start = eyePos;
	probe.end = eyePos + pos ;
	
	i_math::plane3df plane;
	plane.setPlane(center,normal);
	plane.getIntersectionWithLimitedLine(probe.start,probe.end,pos);
}

i_math::vector3df CAgent_MatrixMove::_GetCalNoraml(i_math::vector3df &eyeDir)
{
	i_math::vector3df normal;
	
	i_math::vector3df e = eyeDir;
	float ex = abs(eyeDir.x);
	float ey = abs(eyeDir.y);
	float ez = abs(eyeDir.z);
	float ex2 = e.x*e.x;
	float ey2 = e.y*e.y;
	float ez2 = e.z*e.z;
		
	//与轴平面相交的情况
	i_math::vector3df n[7];
	int nPlanes = 1;

	if(_flag==Active_XY||
		_flag==Active_YZ||
		_flag==Active_ZX)
	{	
		n[0] = _nx + _ny;
		n[1] = _nx - _ny;
		n[2] = _ny + _nz;
		n[3] = _ny - _nz;
		n[4] = _nz + _nx;
		n[5] = _nz - _nx;

		switch(_flag){
			case Active_YZ:
			{
				n[0] = _nx;
				n[1] = _nx+_ny;
				n[2] = _nx-_ny;
				nPlanes = 3;
				break;
			}
			case Active_ZX:
			{
				n[0] = _ny;
				nPlanes = 1;
				break;
			}
			case Active_XY:
			{
				n[0] = _nz;
				nPlanes = 1;
				break;
			}
			default:
				break;
			}

		for(int i = 0;i<nPlanes;i++)
			n[i].normalize();

		float d_min = -999999.0f;
		int i_min = -1;
		for(int i = 0;i<nPlanes;i++){
			float d = n[i].dotProduct(eyeDir);
			d = abs(d);
			if(d>d_min){
				d_min = d;
				i_min = i;
			}
		}

		assert(i_min>=0);
		normal = n[i_min];
	}

	normal.normalize();

	return normal;
}

extern BOOL GetProjScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf,i_math::matrix43f &matScale);
void CAgent_MatrixMove::_CalInitPos(HitProbe &line,IRenderPort * rp,float &x,float &y,float &z,int sx,int sy,BOOL bInit )
{	
	i_math::vector3df center,vecInter;

	ICamera * cam = rp->GetCamera();
	i_math::vector3df eyePos,eyeDir;
	float fNear,fFar;
	cam->GetEyePos(eyePos);
	cam->GetEyeDir(eyeDir);
	cam->GetNearFar(fNear,fFar);

	//记录起始的位置
	if(bInit)
	{
		_ix = center.x;
		_iy = center.y;
		_iz = center.z;
	}
	
	vecInter = _center;
	i_math::vector3df vecAxis,vecAxis2;
	if(_flag==Active_X) { vecAxis = _nx; vecAxis2 = _nz; }
	if(_flag==Active_Y) { vecAxis = _ny; vecAxis2 = _nx; }
	if(_flag==Active_Z) { vecAxis = _nz; vecAxis2 = _ny; }

	switch(_flag)
	{
	case Active_X:
	case Active_Y:
	case Active_Z:
		{
			i_math::vector2df vecCusor;
			vecCusor.x = float(sx) - float(_xCursor);
			vecCusor.y = float(sy) - float(_yCursor);
			float d = vecCusor.dotProduct(_vecInitAxisSc);
			
			i_math::vector3df vecAxisAbs;
			if(d<0){
				vecAxisAbs = - vecAxis;
				d = -d;
			}
			else
				vecAxisAbs = vecAxis;
			
			float len0 = 0,len1 = 200.0f,dp = 0;
			ICamera * cam = rp->GetCamera();
			i_math::volumeCvxf vol;
			i_math::vector3df eyeDir;
			cam->GetEyeDir(eyeDir);
			cam->GetViewFrustum(vol);
			float dN = vol.planes[0].getDistanceTo(_centerInit);
			float dF = vol.planes[1].getDistanceTo(_centerInit);
			if(vecAxisAbs.dotProduct(eyeDir)>0)
				len1 = min(abs(dF),len1);
			else
				len1 = min(abs(dN),len1);
		
			int count = 0;
			i_math::vector3df p;
			while(1){
				float l = (len0 + len1)/2.0f;
				p = _centerInit + l*vecAxisAbs;
				int xp = 0,yp = 0;
				rp->TransPos(p,xp,yp);
				dp = float((xp- _xcScreen)*(xp- _xcScreen) + (yp - _ycScreen)*(yp - _ycScreen));
				dp = sqrtf(dp);
				if(abs(dp-d)<1)
					break;
				else{
					if(dp>d)
						len1 = l;
					else
						len0 = l;
				}
				if(len1-len0<0.001f)
					break;
				
				if(count>40)
					break;
				++count;
			}
			vecInter = p;
			break;
		}
	case Active_XY:
	case Active_YZ:
	case Active_ZX:
		{
			i_math::vector3df normal = _GetCalNoraml(eyeDir);
			_Cal3DPos(rp,vecInter,normal,sx,sy);
			// 如果物体被移动到某个距离是不能再被移动。
			ICamera * cam = rp->GetCamera();
			i_math::volumeCvxf vol;
			cam->GetViewFrustum(vol);
			for(int i = 0;i<vol.nPlanes;++i){
				if(vol.planes[i].getDistanceTo(vecInter)>=0){
					vecInter = _center;
					break;
				}
			}
			break;
		}
	default:
		break;
	}
	x = vecInter.x;
	y = vecInter.y;
	z = vecInter.z; 
}


