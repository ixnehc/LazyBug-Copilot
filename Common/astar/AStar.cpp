// AStar.cpp: implementation of the CAStar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdh.h"
#include "AStar.h"
#include <math.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



#define BIGTILE_CONNECT_ALLCONNECT 1
#define BIGTILE_CONNECT_VERTICAL 2
#define BIGTILE_CONNECT_HORIZONTAL 4

#define NEXTFIND_RECT_SIZELIMIT 900 //30*30




#define BUFFERSIZE 10000

#define MAX_SEARCH_LOOP 4
#define MAX_SEARCH_STEP 1000

#define SET_PATTERNMAP_BLOCK(pMap,x,y) pMap[((y)>>2)*m_nBigChipW+((x)>>2)]|=(1<<(((x)&0x3)+(((y)&0x3)<<2)))
#define CLEAR_PATTERNMAP_BLOCK(pMap,x,y) pMap[((y)>>2)*m_nBigChipW+((x)>>2)]&=(~(1<<(((x)&0x3)+(((y)&0x3)<<2))))
///////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////
CAStar::CAStar()
{
	OnInit();
}

CAStar::~CAStar()
{
	SAFE_DELETE(m_pPatternConnectiveTable);
}

///////////////////////////////////////////////////////////////
//	Initialize necessary data for search
//	�����������ݳ�ʼ��
///////////////////////////////////////////////////////////////
extern BYTE *CalculateBigTileConnectionTable2();
void CAStar::OnInit(void)
{
	m_pSearchMap = NULL;
	m_pPathWork=NULL;
	m_AttrBlocking=0;
	m_pPatternConnectiveTable=CalculateBigTileConnectionTable2();

	m_pPatternMapGround=NULL;
	m_pPatternMapGrace=NULL;
	m_pPatternMapAir=NULL;
	m_pPatternMapWater=NULL;

	m_pDisMap=NULL;
	
	
}

///////////////////////////////////////////////////////////////
//	Release memory allocated in search progress
//	�ͷ��������̷�����ڴ�
///////////////////////////////////////////////////////////////
void CAStar::OnDestroy(void)
{
	if(m_pSearchMap)
	{
		delete [] m_pSearchMap;
		m_pSearchMap = NULL;
	}

	SAFE_DELETE(m_pPathWork);


	SAFE_DELETE(m_pPatternMapGround);
	SAFE_DELETE(m_pPatternMapAir);
	SAFE_DELETE(m_pPatternMapWater);
	SAFE_DELETE(m_pPatternMapGrace);
	
	m_MasterNodeList.Unintialize();
	m_FreeNodeBank.Uninitialize();
	//temp

	if (m_pDisMap)
		delete m_pDisMap;
//	SAFE_DELETE(m_pDisMap);


} 

///////////////////////////////////////////////////////////////
//	Create search data struct according to map walkable infomation
//	���ݵ�ͼ������Ϣ���������������ݽṹ
///////////////////////////////////////////////////////////////
void CAStar::OnCreate(BYTE* pMapData, int nMapWidth, int nMapHeight)
{
	_ASSERT(pMapData != NULL);

	ASSERT(nMapWidth%4==0);
	ASSERT(nMapHeight%4==0);
	m_pSearchMap = new WORD[nMapWidth * nMapHeight];
	m_pPatternMapGround=new WORD[nMapWidth*nMapHeight/16];
	m_pPatternMapGrace=new WORD[nMapWidth*nMapHeight/16];
	m_pPatternMapAir=new WORD[nMapWidth*nMapHeight/16];
	m_pPatternMapWater=new WORD[nMapWidth*nMapHeight/16];
	memset(m_pPatternMapGround,0,sizeof(WORD)*nMapWidth*nMapHeight/16);
	memset(m_pPatternMapGrace,0,sizeof(WORD)*nMapWidth*nMapHeight/16);
	memset(m_pPatternMapAir,0,sizeof(WORD)*nMapWidth*nMapHeight/16);
	memset(m_pPatternMapWater,0,sizeof(WORD)*nMapWidth*nMapHeight/16);

	_ASSERT(m_pSearchMap != NULL);

	//	Save map size
	//	���Ƶ�ͼ�ߴ�
	m_nMapWidth  = nMapWidth;
	m_nMapHeight = nMapHeight;
	
	m_nBigChipW=m_nMapWidth/4;
	m_nBigChipH=m_nMapHeight/4;
	
	m_rcSearch=i_math::recti(0,0,m_nMapWidth,m_nMapHeight);
	
	//	Copy map walkable information to terrain map
	//	���Ƶ�ͼ������Ϣ�����ε�ͼ
	BYTE* s = pMapData;
	WORD* t = m_pSearchMap;
	for(int i = 0; i < nMapHeight; i++)
	{
		for(int j = 0; j < nMapWidth; j++)
		{
			BYTE v;
			v=*s++;

			if ((i==0)||(j==0)||(i==nMapHeight-1)||(j==nMapWidth-1))//The border
			{
				*t=ASTAR_TILE_GENERAL|ASTAR_TILE_FLYINGGENERAL;
				SetTile_StaticBlocked(j,i);
				SetTile_FlyingStaticBlocked(j,i);
				SetTile_Boundary(j,i);
			}
			else
			{
				if (v&(ASTAR_TILE_GENERAL))
				{
					*t=ASTAR_TILE_GENERAL;
					SetTile_General(j,i);
				}
				else
				{
					if (v&(ASTAR_TILE_WATER))
					{
						*t=ASTAR_TILE_WATER;
						SetTile_Water(j,i);
					}
					else
						*t=ASTAR_TILE_FREE;
				}
			}
			t++;
		}
	}
	

	
	//	Calculate maximum search step 
	//	���������߲���(�������ʱ�俼��)
	//	2000.9.21 - ���ܱ䶯
	//by Rosey	
	m_nMaxSearchStep = MAX_SEARCH_STEP;

	m_nPathLen=(m_rcSearch.Width() + m_rcSearch.Height() )*4;
	m_pPathWork=new Node*[m_nPathLen];	
	_ASSERT(m_pPathWork!= NULL);

	m_MasterNodeList.Initialize(m_nMapWidth,m_nMapHeight);
	m_FreeNodeBank.Initialize();
	//temp
	if (TRUE)
	{
		int i,j;
		m_pDisMap=new WORD[m_nMapHeight*m_nMapWidth];
		WORD* p=m_pDisMap;
		for (i=0;i<m_nMapHeight;i++) 
		{
			for (j=0;j<m_nMapWidth;j++)
			{
				*p=sqrtf(i*i+j*j)*1.1;//normally, actual cost will be more than line distance
				p++;
			}
		}

	}
	m_nBigChipW=m_nMapWidth>>2;
	m_nBigChipH=m_nMapHeight>>2;

	m_MasterNodeList.Reset();
}

//by Rosey{
void CAStar::SetSearchRange(i_math::recti &rc)
{
	m_rcSearch=rc;
	if (m_rcSearch.left<3)
		m_rcSearch.left=3;
	if (m_rcSearch.top<3)
		m_rcSearch.top=3;
	if (m_rcSearch.right>=m_nMapWidth-3)
		m_rcSearch.right=m_nMapWidth-4;
	if (m_rcSearch.bottom>=m_nMapHeight-5)
		m_rcSearch.bottom=m_nMapHeight-6;
}

void CAStar::SetDefaultSearchRange()
{
	m_rcSearch=i_math::recti(0,0,m_nMapWidth,m_nMapHeight);
}
//}

///////////////////////////////////////////////////////////////
//	Get walkable information of peferred position
//	�õ�ָ����Ŀ�����Ϣ
///////////////////////////////////////////////////////////////
WORD CAStar::GetTile(int x, int y)
{
	if((x < 0) || (x >= m_nMapWidth))
		return ASTAR_TILE_GENERAL;
	if((y < 0) || (y >= m_nMapHeight))
		return ASTAR_TILE_GENERAL;

	return m_pSearchMap[y * m_nMapWidth + x];
}
//new version ,the following
Node* CAStar::PopOpenQueue()
{
	if (FALSE)
	{
		int fMin=1000000000;
		Node *pBestFather,*pBest=NULL;
		Node* p=m_open.m_pHead;
		Node* pFather=NULL;
		while(p)
		{
			if (p->total < fMin)
			{
				pBest=p;
				pBestFather=pFather;
				fMin=p->total;
			}
			pFather=p;
			p=p->m_pNext;
		}
		ASSERT(pBest);
		if (!pBestFather)
		{
			m_open.m_pHead=m_open.m_pHead->m_pNext;
			pBest->m_pNext=NULL;
			return pBest;
		}
		else
		{
			pBestFather->m_pNext=pBest->m_pNext;
			pBest->m_pNext=NULL;
			return pBest;
		}
	}
	else
	{
		ASSERT(m_open.m_pHead);
		Node* pNode=m_open.m_pHead;
		m_open.m_pHead=pNode->m_pNext;
		ASSERT(pNode->stat==NODE_STAT_OPEN);
		pNode->m_pNext=NULL;

		return pNode;
	}
}

void CAStar::PushOpenQueue(Node* node)
{
	if (FALSE)
	{
		node->m_pNext=m_open.m_pHead;
		m_open.m_pHead=node;
	}
	else
	{
		ASSERT(node->stat==NODE_STAT_OPEN);
		if (!m_open.m_pHead)//empty list
		{
			m_open.m_pHead=node;
			node->m_pNext=NULL;
			return;
		}

		Node* p=m_open.m_pHead;
		Node* pFarther=NULL;
		while(p)
		{
			if (p->total >=node->total)
				break;
			pFarther=p;
			p=p->m_pNext;
		}
		if (pFarther==NULL)//first
		{
			ASSERT(p==m_open.m_pHead);
			node->m_pNext=p;
			m_open.m_pHead=node;
		}
		else
		{
			node->m_pNext=p;
			pFarther->m_pNext=node;
		}
	}
}

void CAStar::UpdateNodeOnOpenQueue( Node* node )
{
	//its total value was changed before this function was called	
	Node* pPickOut,*pInsertEnd;
	Node* p=m_open.m_pHead;
	Node* pFarther=NULL;
	while(p)
	{
		if (p == node)
			break;
		pFarther=p;
		p=p->m_pNext;
	}
	ASSERT(p);

	if (pFarther==NULL)//first
	{
		return;
	}
	else
	{
		pPickOut=p;
		pFarther->m_pNext=pPickOut->m_pNext;
		pInsertEnd=pFarther->m_pNext;
		pPickOut->m_pNext=NULL;
	}
	//insert
	p=m_open.m_pHead;
	pFarther=NULL;
	while(p!=pInsertEnd)
	{
		if (p->total >= node->total)
			break;
		pFarther=p;
		p=p->m_pNext;
	}
	if (pFarther==NULL)//first
	{
		ASSERT(p==m_open.m_pHead);
		pPickOut->m_pNext=p;
		m_open.m_pHead=pPickOut;
	}
	else
	{
		pPickOut->m_pNext=p;
		pFarther->m_pNext=pPickOut;
	}

}

bool CAStar::IsOpenQueueEmpty()
{
   return(! m_open.m_pHead);
}


Node* CAStar::GetNodeFromMasterNodeList(short x,short y)
{
	int n=y*m_nMapWidth+x;
	ASSERT(x>=0);
	ASSERT(y>=0);
	ASSERT(x<m_nMapWidth);
	ASSERT(y<m_nMapHeight);
	

	int nIndex=n>>3;
	if (m_MasterNodeList.m_aValidInfo[nIndex] & (1<<(n-(nIndex<<3))))
	{
		return m_FreeNodeBank.GetANodeByIndex(m_MasterNodeList.m_aNodeBufferIndex[n]);
	}
	return NULL;
}

Node* CAStar::NewANode(short x,short y,BYTE nodeAttr)
{
	WORD wNewNodeIndex;
	Node* newNode=m_FreeNodeBank.GetANewNode(wNewNodeIndex);
	newNode->x=x;
    newNode->y=y;
	newNode->nodeAttr=nodeAttr;
    newNode->stat= NODE_STAT_UNDEFINE;

    //StoreNodeInMasterNodeList places the node into the hash table
	ASSERT(x>=0);
	ASSERT(y>=0);
	ASSERT(x<m_nMapWidth);
	ASSERT(y<m_nMapHeight);
	int n=y*m_nMapWidth+x;
	m_MasterNodeList.m_aNodeBufferIndex[n]=wNewNodeIndex;

	int nIndex=n>>3;
	m_MasterNodeList.m_aValidInfo[nIndex] |= (1<<(n-(nIndex<<3)));//temp

    return( newNode ); 
}



#define GET_NODE_HEURISTIC(x,y) (HIGHRESO_FACTOR*m_pDisMap[abs(m_nEndY - (y))*m_nMapWidth+abs(m_nEndX - (x))])
#define GET_DISTANCE(x0,y0,x1,y1) (m_pDisMap[abs((y1)-(y0))*m_nMapWidth+abs((x1)-(x0))])
#define IS_UNOBSTRUCTED(wPattern) (m_pPatternConnectiveTable[(WORD)(wPattern)]&BIGTILE_CONNECT_ALLCONNECT)


#define PATTERN_BIT(x,y)  (1<<(((y)<<2)+(x)))
#define INSIDE_PATTERN(x,y) (((x)>=0)&&((x)<=3)&&((y)>=0)&&((y)<=3))


 

//#pragma optimize("gts" ,off) 

BOOL Test(WORD v,int x,int y)  
{
	 return ((v) >> (4* y+ x)) & 0x01;
}

BOOL IsNodeConnect_1(Node *root,Node* newNode,WORD* pRootPattern,WORD* pNewNodePattern)
{
	int i,j;
	BOOL bConnect;
	bConnect=FALSE;
	if (root->y==newNode->y)
		{
			if (root->x<newNode->x)//left-right
			{
				int xL,yL,xR,yR;
				xL=3;
				xR=0;
				int nMin,nMax;
				for (i=0;((i<4 )&& (!bConnect));i++)
				{
					yL=i;
					//whether walkable
					BOOL b;
					b= ((WORD)(*pRootPattern) >> (4* yL + xL)) & 0x01;
					if (b)
//					if ( ((WORD)(*pRootPattern) >> (4* yL + xL)) & 0x01)
//					if (Test(*pRootPattern,xL,yL))
					{ 
						nMin=i-1;
						if (nMin<0)
							nMin=0;
						nMax=i+2;
						if (nMax>4)  
							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							yR=j;
							BOOL b;
							b= ((WORD)(*pNewNodePattern) >> (4* yR +xR)) & 0x01;
							if (b)
								//							if ( ((WORD)(*pNewNodePattern) >> (4* yR + xR)) & 0x01)
							//							if (Test(*pNewNodePattern,xR,yR))
							{
								newNode->nodeAttr= ((yR+1)<<4)|(xR+1);
								return TRUE;
							}
						}
					}
				}
			}
			else//right-left
			{
				int xL,yL,xR,yR;
				xL=3;
				xR=0;
				int nMin,nMax;
				for (i=0;((i<4) && (!bConnect));i++)
				{
					yL=i;
					//whether walkable
//					if (Test(*pNewNodePattern,xL,yL))
					BOOL b;
					b= ((WORD)(*pNewNodePattern) >> (4* yL + xL)) & 0x01;
					if (b)
//						if ( ((WORD)(*pNewNodePattern)>> (4* yL + xL)) & 0x01)
					{
						nMin=i-1;
						if (nMin<0)
							nMin=0;
						nMax=i+2;
						if (nMax>4)
							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							yR=j;
//							if (Test(*pRootPattern,xR,yR))
							BOOL b;
							b= ((WORD)(*pRootPattern) >> (4* yR+ xR)) & 0x01;
							if (b)
//								if ( ((WORD)(*pRootPattern) >> (4* yR+ xR)) & 0x01)
							{
								newNode->nodeAttr= ((yL+1)<<4)|(xL+1);
								return TRUE;
							}
						}
					}
				}
			}
		}
		else if (root->x==newNode->x)
		{
			if (root->y < newNode->y)//top-bottom
			{
				int xT,yT,xB,yB;
				yT=3;
				yB=0;
				int nMin,nMax;
				for (i=0;i<4 && !bConnect;i++)
				{
					xT=i;
					//whether walkable
//					if (Test(*pRootPattern,xT,yT))
					BOOL b;
					b= ((WORD)(*pRootPattern) >> (4* yT + xT)) & 0x01;
					if (b)
//						if ( ((WORD)(*pRootPattern) >> (4* yT + xT)) & 0x01)
					{
						nMin=i-1;
						if (nMin<0)
							nMin=0;
						nMax=i+2;
						if (nMax>4)
							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							xB=j;
//							if (Test(*pNewNodePattern,xB,yB))
							BOOL b;
							b= ((WORD)(*pNewNodePattern) >> (4* yB + xB)) & 0x01;
							if (b)
//								if ( ((WORD)(*pNewNodePattern) >> (4* yB + xB)) & 0x01)
							{
								newNode->nodeAttr=((yB+1)<<4)|(xB+1);
								return TRUE;
							}
						}
					}
				}
			}
			else
			{
				int xT,yT,xB,yB;
				yT=3;
				yB=0;
				int nMin,nMax;
				for (i=0;i<4 && !bConnect;i++)
				{
					xT=i;
					//whether walkable
//					if (Test(*pNewNodePattern,xT,yT))
					BOOL b;
					b= ((WORD)(*pNewNodePattern) >> (4* yT + xT)) & 0x01;
					if (b)
//						if ( ((WORD)(*pNewNodePattern) >> (4* yT + xT)) & 0x01)
					{
						nMin=i-1;
						if (nMin<0)
							nMin=0;
						nMax=i+2;
						if (nMax>4)
							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							xB=j;
//							if (Test(*pRootPattern,xB,yB))
							BOOL b;
							b= ((WORD)(*pRootPattern) >> (4* yB+ xB)) & 0x01;
							if (b)
//								if ( ((WORD)(*pRootPattern) >> (4* yB + xB)) & 0x01)
							{
								newNode->nodeAttr=((yT+1)<<4)|(xT+1);
								return TRUE;
							}
						}
					}
				}
			}
		}
		else if (root->x>newNode->x && root->y<newNode->y)//root is at the right top of new node
		{
			bConnect =( (((*pRootPattern)>>12)&0x01)  &&  (((*pNewNodePattern)>>3)&0x01) );
			if (bConnect)
				newNode->nodeAttr= 20;//((0+1)<<4)|(3+1);
			return bConnect;

		}
		else if (newNode->x>root->x && newNode->y<root->y)//new node is at the right top of root
		{
			bConnect= ( (((*pNewNodePattern)>>12)&0x01)  &&  (((*pRootPattern)>>3)&0x01) );
			if (bConnect)
				newNode->nodeAttr=65;//((3+1)<<4)|(0+1);
			return bConnect;
		}
		else if (root->x>newNode->x && root->y>newNode->y)//root is at the right bottom of new node
		{
			bConnect=( (((*pRootPattern)>>0)&0x01)  &&  (((*pNewNodePattern)>>15)&0x01) );
			if (bConnect)
				newNode->nodeAttr=68;//((3+1)<<4)|(3+1);
			return bConnect;
		}
		else if (newNode->x>root->x && newNode->y>root->y)//new node is at the right bottom of root
		{
			bConnect=( (((*pNewNodePattern)>>0)&0x01)  &&  (((*pRootPattern)>>15)&0x01) );
			if (bConnect)
				newNode->nodeAttr=17;//((0+1)<<4)|(0+1);
			return bConnect;
		}

		return FALSE;
		
}

BOOL IsNodeConnect_2(Node *root,Node* newNode,WORD* pRootPattern,WORD* pNewNodePattern)
{
	int i;
	BOOL bConnect=FALSE;
	CPoint aOff[8]={CPoint(-1,-1),CPoint(0,-1),CPoint(1,-1),CPoint(-1,0),CPoint(1,0),CPoint(-1,1),CPoint(0,1),CPoint(1,1)};
	i_math::recti rcClip=i_math::recti(0,0,4,4);
	CPoint pt;
	if (pNewNodePattern)
	{
		ASSERT(!pRootPattern);
		for (i=0;i<8;i++)
		{
			pt.x=root->x+aOff[i].x-newNode->x;
			pt.y=root->y+aOff[i].y-newNode->y;
			if (rcClip.PtInRect(pt))
			{ 
				if ( ((*pNewNodePattern) >> (4* pt.y + pt.x)) & 0x01)
				{
					newNode->nodeAttr=((pt.y+1)<<4)|(pt.x+1);
					return TRUE;
				}
			}
		}
	}
	else
	{
		ASSERT(pRootPattern);
		for (i=0;i<8;i++)
		{
			pt.x=newNode->x+aOff[i].x-root->x;
			pt.y=newNode->y+aOff[i].y-root->y;
			if (rcClip.PtInRect(pt))
			{ 
				if ( ((*pRootPattern)>> (4* pt.y + pt.x)) & 0x01)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


BOOL CAStar::IsNodeConnect(Node *root,Node* newNode,WORD* pRootPattern/*=NULL*/,WORD* pNewNodePattern/*=NULL*/)
{
	int i,j;
	BOOL bConnect=FALSE;
	if (pRootPattern && pNewNodePattern)//both big chip
	{
//		return IsNodeConnect_1(root,newNode,pRootPattern,pNewNodePattern);
	if (root->y==newNode->y)
		{
			if (root->x<newNode->x)//left-right
			{
				int xL,yL,xR,yR;
				xL=3;
				xR=0;
				int nMin,nMax;
				for (i=0;((i<4 )&& (!bConnect));i++)
				{
					yL=i;
					//whether walkable
					BOOL b;
					b= ((WORD)(*pRootPattern) >> (4* yL + xL)) & 0x01;
					if (b)
//					if ( ((WORD)(*pRootPattern) >> (4* yL + xL)) & 0x01)
//					if (Test(*pRootPattern,xL,yL))
					{ 
//						nMin=i-1;
						nMin=i;
//						if (nMin<0)
//							nMin=0;
						nMax=i+1;
//						if (nMax>4)  
//							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							yR=j;
							BOOL b;
							b= ((WORD)(*pNewNodePattern) >> (4* yR +xR)) & 0x01;
							if (b)
								//							if ( ((WORD)(*pNewNodePattern) >> (4* yR + xR)) & 0x01)
							//							if (Test(*pNewNodePattern,xR,yR))
							{
								newNode->nodeAttr= ((yR+1)<<4)|(xR+1);
								return TRUE;
							}
						}
					}
				}
			}
			else//right-left
			{
				int xL,yL,xR,yR;
				xL=3;
				xR=0;
				int nMin,nMax;
				for (i=0;((i<4) && (!bConnect));i++)
				{
					yL=i;
					//whether walkable
//					if (Test(*pNewNodePattern,xL,yL))
					BOOL b;
					b= ((WORD)(*pNewNodePattern) >> (4* yL + xL)) & 0x01;
					if (b)
//						if ( ((WORD)(*pNewNodePattern)>> (4* yL + xL)) & 0x01)
					{
//						nMin=i-1;
						nMin=i;
//						if (nMin<0)
//							nMin=0;
//						nMax=i+2;
						nMax=i+1;
//						if (nMax>4)
//							nMax=4;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							yR=j;
//							if (Test(*pRootPattern,xR,yR))
							BOOL b;
							b= ((WORD)(*pRootPattern) >> (4* yR+ xR)) & 0x01;
							if (b)
//								if ( ((WORD)(*pRootPattern) >> (4* yR+ xR)) & 0x01)
							{
								newNode->nodeAttr= ((yL+1)<<4)|(xL+1);
								return TRUE;
							}
						}
					}
				}
			}
		}
		else if (root->x==newNode->x)
		{
			if (root->y < newNode->y)//top-bottom
			{
				int xT,yT,xB,yB;
				yT=3;
				yB=0;
				int nMin,nMax;
				for (i=0;i<4 && !bConnect;i++)
				{
					xT=i;
					//whether walkable
//					if (Test(*pRootPattern,xT,yT))
					BOOL b;
					b= ((WORD)(*pRootPattern) >> (4* yT + xT)) & 0x01;
					if (b)
//						if ( ((WORD)(*pRootPattern) >> (4* yT + xT)) & 0x01)
					{
//						nMin=i-1;
//						if (nMin<0)
//							nMin=0;
//						nMax=i+2;
//						if (nMax>4)
//							nMax=4;
						nMin=i;
						nMax=i+1;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							xB=j;
//							if (Test(*pNewNodePattern,xB,yB))
							BOOL b;
							b= ((WORD)(*pNewNodePattern) >> (4* yB + xB)) & 0x01;
							if (b)
//								if ( ((WORD)(*pNewNodePattern) >> (4* yB + xB)) & 0x01)
							{
								newNode->nodeAttr=((yB+1)<<4)|(xB+1);
								return TRUE;
							}
						}
					}
				}
			}
			else
			{
				int xT,yT,xB,yB;
				yT=3;
				yB=0;
				int nMin,nMax;
				for (i=0;i<4 && !bConnect;i++)
				{
					xT=i;
					//whether walkable
//					if (Test(*pNewNodePattern,xT,yT))
					BOOL b;
					b= ((WORD)(*pNewNodePattern) >> (4* yT + xT)) & 0x01;
					if (b)
//						if ( ((WORD)(*pNewNodePattern) >> (4* yT + xT)) & 0x01)
					{
//						nMin=i-1;
//						if (nMin<0)
//							nMin=0;
//						nMax=i+2;
//						if (nMax>4)
//							nMax=4;
						nMin=i;
						nMax=i+1;
						for (j=nMin;j<nMax;j++)
						{
							//whether walkable
							xB=j;
//							if (Test(*pRootPattern,xB,yB))
							BOOL b;
							b= ((WORD)(*pRootPattern) >> (4* yB+ xB)) & 0x01;
							if (b)
//								if ( ((WORD)(*pRootPattern) >> (4* yB + xB)) & 0x01)
							{
								newNode->nodeAttr=((yT+1)<<4)|(xT+1);
								return TRUE;
							}
						}
					}
				}
			}
		}
		else if (root->x>newNode->x && root->y<newNode->y)//root is at the right top of new node
		{
			return FALSE;
			bConnect =( (((*pRootPattern)>>12)&0x01)  &&  (((*pNewNodePattern)>>3)&0x01) );
			if (bConnect)
				newNode->nodeAttr= 20;//((0+1)<<4)|(3+1);
			return bConnect;

		}
		else if (newNode->x>root->x && newNode->y<root->y)//new node is at the right top of root
		{
			return FALSE;
			bConnect= ( (((*pNewNodePattern)>>12)&0x01)  &&  (((*pRootPattern)>>3)&0x01) );
			if (bConnect)
				newNode->nodeAttr=65;//((3+1)<<4)|(0+1);
			return bConnect;
		}
		else if (root->x>newNode->x && root->y>newNode->y)//root is at the right bottom of new node
		{
			return FALSE;
			bConnect=( (((*pRootPattern)>>0)&0x01)  &&  (((*pNewNodePattern)>>15)&0x01) );
			if (bConnect)
				newNode->nodeAttr=68;//((3+1)<<4)|(3+1);
			return bConnect;
		}
		else if (newNode->x>root->x && newNode->y>root->y)//new node is at the right bottom of root
		{
			return FALSE;
			bConnect=( (((*pNewNodePattern)>>0)&0x01)  &&  (((*pRootPattern)>>15)&0x01) );
			if (bConnect)
				newNode->nodeAttr=17;//((0+1)<<4)|(0+1);
			return bConnect;
		}
	}
	else //1 small tile,1 big chip
	{
//		return IsNodeConnect_2(root,newNode,pRootPattern,pNewNodePattern);
		CPoint aOff[8]={CPoint(0,-1),CPoint(-1,0),CPoint(1,0),CPoint(0,1),CPoint(-1,-1),CPoint(1,-1),CPoint(-1,1),CPoint(1,1)};
		i_math::recti rcClip=i_math::recti(0,0,4,4);
		CPoint pt;
		if (pNewNodePattern)
		{
			ASSERT(!pRootPattern);
			for (i=0;i<4;i++)
			{
				pt.x=root->x+aOff[i].x-newNode->x;
				pt.y=root->y+aOff[i].y-newNode->y;
				if (rcClip.PtInRect(pt))
				{ 
					if ( ((*pNewNodePattern) >> (4* pt.y + pt.x)) & 0x01)
					{
						newNode->nodeAttr=((pt.y+1)<<4)|(pt.x+1);
						return TRUE;
					}
				}
			}
		}
		else
		{
			ASSERT(pRootPattern);
			for (i=0;i<4;i++)
			{
				pt.x=newNode->x+aOff[i].x-root->x;
				pt.y=newNode->y+aOff[i].y-root->y;
				if (rcClip.PtInRect(pt))
				{ 
					if ( ((*pRootPattern)>> (4* pt.y + pt.x)) & 0x01)
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}



void MasterNodeList::Reset()
{
	memset(m_aValidInfo,0,m_nBufferLen>>3);
}

void MasterNodeList::Initialize(int nWidth,int nHeight)
{
	m_nBufferLen=nWidth*nHeight;
	m_aNodeBufferIndex=new WORD[m_nBufferLen];		
	m_aValidInfo=new BYTE[m_nBufferLen>>3];
}
void MasterNodeList::Unintialize()
{
	SAFE_DELETE(m_aNodeBufferIndex);
	SAFE_DELETE(m_aValidInfo);
}

void CAStar::SetMaxSearchStep(int nMaxSearchStep)
{
	m_nMaxSearchStep=nMaxSearchStep;
}

WORD g_pPatternMapBack[512000];



//return 0,if no path can be found,and the source point cannot even move near to the target point
//return 1,if no path can be found,but the source point can move a segment of path to the target point
//return 2,if a path can be found between the source and the target
int CAStar::CanFindPath(short xStart,short yStart,short xEnd,short yEnd,int searchstep)
{
	if (xStart==xEnd&& yStart==yEnd)
		return TRUE;
	int i,j;
	int xBigChip,yBigChip;
	m_MasterNodeList.Reset();
	m_FreeNodeBank.Reset();
	m_open.Initialize();

	//Not supporting water-walk now
	if (m_AttrBlocking==ASTAR_TILE_GRACEBLOCK)
		m_pPatternMap=m_pPatternMapGrace;
	else
	{
		if (m_AttrBlocking == ASTAR_TILE_GROUNDBLOCK)
			m_pPatternMap=m_pPatternMapGround;
		else
			m_pPatternMap=m_pPatternMapAir;
	}

//	memcpy(g_pPatternMapBack,m_pPatternMap,m_nBigChipW*m_nBigChipH*sizeof(WORD));

	m_nStartX = xStart;
	m_nStartY = yStart;
	m_nEndX = xEnd;
	m_nEndY = yEnd;

	//Create the very first node and put it on the Open list
	Node* startnode;
	startnode= NewANode( xStart,yStart,0);
	startnode->cost = 0;           //This node has no cost to get to
	startnode->stat = NODE_STAT_OPEN;      //This node is going on the Open list
	startnode->parent = 0;         //This node has no parent
	startnode->total = GET_NODE_HEURISTIC( startnode->x,startnode->y);
	PushOpenQueue(startnode);

	int nApproachStep ;
	nApproachStep=searchstep;
	if(!IsValidFreeTile(m_nEndX ,m_nEndY))
	{
		CPoint pt;
		if(FALSE)//if Can find a nearing not-blocked point (pt) of the point
		{
			m_nEndX=pt.x>>MAP_TILE_WIDTH_SHIFT;
			m_nEndY=pt.y>>MAP_TILE_HEIGHT_SHIFT;
		}
		else
		{
			int dx,dy;
			dx=abs(m_nEndX-m_nStartX);
			dy=abs(m_nEndY-m_nStartY);
			if (dx > dy)
				nApproachStep = dx*MAX_SEARCH_LOOP;
			else
				nApproachStep =dy*MAX_SEARCH_LOOP;
			if (nApproachStep>m_nMaxSearchStep)
				nApproachStep=m_nMaxSearchStep;
		}
	}
	

	int xEndBig,yEndBig,xStartBig,yStartBig;
	xEndBig=(m_nEndX>>2);
	yEndBig=(m_nEndY>>2);
	xStartBig=(m_nStartX>>2);
	yStartBig=(m_nStartY>>2);
	
	Node newnode;
	short xOff[8]={0,0,-1,1,1,1,-1,-1};
	short yOff[8]={-1,1,0,0,-1,1,1,-1};
	int aPower[8]={HIGHRESO_FACTOR,HIGHRESO_FACTOR,HIGHRESO_FACTOR,HIGHRESO_FACTOR,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000};
	short xOffBig1[8][4]={{0,1,2,3},{0,1,2,3},{-1,-1,-1,-1},{4,4,4,4},{-1},{-1},{4},{4}};
	short yOffBig1[8][4]={{-1,-1,-1,-1},{4,4,4,4},{0,1,2,3},{0,1,2,3},{-1},{4},{-1},{4}};
	short xOffBig2[8]={0,0,-4,4,-4,-4,4,4};
	short yOffBig2[8]={-4,4,0,0,-4,4,-4,4};
	int aOffBigLen[8]={4,4,4,4,1,1,1,1};
	int nSearchStep=0;
	Node* root;
	WORD v;
	int x,y;
	CPoint aProducedBigChip[8];
	int nProducedBigChip;
	WORD patternParent,patternChild;
	while( !IsOpenQueueEmpty() )
	{
		
		//Get the best candidate node to search next
		root = PopOpenQueue();

		if (root->nodeAttr)//big chip
		{
			patternParent=~(m_pPatternMap[(root->y>>2)*m_nBigChipW+(root->x>>2)]);

			for (i=0;i<4;i++)
			{
				x=root->x+xOffBig2[i];
				y=root->y+yOffBig2[i];
				xBigChip=x>>2;
				yBigChip=y>>2;
				//is unobstructed
				patternChild=~(m_pPatternMap[yBigChip*m_nBigChipW+xBigChip]);
				BOOL bUnobstructed0=FALSE;
				BOOL bUnobstructed=FALSE;
				if (IS_UNOBSTRUCTED(~patternChild)&&//child is big chip
					((abs(xBigChip-xStartBig)>1)||(abs(yBigChip-yStartBig)>1))//not too close to the start big chip
					&&((xBigChip!=xEndBig)||(yBigChip!=yEndBig)))//,and is not the end big chip
				{
					bUnobstructed0=TRUE;
					newnode.x=xBigChip<<2;
					newnode.y=yBigChip<<2;
					//newnode.nodeAttr=((y-newnode.y+1)<<4) | (x-newnode.x+1);//high 4 bit-y,low 4 bit-x,1-4
					//This avoids searching the node we just came from
					if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
						continue;
					if (IsNodeConnect(root,&newnode,&patternParent,&patternChild))
					{
						bUnobstructed=TRUE;
						if (newnode.x!=root->x&&newnode.y!=root->y)
							newnode.cost=root->cost+1414*HIGHRESO_FACTOR/1000;//1.414*HIGHRESO_FACTOR
						else
							newnode.cost=root->cost+HIGHRESO_FACTOR;

						Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
						if (actualnode)//ԭ���������
						{
							if (actualnode->stat!=NODE_STAT_OPEN)
								continue;
							if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
								continue;
							else
								actualnode->nodeAttr=newnode.nodeAttr;
						}
						else
							actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);

						//This node is very promising
						//Take it off the Open and Closed lists (in theory) and push on Open
						actualnode->parent = root;
						actualnode->cost = newnode.cost;
//						actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
						actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
						
						if( actualnode->stat==NODE_STAT_OPEN )
						{  //Since this node is already on the Open list, update it's position
						   UpdateNodeOnOpenQueue( actualnode );
						}
						else
						{  //Put the node on the Open list
						   actualnode->stat= NODE_STAT_OPEN;
						   PushOpenQueue(actualnode);
						}
					}
				}
				if (!bUnobstructed0)//small tile
				{
					for (j=0;j<aOffBigLen[i];j++)
					{
						newnode.x=root->x+xOffBig1[i][j];
						newnode.y=root->y+yOffBig1[i][j];
						newnode.nodeAttr=0;
						//This avoids searching the node we just came from
						if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
							continue;
						if ((newnode.x<0)||(newnode.y<0)||(newnode.x>=m_nMapWidth)||(newnode.y>=m_nMapHeight))//Out of the boundary
							continue;
						Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
						
						if (actualnode)
						{
							if (actualnode->stat==NODE_STAT_BLOCK)
								continue;
							if (actualnode->nodeAttr!=0)//Already a big tile,must blocked here
								continue;
						}
						else
						{
							v=m_pSearchMap[m_nMapWidth * newnode.y + newnode.x];
							if ( v& m_AttrBlocking)
							{
								actualnode=NewANode(newnode.x,newnode.y,0);
								actualnode->stat=NODE_STAT_BLOCK;
								continue;
							}

						}

						//whether connect with root,if root is a big chip
						if (!IsNodeConnect(root,&newnode,&patternParent))
							continue;
						
						if (TRUE)//
						{
							WORD v;
							int xBig,yBig;
							xBig=newnode.x>>2;
							yBig=newnode.y>>2;
							if (((abs(xBig-xStartBig)>1)||(abs(yBig-yStartBig)>1))
								&&((xBig!=xEndBig)||(yBig!=yEndBig)))
							{
								v=m_pPatternMap[(yBig)*m_nBigChipW+xBig];
								ASSERT(!(IS_UNOBSTRUCTED(v)));
							}
						}
						newnode.cost=root->cost+HIGHRESO_FACTOR;
						if (actualnode)//ԭ���������
						{
							ASSERT(actualnode->stat!=NODE_STAT_UNDEFINE);
							if (actualnode->stat!=NODE_STAT_OPEN)
								continue;
							if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
								continue;
						}
						else
							actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);
						//This node is very promising
						//Take it off the Open and Closed lists (in theory) and push on Open
						actualnode->parent = root;
						actualnode->cost = newnode.cost;
//						actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
						actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
						
						if( actualnode->stat==NODE_STAT_OPEN )
						{  //Since this node is already on the Open list, update it's position
						   UpdateNodeOnOpenQueue( actualnode );
						}
						else
						{  //Put the node on the Open list
						   actualnode->stat= NODE_STAT_OPEN;
						   PushOpenQueue(actualnode);
						}
					}
				}
			}
		}
		else//root is small chip
		{
			if( root->x==m_nEndX && root->y==m_nEndY )
			{   //Found the goal node - construct a path and exit
				//The complete path will be stored inside the game object
				return 2;
			}
			//	try childs of this node in eight directions
			//	push appropriate childs in open queue

			nProducedBigChip=0;
			for (i=3;i>=0;i--)
			{
				x=root->x+xOff[i];
				y=root->y+yOff[i];
				xBigChip=x>>2;
				yBigChip=y>>2;
				//is unobstructed
				patternChild=~(m_pPatternMap[yBigChip*m_nBigChipW+xBigChip]);
				BOOL bBigChip;
				if (IS_UNOBSTRUCTED(~patternChild)&&//child is big chip
					((abs(xBigChip-xStartBig)>1)||(abs(yBigChip-yStartBig)>1))//not too close to the start big chip
					&&((xBigChip!=xEndBig)||(yBigChip!=yEndBig)))//,and is not the end big chip
				{
					bBigChip=TRUE;
					BOOL bProduced=FALSE;
					CPoint* pProducedChip=aProducedBigChip;
					for (j=0;j<nProducedBigChip;j++)
					{
						if (pProducedChip->x==xBigChip && pProducedChip->y==yBigChip)
						{
							bProduced=TRUE;
							break;
						}
						pProducedChip++;
					}
					if (bProduced)
						continue;

					ASSERT((xBigChip!=(root->x>>2))||(yBigChip!=(root->y>>2)));
					newnode.x=xBigChip<<2;
					newnode.y=yBigChip<<2;
					//newnode.nodeAttr=((y-newnode.y+1)<<4) | (x-newnode.x+1);//high 4 bit-y,low 4 bit-x,1-4
				}
				else
				{
					bBigChip=FALSE;
					newnode.x=x;
					newnode.y=y;
					newnode.nodeAttr=0;
				}
				//This avoids searching the node we just came from
				if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
					continue;

				Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
				if (!bBigChip)
				{
					if (actualnode)
					{
						if (actualnode->stat==NODE_STAT_BLOCK)
							continue;
						ASSERT(actualnode->nodeAttr==0);// must be a small tile
					}
					else
					{
						v=m_pSearchMap[m_nMapWidth * newnode.y + newnode.x];
						if ( v& m_AttrBlocking)
						{
							actualnode=NewANode(newnode.x,newnode.y,0);
							actualnode->stat=NODE_STAT_BLOCK;
							continue;
						}
					}
				}
				else//unobstructed chip
				{
					if (!IsNodeConnect(root,&newnode,NULL,&patternChild))
						continue;

					aProducedBigChip[nProducedBigChip].x=xBigChip;
					aProducedBigChip[nProducedBigChip++].y=yBigChip;
					ASSERT(nProducedBigChip<=8);
				}

				newnode.cost = root->cost+aPower[i];

				if (actualnode)//ԭ���������
				{
					ASSERT(actualnode->stat!=NODE_STAT_UNDEFINE);
					if (actualnode->stat!=NODE_STAT_OPEN)
						continue;
					if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
						continue;
					else
						actualnode->nodeAttr=newnode.nodeAttr;
				}
				else
					actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);

				//This node is very promising
				//Take it off the Open and Closed lists (in theory) and push on Open
				actualnode->parent = root;
				actualnode->cost = newnode.cost;
//				actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
				actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
				
				if( actualnode->stat==NODE_STAT_OPEN )
				{  //Since this node is already on the Open list, update it's position
				   UpdateNodeOnOpenQueue( actualnode );
				}
				else
				{  //Put the node on the Open list
				   actualnode->stat= NODE_STAT_OPEN;
				   PushOpenQueue(actualnode);
				}
			}
		}

	    //Now that we've explored root, put it on the Closed list
	    root->stat = NODE_STAT_CLOSE;
		
		//Use some method to determine if we've taken too much time
        //this tick and should abort the search until next tick
		//	Break search if out of maximum search steps		
		//	����������������������˳�ѭ��
		nSearchStep++;
		//	���Ԥ�������������˳�����
		if(nSearchStep > nApproachStep)
		{
//			AfxMessageBox("search uncompletely!");
			break;
		}
	}


	//���û���ҵ�Ŀ�ĵ�����һ����Ŀ�ĵ�����ĵ�
	if (root->nodeAttr||root->x!=m_nEndX||root->y!=m_nEndY)//if root is a big chip,it could not be the end.
	{
		int disMin,dis;
		disMin=root->total-root->cost;
		Node* pBest=root;
		
		Node* p;
		for (i=0;i<m_FreeNodeBank.m_nCurBlock;i++)
		{
			p=&(m_FreeNodeBank.m_aNodeBuffer[i][0]);
			for (j=0;j<MAX_NODE_BLOCK_LEN;j++)
			{
				ASSERT(p->stat!=NODE_STAT_UNDEFINE);
				if (p->stat==NODE_STAT_BLOCK)
				{
					p++;
					continue;
				}
				dis=p->total-p->cost;
				if (dis<disMin)
				{
					disMin=dis;
					pBest=p;
				}
				p++;
			}
		}
		p=&(m_FreeNodeBank.m_aNodeBuffer[m_FreeNodeBank.m_nCurBlock][0]);
		while(p<m_FreeNodeBank.m_pBottom)
		{
			ASSERT(p->stat!=NODE_STAT_UNDEFINE);
			if (p->stat==NODE_STAT_BLOCK)
			{
				p++;
				continue;
			}
			dis=p->total-p->cost;
			if (dis<disMin)
			{
				disMin=dis;
				pBest=p;
			}
			p++;
		}
		root=pBest;
	}
	
	
	Node* pRoot;
	//if the end is a big chip,we should take a small tile from it ,and make this small tile the end
	if (root->nodeAttr)		
	{
		ASSERT(((root->x>>2)!=xEndBig)||((root->y>>2)!=yEndBig));
		x=root->x+(root->nodeAttr & 0x0f)-1;
		y=root->y+((root->nodeAttr>>4)&0x0f)-1;
		pRoot=GetNodeFromMasterNodeList(x,y);
		if (!pRoot)
			pRoot=NewANode(x,y,0);
		else
			pRoot->nodeAttr=0;
		pRoot->parent=root->parent;
		root=pRoot;
	}
	
	
	pRoot=root;

	if (DIST2(pRoot->x-xStart,pRoot->y-yStart)<CLOSE_THRESHHOLD*CLOSE_THRESHHOLD)//Too close,seem cannot move on any more
		return 0;
	

	return 1;
}

//#define _OUTPUT_ASTAR_MAP

bool CAStar::FindPath(short xStart, short yStart, short &xEnd,short &yEnd,i_math::recti &rcNextFind)
{ 
	if (xStart==yStart && xEnd==yEnd)
		return FALSE;

//	if (EasyFindPath(xStart,yStart,xEnd,yEnd))
//		return TRUE;
	int i,j;
	int xBigChip,yBigChip;
	m_MasterNodeList.Reset();
	m_FreeNodeBank.Reset();
	m_open.Initialize();

	//Not supporting water-walk now
	if (m_AttrBlocking==ASTAR_TILE_GRACEBLOCK)
		m_pPatternMap=m_pPatternMapGrace;
	else
	{
		if (m_AttrBlocking == ASTAR_TILE_GROUNDBLOCK)
			m_pPatternMap=m_pPatternMapGround;
		else
			m_pPatternMap=m_pPatternMapAir;
	}
	
//	memcpy(g_pPatternMapBack,m_pPatternMap,m_nBigChipW*m_nBigChipH*sizeof(WORD));

	m_nStartX = xStart;
	m_nStartY = yStart;
	m_nEndX = xEnd;
	m_nEndY = yEnd;

	//Create the very first node and put it on the Open list
	Node* startnode;
	startnode= NewANode( xStart,yStart,0);
	startnode->cost = 0;           //This node has no cost to get to
	startnode->stat = NODE_STAT_OPEN;      //This node is going on the Open list
	startnode->parent = 0;         //This node has no parent
	startnode->total = GET_NODE_HEURISTIC( startnode->x,startnode->y);
	PushOpenQueue(startnode);

	int nApproachStep ;
	nApproachStep=m_nMaxSearchStep;
	if(!IsValidFreeTile(m_nEndX ,m_nEndY))
	{
		CPoint pt;
		if(FALSE)//if Can find a nearing not-blocked point (pt) of the point
		{
			m_nEndX=pt.x>>MAP_TILE_WIDTH_SHIFT;
			m_nEndY=pt.y>>MAP_TILE_HEIGHT_SHIFT;
		}
		else
		{
			int dx,dy;
			dx=abs(m_nEndX-m_nStartX);
			dy=abs(m_nEndY-m_nStartY);
			if (dx > dy)
				nApproachStep = dx*MAX_SEARCH_LOOP;
			else
				nApproachStep =dy*MAX_SEARCH_LOOP;
			if (nApproachStep>m_nMaxSearchStep)
				nApproachStep=m_nMaxSearchStep;
		}
	}
	

	int xEndBig,yEndBig,xStartBig,yStartBig;
	xEndBig=(m_nEndX>>2);
	yEndBig=(m_nEndY>>2);
	xStartBig=(m_nStartX>>2);
	yStartBig=(m_nStartY>>2);
	
	Node newnode;
	short xOff[8]={0,0,-1,1,1,1,-1,-1};
	short yOff[8]={-1,1,0,0,-1,1,1,-1};
	int aPower[8]={HIGHRESO_FACTOR,HIGHRESO_FACTOR,HIGHRESO_FACTOR,HIGHRESO_FACTOR,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000,1414*HIGHRESO_FACTOR/1000};
	short xOffBig1[8][4]={{0,1,2,3},{0,1,2,3},{-1,-1,-1,-1},{4,4,4,4},{-1},{-1},{4},{4}};
	short yOffBig1[8][4]={{-1,-1,-1,-1},{4,4,4,4},{0,1,2,3},{0,1,2,3},{-1},{4},{-1},{4}};
	short xOffBig2[8]={0,0,-4,4,-4,-4,4,4};
	short yOffBig2[8]={-4,4,0,0,-4,4,-4,4};
	int aOffBigLen[8]={4,4,4,4,1,1,1,1};
	int nSearchStep=0;
	Node* root;
	WORD v;
	int x,y;
	CPoint aProducedBigChip[8];
	int nProducedBigChip;
	WORD patternParent,patternChild;
	while( !IsOpenQueueEmpty() )
	{
		
		//Get the best candidate node to search next
		root = PopOpenQueue();

		if (root->nodeAttr)//big chip
		{ 
			patternParent=~(m_pPatternMap[(root->y>>2)*m_nBigChipW+(root->x>>2)]);

			for (i=0;i<4;i++)
			{
				x=root->x+xOffBig2[i];
				y=root->y+yOffBig2[i];
				xBigChip=x>>2;
				yBigChip=y>>2;
				//is unobstructed
				patternChild=~(m_pPatternMap[yBigChip*m_nBigChipW+xBigChip]);
				BOOL bUnobstructed0=FALSE;
				BOOL bUnobstructed=FALSE;
				if (IS_UNOBSTRUCTED(~patternChild)&&//child is big chip
					((abs(xBigChip-xStartBig)>1)||(abs(yBigChip-yStartBig)>1))//not too close to the start big chip
					&&((xBigChip!=xEndBig)||(yBigChip!=yEndBig)))//,and is not the end big chip
				{
					bUnobstructed0=TRUE;
					newnode.x=xBigChip<<2;
					newnode.y=yBigChip<<2;
					//newnode.nodeAttr=((y-newnode.y+1)<<4) | (x-newnode.x+1);//high 4 bit-y,low 4 bit-x,1-4
					//This avoids searching the node we just came from
					if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
						continue;
					if (IsNodeConnect(root,&newnode,&patternParent,&patternChild))
					{
						bUnobstructed=TRUE;
						if (newnode.x!=root->x&&newnode.y!=root->y)
							newnode.cost=root->cost+92668;//1.414*HIGHRESO_FACTOR
						else
							newnode.cost=root->cost+HIGHRESO_FACTOR;

						Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
						if (actualnode)//ԭ���������
						{
							if (actualnode->stat!=NODE_STAT_OPEN)
								continue;
							if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
								continue;
							else
								actualnode->nodeAttr=newnode.nodeAttr;
						}
						else
							actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);

						//This node is very promising
						//Take it off the Open and Closed lists (in theory) and push on Open
						actualnode->parent = root;
						actualnode->cost = newnode.cost;
//						actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
						actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
						
						if( actualnode->stat==NODE_STAT_OPEN )
						{  //Since this node is already on the Open list, update it's position
						   UpdateNodeOnOpenQueue( actualnode );
						}
						else
						{  //Put the node on the Open list
						   actualnode->stat= NODE_STAT_OPEN;
						   PushOpenQueue(actualnode);
						}
					}
				}
				if (!bUnobstructed0)//small tile
				{
					for (j=0;j<aOffBigLen[i];j++)
					{
						newnode.x=root->x+xOffBig1[i][j];
						newnode.y=root->y+yOffBig1[i][j];
						newnode.nodeAttr=0;
						//This avoids searching the node we just came from
						if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
							continue;
						if ((newnode.x<0)||(newnode.y<0)||(newnode.x>=m_nMapWidth)||(newnode.y>=m_nMapHeight))//Out of the boundary
							continue;
						Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
						
						if (actualnode)
						{
							if (actualnode->stat==NODE_STAT_BLOCK)
								continue;
							if (actualnode->nodeAttr!=0)//Already a big tile,must blocked here
								continue;
						}
						else
						{
							v=m_pSearchMap[m_nMapWidth * newnode.y + newnode.x];
							if ( v& m_AttrBlocking)
							{
								actualnode=NewANode(newnode.x,newnode.y,0);
								actualnode->stat=NODE_STAT_BLOCK;
								continue;
							}

						}

						//whether connect with root,if root is a big chip
						if (!IsNodeConnect(root,&newnode,&patternParent))
							continue;
						
						if (TRUE)//
						{
							WORD v;
							int xBig,yBig;
							xBig=newnode.x>>2;
							yBig=newnode.y>>2;
							if (((abs(xBig-xStartBig)>1)||(abs(yBig-yStartBig)>1))
								&&((xBig!=xEndBig)||(yBig!=yEndBig)))
							{
								v=m_pPatternMap[(yBig)*m_nBigChipW+xBig];
								if ((IS_UNOBSTRUCTED(v)))
								{
									CString s;
									s.Format("%d,%d--%d,%d",xStart,yStart,xEnd,yEnd);
									AfxMessageBox(s);
								}

								ASSERT(!(IS_UNOBSTRUCTED(v)));
							}
						}
						newnode.cost=root->cost+HIGHRESO_FACTOR;
						if (actualnode)//ԭ���������
						{
							ASSERT(actualnode->stat!=NODE_STAT_UNDEFINE);
							if (actualnode->stat!=NODE_STAT_OPEN)
								continue;
							if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
								continue;
						}
						else
							actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);
						//This node is very promising
						//Take it off the Open and Closed lists (in theory) and push on Open
						actualnode->parent = root;
						actualnode->cost = newnode.cost;
//						actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
						actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
						
						if( actualnode->stat==NODE_STAT_OPEN )
						{  //Since this node is already on the Open list, update it's position
						   UpdateNodeOnOpenQueue( actualnode );
						}
						else
						{  //Put the node on the Open list
						   actualnode->stat= NODE_STAT_OPEN;
						   PushOpenQueue(actualnode);
						}
					}
				}
			}
		}
		else//root is small chip
		{
			if( root->x==m_nEndX && root->y==m_nEndY )
			{   //Found the goal node - construct a path and exit
				//The complete path will be stored inside the game object
				break;
			}
			//	try childs of this node in eight directions
			//	push appropriate childs in open queue

			nProducedBigChip=0;
			for (i=3;i>=0;i--)
			{
				x=root->x+xOff[i];
				y=root->y+yOff[i];
				xBigChip=x>>2;
				yBigChip=y>>2;
				//is unobstructed
				patternChild=~(m_pPatternMap[yBigChip*m_nBigChipW+xBigChip]);
				BOOL bBigChip;
				if (IS_UNOBSTRUCTED(~patternChild)&&//child is big chip
					((abs(xBigChip-xStartBig)>1)||(abs(yBigChip-yStartBig)>1))//not too close to the start big chip
					&&((xBigChip!=xEndBig)||(yBigChip!=yEndBig)))//,and is not the end big chip
				{
					bBigChip=TRUE;
					BOOL bProduced=FALSE;
					CPoint* pProducedChip=aProducedBigChip;
					for (j=0;j<nProducedBigChip;j++)
					{
						if (pProducedChip->x==xBigChip && pProducedChip->y==yBigChip)
						{
							bProduced=TRUE;
							break;
						}
						pProducedChip++;
					}
					if (bProduced)
						continue;

					ASSERT((xBigChip!=(root->x>>2))||(yBigChip!=(root->y>>2)));
					if (!((xBigChip!=(root->x>>2))||(yBigChip!=(root->y>>2))))
					{
						CString s;
						s.Format("1424:%d,%d--%d,%d--%d,%d",xBigChip,yBigChip,xStartBig,yStartBig,root->x,root->y);
						AfxMessageBox(s);
					}
					newnode.x=xBigChip<<2; 
					newnode.y=yBigChip<<2;
					//newnode.nodeAttr=((y-newnode.y+1)<<4) | (x-newnode.x+1);//high 4 bit-y,low 4 bit-x,1-4
				}
				else
				{
					bBigChip=FALSE;
					newnode.x=x;
					newnode.y=y;
					newnode.nodeAttr=0;
				}
				//This avoids searching the node we just came from
				if (root->parent && root->parent->x==newnode.x && root->parent->y==newnode.y)
					continue;

				Node* actualnode = GetNodeFromMasterNodeList(newnode.x,newnode.y );
				if (!bBigChip)
				{
					if (actualnode)
					{
						if (actualnode->stat==NODE_STAT_BLOCK)
							continue;
						ASSERT(actualnode->nodeAttr==0);// must be a small tile
					}
					else
					{
						v=m_pSearchMap[m_nMapWidth * newnode.y + newnode.x];
						if ( v& m_AttrBlocking)
						{
							actualnode=NewANode(newnode.x,newnode.y,0);
							actualnode->stat=NODE_STAT_BLOCK;
							continue;
						}
					}
				} 
				else//unobstructed chip
				{
					if (!IsNodeConnect(root,&newnode,NULL,&patternChild))
						continue;

					aProducedBigChip[nProducedBigChip].x=xBigChip;
					aProducedBigChip[nProducedBigChip++].y=yBigChip;
					ASSERT(nProducedBigChip<=8);
				}

				newnode.cost = root->cost+aPower[i];

				if (actualnode)//ԭ���������
				{
					ASSERT(actualnode->stat!=NODE_STAT_UNDEFINE);
					if (actualnode->stat!=NODE_STAT_OPEN)
						continue;
					if ( !(actualnode->cost > newnode.cost))//ԭ����ȨֵС����ڵ�
						continue;
					else
						actualnode->nodeAttr=newnode.nodeAttr;
				}
				else
					actualnode = NewANode(newnode.x,newnode.y,newnode.nodeAttr);

				//This node is very promising
				//Take it off the Open and Closed lists (in theory) and push on Open
				actualnode->parent = root;
				actualnode->cost = newnode.cost;
//				actualnode->total = newnode.cost+GET_NODE_HEURISTIC(newnode.x,newnode.y);
				actualnode->total = newnode.cost+(HIGHRESO_FACTOR*(3*abs(m_nEndX-newnode.x)+3*abs(m_nEndY-newnode.y))+GET_NODE_HEURISTIC(newnode.x,newnode.y))/4;
				
				if( actualnode->stat==NODE_STAT_OPEN )
				{  //Since this node is already on the Open list, update it's position
				   UpdateNodeOnOpenQueue( actualnode );
				}
				else
				{  //Put the node on the Open list
				   actualnode->stat= NODE_STAT_OPEN;
				   PushOpenQueue(actualnode);
				}
			}
		}

	    //Now that we've explored root, put it on the Closed list
	    root->stat = NODE_STAT_CLOSE;
		
		//Use some method to determine if we've taken too much time
        //this tick and should abort the search until next tick
		//	Break search if out of maximum search steps		
		//	����������������������˳�ѭ��
		nSearchStep++;
		//	���Ԥ�������������˳�����
		if(nSearchStep > nApproachStep)
		{
//			AfxMessageBox("search uncompletely!");
			break;
		}
	}

#ifdef _OUTPUT_ASTAR_MAP
	g_rle44AStar.Create(m_nMapWidth,m_nMapHeight,1);
#endif


	//���û���ҵ�Ŀ�ĵ�����һ����Ŀ�ĵ�����ĵ�
	if (root->nodeAttr||root->x!=m_nEndX||root->y!=m_nEndY)//if root is a big chip,it could not be the end.
	{
		int disMin,dis;
		disMin=root->total-root->cost;
		Node* pBest=root;

		Node* p;
		for (i=0;i<m_FreeNodeBank.m_nCurBlock;i++)
		{
			p=&(m_FreeNodeBank.m_aNodeBuffer[i][0]);
			for (j=0;j<MAX_NODE_BLOCK_LEN;j++)
			{
				ASSERT(p->stat!=NODE_STAT_UNDEFINE);
				if (p->stat==NODE_STAT_BLOCK)
				{
					p++;
					continue;
				}
#ifdef _OUTPUT_ASTAR_MAP
				g_rle44AStar.SetPixel(0,p->x,p->y);
#endif
				dis=p->total-p->cost;
				if (dis<disMin)
				{
					disMin=dis;
					pBest=p;
				}
				p++;
			}
		}
		p=&(m_FreeNodeBank.m_aNodeBuffer[m_FreeNodeBank.m_nCurBlock][0]);
		while(p<m_FreeNodeBank.m_pBottom)
		{
			ASSERT(p->stat!=NODE_STAT_UNDEFINE);
			if (p->stat==NODE_STAT_BLOCK)
			{
				p++;
				continue;
			}
#ifdef _OUTPUT_ASTAR_MAP
			g_rle44AStar.SetPixel(0,p->x,p->y);
#endif
			dis=p->total-p->cost;
			if (dis<disMin)
			{
				disMin=dis;
				pBest=p;
			}
			p++;
		}
		root=pBest;
	}

#ifdef _OUTPUT_ASTAR_MAP
	CGBitmap bmp;
	g_rle44AStar.SaveBmp(bmp);
	bmp.ConvertToTrueColor();
	bmp.Save("path.bmp");
#endif
	

	Node* pRoot;
	//if the end is a big chip,we should take a small tile from it ,and make this small tile the end
	if (root->nodeAttr)		
	{
		ASSERT(((root->x>>2)!=xEndBig)||((root->y>>2)!=yEndBig));
		x=root->x+(root->nodeAttr & 0x0f)-1;
		y=root->y+((root->nodeAttr>>4)&0x0f)-1;
		pRoot=GetNodeFromMasterNodeList(x,y);
		if (!pRoot)
			pRoot=NewANode(x,y,0);
		else
			pRoot->nodeAttr=0;
		pRoot->parent=root->parent;
		root=pRoot;
	}


	pRoot=root;

	m_nTotalStep=0;
	//Record all the nodes
	while (pRoot)
	{
		m_pPathWork[m_nTotalStep]=pRoot;
		m_nTotalStep++;
		pRoot=pRoot->parent;
	}
	
	//Now reverse check every node, until find a node exceed the rcNextFind limit
	rcNextFind.SetRect(xStart,yStart,xStart,yStart);
	Node *pNextFind;
	pNextFind=NULL;
	for (i=m_nTotalStep-1;i>=0;i--)
	{
		Node *p;
		p=m_pPathWork[i];
		if (p->nodeAttr)//A big tile
		{
			if (p->x<rcNextFind.left)
				rcNextFind.left=p->x;
			if (p->y<rcNextFind.top)
				rcNextFind.top=p->y;
			if (p->x+4>rcNextFind.right)
				rcNextFind.right=p->x+4;
			if (p->y+4>rcNextFind.bottom)
				rcNextFind.bottom=p->y+4;
		}
		else
		{
			if (p->x<rcNextFind.left)
				rcNextFind.left=p->x;
			if (p->y<rcNextFind.top)
				rcNextFind.top=p->y; 
			if (p->x>rcNextFind.right)
				rcNextFind.right=p->x;
			if (p->y>rcNextFind.bottom)
				rcNextFind.bottom=p->y;
		}

		if (rcNextFind.Width()*rcNextFind.Height()>NEXTFIND_RECT_SIZELIMIT)
			break;

		pNextFind=p;
	}

	if (!pNextFind)
		return FALSE;

	if (pNextFind->nodeAttr)
	{
		CPoint aBound[12]=
		{
			CPoint(0,0),CPoint(1,0),CPoint(2,0),CPoint(3,0),
				CPoint(0,1),CPoint(3,1),
				CPoint(0,2),CPoint(3,2),
				CPoint(0,3),CPoint(1,3),CPoint(2,3),CPoint(3,3)
		};

		int x,y;

		int dist2;
		dist2=0x7fffffff;

		//Find the nearest one
		for (i=0;i<12;i++)
		{
			x=pNextFind->x+aBound[i].x;
			y=pNextFind->y+aBound[i].y;

			if (IsValidFreeTile(x,y))
			{
				int dist2Test;
				dist2Test= (x-xStart)*(x-xStart)+(y-yStart)*(y-yStart);

				if (dist2Test<dist2)
				{
					xEnd=x;
					yEnd=y;
					dist2=dist2Test;
				}
			}
		}
		ASSERT(dist2<0x7fffffff);
	}
	else
	{
		xEnd=pNextFind->x;
		yEnd=pNextFind->y;
	}
		
	return TRUE;
}



#define SAMPLE_GAP 2


BOOL CAStar::IsValidFreeTile(short x,short y)
{
	DWORD v;
	v=m_pSearchMap[m_nMapWidth * y + x];
	return !( v& m_AttrBlocking); 
}

BYTE *CalculateBigTileConnectionTable()
{
	return NULL;
}


BYTE *CalculateBigTileConnectionTable2()
{
	BYTE *pResult;
	pResult=new BYTE[65536];

	CPoint aBound[12]=
	{
		CPoint(0,0),CPoint(1,0),CPoint(2,0),CPoint(3,0),
		CPoint(0,1),CPoint(3,1),
		CPoint(0,2),CPoint(3,2),
		CPoint(0,3),CPoint(1,3),CPoint(2,3),CPoint(3,3)
	};

	int k;
	for (k=0;k<65536;k++)
	{
		WORD pattern;
		pattern=k;

		//now we have (x1,y1) and (x2,y2) ,find a path connecting them
		int m,n;
		for (m=0;m<12;m++)
		for (n=0;n<12;n++)
		{
			int x1,y1,x2,y2;
			x1=aBound[m].x;
			y1=aBound[m].y;
			x2=aBound[n].x;
			y2=aBound[n].y;

			if  (pattern&PATTERN_BIT(x1,y1))
				continue;
			if  (pattern&PATTERN_BIT(x2,y2))
				continue;
			
			int dx,dy;
			int fDx,fDy;
			int x,y;
			CPoint path[32];
			int nSteps;
			nSteps=0;
			WORD wDirty;
			wDirty=0;//All clean
			BOOL bFound;
			bFound=FALSE;
			while(1)
			{
				path[nSteps].x=x1;
				path[nSteps].y=y1;
				nSteps++;
				if ((x1==x2)&&(y1==y2))
				{
					bFound=TRUE;
					break;
				}
				dx=x2-x1;
				dy=y2-y1;
				if (dx!=0)
					fDx=dx/abs(dx);
				else
					fDx=0;
				if (dy!=0)
					fDy=dy/abs(dy);
				else
					fDy=0;
				
				CPoint *pPreferPoints;
				pPreferPoints=&g_aPreferPoints[(fDy+1)*3+(fDx+1)][0];
				
				int i;
				WORD v;
				for (i=0;i<4;i++)
				{
					x=x1+pPreferPoints[i].x;
					y=y1+pPreferPoints[i].y;
					if (!INSIDE_PATTERN(x,y))
						continue;
					v=PATTERN_BIT(x,y);
					
					if (v&wDirty)//Already go here before
						continue;
					if (pattern&v)//Blocked
						continue;
					
					//Move here
					x1=x;
					y1=y;
					wDirty|=v;
					break;
				}
				
				if (i>=4)
					break;//Not found
			}	

			if (!bFound)
			{
				pResult[k]=0;//not connective
				goto out;
			}
		}

		pResult[k]=BIGTILE_CONNECT_ALLCONNECT;//connective

out:

		//Now the vertical connection check(up and down)
		if (TRUE)
		{
			BOOL bConnected;
			bConnected=FALSE;
			int i;
			for (i=0;i<4;i++)
			{
				if (!(INSIDE_PATTERN(i,0)))
					continue;
				if (pattern&PATTERN_BIT(i,0))//Blocked
					continue;
				if (INSIDE_PATTERN(i-1,3))
				{
					if (!(pattern&PATTERN_BIT(i-1,3)))//Walkable
					{
						bConnected=TRUE;
						break;
					}
				}

				if (INSIDE_PATTERN(i,3))
				{
					if (!(pattern&PATTERN_BIT(i,3)))//Walkable
					{
						bConnected=TRUE;
						break;
					}
				}
				
				if (INSIDE_PATTERN(i+1,3))
				{
					if (!(pattern&PATTERN_BIT(i+1,3)))//Walkable
					{
						bConnected=TRUE;
						break;
					}
				}
			}

			if (bConnected)
				pResult[k]|=BIGTILE_CONNECT_VERTICAL;
		}


		//Now the horizontal connection check(left and right)
		if (TRUE)
		{
			BOOL bConnected;
			bConnected=FALSE;
			int i;
			for (i=0;i<4;i++)
			{
				if (!(INSIDE_PATTERN(0,i)))
					continue;
				if (pattern&PATTERN_BIT(0,i))//Blocked
					continue;
				if (INSIDE_PATTERN(3,i-1))
				{
					if (!(pattern&PATTERN_BIT(3,i-1)))//Walkable
					{
						bConnected=TRUE;
						break;
					}
				}
				
				if (INSIDE_PATTERN(3,i))
				{
					if (!(pattern&PATTERN_BIT(3,i)))//Walkable
					{
						bConnected=TRUE;
						break; 
					}
				}
				
				if (INSIDE_PATTERN(3,i+1))
				{
					if (!(pattern&PATTERN_BIT(3,i+1)))//Walkable
					{
						bConnected=TRUE;
						break;
					}
				}
			}
			
			if (bConnected)
				pResult[k]|=BIGTILE_CONNECT_HORIZONTAL;
		}
		
	}

	return pResult;
}


/*
int CAStar::GetNodeHeuristic(short x,short y)
{
	if (TRUE)
	{
		int nDistX,nDistY,dis;
		int i,nLoop;
		nDistX = abs(m_nEndX - x);
		nDistY = abs(m_nEndY - y);
		dis=m_pDisMap[nDistY*m_nMapWidth+nDistX];
		//		if (!g_bSecondWay)
		return dis;
		nLoop=dis/SAMPLE_GAP;
		if (nLoop==0)
			return dis;
		
		WORD v;
		int xUnit,yUnit;
		xUnit=(m_nEndX - x)/nLoop;
		yUnit=(m_nEndY - y)/nLoop;
		int curX,curY;
		curX=x;
		curY=y;
		for (i=1;i<nLoop;i++)
		{
			v=m_pSearchMap[curY*m_nMapWidth+curX];
			if( v& m_AttrBlocking)
			{
				return dis + 50*(1-(float)(i*SAMPLE_GAP)/(float)dis);
			}
			curX+=xUnit;
			curY+=yUnit;
		}
		return dis;
	}
	else
	{
		//	Heuristic function
		//	���ۺ���
		//		int nDistX = m_nEndX - x;
		//		int nDistY = m_nEndY - y;
		//
		//		return (float)sqrtf(nDistX * nDistX + nDistY * nDistY);
	}
	
	ASSERT(FALSE);
	return 0;
}
*/