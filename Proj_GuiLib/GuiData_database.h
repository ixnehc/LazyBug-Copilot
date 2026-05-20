#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "FileSystem/IFileSystem.h"

#include <unordered_map>

class CVcxprojDatabaseCore;
struct GuiLib_Api GuiData_Database :public GeData
{
	virtual const char* GetName() { return "database"; }

	GuiData_Database()
	{
		Zero();
	}

	void Zero()
	{
		db = NULL;
	}

	void Clear()
	{
		Zero();
	}

	CVcxprojDatabaseCore* db;

	// 	ChangelistID protoid;
	// 
	// 	std::vector<ChangelistNodeID> sels;

};




struct GuiLib_Api GuiData_Changelist:public GeData
{
	virtual const char *GetName()	{		return "changelist";	}

	GuiData_Changelist()
	{
		Zero();
	}

	void Zero()
	{
	}

	void Clear()
	{
		Zero();
	}


// 	ChangelistID protoid;
// 
// 	std::vector<ChangelistNodeID> sels;

};


