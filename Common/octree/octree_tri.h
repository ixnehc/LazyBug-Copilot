#pragma  once

#include "../class/class.h"





#define MAX_GETNODE_BUFFERSIZE (256)


class OcTreeTri;
class CDataPacket;
class OTNodeTri
{
public:
	DEFINE_CLASS(OTNodeTri);
	OTNodeTri();

	// destructor
	~OTNodeTri();

	void Init(i_math::aabbox3df &aabb,int depthCur,OcTreeTri *pOwner);
	void Clear();
	void ClearTris();

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	bool AddTriangle(unsigned short idxTriangle);
//	bool AddTriangle2(int idxTriangle);

	i_math::aabbox3df &GetAABB();
	BOOL IsLeaf();

	OTNodeTri &operator=(const OTNodeTri &src)
	{
		_aabb=src._aabb;
		_indiceTri=src._indiceTri;
		memcpy(_childs,src._childs,sizeof(_childs));
		_pttnChilds=src._pttnChilds;
		_depth=src._depth;
		_owner=src._owner;

		return *this;
	}

	void GetTriangleByAABB(i_math::aabbox3df &aabb,std::vector<WORD>&tris);
	void GetTriangleByLine(i_math::line3df &line,std::vector<WORD>&tris);
	void GetLeafNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize);//get all the leaf nodes
	void GetNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize);//get all the nodes(leaf or not leaf)

public://seem as private
	i_math::aabbox3df _aabb;
	std::vector<unsigned short> _indiceTri;
	unsigned short _childs[8];//index to OctTreeNode
	unsigned short _pttnChilds;
	short _depth;

	OcTreeTri *_owner;

	int _DispatchTris(unsigned short*vecTriangleIdx,int nTiangleIdx);//return the tri count that is accepted by the children

	friend class OcTreeTri;
};

struct OcTreeTriInfo
{
	WORD tris[3];
	WORD flag;
};


class OcTreeTri
{
public:
	OcTreeTri()
	{
		_nMaxTriPerNode=0;
		_nNodeDepth=0;
	}
	void Init(i_math::aabbox3df*boxes,DWORD nBoxes);
	void Clear();
	void ClearTris();

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);


	void SetMaxTrianglePerNode(int nTris)
	{
		_nMaxTriPerNode=nTris;
	}
	void SetNodeDepth(int nDepth)
	{
		_nNodeDepth=nDepth;
	}
	inline int GetMaxTrianglePerNode()
	{
		return _nMaxTriPerNode;
	}
	inline int GetNodeDepth()
	{
		return _nNodeDepth;
	}

	bool AddTriangles(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,BYTE flag);
	void GetTriangleByAABB(i_math::aabbox3df &aabb,std::vector<i_math::vector3df>&tris,std::vector<BYTE>&flags);//添加在tris/flags的尾部(不会清空它们)

	void GetTriangleByLine(i_math::line3df &line,std::vector<i_math::vector3df>&tris,std::vector<BYTE>*flags);//添加在tris/flags的尾部(不会清空它们)
	void GetLeafNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize);//get all the leaf nodes
	void GetNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize);//get all the nodes(leaf or not leaf)


	unsigned short AllocOcTreeNode();
	OTNodeTri* OTNodeFromIdx(unsigned short idxNode);
	unsigned short AllocTriangle(OcTreeTriInfo &info);
	bool TriangleFromIndex(i_math::triangle3df &tri,unsigned short idxTriangle);//返回临时指针
	unsigned long TriFlagFromIndex(unsigned short idxTriangle);

	bool IsEmpty();

public://Seems as private
	i_math::aabbox3df _aabb;
	std::vector<i_math::vector3df>_vertices;
	std::vector<OcTreeTriInfo> _tris;
	std::vector<int>_roots;
	std::vector<OTNodeTri*>_nodes;

	int _nMaxTriPerNode;
	int _nNodeDepth;


};
