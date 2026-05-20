#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "LevelDefines.h"

#include "LevelChancer.h"

#include "records/records.h"

#include "LevelRecordMagicBoard.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "MagicBoardAI.h"

#include "MBUtil.h"


#define CLASSUID_MagicBoard 25


//一组tile位置的集合
struct MagicTileRegion
{
	std::vector<i_math::pos2di>tiles;
};



struct LosMagicBoard:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosMagicBoard,CLASSUID_MagicBoard);

	BEGIN_GOBJ_PURE(LosMagicBoard,1);

		GELEM_ALLOWDISABLE();

		GELEM_AGENTRECORD();

		GELEM_VAR_INIT(RecordID,idBoard,RecordID_Invalid);
			GELEM_EDITVAR("魔法棋盘参数",GVT_U,GSem(GSem_RecordID,"magicboards"),"魔法棋盘参数");

	
	END_GOBJ();


	RecordID idBoard;


	virtual BOOL NeedSyncGUID()	{		return TRUE;	}
};

struct LopMagicBoard:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopMagicBoard,CLASSUID_MagicBoard);


	BEGIN_GOBJ_PURE(LopMagicBoard,1);
		
		GELEM_ALLOWDISABLE();

	END_GOBJ();

};

enum MagicTileState
{
	MagicTileState_None=0,
	MagicTileState_Sealed,
	MagicTileState_UnSealed,
	MagicTileState_Commit,

	//仅在客户端有效
	MagicTileState_OtherUnSealed,
	MagicTileState_OtherCommit,

};


struct MagicTileInfo
{
	MagicTileInfo()
	{
		state=MagicTileState_None;
		tpRgn=MagicTileRegion_None;
		idOwnerPlayer=LevelPlayerID_Invalid;
		reachesPlayer=0;

		candi=NULL;
		x=-1;
		y=-1;
	}

	WORD idx;
	short x;
	short y;

	MagicTileState state;

	MagicTileRegionType tpRgn;//属于哪个区域

	LevelPlayerMask reachesPlayer;//哪些玩家可以访问这个tile
	LevelPlayerID idOwnerPlayer;

	TileCandidate *candi;

	i_math::matrix43f mat;
};

struct MagicBoardInvoke;
class CLoMagicBoard:public CLoAgent
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoMagicBoard,CLASSUID_MagicBoard);

	CLoMagicBoard()
	{
		_w=0;
		_h=0;
		_lenTile=0.0f;
	}

	virtual const char *GetShowName()	{		return "魔法棋盘";	}

	virtual BOOL OnActivate();
	virtual void OnDestroy();

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void Update();

	void Invoke(LevelPlayerID idPlayer,MagicBoardInvoke &invoke);

	MagicBoardAIContext &GetMagicBoardAIContext()	{		return _ai.GetCtx();	}

	BOOL CheckMBResCost(LevelPlayerID idPlayer,MBResCost &cost)	{		return _CheckMBResCost(idPlayer,cost);	}

	MagicTileInfo *GetTileInfo(DWORD idxTile)
	{
		if (idxTile<_tiles.size())
			return &_tiles[idxTile];
		return NULL;
	}


protected:
	void _BuildRgns();

	void _Distrib();

	BOOL _CommitTile(MagicTileInfo *tile);
	BOOL _UnsealTile(MagicTileInfo *tile,LevelPlayerID idPlayer);


	//得到一些区域中的所有尚未被分配的tiles的位置
	std::vector<i_math::pos2di>&_GetFreeTiles(BOOL bEnemy,std::vector<MagicTileRegionType> &rgns);

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	void _SetTileRgnType(int x,int y,MagicTileRegionType tp);

	void _UpdateTileReach(int x,int y,LevelPlayerID idPlayer);
	void _UpdateManaRecover();
	BOOL _CheckMBResCost(LevelPlayerID idPlayer,MBResCost &cost);
	BOOL _CheckReach(MagicTileInfo *ti,LevelPlayerID idPlayer);
	void _ApplyMBResCost(LevelPlayerID idPlayer,MBResCost &cost);

	void _AddDirties(int x,int y);

	void _CreateMainTower(float x,float y,LevelPlayerID idPlayer);

	void _UpdateAI();


	int _w,_h;
	float _lenTile;
	MagicTileRegion _rgns[MagicTileRegion_Max];
	MagicTileRegion _rgnsEnemy[MagicTileRegion_Max];

	std::vector<MagicTileInfo> _tiles;

	std::vector<WORD>_dirties;//哪些tile需要更新

	std::vector<i_math::pos2di> _temp;

	CMagicBoardAI _ai;

	friend struct MagicTileDistrib_Single;
	friend struct MagicTileDistrib_Multiple;

};
