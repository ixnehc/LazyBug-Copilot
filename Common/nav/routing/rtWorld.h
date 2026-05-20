
#pragma once

#include <map>

#include "rtRegion.h"

#include "rtTableManager.h"

#include "datapacket/DataPacket.h"

class rtRegion;
struct dtMeshTile;
class rtWorld
{
public:
	rtWorld(void);
	~rtWorld(void);
	
	unsigned int getVersion(){return 1;}

	
	rtTableManager * getRoutingMgr(){return &_rtTableMgr;}

	float getRegionWidth(){return float(REGION_W);}
	// Region
	void addRegion(rtRegion * region);
	void removeRegion(rtRegion * region);
	rtRegion * getRegion(short x,short y) const;

	void calcRegionLoc(const dtVec3 &pos,int &rx,int &ry,int &x,int &y) const;

	//for path search
	bool	 getBestConGates(rtGateRef&src,rtGateRef *gates,int *cost,int &numGates,int dir,int maxgates) const;	
	bool	 getBestRegionGates(rtRegion *rg,dtVec3 &pos,rtGateRef *gates,int * mincosts,int &numGates,int maxgates);
	int		 getDistGateToGate(const rtGateRef &gstart,const rtGateRef &gend);
	rtRegRef getRegByGate(const rtGateRef &src);
	bool	 getGateMid(const rtGateRef &gate,dtVec3 &mid) const;

	bool getGatePos(const rtGateRef &gate,dtVec3 &pos0,dtVec3 &pos1) const;
	//load and save
	bool load(CDataPacket &dp);
	void save(CDataPacket &dp);
	void clear();

	bool isGateSameReg(const rtGateRef &src,const rtGateRef &dst);

	//buid Region
	rtRegion * build(dtMeshTile * tileMesh,int rx,int ry,const dtVec3 * edges,int numEdges);
	
	typedef std::map<int,rtRegion *>	  RegionMap;
	typedef RegionMap::iterator			  itRegionMap;

protected:
	
	struct Header
	{
		DWORD dataSize;
		DWORD ver;
		DWORD numRegion;
	};

private:
	rtTableManager			_rtTableMgr;
	RegionMap				_mpRegions;
	std::vector<rtRegion>   _dataRegions;	
	std::vector<rtRegion *> _newRegions;
};



