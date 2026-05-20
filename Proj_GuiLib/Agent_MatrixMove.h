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
		lenAxis = 12.0f;
		activeRange = 2.0f;
		mx = my = mz = 100.0f;
	}
	float lenAxis;
	float activeRange;
	float mx,my,mz;
	DWORD colX ,colY,colZ,colActive,colQuad;
};

class CAgent_MatrixMove :public CGuiAgent_Dragger<1,0> ,public CMatrixEditBase
{
public:
	CAgent_MatrixMove(void);
	~CAgent_MatrixMove(void);

public:
	virtual const char * getClassName() {return "CAgent_MatrixMove";}
	virtual BOOL Bind(MatrixEditData data);
	virtual BOOL OnDraw();
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y)	;
	virtual void OnDrag(int x,int y);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);

	void SetUIArg(MoveCtrlArg arg);
protected:
	void _DrawAxisLine(); //in world space.
	void _DrawAxisQuad();
	void _HitTest(int x,int y);
	void _CalInitPos(HitProbe &vecHit,float &x,float &y,float &z,BOOL bInit = FALSE);
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

};
