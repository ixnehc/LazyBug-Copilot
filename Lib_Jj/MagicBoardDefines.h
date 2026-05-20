#pragma once


enum MBResourceType
{
	MBRes_None=0,
	MBRes_Mana,
	MBRes_Gold,
	MBRes_Crystal,

	MBRes_ActualMax,

	MBRes_Reserved0,
	MBRes_Reserved1,
	MBRes_Reserved2,
	MBRes_Reserved3,
	MBRes_Reserved4,
	MBRes_Reserved5,
	MBRes_Reserved6,
	MBRes_Reserved7,
	MBRes_Reserved8,
	MBRes_Reserved9,

	MBRes_Max,
	MBRes_ForceDword=0xffffffff,
};

#define GSemString_MBResourceType "法力:1,金子:2,水晶:3"

struct MBResCost
{
	DWORD costs[MBRes_Max];

	BEGIN_GOBJ_PURE(MBResCost,1);

		GELEM_VARARRAY_INIT(DWORD,costs,0); GELEM_VERSION(2);
			GELEM_EDITVAR("各种资源的消耗",GVT_S,GSem(GSem_Interger,"$Lable{//n/a,法力,金币,水晶}"),"各种资源的消耗");

	END_GOBJ();


};