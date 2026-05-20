
#include "stdh.h"
#include "GameTrisMap.h"

#include "rasterize/rasterize.h"

float CGameTrisMap::GetHeight(float x,float y)
{
	i_math::vector3df posSrc,posTarget,posHit;
	posSrc.set(x,1000,y);
	posTarget.set(x,-1000,y);

	if (RayCheck(posSrc,posTarget,posHit))
		return posHit.y;
	return -1000.0f;
}


BOOL CGameTrisMap::RayCheck(i_math::vector3df &posSrc,i_math::vector3df &posTarget,i_math::vector3df &posHit)
{
	TileByLine(posSrc.x,posSrc.z,posTarget.x,posTarget.z,_hdr.lenTile,_temp);

	i_math::line3df line;
	line.start=posSrc;
	line.end=posTarget;

	float dist2Min=1000000000000.0f;

	BOOL bHit=FALSE;

	i_math::vector3df posIntersect;

	for (int i=0;i<_temp.size();i++)
	{
		if (!_hdr.rcTiles.isPointInside(_temp[i]))
			continue;

		i_math::pos2di ptTile=_temp[i];

		GameTrisTile *tile=&_tiles[(ptTile.y-_hdr.rcTiles.Top())*_hdr.rcTiles.getWidth()+ptTile.x-_hdr.rcTiles.Left()];

		for (int k=0;k<tile->nTris;k++)
		{
			GameTri *tri=&_tris[_indices[tile->tris+k]];
			if (tri->tri.getSafeIntersectionWithLimitedLine(line,posIntersect))
			{
				float dist2=posSrc.getDistanceFromSQ(posIntersect);
				if (dist2<dist2Min)
				{
					bHit=TRUE;
					posHit=posIntersect;
					dist2Min=dist2;
				}
			}
		}

		if (bHit)
		{
			i_math::rectf rcCur;
			rcCur.Left()=_hdr.lenTile*(float)ptTile.x;
			rcCur.Top()=_hdr.lenTile*(float)ptTile.y;
			rcCur.Right()=rcCur.Left()+_hdr.lenTile;
			rcCur.Bottom()=rcCur.Top()+_hdr.lenTile;

			if (rcCur.isPointInside(posHit.x,posHit.z))
				return TRUE;
		}
	}

	return bHit;
	
}
