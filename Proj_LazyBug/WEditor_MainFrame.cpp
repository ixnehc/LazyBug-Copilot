/********************************************************************
	created:	2007/2/15   13:04
	filename: 	e:\IxEngine\Proj_WorldEditor\WEditors.cpp
	author:		cxi
	
	purpose:	world editor construction
*********************************************************************/

#include "stdh.h"

#include <fileapi.h>

#include "WEditor_MainFrame.h"

#include "stringparser/stringparser.h"


std::string GetFinalFilePath(const char* inputPath)
{
	// 打开文件获取句柄
	HANDLE hFile = CreateFileA(
		inputPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// 无法打开文件，可能不存在或权限问题
		return std::string("");
	}

	// 准备缓冲区
	char buffer[MAX_PATH + 1] = { 0 };
	DWORD length = GetFinalPathNameByHandle(
		hFile,
		buffer,
		MAX_PATH,
		FILE_NAME_NORMALIZED
	);

	// 关闭文件句柄
	CloseHandle(hFile);

	if (length == 0 || length > MAX_PATH)
	{
		// 获取路径失败或路径太长
		return std::string("");
	}

	std::string result = buffer;

	// 如果路径以 "\\?\" 前缀开始，去掉它
	if (result.substr(0, 4) == "\\\\?\\")
	{
		result = result.substr(4);
	}

	return result;
}


BOOL CWEditor_MainFrame::Create(CXTPDockingPaneManager *panemgr)
{
	CWorldEditor::Create(panemgr);


//	_AddPanel<CGuiPanel_Db>(&_panelDB,IDR_EDITORPANEL_DB,0,xtpPaneDockLeft);
	_SetPanelIcons(IDB_PANELICONS_PROTOLIB);

	return TRUE;
}

void CWEditor_MainFrame::Destroy()
{
	ResetContent();

//	_lspClient.Shutdown();

	_ClearPanels();
}


BOOL CWEditor_MainFrame::LoadContent(const char* dbFolderPath)
{
	_dbFolderPath = dbFolderPath;
	_checkpoints.Init(dbFolderPath);
	_backupDepot.Init(dbFolderPath);

// 	_changelists.Init(&_db);


//	_lspClient.Initialize(_db._setting.pathWorkspace.c_str(), _db._pathDBFolder.c_str());

	//////////////////////////////////////////////////////////////////////////
	//register 
//	_mgr.RegisterData(&_dataDB);
	//XXXXX:more GuiData

	//XXXXX:more GuiView

//	_mgr.RegisterActor(&_panelDB);
	//XXXXX:More EditorPanels

	//////////////////////////////////////////////////////////////////////////
	//Start the actors
//	_panelDB.Reset();
	//XXXXX:More EditorPanels



	return TRUE;

}

void CWEditor_MainFrame::RequestComplete(const char* filePath, const char* fileContent, int line, int column)
{
//	_lspClient.UpdateDocument(filePath, GetAbsTick(),fileContent);
// 	Sleep(1);
//	_lspClient.RequestCompletion(filePath,line,column);

}

void CWEditor_MainFrame::Update()
{
}

void CWEditor_MainFrame::ResetContent()
{
	_mgr.Reset();

	// 重置解析状态
	_parseState.isParsing = false;
	_parseState.currentFileName.clear();

	_dbFolderPath = "";

	//reset actors
//	_panelDB.Reset();
	//XXXXX:More EditorPanels


	//XXXXX:more GuiView

}

// 初始化LspClient
// bool CWEditor_MainFrame::InitLspClient()
// {
// 	// 如果工作区路径为空，表示未加载项目
// 	if (_db._setting.pathWorkspace.empty())
// 	{
// 		return false;
// 	}
// 	
// 	// 创建缓存目录路径
// 	std::string cachePath = _db._pathDBFolder + "\\_clangd";
// 	// 确保缓存目录存在
// 	if (g_Engine.GetFS())
// 	{
// 		IFileSystem_EnsureFolderAbs(g_Engine.GetFS(), cachePath.c_str());
// 	}
// 	
// 	// 初始化LspClient
// 	return _lspClient.Initialize(_db._setting.pathWorkspace, _db._pathDBFolder);
// }
// 


