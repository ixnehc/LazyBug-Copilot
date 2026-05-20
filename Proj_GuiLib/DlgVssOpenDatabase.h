#pragma once
//#include <XTToolkitPro.h>       // Xtreme Toolkit support

const char* const VSS_SECTION				= "Vss";
const char* const VSS_KEY_DATABASE			= "Database";
const char* const VSS_KEY_USER				= "User";
const char* const VSS_KEY_PASSWORD			= "Password";
const char* const VSS_DATABASE_CONFIGFILE	= "VssDatabase.ini";

// CDlgVssOpenDatabase dialog

class CDlgVssOpenDatabase : public CXTPDialog
{
	DECLARE_DYNAMIC(CDlgVssOpenDatabase)

public:
	CDlgVssOpenDatabase(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVssOpenDatabase();

// Dialog Data
	enum { IDD = IDD_VSS_OPENDATABASE };

public:
	void SetDatabase(const char* db)
	{
		_strDatabase = db;
	}
	const char* GetDatabase() const
	{
		return _strDatabase;
	}
	void SetUser(const char* user)
	{
		_strUser = user;
	}
	const char* GetUser() const
	{
		return _strUser;
	}
	void SetPassword(const char* pwd)
	{
		_strPassword = pwd;
	}
	const char* GetPassword() const
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
