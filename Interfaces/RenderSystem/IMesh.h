
#pragma once

#include "IResource.h"

#include "fvfex/fvfex_type.h"
#include "shaderlib/SLDefines.h"

#include "resdata/MeshData.h"

struct DrawMeshArg
{
	DrawMeshArg()
	{
		Zero();
	}
	void Zero()
	{
		fFrame=0;
		fillmode=3;//solid
		iLod = 0; //draw i.th lod
	}
	void SetFrame(float fFrame0)	{		fFrame=fFrame0;	}
	void SetFillMode(DWORD fm)	{		fillmode=fm;	}
	float fFrame;

	DWORD fillmode;
	int iLod;	
};

struct UVAtlasInfo
{
	DWORD channel:4;
	DWORD w:14;
	DWORD h:14;
	float gutter;
	//IMPORTANT:
	//if elements change,donot forget to make corresponding changing in CMeshMgr::_FillBody();
};

struct MeshSnapshotArg
{
	MeshSnapshotArg()
	{
		Zero();
	}
	void Zero()
	{
		fFrame=0;
		iLod = 0;
	}
	int iLod;
	float fFrame;

};

struct MeshSampleArg
{
	MeshSampleArg()
	{
		channel=-1;
		w=h=0;
		bTsi=FALSE;
	}
	int channel;//use which channel,if -1,use the first found channel that contains uv atlas info
	DWORD w,h;//the map size ,in pixel.If (0,0),use the w/h in the atlas info of the specified channel
	BOOL bTsi;
};



//mesh snapshot is used to get a mesh's actual(in world space) vertex info for each frame
//you could retrieve a snapshot using IMesh::ObtainSnapshot()
struct SpacialTester;
struct TriSample;
class IMatrice43;
class IMeshSnapshot
{
public:
	INTERFACE_REFCOUNT;
	virtual DWORD GetFrameCount()=0;
	//if mats is NULL,indicate to use identity matrix
	virtual BOOL TakeSnapshot(IMatrice43 *mats,MeshSnapshotArg &dmg)=0;
	virtual BOOL TakeSnapshot(i_math::matrix43f &mat,MeshSnapshotArg &msa)=0;

	//the following could/should be called AFTER calling TakeSnapshot(..)
	virtual DWORD GetVBCount()=0;
	virtual DWORD GetIBCount(DWORD iLod)=0;
	virtual FVFEx GetFVF()=0;
	virtual WORD *GetIndices(DWORD iLod)=0;
	virtual BOOL GetUVAtlasInfo(DWORD channel,UVAtlasInfo &ai)=0;//channel is 0-based
	virtual int FindUVAtlasChannel(DWORD start=0)=0;//find the first uv atlas channel available 
																							//search from start(including start)
																							//return -1 on failure
	//the returned pointer should not be kept for later use,it will be invalid when you call
	//any another GetXXX functions for vertex
	//fvf could be any combination of the vb elements this mesh contains
	//the returned stride is size of fvf ,in byte
	virtual void *GetVertices(FVFEx fvf,DWORD *stride)=0;
	virtual i_math::vector3df *GetPos()=0;
	virtual i_math::vector3df *GetNormal()=0;
	//NOTE:if arg is specified using default w/h,and there is no atlas info for that channel,will return FALSE
	virtual TriSample*GetMeshSamples(MeshSampleArg &arg,DWORD &nSamples)=0;
	//返回线段是否与mesh相交,如果相交dist为ray.start到相交点的距离,注意这个函数不是很快
	virtual BOOL HitTest(i_math::line3df &ray,float &dist)=0;

};

class IShader;
class IVertexBuffer;
class IIndexBuffer;
class IMesh:public IResource
{
public:
	virtual DWORD GetFrameCount()=0;

	virtual i_math::aabbox3df &GetAABB()=0;


	//Bind the mesh's vb to device and bind the world matrice to the shader,and draw	
	//pass NULL for mat to indicate an identity matrix
	virtual BOOL Draw(IShader *shader,i_math::matrix43f *mats,DWORD cMats,DrawMeshArg &dmg)	=0;
	virtual BOOL Draw(IShader *shader,i_math::matrix43f &mat,DrawMeshArg &dmg)=0;

	virtual IMeshSnapshot *ObtainSnapshot()=0;//get a mesh snapshot(with 1-refcounted)

	//Get the bone count of the skeleton this mesh is referring to,
	virtual DWORD GetBoneCount()=0;

	//Get the bones that this mesh is using(应该是骨架骨骼的一部分骨骼)
	virtual DWORD GetAffectingBoneCount()=0;
	virtual WORD *GetAffectBones()=0;

	//Get the weight count of each vertex
	virtual DWORD GetWeightCount()=0;

	virtual DWORD GetNumberOfLod() = 0;
	virtual float GetLodDist(DWORD iLod) = 0;
	//Get the features needed to draw this mesh
	//the returned pointer should not be kept
	//if NULL is returned,this mesh could not be drawn
	virtual FeatureCode GetFeature()=0;

	virtual BOOL GetBuffer(IVertexBuffer *&vb,IIndexBuffer *&ib,DWORD iLod=0)=0;

	virtual MeshData::FabricData *GetFabric() = 0;
};

class IMeshMgr:public IResourceMgr
{
public:
};
