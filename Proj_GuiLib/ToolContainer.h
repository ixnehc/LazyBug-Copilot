#pragma  once

#include "ToolBase.h"

class ToolContainer
{
public:	
	ToolContainer();
	int  GetNumberOfTools();
	CToolBase * GetTool(int idx);
	void RegisterToolClass(CClass * cls);
	//
	int GetNumberOfTypes();
	const char * GetTypeName(int idx);

	void InitializeTools(DWORD typeTool);
	void InitializeTools(const char * tools);

	void Release();
	void UpdateUI(CGeActor * actor);

protected:
	void RegisterTool(CToolBase * tool);
	void _Clean();

	struct ToolGrp
	{
		std::string strCategory;
		std::vector<int> idx_tools;
	};
protected:
	std::vector<CToolBase *> _tools;
	std::vector<ToolGrp> _grps;
	std::map<std::string,int> _grps_idx;
};


