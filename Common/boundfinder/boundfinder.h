#pragma once

#include "mempool/mempool.h"
#include <vector>


//Boundary Direction
#define BD_LEFT 0
#define BD_RIGHT 1
#define BD_UP 2
#define BD_DOWN 3


class CStripe 
{
public:
	int start,end;
	BOOL startMarked,endMarked;
	CStripe *pNext;
	CStripe *pPrev;
};

class CBoundNode
{
public:
	unsigned char dir;
	int dist;
	CBoundNode *pNext;
};

class CBound
{
public:
	CBoundNode *pStartNode;
	CBound *pNext;
	int x,y;
};


class CBoundFinder  
{
public:
	CBoundFinder()
	{
		_bounds=NULL;
	}
	virtual ~CBoundFinder()
	{
	}
	//operations
	BOOL Find(unsigned char *pData,int w,int h);

	//the results
	std::vector<i_math::pos2di>m_points;
	std::vector<int>m_contourend;


protected:
	void DestroyBoundary();
	BOOL ConstructPCS(unsigned char *pData,int w,int h);
	void RemovePCS();
	BOOL PCStoChain(int h);
	CBoundNode *BottomUp(CStripe *pStripe,int y);//for the left end,up
	CBoundNode *TopDown(CStripe *pStripe,int y);//for the right end,down
	void Unique(CBound *pBoundary);
	BOOL IsTurningRight(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3);
	BOOL IsTurningLeft(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3);
	int GetAliaseType(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3,i_math::pos2di pt4);
	BOOL IsTurning(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3);

	void Optimize();

protected:
	//working data
	CBound *_bounds;
	std::vector<CStripe *>_stripes;
	std::vector<CStripe *>_stripes2;
	CMemPool<CStripe> _poolStripe;
	CMemPool<CBoundNode> _poolBN;
	CMemPool<CBound> _poolBoundary;
	DWORD _h;


};
