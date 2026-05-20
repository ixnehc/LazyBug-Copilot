
#pragma once

#include "rtRegion.h"

#include "datapacket/DataPacket.h"

#include <map>

class rtTableManager
{
public:
	rtTableManager(void);
	~rtTableManager(void);
	//update routing table ,if not exsit create it
	bool UpdateTable(rtRegion * src,rtRegion *dst);
	rtTable * GetTable(rtRegion * src,int lr_tb) const;
	rtTable * GetTable(int x,int y,int lr_tb) const;

	void RemoveTable(rtRegion *src,int lr_tb);
	
	void Load(CDataPacket &dp,DWORD ver);
	void Save(CDataPacket &dp);

	void Clear();

protected:
	//remove routiong table
	void AddTableRouting(short rx,short ry,RegID * src,RegID *dst,int numPairs,int lr_tb);
	
	struct rtPair
	{
		rtPair(){memset(rts,NULL,sizeof(rts));}
		rtTable  *rts[2];	//0 : routing right to left 1:routing top to bottom
	};

	typedef std::map<int,rtPair> MapRounting;
	typedef MapRounting::iterator itMapRounting;
	
private:
	MapRounting					_mptables;
	std::vector<rtTable>		_rtTablesBuffer;
	std::vector<rtTable *>		_newtalbes;
	std::vector<RegID>			_regIDBuffer;
};


