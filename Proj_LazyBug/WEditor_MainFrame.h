#pragma once

#include "engine/Engine.h"

#include "weditor/WEditor.h"

#include "editor/editor.h"


// #include "GuiData_database.h"


//XXXXX:more GuiView

// #include "GuiActor_database.h"

// #include "GuiActor_proto.h"

//#include "SolutionDB.h"
//#include "CppSymbol.h"
// #include "Changelists.h"
#include "Checkpoints.h"
#include "BackupDepot.h"
#include "LspClient.h"


class CWEditor_MainFrame:public CWorldEditor
{
public:
	virtual BOOL Create(CXTPDockingPaneManager *panemgr);
	virtual void Destroy();
	virtual BOOL LoadContent(const char* dbFolderPath);
	virtual void ResetContent();
	const char* GetDBFolderPath()	{		return _dbFolderPath.c_str();	}
	void Update();
	void RequestComplete(const char* filePath, const char* fileContent, int line,int column);

// 	CChangelists& GetChangelists()	{		return _changelists;	}
	CCheckpoints& GetCheckpoints() { return _checkpoints; }
	CBackupDepot& GetBackupDepot() { return _backupDepot; }

// 	CGuiPanel_Db& GetPanelDB()	{		return _panelDB;	}
	
	// 初始化LspClient
// 	bool InitLspClient();
	
	// 获取LspClient实例
// 	CLspClient& GetLspClient() { return _lspClient; }
	
	
protected:
	std::string _dbFolderPath;
// 	GuiData_Database _dataDB;
// 	CDockPaneWnd<CGuiPanel_Db> _panelDB;
// 	CDockPaneWnd<CGuiPanel_Proto> _panelProto;

	CCheckpoints _checkpoints;
	CBackupDepot _backupDepot;
// 	CChangelists _changelists;
// 	CLspClient _lspClient;

private:
	struct ParseState
	{
		std::string currentFileName;
		bool isParsing;
	};
	ParseState _parseState;

	friend class CMainFrame;
};



