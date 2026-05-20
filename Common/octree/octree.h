#pragma  once

#include <vector>
#include "../math/aabbox3d.h"
#include "../math/triangle3d.h"
#include "../math/line3d.h"

#include "../mempool/mempool.h"

#include "../datapacket/DataPacket.h"



//TOwner should implement the following functions
//CMemPool<OTNode::OTNodeChilds> *GetMemPool_OTNode();//Get a mempool to manage OTNodes
//CMemPoolEx *GetMemPool_PrimHandle();//Get a mempool to manage PrimHandle
//void RegisterOTNode(TPrimHandle ph,OTNode *n,i_math::aabbox3df &aabb);//link a OTNode in prim
//void UnRegisterOTNode(TPrimHandle ph,OTNode *n);//Unlink a OTNode in prim
//BOOL TouchAABB(TPrimHandle ph,i_math::aabbox3df &aabb);//chech whether a prim is touching an aabb
//void SetEnumFlag(TPrimHandle ph,BOOL bEnum),mark/unmark the prim as enumerated
//BOOL CheckEnumFlag(TPrimHandle ph),check whether the prim is enumerated
//BOOL PrimHitTest(TPrimHandle ph,OTNode::LineInfo &li),check whether a prim is hit by a line


template <class TOwner,class TPrimHandle,int MaxNodePrim,int MaxDepth>
class OTNode
{
public:
	typedef OTNode<TOwner,TPrimHandle,MaxNodePrim,MaxDepth> ThisType;

	struct OTNodeChilds
	{
		ThisType childs[8];
	};

	struct LineInfo
	{
		vector3df middle;
		vector3df vect;
		float halflength;
		DWORD browse;//use which browsing order(based on the line's direction)
	};


	OTNode()
	{
		Zero();
	}

	void Zero()
	{
		_childs=NULL;
		_prims=NULL;
		_nPrims=0;
	}

	BOOL AddPrim(TOwner *owner,TPrimHandle ph,i_math::aabbox3df &aabbThis,DWORD depth)
	{
		if (!owner->TouchAABB(ph,aabbThis))
			return FALSE;
		if (!IsLeaf())//Has children
			return (_DispatchDown(owner,&ph,1,aabbThis,depth)==1);
		else
		{
			CMemPoolEx *mempool=owner->GetMemPool_PrimHandle();
			if ((_nPrims<MaxNodePrim)||(depth>=MaxDepth))//Still has capacity or too deep a node
			{
				owner->RegisterOTNode(ph,this,aabbThis);//link me in the prim
				return _AddPrim(ph,mempool);
			}
			else
			{
				TPrimHandle buf[MaxNodePrim+5];//Big enough

				//temply accept it
				if (TRUE)
				{
					TPrimHandle *p=(TPrimHandle *)mempool->ObtainPtr(_prims);
					memcpy(buf,p,sizeof(TPrimHandle)*_nPrims);
					buf[_nPrims++]=ph;
				}

				//first remove the link to me from my prims
				for (int i=0;i<_nPrims-1;i++)
					owner->UnRegisterOTNode(buf[i],this);


				bool bRet;
				int nAccepted;
				nAccepted=_DispatchDown(owner,buf,_nPrims,aabbThis,depth);
				bRet=(nAccepted==_nPrims);

				//Clean all my prims(they belong to my children now)
				mempool->Free(_prims);
				_prims=NULL;
				_nPrims=0;

				return bRet;
			}
		}
	}

	void EnumPrimByAABB(TOwner *owner,i_math::aabbox3df &aabbThis,
				i_math::aabbox3df &aabb,std::vector<TPrimHandle>&result)
	{
		result.clear();
		if (!aabbThis.intersectsWithBox(aabb))
			return;
		_EnumPrimByAABB(owner,aabbThis,aabb,result);

		for (int i=0;i<result.size();i++)
			owner->SetEnumFlag(result[i],FALSE);//Unmark the flag
  
	}

	void EnumPrimByLine(TOwner *owner,i_math::aabbox3df &aabbThis,
					i_math::line3df &line,std::vector<TPrimHandle>&result)
	{
		result.clear();
		LineInfo li;

		li.middle=line.getMiddle();
		li.vect=line.getVector().normalize();
		li.halflength=((float)line.getLength())/2.0f;

		if (!aabbThis.intersectsWithLine(li.middle,li.vect,li.halflength))
			return;

		li.browse=0;
		if (line.end.x>line.start.x)
			li.browse+=XMax;
		if (line.end.y>line.start.y)
			li.browse+=YMax;
		if (line.end.z>line.start.z)
			li.browse+=ZMax;

		_EnumPrimByLine(owner,aabbThis,li,result);

		for (int i=0;i<result.size();i++)
			owner->SetEnumFlag(result[i],FALSE);//Unmark the flag
	}

	BOOL LineHitTest(TOwner *owner,i_math::aabbox3df &aabbThis,i_math::line3df &line)
	{
		LineInfo li;

		li.middle=line.getMiddle();
		li.vect=line.getVector().normalize();
		li.halflength=((float)line.getLength())/2.0f;

		if (!aabbThis.intersectsWithLine(li.middle,li.vect,li.halflength))
			return FALSE;
		li.browse=0;
		if (line.end.x>line.start.x)
			li.browse+=XMax;
		if (line.end.y>line.start.y)
			li.browse+=YMax;
		if (line.end.z>line.start.z)
			li.browse+=ZMax;

		return _LineHitTest(owner,aabbThis,li);
	}

	BOOL IsLeaf()	{		return _childs==NULL;	}
	BOOL IsEmpty()	{		return (_childs==NULL)&&(_nPrims<=0);	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextDword()=(DWORD)_prims;
		dp.Data_NextDword()=_nPrims;
		dp.Data_NextDword()=(DWORD)_childs;
	}
	void Load(CDataPacket &dp)
	{
		_prims=(MemHandle)dp.Data_NextDword();
		_nPrims=dp.Data_NextDword();
		_childs=(MemHandle)dp.Data_NextDword();
	}

protected:

	enum _ChildIdx
	{
		XMax=0x1,
		YMax=0x2,
		ZMax=0x4,
	};

	DWORD _FindChilds(i_math::aabbox3df &aabbThis,i_math::aabbox3df &aabbTest,DWORD *childs)
	{
		i_math::vector3df middle=aabb.getCenter();

		DWORD c=0;

		if(aabbTest.MaxEdge.x>middle.x) // XMAX
		{ 
			if(aabbTest.MaxEdge.y>middle.y) // YMAX
			{
				if(aabbTest.MaxEdge.z>middle.z) // ZMAX
					childs[c++] = XMax+YMax+ZMax;
				if(aabbTest.MinEdge.z<=middle.z) // ZMIN
					childs[c++] = XMax+YMax;
			}

			if(aabbTest.MinEdge.y<=middle.y) // YMIN
			{
				if(aabbTest.MaxEdge.z > middle.z) // ZMAX
					childs[c++] = XMax+ZMax;
				if(aabbTest.MinEdge.z <= middle.z) // ZMIN
					childs[c++] = XMax ;
			}
		}

		if(aabbTest.MinEdge.x <= middle.x) // XMIN
		{ 
			if(aabbTest.MaxEdge.y > middle.y) // YMAX
			{
				if(aabbTest.MaxEdge.z > middle.z) // ZMAX
					childs[c++] =YMax+ZMax;
				if(aabbTest.MinEdge.z <= middle.z) // ZMIN
					childs[c++] =YMax;	
			}

			if(aabbTest.MinEdge.Y <= middle.Y) // YMIN
			{
				if(aabbTest.MaxEdge.Z > middle.Z) // ZMAX
					childs[c++] =ZMax;
				if(aabbTest.MinEdge.Z <= middle.Z) // ZMIN
					childs[c++] =0;
			}
		}

		return c;

	}

	void _CalcChildAABB(i_math::aabbox3df &aabb,i_math::vector3df &middle,
					i_math::aabbox3df &aabbChild,DWORD idxChild)
	{
		if (idxChild&XMax)
		{
			aabbChild.MaxEdge.x=aabb.MaxEdge.x;
			aabbChild.MinEdge.x=middle.x;
		}
		else
		{
			aabbChild.MinEdge.x=aabb.MinEdge.x;
			aabbChild.MaxEdge.x=middle.x;
		}

		if (idxChild&YMax)
		{
			aabbChild.MaxEdge.y=aabb.MaxEdge.y;
			aabbChild.MinEdge.y=middle.y;
		}
		else
		{
			aabbChild.MinEdge.y=aabb.MinEdge.y;
			aabbChild.MaxEdge.y=middle.y;
		}

		if (idxChild&ZMax)
		{
			aabbChild.MaxEdge.z=aabb.MaxEdge.z;
			aabbChild.MinEdge.z=middle.z;
		}
		else
		{
			aabbChild.MinEdge.z=aabb.MinEdge.z;
			aabbChild.MaxEdge.z=middle.z;
		}
	}

	void _EnumPrim(TOwner *owner,std::vector<TPrimHandle>&result)
	{
		CMemPoolEx *mempool=owner->GetMemPool_PrimHandle();
		TPrimHandle *p=(TPrimHandle *)mempool->ObtainPtr(_prims);
		for (int i=0;i<_nPrims;i++)
		{
			if (owner->CheckEnumFlag(p[i]))
				continue;
			owner->SetEnumFlag(p[i],TRUE);//mark it
			result.push_back(p[i]);
		}
	}

	void _EnumPrimByAABB(TOwner *owner,i_math::aabbox3df &aabbThis,i_math::aabbox3df &aabb,std::vector<TPrimHandle>&result)
	{
		if (!IsLeaf())
		{
			DWORD childs[8];
			DWORD c=_FindChilds(aabbThis,aabb,childs);
			if (c>0)
			{
				i_math::vector3df middle=aabbThis.getCenter();
				ThisType *childptr=owner->GetMemPool_OTNode()->ObtainPtr(_childs);
				for (int i=0;i<c;i++)
				{
					i_math::aabbox3df aabbChild;
					_CalcChildAABB(aabbThis,middle,aabbChild,childs[i]);
					childptr[childs[i]]._EnumPrimByAABB(owner,aabbChild,aabb,result);
				}
			}
		}
		else
			_EnumPrim(result);
	}

	void _EnumPrimByLine(TOwner *owner,i_math::aabbox3df &aabbThis,
								LineInfo &li,std::vector<TPrimHandle>&result)
	{

		//			6			7			|
		//2			3					|		y
		//			4			5			|				z
		//0			1					|		o			x

		if (!IsLeaf())
		{
			static DWORD orders[8][8]=
			{
				{7,6,3,5,2,4,1,0},//0
				{6,2,7,4,3,0,5,1},//1
				{5,7,1,4,3,6,0,2},//2
				{4,0,6,5,2,7,1,3},//3
				{3,2,7,1,0,6,5,4},//4
				{2,3,0,6,1,4,7,5},//5
				{1,0,3,5,2,4,7,6},//6
				{0,2,4,1,6,3,5,7},//7
			};

			DWORD *order=&orders[li.browse][0];

			i_math::vector3df middle=aabbThis.getCenter();
			i_math::aabbox3df aabbChild;
			ThisType *childptr=(ThisType *)owner->GetMemPool_OTNode()->ObtainPtr(_childs);
			for (int i=0;i<8;i++)
			{
				if (childptr[order[i]].IsEmpty())
					continue;
				_CalcChildAABB(aabbThis,middle,aabbChild,order[i]);
				if (aabbChild.intersectsWithLine(li.middle,li.vect,li.halflength))
					childptr[order[i]]._EnumPrimByLine(owner,aabbChild,li,result);
			}
		}
		else
			_EnumPrim(owner,result);
	}

	BOOL _LineHitTest(TOwner *owner,i_math::aabbox3df &aabbThis,LineInfo &li)
	{

		//			6			7			|
		//2			3					|		y
		//			4			5			|				z
		//0			1					|		o			x

		if (!IsLeaf())
		{
			static DWORD orders[8][8]=
			{
				{7,6,3,5,2,4,1,0},//0
				{6,2,7,4,3,0,5,1},//1
				{5,7,1,4,3,6,0,2},//2
				{4,0,6,5,2,7,1,3},//3
				{3,2,7,1,0,6,5,4},//4
				{2,3,0,6,1,4,7,5},//5
				{1,0,3,5,2,4,7,6},//6
				{0,2,4,1,6,3,5,7},//7
			};

			DWORD *order=&orders[li.browse][0];

			i_math::vector3df middle=aabbThis.getCenter();
			i_math::aabbox3df aabbChild;
			ThisType *childptr=(ThisType *)owner->GetMemPool_OTNode()->ObtainPtr(_childs);
			for (int i=0;i<8;i++)
			{
				if (childptr[order[i]].IsEmpty())
					continue;
				_CalcChildAABB(aabbThis,middle,aabbChild,order[i]);
				if (aabbChild.intersectsWithLine(li.middle,li.vect,li.halflength))
				{
					if (childptr[order[i]]._LineHitTest(owner,aabbChild,li))
						return TRUE;
				}
			}
		}
		else
		{
			CMemPoolEx *mempool=owner->GetMemPool_PrimHandle();
			TPrimHandle *p=(TPrimHandle *)mempool->ObtainPtr(_prims);
			for (int i=0;i<_nPrims;i++)
			{
				if (owner->PrimHitTest(p[i],li))
					return TRUE;
			}
		}
		return FALSE;
	}


	//return the prim count that is accepted by these children
	//aabbThis is the bb of this node
	DWORD _DispatchDown(TOwner *owner,TPrimHandle *phs,DWORD nPrims,i_math::aabbox3df &aabbThis,DWORD depth)
	{
		ThisType childsT[8];
		if(!_childs)
			_childs=owner->GetMemPool_OTNode()->Alloc();//Not split yet,split me first
		else
		{
			ThisType *childptr=(ThisType *)owner->GetMemPool_OTNode()->ObtainPtr(_childs);
			memcpy(childsT,childptr,sizeof(childsT));
		}

		i_math::aabbox3df aabbChilds[8];
		if (TRUE)
		{
			i_math::vector3df middle=aabbThis.getCenter();
			for (int i=0;i<8;i++)
				_CalcChildAABB(aabbThis,middle,aabbChilds[i],(DWORD)i);
		}
		int nAccepted=0;
		for (int j=0;j<nPrims;j++)
		{
			BOOL bAccepted=FALSE;
			for (int i=0;i<8;i++)
			{
				if (childsT[i].AddPrim(owner,phs[j],aabbChilds[i],depth+1))
					bAccepted=TRUE;
			}
			if (bAccepted)
				nAccepted++;
		}

		ThisType *childptr=(ThisType *)owner->GetMemPool_OTNode()->ObtainPtr(_childs);
		memcpy(childptr,childsT,sizeof(childsT));

		return nAccepted;
	}


	BOOL _AddPrim(TPrimHandle ph,CMemPoolEx *mempool)
	{
		if (_nPrims>0)
		{
			if (FALSE==mempool->ReAlloc(_prims,(_nPrims+1)*sizeof(TPrimHandle)))//increase by 1
				return FALSE;
		}
		else
		{
			if (!(_prims=mempool->Alloc(sizeof(TPrimHandle))))
				return FALSE;
		}
		TPrimHandle *p=(TPrimHandle *)mempool->ObtainPtr(_prims);
		p[_nPrims]=ph;
		_nPrims++;

		return TRUE;
	}

	MemHandle _prims;
	DWORD _nPrims;
	MemHandle _childs;//NULL or pointer to a 8 nodes buffer
};

