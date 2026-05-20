#pragma once


#include "GuiLib.h"
#include ".\resource.h"

#include "editor/editor.h"
#include "GuiEditor.h"
#include "GuiData.h"
#include "GuiViewWnd.h"

enum MatEditDlgNotify
{
	MEDlg_BeginMod,
	MEDlg_Mod,
	MEDlg_EndMod,
};

typedef fastdelegate::FastDelegate1<MatEditDlgNotify> MatEditDlgHandler;


struct GuiLib_Api GuiData_Mat : public GeData
{	
	GuiData_Mat()
	{
		mat=NULL;
	}
	virtual const char *GetName()	{		return "mat";	}

	i_math::matrix43f *mat;

};

class IMesh;
class IMtrl;
class ILight;
class GuiLib_Api CGuiView_Mat : public CGuiView
{
public:
	CGuiView_Mat()
	{
		_mesh=NULL;
		_mtrl=NULL;
		_lgt=NULL;
	}
	~CGuiView_Mat();

	virtual const char*	GetName()	{		return "mat";	}
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingRP;	}

	virtual void _OnDraw( IRenderPort*rp);

private:
	IMesh *_mesh;
	IMtrl *_mtrl;
	ILight *_lgt;

};

class CGuiAgent_Undo: public CGuiAgent
{
public:
	virtual BOOL OnKeyDown( char c, DWORD flag );
};

class CGuiAgent_MatrixEdit;
class GuiLib_Api CGuiActor_Mat: public CGuiActor
{
public:
	CGuiActor_Mat()
	{
		_matedit=NULL;
		_handler=NULL;
	}
	virtual const char *GetName()	{		return "mat";	}


	virtual void Reset();

protected:
	virtual const char *_GetModMgrName()	{		return "mat";	}

	void _BeginEdit(i_math::matrix43f *mat);
	void _Edit(i_math::matrix43f *mat);
	void _EndEdit(i_math::matrix43f *mat);

	MatEditDlgHandler _handler;

	CGuiAgent_MatrixEdit *_matedit;
	i_math::matrix43f _matBack;

	friend class CMatWnd;
	friend class CMod_Mat;

};




class GuiLib_Api CMatWnd: public CGuiViewWnd<CWnd>
{
public:
	CMatWnd()
	{
		_idTimer=0;
	}
	~CMatWnd()
	{

	}

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL Create( RECT &rc, CWnd *pParentWnd );
	void			Bind(i_math::matrix43f *mat);
	void SetHandler(MatEditDlgHandler handler)	{		_actor._handler=handler;	}
private:
	GuiData_Mat		_data;
	GuiData_Camera _dataCam;
	CGuiView_Mat		_view;
	CGuiActor_Mat	_actor;	

	UINT _idTimer;
	CGuiMgr		_mgr;

};




class GuiLib_Api CMatEditDlg : public CDialog
{
public:
	CMatEditDlg( CWnd* pParent = NULL );
	// dialog template
	enum { IDD = IDD_MATEDITDLG};
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
public:
	DECLARE_MESSAGE_MAP()
	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonResetBl();
	afx_msg void OnBnClickedCheckConst();
public:
	void Bind(i_math::matrix43f *mat);
	void SetHandler(MatEditDlgHandler handler)	{		_wnd.SetHandler(handler);	}
protected:
	CMatWnd _wnd;
};

