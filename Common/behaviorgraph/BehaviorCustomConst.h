#pragma once

#include "../strlib/strlibdefines.h"
#include "../class/class.h"
#include "../gds/GObj.h"
#include "../gds/GObjEx.h"
#include "../records/recordsdefine.h"
#include "behaviordefines.h"


struct FillDescAssist;

struct BccClasses
{
	struct Entry
	{
		CClass *clss;
		std::string showname;
	};
	void Add(CClass *clss,const char *showname);
	CClass *Find(const char *nmClss);
	std::unordered_map<std::string,Entry> clsses;
};

class BccClassRegister
{
public:
	BccClassRegister(CClass *clss,const char *showname);

};

#define REGISTER_BCC_CLASS(clss,showname)															\
	BccClassRegister __bccregister##clss(Class_Ptr2(clss),showname);


struct BccArea
{
	DEFINE_CLASS(BccArea);

	BEGIN_GOBJ_PURE(BccArea,1);
		GELEM_VARVECTOR(i_math::spheref,sphereset)
			GELEM_EDITVAR("区域定义",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"区域数据");
		GELEM_VAR_INIT(BOOL,bWS,TRUE);
			GELEM_EDITVAR("定义空间",GVT_S,GSem(GSem_Interger,"局部空间,世界空间"),"定义在世界控件还是局部空间")
	END_GOBJ();

	BOOL IsEmpty()	{		return sphereset.empty();	}

	BOOL CheckIn(i_math::vector2df &pos)
	{
		for (int i=0;i<sphereset.size();i++)
		{
			if (sphereset[i].isPointIn(pos))
				return TRUE;
		}
		return FALSE;
	}

	BOOL bWS;
	std::vector<i_math::spheref> sphereset;
};

struct BccRoute
{
	BEGIN_GOBJ_PURE(BccRoute,1);
		GELEM_VARVECTOR(i_math::spheref,sphereset)
			GELEM_EDITVAR("路点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet,Route"),"路点数据");
	END_GOBJ();

	BOOL CheckIn(i_math::vector2df &pos)
	{
		for (int i=0;i<sphereset.size();i++)
		{
			if (sphereset[i].isPointIn(pos))
				return TRUE;
		}
		return FALSE;
	}

	void UpdateDistsToGo();

	std::vector<i_math::spheref> sphereset;
	std::vector<float> distsToGo;
};

struct BP_MatSet
{
	BEGIN_GOBJ_PURE(BP_MatSet,1);
		GELEM_VARVECTOR(i_math::matrix43f,data)
			GELEM_EDITVAR("数据",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"数据");
		GELEM_VAR_INIT(BOOL,bWS,TRUE);
			GELEM_EDITVAR("定义空间",GVT_S,GSem(GSem_Interger,"局部空间,世界空间"),"定义在世界控件还是局部空间")
	END_GOBJ();

	BOOL bWS;
	std::vector<i_math::matrix43f> data;

};

struct BP_Area
{

	BEGIN_GOBJ_PURE(BP_Area,1);
		GELEM_VARVECTOR(i_math::spheref,sphereset)
			GELEM_EDITVAR("区域定义",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"区域数据");
		GELEM_VAR_INIT(BOOL,bWS,TRUE);
			GELEM_EDITVAR("定义空间",GVT_S,GSem(GSem_Interger,"局部空间,世界空间"),"定义在世界控件还是局部空间")
	END_GOBJ();

	BOOL CheckIn(i_math::vector2df &pos)
	{
		for (int i=0;i<sphereset.size();i++)
		{
			if (sphereset[i].isPointIn(pos))
				return TRUE;
		}
		return FALSE;
	}

	BOOL bWS;
	std::vector<i_math::spheref> sphereset;
};
