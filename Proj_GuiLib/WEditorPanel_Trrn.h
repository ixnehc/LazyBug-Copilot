#pragma once

#include "GuiLib.h"

#include "resource.h"

#include "EditorPanel.h"

#include "WorldEditorDefines.h"

#include "WorldSystem/IWorldSystemDefines.h"
#include "WorldSystem/IAssetSystemDefines.h"

class CEditorPanel_Trrn;

#define INVALID_VHIT (i_math::vector3df(10000,10000,10000))

class CWEA_PaintTrrnBrush:public CWEA_EditorPanel<CEditorPanel_Trrn>
{
public:
	CWEA_PaintTrrnBrush()
	{
		_bPaint=FALSE;
	}
	virtual void OnEnable();
	virtual void OnDisable();

	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual void OnKillFocus(OpType type);

protected:
	BOOL _PrepareSeedMap(int x,int y);
	void _DoPaint();
	TrrnSeedMap _seedmap;

	BOOL _bPaint;


};


class CPttnCtrl2;
class CTexCtrl2;
class CTBLImageLib;
class ITrrnMapEditor;
class GuiLib_Api CEditorPanel_Trrn:public CEditorPanel
{
public:
	CEditorPanel_Trrn(CWnd* pParent = NULL);

	BOOL Create(CWnd *pParent)	{		return CDialog::Create(IDD,pParent);	}

	enum { IDD = IDD_EDITPANEL_TRRN};

	CTBLImageLib *GetImageLib()	{		return _imagelib;	}

	ITrrnMapEditor *GetTrrnMapEditor()	{		return _editor;	}
	TrrnSeedMapArg &GetTrrnSeedMapArg()	{		return _arg;	}

	BrushID GetSelBrushID()	{		return (BrushID)_idSelBr;	}

	virtual void SetEnv(EditorEnv&env);
	virtual void OnInitAgent();//use DefineEditorAgent(xxx) to define agents

	virtual void OnUpdateUI();

protected:

	void _ResetContent();
	CPttnCtrl2 *_pttnctrl;
	CTexCtrl2 *_texctrl;
	CTBLImageLib *_imagelib;

	ITrrnBrushLib *_brlib;
	ITrrnMapEditor *_editor;
	ITrrnMap *_map;

	DWORD _idSelBr;

	TrrnSeedMapArg _arg;//NOTE:Not every element in this arg is valid,



public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeBrushcombo();
	afx_msg void OnBnClickedPainttrrnbtn();
	afx_msg void OnBnClickedPaintbasetrrnbtn();
	afx_msg void OnBnClickedPaintheightmapbtn();
	afx_msg void OnBnClickedSavetrrnmap();
	afx_msg void OnBnClickedLoadheightmap();
	afx_msg void OnBnClickedPaintholebtn();
	afx_msg void OnBnClickedClearholebtn();
};
