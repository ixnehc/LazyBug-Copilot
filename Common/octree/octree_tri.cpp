/********************************************************************
	created:	2011/8/5   14:36
	file path:	e:\IxEngine\Common\octree
	author:		chenxi
	
	purpose:	存放三角形的八叉树
*********************************************************************/
#include "Stdh.h"
#include "commondefines/general_stl.h"
#include "../datapacket/DataPacket.h" 
#pragma warning ( disable: 4786 )
#pragma warning ( disable: 4018 )
#pragma warning ( disable: 4267 )
  

#include "octree_tri.h"  



int g_MAX_TRIANGLE_PER_NODE_DEFAULT=12;
int g_MAX_NODE_DEPTH_DEFAULT=6;
  
#define MAX_NODE_PER_TREE 64000
#define MAX_TRIANGLE_PER_TREE 64000

#define MAX_WSM_WIDTH 250
#define MAX_WSM_HEIGHT 250

#define WSM_VERSION_BASE (1)
#define WSM_VERSION_CURRENT WSM_VERSION_BASE


//////////////////////////////////////////////////////////////////////////
//OctTreeNode
OTNodeTri::OTNodeTri()
{
	memset(_childs,0,sizeof(_childs));
	_depth=0;
	_aabb.reset(0,0,0);
	_owner=0;
	_pttnChilds=0;
}


void OTNodeTri::Init(i_math::aabbox3df &aabb,int depthCur,OcTreeTri *pOwner)
{
	_depth = depthCur;

	memset(_childs,0,sizeof(_childs));
	_aabb=aabb;
	_owner=pOwner;
	_pttnChilds=0;
}



// destructor
OTNodeTri::~OTNodeTri()
{
	Clear();
}

void OTNodeTri::Clear()
{
	_aabb.reset(0,0,0);
	for (int i=0; i<8; ++i)
		_childs[i]=0;
	_pttnChilds=0;
	_depth=0;
	_owner=0;
	_indiceTri.clear();
}

void OTNodeTri::ClearTris()
{
	memset(_childs,0,sizeof(_childs));
	_depth=0;
	_pttnChilds=0;
	_indiceTri.clear();

}


void OTNodeTri::Save(CDataPacket &dp)
{
	DP_WriteVar(dp,_aabb);
	DP_WriteVectorN(dp,_indiceTri);
	DP_WriteArray(dp,_childs);
	DP_WriteVar(dp,_pttnChilds);
	DP_WriteVar(dp,_depth);
}

void OTNodeTri::Load(CDataPacket &dp)
{
	DP_ReadVar(dp,_aabb);
	DP_ReadVectorN(dp,_indiceTri);
	DP_ReadArray(dp,_childs);
	DP_ReadVar(dp,_pttnChilds);
	DP_ReadVar(dp,_depth);
}


i_math::aabbox3df &OTNodeTri::GetAABB()
{
	return _aabb;
}

int OTNodeTri::_DispatchTris(unsigned short*vecTriangleIdx,int nTriangleIdx)
{
	if (!_owner)
		return 0;
	i_math::vector3df middle = _aabb.getCenter();
	i_math::vector3df edges[8];
	_aabb.getCorners(edges);
	i_math::aabbox3df box;

	int nAccepted;
	nAccepted=0;
	int j;
	for (j=0;j<nTriangleIdx;j++)
	{
		i_math::triangle3df tri;
		if (!_owner->TriangleFromIndex(tri,vecTriangleIdx[j]))
			continue;
		bool bAccepted;
		bAccepted=false;
		int i;
		for (i=0;i<8;i++)
		{
			if (!(_pttnChilds&(1<<i)))
			{
				//The child is not instantiated yet,first we check whether its aabb could intersect with the triangle
				box.reset(middle);
				box.addInternalPoint(edges[i]);
				box.inflate(0.005f,0.005f,0.005f);//inflate a little to avoid some edge effects
				if(!tri.intersectsWithAABB(box))
					continue;//this child does not intersect with the triangle,do not instantiate it

				_childs[i]=_owner->AllocOcTreeNode();
				OTNodeTri *p;
				p=_owner->OTNodeFromIdx(_childs[i]);
				if (!p)
					continue;
				p->Init(box,_depth+1,_owner);
				_pttnChilds|=(1<<i);

				//pass the triangle to it
				OTNodeTri temp;
				temp=*p;
				if (temp.AddTriangle(vecTriangleIdx[j]))
					bAccepted=true;
				p=_owner->OTNodeFromIdx(_childs[i]);
				*p=temp;
			}
			else
			{
				//this child is already instantiated,check and pass it
				if (_childs[i]==0)
				{
// 					LogLine ln(g_logWorldStruct);
// 					ln<<newln<<"One of the children of an OctTreeNode is not instantiated correctly!";
					continue;
				}
				OTNodeTri *p;
				p=_owner->OTNodeFromIdx(_childs[i]);
				if (!p)
				{
// 					LogLine ln(g_logWorldStruct);
// 					ln<<newln<<"failed to get pointer of an OctTreeNode's child!";
					continue;
				}

				i_math::aabbox3df aabb;
				aabb=p->GetAABB();
				aabb.inflate(0.005f,0.005f,0.005f);//inflate a little to avoid some edge effects
				if (tri.intersectsWithAABB(aabb))
				{
					OTNodeTri temp;
					temp=*p;
					if (temp.AddTriangle(vecTriangleIdx[j]))
						bAccepted=true;
					p=_owner->OTNodeFromIdx(_childs[i]);
					*p=temp;
				}
			}
		}
		if (bAccepted)
			nAccepted++;
	}

	return nAccepted;
}



bool OTNodeTri::AddTriangle(unsigned short idxTriangle)
{
	if (!_owner)
		return false;
	i_math::triangle3df tri;
	if (!_owner->TriangleFromIndex(tri,idxTriangle))
		return false;

	if (_pttnChilds!=0)//Has children
		return (_DispatchTris(&idxTriangle,1)==1);
	else
	{
		if ((_indiceTri.size()<_owner->GetMaxTrianglePerNode())||(_depth>=_owner->GetNodeDepth()))//Still has capacity or too deep a node
		{
			_indiceTri.push_back(idxTriangle);//accept myself
			return true;
		}
		else
		{
			_indiceTri.push_back(idxTriangle);//first accept it
			bool bRet;
			int nAccepted;
			nAccepted=_DispatchTris(_indiceTri.data(),_indiceTri.size());
			bRet=(nAccepted==_indiceTri.size());

			//Clean all my triangles(they belong to my children now)
			_indiceTri.clear();

			return bRet;
		}
	}
}


//bool OctTreeNode::AddTriangle2(int idxTriangle)
//{
//	if (!m_pOwner)
//		return false;
//	i_math::triangle3df *pTri;
//	pTri=m_pOwner->TriangleFromIndex(idxTriangle);
//	if (!pTri)
//		return false;
//
//	if (m_ChildrenPattern!=0)//Has children
//		return (_DispatchTris(&idxTriangle,1)==1);
//	else
//	{
//		if ((m_vecTriangleIdx.size()<g_MAX_TRIANGLE_PER_NODE)||(m_depth>=g_MAX_NODE_DEPTH))//Still has capacity or too deep a node
//		{
//			m_vecTriangleIdx.push_back(idxTriangle);//accept myself
//			return true;
//		}
//		else
//		{
//			m_vecTriangleIdx.push_back(idxTriangle);//first accept it
//			bool bRet;
//			int nAccepted;
//			nAccepted=_DispatchTris(m_vecTriangleIdx.data(),m_vecTriangleIdx.size());
//			bRet=(nAccepted==m_vecTriangleIdx.size());
//
//			//Clean all my triangles(they belong to my children now)
//			m_vecTriangleIdx.clear();
//
//			return bRet;
//		}
//	}
//}


void OTNodeTri::GetTriangleByAABB(i_math::aabbox3df &aabb,std::vector<WORD>&tris)
{
	if (!_aabb.intersectsWithBox(aabb))
		return;
	if (!_owner)
		return;
	if (_pttnChilds!=0)//Has children
	{
		OTNodeTri *p;
		int i;
		for (i=0;i<8;i++)
		{
			if (!(_pttnChilds&(1<<i)))
				continue;
			p=_owner->OTNodeFromIdx(_childs[i]);
			if (p)
				p->GetTriangleByAABB(aabb,tris);
		}
		return;
	}
	int i,n;
	n=_indiceTri.size();
	for (i=0;i<n;i++)
	{
		unsigned short idx;
		idx=_indiceTri[i];
		int j;
		for (j=0;j<tris.size();j++)
		{
			if (tris[j]==idx)
				break;
		}
		if (j>=tris.size())
			tris.push_back(idx);
	}
}

BOOL OTNodeTri::IsLeaf()
{
	if (_pttnChilds!=0)//Has children
		return FALSE;
	return TRUE;
}


//Get all the descendent nodes that intersects with the given line 
void OTNodeTri::GetLeafNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize)
{
	if (!_owner)
		return;
	OTNodeTri *p;
	if (_pttnChilds!=0)//Has children
	{
		int i;
		for (i=0;i<8;i++)
		{
			if (!(_pttnChilds&(1<<i)))
				continue;
			if (nNodes>=nBufferSize)
				break;
			p=_owner->OTNodeFromIdx(_childs[i]);
			if (p)
			{
				if (p->GetAABB().intersectsWithLine(line))
				{
					if (p->IsLeaf())
					{
						pNodeIdx[nNodes]=_childs[i];
						nNodes++;
					}
					else
						p->GetLeafNodeByLine(line,pNodeIdx,nNodes,nBufferSize);
				}
			}
		}
	}
}

void OTNodeTri::GetTriangleByLine(i_math::line3df &line,std::vector<WORD>&tris)
{
	if (!_owner)
		return;
	OTNodeTri *p;
	if (_pttnChilds!=0)//Has children
	{
		int i;
		for (i=0;i<8;i++)
		{
			if (!(_pttnChilds&(1<<i)))
				continue;
			p=_owner->OTNodeFromIdx(_childs[i]);
			if (p)
			{
				if (p->GetAABB().intersectsWithLine(line))
				{
					if (p->IsLeaf())
					{
						int i,n;
						n=p->_indiceTri.size();
						for (i=0;i<n;i++)
						{
							unsigned short idx;
							idx=p->_indiceTri[i];
							int j;
							for (j=0;j<tris.size();j++)
							{
								if (tris[j]==idx)
									break;
							}
							if (j>=tris.size())
								tris.push_back(idx);
						}
					}
					else
						p->GetTriangleByLine(line,tris);
				}
			}
		}
	}
}

//get all the nodes(leaf or not leaf)
void OTNodeTri::GetNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize)
{
	if (!_owner)
		return;
	OTNodeTri *p;
	if (_pttnChilds!=0)//Has children
	{
		int i;
		for (i=0;i<8;i++)
		{
			if (!(_pttnChilds&(1<<i)))
				continue;
			if (nNodes>=nBufferSize)
				break;
			p=_owner->OTNodeFromIdx(_childs[i]);
			if (p)
			{
				if (p->GetAABB().intersectsWithLine(line))
				{
					pNodeIdx[nNodes]=_childs[i];
					nNodes++;
					if (!p->IsLeaf())
					{
						p->GetNodeByLine(line,pNodeIdx,nNodes,nBufferSize);
					}
				}
			}
		}
	}
}




//////////////////////////////////////////////////////////////////////////
//OcTreeTri
void OcTreeTri::Init(i_math::aabbox3df*boxes,DWORD nBoxes)
{
	_nodes.push_back(NULL);//push a dummy element to reserve 0 as an invalid index
	_tris.push_back(OcTreeTriInfo());//push a dummy element to reserve 0 as an invalid index

	_nMaxTriPerNode=g_MAX_TRIANGLE_PER_NODE_DEFAULT;
	_nNodeDepth=g_MAX_NODE_DEPTH_DEFAULT;

	int i;
	for (i=0;i<nBoxes;i++)
	{
		unsigned short idx;
		idx=AllocOcTreeNode();
		OTNodeTri *pRoot;
		pRoot=OTNodeFromIdx(idx);
		if (pRoot)
		{
			pRoot->Init(boxes[i],0,this);
			_roots.push_back(idx);
		}
	}

	if (nBoxes>0)
	{
		_aabb.reset(boxes[0]);
		_aabb.addInternalBox(boxes[nBoxes-1]);
	}
	else
		_aabb.reset(0,0,0);

}

void OcTreeTri::Clear()
{
	_nMaxTriPerNode=g_MAX_TRIANGLE_PER_NODE_DEFAULT;
	_nNodeDepth=g_MAX_NODE_DEPTH_DEFAULT;

	for (int i=1;i<_nodes.size();i++)
		Class_Delete(_nodes[i]);

	_nodes.clear();
	_tris.clear();

	_roots.clear();
}

void OcTreeTri::ClearTris()
{
	int nMax=0;
	for (int i=0;i<_roots.size();i++)
	{
		if (_roots[i]>nMax)
			nMax=_roots[i];
		OTNodeTri *node=OTNodeFromIdx(_roots[i]);
		node->ClearTris();
	}

	for (int i=nMax+1;i<_nodes.size();i++)
		Class_Delete(_nodes[i]);
	_nodes.resize(nMax+1);
	_tris.resize(1);
}


#define CURRENT_VER 1

void OcTreeTri::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=CURRENT_VER;
	DP_WriteVar(dp,_aabb);
	DP_WriteVectorN(dp,_vertices);
	DP_WriteVectorN(dp,_tris);
	DP_WriteVectorN(dp,_roots);
	dp.Data_NextDword()=_nodes.size();
	for (int i=1;i<_nodes.size();i++)
		_nodes[i]->Save(dp);

	DP_WriteVar(dp,_nMaxTriPerNode);
	DP_WriteVar(dp,_nNodeDepth);

}

void OcTreeTri::Load(CDataPacket &dp)
{
	Clear();
	DWORD	ver=dp.Data_NextDword();
	DP_ReadVar(dp,_aabb);
	DP_ReadVectorN(dp,_vertices);
	DP_ReadVectorN(dp,_tris);
	DP_ReadVectorN(dp,_roots);
	DWORD nNodes=dp.Data_NextDword();

	_nodes.resize(nNodes);
	_nodes[0]=NULL;

	for (int i=1;i<nNodes;i++)
	{
		OTNodeTri *node;
		node=_nodes[i]=Class_New2(OTNodeTri);
		node->Load(dp);
		node->_owner=this;
	}

	DP_ReadVar(dp,_nMaxTriPerNode);
	DP_ReadVar(dp,_nNodeDepth);

}


bool OcTreeTri::IsEmpty()
{
	if (_nodes.size()>1)
		return false;

	if (_tris.size()>1)
		return false;

	if (_roots.size()>0)
		return false;

	return true;
}


unsigned short OcTreeTri::AllocOcTreeNode()
{
	if (_nodes.size()>=MAX_NODE_PER_TREE)
		return 0;

	unsigned short idx;
	idx=_nodes.size();
	OTNodeTri *node=Class_New2(OTNodeTri);
	_nodes.push_back(node);

	return idx;
}

OTNodeTri* OcTreeTri::OTNodeFromIdx(unsigned short idxNode)
{
	if (idxNode<1)
		return 0;
	if (idxNode>=_nodes.size())
		return 0;
	return _nodes[idxNode];
}

unsigned short OcTreeTri::AllocTriangle(OcTreeTriInfo&info)
{
	if (_tris.size()>=MAX_TRIANGLE_PER_TREE)
		return 0;

	unsigned short idx;
	idx=_tris.size();
	_tris.push_back(info);

	return idx;
}

bool OcTreeTri::TriangleFromIndex(i_math::triangle3df &tri,unsigned short idxTriangle)
{
	if (idxTriangle<1)
		return false;
	if (idxTriangle>=_tris.size())
		return false;
	tri.pointA=_vertices[_tris[idxTriangle].tris[0]];
	tri.pointB=_vertices[_tris[idxTriangle].tris[1]];
	tri.pointC=_vertices[_tris[idxTriangle].tris[2]];
	return true;
}

unsigned long OcTreeTri::TriFlagFromIndex(unsigned short idxTriangle)
{
	if (idxTriangle<1)
		return 0;
	if (idxTriangle>=_tris.size())
		return 0;
	return _tris[idxTriangle].flag;
}



bool OcTreeTri::AddTriangles(i_math::vector3df *vertices0,DWORD nVertices,WORD *indices0,DWORD nIndices,BYTE flag)
{
	WORD nBase=_vertices.size();

	static std::vector<i_math::vector3df>vertices;
	static std::vector<WORD>indices;
	if (TRUE)
	{
		//先过滤掉不在_aabb范围内的三角形索引
		assert(nIndices%3==0);
		indices.clear();
		for (int i=0;i<nIndices;i+=3)
		{
			i_math::triangle3df tri;
			tri.pointA=vertices0[indices0[i]];
			tri.pointB=vertices0[indices0[i+1]];
			tri.pointC=vertices0[indices0[i+2]];

			if (!tri.intersectsWithAABB(_aabb))
				continue;

			indices.push_back(indices0[i]);
			indices.push_back(indices0[i+1]);
			indices.push_back(indices0[i+2]);
		}

		//过滤掉没有被引用到的顶点,并记录下新的索引值
		std::vector<WORD>remap;
		remap.resize(nVertices);
		VEC_SET(remap,0xff);
		if (TRUE)
		{
			std::vector<BYTE>used;
			used.resize(nVertices);
			VEC_SET(used,0);

			for (int i=0;i<indices.size();i++)
				used[indices[i]]=1;

			vertices.resize(nVertices);

			WORD c=0;
			for (int i=0;i<nVertices;i++)
			{
				if (used[i])
				{
					vertices[c]=vertices0[i];
					remap[i]=c;
					c++;
				}
			}

			vertices.resize(c);
		}

		//根据remap调整索引值
		for (int i=0;i<indices.size();i++)
			indices[i]=remap[indices[i]]+nBase;
	}

	if (vertices.size()+nBase>64000)
		return false;

	VEC_APPEND(_vertices,vertices);

	bool bAccepted=false;

	for (int i=0;i<indices.size();i+=3)
	{

		//add the triangle 
		unsigned short idxTriangle;
		OcTreeTriInfo info;
		info.tris[0]=indices[i];
		info.tris[1]=indices[i+1];
		info.tris[2]=indices[i+2];
		info.flag=flag;

		idxTriangle=AllocTriangle(info);

		i_math::triangle3df tri;
		TriangleFromIndex(tri,idxTriangle);


		for (int i=0;i<_roots.size();i++)
		{
			OTNodeTri *pRoot;
			pRoot=OTNodeFromIdx(_roots[i]);
			if (pRoot)
			{
				if (tri.intersectsWithAABB(pRoot->GetAABB()))
				{
					OTNodeTri temp;
					temp=*pRoot;
					if (temp.AddTriangle(idxTriangle))
						bAccepted=true;
					pRoot=OTNodeFromIdx(_roots[i]);
					*pRoot=temp;
				}
			}
		}
	}

//	if (!bAccepted)
//	{
//		int v;
//		v=0;
//	}
	return bAccepted;
}

//get all the triangles whose bounding box intersects with the given aabb
void OcTreeTri::GetTriangleByAABB(i_math::aabbox3df &aabb,std::vector<i_math::vector3df>&tris,std::vector<BYTE>&flags)
{
	static std::vector<WORD>indices;
	indices.clear();
	int i,n;
	n=_roots.size();
	for (i=0;i<n;i++)
	{
		OTNodeTri *p;
		p=OTNodeFromIdx(_roots[i]);
		if (p)
			p->GetTriangleByAABB(aabb,indices);
	}

	for (i=0;i<indices.size();i++)
	{
		i_math::triangle3df tri;
		TriangleFromIndex(tri,indices[i]);
		i_math::aabbox3df aabbTri;
		if (TRUE)//Ignore those triangles that does not intersect with the aabb
		{
			aabbTri.reset(tri.pointA);
			aabbTri.addInternalPoint(tri.pointB);
			aabbTri.addInternalPoint(tri.pointC);

			if (!aabb.intersectsWithBox(aabbTri))
				continue;
		}

		tris.resize(tris.size()+3);
		memcpy(&tris[tris.size()-3],&tri,sizeof(i_math::triangle3df));
		flags.push_back((BYTE)TriFlagFromIndex(indices[i]));
	}
}

//get all the leaf nodes that intersects with the line
void OcTreeTri::GetLeafNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize)
{
	int i,n;
	n=_roots.size();
	for (i=0;i<n;i++)
	{
		OTNodeTri *p;
		p=OTNodeFromIdx(_roots[i]);
		if (p)
		{
			p->GetLeafNodeByLine(line,pNodeIdx,nNodes,nBufferSize);
			if (nNodes>=nBufferSize)
				break;
		}
	}
}

//get all the nodes that intersects with the line(leaf or not leaf)
void OcTreeTri::GetNodeByLine(i_math::line3df &line,unsigned short *pNodeIdx,int &nNodes,int nBufferSize)
{
	int i,n;
	n=_roots.size();
	for (i=0;i<n;i++)
	{
		OTNodeTri *p;
		p=OTNodeFromIdx(_roots[i]);
		if (p)
		{
			if (nNodes>=nBufferSize)
				break;
			pNodeIdx[nNodes]=_roots[i];
			nNodes++;
			p->GetNodeByLine(line,pNodeIdx,nNodes,nBufferSize);
		}
	}
}


//Get all the triangles whose bounding box intersects with the line
void OcTreeTri::GetTriangleByLine(i_math::line3df &line,std::vector<i_math::vector3df>&tris,std::vector<BYTE>*flags)
{
	static std::vector<WORD>indices;
	indices.clear();
	int i,n;
	n=_roots.size();
	for (i=0;i<n;i++)
	{
		OTNodeTri *p;
		p=OTNodeFromIdx(_roots[i]);
		if (p)
			p->GetTriangleByLine(line,indices);
	}

	for (i=0;i<indices.size();i++)
	{
		i_math::triangle3df tri;
		TriangleFromIndex(tri,indices[i]);
		i_math::aabbox3df aabbTri;
		if (TRUE)//Ignore those triangles whose bb does not intersect with the line
		{
			aabbTri.reset(tri.pointA);
			aabbTri.addInternalPoint(tri.pointB);
			aabbTri.addInternalPoint(tri.pointC);

			aabbTri.inflate(0.005f,0.005f,0.005f);//Inflate a little to avoid edging effect

			if (!aabbTri.intersectsWithLine(line))
				continue;
		}

		tris.resize(tris.size()+3);
		memcpy(&tris[tris.size()-3],&tri,sizeof(i_math::triangle3df));
		if (flags)
			flags->push_back((BYTE)TriFlagFromIndex(indices[i]));
	}
}

