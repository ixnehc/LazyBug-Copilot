#pragma once

#include "LevelDefines.h"

#include "BuffCalc.h"

#include "class/class.h"

#include "fastdelegate/FastDelegate.h"

#include <set>

#define LEVELOBJMAP_TILE_LEN 8.0f//单位为米
#define LEVELOBJMAP_BLOCK_LEN 64.0f//单位为米

#define LEVELOBJMAP_TILE_PER_BLOCK ((int)(LEVELOBJMAP_BLOCK_LEN/LEVELOBJMAP_TILE_LEN))

typedef DWORD LevelObjTileKey;

class CLevelObj;
typedef fastdelegate::FastDelegate2<CLevelObj *,float,BOOL> LevelObjMapEnumCallBack;


class CLevelObjBlock;
class CLevelObj;
struct LevelObjTile
{
	std::deque<CLevelObj*> objs;//注意这些LevelObj都不带引用计数
	CLevelObjBlock *blk;
	i_math::pos2d_sh pos;
	DWORD idxBlk;
};

class CLevelObjBlock
{
public:
	DEFINE_CLASS(CLevelObjBlock);
	CLevelObjBlock()
	{
		_nLevelObjs=0;
	}
	void Init(int xBlk,int yBlk,DWORD idxBlk)
	{
		int xStart=xBlk*LEVELOBJMAP_TILE_PER_BLOCK;
		int yStart=yBlk*LEVELOBJMAP_TILE_PER_BLOCK;
		for (int j=0;j<LEVELOBJMAP_TILE_PER_BLOCK;j++)
		for (int i=0;i<LEVELOBJMAP_TILE_PER_BLOCK;i++)
		{
			LevelObjTile *tile=&_tiles[j*LEVELOBJMAP_TILE_PER_BLOCK+i];
			tile->blk=this;
			tile->idxBlk=idxBlk;
			tile->pos.x=(short)(i+xStart);
			tile->pos.y=(short)(j+yStart);
		}
	}

public:
	LevelObjTile _tiles[LEVELOBJMAP_TILE_PER_BLOCK*LEVELOBJMAP_TILE_PER_BLOCK];
	int _nLevelObjs;//block里一共有多少个units

};

struct LevelDetectWeightsBase;
struct CalcDetectRateContext
{
	float radiusMin;
	float radiusMax;
	i_math::vector2df center;
	BOOL bTouching;
	LevelDetectWeightsBase *weights;
	CLevelObj *src;
	CLevelObj *recent;
	CLevelObj *fail;
};

struct LevelDetectRate
{
	float rate;
	float dist;
};



#define LEVELOBJMAP_BLKIDX_AGENT (16)
#define LEVELOBJMAP_BLKIDX_ITEM (17)

class CLevel;
struct LevelDetectWeightsBase;
class CLevelObjMap
{
public:
	struct BlkIdxInfo
	{
		DWORD methods:6;
		DWORD flagsPlayerOrUnit:2;
		DWORD idx:8;
		DWORD maskPlayer:16;
	};
	CLevelObjMap()
	{
		_w=_h=0;
		_lvl=NULL;

		_ClearEnumFilter();
	}

	void Create(CLevel *lvl,i_math::recti &rcMap);//rcMap单位为米
	void Destroy();

	void AddLo(CLevelObj*lo);
	void RemoveLo(CLevelObj*lo);
	void UpdateLo(CLevelObj*lo);

	void SetEnumRange(CLevelObj*loCenter,float radiusMin,float radiusMax);
	void SetEnumRange(i_math::vector2df &center,float radiusMin,float radiusMax);
	void SetEnumRange(i_math::rectf &rc0)
	{
		_rcRange=rc0;
		_rcRange.Left()-=_xStart;
		_rcRange.Right()-=_xStart;
		_rcRange.Top()-=_yStart;
		_rcRange.Bottom()-=_yStart;
		_radiusMax=-1.0f;//不使用半径
		_radiusMin=-1.0f;//不使用半径
	}


	void SetEnumFilter_Agents_(LevelPlayerMask mask)	{		_AddBlkIdx2(LEVELOBJMAP_BLKIDX_AGENT,1,mask,FALSE);	}
	void SetEnumFilter_AllAgents()	{		_AddBlkIdx2(LEVELOBJMAP_BLKIDX_AGENT,1,0xffff,FALSE);	}
	void SetEnumFilter_Items()	{		_AddBlkIdx2(LEVELOBJMAP_BLKIDX_ITEM,1,0xffff,FALSE);	}
	void SetEnumFilter_UnitsOfPlayer(LevelPlayerID idPlayer,LevelMoveMethodMask methods)	
	{		_AddBlkIdx(idPlayer,methods,idPlayer,FALSE);	}
	void SetEnumFilter_Player(LevelPlayerID idPlayer,LevelMoveMethodMask methods)	
	{		_AddBlkIdx(idPlayer,methods,idPlayer,TRUE);	}
	void SetEnumFilter_Ignore(CLevelObj *toIgnore)	
	{		
		if (_nIgnores>=ARRAY_SIZE(_ignores))
			return;
		_ignores[_nIgnores]=toIgnore;_nIgnores++;
	}
	void SetEnumFilter_AgentIDs(RecordID *idAgents,DWORD count)	
	{		
		if (_nAgents+count>ARRAY_SIZE(_idAgents))
			count=ARRAY_SIZE(_idAgents)-_nAgents;
		memcpy(&_idAgents[_nAgents],idAgents,count*sizeof(LevelObjRequire));
		_nAgents+=count;
	}

	void SetEnumFilter_Require(LevelObjRequire *requires,DWORD count)
	{
		if (_nRequires+count>ARRAY_SIZE(_requires))
			count=ARRAY_SIZE(_requires)-_nRequires;
		memcpy(&_requires[_nRequires],requires,count*sizeof(LevelObjRequire));
		_nRequires+=count;
	}

	void SetEnumFilter_Touching(BOOL bTouching)	{		_bTouching=bTouching;	}
	void SetEnumFilter_DetectWeights(LevelDetectWeightsBase *weights)	{		_weightsDetect=weights;	}
	void SetEnumFilter_Src(CLevelObj *lo)	{		_src=lo;	}
	void SetEnumFilter_RecentTarget(CLevelObj *lo)	{		_recent=lo;	}
	void SetEnumFilter_FailTarget(CLevelObj *lo)	{		_fail=lo;	}

	CLevelObj**Enum(LevelObjMapEnumCallBack dlgt,DWORD &c)//每次Enum后,所有设置的Filter会被清除
	{
		_Enum(dlgt);
		c=_enum.size();
		_ClearEnumFilter();
		return _enum.data();
	}

	CLevelObj**EnumInAll(LevelObjMapEnumCallBack dlgt,DWORD &c)//每次Enum后,所有设置的Filter会被清除
	{
		_EnumInAll(dlgt);
		c=_enum.size();
		_ClearEnumFilter();
		return _enum.data();
	}


	CLevelObj *EnumFirst(LevelObjMapEnumCallBack dlgt);//枚举第一个,每次Enum后,所有设置的Filter会被清除
	CLevelObj *EnumBest(LevelObjMapEnumCallBack dlgt);
	float EnumCalcStrength(LevelObjMapEnumCallBack dlgt);//计算枚举到的单位的力量总和

	void GarbageCollect();

	static BOOL CheckInRange(CLevelObj *lo,float dist2,float radiusMin,float radiusMax,BOOL bTouching);
	static BOOL CheckInRange(CLevelObj *lo,i_math::rectf &rcRange,BOOL bTouching);

public:

	void _ClearEnumFilter()
	{
		_nBlkIdx=0;
		_radiusMax=-1;
		_radiusMin=-1;
		_rcRange.set(0,0,0,0);
		_nIgnores=0;
		_nRequires=0;
		_nAgents=0;
		_bTouching=FALSE;
		_weightsDetect=NULL;
		_src=NULL;
		_recent=NULL;
		_fail=NULL;
	}

	void _Enum(LevelObjMapEnumCallBack dlgt);
	void _EnumInAll(LevelObjMapEnumCallBack dlgt);

	BOOL _IsIgnore(CLevelObj *lo)
	{
		for (int i=0;i<_nIgnores;i++)
		{
			if (_ignores[i]==lo)
				return TRUE;
		}
		return FALSE;
	}

	BOOL _CheckRequire(CLevelObj *lo)
	{
		extern BOOL LevelUtil_CheckLoRequire(CLevelObj *lo,LevelObjRequire *requires,DWORD c);
		return LevelUtil_CheckLoRequire(lo,_requires,_nRequires);
	}

	BOOL _CheckAgent(CLevelObj *lo)
	{
		if (_nAgents<=0)
			return TRUE;
		extern RecordID LevelUtil_GetAgentRecID(CLevelObj *lo);
		RecordID idRec=LevelUtil_GetAgentRecID(lo);
		for (int i=0;i<_nAgents;i++)
		{
			if (_idAgents[i]==idRec)
				return TRUE;
		}
		return FALSE;
	}

	CLevel *_lvl;

	float _xStart,_yStart;
	int _wTile,_hTile;//以tile为单位
	int _w,_h;//以Block为单位


	std::vector<CLevelObjBlock *> _blocks[16+2];
	std::set<CLevelObj *> _alls[16+2];

	void _AddBlkIdx(DWORD idxBlk,LevelMoveMethodMask methods,LevelPlayerID idPlayer,DWORD bPlayer)
	{
		_AddBlkIdx2(idxBlk,methods,(1<<idPlayer),bPlayer);
	}
	void _AddBlkIdx2(DWORD idxBlk,LevelMoveMethodMask methods,DWORD maskPlayer,DWORD bPlayer)
	{
		BlkIdxInfo info;
		info.flagsPlayerOrUnit=bPlayer?1:2;
		info.idx=idxBlk;
		info.maskPlayer=(maskPlayer);
		info.methods=methods;
		for (int i=0;i<_nBlkIdx;i++)
		{
			if (memcmp(&_blkidx[i],&info,sizeof(info)==0))
				return;

			//尝试合并
			if (info.idx<LEVELOBJMAP_BLKIDX_AGENT)
			{
				if ((info.idx==_blkidx[i].idx)&&(info.methods==_blkidx[i].methods))
				{
					_blkidx[i].flagsPlayerOrUnit|=info.flagsPlayerOrUnit;
					return;
				}
			}
		}
		_blkidx[_nBlkIdx]=info;
		_nBlkIdx++;
	}
	BlkIdxInfo _blkidx[64];//Big Enough
	DWORD _nBlkIdx;
	i_math::rectf _rcRange;
	float _radiusMax;
	float _radiusMin;
	DWORD _nIgnores;
	CLevelObj *_ignores[32];
	CLevelObj *_src;
	CLevelObj *_recent;
	CLevelObj *_fail;
	DWORD _nRequires;
	LevelObjRequire _requires[8];
	DWORD _nAgents;
	RecordID _idAgents[8];
	BOOL _bTouching;//表示只要某个单位的一部分在范围内,就可以枚举到(如果为FALSE,则要求这个单位的中心点在范围内)
	std::vector<CLevelObj*>_enum;
	LevelDetectWeightsBase *_weightsDetect;
};
