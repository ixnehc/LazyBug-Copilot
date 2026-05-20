#pragma once

#include "resource.h"

#include "ResEditCtrl.h"
#include "afxwin.h"


// CMeshContent dialog

struct Reps_Mesh;
struct MeshData;

class CMeshContent : public CDialog,public CResEditCtrl
{
	DECLARE_DYNAMIC(CMeshContent)

public:
	CMeshContent(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMeshContent();

	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);

	Reps_Mesh *GetState()	{		return (Reps_Mesh *)_state;	}
	MeshData *GetMeshData()	{		return (MeshData*)_GetResData();	}


// Dialog Data
	enum { IDD = IDD_MESHCONTENT};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void EnableCtrl(BOOL bActive=TRUE);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeFramecombo();
	afx_msg void OnCbnSelchangeSurfcombo();
	afx_msg void OnBnClickedTangentcheck();
	afx_msg void OnBnClickedRemovesurf();
	afx_msg void OnBnClickedRemoveframe();
	afx_msg void OnLodChange();
	
protected:
	void _Refresh();
	CComboBox _comboFrame;
	CComboBox _comboLod;
	CButton _btnTangentInfo;
	CButton _btnRemoveFrame;
	CButton _btnMakeUVAtlas;
public:
protected:
public:
	afx_msg void OnBnClickedMakeuvatlas();
protected:
};
