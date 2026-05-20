#pragma once

#include "WorldSystem/IEntitySystemDefines.h"


class IProtoLib;
class CChildDoc : public CDocument
{
protected: // create from serialization only
	CChildDoc();
	DECLARE_DYNCREATE(CChildDoc)

// Attributes
public:
// 	ProtoID GetProtoID()	{		return _protoid;	}
// 	void SetProtoID(ProtoID protoid)	{		_protoid=protoid;	}
// 	const char *GetProtoFilePath();
// 	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildDoc();

protected:

// 	ProtoID _protoid;

// Generated message map functions
protected:
	//{{AFX_MSG(CChildDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
										AFX_CMDHANDLERINFO* pHandlerInfo)
	{
		return FALSE;
	}

};
