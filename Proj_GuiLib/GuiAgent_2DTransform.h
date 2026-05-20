#pragma once
#include "GuiEditor.h"

class GuiLib_Api CGuiAgent_2DTransform : public CGuiAgent_Dragger<DRAG_BUTTON_MIDDLE,FALSE>
{
public:
	CGuiAgent_2DTransform();
	~CGuiAgent_2DTransform(void);

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag)	;
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);

protected:
	virtual void OnUpdateTransform( const i_math::pos2df &pos, const i_math::pos2df &scale){  }
	
	// set/ get max zoom in rate
	void	SetMaxZoomIn( float rate )	{		_fMaxZoomIn = rate;	}
	float	GetMaxZoomIn()	{		return _fMaxZoomIn;	}
	// set/ get max zoom out rate
	void	SetMaxZoomOut( float rate )	{		_fMaxZoomOut = rate;	}
	float	GetMaxZoomOut()	{		return _fMaxZoomOut;	}
	
protected:
	void _UpdateTransform(i_math::matrix43f & mat);

	void _CalcTransform(i_math::matrix43f & mat);
	
	int _x,_y;
	i_math::matrix43f _mat;
	float _fMaxZoomIn;
	float _fMaxZoomOut;
};
