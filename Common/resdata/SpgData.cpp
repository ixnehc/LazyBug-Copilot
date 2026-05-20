
#include "stdh.h"

#include "stringparser/stringparser.h"

#include "datapacket/DataPacket.h"

#include <assert.h>


#include "SpgData.h"

IMPLEMENT_CLASS(SpgData);

void SpgData::SaveHeader(CDataPacket &dp)
{	
}
void SpgData::LoadHeader(CDataPacket &dp)
{	
}

void SpgData::Save(CDataPacket &dp)
{
	DWORD version = GetVersion();
	dp.Data_NextDword() = version;
	
	DP_PreSafeSave(dp);
	
	dp.Data_WriteData(&fvf,sizeof(fvf));
	
	DP_WriteVector(dp,blendPos);
	DP_WriteVector(dp,normals);
	DP_WriteVector(dp,uvs);

	DP_WriteVector(dp,indices);
	dp.Data_WriteData(&aabb,sizeof(aabb));
	
	DP_PostSafeSave();
}

void SpgData::Load(CDataPacket &dp)
{
	DWORD version = dp.Data_NextDword();
	
	DP_PreSafeLoad(dp);
	
	dp.Data_ReadData(&fvf,sizeof(fvf));
	
	if(version == 1){
		DWORD nVtx = dp.Data_NextDword();
		std::vector<BYTE> data;		
		DP_ReadVector(dp,data);
	}
	else{
		DP_ReadVector(dp,blendPos);
		DP_ReadVector(dp,normals);
		DP_ReadVector(dp,uvs);
	}
	
	DP_ReadVector(dp,indices);
	if (version<3)
	{
		std::string pathDifMap;
		dp.Data_ReadString(pathDifMap);
	}
	dp.Data_ReadData(&aabb,sizeof(aabb));
	
	DP_PostSafeLoad();
}








