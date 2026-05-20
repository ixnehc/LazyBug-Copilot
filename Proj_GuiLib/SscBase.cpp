/********************************************************************
	created:	2008/03/03
	created:	3:3:2008   17:30
	filename: 	f:\ixengine\proj_guilib\sscbase.cpp
	file path:	f:\ixengine\proj_guilib
	file base:	sscbase
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"

#include "SscBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"
#include "registry/Registry.h"

#include "../Common/Log/LogFile.h"

#include "DlgSscOpenDatabase.h"
#include "DlgSscWorkfold.h"
#include "DlgSscChoose.h"

CSscSystemWrapper::CSscSystemWrapper(int iSerial) : _pFS(NULL), _pSS(NULL), _hParentWnd(NULL)
{
	_bApplyAllItems = FALSE;
	_bLeave = FALSE;
	_bRestoreDefaultSetting = TRUE;

	_iSerial=iSerial;
}

BOOL CSscSystemWrapper::TryConnect(CCurrentUserRegistry&reg)
{
	SscConfig cfg;

	//先强制中断
	Disconnect();

	BYTE *data;
	DWORD szData;
	if(TRUE==reg.ReadData("SscConfig",_GetRegKey(),(void*&)data,szData))
	{
		CDataPacket dp;
		dp.SetDataBufferPointer(data);
		cfg.GLoad(dp);

		Connect(cfg,TRUE);

		return IsConnected();
	}	

	return FALSE;
}


const char *CSscSystemWrapper::_GetRegKey()
{
	static std::string s;

	std::string path;
	if (_pathEngine.empty())
	{
		path=GetModuleFolderPath(NULL);

		CutTailSubPath(path,s);
	#ifdef _DEBUG
		CutTailSubPath(path,s);
	#endif
	}
	else
		path=_pathEngine;

	FormatString(s,"Data%02d(%s)",_iSerial+1,path.c_str());

	return s.c_str();
}

BOOL CSscSystemWrapper::HasConfig(CCurrentUserRegistry &reg)
{
	SscConfig cfg;

	BYTE *data;
	DWORD szData;
	if(FALSE==reg.ReadData("SscConfig",_GetRegKey(),(void*&)data,szData))
		return FALSE;

	CDataPacket dp;
	dp.SetDataBufferPointer(data);
	cfg.GLoad(dp);

	if (cfg.database=="")
		return FALSE;
	return TRUE;
}


void CSscSystemWrapper::PromptConnect(CCurrentUserRegistry &reg,BOOL bSilent)
{
	SscConfig cfg;


	//先强制中断
	Disconnect();

	BYTE *data;
	DWORD szData;
	if(TRUE==reg.ReadData("SscConfig",_GetRegKey(),(void*&)data,szData))
	{
		CDataPacket dp;
		dp.SetDataBufferPointer(data);
		cfg.GLoad(dp);

		Connect(cfg,bSilent);
		if ((bSilent)&&(!IsConnected()))
			Connect(cfg,FALSE);//silent connect failed,let the user choose
	}
	else
		Connect(cfg,FALSE);//let the user choose

	if (!IsConnected())
		Connect(cfg,TRUE);

	if (IsConnected())
	{
		std::vector<BYTE>buf;
		DP_BeginSave(dp,buf);
		cfg.GSave(dp);
		DP_EndSave();

		reg.WriteData("SscConfig",_GetRegKey(),(void*)buf.data(),buf.size());
	}
	
}


BOOL CSscSystemWrapper::Init(ISscSystem* ss, IFileSystem* fs)
{
	_pSS = ss;
	_pFS = fs;	
	return TRUE;
}

BOOL CSscSystemWrapper::Connect(SscConfig& cfg, BOOL bSilent)
{
	const TCHAR* S_OPENDATABASE_FAIL = _T("Can't open the database, please check the settings.");
	
	BOOL bConnected = FALSE;
	if (!_pSS || !_pFS)
		return bConnected;

	_bApplyAllItems = FALSE;
	_bLeave = FALSE;
	_bRestoreDefaultSetting = TRUE;

	if (bSilent)
	{
		if (bConnected = _pSS->Connect(
			cfg.database.c_str(), cfg.user.c_str(), cfg.passwd.c_str()
			))
		{
			bConnected = 
				_pSS->SetWorkingFolder(cfg.workingFolder.c_str(), cfg.project.c_str());
			return bConnected; 
		}
//		else
//		{
//			MessageBox(_hParentWnd, S_OPENDATABASE_FAIL, "Open SourceSafe Database", MB_OK);
//		}
	}
	else
	{
		CDlgSscOpenDatabase dlgDatabase;
		dlgDatabase.SetDatabase(cfg.database.c_str());
		dlgDatabase.SetUser(cfg.user.c_str());
		dlgDatabase.SetPassword(cfg.passwd.c_str());
		while (!bConnected && dlgDatabase.DoModal() == IDOK)
		{
			std::string db = toMBCS(dlgDatabase.GetDatabase());
			std::string user= toMBCS(dlgDatabase.GetUser());
			std::string password = toMBCS(dlgDatabase.GetPassword());
			bConnected = _pSS->Connect(db.c_str(), user.c_str(), password.c_str());
			if (!bConnected)
			{
				MessageBox(_hParentWnd, S_OPENDATABASE_FAIL, _T("Open SourceSafe Database"), MB_OK);
			}
		}
		if (!bConnected)
			return bConnected;

		const char** items;
		int nCount = 0;
		_pSS->ListProject(VSS_ROOT, items, nCount);				

		CDlgSscWorkfold dlgWorkfold;
		dlgWorkfold.SetProjectItems(items, nCount);
		dlgWorkfold.EnableWorkingFolderEditable(TRUE);
//		dlgWorkfold.EnableWorkingFolderEditable(cfg.workingFolder.empty());
		dlgWorkfold.SetWorkingFolder(cfg.workingFolder.c_str());

		if (dlgWorkfold.DoModal() == IDOK)
		{
			std::string folder = toMBCS(dlgWorkfold.GetWorkingFolder());
			std::string project= toMBCS(dlgWorkfold.GetCurrentProject());

			bConnected = _pSS->SetWorkingFolder(folder.c_str(), project.c_str());
			if (!bConnected)
				return bConnected;

			cfg.database = toMBCS(dlgDatabase.GetDatabase());
			cfg.user = toMBCS(dlgDatabase.GetUser());
			cfg.passwd = toMBCS(dlgDatabase.GetPassword());

			cfg.project = toMBCS(dlgWorkfold.GetCurrentProject());
			cfg.workingFolder = toMBCS(dlgWorkfold.GetWorkingFolder());

			return (bConnected = TRUE);
		}
	}
	return (bConnected = FALSE);
}

BOOL CSscSystemWrapper::IsConnected() const
{
	return (_pSS ? _pSS->IsConnected() : FALSE);
}

void CSscSystemWrapper::Disconnect()
{
	if (IsConnected())
		_pSS->Disconnect();
}


int CSscSystemWrapper::GetUserName(char* user, int len)
{
	return (_pSS ? _pSS->GetUserName(user, len) : 0);
}

BOOL CSscSystemWrapper::ListProject(const char* pathProject, const char**& items, int& nCount)
{
	return (_pSS ? _pSS->ListProject(pathProject, items, nCount) : FALSE);
}

BOOL CSscSystemWrapper::SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject)
{
	return (_pSS ? _pSS->SetWorkingFolder(pathWorkingFolder, pathProject) : FALSE);
}

BOOL CSscSystemWrapper::IsControlled(const char *pathFolderOrFile)
{
	return (_pSS ? _pSS->IsControlled(pathFolderOrFile) : FALSE);
}

BOOL CSscSystemWrapper::CheckIn(const char *pathFolderOrFile,long flags)
{
	return (_pSS ? _pSS->CheckIn(pathFolderOrFile, flags) : FALSE);
}

BOOL CSscSystemWrapper::CheckOut(const char *pathFolderOrFile,long flags)
{
	BOOL bResult = FALSE;
	if (!_pSS || !_pSS->IsConnected())
		return FALSE;

	// Restore the default settings
	if (_bRestoreDefaultSetting)
	{
		_bApplyAllItems = FALSE;
		_bLeave = FALSE;
	}

	int nCount = 0;
	const char** items;
	if (!_pFS->ExistFileAbs(pathFolderOrFile))
	{
		std::string proj;
		_GetProjectPath(pathFolderOrFile, proj);	

		_pSS->ListProject(proj.c_str(), items, nCount, 1); // List files
	}

	if (nCount > 0)
	{
		std::string subPath;
		size_t offset = strlen(GetCurrentProject());
		for (int i = 0; i < nCount; i++)
		{
			const char* pathFile = items[i] + offset;
			subPath = pathFile;
			_ReplaceString(subPath, std::string(VSS_DELIM), std::string(WIN32_DELIM));
			bResult = _CheckOutFile(subPath.c_str());
		}
	}
	else
	{
		bResult = _CheckOutFile(pathFolderOrFile);
	}

	return bResult;
}

BOOL CSscSystemWrapper::CheckOut(const char **pathFoldersOrFiles,DWORD count,long flags)
{
	BOOL bResult = FALSE;
	for (DWORD i = 0; i < count; i++)
	{
		bResult = CheckOut(pathFoldersOrFiles[i], flags);
		_bRestoreDefaultSetting = FALSE;
	}
	_bRestoreDefaultSetting = TRUE;
	return bResult;
}

BOOL CSscSystemWrapper::GetLatestVersion(const char *pathFolderOrFile,long flags)
{
	BOOL bResult = FALSE;
	if (!_pSS || !_pSS->IsConnected())
		return FALSE;

	// Restore the default settings
	if (_bRestoreDefaultSetting)
	{
		_bApplyAllItems = FALSE;
		_bLeave = FALSE;
	}

	const char** items;
	int nCount = 0;

	if (!_pFS->ExistFileAbs(pathFolderOrFile))
	{
		std::string proj;
		_GetProjectPath(pathFolderOrFile, proj);	

		_pSS->ListProject(proj.c_str(), items, nCount, 1);	// List Files
	}

	if (nCount > 0)
	{
		std::string subPath;
		size_t offset = strlen(GetCurrentProject());
		for (int i = 0; i < nCount; i++)
		{
			const char* pathFile = items[i] + offset;
			subPath = pathFile;
			_ReplaceString(subPath, std::string(VSS_DELIM), std::string(WIN32_DELIM));
			bResult = _GetFileLatestVersion(subPath.c_str());
		}
	}
	else
	{
		bResult = _GetFileLatestVersion(pathFolderOrFile);
	}

	return bResult;
}

BOOL CSscSystemWrapper::GetLatestVersion(const char **pathFoldersOrFiles,DWORD count,long flags)
{
	_bRestoreDefaultSetting = TRUE;
	_bLeave=FALSE;
	_bApplyAllItems=FALSE;

	BOOL bResult = FALSE;
	for (DWORD i = 0; i < count; i++)
	{
		bResult = GetLatestVersion(pathFoldersOrFiles[i], flags);
		_bRestoreDefaultSetting=FALSE;
	}

	_bRestoreDefaultSetting = TRUE;
	return bResult;
}

BOOL CSscSystemWrapper::GetState(const char *pathFile,SscState &state)
{
	return (_pSS ? _pSS->GetState(pathFile, state) : FALSE);
}

BOOL CSscSystemWrapper::Delete(const char *pathFolderOrFile)
{
	return (_pSS ? _pSS->Delete(pathFolderOrFile) : FALSE);
}

BOOL CSscSystemWrapper::Rename(const char *pathFolderOrFile, const char *pszNewName)
{
	return (_pSS ? _pSS->Rename(pathFolderOrFile, pszNewName) : FALSE);
}

BOOL CSscSystemWrapper::CreateSubProject(const char *pathProject)
{
	return (_pSS ? _pSS->CreateSubProject(pathProject) : FALSE);
}

const char* CSscSystemWrapper::GetWorkingFolder() const
{
	return (_pSS ? _pSS->GetWorkingFolder() : NULL);
}

const char* CSscSystemWrapper::GetCurrentProject() const
{
	return (_pSS ? _pSS->GetCurrentProject() : NULL);
}

BOOL CSscSystemWrapper::_CheckOutFile(const char* pathFile, BOOL bSilent)
{
	const long VSSFLAG_REPREPLACE = 128;
	const long VSSFLAG_REPSKIP = 192;
	const long VSSFLAG_REPMERGE = 256;	

	if (!bSilent && !_bApplyAllItems)
	{
		std::string workingPath;
		_GetWorkingPath(pathFile, workingPath);
		if (_IsWritable(workingPath.c_str()))
		{
			CDlgSscChoose dlgChoose(CWnd::FromHandle(_hParentWnd));
			dlgChoose._strContent = workingPath.c_str();
			if (dlgChoose.DoModal() != IDOK)
			{
				return FALSE;
			}

			_bLeave = dlgChoose._bLeave;
			_bApplyAllItems = dlgChoose._bApplyAllItems;
		}		
	}

	long flags = (_bLeave ? VSSFLAG_REPSKIP : VSSFLAG_REPREPLACE);

	return _pSS->CheckOut(pathFile, flags);
}

BOOL CSscSystemWrapper::_GetFileLatestVersion(const char* pathFile, BOOL bSilent)
{
	const long VSSFLAG_REPREPLACE = 128;
	const long VSSFLAG_REPSKIP = 192;
	const long VSSFLAG_REPMERGE = 256;

	if (!bSilent && !_bApplyAllItems)
	{
		std::string workingPath;
		_GetWorkingPath(pathFile, workingPath);
		if (_IsWritable(workingPath.c_str()))
		{
			CDlgSscChoose dlgChoose(CWnd::FromHandle(_hParentWnd));
			dlgChoose._strContent = workingPath.c_str();
			if (dlgChoose.DoModal() != IDOK)
			{
				return FALSE;
			}

			_bLeave = dlgChoose._bLeave;
			_bApplyAllItems = dlgChoose._bApplyAllItems;
		}		
	}	

	long flags = (_bLeave ? VSSFLAG_REPSKIP : VSSFLAG_REPREPLACE);

	return _pSS->GetLatestVersion(pathFile, flags);
}

void CSscSystemWrapper::_GetWorkingPath(const char* pathFolderOrFile, std::string& rWorkingPath)
{
	std::string validPath = pathFolderOrFile;
	int len = static_cast<int>(validPath.length());
	if ((--len > 0) && (validPath[len] == *WIN32_DELIM))
		validPath.erase(len);

	if (validPath.find(GetWorkingFolder()) == 0)
	{
		rWorkingPath = validPath;
	}
	else
	{
		rWorkingPath = GetWorkingFolder();
		rWorkingPath += validPath;
	}
}

void CSscSystemWrapper::_GetProjectPath(const char* pathFolderOrFile, std::string& rProjectPath)
{
	std::string validPath = pathFolderOrFile;
	int len = static_cast<int>(validPath.length());
	if ((--len > 0) && (validPath[len] == *WIN32_DELIM))
		validPath.erase(len);

	rProjectPath = GetCurrentProject();

	if (validPath.find(GetWorkingFolder()) == 0)
	{
		std::string t = validPath.substr(strlen(GetWorkingFolder()));
		validPath = t;
	}
	_ReplaceString(validPath, std::string(WIN32_DELIM), std::string(VSS_DELIM));
	rProjectPath += validPath;
}

void CSscSystemWrapper::_ReplaceString(std::string& str, const std::string& src, const std::string& dest)
{
	std::string::size_type off = 0;
	std::string::size_type pos;
	std::string::size_type num = dest.length();
	while ((pos = str.find_first_of(src, off)) != std::string::npos)
	{
		str.replace(pos, num, dest);
		off = pos + num;
	}
}

inline BOOL CSscSystemWrapper::_IsWritable(const char* pathFolderOrFile)
{
	DWORD dwAttrs = GetFileAttributes(fromMBCS(pathFolderOrFile));
	return ((dwAttrs != INVALID_FILE_ATTRIBUTES) && 
		!(dwAttrs & FILE_ATTRIBUTE_DIRECTORY) &&
		!(dwAttrs & FILE_ATTRIBUTE_READONLY));
}