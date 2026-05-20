#pragma once
#include "GuiEditor.h"

class CGuiAgent_2DMove : public CGuiAgent_Dragger<1,TRUE>
{
public:
	CGuiAgent_2DMove(void);
	~CGuiAgent_2DMove(void);

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag)	;
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
protected:
	void _UpdateTransform(i_math::matrix43f & mat);
	
	int _x,_y;
//	float _scale;
//	i_math::pos2di _pos;
	i_math::matrix43f _mat;
};
