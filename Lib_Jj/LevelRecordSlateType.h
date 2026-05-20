#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

#include "LevelSlateDefinesA.h"
#include "LevelSlateDefinesB.h"



struct LevelRecordSlateType:public CRecord
{
	DEFINE_CLASS(LevelRecordSlateType);

	std::string Name;
	LevelSlateFamily family;
	LevelSlateType TypeA;
	LevelSlateType TypeB;
	LevelSlateA_Cover Cover;

	unsigned __int64 effectBase;
	StringID idRevealBG;
	StringID idProcessBG;

	int nMinGem;
	int nMaxGem;


    BEGIN_GOBJ_PURE(LevelRecordSlateType,1);

		GELEM_STRING_INIT(Name,"");GELEM_UID(8);
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"SlateType的名称");

		GELEM_VAR_INIT(LevelSlateFamily,family,LevelSlateFamily_A); GELEM_UID(9);
			GELEM_EDITVAR("家族",GVT_S,GSem(GSem_Interger,
				"A"		"|类型(B),"
				"B"		"|类型(A)&Cover类型&最少Gem数&最多Gem数"
				),"");

		GELEM_VAR_INIT(LevelSlateType,TypeA,LevelSlateTypeA_Blank); GELEM_UID(1);
			GELEM_EDITVAR("类型(A)",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeA),"类型");

		GELEM_VAR_INIT(LevelSlateType,TypeB,LevelSlateTypeB_Cross); GELEM_UID(1);
			GELEM_EDITVAR("类型(B)",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeB),"类型");

		GELEM_VAR_INIT(LevelSlateA_Cover,Cover,LevelSlateA_Cover_Normal);GELEM_UID(2);
			GELEM_EDITVAR("Cover类型",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateA_Cover),"Cover类型");

		GELEM_VAR_INIT(unsigned __int64,effectBase,0);GELEM_UID(3);
			GELEM_EDITVAR("基本效果",GVT_Bx8,GSem_ProtoPath,"基本效果");

		GELEM_VAR_INIT(StringID,idRevealBG,StringID_Invalid);	GELEM_UID(4);
			GELEM_EDITVAR( "Reveal行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );

		GELEM_VAR_INIT(StringID,idProcessBG,StringID_Invalid);	GELEM_UID(5);
			GELEM_EDITVAR( "Process行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );

		GELEM_VAR_INIT(DWORD,nMinGem,0);GELEM_UID(6);
			GELEM_EDITVAR("最少Gem数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"最少Gem数");

		GELEM_VAR_INIT(DWORD,nMaxGem,0);GELEM_UID(7);
			GELEM_EDITVAR("最多Gem数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"最多Gem数");

    END_GOBJ();    
};
