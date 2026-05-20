
#pragma  once

#include "WorldSystem/IObjMap.h"

#include "gamedata/GameRgnGrid.h"


class IGameRgnMapEditor :public IObjMapEditor
{
public:
	virtual float GetGridWidth()=0;//一个Grid的宽度,以米为单位
// 	virtual BOOL ModifyFlag(i_math::recti &rcGrid,GameRgnFlags flag,BOOL bSet) = 0;
// 	virtual BOOL SetRegionID(i_math::recti &rcGrid,GameRgnID regID) = 0;
	virtual const GameRgnGrid * GetGrid(int xGrid,int yGrid) = 0;
	virtual GameRgnGrid * QueryGrid(int xGrid,int yGrid)=0;
	virtual BOOL Export(const char * filename) = 0;
};

