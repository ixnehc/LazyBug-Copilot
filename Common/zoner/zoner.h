
#pragma once

#include "../math/range.h"

#include "fastdelegate/FastDelegate.h"

#include "../cyclemap/cyclemap.h"
#include "../class/class.h"
#include "../mempool/mempool.h"

#include "../spatialtester/spatialtester.h"

#include <deque>
#include <map>

struct Zonee;
struct ZoneRgn;

typedef Zonee *ZoneeID;


typedef WORD RgnType;
typedef WORD RgnFlag;
typedef BYTE ZoneeFlag;
typedef BYTE ZoneeType;



#define ZoneeFlag_Dead ((ZoneeFlag)1)
#define ZoneeFlag_Motive ((ZoneeFlag)2)
#define ZoneeFlag_Enum ((ZoneeFlag)4)
#define ZoneeFlag_Global ((ZoneeFlag)8)
#define ZoneeFlag_Enum2 ((ZoneeFlag)16)
#define ZoneeFlag_Enum3 ((ZoneeFlag)32)

#define ZoneeType_None 0
#define ZoneeType_All ((ZoneeType)0xffff)

#define RgnType_None 0
#define RgnType_All 0xffff

#define RgnFlag_Enum ((RgnFlag)1)

enum RgnEvent
{
	ZoneeEntering,
	ZoneeLeaving,
};


//这个call back用来更精确的决定这个zonee是否被线段选中了
//函数原型为:
// BOOL CastRayCallBack(i_math::line3df &line,Zonee *zonee,float &distSQ); 
//其中distSQ为这个zonee的hit点到line.start的距离平方
typedef fastdelegate::FastDelegate3<i_math::line3df&,Zonee*,float&,BOOL> CastRayCallBack;

//这个call back用来接受zonee进入/离开rgn的消息
typedef fastdelegate::FastDelegate3<ZoneRgn*,Zonee *,RgnEvent>RgnEventHandler;
//这个call back用来更精确的测试zonee是否在rgn内,返回zonee是否在rgn内
typedef fastdelegate::FastDelegate2<ZoneRgn*,Zonee*,BOOL> RgnTestCallBack;


enum ZonerEnum
{
	ZEnum_Zonee=1,
	ZEnum_Rgn=2,
};



struct ZoneRgn
{
	ZoneRgn()
	{
		refcount=0;
		data=NULL;
		flag=0;
	}
	void AddRef()		{			refcount++;		}
	void Release()		
	{			
		refcount--;		
	}
	void SetFlag(RgnFlag f)	{		flag|=f;	}
	void ClearFlag(RgnFlag f)	{		flag&=~f;	}
	int refcount;
	void *data;
	RgnType type;
	RgnFlag flag;
	SpacialTester tester;
	i_math::recti rc;//the rect this region affects,in block,of world coord

	std::deque<Zonee*>zonees;//the containee of this region

	DECLARE_CLASS(ZoneRgn);
};

struct ZoneAnchor
{
	DECLARE_CLASS(ZoneAnchor);
	ZoneRgn *rgn;
	ZoneAnchor *next;
};

struct Zonee
{
	DECLARE_CLASS(Zonee);

	BOOL IsAlive()	{		return ((flags&ZoneeFlag_Dead)==0);	}
	BOOL TestFlag(ZoneeFlag f)	{		return (f&flags)!=0;	}
	void SetFlag(ZoneeFlag f)	{		flags|=f;	}
	void ClearFlag(ZoneeFlag f)	{		flags&=~f;	}



	void AddRef()		{			refcount++;		}
	void Release()		
	{			
		refcount--;		
		if (refcount<=0)
			Class_Delete(this);
	}
	int refcount;

	void *owner;
	ZoneeFlag flags;
	ZoneeType type;
	RgnType containable;//可以包含这个zonee的所有Rgn的类型
	ZoneAnchor *anchor;//包含这个Zonee的rgn列表
	i_math::aabbox3df aabb;
};



struct SpacialTester;
class CZoner:public CCycleMap2
{
protected:
	struct _ZoneeQueue
	{
		std::deque<Zonee*>zonees;
		i_math::rangef vr;
		int nDead;
	};

	struct _Block
	{
		_Block()
		{
			Zero();
			zoner=NULL;
		}
		void Zero()
		{
			bVRDirty=FALSE;
			for (int i=0;i<ARRAY_SIZE(zqs);i++)
			{
				zqs[i].vr.zero();
				zqs[i].nDead=0;
				zqs[i].zonees.clear();
			}
			rgns.clear();
			pt.set(-10000,-10000);
		}
		void Clear();
		void Fetch(_Block *blk);//move the content from blk to me(the content in blk will be cleared)
		BOOL IsEmpty();
		void FlushDead(BOOL bFlushAll);//if bFlushAll is TRUE,will remove all the dead,
																//otherwise will only remove the detected
		void IncDead(DWORD idx)		{			zqs[idx].nDead++;		}
		void RecalcVR();
		BOOL CalcAABB(i_math::aabbox3df &aabb);
		_ZoneeQueue zqs[2];//assetqueue,0 for fixed assets,1 for motive assets
		std::vector<ZoneRgn *>rgns;

		BOOL bVRDirty;//indicate whether motive vr need be re-calculated
		i_math::pos2di pt;//the block's position,of world coord
		CZoner *zoner;
	};

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

	struct _EnumCache
	{
		SpacialTester tester;
		std::vector<_Block*>blocks;
	};

public:
	CZoner()
	{
		Zero();
	}
	void Zero()
	{
		_blkmap=NULL;
		_rcMap.set(0,0,0,0);
		_leafstart=0xffffffff;
		_depth=-1;
		_bGlobalDirty=FALSE;
		_handlerRgnEvent=NULL;
		_callbackRgnTest=NULL;

		_lastgc=0;
	}

	void Register(ZoneeType type,RgnType containable);

	Zonee*AddZonee(i_math::aabbox3df &aabb,ZoneeType type,RgnType containable,void *owner,BOOL bMotive);

	Zonee *AddGlobal(ZoneeType type,void *owner);
	void RemoveZonee(Zonee *zonee);
	BOOL Update(Zonee *zonee,i_math::aabbox3df &aabb);
	void EnumZonee(SpacialTester &tester,ZoneeType types)	
	{		
		_Enum(&tester,1,ZEnum_Zonee,(DWORD)types);	
	}
	void EnumZonee(SpacialTester *testers,DWORD nTesters,ZoneeType types)	
	{		
		_Enum(testers,nTesters,ZEnum_Zonee,(DWORD)types);	
	}
	void EnumZoneeEx(SpacialTester &tester,DWORD types,SpacialTester *includer,DWORD nIncluders,SpacialTester *occluder,DWORD nOccluders,DWORD *cullInc=NULL,DWORD *cullExc=NULL);
	Zonee *CastRayZonee(i_math::line3df &line,ZoneeType types,CastRayCallBack callback=NULL,i_math::vector3df *pos=NULL);
	BOOL EnumRgn(SpacialTester &tester,RgnType types)	{		_Enum(&tester,1,ZEnum_Rgn,(DWORD)types);	return TRUE;}
	Zonee **GetEnumZonee(DWORD &count);//call this immediately after Enum(..) is called
	ZoneRgn **GetEnumRgn(DWORD &count);//call this immediately after Enum(..) is called
	virtual Zonee **GetGlobalZonee(DWORD &count);

	BOOL Init(DWORD w,DWORD h,float blocklen);//w和h以block为单位
	void Clear();
	BOOL Locate(float x,float z);//Set the map center,in meter
	void GarbageCollect();

	ZoneRgn *AddRgn(SpacialTester &tester,void *data,RgnType type);
	BOOL RemoveRgn(ZoneRgn *);
	BOOL UpdateRgn(ZoneRgn *,SpacialTester &tester);


	void SetRgnTestCallBack(RgnTestCallBack callback)	{		_callbackRgnTest=callback;	}

	void SetRgnEventHandler(RgnEventHandler handler)	{		_handlerRgnEvent=handler;	}

protected:	



	//Overriding for CCycleMap2
	virtual void _BeginTouchRect(i_math::recti &rc){}
	virtual void _Touch(i_math::pos2di &ptUnit,i_math::pos2di &ptRelative);
	virtual void _EndTouchRect()	{	}
	//

	_Node* _BuildQTreeNode(std::vector<_Node>&tree,
				i_math::recti &rc,i_math::recti &rcMap,DWORD iNode,DWORD depth);
	void _BuildQTree();

	//x,y is of local coordinate
	_Block *_GetBlock(i_math::pos2di &pt,BOOL bCINE,BOOL *bOutter=NULL);
	_Block *_GetInnerBlock(int x,int y)	{		return &_blkmap[y*CCycleMap2::GetWidth()+x];	}
	__forceinline _Block *_GetOutterBlock(i_math::pos2di &pt,BOOL bCINE);//CINE:Create If Not Existing

	//x,y is relative to the left-up corner of the map
	_Node *_GetBlockNode(int x,int y){	return _blknodes[y*CCycleMap2::GetWidth()+x];	}


	__forceinline void _Add(Zonee*zonee,BOOL bAlreadyIn,i_math::pos2di &pt0,DWORD idx);
	__forceinline void _Remove(Zonee*zonee,i_math::pos2di &pt0,DWORD idx);

	void _AddRgn(_Block *blk,ZoneRgn *rgn);
	void _RemoveRgn(_Block *blk,ZoneRgn *rgn);
	void _FlushZoneeRgn(Zonee*zonee,int method);	
	void _LinkZoneeRgn(Zonee*zonee,ZoneRgn *rgn);

	void _AddRgnToBlks(ZoneRgn *rgn,i_math::aabbox3df &aabbRgn);
	void _RemoveRgnFromBlks(ZoneRgn *rgn);


	void _FlushDead(BOOL bFlushAll);
	void _UpdateQTree();

	void _FlushGlobalDead();


	void  _AddEnumBlock(_Block *blk,i_math::aabbox3df *aabb,ZoneeFlag flagFilter);//只有有flagFilter里的标志位的zonee才会被enum

	void _EnumNode(SpacialTester &tester,_Node *node,ZoneeFlag flagFilter=(ZoneeFlag)0);
	void _Enum(SpacialTester *testers,DWORD nTester,ZonerEnum ze,DWORD types);
	void _EnumCore(SpacialTester &tester,ZoneeFlag flagFilter=(ZoneeFlag)0);

	void _FilterZonee_And(SpacialTester *testers,DWORD nTester,DWORD types);

	inline BOOL _TestRgnAgainstZonee(ZoneRgn *rgn,Zonee *zonee)
	{
		if (zonee->TestFlag(ZoneeFlag_Dead))
			return FALSE;

		if (_callbackRgnTest)
			return _callbackRgnTest(rgn,zonee);
		if (SpacialTester::NoTouch==rgn->tester.Test(zonee->aabb))
			return FALSE;
		return TRUE;
	}

	float _blocklen;

	_Block *_blkmap;

	std::map<__int64,_Block *>_blksOutter;//_blksOutter records all the blocks that 
																		//are out of _rcMap,and the blocks in it
																		//are allocated from _blkpool.These blocks
																		//will NOT be managed by the quad tree
	CMemPool<_Block>_blkpool;

	std::vector<_Node> _qtree;//quad tree,the 1st element of this vector is the root node
	DWORD _leafstart;//the index to the first leaf node in _qtree
	int _depth;//max depth of the quad tree

	std::vector<_Node*> _blknodes;//a map to convert a block coordinate to the node occupying it
															//the coord is releative to the left-up corner of the map

	i_math::recti _rcMap;//in block,of world coordinate

	BOOL _bGlobalDirty;
	std::vector<Zonee*>_globals;//全局的zonee

	ZonerEnum _zeCur;
	DWORD _typesCur;
	CastRayCallBack _callback;
	SpacialTester *_testerCur;

	std::vector<void*>_bufTemp;//temp buffer 
	std::vector<Zonee*>_bufZonee;//temp buffer for zonee enumeration
	std::deque<Zonee*>_bufZonee2;//temp buffer for zonee enumeration
	std::vector<Zonee*>_bufZonee3;//temp buffer for zonee enumeration
	std::vector<ZoneRgn*>_bufRgn;//temp buffer for rgn enumeration
	std::vector<ZoneRgn*>_bufAnchorRgn;//temp buffer for GetRgnFromAnchor(..)

	RgnTestCallBack _callbackRgnTest;
	RgnEventHandler _handlerRgnEvent;

	int _lastgc;


	friend struct _Block;

};


#pragma warning(default:4312)

