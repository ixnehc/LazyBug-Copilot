
#pragma once

#include "math/range.h"
#include "math/rect.h"
#include "math/pos2d.h"
#include "math/volume.h"


class CQuadTree
{
	struct _Node
	{
		_Node()
		{
			flag=0;
			parent=NULL;
			memset(childs,0,sizeof(childs));
		}
		WORD flag;// 1: leaf,
		WORD depth;//the depth in the quadtree,the root node has a depth of 0
		_Node *parent;
		_Node *childs[4];
		i_math::recti rc;//the rect of the blocks this node occupies.the rect is relative to the left-up corner of the _rcMap
		i_math::rangef vr;
	};

public:
	CQuadTree()
	{
		Zero();
	}
	void Zero()
	{
		_rcMap.set(0,0,0,0);
		_leafstart=0xffffffff;
		_depth=-1;
		_blocklen=4;
	}

	BOOL Enum(i_math::volumeCvxf &vol);
	i_math::pos2di *GetEnum(DWORD &count);//返回枚举到的node的坐标

	BOOL Init(int w,int h,float nodelen);//w/h are in node
	void Clear();
	BOOL Locate(i_math::pos2di &ptCenter);//Set the map center,单位是node,世界坐标


protected:
	virtual void _CalcVR(int x,int y,i_math::rangef &vr)	{	}

	_Node* _BuildQTreeNode(std::vector<_Node>&tree,
				i_math::recti &rc,i_math::recti &rcMap,DWORD iNode,DWORD depth);
	void _BuildQTree(int w,int h);

	void _UpdateQTree();

	void _EnumNode(i_math::volumeCvxf &vol,_Node *node);

	std::vector<_Node> _qtree;//quad tree,the 1st element of this vector is the root node
	DWORD _leafstart;//the index to the first leaf node in _qtree
	int _depth;//max depth of the quad tree

	i_math::recti _rcMap;//in node,of world coordinate

	float _blocklen;

	std::vector<i_math::pos2di> _enum;

};


#pragma warning(default:4312)

