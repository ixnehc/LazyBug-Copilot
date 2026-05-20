/********************************************************************
	created:	2008/02/22
	created:	22:2:2008   16:08
	filename: 	f:\IxEngine\Proj_FileSystem\SscSystem.cpp
	file path:	f:\IxEngine\Proj_FileSystem
	file base:	SscSystem
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"
#include "SscSystem.h"
#include "stringparser/stringparser.h"
#include <ShellAPI.h>

#include "interface/interface.h"
#include "Timer/profiler.h"

//EXPOSE_SINGLE_INTERFACE(CSscSystem,ISscSystem,"SscSystem01")

CSscSystem::CSscSystem() : _bConnected(FALSE)
{
	GetDefaultOutputFileName(_outputFileName);
	CreateOutputFile();

	_resultParser.SetResultFileName(_outputFileName);

	GetSsExePath(_ssExePath);
}

CSscSystem::~CSscSystem()
{
	DeleteOutputFile();
}

BOOL CSscSystem::Connect(const char *pathServer,const char *user,const char *pwd)
{
	const char* const EV_SSDIR = "ssdir=";
	const char* const EV_SSUSER = "ssuser=";
	const char* const EV_SSPWD = "sspwd=";
	if (pathServer && user && pwd)
	{
		_user = EV_SSUSER;
		_user += user;

		_pwd = EV_SSPWD;
		_pwd += pwd;

		_server = EV_SSDIR;
		_server += pathServer;

		// Test VSS
		if (TestVss())
			return (_bConnected = TRUE);

		_user.clear();
		_pwd.clear();
		_server.clear();
		_bConnected = FALSE;
	}
	return FALSE;
}

BOOL CSscSystem::IsConnected() const
{
	return _bConnected;
}

int CSscSystem::GetUserName(char* user, int nCount)
{
	int len = 0;
	if (IsConnected())
	{
		len = static_cast<int>(_user.length()) + 1;
		if (nCount >= len)
			strncpy(user, _user.c_str(), len);
	}
	return len;
}

BOOL CSscSystem::ListProject(const char* pathProject, const char**& items, int& nCount)
{
	static std::vector<const char*> ProjItems;
	static ProjItemList StrProjItems;

	// Clear last result
	ProjItems.clear();
	StrProjItems.clear();

	if (IsConnected() && GetProjectItems(pathProject, StrProjItems))
	{
		int c = static_cast<int>(StrProjItems.size());
		for (int i = 0; i < c; i++)
		{
			ProjItems.push_back(StrProjItems[i].c_str());
		}
		items  = (const char**)&ProjItems[0];
		nCount = c;
		return TRUE;
	}
	nCount = 0;
	return FALSE;
}

BOOL CSscSystem::SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject)
{
	if (IsConnected() && pathProject && pathWorkingFolder)
	{
		String validFolder(pathWorkingFolder);
		if (!IsDirectory(validFolder))
			return FALSE;

		// Remove the end '\'.
		int len = static_cast<int>(validFolder.length());
		if ((--len > 0) && (validFolder[len] == *WIN32_DELIM))
			validFolder.erase(len);

		String validProject(pathProject);
		len = static_cast<int>(validProject.length());
		if ((--len > 0) && (validProject[len] == *WIN32_DELIM))
			validProject.erase(len);

		String project;
		project += DOUBLEQUOTES;
		project += validProject;
		project += DOUBLEQUOTES;

		String workingFolder;
		workingFolder += DOUBLEQUOTES;
		workingFolder += validFolder;
		workingFolder += DOUBLEQUOTES;

		const char* options[3] = { project.c_str(), workingFolder.c_str(), NULL };	

		if (0 == ExecSscCommand(VSS_WORKFOLD, options))
		{
			_currentProject = validProject;
			_currentProject += *VSS_DELIM;

			_workingFolder = validFolder;
			_workingFolder += *WIN32_DELIM;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSscSystem::IsControlled(const char *pathFolderOrFile)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		return (VSS_E_FAIL != ExecSscCommand(VSS_STATUS, options));
	}
	return FALSE;
}

BOOL CSscSystem::CheckIn(const char *pathFolderOrFile,BOOL *bModified)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		String gOption;
		String workingPath;
		GetWorkingPath(pathFolderOrFile, workingPath);
		if (!IsProject(pathFolderOrFile))
		{
			String prevWorkingDir;
			GetPrevPath(workingPath, prevWorkingDir);
			FormatString(gOption, "%s\"%s\"", VSS_OPTIONS_G, prevWorkingDir.c_str());
		}
		else
			FormatString(gOption, "%s\"%s\"", VSS_OPTIONS_G, workingPath.c_str());

		const char* options[5] = { path.c_str(), gOption.c_str(), VSS_OPTIONS_C, VSS_OPTIONS_R, NULL };

		if (VSS_E_OK == ExecSscCommand(VSS_CHECKIN, options))
			return TRUE;

		path = pathFolderOrFile;
		if (path.find(TAG_DIR) == String::npos || path.find(_workingFolder) == 0)
		{
			return AddProject(pathFolderOrFile);
		}
	}
	return FALSE;
}

BOOL CSscSystem::CheckOut(const char *pathFolderOrFile,BOOL *bModified)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		String workingPath;
		GetWorkingPath(pathFolderOrFile, workingPath);

		// If not, create directory
		// Is a project, should create a local working folder
		if (IsProject(pathFolderOrFile))
		{
			int len = static_cast<int>(workingPath.length());
			if ((--len > 0) && (workingPath[len] != *WIN32_DELIM))
				workingPath += *WIN32_DELIM;
		}

		String lastDir;
		if (CreateDirectories(workingPath, lastDir))
		{
			String gOption;
			FormatString(gOption, "%s\"%s\"", VSS_OPTIONS_G, lastDir.c_str());

			const char* options[5] = { path.c_str(), gOption.c_str(), VSS_OPTIONS_C, VSS_OPTIONS_R, NULL };

			return (VSS_E_OK == ExecSscCommand(VSS_CHECKOUT, options));
		}		
	}
	return FALSE;
}

BOOL CSscSystem::GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		String workingPath;
		GetWorkingPath(pathFolderOrFile, workingPath);

		// If not, create directory
		// Is a project, should create a local working folder
		if (IsProject(pathFolderOrFile))
		{
			int len = static_cast<int>(workingPath.length());
			if ((--len > 0) && (workingPath[len] != *WIN32_DELIM))
				workingPath += *WIN32_DELIM;
		}

		String lastDir;
		if (CreateDirectories(workingPath, lastDir))
		{
			String gOption;
			FormatString(gOption, "%s\"%s\"", VSS_OPTIONS_G, lastDir.c_str());

			const char* options[4] = { path.c_str(), gOption.c_str(), VSS_OPTIONS_R, NULL };

			return (VSS_E_OK == ExecSscCommand(VSS_GET, options));
		}
	}
	return FALSE;
}

BOOL CSscSystem::GetState(const char *pathFile,SscState &state)
{
	if (IsConnected() && pathFile)
	{
		String proj;
		GetProjectPath(pathFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		DWORD dwRet = ExecSscCommand(VSS_STATUS, options);
		if (dwRet != VSS_E_FAIL)
		{
			if (dwRet == VSS_E_WARNING)
			{
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CSscSystem::Delete(const char *pathFolderOrFile)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		return (VSS_E_OK == ExecSscCommand(VSS_DELETE, options));
	}
	return FALSE;
}

BOOL CSscSystem::Rename(const char *pathFolderOrFile, const char *pszNewName)
{
	if (IsConnected() && pathFolderOrFile && pszNewName)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		String workingPath;
		workingPath += DOUBLEQUOTES;
		workingPath += pszNewName;
		workingPath += DOUBLEQUOTES;

		const char* options[3] = { path.c_str(), workingPath.c_str(), NULL };

		if (VSS_E_OK == ExecSscCommand(VSS_RENAME, options))
		{			
			String newWorkingPath;
			GetWorkingPath(pathFolderOrFile, workingPath);
			GetPrevPath(workingPath, newWorkingPath);
			newWorkingPath += *WIN32_DELIM;
			newWorkingPath += pszNewName;
			return RenameFile(workingPath, newWorkingPath);
		}
	}
	return FALSE;
}

const char* CSscSystem::GetWorkingFolder() const
{
	return _workingFolder.c_str();
}

const char* CSscSystem::GetCurrentProject() const
{
	return _currentProject.c_str();
}

BOOL CSscSystem::TestVss()
{
	const char* options[2] = { VSS_ROOT, NULL };

	return (VSS_E_OK == ExecSscCommand(VSS_CP, options, VSS_CONNECT_TIMEOUT));
}

BOOL CSscSystem::Whoami()
{
	return (VSS_E_OK == ExecSscCommand(VSS_WHOAMI, NULL, VSS_CONNECT_TIMEOUT));
}

BOOL CSscSystem::GetProjectItems(const char* pathProject, ProjItemList& items)
{
	if (IsConnected() && pathProject)
	{
		String project;
		project += DOUBLEQUOTES;
		project += pathProject;
		project += DOUBLEQUOTES;

		const char* options[4] = { project.c_str(), VSS_OPTIONS_F, VSS_OPTIONS_R, NULL };	

		_resultParser.ClearLastResult();
		if (VSS_E_OK == ExecSscCommand(VSS_DIR, options) && _resultParser.GetProjectItemsList(items))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSscSystem::SetCurrentProject(const char* proj)
{
	if (IsConnected() && proj)
	{
		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		return (VSS_E_OK == ExecSscCommand(VSS_CP, options));
	}
	return FALSE;
}

BOOL CSscSystem::IsProject(const char *pathFolderOrFile)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[3] = { path.c_str(), VSS_OPTIONS_F, NULL };

		_resultParser.ClearLastResult();
		if (VSS_E_FAIL != ExecSscCommand(VSS_PROPERTIES, options))
		{
			return _resultParser.IsProject(proj);
		}
	}
	return FALSE;
}

BOOL CSscSystem::AddProject(const char *pathFolderOrFile)
{
	if (IsConnected() && pathFolderOrFile)
	{
		String proj;
		String prevProj;
		GetProjectPath(pathFolderOrFile, proj);
		GetPrevPath(proj, prevProj, VSS_DELIM);
		if (!SetCurrentProject(prevProj.c_str()))
			return FALSE;

		String workingPath;
		GetWorkingPath(pathFolderOrFile, workingPath);

		String path;
		path += DOUBLEQUOTES;
		path += workingPath;
		path += DOUBLEQUOTES;

		const char* options[5] = { path.c_str(), VSS_OPTIONS_C, VSS_OPTIONS_B, VSS_OPTIONS_R, NULL };

		return (VSS_E_OK == ExecSscCommand(VSS_ADD, options));
	}
	return FALSE;
}

void CSscSystem::GetWorkingPath(const char* pathFolderOrFile, String& rWorkingPath)
{
	String validPath = pathFolderOrFile;
	if (validPath.find(_workingFolder) == 0)
	{
		rWorkingPath = validPath;
	}
	else
	{
		rWorkingPath = _workingFolder;
		rWorkingPath += validPath;
	}
}

void CSscSystem::GetProjectPath(const char* pathFolderOrFile, String& rProjectPath)
{
	String validPath = pathFolderOrFile;
	int len = static_cast<int>(validPath.length());
	if ((--len > 0) && (validPath[len] == *WIN32_DELIM))
		validPath.erase(len);

	rProjectPath = _currentProject;
	if (validPath.find(_workingFolder) == 0)
	{
		String t = validPath.substr(_workingFolder.length());
		validPath = t;
	}
	ReplaceString(validPath, String(WIN32_DELIM), String(VSS_DELIM));
	rProjectPath += validPath;

}

BOOL CSscSystem::CreateOutputFile()
{
	HANDLE hFile = CreateFile(_outputFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		CREATE_ALWAYS, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}

void CSscSystem::DeleteOutputFile()
{
	DeleteFile(_outputFileName.c_str());
}

DWORD CSscSystem::ExecSscCommand(const char* cmd, const char* options[], DWORD dwTimeout)
{
	DWORD dwExitCode = VSS_E_FAIL;

	String commandLine;
	FormatString(commandLine, "\"%s\"", _ssExePath.c_str());
	commandLine += " ";
	commandLine += cmd;
	if (options)
	{
		const char** opt = options;
		for(; *opt != NULL; opt++)
		{
			commandLine += " ";
			commandLine += *opt;
		}
		commandLine += " ";
		commandLine += VSS_OPTIONS_I;
		commandLine += " ";
		commandLine += VSS_OPTIONS_N;

		String optOutput;
		FormatString(optOutput, "%s\"%s\"", VSS_OPTIONS_O, _outputFileName.c_str());
		commandLine += " ";
		commandLine += optOutput;
	}	
	
	const int ENVP_LENGTH = 512;
	char szEnvp[ENVP_LENGTH] = { 0 };
	char* p = szEnvp;
	strcpy(p, _server.c_str());
	p += static_cast<int>(_server.length()) + 1;
	strcpy(p, _user.c_str());
	p += static_cast<int>(_user.length()) + 1;
	strcpy(p, _pwd.c_str());
	p += static_cast<int>(_pwd.length()) + 1;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	ProfilerStart(SscSystem_ExecuteCommand);
	BOOL bRet = CreateProcess(NULL, (LPSTR) commandLine.c_str(), NULL, NULL, 
		FALSE, 0, szEnvp, NULL, &si, &pi);
	if (bRet)
	{
		// Wait until child process exits.
		dwExitCode = WaitForSingleObject( pi.hProcess, dwTimeout );
		if (dwExitCode != WAIT_OBJECT_0)
		{
			TerminateProcess(pi.hProcess, VSS_E_FAIL);
			dwExitCode = VSS_E_FAIL;
		}
		else
			GetExitCodeProcess(pi.hProcess, &dwExitCode);

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );		
	}
	ProfilerEnd();
	return dwExitCode;
}

const String& CSscSystem::GetDefaultOutputFileName(String& outputFile)
{
	const char* const VSS_OUTPUT_FILENAME = "SscOutput";

	char szModulePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szModulePath, MAX_PATH);
	char* p = strrchr(szModulePath, '\\');
	if (p)
		*(++p) = '\0';

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	FormatString(outputFile, "%s%s%04d%02d%02d%02d%02d%02d.txt", szModulePath, VSS_OUTPUT_FILENAME, 
		sysTime.wYear, sysTime.wMonth, sysTime.wDay, 
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	return outputFile;
}

BOOL CSscSystem::GetSsExePath(String& ssExePath)
{
	// open the registry key
	HKEY hKey;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\SourceSafe", 0, KEY_ALL_ACCESS, &hKey);
	if (lRet!= ERROR_SUCCESS)
	{
		return FALSE;
	}

	// set the value to current application
	char achrKeyValue[MAX_PATH] = { 0 };
	ULONG ctType;
	ULONG ctLength = sizeof(achrKeyValue);
	lRet = RegQueryValueEx(hKey, "SCCServerPath", 0, &ctType, (unsigned char *)achrKeyValue, &ctLength);
	if( lRet != ERROR_SUCCESS)
	{
		return FALSE;
	}

	char* p = strrchr(achrKeyValue, '\\');
	if (p)
		*(++p) = '\0';
	strcat(achrKeyValue, VSS_EXE);
	ssExePath = achrKeyValue;

	return TRUE;
}

void CSscSystem::ReplaceString(String& str, const String& src, const String& dest)
{
	String::size_type off = 0;
	String::size_type pos;
	String::size_type num = dest.length();
	while ((pos = str.find_first_of(src, off)) != String::npos)
	{
		str.replace(pos, num, dest);
		off = pos + num;
	}
}

int CSscSystem::SplitPath(const String& path, SubDirList& subdirs, const String& delim)
{
	String::size_type off = 0;
	String::size_type pos;
	String::size_type len = delim.length();
	while ((pos = path.find_first_of(delim, off)) != String::npos)
	{
		subdirs.push_back(path.substr(0, pos));
		off = pos + len;
	}
	return static_cast<int>(subdirs.size());
}

inline void CSscSystem::GetPrevPath(const String& path, String& prev, const String& delim)
{
	String::size_type len = path.length();
	String::size_type pos;
	if ((pos = path.rfind(delim, len)) != String::npos)
	{
		if (pos == (len - delim.length()))
		{
			if ((pos = path.rfind(delim, pos)) != String::npos)
				prev = path.substr(0, pos);
		}
		else
			prev = path.substr(0, pos);	
	}
}

inline void CSscSystem::GetFileName(const String& path, String& fileName, const String& delim)
{
	String::size_type len = path.length();
	String::size_type pos;
	if ((pos = path.rfind(delim, len)) != String::npos)
	{
		if (pos == (len - delim.length()))
		{
			if ((pos = path.rfind(delim, pos)) != String::npos)
			{
				len -= (pos + delim.length());
				fileName = path.substr(pos, len);
			}
		}
		else
			fileName = path.substr(++pos);	
	}
}

inline BOOL CSscSystem::IsDirectory(const String& path)
{
	DWORD dwAttr = GetFileAttributes(path.c_str());
	return ((dwAttr != INVALID_FILE_ATTRIBUTES) && 
		(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL CSscSystem::CreateDirectories(const String& path, String& lastDir)
{
	BOOL bCreated = FALSE;
	SubDirList subdirs;
	int count = SplitPath(path, subdirs);
	bCreated = (count > 1);
	for (int i = 1; i < count; i++)
	{
		const String& dir = subdirs[i];
		if (!IsDirectory(dir) && 
			!CreateDirectory(dir.c_str(), NULL))
		{
			bCreated = FALSE;
			break;
		}
	}
	if (bCreated)
		lastDir = subdirs[--count];

	return bCreated;
}

BOOL CSscSystem::RenameFile(const String& fileName, const String& newFileName)
{
	char szFileName[MAX_PATH] = { 0 };
	char szNewFileName[MAX_PATH] = { 0 };
	strcpy(szFileName, fileName.c_str());
	strcpy(szNewFileName, newFileName.c_str());
	SHFILEOPSTRUCT fo;
	fo.hwnd = NULL;
	fo.wFunc = FO_RENAME;
	fo.pFrom = szFileName;
	fo.pTo = szNewFileName;
	fo.hNameMappings = NULL;
	fo.lpszProgressTitle = NULL;
	fo.fAnyOperationsAborted = FALSE;
	fo.fFlags = FOF_NOERRORUI | FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
	return (0 == ::SHFileOperation(&fo));
}

String CSscSystem::SetCurDirectory(const String& dir)
{
	String oldDir;
	char szDir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szDir);
	oldDir = szDir;
	SetCurrentDirectory(dir.c_str());
	return oldDir;
}