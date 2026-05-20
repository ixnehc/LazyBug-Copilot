
#include "stdh.h"

#include "rtTableManager.h"

#include "assert.h"

#define  TOKEY(x,y) x<<16|(0x0000ffff&y)

rtTableManager::rtTableManager(void)
{
}

rtTableManager::~rtTableManager(void)
{
}

void rtTableManager::Clear()
{
	for(int i = 0;i<_newtalbes.size();++i)
	{
		rtTable * t = _newtalbes[i];
		
		if(t->reg0)
			delete t->reg0;

		if(t->reg1)
			delete t->reg1;

		delete t;
	}

	_mptables.clear();
	_rtTablesBuffer.clear();
	_newtalbes.clear();
	_regIDBuffer.clear();
}

inline bool isGateCon(rtRegion *rg0,rtGate *g0,rtRegion *rg1,rtGate *g1,float h)
{
	dtVec3 s0,e0,s1,e1,out;
	
	rg0->getGatePos(g0,s0,e0);
	rg1->getGatePos(g1,s1,e1);

	if(g0->p1.x<g1->p0.x||g0->p0.x>g1->p1.x)
		return false;

	i_math::line2df line0,line1;

	line0.start.set(g0->p0.x,g0->p0.y);
	line0.end.set(g0->p1.x,g0->p1.y);
	
	line1.start.set(g1->p0.x,g1->p0.y);
	line1.end.set(g1->p1.x,g1->p1.y);

	float k0 = (line0.end.y - line0.start.y)/(line0.end.x - line0.start.x);
	float b0 = line0.start.y - k0*line0.start.x;
	
	float k1 = (line1.end.y - line1.start.y)/(line1.end.x - line1.start.x);
	float b1 = line1.start.y - k1*line1.start.x;
	
	if(k1==k0)
	{
		if(abs(s0.y-s1.y)<=h)
			return true;
	}
	else
	{
		float x = (b1-b0)/(k0-k1);
		float y = (k0*b1 - k1*b0)/(k0-k1);
		
		float n0 = y,n1 = y;

		if(x<g0->p0.x)
			n0 = s0.y;
		else if(x>g0->p1.x)
			n0 = e0.y;
		
		if(x<g1->p0.x)
			n1 = s1.y;
		else if(x>g1->p1.x)
			n1 = e1.y;

		if(abs(n0-n1)<h)
			return true;
	}

	return false;
}

bool rtTableManager::UpdateTable(rtRegion * src,rtRegion *dst)
{
	assert(abs(src->rx-dst->rx)<=1&&abs(src->ry-dst->ry)<=1);
	
	
	short bx,by;
	static const int maxgates = 255;
	rtGate * gatesSrc[maxgates];
	rtGate * gatesDst[maxgates];
	int numSrcGates = 0;
	int numDstGates = 0;
	int lr_tb = 0;

	if(src->rx==dst->rx)
	{
		bx = src->rx;
		if(src->ry<dst->ry)
		{
			src->getGates(gatesSrc,numSrcGates,maxgates,BottomGate);
			dst->getGates(gatesDst,numDstGates,maxgates,TopGate);
			by = src->ry;
		}
		else
		{
			src->getGates(gatesDst,numDstGates,maxgates,TopGate);
			dst->getGates(gatesSrc,numSrcGates,maxgates,BottomGate);
			by = dst->ry;
		}
		lr_tb = 1;
	}
	else if(src->ry==dst->ry)
	{
		by = src->ry;
		if(src->rx<dst->rx)
		{
			src->getGates(gatesSrc,numSrcGates,maxgates,RightGate);
			dst->getGates(gatesDst,numDstGates,maxgates,LeftGate);
			bx = src->rx;
		}
		else
		{
			src->getGates(gatesDst,numDstGates,maxgates,LeftGate);
			dst->getGates(gatesSrc,numSrcGates,maxgates,RightGate);
			bx = dst->rx;
		}
		lr_tb = 0;
	}
	
	std::vector<RegID> srcRegs;
	std::vector<RegID> dstRegs;

	for(int i = 0;i<numSrcGates;++i)
	{
		rtGate * gs = gatesSrc[i];

		for(int j = 0;j<numDstGates;++j)
		{
			rtGate *gd = gatesDst[j];
			
			if(isGateCon(src,gs,dst,gd,0.2f))
			{
				int k = 0;
				for(;k<srcRegs.size();++k)
					if(gs->regID==srcRegs[k])
						break;
				//
				if(k<srcRegs.size()&&
					dstRegs[k]==gd->regID)
					continue;
				
				srcRegs.push_back(gs->regID);
				dstRegs.push_back(gd->regID);
			}
		}
	}

	AddTableRouting(bx,by,srcRegs.data(),dstRegs.data(),srcRegs.size(),lr_tb);

	return true;	
}

void rtTableManager::AddTableRouting(short rx,short ry,RegID * src,RegID *dst,int numPairs,int lr_tb)
{
	int key = TOKEY(rx,ry);
	itMapRounting it = _mptables.find(key);
	
	rtTable * rt = NULL;
	if(it!=_mptables.end())
	{
		rtPair rp = (*it).second;
		rt = rp.rts[lr_tb];
		if(rt)
		{
			int i = 0;
			for(;i<_newtalbes.size();++i)
			{
				if(_newtalbes[i]==rt)
				{
					SAFE_DELETE_ARRAY(rt->reg0);
					SAFE_DELETE_ARRAY(rt->reg1);
					break;
				}
			}
			if(i>=_newtalbes.size())
			{
				rt = new rtTable();
				_newtalbes.push_back(rt);

				rp.rts[lr_tb] = rt;
				_mptables[key] = rp;
			}
		}
		else
		{
			rt = new rtTable();
			_newtalbes.push_back(rt);

			rp.rts[lr_tb] = rt;
			_mptables[key] = rp;
		}
	}
	else
	{
		rt = new rtTable();
		_newtalbes.push_back(rt);

		rtPair rp;
		rp.rts[lr_tb] = rt;
		_mptables[key] = rp;
	}

	rt->numPairs = numPairs;
	rt->reg0 = new RegID[numPairs];
	rt->reg1 = new RegID[numPairs];
	memcpy(rt->reg0,src,numPairs*sizeof(RegID));
	memcpy(rt->reg1,dst,numPairs*sizeof(RegID));
}

rtTable * rtTableManager::GetTable(rtRegion * src,int lr_tb) const
{
	if(!src)
		return NULL;

	return GetTable(src->rx,src->ry,lr_tb);
}

rtTable * rtTableManager::GetTable(int x,int y,int lr_tb) const
{
	rtTable * rt = NULL;

	int key = TOKEY(x,y);
	MapRounting::const_iterator it = _mptables.find(key);

	if(it!=_mptables.end())
	{
		rtPair rp = (*it).second;
		rt = rp.rts[lr_tb];
	}
	return rt;
}

void rtTableManager::RemoveTable(rtRegion *src,int lr_tb)
{
	rtTable * rtLR = NULL;
	rtTable * rtTD = NULL;

	int key = TOKEY(src->rx,src->ry);
	itMapRounting it = _mptables.find(key);
	
	rtTable *rt = NULL;

	if(it!=_mptables.end())
	{
		rtPair rp = (*it).second;
		rt = rp.rts[lr_tb];
		rp.rts[lr_tb] = NULL;
		_mptables[key] = rp;
	}

	if(rt&&!_newtalbes.empty())
	{
		for(int i = 0;i<_newtalbes.size();++i)
		{
			if(rt==_newtalbes[i])
			{
				SAFE_DELETE_ARRAY(rt->reg0);
				SAFE_DELETE_ARRAY(rt->reg1);

				delete rt;

				_newtalbes.erase(_newtalbes.begin()+i);
				break;
			}
		}
	}
}

void rtTableManager::Load(CDataPacket &dp,DWORD ver)
{
	size_t nPairs = dp.Data_NextDword();
	size_t nRoutingTable = dp.Data_NextDword();
	size_t nRountingRegion = dp.Data_NextDword();
	
	_regIDBuffer.resize(nPairs);
	_rtTablesBuffer.resize(nRoutingTable);
	
	int irt = 0;
	int iregID = 0;
	for(int i = 0;i<nRountingRegion;++i)
	{
		int key = dp.Data_NextDword();
		rtPair rp;
		for(int k = 0;k<2;++k){
			int nPair = dp.Data_NextWord();
			if(nPair>0){
				rtTable &rt = _rtTablesBuffer[irt++];
				rt.numPairs = nPair;
				size_t oldSz = _regIDBuffer.size();
				_regIDBuffer.resize(oldSz + 2*nPair);
				rt.reg0 = _regIDBuffer.data() + iregID;
				rt.reg1 = _regIDBuffer.data() + iregID + nPair;
				dp.Data_ReadData(rt.reg0,nPair*sizeof(RegID));
				dp.Data_ReadData(rt.reg1,nPair*sizeof(RegID));
				rp.rts[k] = &rt;
				iregID += 2*nPair;
			}
			else
				rp.rts[k] = NULL;
		}
		_mptables[key] = rp;
	}
	assert(irt==nRoutingTable);
}

void rtTableManager::Save(CDataPacket &dp)
{
	//路由表的数量
	DWORD nRoutingTable  = 0;
	DWORD nPairs = 0;
	itMapRounting it = _mptables.begin();
	for(;it!=_mptables.end();++it){
		rtPair rp = (*it).second;
		for(int i = 0;i<2;++i)
			if(rp.rts[i]&&rp.rts[i]->numPairs>0)
			{
				++nRoutingTable;
				nPairs += 2*rp.rts[i]->numPairs;
			}
	}
	
	dp.Data_NextDword() = nPairs;
	dp.Data_NextDword() = nRoutingTable;
	dp.Data_NextDword() = _mptables.size();
	
	it = _mptables.begin();
	for(;it!=_mptables.end();++it)
	{
		int key = (*it).first;
		
		dp.Data_NextDword() = key;

		rtPair rp = (*it).second;
		for(int i = 0;i<2;++i)
		{
			rtTable * rt = rp.rts[i];
			if(!rt||rt->numPairs==0)
				dp.Data_NextWord() = 0;
			else
			{
				dp.Data_NextWord() = rt->numPairs;
				dp.Data_WriteData(rt->reg0,rt->numPairs*sizeof(RegID));
				dp.Data_WriteData(rt->reg1,rt->numPairs*sizeof(RegID));
			}
		}
	}
}



