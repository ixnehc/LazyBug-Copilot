#include "stdafx.h"
#include "VssExecuteCommand.h"
#include <process.h>
asfdasdfasfsdfasdf
CVssExecuteCommand::CVssExecuteCommand()
{
	GetDefaultOutputFileName(_outputFileName);
	CreateOutputFile();

	_resultParser.SetResultFileName(_outputFileName);

	GetSsExePath(_ssExePath);
}

CVssExecuteCommand::~CVssExecuteCommand()
{
	DeleteOutputFile();
}

BOOL CVssExecuteCommand::SetSscServer(const char *pathServer,const char *user,const char *pwd)
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

		return TRUE;
	}
	return FALSE;
}

BOOL CVssExecuteCommand::ListProject(const char* pathProject, const char**& items, int& nCount)
{
	static std::vector<const char*> ProjItems;
	static ProjectItemList ProjItemList;

	// Clear last result
	ProjItems.clear();
	ProjItemList.clear();

	if (GetProjectItems(pathProject, ProjItemList))
	{
		int c = static_cast<int>(ProjItemList.size());
		for (int i = 0; i < c; i++)
		{
			ProjItems.push_back(ProjItemList[i].c_str());
		}
		items  = (const char**)&ProjItems[0];
		nCount = c;
		return TRUE;
	}
	nCount = 0;
	return FALSE;
}

BOOL CVssExecuteCommand::SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject)
{
	if (pathProject && pathWorkingFolder)
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

		if (0 == ExecVssCommand(VSS_WORKFOLD, options))
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

BOOL CVssExecuteCommand::IsControlled(const char *pathFolderOrFile)
{
	if (pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

        const char* options[2] = { path.c_str(), NULL };

		return (VSS_E_FAIL != ExecVssCommand(VSS_STATUS, options));
	}
	return FALSE;
}

BOOL CVssExecuteCommand::CheckIn(const char *pathFolderOrFile,BOOL *bModified)
{
	if (pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		String workingPath;
		String prevWorkingDir;
		GetWorkingPath(pathFolderOrFile, workingPath);
		GetPrevPath(workingPath, prevWorkingDir);

		char gOption[MAX_PATH] = { 0 };
		sprintf(gOption, "%s\"%s\"", VSS_OPTIONS_G, prevWorkingDir.c_str());

		const char* options[5] = { path.c_str(), gOption, VSS_OPTIONS_C, VSS_OPTIONS_R, NULL };

		if (VSS_E_OK == ExecVssCommand(VSS_CHECKIN, options))
			return TRUE;

		path = pathFolderOrFile;
		if (path.find(TAG_DIR) == String::npos || path.find(_workingFolder) == 0)
		{
			return AddProject(pathFolderOrFile);
		}
	}
	return FALSE;
}

BOOL CVssExecuteCommand::CheckOut(const char *pathFolderOrFile,BOOL *bModified)
{
	if (pathFolderOrFile)
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
			char gOption[MAX_PATH] = { 0 };
			sprintf(gOption, "%s\"%s\"", VSS_OPTIONS_G, lastDir.c_str());

			const char* options[5] = { path.c_str(), gOption, VSS_OPTIONS_C, VSS_OPTIONS_R, NULL };

			return (VSS_E_OK == ExecVssCommand(VSS_CHECKOUT, options));
		}		
	}
	return FALSE;
}

BOOL CVssExecuteCommand::GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified)
{
	if (pathFolderOrFile)
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
			char gOption[MAX_PATH] = { 0 };
			sprintf(gOption, "%s\"%s\"", VSS_OPTIONS_G, lastDir.c_str());

			const char* options[4] = { path.c_str(), gOption, VSS_OPTIONS_R, NULL };

			return (VSS_E_OK == ExecVssCommand(VSS_GET, options));
		}
	}
	return FALSE;
}

BOOL CVssExecuteCommand::GetState(const char *pathFile,SscState &state)
{
	state.bControlled = FALSE;
	state.bCheckOut = FALSE;
	state.owner.clear();
	if (pathFile)
	{
		String proj;
		GetProjectPath(pathFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		_resultParser.ClearLastResult();
		DWORD dwRet = ExecVssCommand(VSS_STATUS, options);
		if (dwRet != VSS_E_FAIL)
		{
			if (dwRet == VSS_E_WARNING)
				_resultParser.GetState(state);
			else
				state.bControlled = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CVssExecuteCommand::GetProjectItems(const char* pathProject, ProjectItemList& items)
{
	if (pathProject)
	{
		String project;
		project += DOUBLEQUOTES;
		project += pathProject;
		project += DOUBLEQUOTES;

		const char* options[4] = { project.c_str(), VSS_OPTIONS_F, VSS_OPTIONS_R, NULL };	

		_resultParser.ClearLastResult();
		if (0 == ExecVssCommand(VSS_DIR, options) && _resultParser.GetProjectItemsList(items))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CVssExecuteCommand::SetCurrentProject(const char* proj)
{
	if (proj)
	{
		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[2] = { path.c_str(), NULL };

		return (VSS_E_OK == ExecVssCommand(VSS_CP, options));
	}
	return FALSE;
}

BOOL CVssExecuteCommand::IsProject(const char *pathFolderOrFile)
{
	if (pathFolderOrFile)
	{
		String proj;
		GetProjectPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[3] = { path.c_str(), VSS_OPTIONS_F, NULL };

		_resultParser.ClearLastResult();
		if (VSS_E_FAIL != ExecVssCommand(VSS_PROPERTIES, options))
		{
			return _resultParser.IsProject(proj);
		}
	}
	return FALSE;
}

BOOL CVssExecuteCommand::AddProject(const char *pathFolderOrFile)
{
	if (pathFolderOrFile)
	{
		if (!SetCurrentProject(_currentProject.c_str()))
			return FALSE;

		String proj;
		GetWorkingPath(pathFolderOrFile, proj);

		String path;
		path += DOUBLEQUOTES;
		path += proj;
		path += DOUBLEQUOTES;

		const char* options[5] = { path.c_str(), VSS_OPTIONS_C, VSS_OPTIONS_B, VSS_OPTIONS_R, NULL };

		return (VSS_E_OK == ExecVssCommand(VSS_ADD, options));
	}
	return FALSE;
}

void CVssExecuteCommand::GetWorkingPath(const char* pathFolderOrFile, String& rWorkingPath)
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

void CVssExecuteCommand::GetProjectPath(const char* pathFolderOrFile, String& rProjectPath)
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

BOOL CVssExecuteCommand::CreateOutputFile()
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

void CVssExecuteCommand::DeleteOutputFile()
{
	DeleteFile(_outputFileName.c_str());
}

DWORD CVssExecuteCommand::ExecVssCommand(const char* cmd, const char* options[])
{
	const int CMDLINE_LENGTH	= 1024;
	const int ENVP_LENGTH		= 512;

	DWORD dwExitCode = VSS_E_FAIL;

	char szCommandLine[CMDLINE_LENGTH] = { 0 };
	sprintf(szCommandLine, "\"%s\"", _ssExePath.c_str());
	strcat(szCommandLine, " ");
	strcat(szCommandLine, cmd);
	const char** opt = options;
	for(; *opt != NULL; opt++)
	{
		strcat(szCommandLine, " ");
		strcat(szCommandLine, *opt);
	}
	strcat(szCommandLine, " ");
	strcat(szCommandLine, VSS_OPTIONS_I);
	strcat(szCommandLine, " ");
	strcat(szCommandLine, VSS_OPTIONS_N);
	char szOptOutput[MAX_PATH] = { 0 };
	sprintf(szOptOutput, "%s\"%s\"", VSS_OPTIONS_O, _outputFileName.c_str());
	strcat(szCommandLine, " ");
	strcat(szCommandLine, szOptOutput);
	
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

	BOOL bRet = CreateProcess(NULL, szCommandLine, NULL, NULL, 
		FALSE, 0, szEnvp, NULL, &si, &pi);
	if (bRet)
	{
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );		
	}
	return dwExitCode;
}

const String& CVssExecuteCommand::GetDefaultOutputFileName(String& outputFile)
{
	const char* const VSS_OUTPUT_FILENAME = "VssOutput";

	static char szFileName[MAX_PATH] = { 0 };
	if (szFileName[0] == 0)
	{
		char szModulePath[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, szModulePath, MAX_PATH);
		char* p = strrchr(szModulePath, '\\');
		if (p)
			*(++p) = '\0';

		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		sprintf(szFileName, "%s%s%04d%02d%02d%02d%02d%02d.txt", szModulePath, VSS_OUTPUT_FILENAME, 
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, 
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	}	
	return outputFile = szFileName;
}

BOOL CVssExecuteCommand::GetSsExePath(String& ssExePath)
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

void CVssExecuteCommand::ReplaceString(String& str, const String& src, const String& dest)
{
	String::size_type off = 0;
	String::size_type pos;
	String::size_type num = dest.length();
	while ((pos = str.find(src, off)) != String::npos)
	{
		str.replace(pos, num, dest);
		off = pos + num;
	}
}

int CVssExecuteCommand::SplitPath(const String& path, SubDirList& subdirs, const String& delim)
{
	String::size_type off = 0;
	String::size_type pos;
	String::size_type len = delim.length();
	while ((pos = path.find(delim, off)) != String::npos)
	{
		subdirs.push_back(path.substr(0, pos));
		off = pos + len;
	}
	return static_cast<int>(subdirs.size());
}

inline void CVssExecuteCommand::GetPrevPath(const String& path, String& prev, const String& delim)
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

inline BOOL CVssExecuteCommand::IsDirectory(const String& path)
{
	DWORD dwAttr = GetFileAttributes(path.c_str());
	return ((dwAttr != INVALID_FILE_ATTRIBUTES) && 
			(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL CVssExecuteCommand::CreateDirectories(const String& path, String& lastDir)
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

String CVssExecuteCommand::SetCurDirectory(const String& dir)
{
	String oldDir;
	char szDir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szDir);
	oldDir = szDir;
    SetCurrentDirectory(dir.c_str());
	return oldDir;
}