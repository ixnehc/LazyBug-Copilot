
#include "stdh.h"

#include "..\detour\DetourTileQuery.h"

#include "rtWorld.h"

#include <fstream>


#define BORDER_W	0.6f

inline int getRegionKey(short x,short y)
{
	return (x<<16)|(0x0000ffff&y);
}

inline void getDirOffset(int dir,int &xoff,int &yoff)
{
	switch(dir)
	{
	case 0:
		xoff = -1;
		yoff = 0;
		break;
	case 1:
		xoff = 1;
		yoff = 0;
		break;
	case 2:
		xoff = 0;
		yoff = 1;
		break;
	case 3:
		xoff = 0;
		yoff = -1;
		break;
	}
}

inline int getInvDir(int dir)
{
	const int rdir[] = {1,0,3,2};
	return rdir[dir];
}


rtWorld::rtWorld(void)
{
}

rtWorld::~rtWorld(void)
{
}

void rtWorld::clear()
{
	for(int i = 0;i<_newRegions.size();++i)
	{
		rtRegion * r = _newRegions[i];
		delete r;
	}
	_newRegions.clear();
	_mpRegions.clear();
	_dataRegions.clear();
	_rtTableMgr.Clear();
}

rtRegion * rtWorld::getRegion(short x,short y) const
{
	rtRegion *rg = NULL;
	int k = getRegionKey(x,y);
	RegionMap::const_iterator it = _mpRegions.find(k);
	if(it!=_mpRegions.end())
	{
		rg = const_cast<rtRegion *>(it->second);
		assert(rg->rx==x&&rg->ry==y);
	}
	return rg;
}

void rtWorld::calcRegionLoc(const dtVec3 &pos,int &rx,int &ry,int &x,int &y) const
{
	i_math::pos2di pt,ptRegion;
	pt.x = int(pos.x);
	pt.y = int(pos.z);

	ptRegion = pt;
	ptRegion.scale_signed(REGION_W);
	
	rx = ptRegion.x;
	ry = ptRegion.y;

	x = 255*ptRegion.x + encodeF2C_ceil(float(pt.x - REGION_W*ptRegion.x)); 
	y = 255*ptRegion.y +  encodeF2C_ceil(float(pt.y - REGION_W*ptRegion.y));
}

void rtWorld::addRegion(rtRegion * region)
{
	if(region)
	{
		int key = getRegionKey(region->rx,region->ry);
		_mpRegions[key] = region;

		int xoff,yoff,x,y;
		for(int i = 0;i<4;++i)
		{
			getDirOffset(i,xoff,yoff);
			x = xoff + region->rx;
			y = yoff + region->ry;
			rtRegion * rnei = getRegion(x,y);
			if(rnei)
				_rtTableMgr.UpdateTable(region,rnei);
		}
	}
}

void rtWorld::removeRegion(rtRegion * region)
{
	int key = getRegionKey(region->rx,region->ry);
	itRegionMap it = _mpRegions.find(key);
	if(it!=_mpRegions.end())
	{
		_mpRegions.erase(it);
		
		//内向外连接
		_rtTableMgr.RemoveTable(region,0);
		_rtTableMgr.RemoveTable(region,1);
		
		//外向内连接
		rtRegion *rleft = getRegion(region->rx-1,region->ry);
		if(rleft)
			_rtTableMgr.RemoveTable(rleft,0);
		
		rtRegion *rtop = getRegion(region->rx,region->ry-1);
		if(rtop)
			_rtTableMgr.RemoveTable(rtop,0);
	}
}

rtRegion * rtWorld::build(dtMeshTile * tileMesh,int rx,int ry,const dtVec3 * edges,int numEdges)
{
	dtVec2 edgeVec;
	
	std::vector<int> leftEdges;
	std::vector<int> rightEdges;
	std::vector<int> topEdges;
	std::vector<int> bottomEdges;
	
	i_math::rectf rcBound;
	rcBound.UpperLeftCorner.set(float(rx)*getRegionWidth(),float(ry)*getRegionWidth());
	rcBound.LowerRightCorner.set(float(rx+1)*getRegionWidth(),float(ry+1)*getRegionWidth());

	const float minr = 0.25f;
	//Filter all bound edge
	for(int i = 0;i<2*numEdges; i+=2)
	{
		edgeVec.x = edges[i+1].x - edges[i].x;
		edgeVec.y = edges[i+1].z - edges[i].z;
		
		if(abs(edgeVec.x)<0.01f)
		{
			if(abs(rcBound.Left()-edges[i].x)<minr)
				leftEdges.push_back(i);
			else if(abs(rcBound.Right()-edges[i].x)<minr)
				rightEdges.push_back(i);
		}
		else if(abs(edgeVec.y)<0.01f)
		{
			if(abs(rcBound.Top()-edges[i].z)<minr)
				topEdges.push_back(i);
			else if(abs(rcBound.Bottom()-edges[i].z)<minr)
				bottomEdges.push_back(i);
		}
	}
	
	size_t sz = 0;
	sz += leftEdges.size();
	sz += rightEdges.size();
	sz += topEdges.size();
	sz += bottomEdges.size();
	
	if(!sz)
		return NULL;

	rtRegion * region = new rtRegion();
	region->rx = short(rx);
	region->ry = short(ry);

	region->gates.resize(sz);
	
#define FIND_MINMAX(ch)							\
	{											\
		if(edges[idx].ch>edges[idx+1].ch)		\
		{										\
			imin = idx + 1;						\
			imax = idx;							\
		}										\
		else									\
		{										\
			imin = idx;							\
			imax = idx + 1;						\
		}										\
	}											\

#define CHECK_GATE_W(ch)														\
	{																			\
		if(edges[imax].ch-edges[imin].ch<tileMesh->header->walkableRadius)		\
			continue;															\
	}																			\

	//Left Edge
	int ie = 0,imin,imax;
	for(int i = 0;i<leftEdges.size();++i)
	{
		int idx = leftEdges[i];
	
		FIND_MINMAX(z);

		CHECK_GATE_W(z);
		
		region->gates[ie].p0.x = i_math::clamp_f(edges[imin].z,rcBound.Top(),rcBound.Bottom());
		region->gates[ie].p1.x = i_math::clamp_f(edges[imax].z,rcBound.Top(),rcBound.Bottom());

		region->gates[ie].p0.y = edges[imin].y;
		region->gates[ie].p1.y = edges[imax].y;

		region->gates[ie].flag.gateType = LeftGate; 		

		++ie;
	}
	
	//Right Edge
	for(int i = 0;i<rightEdges.size();++i)
	{
		int idx = rightEdges[i];
		
		FIND_MINMAX(z);

		CHECK_GATE_W(z);
		
		region->gates[ie].p0.x = i_math::clamp_f(edges[imin].z,rcBound.Top(),rcBound.Bottom());
		region->gates[ie].p1.x = i_math::clamp_f(edges[imax].z,rcBound.Top(),rcBound.Bottom());
		
		region->gates[ie].p0.y = edges[imin].y;
		region->gates[ie].p1.y = edges[imax].y;

		region->gates[ie].flag.gateType = RightGate; 
		
		++ie;
	}

	//Top Edge
	for(int i = 0;i<topEdges.size();++i)
	{
		int idx = topEdges[i];
		
		FIND_MINMAX(x);

		CHECK_GATE_W(x);
		
		region->gates[ie].p0.x = i_math::clamp_f(edges[imin].x,rcBound.Left(),rcBound.Right());
		region->gates[ie].p1.x = i_math::clamp_f(edges[imax].x,rcBound.Left(),rcBound.Right());

		region->gates[ie].p0.y = edges[imin].y;
		region->gates[ie].p1.y = edges[imax].y;
		
		region->gates[ie].flag.gateType = TopGate; 
		
		++ie;
	}	

	//Bottom Edge
	for(int i = 0;i<bottomEdges.size();++i)
	{
		int idx = bottomEdges[i];
		
		FIND_MINMAX(x);

		CHECK_GATE_W(x);

		region->gates[ie].p0.x = i_math::clamp_f(edges[imin].x,rcBound.Left(),rcBound.Right());
		region->gates[ie].p1.x = i_math::clamp_f(edges[imax].x,rcBound.Left(),rcBound.Right());

		region->gates[ie].p0.y = edges[imin].y;
		region->gates[ie].p1.y = edges[imax].y;

		region->gates[ie].flag.gateType = BottomGate; 
		
		++ie;
	}
#undef FIND_MINMAX
#undef CHECK_GATE_W

	// 计算连通关系
	int numGates = region->gates.size();
	int  n = numGates*numGates;
	bool *bCon = new bool[n];

	memset(bCon,false,n*sizeof(bool));
	
	for(int i = 0;i<numGates;++i)
		bCon[i*numGates+i] = true;

	rtGate * gateSrc = NULL;
	rtGate * gateDst = NULL;
	dtVec3 csrc,cdst;
	dtVec3 p0,p1;

	i_math::rectf rcInner = rcBound;
	rcInner.inflate(-0.6f,-0.6f,-0.6f,-0.6f);
	
	dtMeshTileQuery * queryCon = new dtMeshTileQuery();
	queryCon->init(1000);
	queryCon->setMeshTile(tileMesh);

	for(int i = 0;i<numGates;++i)
	{
		gateSrc = &region->gates[i];
		region->getGateMid(gateSrc,csrc);

		for (int j = 0;j<numGates;++j)
		{
			if(i<j)	continue;
			
			if(i!=j)
			{
				gateDst = &region->gates[j];				
				region->getGateMid(gateDst,cdst);

				if(queryCon->isCon(csrc,cdst))
				{
					bCon[numGates*i+j] = true;
					bCon[numGates*j+i] = true;
				}
			}
			else
			{
				bCon[numGates*i+j] = true;
				bCon[numGates*j+i] = true;
			}
		}
	}
	queryCon->unInit();
	delete queryCon;
	
	//连通区域划分
	RegID curReg = 1;
	std::vector<int> stackCon;
	stackCon.push_back(0);
	while(!stackCon.empty())
	{
		int ig = stackCon.back();
		stackCon.pop_back();
		region->gates[ig].regID = curReg;
		for(int i = 0;i<numGates;++i)
		{
			if(bCon[ig*numGates + i]&&region->gates[i].regID==INVALID_REGID)
			{
				region->gates[i].regID = curReg;
				stackCon.push_back(i);
			}
		}
		if(stackCon.empty())
		{
			if(curReg==255)
				break;

			for(int i = 0;i< numGates;++i)
			{
				if(region->gates[i].regID==INVALID_REGID)
				{
					stackCon.push_back(i);
					curReg++;
					break;
				}
			}
		}
	}

	delete []bCon;
	bCon = NULL;
	
	region->rx = rx;
	region->ry = ry;

	_newRegions.push_back(region);
	
	//add region to world
	addRegion(region);

	return region;
}

struct rtGateRef_Inner
{
	short rx,ry;
	short igate;	
	unsigned char regID;
	unsigned char gateType;
};

rtRegRef rtWorld::getRegByGate(const rtGateRef &src)
{
	rtGateRef_Inner *gRef = (rtGateRef_Inner *)(&src);
	rtRegRef refReg = src;
	rtGateRef_Inner *gRefDst = (rtGateRef_Inner *)(&refReg);
	gRefDst->gateType = 0;
	gRefDst->igate = 0;
	return refReg;
}

bool rtWorld::getBestConGates(rtGateRef &src,/*const i_math::vector2df &dirTarget,*/
							  rtGateRef *gates,int*mincosts,int &numGates,
							  int dir,int maxgates) const
{
	numGates = 0;
	rtGateRef_Inner *gRef = (rtGateRef_Inner *)(&src);
	int xoff,yoff;
	getDirOffset(dir,xoff,yoff);
	
	rtRegion * region = getRegion(gRef->rx,gRef->ry);	
	rtRegion *rgNei = getRegion(gRef->rx + xoff,gRef->ry + yoff);
	if(!rgNei)
		return false;

	bool brev = false;
	rtTable * table = NULL;
	unsigned char gateType = 0;

	if(0==yoff)
	{
		if(xoff>0)//向右
		{
			table = _rtTableMgr.GetTable(gRef->rx,gRef->ry,0);
			gateType = LeftGate;
			brev = false;
		}
		else//向左
		{
			table = _rtTableMgr.GetTable(gRef->rx-1,gRef->ry,0);
			gateType = RightGate;
			brev = true;
		}
	}
	else
	{
		if(yoff>0)//向下
		{
			table = _rtTableMgr.GetTable(gRef->rx,gRef->ry,1);
			gateType = TopGate;
			brev = false;
		}
		else //向上
		{
			table = _rtTableMgr.GetTable(gRef->rx,gRef->ry-1,1);
			gateType = BottomGate;
			brev = true;
		}
	}
	
	if(!table)
		return false;

	RegID * pSrcReg = NULL;
	RegID * pDstReg = NULL;
	if(brev)
	{
		pSrcReg = table->reg1;
		pDstReg = table->reg0;
	}
	else
	{
		pSrcReg = table->reg0;
		pDstReg = table->reg1;
	}
	
	rtGate * gateSrc = &region->gates[gRef->igate];
	dtVec3 mid0,mid1;
	region->getGateMid(gateSrc,mid0);

	for(int i = 0;i<table->numPairs;++i)
	{
		if(pSrcReg[i]==gRef->regID)
		{
			int minCost = 0x0fffffff;
			int ig = -1;
			RegID idReg = 0;
			for(int j = 0;j<rgNei->gates.size();++j)
			{
				rtGate * gateDst = &rgNei->gates[j];
				if( gateDst->regID==pDstReg[i]&&
					gateDst->flag.gateType==gateType)
				{
					rgNei->getGateMid(gateDst,mid1);
					int cost = int((mid0-mid1).getLength());//getGateCost(gateSrc,gateDst,dirTarget);
					if(cost<minCost)
					{
						minCost = cost;
						ig = j;
						idReg = pDstReg[i];
					}
				}
			}

			if(ig>=0)
			{
				rtGateRef_Inner *gRefDst = (rtGateRef_Inner *)(gates + numGates);
				gRefDst->gateType = gateType;
				gRefDst->igate = ig;
				gRefDst->rx = gRef->rx + xoff;
				gRefDst->ry = gRef->ry + yoff;
				gRefDst->regID = idReg;
				mincosts[numGates] = minCost;
				++numGates;
			}
		}
		if(numGates>=maxgates)
			break;
	}

	return numGates>0;
}

bool rtWorld::isGateSameReg(const rtGateRef &src,const rtGateRef &dst)
{
	rtGateRef_Inner * gRef0 = (rtGateRef_Inner *)(&src);
	rtGateRef_Inner * gRef1 = (rtGateRef_Inner *)(&dst);

	if( gRef0->rx==gRef1->rx&&
		gRef0->ry==gRef1->ry&&
		gRef0->regID==gRef1->regID)
		return true;

	return false;
}

int rtWorld::getDistGateToGate(const rtGateRef &gstart,const rtGateRef &gend)
{
	dtVec3 mid0,mid1;

	int dist = 0x0fffffff;
	rtGateRef_Inner * pRef0 = (rtGateRef_Inner *)&gstart;
	rtGateRef_Inner * pRef1 = (rtGateRef_Inner *)&gend;

	rtRegion * rg0 = getRegion(pRef0->rx,pRef0->ry);
	rtRegion * rg1 = getRegion(pRef1->rx,pRef1->ry);

	if(rg0&&rg1)
	{
		rtGate *g0 = &rg0->gates[pRef0->igate];
		rtGate *g1 = &rg1->gates[pRef1->igate];

		rg0->getGateMid(g0,mid0);
		rg1->getGateMid(g1,mid1);

		dist = int((mid0-mid1).getLength());
	}

	return dist;
}

bool rtWorld::getBestRegionGates(rtRegion *rg,dtVec3 &pos,rtGateRef *gates,int * mincosts,int &numGates,int maxgates)
{
	numGates = 0;
	int numReg = 0;
	for(int i = 0;i<rg->gates.size();++i)
		numReg = max(rg->gates[i].regID,numReg);
	
	int xOrg = rg->rx*255;
	int yOrg = rg->ry*255;

	dtVec3 mid;
	rtTable * table = NULL;
	for(int i = 1;i<=numReg;++i)
	{
		if(numGates>=maxgates)
			break;
		
		RegID id = RegID(i);
		int mincost = 0x0fffffff;
		int ig = -1;
		rtRegion * rgTo = NULL;
		for(int j = 0;j<rg->gates.size();++j)
		{
			rtGate * gatecur = &rg->gates[j];
			if( gatecur->regID!=id)
				continue;
			
			rg->getGateMid(gatecur,mid);
			int cost = int((mid-pos).getLength());
			
			if(cost<mincost)
			{
				mincost = cost;
				ig  = j;
				rgTo = rg;
			}
		}
		
		if(ig>=0)
		{
			//插入排序
			int k = numGates;
			for(;k>0;--k)
			{
				if(mincost>=mincosts[k-1])
					break;
				gates[k] = gates[k-1];
				mincosts[k] = mincosts[k-1];
			}
			
			rtGateRef_Inner * gRef = (rtGateRef_Inner *)(gates + k);
			rtGate * gate = &rgTo->gates[ig];
			gRef->rx = rgTo->rx;
			gRef->ry = rgTo->ry;
			gRef->gateType = gate->flag.gateType;
			gRef->igate = ig;
			gRef->regID = id;
			mincosts[k] = mincost;

			++numGates;
		}
	}

	return numGates>0;
}

bool rtWorld::getGateMid(const rtGateRef &gate,dtVec3 &mid) const
{
	rtGateRef_Inner * gRef = (rtGateRef_Inner *)(&gate);
	rtRegion * rg = getRegion(gRef->rx,gRef->ry);
	rtGate * g = NULL;
	if(rg)
	{
		assert(gRef->igate<rg->gates.size());
		g = &rg->gates[gRef->igate];
	}
	
	if(g)
	{
		dtVec3 start,end;
		rg->getGatePos(g,start,end);
		mid = (start + end)*0.5f;
		
		switch(g->flag.gateType)
		{
			case	LeftGate:
				mid.x += 0.01f; break;
			case	RightGate:
				mid.x -= 0.01f; break;
			case	TopGate:
				mid.z += 0.01f; break;
			case	BottomGate:
				mid.z -= 0.01f; break;
			default: break;
		}

		return true;
	}
	return false;
}

bool rtWorld::getGatePos(const rtGateRef &gate,dtVec3 &pos0,dtVec3 &pos1) const
{
	rtGateRef_Inner * gRef = (rtGateRef_Inner *)(&gate);
	rtRegion * rg = getRegion(gRef->rx,gRef->ry);
	rtGate * g = NULL;
	if(rg)
	{
		assert(gRef->igate<rg->gates.size());
		g = &rg->gates[gRef->igate];
	}

	if(g)
	{
		dtVec3 start,end;
		rg->getGatePos(g,pos0,pos1);
		return true;
	}	
	return false;
}

bool rtWorld::load(CDataPacket &dp)
{
	Header header;

	dp.Data_ReadData(&header,sizeof(header));

	_dataRegions.resize(header.numRegion);

	for(int i = 0;i<header.numRegion;++i)
	{
		rtRegion &rg = _dataRegions[i];
		rg.Load(dp,header.ver);
		int k = getRegionKey(rg.rx,rg.ry);
		_mpRegions[k] = &rg;
	}
	
	_rtTableMgr.Load(dp,header.ver);

	return true;
}

void rtWorld::save(CDataPacket &dp)
{
	Header header;
	header.numRegion = _mpRegions.size();
	header.ver = getVersion();

	dp.Data_WriteData(&header,sizeof(header));

	itRegionMap it = _mpRegions.begin();
	for(;it!=_mpRegions.end();++it)
	{
		rtRegion * rt = (*it).second;
		rt->Save(dp);
	}
	_rtTableMgr.Save(dp);
}

