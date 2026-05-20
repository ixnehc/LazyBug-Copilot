#pragma once

#include "resource.h"

#include "CCompDCWnd.h"

#include "SpinEdit.h"

#include "CCompDCWnd.h"

#include "resdata/MeshData.h"

// CUVAtlasMakeDlg dialog
class IRenderSystem;
class IUtilRS;


class CAtlasView:public CCompDCWnd<CStatic>
{
public:
	CAtlasView()
	{
		data=NULL;
		channel=0;
	}
	void SetData(MeshData*data0)	{		data=data0;	}
	void SetChannel(DWORD ch)	{		channel=ch;	}

	virtual void Draw(CDC *pDC);

protected:
	MeshData *data;
	DWORD channel;
};

class CUVAtlasMakeDlg : public CDialog
{
	DECLARE_DYNAMIC(CUVAtlasMakeDlg)

public:
	CUVAtlasMakeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUVAtlasMakeDlg();

// Dialog Data
	enum { IDD = IDD_UVATLASMAKEDLG};

	void SetWorkingData(MeshData*data,IRenderSystem *pRS,IUtilRS *pUtilRS);
	BOOL IsModified()	{		return _bModified;	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:

	MeshData *_data;
	IRenderSystem *_pRS;
	IUtilRS *_pUtilRS;

	DWORD _iSel;
	void _Update();
	void _UpdateDesc();
	void _CollectArg(UVAtlasArg &arg);

	CSpinEdit _spinWidth;
	CSpinEdit _spinHeight;
	CSpinEdit _spinGutter;
	CSpinEdit _spinStretch;
	CEdit _width;
	CEdit _height;
	CAtlasView _viewport;
	CStatic _desc;
	CComboBox _channelcombo;

	BOOL _bModified;

public:
	afx_msg void OnCbnSelchangeUvchannel();
	afx_msg void OnBnClickedMakeuvatlas();
	afx_msg void OnBnClickedRemoveuvchannel();
	afx_msg void OnBnClickedAssignatlasparam();
};
