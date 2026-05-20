#pragma once

#include "AssetCtrl.h"


#include "WorldSystem/IAssetShell.h"

#include "WorldSystem/IAssetRenderer.h"

typedef DWORD DirtyFlag;

#define DirtyFlag_Init 1
#define DirtyFlag_Rect 2
#define DirtyFlag_Text 4
#define DirtyFlag_Image 8

#define DirtyFlagExt(v) (256<<(v))

class CShellText;
class ITextPiece;
struct TextStyle;
struct ImageCombo;

class CAssetCtrlRenderer
{
public:
	virtual ~CAssetCtrlRenderer(void) {}

public:
	virtual CClass* GetClass(void) { return 0; }

public:
	virtual BOOL Create(IRatomsShell* ratoms, CAssetCtrl* window)
	{
		_ratoms = ratoms;
		_window = window;
		_dirties=0xffffffff;
		return TRUE;
	}

	CAssetCtrl* GetWindow(void) { return _window; }

	void DoRender(void);
	virtual void Render(void) = 0;
	virtual void RefreshRender()=0;
	virtual void InitRender()	{}

	virtual void OnRectChanged(void)		{		SetDirty(DirtyFlag_Rect);	}
	virtual void OnTextChanged(void)		{		SetDirty(DirtyFlag_Text)	;}
	virtual void OnImageChanged(void) 		{		SetDirty(DirtyFlag_Image);	}

	void SetDirty(DirtyFlag v)	{		_dirties|=v;	}
	BOOL TestDirty(DirtyFlag v)	{		return _dirties&v?TRUE:FALSE;	}
	void ClearDirty(DirtyFlag v)	{		_dirties&=~v;	}
	void ClearAllDirty()	{		_dirties=0;	}

protected:
	typedef void * RatomID;

	void _UpdateAlpha(RatomID ratom);
	void _UpdateColor(RatomID ratom,DWORD);

	virtual void _UpdateBlankImage(RatomID &ratomImage);
	virtual void _UpdateComboImage(RatomID &ratomImage,ImageCombo *combo=NULL,ShellRect * rc= NULL);
	virtual void _UpdateBtnImage(RatomID &ratomImage,const char *path,DWORD idx);
	virtual void _UpdateTextRect(RatomID &ratomText,CShellText *text,ShellPos *ptOff=NULL);
	virtual void _UpdateTextRect(RatomID &ratomText,TextStyle *style,ShellPos *ptOff=NULL);

	void _ValidateTextRatom(RatomID &ratomText,CShellText *text);
	virtual void _UpdateTextContent(RatomID &ratomText,CShellText *text);

	void _ValidateTextRatom(RatomID &ratomText,ITextPiece*tp);
	virtual void _UpdateTextContent(RatomID &ratomText,ITextPiece*tp);

	void _UpdateImage_Row3(RatomID &ratomImage,ImageCombo &fmt,ShellRect &rc);

protected:
	CAssetCtrl* _window;	//!< Pointer to the window this windowrenderer is assigned to.
	IRatomsShell* _ratoms;
	DirtyFlag _dirties;


};
