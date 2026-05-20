#pragma once

#include <vector>



#define TOPO_LVL_INVALID 15//max value of 4bit
struct TrrnTopo
{
	//Note:lvlLeft,lvlRight,lvlTop,lvlBottom are offset value to lvlThis
	DWORD lvlLeft:5;//Relative value
	DWORD lvlRight:5;//Relative value
	DWORD lvlTop:5;//Relative value
	DWORD lvlBottom:5;//Relative value
	DWORD lvlThis:6;//Absolute value,the block's ACTUAL lvl
	DWORD lvlFull:6;//Absolute value,the block's max lvl
};

#define TrrnTopo_DWORD(tt) (*((DWORD*)&(tt)))


struct TopoVtx
{
	i_math::pos2df pt;//the rated 2d position in a block,ranged in [0..1]
	i_math::pos2di pos;//the actual poistion in a block
	WORD off;//the offset 
};

struct TopoInfo
{
	struct IndexRange
	{
		WORD start;
		WORD count;
	};
	std::vector<TopoVtx> vtx;
	std::vector<WORD>idx;
	std::vector<IndexRange>tiles;
	DWORD lvl;
	DWORD lvlFull;
};
