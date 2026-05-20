#pragma once
#include "matrixedit_base.h"
#include "GuiEditor.h"

struct SimpleScaleCtrlArg
{
	SimpleScaleCtrlArg()
	{
		colActive = ColorAlpha(0xffff00,0xff);
		colSel =  ColorAlpha(0xffff00,0x66);
		lenAxis = 60.0f;
		speed = 0.01f;
	}
	float lenAxis;
	float speed;
	DWORD colActive,colSel;
};

class CAgent_MatrixSimpleScale :public CGuiAgent_Dragger<1,0> ,public CMatrixEditBase
{
public:
	CAgent_MatrixSimpleScale(void);
	~CAgent_MatrixSimpleScale(void);
public:
	virtual const char * getClassName() {return "CAgent_MatrixScale";}
	virtual BOOL OnDraw();
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag)	;
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	void SetUIArg(SimpleScaleCtrlArg arg){_arg = arg;}
	virtual BOOL Bind(MatrixEditData &data);
	virtual void OnDlgEdit(BOOL bRelative,DWORD changeType,const i_math::vector3df& vecChange,i_math::matrix43f &matLocal);
protected:
	void _DrawAxisWire();
	void _DrawTriAreaWire();
	void _DrawActiveArea();
	BOOL _HitTest(int x,int y);
	void _CheckScale(i_math::matrix43f& matScale); //确保不会变得很小或很大 缩小为原来的20或放大为原来的20倍

	DWORD _flag;
	i_math::vector3df _center;
	i_math::vector3df _nx,_ny,_nz;
	i_math::matrix43f _matInit,_matScaleEdit;
	float _lenAxisX,_lenAxisY,_lenAxisZ;
	SimpleScaleCtrlArg  _arg;

	i_math::vector3df _x0,_y0,_z0;
	i_math::vector2df  _curNs;
	int _ix,_iy,_scrX,_scrY;

};

