/********************************************************************
	created:	2007/5/30   14:36
	filename: 	e:\IxEngine\Common\rasterize\rasterize.cpp
	author:		cxi
	
	purpose:	line,triangle,polygon rasterization 
*********************************************************************/
#include "stdh.h"

#include "rasterize.h"

#include "assert.h"

#include "math/imath_all.h"

#include <set>



//Calculate all the tile the line interesects
void TileByLine(float xSrc,float ySrc,float xTarget,float yTarget,float tilelength,std::vector<i_math::pos2di>&queueTiles)
{
	queueTiles.clear();
	i_math::pos2di ptSrcTile,ptTargetTile;
	i_math::pos2di ptBase;
	if (TRUE)
	{
		ptSrcTile.x=(long)floor(xSrc/tilelength);
		ptSrcTile.y=(long)floor(ySrc/tilelength);

		ptTargetTile.x=(long)floor(xTarget/tilelength);
		ptTargetTile.y=(long)floor(yTarget/tilelength);
	}
	ptBase.x=i_math::min_<s32>(ptSrcTile.x,ptTargetTile.x);
	ptBase.y=i_math::min_<s32>(ptSrcTile.y,ptTargetTile.y);


	if (TRUE)
	{
		ptSrcTile-=ptBase;
		ptTargetTile-=ptBase;
		xSrc-=ptBase.x*tilelength;
		ySrc-=ptBase.y*tilelength;
		xTarget-=ptBase.x*tilelength;
		yTarget-=ptBase.y*tilelength;

		BOOL bHorizontal;

		long dx,dy;
		dx=abs(ptTargetTile.x-ptSrcTile.x);
		dy=abs(ptTargetTile.y-ptSrcTile.y);

		if ((dx==0)&&(dy==0))
		{
			queueTiles.resize(1);
			queueTiles[0]=ptSrcTile+ptBase;//Only the start
			return;
		}
		if (dx>dy)
			bHorizontal=TRUE;
		else
			bHorizontal=FALSE;

		if (bHorizontal)
		{
			long x,y;
			x=ptSrcTile.x;
			y=ptSrcTile.y;
			long p;

			p=2*dy-dx;
			long xsign,ysign;
			if (ptTargetTile.y>ptSrcTile.y)
				ysign=1;
			else
				ysign=-1;

			if (ptTargetTile.x>ptSrcTile.x)
				xsign=1;
			else
				xsign=-1;

			while(x-xsign !=ptTargetTile.x)
			{
				queueTiles.push_back(i_math::pos2di(x,y));

				f32 xTest,yTest;
				if (xsign>0)
					xTest=((f32)(x+1))*tilelength;
				else
					xTest=((f32)x)*tilelength;

				yTest=(yTarget-ySrc)*(xTest-xSrc)/(xTarget-xSrc)+ySrc;

				long tileTest=(long)(yTest/tilelength);

				if (tileTest!=y)
				{
					long d;
					d=tileTest-y;
					if (d>0)
						d=1;
					else
						d=-1;
					while((tileTest!=y)&&(y!=ptTargetTile.y))
					{
						y+=d;
						queueTiles.push_back(i_math::pos2di(x,y));
					}
				}

				x+=xsign;
			}
		}
		else
		{//vertical 
			long x,y;
			x=ptSrcTile.x;
			y=ptSrcTile.y;
			long p;

			p=2*dx-dy;
			long xsign,ysign;
			if (ptTargetTile.x>ptSrcTile.x)
				xsign=1;
			else
				xsign=-1;

			if (ptTargetTile.y>ptSrcTile.y)
				ysign=1;
			else
				ysign=-1;

			while(y-ysign !=ptTargetTile.y)
			{
				queueTiles.push_back(i_math::pos2di(x,y));

				f32 yTest,xTest;
				if (ysign>0)
					yTest=((f32)(y+1))*tilelength;
				else
					yTest=((f32)y)*tilelength;

				xTest=(xTarget-xSrc)*(yTest-ySrc)/(yTarget-ySrc)+xSrc;

				long tileTest=(long)(xTest/tilelength);//xTest所在的tile的横坐标
				if (tileTest!=x)
				{
					long d=tileTest-x;
					if (d>0)
						d=1;
					else
						d=-1;
					while((tileTest!=x)&&(x!=ptTargetTile.x))
					{
						x+=d;
						queueTiles.push_back(i_math::pos2di(x,y));

					}
				}

				y+=ysign;
			}
		}


		int szQueue=queueTiles.size();

		for (int i=0;i<szQueue;i++)
			queueTiles[i]+=ptBase;
		ptSrcTile+=ptBase;
		ptTargetTile+=ptBase;
		xSrc+=ptBase.x*tilelength;
		ySrc+=ptBase.y*tilelength;
		xTarget+=ptBase.x*tilelength;
		yTarget+=ptBase.y*tilelength;

		if (szQueue>0)
		{
			while ((queueTiles[szQueue-1].x!=ptTargetTile.x)||(queueTiles[szQueue-1].y!=ptTargetTile.y))
			{
				szQueue--;
				if (szQueue<=0)
				{
					queueTiles.resize(1);
					queueTiles[0]=ptSrcTile;//Only the start
					return;
				}
			}
			queueTiles.resize(szQueue);
		}
	}
}


//Calculate all the tiles the triangle interesects
void TileByTriangle(float *xys,float tilelength,std::vector<i_math::pos2di>&queueTiles)
{
	i_math::vector2df tri[3];
	memcpy(tri,xys,sizeof(tri));

	int xmin=0x1fffffff,xmax=-0x1fffffff;
	for (int i=0;i<3;i++)
	{
		int v=(int)(tri[i].x/tilelength);
		if (v-2<xmin)
			xmin=v-2;
		if (v+2>xmax)
			xmax=v+2;
	}

	DWORD w=xmax-xmin+1;
	std::vector<int>ymin;
	std::vector<int>ymax;
	ymin.resize(w);
	ymax.resize(w);


	for (int i=0;i<w;i++)
	{
		ymin[i]=0x1fffffff;
		ymax[i]=-0x1fffffff;
	}

	std::vector<i_math::pos2di>q;
	for (int i=0;i<3;i++)
	{
		float xSrc,ySrc,xTgt,yTgt;
		xSrc=tri[i].x;
		ySrc=tri[i].y;
		xTgt=tri[(i+1)%3].x;
		yTgt=tri[(i+1)%3].y;

		TileByLine(xSrc,ySrc,xTgt,yTgt,tilelength,q);

		for (int j=0;j<q.size();j++)
		{
			int x=q[j].x-xmin;
			int y=q[j].y;

			if (y>ymax[x])
				ymax[x]=y;
			if (y<ymin[x])
				ymin[x]=y;
		}
	}

	queueTiles.clear();
	for (int i=0;i<w;i++)
	for (int j=ymin[i];j<=ymax[i];j++)
		queueTiles.push_back(i_math::pos2di(i+xmin,j));
}



static int compare_int(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

void TileByPolygon(i_math::vector2df *xys, int n, float tilelength, std::set<i_math::pos2di>&tiles)
{
	tiles.clear();

	int i;
	int y;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;
	static std::vector<int> polyints;
	static std::vector<i_math::pos2di> buf;
	polyints.resize(n);
	buf.resize(n);

	for (int i = 0;i < n;i++)
	{
		buf[i].x= (int)floor(xys[i].x / tilelength);
		buf[i].y = (int)floor(xys[i].y / tilelength);
	}

	/* Determine Y maxima */
	miny = buf[0].y;
	maxy = buf[0].y;
	for (i = 1; i < n; i++)
	{
		miny = i_math::min_(miny, buf[i].y);
		maxy = i_math::max_(maxy, buf[i].y);
	}

	/* Draw, scanning y */
	for (y = miny; y <= maxy; y++)
	{
		ints = 0;
		for (i = 0; i < n; i++)
		{
			if (!i)
			{
				ind1 = n - 1;
				ind2 = 0;
			}
			else 
			{
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = buf[ind1].y;
			y2 = buf[ind2].y;
			if (y1 < y2) 
			{
				x1 = buf[ind1].x;
				x2 = buf[ind2].x;
			}
			else if (y1 > y2) 
			{
				y2 = buf[ind1].y;
				y1 = buf[ind2].y;
				x2 = buf[ind1].x;
				x1 = buf[ind2].x;
			}
			else 
			{
				continue;
			}
			if ((y >= y1) && (y < y2)) 
			{
				polyints[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			}
			else if ((y == maxy) && (y > y1) && (y <= y2)) 
			{
				polyints[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			}
		}
		qsort(polyints.data(), ints, sizeof(int), compare_int);

		for (i = 0; (i < ints); i += 2) 
		{
			for (int j = polyints[i];j <= polyints[i + 1];j++)
				tiles.insert(i_math::pos2di(j, y));
		}
	}
}
