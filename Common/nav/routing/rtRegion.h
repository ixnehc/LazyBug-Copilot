
#pragma once

#include "routing.h"

#include "datapacket/DataPacket.h"

#include "../detour/DetourMath.h"

// [0-1] gate flag
#define VerticalGate 0x1
#define LowGate		0x2

#define LeftGate	(VerticalGate|LowGate)		
#define RightGate	VerticalGate			
#define TopGate		LowGate	
#define BottomGate	0

#define REGION_W	64

struct  rtGate
{
	rtGate(){regID = 0;/*v0 = v1 = 0;*/flag.gateType = 0;}
// 	unsigned char v0;
// 	unsigned char v1;
	dtVec2   p0,p1;
	RegID	 regID;
	struct
	{
		unsigned char gateType:2;
	} flag;
};

struct rtTable
{	
	unsigned short numPairs;
	RegID * reg0;			
	RegID * reg1;	
};

class rtRegion
{
public:
	short rx,ry;
	std::vector<rtGate> gates;
	
	void getGates(rtGate **gatesRet,int &numGatesRet,int maxGates,unsigned char gateType,RegID reg=INVALID_REGID);
	void getGatePos(rtGate * gate,dtVec3 &start,dtVec3 &end);
	void getGateMid(rtGate * gate,dtVec3 &mid)
	{
		dtVec3 start,end;
		getGatePos(gate,start,end);
		mid = (start+end)*0.5f;
	}
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp,DWORD ver);
};

extern inline unsigned char encodeF2C_ceil(float v);
extern inline unsigned char encodeF2C_floor(float v);





