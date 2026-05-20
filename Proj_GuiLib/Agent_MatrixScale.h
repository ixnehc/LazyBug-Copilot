#pragma once
#include "matrixedit_base.h"
#include "GuiEditor.h"
struct ScaleCtrlArg
{
	ScaleCtrlArg()
	{
		colX = ColorAlpha(0xff0000,0xff);
		colY = ColorAlpha(0x00ff00,0xff);
		colZ = ColorAlpha(0x0000ff,0xff);
		colActive = ColorAlpha(0xffff00,0xff);
		colSel =  ColorAlpha(0xffff00,0x66);
		lenAxis = 20.0f;
		activeRange =12.0f;
		s0 = 0.4f;
		s1 = 0.6f;
		speed = 0.01f;
	}
	float lenAxis;
	float activeRange;
	float s0,s1;
	float speed;
	DWORD colX ,colY,colZ,colActive,colSel;
};


class CAgent_MatrixScale :public CGuiAgent_Dragger<1,0> ,public CMatrixEditBase
{
public:
	CAgent_MatrixScale(void);
	~CAgent_MatrixScale(void);

public:
	virtual const char * getClassName() {return "CAgent_MatrixScale";}
	virtual BOOL Bind(MatrixEditData data);
	virtual BOOL OnDraw();
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y)	;
	virtual void OnDrag(int x,int y);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	void SetUIArg(ScaleCtrlArg arg){_arg = arg;}

protected:
	void _DrawAxisWire();
	void _DrawTriAreaWire();
	void _DrawActiveArea();
	void _HitTest(int x,int y);
	BOOL _HitTestArea(int x,int z);
	void _CalScreenNormal();
	float _CalCurrentScaleValue(int x,int y);

	DWORD _flag;
	i_math::vector3df _center;
	i_math::vector3df _nx,_ny,_nz;
	i_math::matrix43f _matInit,_matScaleEdit;
	float _lenAxisX,_lenAxisY,_lenAxisZ;
	ScaleCtrlArg _arg;

	i_math::vector3df _x0,_x1,_y0,_y1,_z0,_z1;
	i_math::vector2df _sxy,_syz,_szx,_sx,_sy,_sz;
	int _ix,_iy;
};
