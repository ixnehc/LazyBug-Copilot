

#ifndef _ParticleFeatureEdit_H_
#define _ParticleFeatureEdit_H_


#include "CCompDCWnd.h"
#define ScaleVal 2.0f
#define ScaleDefault 1.0f




class CParticleFeatureEdit : public CCompDCWnd<CWnd>
{
public:
	CParticleFeatureEdit();
	virtual ~CParticleFeatureEdit();
	enum
	{
		GRID_NUM = 25,
	};
	enum OpEnum
	{
		OP_NONE,
		OP_CONTROL_POINT,
		OP_BACKGROUND,
	};
public:
	virtual void Draw( CDC* pDC );

	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );

	void	_DrawEditBackground( CDC* pDC, bool changeScale );
	void	_Zoom( CDC *pDC,float scale );
	float	_fScale;
	bool	_bChangeScale;
protected:
	DECLARE_MESSAGE_MAP()
	
	OpEnum	_op;
	CBrush	_brBackGround;
	CPen	_penGrid;
	CPen	_penBrokenLine;
	bool	_bChoose;		// 表示是否选种控制点
	CPoint	_ptLeft;		// 鼠标左键操作的位置
	CPoint	_ptView;		// 视点
	CPoint	_ptRight;		// 鼠标右键操作的位置
	CPoint	_ptOffset;		// 移动的偏移
	CDC		_dc;
	CBitmap	_bmp;
};
#endif