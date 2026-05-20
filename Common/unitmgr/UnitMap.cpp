
#include "stdh.h"
#include "UnitMap.h"
#include "UnitMgr.h"

#include "commondefines/general_stl.h"

#include "timer/profiler.h"


void CUnitMap::Create(i_math::recti &rcMap)
{
	_xStart=(float)rcMap.Left(); 
	_yStart=(float)rcMap.Top();

	_wTile=(int)(((float)rcMap.getWidth())/(float)UNITMAP_TILE_LEN)+1;
	_hTile=(int)(((float)rcMap.getHeight())/(float)UNITMAP_TILE_LEN)+1;

	_w=(_wTile+UNITMAP_TILE_PER_BLOCK-1)/UNITMAP_TILE_PER_BLOCK;
	_h=(_hTile+UNITMAP_TILE_PER_BLOCK-1)/UNITMAP_TILE_PER_BLOCK;

	_blocks.resize(_w*_h);
	VEC_SET(_blocks,0);
}

void CUnitMap::Destroy()
{
	BOOL bEmpty=TRUE;
	for (int i=0;i<_blocks.size();i++)
	{
		CUnitBlock *blk=_blocks[i];
		if (bEmpty)
		{
			if (blk)
			{
				if (blk->_nUnits>0)
					bEmpty=FALSE;
				else
				{
					for (int j=0;j<ARRAY_SIZE(blk->_tiles);j++)
					{
						if (blk->_tiles[j].units)
						{
							bEmpty=FALSE;
							break;
						}
					}
				}
			}
		}
		Safe_Class_Delete(blk);
	}
	assert(bEmpty);
	_blocks.clear();
}


void CUnitMap::AddUnit(CUnitBase *unit)
{
	if (unit->_tile)
		return;//已经加入了

	unsigned int xTile,yTile;
	xTile=(unsigned int)((unit->_pos.x-_xStart)/UNITMAP_TILE_LEN);
	yTile=(unsigned int)((unit->_pos.y-_yStart)/UNITMAP_TILE_LEN);

	if ((xTile>=_wTile)||(yTile>=_hTile))
		return;//越界

	unsigned int xBlk,yBlk;
	xBlk=xTile/UNITMAP_TILE_PER_BLOCK;
	yBlk=yTile/UNITMAP_TILE_PER_BLOCK;

	unsigned int xLocal,yLocal;
	xLocal=xTile%UNITMAP_TILE_PER_BLOCK;
	yLocal=yTile%UNITMAP_TILE_PER_BLOCK;

	CUnitBlock *&blk=_blocks[yBlk*_w+xBlk];
	if (!blk)
		blk=Class_New2(CUnitBlock);

	UnitTile *tile=&blk->_tiles[yLocal*UNITMAP_TILE_PER_BLOCK+xLocal];
	unit->_next=tile->units;
	tile->units=unit;

	unit->_ptUnitMapTile.set((short)xTile,(short)yTile);
	unit->_tile=tile;

	blk->_nUnits++;
}

void CUnitMap::RemoveUnit(CUnitBase *unit)
{
	if (!unit->_tile)
		return;//没有加入过


	CUnitBlock *blk=unit->_tile->blk;

	CUnitBase **p=&unit->_tile->units;

	while(*p)
	{
		if ((*p)==unit)
		{
			(*p)=unit->_next;
			break;
		}
		p=&((*p)->_next);
	}

	unit->_next=NULL;
	unit->_ptUnitMapTile.set(-1,-1);
	unit->_tile=NULL;

	blk->_nUnits--;
}

void CUnitMap::UpdateUnit(CUnitBase *unit)
{
	unsigned int xTile,yTile;
	xTile=(unsigned int)((unit->_pos.x-_xStart)/UNITMAP_TILE_LEN);
	yTile=(unsigned int)((unit->_pos.y-_yStart)/UNITMAP_TILE_LEN);

	if ((xTile==unit->_ptUnitMapTile.x)&&(yTile==unit->_ptUnitMapTile.y))
		return;

	RemoveUnit(unit);
	AddUnit(unit);
}

void CUnitMap::_Enum(i_math::rectf &rc)
{
	_enum.clear();
	if (rc.Left()<0.0f)
	{
		rc.Left()=0.0f;
		if (rc.Right()<0.0f)
			rc.Right()=0.0f;
	}
	if (rc.Top()<0.0f)
	{
		rc.Top()=0.0f;
		if (rc.Bottom()<0.0f)
			rc.Bottom()=0.0f;
	}

	i_math::recti rcTile;
	rcTile.Left()=(int)(rc.Left()/UNITMAP_TILE_LEN);
	rcTile.Top()=(int)(rc.Top()/UNITMAP_TILE_LEN);
	rcTile.Right()=(int)(rc.Right()/UNITMAP_TILE_LEN)+1;
	rcTile.Bottom()=(int)(rc.Bottom()/UNITMAP_TILE_LEN)+1;
	if (rcTile.Right()>_wTile)
		rcTile.Right()=_wTile;
	if (rcTile.Bottom()>_hTile)
		rcTile.Bottom()=_hTile;

	for (DWORD i=rcTile.Left();i<rcTile.Right();i++)
	for (DWORD j=rcTile.Top();j<rcTile.Bottom();j++)
	{
		DWORD xBlk,yBlk;
		xBlk=i/UNITMAP_TILE_PER_BLOCK;
		yBlk=j/UNITMAP_TILE_PER_BLOCK;
		CUnitBlock *blk=_blocks[yBlk*_w+xBlk];
		if (blk)
		{
			DWORD xLocal,yLocal;
			xLocal=i%UNITMAP_TILE_PER_BLOCK;
			yLocal=j%UNITMAP_TILE_PER_BLOCK;

			UnitTile *tile=&blk->_tiles[yLocal*UNITMAP_TILE_PER_BLOCK+xLocal];
			CUnitBase *p=tile->units;
			while(p)
			{
				_enum.push_back(p);
				p=p->_next;
			}
		}
	}

}


void CUnitMap::Enum(i_math::vector2df &center,float radius)
{
	i_math::rectf rc;
	rc.set(center.x-_xStart,center.y-_yStart,center.x-_xStart,center.y-_yStart);
	rc.inflate(radius,radius,radius,radius);

	_Enum(rc);
}


void CUnitMap::Enum(CUnitBase *unitCenter,float radius)
{
	Enum(unitCenter->_pos,radius);
}
