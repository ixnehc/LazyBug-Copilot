#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include <unordered_map>


#define DEFINE_GUIDATA_PATHES(v)															\
	GuiData_Pathes *v=(GuiData_Pathes*)FindData("pathes");							\



struct ResData;

class IUtilRS;

struct GuiLib_Api GuiData_Pathes:public GeData
{
	virtual const char *GetName()	{		return "pathes";	}
	GuiData_Pathes()
	{
		Zero();
		pUtilRS=NULL;
	}

	void Zero()
	{
		iSelCP=-1;
		bLocateCP=FALSE;
		bAddCP=FALSE;
		heightLocate=2.0f;

		bRunning=FALSE;

		bForceShow=FALSE;
	}

	void Clear();

	void LoadData();

	ResData *FindData(const char *path);
	BOOL IsSelReadOnly();
	
	IUtilRS *pUtilRS;

	BOOL bForceShow;//强制绘制路径,不管config里的设置(是否要显示Helper)

	std::string pathRoot;//完整路径
	std::unordered_map<std::string,ResData*>dataes;
	std::string sel;
	int iSelCP;
	BOOL bLocateCP;//是否在定位CP的状态中
	BOOL bAddCP;//只在bLocateCP为TRUE时有效,表明是否为新添加CP
	float heightLocate;//在定位CP时的高度偏移值

	BOOL bRunning;//是否处于巡航模式

	std::string modified;


};
