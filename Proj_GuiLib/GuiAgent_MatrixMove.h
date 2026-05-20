#pragma once

#include "../Interfaces/RenderSystem/IRenderSystem.h"
#include "GuiEditor.h"
#include "matrixedit_base.h"
#include "math/imath_all.h"

struct MoveCtrlArg
{
	MoveCtrlArg()
	{
		colX = ColorAlpha(0xff0000,0xff);
		colY = ColorAlpha(0x00ff00,0xff);
		colZ = ColorAlpha(0x0000ff,0xff);
		colActive = ColorAlpha(0xffff00,0xff);
		colQuad =  ColorAlpha(0xffff00,0x66);
		lenAxis = 60.0f;
		activeRange = 20.0f;
		mx = my = mz = 500.0f;
	}
	float lenAxis;
	float activeRange;
	float mx,my,mz;
	DWORD colX ,colY,colZ,colActive,colQuad;
};

/************************************************************************/
/*编辑矩阵：移动。
/*目标：基本的移动功能，轴向 平面内。方便的选取移动的方式。稳定的工作。Snap功能
/*数值输入的模式工作
/************************************************************************/

class CAgent_MatrixMove :public CGuiAgent_Dragger<1,0> ,public CMatrixEditBase
{

public:

	CAgent_MatrixMove(void);
	~CAgent_MatrixMove(void);

public:

	virtual const char * getClassName() {return "CAgent_MatrixMove";}
	virtual BOOL Bind(MatrixEditData &data);
	virtual BOOL OnDraw();
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag)	;
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	void SetUIArg(MoveCtrlArg arg);
	
	virtual void OnDlgEdit(BOOL bRelative,DWORD changeType,const i_math::vector3df& vecChange,i_math::matrix43f &matLocal);
	
protected:

	void _DrawAxisLine(); //in world space.
	void _DrawAxisQuad();
	void _HitTest(int x,int y);
	void _CalInitPos(HitProbe &vecHit,IRenderPort * rp,float &x,float &y,float &z,int sx,int sy,BOOL bInit = FALSE);
	void _Cal3DPos(IRenderPort * rp,i_math::vector3df &pos,i_math::vector3df &normal,int x,int y);
	i_math::vector3df _GetCalNoraml(i_math::vector3df &eyeDir);
	BOOL _CheckWorkable();
	
	//以_center为起点，轴向为nx,ny构成四边形  判断与直线相交的情况，dist返回与激活点的距离
	BOOL _HitTestQuad(i_math::line3df &line,i_math::vector3df &nx,i_math::vector3df &ny,float &dist);
	//测试与轴向相交的情况 ,返回为当前激活的标志(例如：Active_X)，dist返回与相交的坐标轴的激活点距离
	DWORD _HitTestAxis(i_math::line3df &line,float &dist); 
	//测试与轴平面相交的情况	
	DWORD _HitTest3Quad(i_math::line3df &line,float &dist);
	float _DistLine2Pt(i_math::vector2di &sl,i_math::vector2di &el,i_math::vector2di &pt);
	
	void _SnapCal(i_math::vector3df &off);
private:

	DWORD _flag;  //ative flag ,indicate which axis direction is active.
	MoveCtrlArg _arg;
	i_math::vector3df _center;
	i_math::vector3df _nx,_ny,_nz;
	float  _lenAxis;
	float  _lenQuad;
	float _ix,_iy,_iz;
	float _w;

	i_math::matrix43f _matInit,_matSpace,_matFromWld;
	int _xcScreen,_ycScreen,_xCursor,_yCursor;
	
	i_math::vector2df _vecInitAxisSc;
	i_math::vector3df _centerInit;
};
