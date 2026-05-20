
#include "stdh.h"

#include "rtRegion.h"

void rtRegion::getGates(rtGate ** gatesRet,int & numGatesRet,int maxGates,unsigned char gateType,RegID reg/* = INVALID_REGID*/)
{
	numGatesRet = 0;
	for(int i = 0;i<gates.size();++i)
	{
		if(numGatesRet>maxGates)
			break;
		rtGate * gate = &gates[i];
		if((gate->flag.gateType==gateType)&&(reg==INVALID_REGID||gate->regID==reg))
		{
			gatesRet[numGatesRet] = gate;
			++numGatesRet;
		}
	}
}

inline unsigned char encodeF2C_ceil(float v)
{
	 return (unsigned char)ceilf(v*255.0f/64.0f);
}

inline unsigned char encodeF2C_floor(float v)
{
	return (unsigned char)floorf(v*255.0f/64.0f);
}

inline float decodeC2F(unsigned char v)
{
	const float s = 64.0f/255.0f;
	return float(v)*s;
}

void rtRegion::getGatePos(rtGate * gate,dtVec3 &start,dtVec3 &end)
{

	switch(gate->flag.gateType)
	{
	case LeftGate:
	case RightGate:
		{
			float x = float(rx)*REGION_W;
			if(!(gate->flag.gateType&LowGate))
				x += REGION_W;
			
			start.x = x;
			start.y = gate->p0.y;
			start.z = gate->p0.x;

			end.x = x;
			end.y = gate->p1.y;
			end.z = gate->p1.x;

			break;
		}
	case TopGate:
	case BottomGate:
		{
			float z = float(ry)*REGION_W;
			if(!(gate->flag.gateType&LowGate))
				z += REGION_W;

			start.x = gate->p0.x;
			start.y = gate->p0.y;
			start.z = z;

			end.x = gate->p1.x;
			end.y = gate->p1.y;
			end.z =  z;

			break;
		}
	default: break;
	}
}

void rtRegion::Save(CDataPacket &dp)
{
	dp.Data_NextShort() = rx;
	dp.Data_NextShort() = ry;	
	DP_WriteVector(dp,gates);
}

void rtRegion::Load(CDataPacket &dp,DWORD ver)
{
	rx = dp.Data_NextShort();
	ry = dp.Data_NextShort();
	DP_ReadVector(dp,gates);
}