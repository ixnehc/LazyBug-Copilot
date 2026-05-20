#pragma once

#include "anim/animbase.h"

#include "GuiAgent_2DTransform.h"


// view的拖动 缩放
class CGuiAgent_ValueSet2DTransform : public CGuiAgent_2DTransform
{
public:
	CGuiAgent_ValueSet2DTransform()
		: CGuiAgent_2DTransform()
	{
		SetMaxZoomIn( 250000.0f );
		SetMaxZoomOut( 5.f );
	}
	virtual void OnUpdateTransform( const i_math::pos2df &pos, const i_math::pos2df &scale );
	virtual BOOL OnRButtonClick( int x, int y, DWORD flag );
	virtual BOOL OnCommand( DWORD idCmd );

	BOOL Fit(const char *sel);

protected:

};

class CGuiAgent_ValueSetCommand:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick( int x, int y, DWORD flag );
	virtual BOOL OnCommand( DWORD idCmd );

};

struct ValueSetEntry;
// 控制点的操作
class CGuiAgent_ValueSetEdit: public CGuiAgent_Dragger<DRAG_BUTTON_LEFT,0>
{
public:

	struct Clipboard
	{
		Clipboard()
		{
			bContent=FALSE;
		}
		BOOL bContent;
		KeyType kt;
		float v;
		DWORD col;
	};
	CGuiAgent_ValueSetEdit();
	virtual BOOL OnRButtonClick( int x, int y, DWORD flag );
	virtual BOOL OnLButtonDown( int x,int y,DWORD flag );
	virtual BOOL OnLButtonDblClk( int x,int y,DWORD flag );
	virtual BOOL OnBeginDrag( int x, int y, DWORD flag );
	virtual void OnEndDrag( int x, int y, DWORD flag );
	virtual void OnDrag( int x, int y, DWORD flag );
	virtual BOOL OnCommand( DWORD idCmd );
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
private:
	int _HitTest(int x,int y);

	Clipboard _cb;

	void _BeginMod();
	void _EndMod();

	i_math::pos2di _ptDrag;
	BOOL _bDragLock;
};


