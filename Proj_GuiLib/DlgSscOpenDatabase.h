#pragma once
//#include <XTToolkitPro.h>       // Xtreme Toolkit support

#include "resource.h"

const char* const VSS_SECTION				= "Ssc";
const char* const VSS_KEY_DATABASE			= "Database";
const char* const VSS_KEY_USER				= "User";
const char* const VSS_KEY_PASSWORD			= "Password";
const char* const VSS_DATABASE_CONFIGFILE	= "SscDatabase.ini";

// CDlgSscOpenDatabase dialog

class CDlgSscOpenDatabase : public CXTPDialog
{
	DECLARE_DYNAMIC(CDlgSscOpenDatabase)

public:
	CDlgSscOpenDatabase(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSscOpenDatabase();

// Dialog Data
	enum { IDD = IDD_SSC_OPENDATABASE };

public:
	void SetDatabase(const char* db)
	{
		_strDatabase = db;
	}
	const TCHAR* GetDatabase() const
	{
		return (LPCTSTR)_strDatabase;
	}
	void SetUser(const char* user)
	{
		_strUser = user;
	}
	const TCHAR* GetUser() const
	{
		return _strUser;
	}
	void SetPassword(const char* pwd)
	{
		_strPassword = pwd;
	}
	const TCHAR* GetPassword() const
	{
		return _strPassword;
	}

public:
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString _strDatabase;
	CString _strUser;
	CString _strPassword;
};
