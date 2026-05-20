#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

//各种被动效果
struct LevelPassiveEffects
{
	std::vector<unsigned __int64>hits;//受击
	std::vector<unsigned __int64>stuns;//stun
	std::vector<unsigned __int64>deaths;
	std::vector<unsigned __int64>corpses;

	BEGIN_GOBJ_PURE(LevelPassiveEffects,1);

		GELEM_VARVECTOR_INIT(unsigned __int64,hits,0);
			GELEM_EDITVAR("受击打效果",GVT_Bx8,GSem_ProtoPath,"各种受击的效果");

		GELEM_VARVECTOR_INIT(unsigned __int64,stuns,0);
			GELEM_EDITVAR("Stun效果",GVT_Bx8,GSem_ProtoPath,"各种Stun的效果");

		GELEM_VARVECTOR_INIT(unsigned __int64,deaths,0);
			GELEM_EDITVAR("死亡效果",GVT_Bx8,GSem_ProtoPath,"各种死亡的效果");

		GELEM_VARVECTOR_INIT(unsigned __int64,corpses,0);
			GELEM_EDITVAR("尸体效果",GVT_Bx8,GSem_ProtoPath,"各种尸体的效果");


	END_GOBJ();

};



//各种效果
struct LevelRecordEffectSet:public CRecord
{
	DEFINE_CLASS(LevelRecordEffectSet);

	std::string Name;

	LevelPassiveEffects melee;
	LevelPassiveEffects range;

	LevelPassiveEffects fire;
	LevelPassiveEffects electric;
	LevelPassiveEffects cold;
	LevelPassiveEffects poison;


	BEGIN_GOBJ_PURE(LevelRecordEffectSet,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"EffectSet的名称");

	END_GOBJ();


};
