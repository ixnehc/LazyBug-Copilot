
#include "stdh.h"

#include "commondefines/general_stl.h"
#include "UnitMgrMP.h"

//////////////////////////////////////////////////////////////////////////
//CTileMap
void CTileMap::Create(DWORD w,DWORD h,float lenTile)
{
	_tiles.resize(w*h);
	VEC_SET(_tiles,0);
	_lenTile=lenTile;
	_wTile=w;
	_hTile=h;
}

void CTileMap::Destroy()
{
	_tiles.clear();
	Zero();
}


#define STATE_TO_XY(state,x,y)					\
(x)=((short*)&state)[0];								\
(y)=((short*)&state)[1];

#define STATE_FROM_XY(state,x,y)				\
((short*)&state)[0]=(short)(x);						\
((short*)&state)[1]=(short)(y);




float CTileMap::LeastCostEstimate( void* stateStart, void* stateEnd)
{
	short xStart,yStart,xEnd,yEnd;
	STATE_TO_XY(stateStart,xStart,yStart);
	STATE_TO_XY(stateEnd,xEnd,yEnd);

	int dx,dy;
	dx=(int)(xEnd-xStart);
	dy=(int)(yEnd-yStart);

	return sqrtf((float)(dx*dx+dy*dy));
}

void CTileMap::AdjacentCost(void* state, std::vector< MPStateCost > *neighbors)
{
	int x, y;
	const int dx[8] = { 1, 0, -1, 0,	1, -1, -1, 1 };
	const int dy[8] = { 0, -1, 0, 1,	-1, -1, 1, 1 };
	const float cost[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 
		1.41f, 1.41f, 1.41f, 1.41f };

	STATE_TO_XY( state, x, y );

	for( int i=0; i<8; ++i ) 
	{
		DWORD nx=(DWORD)(x+dx[i]);
		DWORD ny=(DWORD)(y+dy[i]);

		if ( GetTile(nx,ny)==0)
		{
			MPStateCost nodeCost;
			nodeCost.cost=cost[i];
			STATE_FROM_XY(nodeCost.state,nx,ny);
			neighbors->push_back( nodeCost );
		}
	}


}

void CTileMap::PrintStateInfo(void* state)
{

}





//////////////////////////////////////////////////////////////////////////
//CUnitMgrMP


BOOL CUnitMgrMP::Create(DWORD w,DWORD h,float lenTile)
{
	_mp.Create(w,h,lenTile);

	_pather=new MicroPather(&_mp);

	return TRUE;

}

void CUnitMgrMP::Destroy()
{
	SAFE_DELETE(_pather);
	_mp.Destroy();
}

BOOL CUnitMgrMP::FindPath(i_math::pos2d_sh &posSrc,i_math::pos2d_sh& posTarget,std::vector<i_math::pos2d_sh>&path)
{
	if (!_pather)
		return FALSE;
	float cost;
	_pather->Solve(*(void**)&posSrc,*(void**)&posTarget,(std::vector<void*>*)&path,&cost);
	return TRUE;
}
