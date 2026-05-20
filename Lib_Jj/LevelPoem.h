#pragma once

#include "gds/GObj.h"

#include "LevelAbility.h"

#include "BgnGA_RollAwards.h"

//Poem奖励
struct PoemAwards
{
	StringID nm;
	BOOL bAllowChoose;
	RollAwardParam award;

	BEGIN_GOBJ_PURE_UID2(PoemAwards,429,1)

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	GELEM_UID(1);
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"Poem奖励名称"), "名称" );
		GELEM_VAR_INIT(BOOL,bAllowChoose,FALSE);GELEM_UID(2);
			GELEM_EDITVAR("是否允许选择",GVT_S,GSem_Boolean,"是否允许选择");
		GELEM_OBJ(RollAwardParam,award);GELEM_UID(3);
			GELEM_EDITOBJ("奖励参数","奖励参数");
	END_GOBJ();

};

struct PoemContent
{
	StringID title;
	StringID text;

	BEGIN_GOBJ_PURE(PoemContent,1)

		GELEM_VAR_INIT( StringID,title,StringID_Invalid);	
			GELEM_EDITVAR( "标题", GVT_U, GSem(GSem_StringID,"Poem标题"), "标题" );
		GELEM_VAR_INIT( StringID,text,StringID_Invalid);	
			GELEM_EDITVAR( "文本", GVT_U, GSem(GSem_StringID,"Poem文本"), "文本" );
	END_GOBJ();

};

class CLevelPoemInitial:public CLevelAbilityInitial
{
public:

	std::vector<PoemAwards> _awards;
	PoemContent _content;

};

#define GELEM_POEM_BASE()\
		GELEM_OBJVECTOR(PoemAwards,_awards);GELEM_UID(1)\
			GELEM_EDITOBJ("交易奖励","交易奖励");\
		GELEM_OBJ(PoemContent,_content);GELEM_UID(2)\
			GELEM_EDITOBJ("内容","内容");


class CLevelPoem: public CLevelAbility
{
public:

    BOOL IsPoem() override { return TRUE; }
	virtual PoemAwards *FindPoemAwards(StringID nm) override;


};
