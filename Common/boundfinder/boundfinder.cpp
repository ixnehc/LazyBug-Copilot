/********************************************************************
	created:	2007/10/10   17:00
	filename: 	E:\IxEngine\Common\boundconstructor\BoundFinder.cpp
	author:		cxi
	
	purpose:	a tool class to find boundary for a mono picture
*********************************************************************/

#include "stdh.h"
#include "BoundFinder.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL CBoundFinder::Find(unsigned char *pData,int w,int h)
{
	_poolStripe.Reset();
	if (pData==NULL)
		return FALSE;

	_h=h;
	if (TRUE)
	{ // fill the boundary with transparent color .
		BYTE trans;
		trans=*pData;
		BYTE *p;
		p=pData;
		int i;
		for (i=0;i<w;i++)
		{
			*p=trans;
			p++;
		}
		for (i=1;i<h-1;i++)
		{
			*p=trans;
			p+=w-1;
			*p=trans;
			p++;
		}
		for (i=0;i<w;i++)
		{
			*p=trans;
			p++;
		}
	}
	if (!(ConstructPCS(pData,w,h)))
		return FALSE;
	PCStoChain(h);
	RemovePCS();
	int n,m,nPoints;
	nPoints=_poolBN.GetCount();
	n=0;
	CBound *pBoundary;
	CBoundNode *pNode;
	pBoundary=_bounds;
	Unique(pBoundary);
	while(pBoundary)
	{
		n++;
		pBoundary=pBoundary->pNext;
	}
	m_points.resize(nPoints+n+10);
	m_contourend.resize(n);
	n=0;
	m=0;
	pBoundary=_bounds;
	while (pBoundary)
	{ 
		pNode=pBoundary->pStartNode;
		if (pNode==NULL)
		{
			pBoundary=pBoundary->pNext;
			continue;
		}
		int x,y;
		x=pBoundary->x;
		y=pBoundary->y;
		m_points[n]=i_math::pos2di(x,y);
		n++;
		while (pNode)
		{
			switch(pNode->dir)
			{
				case BD_LEFT:
					x-=pNode->dist;
					break;
				case BD_RIGHT:
					x+=pNode->dist;
					break;
				case BD_UP:
					y-=pNode->dist;
					break;
				case BD_DOWN:
					y+=pNode->dist;
					break;
			}
			m_points[n]=i_math::pos2di(x,y);
			n++;
			pNode=pNode->pNext;
		}
		m_contourend[m]=n;
		m++;
		pBoundary=pBoundary->pNext;
	}
	m_points.resize(n);
	m_contourend.resize(m);
	DestroyBoundary();

	Optimize();

	return TRUE;
}

BOOL CBoundFinder::ConstructPCS(unsigned char *pData,int w,int h)
{
	CStripe **stripeArray,**stripeArray2;
	if(TRUE)
	{
		_stripes.resize(h);
		_stripes2.resize(h);
		stripeArray=_stripes.data();
		stripeArray2=_stripes2.data();
		int i,j;
		BOOL bOn;
		CStripe *pStripe;
		unsigned char *p;
		p=pData;
		for (i=0;i<h;i++)
		{
			bOn=FALSE;
			stripeArray[i]=(CStripe *)_poolStripe.Alloc();
			pStripe=stripeArray[i];
			pStripe->pNext=NULL;
			stripeArray2[i]=NULL;
			for (j=0;j<w;j++)
			{
				if ((!*p)^(!bOn))
				{
					if (bOn)
					{
						pStripe->pNext->end=j-1;
						pStripe->pNext->pNext=NULL;
						pStripe->pNext->pPrev=pStripe;
						pStripe=pStripe->pNext;
						stripeArray2[i]=pStripe;
					}
					else
					{
						pStripe->pNext=(CStripe *)_poolStripe.Alloc();
						pStripe->pNext->start=j;
						pStripe->pNext->startMarked=FALSE;
						pStripe->pNext->endMarked=FALSE;
					}
					bOn=*p;
				}
				p++;
			}
			if (bOn)
			{
				pStripe->pNext->end=j-1;
				pStripe->pNext->pNext=NULL;
				pStripe->pNext->pPrev=pStripe;
				pStripe=pStripe->pNext;
				stripeArray2[i]=pStripe;
			}
			pStripe=stripeArray[i];
			stripeArray[i]=pStripe->pNext;
			if (stripeArray[i])
				stripeArray[i]->pPrev=NULL;
		}
		return TRUE;
	}
}

void CBoundFinder::RemovePCS()
{
	_poolStripe.Reset();
	_stripes.clear();
	_stripes2.clear();
}

BOOL CBoundFinder::PCStoChain(int h)
{
	int i;
	int n;
	CBound *p;
	p=_bounds=_poolBoundary.Alloc();
	p->pNext=NULL;
	for (i=0;i<h;i++)
	{
		n=0;
		CStripe *pStripe;
		pStripe=_stripes[i];
		while(pStripe)
		{
			if (!pStripe->startMarked)
			{
				p->pNext=_poolBoundary.Alloc();
				p=p->pNext;
				p->pNext=NULL;
				p->x=pStripe->start;
				p->y=i;
				p->pStartNode=BottomUp(pStripe,i);
			}
			else
			{
				if (!pStripe->endMarked)
				{
					p->pNext=_poolBoundary.Alloc();
					p=p->pNext;
					p->pNext=NULL;
					p->x=pStripe->end;
					p->y=i;
					p->pStartNode=TopDown(pStripe,i);
				}
			}
			pStripe=pStripe->pNext;
		}
	}
	p=_bounds;
	_bounds=_bounds->pNext;
	_poolBoundary.Free(p);
	return TRUE;
}

CBoundNode *CBoundFinder::BottomUp(CStripe *pStripe,int y)//for the left end,up
{
	if (pStripe->startMarked)
		return NULL;
	pStripe->startMarked=TRUE;
	assert(y>=1);
	
	CBoundNode *pReturn;
	
	CStripe *pStripeUp=NULL;
	if (y>1)
		pStripeUp=_stripes[y-1];
	while(pStripeUp)
	{
		if ((pStripe->start<=pStripeUp->end)&&(pStripe->end>=pStripeUp->start))
		{
			//have part in common
			if (pStripeUp->start>=pStripe->start)
			{
				CBoundNode *p;
				pReturn=(CBoundNode *)_poolBN.Alloc();
				if (pStripeUp->start>pStripe->start)
				{
					pReturn->dir=BD_RIGHT;
					pReturn->dist=pStripeUp->start-pStripe->start;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_UP;
					p->dist=1;
					p->pNext=BottomUp(pStripeUp,y-1);
				}
				else
				{
					pReturn->dir=BD_UP;
					pReturn->dist=1;
					pReturn->pNext=BottomUp(pStripeUp,y-1);
				}
				return pReturn;
			}
			else
			{
				if((pStripe->pPrev==NULL)||(pStripe->pPrev->end<pStripeUp->start))
				{
					CBoundNode *p;
					pReturn=(CBoundNode *)_poolBN.Alloc();
					pReturn->dir=BD_UP;
					pReturn->dist=1;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_LEFT;
					p->dist=pStripe->start-pStripeUp->start;
					p->pNext=BottomUp(pStripeUp,y-1);
					return pReturn;
				}
				else
				{
					CBoundNode *p;
					pReturn=(CBoundNode *)_poolBN.Alloc();
					pReturn->dir=BD_UP;
					pReturn->dist=1;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_LEFT;
					p->dist=pStripe->start-pStripe->pPrev->end;
					p->pNext=(CBoundNode *)_poolBN.Alloc();
					p=p->pNext;
					p->dir=BD_DOWN;
					p->dist=1;
					p->pNext=TopDown(pStripe->pPrev,y);
					return pReturn;
				}
			}
		}
		pStripeUp=pStripeUp->pNext;
	}
	pReturn=_poolBN.Alloc();
	pReturn->dir=BD_RIGHT;
	pReturn->dist=pStripe->end-pStripe->start;
	pReturn->pNext=TopDown(pStripe,y);
	return pReturn;
}

CBoundNode *CBoundFinder::TopDown(CStripe *pStripe,int y)//for the right end,down
{
	if (pStripe->endMarked)
		return NULL;
	pStripe->endMarked=TRUE;
	
	CBoundNode *pReturn;
	
	CStripe *pStripeDown=NULL;
	if (y+1<_h)
		pStripeDown=_stripes2[y+1];
	while(pStripeDown)
	{
		if ((pStripe->start<=pStripeDown->end)&&(pStripe->end>=pStripeDown->start))
		{
			//have part in common
			if (pStripeDown->end<=pStripe->end)
			{
				CBoundNode *p;
				pReturn=(CBoundNode *)_poolBN.Alloc();
				if (pStripeDown->end<pStripe->end)
				{
					pReturn->dir=BD_LEFT;
					pReturn->dist=pStripe->end-pStripeDown->end;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_DOWN;
					p->dist=1;
					p->pNext=TopDown(pStripeDown,y+1);
				}
				else
				{
					pReturn->dir=BD_DOWN;
					pReturn->dist=1;
					pReturn->pNext=TopDown(pStripeDown,y+1);
				}
				return pReturn;
			}
			else
			{
				if((pStripe->pNext==NULL)||(pStripe->pNext->start>pStripeDown->end))
				{
					CBoundNode *p;
					pReturn=(CBoundNode *)_poolBN.Alloc();
					pReturn->dir=BD_DOWN;
					pReturn->dist=1;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_RIGHT;
					p->dist=pStripeDown->end-pStripe->end;
					p->pNext=TopDown(pStripeDown,y+1);
					return pReturn;
				}
				else
				{
					CBoundNode *p;
					pReturn=(CBoundNode *)_poolBN.Alloc();
					pReturn->dir=BD_DOWN;
					pReturn->dist=1;
					p=pReturn->pNext=(CBoundNode *)_poolBN.Alloc();
					p->dir=BD_RIGHT;
					p->dist=pStripe->pNext->start-pStripe->end;
					p->pNext=(CBoundNode *)_poolBN.Alloc();
					p=p->pNext;
					p->dir=BD_UP;
					p->dist=1;
					p->pNext=BottomUp(pStripe->pNext,y);
					return pReturn;
				}
			}
		}
		pStripeDown=pStripeDown->pPrev;
	}
	pReturn=(CBoundNode *)_poolBN.Alloc();
	pReturn->dir=BD_LEFT;
	pReturn->dist=pStripe->end-pStripe->start;
	pReturn->pNext=BottomUp(pStripe,y);
	return pReturn;
}

void CBoundFinder::DestroyBoundary()
{
	CBound *pBoundary;
	pBoundary=_bounds;
	_poolBN.Reset();
	_poolBoundary.Reset();
}

void CBoundFinder::Unique(CBound *pBoundary)
{
	CBound *t;
	t=pBoundary;
	while (pBoundary)
	{
		CBoundNode *pNode,**ppNode;
		pNode=pBoundary->pStartNode;
		ppNode=&(pBoundary->pStartNode);
		while (pNode)
		{
			if (pNode->dist==0)
				*ppNode=pNode->pNext;
			else
				ppNode=&(pNode->pNext);
			pNode=pNode->pNext;
		}
		pBoundary=pBoundary->pNext;
	}

	pBoundary=t;
	while (pBoundary)
	{
		CBoundNode *pNode,**ppNode;
		pNode=pBoundary->pStartNode;
		ppNode=&(pBoundary->pStartNode);
		int dir,dist;
		if (pNode)
			dir=pNode->dir;
		dist=0;
		while (pNode)
		{
			if (pNode->dir==dir)
			{
				dist+=pNode->dist;
			}
			else
			{
				(*ppNode)->dist=dist;
				dir=pNode->dir;
				dist=pNode->dist;
				(*ppNode)->pNext=pNode;
				ppNode=&((*ppNode)->pNext);
			}
			pNode=pNode->pNext;
		}
		if (*ppNode)
		{
			(*ppNode)->dist=dist;
			(*ppNode)->pNext=NULL;
		}
		pBoundary=pBoundary->pNext;
	}
}


BOOL CBoundFinder::IsTurningRight(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3)
{
	assert(pt1!=pt2);
	assert(pt2!=pt3);
//	assert(pt1!=pt3);
	int dx1,dy1,dx2,dy2;
	dx1=pt2.x-pt1.x;
	dx2=pt3.x-pt2.x;
	dy1=pt2.y-pt1.y;
	dy2=pt3.y-pt2.y;
	
	if ((dx1==0)&&(dy1>0)&&(dy2==0)&&(dx2<0))
		return TRUE;
	if ((dx1>0)&&(dy1==0)&&(dx2==0)&&(dy2>0))
		return TRUE;
	if ((dy1==0)&&(dx1<0)&&(dx2==0)&&(dy2<0))
		return TRUE;
	if ((dx1==0)&&(dy1<0)&&(dy2==0)&&(dx2>0))
		return TRUE;
	return FALSE;	
}


BOOL CBoundFinder::IsTurningLeft(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3)
{
	assert(pt1!=pt2);
	assert(pt2!=pt3);
//	assert(pt1!=pt3);
	int dx1,dy1,dx2,dy2;
	dx1=pt2.x-pt1.x;
	dx2=pt3.x-pt2.x;
	dy1=pt2.y-pt1.y;
	dy2=pt3.y-pt2.y;
	
	if ((dx1==0)&&(dy1>0)&&(dy2==0)&&(dx2>0))
		return TRUE;
	if ((dx1>0)&&(dy1==0)&&(dx2==0)&&(dy2<0))
		return TRUE;
	if ((dy1==0)&&(dx1<0)&&(dx2==0)&&(dy2>0))
		return TRUE;
	if ((dx1==0)&&(dy1<0)&&(dy2==0)&&(dx2<0))
		return TRUE;
	return FALSE;	
}


BOOL CBoundFinder::IsTurning(i_math::pos2di pt1,i_math::pos2di pt2,i_math::pos2di pt3)
{
//	assert(pt1!=pt2);
	if (pt1==pt2)
		return FALSE;
//	assert(pt2!=pt3);
	if (pt2==pt3)
		return FALSE;
//	assert(pt1!=pt3);
	
	if ((pt1.x==pt2.x)&&(pt2.x==pt3.x))
		return FALSE;
	if ((pt1.y==pt2.y)&&(pt2.y==pt3.y))
		return FALSE;
	return TRUE;
}


void CBoundFinder::Optimize()
{
	if (m_points.size()<=0)
		return;
	if (m_contourend.size()<=1)
		return;

	std::vector<i_math::pos2di>pointsNew;
	std::vector<int>contourendNew;

	pointsNew.reserve(m_points.size());
	contourendNew.reserve(contourendNew.size());

	for (int i=0;i<m_contourend.size();i++)
	{
		int iStart,iEnd,iFinish;
		if (i==0)
			iStart=0;
		else
			iStart=m_contourend[i-1];
		iFinish=m_contourend[i];

		if (iFinish-iStart<3)
		{//这一段很短,我们不优化了
			for (int i=iStart;i<iFinish;i++)
				pointsNew.push_back(m_points[i]);
			contourendNew.push_back(pointsNew.size());
			continue;
		}

		iEnd=iStart+2;

		while(iEnd<iFinish)
		{
			i_math::pos2di posStart,posEnd;
			posStart=m_points[iStart];
			posEnd=m_points[iEnd];

			i_math::line2df line;
			line.start.set((float)posStart.x,(float)posStart.y);
			line.end.set((float)posEnd.x,(float)posEnd.y);

			BOOL bFail=FALSE;
			if (posStart==posEnd)
				bFail=TRUE;
			else
			{
				for (int i=iStart+1;i<iEnd;i++)
				{
					i_math::pos2di pos=m_points[i];

					i_math::vector2df v;
					v.set((float)pos.x,(float)pos.y);

					float rate;
					line.getProjection(v,rate);

					if ((rate<0.0f)||(rate>1.0f))
					{
						bFail=TRUE;
						break;
					}

					if (line.getDistTo(v)>1.0f)
					{
						bFail=TRUE;
						break;
					}
				}
			}

			if (bFail)
			{
				//新增一个点
				pointsNew.push_back(m_points[iEnd-1]);

				//开始新的检测
				iStart=iEnd-1;
				iEnd++;
			}
			else
				iEnd++;
		}

		pointsNew.push_back(m_points[iEnd-1]);
		contourendNew.push_back(pointsNew.size());

	}

	m_points.swap(pointsNew);
	m_contourend.swap(contourendNew);
}
