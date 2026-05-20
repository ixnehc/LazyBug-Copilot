/********************************************************************
	created:	2008/03/03
	created:	3:3:2008   17:30
	filename: 	f:\ixengine\proj_guilib\vssbase.cpp
	file path:	f:\ixengine\proj_guilib
	file base:	vssbase
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"

#include "VssBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"
#include "ximage.h"
#include "../Common/Log/LogFile.h"

#include "DlgVssOpenDatabase.h"
#include "DlgVssWorkfold.h"
#include "DlgVssChoose.h"

CSscSystemWrapper::CSscSystemWrapper() : _fs(NULL), _ss(NULL), _hParentWnd(NULL)
{
}

BOOL CSscSystemWrapper::Init(ISscSystem* ss, IFileSystem* fs)
{
	_ss = ss;
	_fs = fs;
	return TRUE;
}

BOOL CSscSystemWrapper::Connect(VssConfig& cfg, BOOL bSilent)
{
	BOOL bConnected = FALSE;
	if (!_ss)
		return bConnected;

	if (bSilent)
	{
		if (bConnected = _ss->SetSscServer(
			cfg.database.c_str(), cfg.user.c_str(), cfg.passwd.c_str()
			))
		{
			bConnected = 
				_ss->SetWorkingFolder(cfg.workingFolder.c_str(), cfg.project.c_str());
			return bConnected; 
		}
	}
	else
	{
		CDlgVssOpenDatabase dlgDatabase;
		dlgDatabase.SetDatabase(cfg.database.c_str());
		dlgDatabase.SetUser(cfg.user.c_str());
		dlgDatabase.SetPassword(cfg.passwd.c_str());
		if (dlgDatabase.DoModal() == IDOK)
		{
			bConnected = _ss->SetSscServer(
				dlgDatabase.GetDatabase(), dlgDatabase.GetUser(), dlgDatabase.GetPassword());
			if (!bConnected)
				return (bConnected = FALSE);

			const char** items;
			int nCount = 0;
			_ss->ListProject("$/", items, nCount);				

			CDlgVssWorkfold dlgWorkfold;
			dlgWorkfold.SetProjectItems(items, nCount);
			dlgWorkfold.EnableWorkingFolderEditable(cfg.workingFolder.empty());
			dlgWorkfold.SetWorkingFolder(cfg.workingFolder.c_str());
			if (dlgWorkfold.DoModal() == IDOK)
			{
				bConnected = _ss->SetWorkingFolder(dlgWorkfold.GetWorkingFolder(), dlgWorkfold.GetCurrentProject());
				if (!bConnected)
					return bConnected;

				cfg.database = dlgDatabase.GetDatabase();
				cfg.user = dlgDatabase.GetUser();
				cfg.passwd = dlgDatabase.GetPassword();

				cfg.project = dlgWorkfold.GetCurrentProject();
				cfg.workingFolder = dlgWorkfold.GetWorkingFolder();

				return (bConnected = TRUE);
			}
		}
	}
	return (bConnected = FALSE);
}

BOOL CSscSystemWrapper::IsConnected() const
{
	return (_ss ? _ss->IsConnected() : FALSE);
}

int CSscSystemWrapper::GetUserName(char* user, int len)
{
	return (_ss ? _ss->GetUserName(user, len) : 0);
}

BOOL CSscSystemWrapper::ListProject(const char* pathProject, const char**& items, int& nCount)
{
	return (_ss ? _ss->ListProject(pathProject, items, nCount) : FALSE);
}

BOOL CSscSystemWrapper::SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject)
{
	return (_ss ? _ss->SetWorkingFolder(pathWorkingFolder, pathProject) : FALSE);
}

BOOL CSscSystemWrapper::IsControlled(const char *pathFolderOrFile)
{
	return (_ss ? _ss->IsControlled(pathFolderOrFile) : FALSE);
}

BOOL CSscSystemWrapper::CheckIn(const char *pathFolderOrFile,BOOL *bModified)
{
	return (_ss ? _ss->CheckIn(pathFolderOrFile, bModified) : FALSE);
}

BOOL CSscSystemWrapper::CheckOut(const char *pathFolderOrFile,BOOL *bModified)
{
	BOOL bCheckedout = FALSE;
	if (!_ss || !_fs || !pathFolderOrFile)
		return bCheckedout;

	std::string strPath = pathFolderOrFile;
	if (strPath.find(GetWorkingFolder()) == std::string::npos)
	{
		strPath = GetWorkingFolder();
		strPath += pathFolderOrFile;
	}
	BOOL bWritable = IsWritable(strPath.c_str());
	if (bWritable && !CDlgVssChoose::ms_bApplyAllItems)
	{
		CDlgVssChoose dlgChoose(CWnd::FromHandle(_hParentWnd));
		dlgChoose._strContent = strPath.c_str();
		if (dlgChoose.DoModal() != IDOK)
		{
			return FALSE;
		}
	}
	std::string strBackFileName;
	BOOL bRestore = (bWritable && CDlgVssChoose::ms_bLeave);
	if (bRestore)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		// Back the old version
		std::string strFolderPath = GetFileFolderPath(strPath);
		std::string strFileTitle = GetFileTitle(strPath);
		FormatString(strBackFileName, 
			"%s\\%s_%04d%02d%02d%02d%02d%02d.bak", 
			strFolderPath.c_str(), strFileTitle.c_str(), 
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		_fs->SetSearchPath("");
		_fs->RemoveFile(strBackFileName.c_str());
		if (!_fs->RenameAbs(strPath.c_str(), strBackFileName.c_str()))
			return bCheckedout;
		//RemoveFile(strBackFileName.c_str());
		//if (!RenameFile(strPath.c_str(), strBackFileName.c_str()))
		//	return bCheckedout;
	}
	if (bCheckedout = _ss->CheckOut(pathFolderOrFile, bModified))
	{
		if (bRestore)
		{
			_fs->SetSearchPath("");
			_fs->RemoveFile(strPath.c_str());
			bCheckedout = _fs->RenameAbs(strBackFileName.c_str(), strPath.c_str());
			//RemoveFile(strPath.c_str());
			//bCheckedout = RenameFile(strBackFileName.c_str(), strPath.c_str());
		}		
	}
	else if (bRestore)
	{
		_fs->RenameAbs(strBackFileName.c_str(), strPath.c_str());
		//RenameFile(strBackFileName.c_str(), strPath.c_str());
	}
	return bCheckedout;
}

BOOL CSscSystemWrapper::GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified)
{
	CString strPath = pathFolderOrFile;
	if (strPath.Find(GetWorkingFolder()) == -1)
	{
		strPath = GetWorkingFolder();
		strPath += pathFolderOrFile;
	}
	if (IsWritable(strPath) && !CDlgVssChoose::ms_bApplyAllItems)
	{
		CDlgVssChoose dlgChoose(CWnd::FromHandle(_hParentWnd));
		dlgChoose._strContent = strPath;
		if (dlgChoose.DoModal() != IDOK)
		{
			return FALSE;
		}
	}
	return (_ss ? 
		(CDlgVssChoose::ms_bLeave ? TRUE : _ss->GetLatestVersion(pathFolderOrFile, bModified)) : FALSE);
}

BOOL CSscSystemWrapper::GetState(const char *pathFile,SscState &state)
{
	return (_ss ? _ss->GetState(pathFile, state) : FALSE);
}

BOOL CSscSystemWrapper::Delete(const char *pathFolderOrFile)
{
	return (_ss ? _ss->Delete(pathFolderOrFile) : FALSE);
}

BOOL CSscSystemWrapper::Rename(const char *pathFolderOrFile, const char *pszNewName)
{
	return (_ss ? _ss->Rename(pathFolderOrFile, pszNewName) : FALSE);
}

const char* CSscSystemWrapper::GetWorkingFolder() const
{
	return (_ss ? _ss->GetWorkingFolder() : NULL);
}

const char* CSscSystemWrapper::GetCurrentProject() const
{
	return (_ss ? _ss->GetCurrentProject() : NULL);
}

inline BOOL CSscSystemWrapper::IsWritable(const char* pathFolderOrFile)
{
	DWORD dwAttrs = GetFileAttributes(pathFolderOrFile);
	return ((dwAttrs != INVALID_FILE_ATTRIBUTES) && !(dwAttrs & FILE_ATTRIBUTE_READONLY));
}

//BOOL CSscSystemWrapper::RemoveFile(const char* pszFileName)
//{
//	return ::DeleteFile(pszFileName);
//}
//
//BOOL CSscSystemWrapper::RenameFile(const char* pszFileName, const char* pszNewFileName)
//{
//	char szFileName[MAX_PATH] = { 0 };
//	char szNewFileName[MAX_PATH] = { 0 };
//	strcpy(szFileName, pszFileName);
//	strcpy(szNewFileName, pszNewFileName);
//	SHFILEOPSTRUCT fo;
//	fo.hwnd = NULL;
//	fo.wFunc = FO_RENAME;
//	fo.pFrom = szFileName;
//	fo.pTo = szNewFileName;
//	fo.hNameMappings = NULL;
//	fo.lpszProgressTitle = NULL;
//	fo.fAnyOperationsAborted = FALSE;
//	fo.fFlags = FOF_NOERRORUI | FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
//	return (0 == ::SHFileOperation(&fo));
//}