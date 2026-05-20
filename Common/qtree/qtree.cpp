/********************************************************************
	created:	2008/07/30   17:28
	filename: 	D:\IxEngine\Common\qtree\qtree.cpp
	author:		cxi
	
	purpose:	quad tree
*********************************************************************/

#include "stdh.h"
#include "qtree.h"

#include "timer/profiler.h"

#include "rasterize/rasterize.h"
#include "../commondefines/general_stl.h"

#include <assert.h>

#pragma warning(disable:4312)
#pragma warning(disable:4311)





//////////////////////////////////////////////////////////////////////////
//CQuadTree

//w,h are in meter
BOOL CQuadTree::Init(int w,int h,float blocklen)
{

	_BuildQTree(w,h);

	_blocklen=blocklen;

	_rcMap.set(-100000,-100000,-100000+w,-100000+h);

	return TRUE;
}

void CQuadTree::Clear()
{

	_qtree.clear();

	_enum.clear();

	Zero();
}



CQuadTree::_Node* CQuadTree::_BuildQTreeNode(std::vector<CQuadTree::_Node>&tree,
												 i_math::recti &rc,i_math::recti &rcMap,DWORD iNode,DWORD depth)
{
	assert(iNode<tree.size());
	_Node *node=&tree[iNode];

	node->rc=rc;
	node->rc.clipAgainst(rcMap);
	node->rc-=rcMap.UpperLeftCorner;
	node->depth=(WORD)depth;

	//make children
	if (rc.getWidth()>1)
	{//still can be split
		DWORD iChild=iNode*4;
		i_math::recti rcChild[4];

		//childs' order
		//		z
		//		^
		//		|			2	3
		//		|			0	1
		//		|
		//		o----------------> x

		i_math::pos2di pt=rc.getCenter();
		rcChild[0].set(rc.Left(),rc.Top(),pt.x,pt.y);
		rcChild[1].set(pt.x,rc.Top(),rc.Right(),pt.y);
		rcChild[2].set(rc.Left(),pt.y,pt.x,rc.Bottom());
		rcChild[3].set(pt.x,pt.y,rc.Right(),rc.Bottom());

		for (int i=0;i<4;i++)
			node->childs[i]=_BuildQTreeNode(tree,rcChild[i],rcMap,iChild+i+1,depth+1);
	}

	//make parent
	if (iNode==0)
		node->parent=NULL;
	else
		node->parent=&tree[(iNode-1)/4];

	//the vertical range
	node->vr.zero();

	//the flag
	node->flag=0;
	if (!node->rc.isValid())
	{
		node->flag|=2;//out of range,mark as empty node
		return NULL;
	}
	else
	{//in the range,
		if (rc.getWidth()==1)//contains only 1 block
			node->flag|=1;//mark as leaf
	}

	return node;
}


void CQuadTree::_BuildQTree(int w,int h)
{
	_qtree.clear();

	int len=max(w,h);


	_depth=i_math::fastmaxlog2(len);
	//adjust to 2-powered
	len=1<<_depth;

	//Calculate total count of the nodes for the full tree
	std::vector<_Node> tree;
	if (TRUE)
	{
		DWORD len2=len*len;
		DWORD c=0;
		while(len2>0)
		{
			c+=len2;
			len2/=4;
		}

		tree.resize(c);
	}


	i_math::recti rc;
	rc.set(-len/2,-len/2,len/2,len/2);
	i_math::recti rcMap;
	rcMap.set(-((int)w)/2,-((int)h)/2,
		((int)w)/2,((int)h)/2);

	_BuildQTreeNode(tree,rc,rcMap,0,0);//first build a full tree,0 is the root node

	//make the squeezing re-map indices
	std::vector<DWORD>indices;
	DWORD c=0;
	if (TRUE)
	{
		indices.resize(tree.size());
		VEC_SET(indices,0xff);//all 0xffffffff

		for (int i=0;i<tree.size();i++)
		{
			if (tree[i].flag&2)//this node is empty
				continue;
			indices[i]=c;
			c++;
		}
	}

	//squeeze it
	if (TRUE)
	{
		_qtree.resize(c);

		_Node *start=tree.data();
		for (int i=0;i<tree.size();i++)
		{
			if (indices[i]==0xffffffff)
				continue;//ignore the empty nodes
			_Node *node=&tree[i];
			//remap the node-link(parent,children)
			if (node->parent)
				node->parent=&_qtree[indices[node->parent-start]];
			for (int j=0;j<ARRAY_SIZE(node->childs);j++)
			{
				if (node->childs[j])
					node->childs[j]=&_qtree[indices[node->childs[j]-start]];
			}

			_qtree[indices[i]]=*node;
		}
	}

	//find the position of the first leaf node
	_leafstart=0xffffffff;
	for (int i=0;i<_qtree.size();i++)
	{
		if (_qtree[i].flag&1)
		{
			_leafstart=i;
			break;
		}
	}
	DWORD sz=_qtree.size()-rcMap.getArea();
	assert(_leafstart==_qtree.size()-rcMap.getArea());

}

void CQuadTree::_UpdateQTree()
{
	//first update all the leaf node
	DWORD sz=_rcMap.getArea();
	for (int i=_qtree.size()-1;i>=_leafstart;i--)
	{
		_Node *node=&_qtree[i];

		int x,y;
		x=node->rc.Left()+_rcMap.Left();
		y=node->rc.Top()+_rcMap.Top();

		_CalcVR(x,y,node->vr);
	}

	for (int i=_leafstart-1;i>=0;i--)
	{
		_Node *node=&_qtree[i];
		node->vr.zero();
		for (int j=0;j<ARRAY_SIZE(node->childs);j++)
		{
			if (node->childs[j])
				node->vr.merge(node->childs[j]->vr);
		}
	}
}

//If changed,return TRUE and ptCenter will be filled the new center.
//Note that the center is in block
static BOOL UpdateBlockMapCenter(i_math::pos2di &ptCenter,float x,float z,float radius,float blocklen)
{
	float xc,zc;
	xc=(float)(ptCenter.x*blocklen);
	zc=(float)(ptCenter.y*blocklen);

	if ((fabsf(x-xc)>radius)||(fabsf(z-zc)>radius))
	{
		ptCenter.x=FloatToNearestInt(x/blocklen);
		ptCenter.y=FloatToNearestInt(z/blocklen);
		return TRUE;
	}
	return FALSE;
}

//Set the map center,in node
BOOL CQuadTree::Locate(i_math::pos2di &ptCenter)
{
	i_math::pos2di pt=_rcMap.getCenter();
	if (pt==ptCenter)
		return TRUE;
	_rcMap+=ptCenter-pt;
	_UpdateQTree();

	return TRUE;

}

#define ENUMNODE_DEPTH (_depth-1)		//only the nodes that has a depth less-equal than 
//ENUMNODE_DEPTH will be check against the tester
void CQuadTree::_EnumNode(i_math::volumeCvxf &vol,_Node *node)
{
	if (node->vr.isEmpty())
		return;

	//calculate the aabb for the node
	i_math::aabbox3df aabbBlock;
	i_math::recti rcNode;//in block,of world coord
	if (TRUE)
	{
		rcNode=node->rc;
		rcNode+=_rcMap.UpperLeftCorner;
		aabbBlock.MinEdge.set((float)rcNode.Left()*_blocklen,node->vr.low,(float)rcNode.Top()*_blocklen);
		aabbBlock.MaxEdge.set((float)rcNode.Right()*_blocklen,node->vr.hi,(float)rcNode.Bottom()*_blocklen);
	}

	i_math::EIntersectionRelation3D r=vol.classifyAABB(aabbBlock);

	if (r==i_math::ISREL3D_FRONT)
		return;

	if ((r==i_math::ISREL3D_BACK)||((int)node->depth>=ENUMNODE_DEPTH))
	{
		for (int i=rcNode.Left();i<rcNode.Right();i++)
		for (int j=rcNode.Top();j<rcNode.Bottom();j++)
			_enum.push_back(i_math::pos2di(i,j));
		return;
	}

	//test the children
	for (int i=0;i<ARRAY_SIZE(node->childs);i++)
	{
		if (node->childs[i])
			_EnumNode(vol,node->childs[i]);
	}
}

BOOL CQuadTree::Enum(i_math::volumeCvxf &vol)
{
	_enum.clear();
	_EnumNode(vol,_qtree.data());
	return TRUE;
}


i_math::pos2di *CQuadTree::GetEnum(DWORD &count)
{
	count=_enum.size();
	return _enum.data();
}
