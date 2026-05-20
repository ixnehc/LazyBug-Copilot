#pragma once

#include "math/imath_all.h"
#include "../Interfaces/RenderSystem/IRenderSystem.h"
#include "GuiEditor.h"
#include "matrixedit_base.h"
#include "fastdelegate/FastDelegate.h"

struct RotCtrlArg
{
	RotCtrlArg()
	{
		radius = 0.5f;
		lencs  = 10.0f;
		nSeg   = 200;
		activeRange = 3.0f;
		lenDirline = 10.5f;
		speed =0.7f;
		colActive = ColorAlpha(0xffff00,0xff);
		colSel = ColorAlpha(0x666666,0x66);
		colCenterLine = ColorAlpha(0x666666,0xff);
	}
	float radius;  // radius of edit sphere
	float lencs;  //cross pointer len
	DWORD nSeg;		
	float activeRange; 
	float lenDirline;
	float speed;
	DWORD colActive;
	DWORD colSel;
	DWORD colCenterLine;
};


class CGuiAgent_MatrixRot : public CGuiAgent_Dragger<1,0> ,public CMatrixEditBase
{
public:
	CGuiAgent_MatrixRot(void)
	{
		_flag = 0;
		_angleRot = 0;
	}
	~CGuiAgent_MatrixRot(void)
	{
	}
struct DirectLine
{
	i_math::vector3df normal;
	i_math::vector3df nvec;
	i_math::vector3df nradius;
	i_math::vector3df start;
	i_math::vector3df center;
};

const char * getClassName()
{
	return "CGuiAgent_MatrixRot";
}

public:
	void SetUIArg(RotCtrlArg arg);

//override
public:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

	virtual void OnEndDrag(int x,int y)	;
	virtual void OnDrag(int x,int y);

	virtual BOOL Bind(MatrixEditData data);
	
private:
	void _DrawCross();
	void _DrawArc();
	void _DrawDirLine();
	void _DrawDragInfo();
	void _DrawDragArc();
	void _DrawCenterArc();
	

	BOOL _HitCircleTest(HitProbe & probe, i_math::vector3df &center ,i_math::vector3df &normal,float radius,BOOL bUpdate);
	BOOL _HitCenterTest(int x,int y);

	//member
protected:
	RotCtrlArg _arg;
	
	DWORD _flag;
	float _radius;
	float _outdius;
	float _innerdius;
	
	DirectLine  _dirLine;

	int _x , _y;
	int _scrX,_scrY;
	float _angleRot;
	float _angleRotOther;
	i_math::vector3df _center;
	i_math::vector3df _nx,_ny,_nz;
	i_math::vector3df _nsx,_nsy,_nsz;

	i_math::matrix43f _matInit;
};

