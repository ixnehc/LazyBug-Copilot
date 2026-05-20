#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/matrix43.h"
#include "../math/vector3d.h"
#include "../math/vector2d.h"
#include "../math/xform.h"

#include "ResData.h"

#include "SkeletonInfo.h"



struct MeshData:public ResData
{
	DECLARE_CLASS(MeshData);

	struct VtxData
	{
		VtxData()
		{
			memset(this,0,sizeof(*this));
		}
		i_math::vector3df *pos;
		i_math::vector3df *normal;
		i_math::vector3df *binormal;
		i_math::vector3df *tangent;
		i_math::texcoordf *tex[8];
		DWORD *color;
		i_math::weight3f *weight;
		DWORD *boneindex0;
		DWORD *boneindex1;
		i_math::vector4df *colorF;
		i_math::vector3df *posExt;
		i_math::vector3df *normalExt;
		DWORD *reserve[13];//reserved for other vertex elements in the future,should always be NULL
		//XXXXX:More VtxData Element

	};

    struct FabricData
    {
        FabricData()
        {
        }
        struct Node
        {
			i_math::vector3df pos;

			WORD center;

			//neighbours
			WORD left;
			WORD right;
			WORD up;
			WORD down;
        };

        std::vector<Node> nodes;

		struct Constraint
		{
			WORD nodeA;
			WORD nodeB;
		};

		void AddUniqueConstraint(WORD nodeA,WORD nodeB)
		{
			if (nodeA==0xffff)
				return;
			if (nodeB==0xffff)
				return;
			if (nodeA>nodeB)
				Swap(nodeA,nodeB);

			for (int i=0;i<constraints.size();i++)
			{
				if ((nodeA==constraints[i].nodeA)&&(nodeB==constraints[i].nodeB))
					return;
			}

			constraints.resize(constraints.size()+1);
			constraints[constraints.size()-1].nodeA=nodeA;
			constraints[constraints.size()-1].nodeB=nodeB;

		}

		std::vector<Constraint> constraints;

		BOOL IsEmpty()		{			return nodes.size()<=0;		}

		void CopyFrom(FabricData &other)
		{
			nodes=other.nodes;
			constraints=other.constraints;
		}

        void Save(CDataPacket &dp);
        void Load(CDataPacket &dp);

        void Clear();

    };

	class  VtxFrames:public std::vector<VtxData>
	{
	public:
		DWORD m_nVtx;//vertex count in each frame
		VtxFrames();
		~VtxFrames();
		void Zero();
		void CopyFrom(VtxFrames &src);//Does not re-alloc the data(just copy data ptr)
		void Build(DWORD nVtx,DWORD nFrames);
		void Build(DWORD nVtx,VtxFrames&pattern);
		void Clean();
		void CleanFrame(DWORD iFrame);
		void VertexCopyFrom(VtxFrames &other,DWORD iDest,DWORD iSrc);
		BOOL VertexSame(DWORD i1,DWORD i2);
		BOOL FrameSame(DWORD f1,DWORD f2);
	};
	struct SegInfo
	{
		WORD bs,bc;//bone start/count
		WORD ps,pc;//primtive start/count
	};
	struct AtlasInfo
	{
		WORD channel;//which uv channel(0-based)
		WORD w,h;
		float gutter;
	};
	enum Flag
	{
		DoubleSided=1,
	};

	MeshData();
	virtual ~MeshData();
	virtual void Zero();
	virtual void Clean();

	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "msh";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);


	//Some Access functions
	void CalcAABB();
	BOOL HasTangentInfo();
	BOOL RemoveTangentInfo();
	BOOL BuildTangentInfo();

	//Operations
	BOOL RemoveFrame(DWORD iFrame);
	void CollapseDupeVtx();
	void MakeSegBone(DWORD minbone);
	BOOL MakeAtlasInfo(DWORD ch);//检查ch上有没有tex coord,如果有,根据mesh的大小自动生成一个uv atlas
	AtlasInfo *FindAtlasInfo(DWORD ch)
	{
		for (int i=0;i<atlases.size();i++)
		{
			if (atlases[i].channel==ch)
				return &atlases[i];
		}
		return NULL;
	}
	void MakeFabric(BOOL bDiagonalConstraints);

	//被融合的Mesh受骨骼的影响数数量不多于本身
	bool Merge(MeshData * data);
	bool AddMorph(MeshData *data);
	//properties
	//XXXXX:Mesh Head/Body
	//Head data
	i_math::aabbox3df aabb;//bounding box of this mesh,in the local space,(note: this aabb is adjusted by the matOff)
	DWORD nWeight;//weight count for every vertex(if 1,VtxData::weight is NULL)
	DWORD flag;
	std::vector<WORD> frames;//each frame record an index to vtxframes.
	DWORD nSkeletonBones;//The bone count of the skeleton this mesh is referring to,
												//if this value is 0,this mesh does not support a skeleton

	//Body data
	VtxFrames vtxframes;//the real data array,size should be 1 if bone's size is more than 1 

    FabricData fabric;

	struct LodMeshInfo
	{
		//each segment refers to a primitive range in vb and a bone range in segbones.
		//each segment has a max limit of its bones count,such as 25. That will ensure the hw
		//shader could render a segment in a single DrP
//		std::vector<WORD> segbones;
		std::vector<SegInfo> segs;
		std::vector<WORD> indice;
		float dist;
		void Load(CDataPacket &dp);
		void Save(CDataPacket &dp);
		void Clear();
	};
	
	struct MeshLod :public std::vector<LodMeshInfo>
	{
		void Clean();
		size_t GetNumberOfLods(){return size();}
	}lodInfos;
	
	void ResizeTo(VtxData& vtx,DWORD szSrc,DWORD szDst);
	//bones that affecting this surface,should be 1 bone if vtxframes's size is more than 1
	//each WORD records an index to the total bone array
	//this bone reference table are only used when the bones count is less than the hardware
	//capability.if too many bones for the hardware,we should use the segbones
	std::vector<WORD> bones;
	std::vector<WORD> segbones;
	std::vector<AtlasInfo> atlases;
};
