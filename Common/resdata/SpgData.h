#pragma once

#include <string>
#include <vector>


#include "ResData.h"

#include "fvfex/fvfex.h"

class CDataPacket;

struct SpgData:public ResData
{
	enum 
	{
		Res_Ver = 2,
	};

	DECLARE_CLASS(SpgData);

	virtual 	ResType GetType()	{		return Res_Spg;	}
	virtual const char *GetTypeName()	{		return "SpeedGlass";	}
	virtual const char *GetTypeSuffix()	{		return "spg";	}

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);

	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);
	
	virtual DWORD GetVersion(){ return 3;}
	
	virtual void CollectRefs(std::vector<std::string>&buf)	
	{
		buf.push_back(std::string(".\\d.dds"));
		buf.push_back(std::string(".\\s.dds"));
		buf.push_back(std::string(".\\n.dds"));
	}

public:
	// the vertex declaration flag
	FVFEx fvf;

	std::vector<WORD> indices;				 // the index.
	i_math::aabbox3df aabb;					 // axis aligning bound box

	//version2
	std::vector<i_math::vector4df> blendPos; //.xyz pos  .w blend factor
	std::vector<i_math::vector3df> normals;	 //
	std::vector<i_math::vector2df> uvs;		 //
};






