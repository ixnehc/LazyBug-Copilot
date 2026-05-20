
#pragma once

#include "class/class.h"


typedef BYTE GameRgnID;

struct GameRgnGrid
{
	GameRgnGrid()
	{
		id=0;
	}

	DWORD id;
};

struct GameRgnGridRT
{
	GameRgnGridRT() 
	{ 
		id = 0;
	}
	void From(GameRgnGrid &grid)
	{
		id=(GameRgnID)grid.id;
	}
	GameRgnID id;
};


class CGameRgnGrids
{
public:
	DEFINE_CLASS(CGameRgnGrids);
	CGameRgnGrids(void) 
	{ 
		_hdr.Zero();
	}
	~CGameRgnGrids(void)
	{
		Clear();
	}

	BOOL Init(i_math::recti &rc,DWORD lenGrid,BOOL bRT);
	void Clear();

	BOOL IsEmpty()
	{
		return !_hdr.rc.isValid();
	}
	BOOL IsRT()	{		return _hdr.bRT;	}

	BOOL Load(const char * file);
	BOOL Save(const char * file);
	BOOL SaveRT(const char * file);

	i_math::recti GetRect()	{		return _hdr.rc;	}//以Grid为单位
	DWORD GetWidth()	{		return _hdr.rc.getWidth();	}
	DWORD GetHeight()	{		return _hdr.rc.getHeight();	}
	DWORD GetGridLen()	{		return _hdr.lenGrid;	}

	GameRgnGridRT *GetGridRT(DWORD xGrid,DWORD yGrid)	{		return &_gridsRT[(yGrid-_hdr.rc.Top())*_hdr.rc.getWidth()+xGrid-_hdr.rc.Left()];	}
	GameRgnGrid *GetGrid(DWORD xGrid,DWORD yGrid){		return &_grids[(yGrid-_hdr.rc.Top())*_hdr.rc.getWidth()+xGrid-_hdr.rc.Left()];	}

	GameRgnGridRT *GetGridRT(float x,float z)
	{
		int xx=(int)((x/(float)_hdr.lenGrid-(float)_hdr.rc.Left()));
		int yy=(int)((z/(float)_hdr.lenGrid-(float)_hdr.rc.Top()));

		if (xx<0)
			return NULL;
		if (yy<0)
			return NULL;

		int w,h;
		w=_hdr.rc.getWidth();
		h=_hdr.rc.getHeight();

		if (xx>=w)
			return NULL;
		if (yy>=h)
			return NULL;
		return &_gridsRT[yy*w+xx];
	}


	BOOL IsIn(DWORD xGrid,DWORD yGrid)
	{
		return (_hdr.rc.isPointInside(xGrid,yGrid));
	}

protected:	
	struct Header
	{
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL bRT;
		DWORD lenGrid;//一个grid的宽(高)度,单位为米
		i_math::recti rc;
	};

private:
	Header _hdr;
	std::vector<GameRgnGridRT>  _gridsRT;
	std::vector<GameRgnGrid>  _grids;
};



