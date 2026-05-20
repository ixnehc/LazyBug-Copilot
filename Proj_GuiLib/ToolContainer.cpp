
#include "stdh.h"
#include "ToolContainer.h"
#include "stringparser/stringparser.h"

ToolContainer::ToolContainer()
{
}
int ToolContainer::GetNumberOfTools()
{
	return int(_tools.size());
}
int ToolContainer::GetNumberOfTypes()
{
	return int(_grps.size());
}
const char * ToolContainer::GetTypeName(int idx)
{
	if(idx<0||idx>=_grps.size())
		return NULL;
	return _grps[idx].strCategory.c_str();
}
CToolBase * ToolContainer::GetTool(int idx)
{
	if(idx<0||idx>=_tools.size())
		return NULL;
	else
		return _tools[idx];
}
void ToolContainer::RegisterTool(CToolBase * tool)
{
	if(!tool)
		return;

	int idxTool = int(_tools.size());
	int idxGrp = int(_grps.size());

	std::string strType = tool->GetTypeName();
	std::map<std::string,int>::iterator it;
	it = _grps_idx.find(strType);
	if(it==_grps_idx.end())
	{
		ToolGrp  grp;
		grp.strCategory = strType;
		grp.idx_tools.push_back(idxTool);	//

		_grps.push_back(grp);				// a new tool type
		_grps_idx[strType] = idxGrp;
	}
	else
	{
		ToolGrp & grp = _grps[(*it).second];
		grp.idx_tools.push_back(idxTool);	//
	}

	_tools.push_back(tool);
}

void ToolContainer::InitializeTools(DWORD typeTool)
{
	_Clean();
	
	ToolFactory * factory = GetToolFactory();
	int n = factory->GetNumberOfClass();

	for(int i = 0;i<n;i++)
	{
		CClassT * cls = factory->GetClass(i);
		if(cls->typetl&typeTool)
		{
			CToolBase * tool =(CToolBase *)cls->_new();
			tool->RegisterMode();
			RegisterTool(tool);
		}
	}
}

void ToolContainer::_Clean()
{
	for(int i = 0;i<_tools.size();i++)
	{
		CToolBase * tool = _tools[i];
		CClass * cls = tool->GetClass();
		cls->_del(tool);
	}

	_tools.clear();
	_grps.clear();
	_grps_idx.clear();
}
void ToolContainer::InitializeTools(const char * tools)
{
	_Clean();

	std::vector<std::string> cls;
	std::string clstotal = tools;
	SplitStringBy("|",clstotal,&cls);

	ToolFactory * factory = GetToolFactory();
	for(int i = 0;i<cls.size();i++)
	{
		std::string namecls = cls[i];
		CClassT * tcls = factory->GetClass(namecls.c_str());
		if(tcls)
		{
			CToolBase * tool = (CToolBase *)tcls->_new();
			tool->RegisterMode();
			RegisterTool(tool);
		}
	}
}

void ToolContainer::UpdateUI(CGeActor * actor)
{
	for(int i = 0;i<_tools.size();++i){
		CToolBase * tool = _tools[i];
		tool->UpdateUI(actor);
	}
}

void ToolContainer::Release()
{
	_Clean();
}


 