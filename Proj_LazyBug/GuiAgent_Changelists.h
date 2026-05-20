#pragma once

#include "anim/animbase.h"

#include "GuiAgent_2DTransform.h"


// view的拖动 缩放
class CGuiAgent_Changelists2DTransform : public CGuiAgent_2DTransform
{
public:
	CGuiAgent_Changelists2DTransform()
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

class CGuiAgent_ChangelistsCommand:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick( int x, int y, DWORD flag );
	virtual BOOL OnCommand( DWORD idCmd );

};

struct ChangelistsEntry;
// 控制点的操作
class CGuiAgent_ChangelistsEdit: public CGuiAgent_Dragger<DRAG_BUTTON_LEFT,0>
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
	CGuiAgent_ChangelistsEdit();
	virtual BOOL OnRButtonClick( int x, int y, DWORD flag );
	virtual BOOL OnRButtonDown(int x, int y, DWORD flag);
	virtual BOOL OnLButtonDown( int x,int y,DWORD flag );
	virtual BOOL OnLButtonDblClk( int x,int y,DWORD flag );
	virtual BOOL OnBeginDrag( int x, int y, DWORD flag );
	virtual void OnEndDrag( int x, int y, DWORD flag );
	virtual void OnDrag( int x, int y, DWORD flag );
	virtual BOOL OnCommand( DWORD idCmd );
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta, DWORD flag);
private:

	Clipboard _cb;

	void _BeginMod();
	void _EndMod();

	i_math::pos2di _ptDrag;
	BOOL _bDragLock;
};


