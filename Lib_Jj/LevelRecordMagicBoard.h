#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

#include "MagicBoardDefines.h"



class CClass;
struct GObjBase;

enum MagicTileRegionType
{
	MagicTileRegion_None,
	MagicTileRegion_Grade1,
	MagicTileRegion_Grade2,
	MagicTileRegion_Grade3,
	MagicTileRegion_Grade4,

	MagicTileRegion_Reserved1,
	MagicTileRegion_Reserved2,
	MagicTileRegion_Reserved3,
	MagicTileRegion_Reserved4,
	MagicTileRegion_Reserved5,
	MagicTileRegion_Reserved6,
	MagicTileRegion_Reserved7,
	MagicTileRegion_Reserved8,
	MagicTileRegion_Reserved9,
	MagicTileRegion_Reserved10,
	MagicTileRegion_Reserved11,
	MagicTileRegion_Reserved12,
	MagicTileRegion_Reserved13,
	MagicTileRegion_Reserved14,
	MagicTileRegion_Reserved15,
	MagicTileRegion_Reserved16,
	MagicTileRegion_Reserved17,
	MagicTileRegion_Reserved18,
	MagicTileRegion_Reserved19,
	MagicTileRegion_Reserved20,
	MagicTileRegion_Reserved21,
	MagicTileRegion_Reserved22,
	MagicTileRegion_Reserved23,
	MagicTileRegion_Reserved24,
	MagicTileRegion_Reserved25,
	MagicTileRegion_Reserved26,
	MagicTileRegion_Reserved27,
	MagicTileRegion_Reserved28,
	MagicTileRegion_Reserved29,
	MagicTileRegion_Reserved30,
	MagicTileRegion_Reserved31,
	MagicTileRegion_Reserved32,

	MagicTileRegion_Max,

	MagicTileRegion_ForceDword=0xffffffff,
};

struct TileCandidate
{
	float wt;

	RecordID idTile;

    BEGIN_GOBJ_PURE(TileCandidate,1);

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0,100,0.1"),"权重");

		GELEM_VAR_INIT(RecordID,idTile,RecordID_Invalid);
			GELEM_EDITVAR("Tile",GVT_U,GSem(GSem_RecordID,"magictiles"),"Magic Tile");
    END_GOBJ();    
};

class CLoMagicBoard;
struct MagicTileDistrib
{
	std::vector<MagicTileRegionType> rgns;

	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;

	virtual void Distrib(BOOL bEnemy,CLoMagicBoard *lo)=0;
};

struct MagicTileDistrib_Single:public MagicTileDistrib
{
	DEFINE_CLASS(MagicTileDistrib_Single);
	int cMin;
	int cMax;

	TileCandidate candi;

	virtual void Distrib(BOOL bEnemy,CLoMagicBoard *lo);



	BEGIN_GOBJ_PURE(MagicTileDistrib_Single,1);

		GELEM_VARVECTOR_INIT(MagicTileRegionType,rgns,MagicTileRegion_None);
			GELEM_EDITVAR("区域列表",GVT_S,GSem(GSem_Interger,"n/a:0,等级1:1,等级2:2,等级3:3,等级4:4"),"在哪些区域内进行分布");

		GELEM_VAR_INIT(int,cMin,1);
			GELEM_EDITVAR("最少出现个数",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"最少要出现几个这个格子");
		GELEM_VAR_INIT(int,cMax,1);
			GELEM_EDITVAR("最多出现个数",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"最多能出现几个这个格子");
		GELEM_OBJ(TileCandidate,candi)
			GELEM_EDITOBJ("格子信息","格子信息");
	END_GOBJ();    
};

struct MagicTileDistrib_Multiple:public MagicTileDistrib
{
	DEFINE_CLASS(MagicTileDistrib_Multiple);

	int cMax;
	std::vector<TileCandidate> candi;

	virtual void Distrib(BOOL bEnemy,CLoMagicBoard *lo);

	BEGIN_GOBJ_PURE(MagicTileDistrib_Multiple,1);

		GELEM_VARVECTOR_INIT(MagicTileRegionType,rgns,MagicTileRegion_None);
			GELEM_EDITVAR("区域列表",GVT_S,GSem(GSem_Interger,"n/a:0,等级1:1,等级2:2,等级3:3,等级4:4"),"在哪些区域内进行分布");

		GELEM_VAR_INIT(int,cMax,0);
			GELEM_EDITVAR("最多格数",GVT_S,GSem_Interger,"最多能出现几个格子,0表示无限多个(填满)");
		GELEM_OBJVECTOR(TileCandidate,candi)
			GELEM_EDITOBJ("多个格子信息","多个格子信息");
	END_GOBJ();    
};


struct MagicTileDistribWrap
{
	BEGIN_GOBJ_PURE(MagicTileDistribWrap,1);
		GELEM_VAR_INIT( StringID,Name,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"魔法格分布名称"), "魔法格分布名称" );

		GELEM_DYNOBJPTR(MagicTileDistrib,distrib,MagicTileDistrib_Single, "分布信息", "分布信息" );
			GELEM_DYNOBJPTR_CLASS( "0.单个格子",MagicTileDistrib_Single);								\
			GELEM_DYNOBJPTR_CLASS( "1.多个格子",MagicTileDistrib_Multiple);

	END_GOBJ();

	StringID Name;
	MagicTileDistrib*distrib;
};

struct MagicBoardLayout
{
	int wTile;
	int nGrd1;
	int nGrd2;
	int nGrd3;
	int nGrd4;
	float lenTile;

	BEGIN_GOBJ_PURE(MagicBoardLayout,1);
		
		GELEM_VAR_INIT(int,wTile,16);
			GELEM_EDITVAR("宽度(格数)",GVT_S,GSem_Interger,"棋盘的宽度");
		GELEM_VAR_INIT(int,nGrd1,4);
			GELEM_EDITVAR("等级1行数",GVT_S,GSem_Interger,"等级1行数");
		GELEM_VAR_INIT(int,nGrd2,2);
			GELEM_EDITVAR("等级2行数",GVT_S,GSem_Interger,"等级2行数");
		GELEM_VAR_INIT(int,nGrd3,1);
			GELEM_EDITVAR("等级3行数",GVT_S,GSem_Interger,"等级3行数");
		GELEM_VAR_INIT(int,nGrd4,1);
			GELEM_EDITVAR("等级4行数",GVT_S,GSem_Interger,"等级4行数");

		GELEM_VAR_INIT(float,lenTile,1.0f);
			GELEM_EDITVAR("基本格长度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"基本格的长度");
	END_GOBJ();

};


struct LevelRecordMagicBoard:public CRecord
{
	DEFINE_CLASS(LevelRecordMagicBoard);


	BEGIN_GOBJ_PURE(LevelRecordMagicBoard,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"道具的名称");

		GELEM_OBJ(MagicBoardLayout,layout)
			GELEM_EDITOBJ("魔法格区域信息","魔法格区域信息");
		GELEM_OBJVECTOR(MagicTileDistribWrap,distribs)
			GELEM_EDITOBJ("魔法格分布信息","魔法格分布信息");

		GELEM_OBJARRAY(MBResCost,costRgns);
			GELEM_EDITOBJ_EX("各区域消耗资源","各区域消耗资源",GSem(GSem_Unknown,
				"$Lable{//n/a,等级1,等级2,等级3,等级4}"))

		GELEM_VAR_INIT( StringID,nmBg,StringID_Invalid);	
			GELEM_EDITVAR( "AI行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "这个Board的AI" );

		GELEM_VAR_INIT(RecordID,idTower,RecordID_Invalid);
			GELEM_EDITVAR("主塔",GVT_U,GSem(GSem_RecordID,"agents"),"主塔");
	END_GOBJ();

	std::string Name;
	MagicBoardLayout layout;
	std::vector<MagicTileDistribWrap> distribs;

	RecordID idTower;

	StringID nmBg;


	MBResCost costRgns[MagicTileRegion_Max];


};
