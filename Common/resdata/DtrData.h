#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/matrix43.h"
#include "../math/vector3d.h"
#include "../math/vector2d.h"
#include "../math/xform.h"

#include "ResData.h"
#include "../fvfex/fvfex_type.h"


#define FVFEx_DtrVtx (FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_BINORMAL|FVFEX_TANGENT|FVFEX_FLAG_TEX0|FVFEX_BONEINDICE0)
struct VtxDtr
{
	i_math::vector3df  pos;
	i_math::vector3df normal;
	i_math::vector3df binormal;
	i_math::vector3df tangent;
	DWORD boneindex;
	i_math::texcoordf uv;
};

struct DtrPieceInfo_old
{
	DWORD startCode;
	DWORD szCode;

	DWORD startVerticesShp;
	DWORD szVerticesShp;

	DWORD startIndicesShp;
	DWORD szIndicesShp;

	DWORD tpBuild;
	i_math::vector4df offset;

	i_math::vector3df posBase;//这个piece的中心点
};

struct DtrPieceInfo
{
	void FromOld(DtrPieceInfo_old &old)
	{
		tpData = 0;
		startData=old.startCode;
		szData = old.szCode;

		startVerticesShp = old.startVerticesShp;
		szVerticesShp = old.szVerticesShp;

		startIndicesShp = old.startIndicesShp;
		szIndicesShp = old.szIndicesShp;

		tpBuildHk = old.tpBuild;
		offsetHk = old.offset;

		posBase = old.posBase;
	}
	BYTE tpData;//0: hk, 1: px
	DWORD startData;
	DWORD szData;

	DWORD startVerticesShp;
	DWORD szVerticesShp;

	DWORD startIndicesShp;
	DWORD szIndicesShp;

	DWORD tpBuildHk;
	i_math::vector4df offsetHk;

	i_math::vector3df posBase;//这个piece的中心点
};

#define MAX_DTR_PIECE (64)

struct DtrPiecesData
{
	DtrPiecesData()
	{
		Zero();
	}

	void Zero()
	{
		iParent=-1;
		iParentSub=-1;

		tpData = 0;//hk

		nParts=0;
	}

	void Clear()
	{
		vertices.clear();
		indices.clear();
		primsParts.clear();

		verticesShp.clear();
		indicesShp.clear();
		data.clear();
		pieces.clear();

		Zero();
	}

	struct PrimRange
	{
		PrimRange()
		{
			ps=pc=0;
		};
		WORD ps,pc;//primtive start/count
	};


	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp,DWORD ver);

	short iParent;
	short iParentSub;//属于parent的第几个Piece

	i_math::aabbox3df aabb;//初始状态的包围盒,Local空间

	//Drawing Parts
	std::vector<VtxDtr> vertices;//顶点的位置位于各个piece的local 空间
	std::vector<WORD> indices;
	DWORD nParts;
	std::vector<PrimRange> primsParts;//这个数组记录了每个part的各个piece的prim range,
																//所以这个数组的大小为nParts*pieces.size()

	//Pieces for phys simulation
	std::vector<i_math::vector3df> verticesShp;//shape 的顶点
	std::vector<WORD> indicesShp;//shape的顶点索引
	BYTE tpData;//0: hk, 1: px
	std::vector<BYTE> data;//所有mopp的code
	std::vector<DtrPieceInfo> pieces;

};


struct DtrData:public ResData
{
	DECLARE_CLASS(DtrData);

	DtrData();
	virtual ~DtrData();
	virtual void Zero();
	virtual void Clean();

	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "dtr";	}
	virtual void CalcContent(std::string &s){}
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);


	std::vector<DtrPiecesData> piecesAll;
	
};
