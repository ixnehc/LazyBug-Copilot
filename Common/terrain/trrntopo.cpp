// ***************************************************************
//  trrntopo   version:  1.0   ? date: 03/18/2007
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2007 - All Rights Reserved
// ***************************************************************
//  Purpose: help functions for generate trrn topology information
// ***************************************************************
#include "stdh.h"

#include "trrntopo.h"
#include <assert.h>

#include <map>
#include <vector>

#include "Log/LogFile.h"



void AddFace(std::vector<int>&vertice,std::vector<WORD>&indice,std::vector<i_math::pos2di>&indicetile,
			 DWORD wHi,DWORD wLow,DWORD i0,DWORD i1,DWORD i2)
{
	DWORD idx[3]={i0,i1,i2};
	DWORD step=wHi/wLow;
	DWORD wHi1=wHi+1;
	i_math::recti rc;
	i_math::pos2di pt;
	for (int i=0;i<3;i++)
	{
		pt.set(idx[i]%wHi1,idx[i]/wHi1);
		rc.merge(pt);
	}

	pt=rc.UpperLeftCorner/step;
	for (int i=0;i<3;i++)
	{
		assert(vertice[idx[i]]!=-1);
		indice.push_back((WORD)vertice[idx[i]]);
		indicetile.push_back(pt);//which tile this index belongs to
	}
}

TopoInfo *CalcTopoInfo(TrrnTopo topo)
{
	static std::map<DWORD,TopoInfo> cache;

	std::map<DWORD,TopoInfo>::iterator it;
	it=cache.find(TrrnTopo_DWORD(topo));
	while(it==cache.end())
	{//Not in cache ,build one

		if ((topo.lvlThis==0)&&
			(topo.lvlLeft==0)&&(topo.lvlTop==0)&&(topo.lvlRight==0)&&(topo.lvlBottom==0)&&
			(topo.lvlFull>0))
		{//handle this situation specially

			TopoInfo &ti=cache[TrrnTopo_DWORD(topo)];

			ti.vtx.resize(4);
			ti.idx.resize(6);
			ti.tiles.resize(1);

			ti.tiles[0].start=0;
			ti.tiles[0].count=6;

			WORD t[6]={0,3,1,0,2,3	};
			memcpy(&ti.idx[0],t,sizeof(t));

			DWORD w1=(1<<topo.lvlFull)+1;
			ti.vtx[0].pt.set(0,0);
			ti.vtx[1].pt.set(1,0);
			ti.vtx[2].pt.set(0,1);
			ti.vtx[3].pt.set(1,1);
			ti.vtx[0].pos.set(0,0);
			ti.vtx[1].pos.set(1,0);
			ti.vtx[2].pos.set(0,1);
			ti.vtx[3].pos.set(1,1);
			ti.vtx[0].off=0;
			ti.vtx[1].off=(WORD)(w1-1);
			ti.vtx[2].off=(WORD)((w1-1)*w1);
			ti.vtx[3].off=(WORD)(w1*w1-1);

			ti.lvl=0;
			ti.lvlFull=topo.lvlFull;

			it=cache.find(TrrnTopo_DWORD(topo));
			break;
		}

		DWORD wHi,wLow;
		DWORD wLeft,wRight,wTop,wBottom;
		DWORD wHi1,wLow1;
		int step;
		if (TRUE)
		{
			wLow=1<<topo.lvlThis;

			wLeft=1<<(topo.lvlThis+topo.lvlLeft);
			wRight=1<<(topo.lvlThis+topo.lvlRight);
			wTop=1<<(topo.lvlThis+topo.lvlTop);
			wBottom=1<<(topo.lvlThis+topo.lvlBottom);


			wHi=max(wLeft,wRight);
			wHi=max(wHi,wTop);
			wHi=max(wHi,wBottom);
			wHi=max(wHi,1<<topo.lvlFull);
			if (wHi!=1<<topo.lvlFull)
				return NULL;//这个block的topo不合法,它本身的lvl应该大于等于它四周的block的lvl才行

			if ((wLow==1)&&(wHi>1))
				wLow=2;

			wHi1=wHi+1;
			wLow1=wLow+1;

			step=wHi/wLow;
		}

		TopoInfo &ti=cache[TrrnTopo_DWORD(topo)];

		std::vector<int>vertice;
		vertice.resize(wHi1*wHi1);
		for (int i=0;i<vertice.size();i++)
			vertice[i]=-1;

		int nVtx=0;
		for(int j=0;j<wHi1;j++)
		for(int i=0;i<wHi1;i++)
		{
			if ((i!=0)&&(j!=0)&&(i!=wHi1-1)&&(j!=wHi1-1))
			{//Not on edge
				if ((i%step==0)&&(j%step==0))
				{//on the low grid
					vertice[j*wHi1+i]=nVtx;
					nVtx++;
					continue;
				}
			}

			if (	(i==0) && (j%(wHi/wLeft)==0)		||
				(i==wHi1-1) && (j%(wHi/wRight)==0)		||
				(j==0) && (i%(wHi/wTop)==0)		||
				(j==wHi1-1) && (i%(wHi/wBottom)==0)		)
			{
				vertice[j*wHi1+i]=nVtx;
				nVtx++;
			}
		}

		std::vector<WORD>indice;
		std::vector<i_math::pos2di>indicetile;

#define GET_VERTEX(x,y) ((x)*step+(y)*step*wHi1)
#define ADDFACE(idx0,idx1,idx2) \
	AddFace(vertice,indice,indicetile,wHi,1<<topo.lvlThis,idx0,idx1,idx2);

		//Now for each tile that NOT edge
		for (int j=0;j<wLow;j++)
		for (int i=0;i<wLow;i++)
		{
			if ((i==0)||(j==0)||(i==wLow-1)||(j==wLow-1))
				continue;

			int ii,jj;
			ii=i*step;
			jj=j*step;
			int i00,i10,i01,i11;
			i00=GET_VERTEX(i,j);
			i10=GET_VERTEX(i+1,j);
			i01=GET_VERTEX(i,j+1);
			i11=GET_VERTEX(i+1,j+1);
			if ((i+j)%2==0)
			{
				ADDFACE(i00,i11,i10);
				ADDFACE(i00,i01,i11);
			}
			else
			{
				ADDFACE(i00,i01,i10);
				ADDFACE(i10,i01,i11);
			}
		}

		int i0,i1,i2;

		//The inner edge
		if (wLow>2)
		{
			for (int i=1;i<wLow-1;i++)
			{
				int ic=i;
				if ((i%2)!=0)
					ic++;

				//top
				i0=GET_VERTEX(i,1);
				i1=GET_VERTEX(i+1,1);
				i2=GET_VERTEX(ic,0);
				ADDFACE(i0,i1,i2);

				//Left
				i0=GET_VERTEX(1,i);
				i1=GET_VERTEX(1,i+1);
				i2=GET_VERTEX(0,ic);
				ADDFACE(i0,i2,i1);

				//Bottom
				i0=GET_VERTEX(i,wLow-1);
				i1=GET_VERTEX(i+1,wLow-1);
				i2=GET_VERTEX(ic,wLow);
				ADDFACE(i0,i2,i1);

				//Right
				i0=GET_VERTEX(wLow-1,i);
				i1=GET_VERTEX(wLow-1,i+1);
				i2=GET_VERTEX(wLow,ic);
				ADDFACE(i0,i1,i2);
			}
		}

		//Top edge
		for (int i=0;i<wTop;i++)
		{
			int ic=(wTop>=wLow)?i/(wTop/wLow):i;
			if (ic%2==0)
				ic++;

			i0=i*(wHi/wTop);
			i1=(i+1)*(wHi/wTop);
			i2=step*wHi1+ic*step;
			ADDFACE(i0,i2,i1);
		}

		//Bottom edge
		if (wLow>1)
			for (int i=0;i<wBottom;i++)
			{
				int ic=(wBottom>=wLow)?i/(wBottom/wLow):i;
				if (ic%2==0)
					ic++;

				i0=wHi*wHi1+i*(wHi/wBottom);
				i1=wHi*wHi1+(i+1)*(wHi/wBottom);
				i2=(wHi-step)*wHi1+ic*step;
				ADDFACE(i0,i1,i2);
			}

		//Left edge
		for (int i=0;i<wLeft;i++)
		{
			int ic=(wLeft>=wLow)?i/(wLeft/wLow):i;
			if (ic%2==0)
				ic++;

			i0=i*(wHi/wLeft)*wHi1;
			i1=(i+1)*(wHi/wLeft)*wHi1;
			i2=ic*step*wHi1+step;
			ADDFACE(i0,i1,i2);
		}

		//Right edge
		if (wLow>1)
			for (int i=0;i<wRight;i++)
			{
				int ic=(wRight>=wLow)?i/(wRight/wLow):i;
				if (ic%2==0)
					ic++;

				i0=i*(wHi/wRight)*wHi1+wHi;
				i1=(i+1)*(wHi/wRight)*wHi1+wHi;
				i2=ic*step*wHi1+wHi-step;
				ADDFACE(i0,i2,i1);
			}

		ti.lvl=topo.lvlThis;
		ti.lvlFull=topo.lvlFull;
		ti.vtx.resize(nVtx);
		if (TRUE)
		{
			int *p=vertice.data();
			for(int j=0;j<wHi1;j++)
			for(int i=0;i<wHi1;i++)
			{
				if (*p!=-1)
				{
					ti.vtx[*p].off=(WORD)(j*wHi1+i);
					ti.vtx[*p].pt.set(((float)i)/(float)(wHi1-1),((float)j)/(float)(wHi1-1));
					ti.vtx[*p].pos.set(i,j);
				}
				p++;
			}
		}
		ti.idx.resize(indice.size());
		DWORD c=0;
		for (int j=0;j<(1<<ti.lvl);j++)
		for (int i=0;i<(1<<ti.lvl);i++)
		{
			TopoInfo::IndexRange ir;
			ir.start=(WORD)c;
			for (int k=0;k<indice.size();k++)
			{
				if (indicetile[k]!=i_math::pos2di(i,j))
					continue;
				ti.idx[c]=indice[k];
				c++;
			}
			ir.count=(WORD)(c-ir.start);
			ti.tiles.push_back(ir);
		}

		it=cache.find(TrrnTopo_DWORD(topo));
		break;
	}

	return &(*it).second;
}

