#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/vector3d.h"

#include "ResData.h"

#include "AnimTreePads.h"

#include "../gds/GObj.h"

struct AT_Model
{
	std::string mesh;
	std::string mtrl;
	BEGIN_GOBJ_PURE(AT_Model,1);
		GELEM_STRING_INIT(mesh,"");
			GELEM_EDITVAR("Mesh",GVT_String,GSem_MeshPath,"Mesh路径");
		GELEM_STRING_INIT(mtrl,"");
			GELEM_EDITVAR("材质",GVT_String,GSem_MtrlPath,"材质路径");
	END_GOBJ();    
};

struct AT_Preview
{
	std::vector<AT_Model>models;
	std::string anim;
	std::vector<std::string> animsAddon;

	BEGIN_GOBJ_PURE(AT_Preview,1);
		GELEM_OBJVECTOR(AT_Model,models);
			GELEM_EDITOBJ("模型","模型数据");
		GELEM_STRING_INIT(anim,"");
			GELEM_EDITVAR("骨骼动画",GVT_String,GSem_BoneAnim2Path,"骨骼动画路径");
		GELEM_STRINGVECTOR_INIT(animsAddon,"");
			GELEM_EDITVAR("骨骼动画(额外)",GVT_String,GSem_BoneAnim2Path,"额外的骨骼动画路径");

	END_GOBJ();    

};

struct AT_Group
{
	StringID name;
	DWORD flags;

	BEGIN_GOBJ_PURE(AT_Group,1);
		GELEM_VAR_INIT(StringID,name,StringID_Invalid);
			GELEM_EDITVAR("名称",GVT_U,GSem_StringID,"同步Group的名称");
		GELEM_VAR_INIT(DWORD,flags,4);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"被激活时重播动画:4"),"标志");
	END_GOBJ();    

};

struct AT_Params
{
	std::vector<AT_Group> grps;

	BEGIN_GOBJ_PURE(AT_Params,1);

		GELEM_OBJVECTOR(AT_Group,grps);
			GELEM_EDITOBJ("Sync Groups","同步组");

	END_GOBJ();    
};



struct AnimTreeData:public ResData
{
	DECLARE_CLASS(AnimTreeData);


	AnimTreeData();
	virtual ~AnimTreeData();
	virtual void Zero();
	virtual void Clean();

	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "atr";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);

	virtual void CollectRefs(std::vector<std::string>&buf);


	//properties
	CAnimTreePads pads;
	AT_Params params;
	AT_Preview preview;
};