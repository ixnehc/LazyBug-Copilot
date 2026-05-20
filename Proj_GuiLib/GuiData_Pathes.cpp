#include "stdh.h"
#include "GuiData_Pathes.h"

#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IUtilRS.h"

#include "resdata/ResData.h"

#include "resdata/AnimData.h"
#include "stringparser/stringparser.h"


void GuiData_Pathes::Clear()
{
	std::unordered_map<std::string,ResData*>::iterator it;
	for (it=dataes.begin();it!=dataes.end();it++)
		ResData_Delete((*it).second);

	dataes.clear();
	sel="";

	Zero();
}

void GuiData_Pathes::LoadData()
{
	Clear();
	if (pathRoot=="")
		return;

	IFileSystem *pFS=pUtilRS->GetFS();

	const char *suffix;
	if (TRUE)
	{
		XFormData t;
		suffix=t.GetTypeSuffix();
	}

	std::vector<std::string>filelist;

	IFileSystem_EnumFilesR(pFS,pathRoot.c_str(),filelist);

	std::string path;

	for (int i=0;i<filelist.size();i++)
	{
// 		StringLower(filelist[i]);
		if (!CheckFileSuffix(filelist[i].c_str(),suffix))
			continue;

		path=pathRoot+"\\"+filelist[i];

		ResData *data=pUtilRS->LoadRes(path.c_str(),FALSE);
		if (!data)
			continue;

		dataes[filelist[i]]=data;
	}


}

ResData *GuiData_Pathes::FindData(const char *path)
{
	std::string s=path;
	StringLower(s);
	std::unordered_map<std::string,ResData*>::iterator it=dataes.find(s);
	if (it==dataes.end())
		return NULL;

	return (*it).second;
}

BOOL GuiData_Pathes::IsSelReadOnly()
{
	if (!pUtilRS)
		return FALSE;
	IFileSystem *pFS=pUtilRS->GetFS();

	static std::string s;
	s=pathRoot+"\\"+sel;
	return (pFS->GetFileAttrAbs(s.c_str())==File_ReadOnly);
}
