// AStar.h: interface for the CAStar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASTAR_H__188F22E5_30B3_11D4_83C9_0080C8F6368B__INCLUDED_)
#define AFX_ASTAR_H__188F22E5_30B3_11D4_83C9_0080C8F6368B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <queue>
#include <vector>
#include <list>
/*-------------------------------------------------------------
					路徑搜索引擎:
	目前使用A*路徑搜索算法, 以後可以考慮擴充其他算法,
	適用於任何以Tile為單位的2D數組的地圖格式, 對於這種
	格式, 基本上只需要重新定義Tile并修改讀取自定義格式
	地圖的接口函數即可.
-------------------------------------------------------------*/

#include "AStarTileDefine.h"

#define SET_PATTERNMAP_BLOCK(pMap,x,y) pMap[((y)>>2)*m_nBigChipW+((x)>>2)]|=(1<<(((x)&0x3)+(((y)&0x3)<<2)))
#define CLEAR_PATTERNMAP_BLOCK(pMap,x,y) pMap[((y)>>2)*m_nBigChipW+((x)>>2)]&=(~(1<<(((x)&0x3)+(((y)&0x3)<<2))))


#define CLOSE_THRESHHOLD 2 //Used to check invalid target



///////////////////////////////////////////////////////////////////////////
//The original AStar method

//	A step in path
//	行走路徑的一個點
class PATH
{
public:
	short x;short y;
};

#define NODE_STAT_UNDEFINE 0
#define NODE_STAT_OPEN 1
#define NODE_STAT_CLOSE 2
#define NODE_STAT_BLOCK 3

class Node
{
public:
	Node* m_pNext;//link in open list
    Node* parent;            // parent node (zero pointer represents starting node)
    int cost;              // cost to get to this node
    int total;             // total cost (cost + heuristic estimate)
    short x;short y;   // location of node (some location representation)
    BYTE  stat;             // on Open list or close list,or neither
	BYTE nodeAttr;
};


class PriorityQueue
{
public:
	Node* m_pHead;
	void Initialize()
	{
		m_pHead=NULL;
	}
};

/*class PriorityQueue
{
public:
	Node *m_aNode[MAX_QUEUE_LEN];
	int m_nCurNodeLen;
	void Initialize()
	{
		m_nCurNodeLen=0;
	}
};
*/


/*
class PriorityQueue
{
public:
   //Heap implementation using an STL vector
   //Note: the vector is an STL container, but the
   //operations done on the container cause it to
   //be a priority queue organized as a heap
	std::vector<Node*> heap;
   void Initialize()
   {
	   heap.clear();
   }
};
class NodeTotalGreater 
{
public:
   //This is required for STL to sort the priority queue
   //(its entered as an argument in the STL heap functions)
   bool operator()( Node * first, Node * second ) const {
      return( first->total > second->total );
   }
};
*/


#define MAX_NODE_BLOCK_LEN 4096
#define MAX_NODE_BLOCK_SHIFT 12

class MasterNodeList
{
public:
	MasterNodeList()
	{
		m_aValidInfo=NULL;
		m_aNodeBufferIndex=NULL;
	}
	BYTE *m_aValidInfo;
	WORD *m_aNodeBufferIndex;
	int m_nBufferLen;
	void Reset();
	void Initialize(int nWidth,int nHeight);
	void Unintialize();
};

class FreeNodeBank
{
public:
	FreeNodeBank()
	{
		m_nCurBlock=0;
		m_aNodeBuffer[0]=0;
	}
	Node *m_aNodeBuffer[10];
	int m_nCurBlock;//0-9
	Node* m_pBottom;

	void Initialize()
	{
		m_aNodeBuffer[0]=new Node[MAX_NODE_BLOCK_LEN];
		int i;
		for (i=1;i<10;i++)
		{
			m_aNodeBuffer[i]=0;
		}
	}
	void Reset()
	{
		m_pBottom=&(m_aNodeBuffer[0][0]);
		m_nCurBlock=0;
	}
	void Uninitialize()
	{
		int i;
		for (i=0;i<=m_nCurBlock;i++)
		{
//				ASSERT(m_aNodeBuffer[i]);
			if (m_aNodeBuffer[i])
			{
				delete m_aNodeBuffer[i];
				m_aNodeBuffer[i]=NULL;
			}
		}
		m_nCurBlock=0;
	}
	Node* GetANewNode(WORD& wIndex)
	{
		int nIndexInBlock=m_pBottom-&(m_aNodeBuffer[m_nCurBlock][0]);
		if ( nIndexInBlock>=MAX_NODE_BLOCK_LEN )
		{
			m_nCurBlock++;
			ASSERT(m_nCurBlock<10);
			if (!m_aNodeBuffer[m_nCurBlock])
				m_aNodeBuffer[m_nCurBlock]=new Node[MAX_NODE_BLOCK_LEN];
			m_pBottom=&(m_aNodeBuffer[m_nCurBlock][0]);

			wIndex=m_nCurBlock<<MAX_NODE_BLOCK_SHIFT;
			return m_pBottom++;
		}
		else
		{
			wIndex=(m_nCurBlock<<MAX_NODE_BLOCK_SHIFT)+nIndexInBlock;
			return m_pBottom++;
		}
	}
	Node* GetANodeByIndex(WORD& wIndex)
	{
		int nBlock=wIndex>>MAX_NODE_BLOCK_SHIFT;
		return &(m_aNodeBuffer[nBlock][wIndex-(nBlock<<MAX_NODE_BLOCK_SHIFT)]);
	}
};

struct BigTileInfo
{
	DWORD low;
	DWORD high;
};


class CAStar  
{
public://temp
	WORD* m_pDisMap;
	int m_nBigChipW;
	int m_nBigChipH;
	BYTE* m_pPatternConnectiveTable;
public:
	PriorityQueue m_open;
	
	inline Node* PopOpenQueue();
	bool IsOpenQueueEmpty();
	inline void UpdateNodeOnOpenQueue(Node* node );
	inline void PushOpenQueue(Node* node);

	FreeNodeBank m_FreeNodeBank;
	MasterNodeList m_MasterNodeList;
	inline Node* GetNodeFromMasterNodeList(short x,short y );
	inline Node* NewANode(short x,short y ,BYTE nodeAttr);

	BOOL IsNodeConnect(Node *root,Node* newNode,WORD* pRootPattern=NULL,WORD* pNewNodePattern=NULL);
	//	Start pf path find
	//	路徑搜索起始點
	int		m_nStartX;
	int		m_nStartY;
	//	Goal of path find
	//	路徑搜索目標點
	int		m_nEndX;
	int		m_nEndY;

	BOOL IsValidFreeTile(short x,short y);

	//	Walkable information for real path find, include all map units
	//	用來進行路徑搜索的可走信息(包括地圖所有單元)
	WORD*m_pSearchMap;
	WORD *m_pPatternMap;
	WORD *m_pPatternMapGrace;
	WORD *m_pPatternMapGround;
	WORD *m_pPatternMapWater;
	WORD *m_pPatternMapAir;

	//	Record path been found
	//	紀錄搜索到的路徑	
	Node**m_pPathWork;
	// m_path length
	// 為路徑分配的長度
	int m_nPathLen;
	//	Total steps in path
	//	路徑點總數
	int		m_nTotalStep;
	//	Maximum search step when think of search time
	//	最大可走步數(出於搜索時間考量)
	int		m_nMaxSearchStep;
	void SetMaxSearchStep(int nMaxSearchStep);
	//	Size of search map
	//	搜索地圖的尺寸
	int		m_nMapWidth;
	int		m_nMapHeight;
	// search range rect
	i_math::recti m_rcSearch;

public:
	CAStar();
	virtual ~CAStar();
	//	Initialize necessary data for search
	//	搜索必需數據初始化
	virtual void OnInit(void);
	//	Release memory allocated in search progress
	//	釋放搜索過程分配的內存
	virtual void OnDestroy(void);
	//	Create search data struct according to map walkable infomation
	//	根據地圖可走信息創建搜索必需數據結構
	virtual void OnCreate(BYTE* pMapData, int nMapWidth, int nMapHeight);
	//	Refresh walkable information of move objects
	//	刷新移動物體可走信息
public:
	BOOL CanFindPath(short xStart,short yStart,short xEnd,short yEnd,int searchstep);
	bool FindPath(short xStart, short yStart, short &xEnd,short &yEnd,i_math::recti &rcNextFind);

	WORD m_AttrBlocking;//What tile attr is blocking
	WORD SetBlockingAttr(DWORD attrBlocking)
	{
		attrBlocking|=ASTAR_TILE_BOUNDARY;
		WORD temp;
		temp=m_AttrBlocking;
		m_AttrBlocking=attrBlocking;
		return temp;
	}
	//	Get total path steps, include start and goal
	//	得到路徑點總數(包括起始點和目標點)
	int GetTotalStep(void) { return m_nTotalStep; }
	//	Decide whether the tile is walkable
	//	判斷指定點是否可走
	BOOL IsFreeTile(int x, int y) 
	{
		if (m_pSearchMap[y*m_nMapWidth+x]&m_AttrBlocking)
			return FALSE;
		return TRUE;
	}

	BOOL IsFreeTile(int idx)
	{
		if(idx<0)
			return FALSE;
		if(idx>=m_nMapHeight*m_nMapWidth)
			return FALSE;
		if (m_pSearchMap[idx]&m_AttrBlocking)
			return FALSE;
		return TRUE;
	}

	void SetTile_Boundary(int x,int y)
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_BOUNDARY; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		SET_PATTERNMAP_BLOCK(m_pPatternMapAir,x,y);
		SET_PATTERNMAP_BLOCK(m_pPatternMapGrace,x,y);
	}

	void SetTile_DynGeneral(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_DYNGENERAL; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		//		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	void ClearTile_DynGeneral(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_DYNGENERAL)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		
		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	
	void SetTile_General(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_GENERAL; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		//		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	void ClearTile_General(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_GENERAL)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		
		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	

	void SetTile_Water(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_WATER; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		//		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	void ClearTile_Water(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_WATER)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		
		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	
	
	//	Set/Clear walkable information of peferred position
	//	設置/清除指定點的可走信息
	void SetTile_StaticBlocked(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_STATICBLOCKED; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		//		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	void ClearTile_StaticBlocked(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_STATICBLOCKED)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);

		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}

	void SetTile_FlyingStaticBlocked(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_FLYINGSTATICBLOCKED; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapAir,x,y);
	}
	void ClearTile_FlyingStaticBlocked(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_FLYINGSTATICBLOCKED)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_FLYINGSTATICBLOCKED|ASTAR_TILE_FLYINGCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapAir,x,y);
	}
	


//	void SetTile_Water(int x, int y) 
//	{
//		WORD v;
//		v=(m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_WATER); 
//
//		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
//
//		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
//			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
//	}
//	//tileAttr should be none-creature
//	void ClearTile_Water(int x, int y) 
//	{ 
//		WORD v;
//		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_WATER)); 
//
//		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATER|ASTAR_TILE_WATERCREATURE)))
//			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
//		
//		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
//	}

	void SetTile_Creature(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_CREATURE; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		SET_PATTERNMAP_BLOCK(m_pPatternMapGrace,x,y);
		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);

	}
	void ClearTile_Creature(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_CREATURE)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);

		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_CREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGrace,x,y);
		
		
		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_GENERAL|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}

	void SetTile_FlyingCreature(int x, int y) 
	{
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_FLYINGCREATURE); 
		SET_PATTERNMAP_BLOCK(m_pPatternMapAir,x,y);

		if (v&ASTAR_TILE_GENERAL)
			SET_PATTERNMAP_BLOCK(m_pPatternMapGrace,x,y);
	}
	void ClearTile_FlyingCreature(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_FLYINGCREATURE)); 
		
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_FLYINGSTATICBLOCKED|ASTAR_TILE_FLYINGCREATURE)))
		{
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapAir,x,y);
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGrace,x,y);
		}
	}
	
	void SetTile_WaterCreature(int x, int y) 
	{
		m_pSearchMap[y * m_nMapWidth + x] |= ASTAR_TILE_WATERCREATURE; 
		SET_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		SET_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}
	void ClearTile_WaterCreature(int x, int y) 
	{ 
		WORD v;
		v=(m_pSearchMap[y * m_nMapWidth + x] &= ~((WORD)ASTAR_TILE_WATERCREATURE)); 
		if (!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_CREATURE|ASTAR_TILE_WATERCREATURE)))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapGround,x,y);
		
		if ((v&ASTAR_TILE_WATER)&&(!(v&(ASTAR_TILE_BOUNDARY|ASTAR_TILE_STATICBLOCKED|ASTAR_TILE_WATERCREATURE|ASTAR_TILE_CREATURE))))
			CLEAR_PATTERNMAP_BLOCK(m_pPatternMapWater,x,y);
	}

	BOOL TestTile(int x,int y,WORD tileAttr) { return m_pSearchMap[y * m_nMapWidth + x]&tileAttr;}

	//	Get walkable information of peferred position
	//	得到指定點的可走信息
	WORD GetTile(int x, int y);

	//by Rosey
	void SetSearchRange(i_math::recti &rc);
	void SetDefaultSearchRange();

	WORD *GetSearchMap(){return m_pSearchMap;}

	int GetWidth(){return m_nMapWidth;}
	int GetHeight(){return m_nMapHeight;}

};

#endif // !defined(AFX_ASTAR_H__188F22E5_30B3_11D4_83C9_0080C8F6368B__INCLUDED_)

