/********************************************************************
	created:	2007/7/23   13:39
	filename: 	e:\IxEngine\Proj_WorldSystem\assetzoner.cpp
	author:		cxi
	
	purpose:	zone manager
*********************************************************************/
#include "stdh.h"
#include "zoner.h"

#include "../commondefines/general_stl.h"
#include "spatialtester/spatialtester.h"

#include "timer/profiler.h"
#include "timer/timer.h"

#include "rasterize/rasterize.h"

#include "stringparser/stringparser.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)

#define ZQ_FIXED 0
#define ZQ_MOTIVE 1




IMPLEMENT_CLASS(ZoneRgn);
IMPLEMENT_CLASS(ZoneAnchor);
IMPLEMENT_CLASS(Zonee);

//////////////////////////////////////////////////////////////////////////
//CZoner::_Block

BOOL CZoner::_Block::IsEmpty()
{
	for (int i=0;i<ARRAY_SIZE(zqs);i++)
	{
		if (zqs[i].zonees.size()>0)
			return FALSE;
	}
	if (rgns.size()>0)
		return FALSE;
	return TRUE;
}


void CZoner::_Block::Clear()
{
	for (int i=0;i<ARRAY_SIZE(zqs);i++)
	{
		std::deque<Zonee*>::iterator it;
		for (it=zqs[i].zonees.begin();it!=zqs[i].zonees.end();it++)
		{
			Zonee *p=(Zonee*)(*it);
			SAFE_RELEASE(p);
		}
		zqs[i].zonees.clear();
	}

	for (int i=0;i<rgns.size();i++)
		rgns[i]->Release();

	Zero();
}

//move the content from blk to me(the content in blk will be cleared)
void CZoner::_Block::Fetch(CZoner::_Block *blk)
{
	bVRDirty=blk->bVRDirty;
	pt=blk->pt;
	for (int i=0;i<ARRAY_SIZE(zqs);i++)
	{
		zqs[i].zonees.swap(blk->zqs[i].zonees);
		zqs[i].vr=blk->zqs[i].vr;
		zqs[i].nDead=blk->zqs[i].nDead;
	}
	rgns.swap(blk->rgns);

	blk->Zero();
}


void CZoner::_Block::FlushDead(BOOL bFlushAll)
{
	std::deque<Zonee *>::iterator it,it2;
	for (int i=0;i<ARRAY_SIZE(zqs);i++)
	{
		CZoner::_ZoneeQueue *q=&zqs[i];
		DWORD nDead=0;

		if (bFlushAll||(q->nDead>5))//5 is a threshold,maybe adjusted
		{
			if (q->nDead<q->zonees.size()/2)
			{//the alive is in majority,find dead and swap to the end
				it=q->zonees.begin();
				it2=q->zonees.end();

				while((it!=it2)&&(q->nDead>0))
				{
					Zonee *p=(Zonee *)(*it);
					if (!p->IsAlive())
					{
						it2--;
						Swap<Zonee*>(*it,*it2);
						nDead++;
						q->nDead--;
					}
					else
						it++;
				}
			}
			else
			{//the dead is in majority,find alive and swap to the begin
				DWORD nAlive=0;
				it=q->zonees.begin();
				it2=q->zonees.end();
				while(it!=it2)
				{
					Zonee *p=(Zonee *)(*(it2-1));
					if (p->IsAlive())
					{
						Swap<Zonee*>(*it,*(it2-1));
						it++;
						nAlive++;
					}
					else
						it2--;
				}

				nDead=q->zonees.size()-nAlive;
			}
		}

		//Now remove nDead from the end
		it=q->zonees.end();
		for (int j=0;j<nDead;j++)
		{
			it--;
			Zonee *p=(Zonee *)(*it);

			zoner->_FlushZoneeRgn(p,0);//break all

			SAFE_RELEASE(p);
		}

		q->zonees.resize(q->zonees.size()-nDead);
		if ((nDead>0)&&(i==ZQ_MOTIVE))
			bVRDirty=TRUE;

		q->nDead=0;
	}
}

void CZoner::_Block::RecalcVR()
{
	FlushDead(FALSE);
	if (!bVRDirty)
		return;

	_ZoneeQueue *q=&zqs[ZQ_MOTIVE];
	q->vr.zero();

	std::deque<Zonee *>::iterator it;
	for (it=q->zonees.begin();it!=q->zonees.end();it++)
	{
		Zonee *p=(Zonee *)(*it);
		if (!p->IsAlive())
		{
			IncDead(ZQ_MOTIVE);
			continue;
		}
		q->vr.mergeY(p->aabb);
	}

	bVRDirty=FALSE;
}

BOOL CZoner::_Block::CalcAABB(i_math::aabbox3df &aabb)
{
	i_math::rangef vr;
	for (int k=0;k<ARRAY_SIZE(zqs);k++)
		vr.merge(zqs[k].vr);
	if (vr.isEmpty())
		return FALSE;
	aabb.MinEdge.set((float)pt.x*zoner->_blocklen,vr.low,(float)pt.y*zoner->_blocklen);
	aabb.MaxEdge.set((float)(pt.x+1)*zoner->_blocklen,vr.hi,(float)(pt.y+1)*zoner->_blocklen);

	return TRUE;
}





//////////////////////////////////////////////////////////////////////////
//CZoner

//w,h are in block
BOOL CZoner::Init(DWORD w,DWORD h,float blocklen)
{
	if ((CCycleMap2::GetWidth()>0)&&(CCycleMap2::GetHeight()>0))
		return TRUE;//by now re-init is not allowed
#pragma message("------------------------------should do the re-init if needed")

	//确保是偶数
	if (w%2!=0)
		w++;
	if (h%2!=0)
		h++;

	if (FALSE==CCycleMap2::Init(w,h))
		return FALSE;

	_blocklen=blocklen;

	_BuildQTree();

	_blkmap=new _Block[w*h];
	for (int i=0;i<w*h;i++)
		_blkmap[i].zoner=this;

	_rcMap.set(-100000,-100000,-100000,-100000);

	_blkpool.SetName("zoner_blkpool");

	return TRUE;
}

void CZoner::Clear()
{
	_FlushDead(TRUE);
	_FlushGlobalDead();

	GarbageCollect();

	CCycleMap2::Clear();

	DWORD sz=_rcMap.getArea();
	for (int i=0;i<sz;i++)
		_blkmap[i].Clear();
	SAFE_DELETE_ARRAY(_blkmap);

	std::map<__int64,_Block *>::iterator it;
	for (it=_blksOutter.begin();it!=_blksOutter.end();it++)
	{
		((*it).second)->Clear();
		_blkpool.Free(((*it).second));
	}
	_blksOutter.clear();
	_blkpool.Reset();

	_qtree.clear();
	_blknodes.clear();

	_bufTemp.clear();
	_bufZonee.clear();
	_bufRgn.clear();

	Zero();
}


void CZoner::_Touch(i_math::pos2di &ptUnit,i_math::pos2di &ptRelative)
{
	i_math::pos2di pt=ptUnit;
	_core.ToLocal(pt);

	_Block *blk=_GetInnerBlock(pt.x,pt.y);

	blk->FlushDead(TRUE);//flush all the dead
	if (!blk->IsEmpty())
	{//this block still contains some alive,we should store it in the outter buffer
		assert(_blksOutter.find(FORCE_TYPE(__int64,blk->pt))==_blksOutter.end());

		_Block *blkOut=_blkpool.Alloc();
		blkOut->zoner=this;
		blkOut->Fetch(blk);
		_blksOutter[FORCE_TYPE(__int64,blkOut->pt)]=blkOut;
	}
	else
		blk->Zero();

	blk->pt=ptUnit;

	std::map<__int64,_Block *>::iterator it=_blksOutter.find(FORCE_TYPE(__int64,ptUnit));
	if (it!=_blksOutter.end())
	{//the block of the new position can be found in outter,fetch it
		blk->Fetch((*it).second);

		//remove it from outter
		_blkpool.Free((*it).second);
		_blksOutter.erase(it);
	}


}



CZoner::_Node* CZoner::_BuildQTreeNode(std::vector<CZoner::_Node>&tree,
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


void CZoner::_BuildQTree()
{
	_qtree.clear();

	int len=max(CCycleMap2::GetWidth(),CCycleMap2::GetHeight());


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
	rcMap.set(-((int)CCycleMap2::GetWidth())/2,-((int)CCycleMap2::GetHeight())/2,
						((int)CCycleMap2::GetWidth())/2,((int)CCycleMap2::GetHeight())/2);

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


	//register the leaf nodes in the blknode map
	_blknodes.resize(rcMap.getArea());
	for (int i=_leafstart;i<_qtree.size();i++)
	{
		_Node *node=&_qtree[i];
		_blknodes[node->rc.Top()*rcMap.getWidth()+node->rc.Left()]=node;
	}

}

CZoner::_Block *CZoner::_GetOutterBlock(i_math::pos2di &pt,BOOL bCINE)
{
	assert(!_rcMap.isPointInside(pt));

	std::map<__int64,_Block *>::iterator it=_blksOutter.find(FORCE_TYPE(__int64,pt));
	if (it!=_blksOutter.end())
		return (*it).second;
	if (!bCINE)
		return NULL;

	_Block *blk=_blkpool.Alloc();
	blk->zoner=this;
	blk->pt=pt;
	_blksOutter[FORCE_TYPE(__int64,pt)]=blk;
	return blk;
}

CZoner::_Block *CZoner::_GetBlock(i_math::pos2di &pt0,BOOL bCINE,BOOL *bOutter)
{
	_Block *q;
	if (_rcMap.isPointInside(pt0))
	{
		i_math::pos2di pt=pt0;
		_core.ToLocal(pt);
		q=_GetInnerBlock(pt.x,pt.y);
		if (bOutter)
			*bOutter=FALSE;
	}
	else
	{
		q=_GetOutterBlock(pt0,bCINE);//Create if not existing
		if (bOutter)
			*bOutter=TRUE;
	}

	return q;

}


//break the link with the region in the zonee
//method:	0,flush all ; 
//					1,flush the only enumed region,and do not remove the zonee from the region
//					2,flush all the enumed regions(maybe several)
void CZoner::_FlushZoneeRgn(Zonee *zonee,int method)
{
	Zonee *p=((Zonee *)zonee);
	ZoneAnchor *anchor=(ZoneAnchor*)p->anchor;
	if (!anchor)
		return;

	ZoneAnchor **pp=&anchor;
	while(*pp)//remove the rgn in the zonee's region anchor
	{
		ZoneRgn *rgn=(*pp)->rgn;
		if (method==0)
			rgn->flag|=RgnFlag_Enum;
		if (rgn->flag&RgnFlag_Enum)
		{
			if (_handlerRgnEvent)
				_handlerRgnEvent(rgn,zonee,ZoneeLeaving);
			rgn->Release();
			rgn->flag&=~RgnFlag_Enum;
			if (method!=1)
			{//remove the zonee in the rgn
				std::deque<Zonee *>::iterator it,itEnd;
				for (it=rgn->zonees.begin();it!=rgn->zonees.end();it++)
				{
					if ((*it)==p)
					{
						if(p)
							p->Release();
						itEnd=rgn->zonees.end();
						itEnd--;
						(*it)=(*itEnd);
						rgn->zonees.pop_back();
						break;
					}
				}
			}
			ZoneAnchor *t=(*pp);
			(*pp)=(*pp)->next;
			Class_Delete(t);
			if (method==1)
				break;//there is only one to break
			continue;
		}
		pp=&(*pp)->next;
	}

	zonee->anchor=anchor;

}

//link an zonee and a rgn
void CZoner::_LinkZoneeRgn(Zonee *zonee,ZoneRgn *rgn)
{
	if (TRUE)//add the rgn in the zonee
	{
		ZoneAnchor *anchor=Class_New(ZoneAnchor);
		ZoneAnchor *old=zonee->anchor;

		anchor->next=old;
		anchor->rgn=rgn;
		zonee->anchor=anchor;
		rgn->AddRef();
	}

	if (TRUE)//add the zonee into the rgn
	{
		rgn->zonees.push_back(zonee);
		zonee->AddRef();
	}

	if (_handlerRgnEvent)
		_handlerRgnEvent(rgn,zonee,ZoneeEntering);
}


void CZoner::_AddRgn(_Block *blk,ZoneRgn *rgn)
{
	blk->rgns.push_back(rgn);
	rgn->AddRef();

	for (int k=0;k<ARRAY_SIZE(blk->zqs);k++)
	{
		_ZoneeQueue *q=&blk->zqs[k];
		std::deque<Zonee *>::iterator it;
		for (it=q->zonees.begin();it!=q->zonees.end();it++)
		{
			Zonee *zonee=(Zonee *)(*it);
			if (!(zonee->containable&rgn->type))
				continue;
			if (zonee->TestFlag(ZoneeFlag_Enum))
				continue;

			zonee->SetFlag(ZoneeFlag_Enum);
			_bufTemp.push_back(zonee);

			if (_TestRgnAgainstZonee(rgn,zonee))
				_LinkZoneeRgn(zonee,rgn);
		}
	}
}

void CZoner::_RemoveRgn(_Block *blk,ZoneRgn *rgn)
{
	for (int i=0;i<blk->rgns.size();i++)
	{
		if (blk->rgns[i]==rgn)
		{
			rgn->Release();
			blk->rgns.erase(blk->rgns.begin()+i);
			break;
		}
	}
}


//bAlreadyIn indicate whether the zonee is already in the given block
void CZoner::_Add(Zonee *zonee,BOOL bAlreadyIn,i_math::pos2di &pt0,
									  DWORD idx)
{
	_Block *q;
	BOOL bOut=TRUE;
	q=_GetBlock(pt0,TRUE,&bOut);

	if (!bAlreadyIn)
	{
		q->zqs[idx].zonees.push_back(zonee);
		zonee->AddRef();
	}

	//Now update the vertical range
	if (!bOut)
	{
		i_math::rangef vrTotal;
		for (int i=0;i<ARRAY_SIZE(q->zqs);i++)
			vrTotal.merge(q->zqs[i].vr);

		q->zqs[idx].vr.mergeY(zonee->aabb);

		if (vrTotal.mergeY(zonee->aabb))
		{//Modified,need update the node in QTree
			i_math::pos2di pt=pt0;
			assert(_rcMap.isPointInside(pt));
			pt-=_rcMap.UpperLeftCorner;
			_Node *node=_GetBlockNode(pt.x,pt.y);
			while(node)
			{
				if (!node->vr.mergeY(zonee->aabb))
					break;
				node=node->parent;
			}
		}
	}
	else
		q->zqs[idx].vr.mergeY(zonee->aabb);

	//Now check for rgn
	for (int i=0;i<q->rgns.size();i++)
	{
		ZoneRgn *rgn=q->rgns[i];
		if (!(zonee->containable&rgn->type))
			continue;
		if (rgn->flag&RgnFlag_Enum)
			continue;//already tested

		rgn->flag|=RgnFlag_Enum;
		_bufTemp.push_back(rgn);

		if (_TestRgnAgainstZonee(rgn,zonee))
			_LinkZoneeRgn(zonee,rgn);
	}
	
}

void CZoner::_Remove(Zonee *zonee,i_math::pos2di &pt0,DWORD idx)
{
	_Block *q=_GetBlock(pt0,FALSE);

	if (idx==ZQ_MOTIVE)
		q->bVRDirty=TRUE;//mark as dirty

	if (zonee)
	{
		DEQUE_REMOVE(q->zqs[idx].zonees,Zonee*,zonee);

		assert(zonee->refcount>1);
		SAFE_RELEASE(zonee);
	}

}

inline void CalcZoneInfo(i_math::recti &rc,i_math::aabbox3df &aabb,float blocklen)
{
	rc.Left()=(int)floor(aabb.MinEdge.x/blocklen);
	rc.Top()=(int)floor(aabb.MinEdge.z/blocklen);
	rc.Right()=(int)floor(aabb.MaxEdge.x/blocklen)+1;
	rc.Bottom()=(int)floor(aabb.MaxEdge.z/blocklen)+1;
}


Zonee *CZoner::AddZonee(i_math::aabbox3df &aabb,ZoneeType type,RgnType containable,void *owner,BOOL bMotive)
{
//	MessageBox(NULL,"AddZonee0","log",MB_OK);
	Zonee *zonee=Class_New(Zonee);
	zonee->refcount=1;
	zonee->owner=owner;
	zonee->flags=bMotive?ZoneeFlag_Motive:0;
	zonee->type=type;
	zonee->containable=containable;
	zonee->anchor=NULL;
	zonee->aabb=aabb;

	i_math::recti rc;
	CalcZoneInfo(rc,aabb,_blocklen);



	DWORD idx=bMotive?ZQ_MOTIVE:ZQ_FIXED;

	_bufTemp.clear();


	//Now add the zonee to all the affected blocks
	if (TRUE)
	{
		for (int i=rc.Left();i<rc.Right();i++)
		for (int j=rc.Top();j<rc.Bottom();j++)
			_Add(zonee,FALSE,i_math::pos2di(i,j),idx);
	}

	//clear enum flag for rgns
	for (int i=0;i<_bufTemp.size();i++)
		((ZoneRgn*)_bufTemp[i])->flag&=~RgnFlag_Enum;


	return zonee;
}

Zonee *CZoner::AddGlobal(ZoneeType type,void *owner)
{
	Zonee *zonee=Class_New(Zonee);
	zonee->refcount=1;
	zonee->owner=owner;
	zonee->flags=ZoneeFlag_Global;
	zonee->type=type;
	zonee->containable=0;
	zonee->anchor=NULL;

	zonee->AddRef();
	_globals.push_back(zonee);

	return zonee;
}

void CZoner::RemoveZonee(Zonee *zonee)
{
	_FlushZoneeRgn(zonee,0);
	if (zonee->TestFlag(ZoneeFlag_Global))
		_bGlobalDirty=TRUE;
	zonee->Release();
	zonee->SetFlag(ZoneeFlag_Dead);
}



BOOL CZoner::Update(Zonee *zonee,i_math::aabbox3df &aabb)
{
	if(zonee->TestFlag(ZoneeFlag_Global))
		return FALSE;
	if (!zonee->TestFlag(ZoneeFlag_Motive))
		return FALSE;

	zonee->AddRef();//临时加一个引用计数,以确保在block里删除/加入的过程中,这个zonee始终保持有效

	i_math::recti rcOld;
	CalcZoneInfo(rcOld,zonee->aabb,_blocklen);

	i_math::recti rcNew;
	CalcZoneInfo(rcNew,aabb,_blocklen);

	for (int i=rcOld.Left();i<rcOld.Right();i++)
	for (int j=rcOld.Top();j<rcOld.Bottom();j++)
	{
		if (rcNew.isPointInside((short)i,(short)j))
			_Remove(NULL,i_math::pos2di(i,j),ZQ_MOTIVE);
		else
			_Remove(zonee,i_math::pos2di(i,j),ZQ_MOTIVE);
	}

	zonee->aabb=aabb;

	//flush the rgn that no longer contains the zonee
	_bufTemp.clear();
	if (TRUE)
	{
		ZoneAnchor*anchor=(ZoneAnchor*)zonee->anchor;
		while(anchor)
		{
			if (!_TestRgnAgainstZonee(anchor->rgn,zonee))
				anchor->rgn->flag|=RgnFlag_Enum;//this rgn should be flushed(removed)
			_bufTemp.push_back(anchor->rgn);//record the tested rgns
			anchor=anchor->next;
		}

		_FlushZoneeRgn(zonee,2);//flush all the enum rgn
	}

	//mark all the tested rgns,so that they will not be tested again
	for (int i=0;i<_bufTemp.size();i++)
		((ZoneRgn*)_bufTemp[i])->flag|=RgnFlag_Enum;


	for (int i=rcNew.Left();i<rcNew.Right();i++)
	for (int j=rcNew.Top();j<rcNew.Bottom();j++)
	{
		if (rcOld.isPointInside((short)i,(short)j))
			_Add(zonee,TRUE,i_math::pos2di(i,j),ZQ_MOTIVE);
		else
			_Add(zonee,FALSE,i_math::pos2di(i,j),ZQ_MOTIVE);
	}

	//clear enum flag for rgns
	for (int i=0;i<_bufTemp.size();i++)
		((ZoneRgn*)_bufTemp[i])->flag&=~RgnFlag_Enum;


	zonee->Release();

	return TRUE;
}

void CZoner::_AddRgnToBlks(ZoneRgn *rgn,i_math::aabbox3df &aabbRgn)
{
	i_math::aabbox3df aabb;
	i_math::recti rc;
	CalcZoneInfo(rc,aabbRgn,_blocklen);
	rgn->rc=rc;
	for (int i=rc.Left();i<rc.Right();i++)
	for (int j=rc.Top();j<rc.Bottom();j++)
	{
		_Block *q=_GetBlock(i_math::pos2di(i,j),TRUE);

		if (FALSE==q->CalcAABB(aabb))
			continue;

		if (!aabbRgn.intersectsWithBox(aabb))
			continue;

		_AddRgn(q,rgn);
	}
}

void CZoner::_RemoveRgnFromBlks(ZoneRgn *rgn)
{
	for (int i=rgn->rc.Left();i<rgn->rc.Right();i++)
	for (int j=rgn->rc.Top();j<rgn->rc.Bottom();j++)
	{
		_Block *q=_GetBlock(i_math::pos2di(i,j),FALSE);
		assert(q);

		_RemoveRgn(q,rgn);
	}
}


ZoneRgn *CZoner::AddRgn(SpacialTester &tester,void *data,RgnType type)
{
	i_math::aabbox3df aabb;
	if (FALSE==tester.GetAABB(aabb))
		return NULL;


	ZoneRgn *rgn=Class_New(ZoneRgn);
	rgn->refcount=1;
	rgn->type=type;
	rgn->flag=0;
	rgn->data=data;
	rgn->tester=tester;

	_bufTemp.clear();

	_AddRgnToBlks(rgn,aabb);

	//clear the enum flag for all the enumed zonees
	for (int i=0;i<_bufTemp.size();i++)
		((Zonee*)_bufTemp[i])->ClearFlag(ZoneeFlag_Enum);

	return rgn;
}


BOOL CZoner::RemoveRgn(ZoneRgn *rgn)
{

	RgnEventHandler t=_handlerRgnEvent;
	_handlerRgnEvent=NULL;//删除rgn时,不能发消息

	//first remove from the blocks
	_RemoveRgnFromBlks(rgn);

	//now break the link with the zonees
	std::deque<Zonee *>::iterator it;
	for (it=rgn->zonees.begin();it!=rgn->zonees.end();it++)
	{
		rgn->flag|=RgnFlag_Enum;
		_FlushZoneeRgn((*it),1);//flush the only one
		SAFE_RELEASE(*it);
	}
	rgn->zonees.clear();

	assert(rgn->refcount==1);

	Class_Delete(rgn);

	_handlerRgnEvent=t;

	return TRUE;
}

BOOL CZoner::UpdateRgn(ZoneRgn *rgn,SpacialTester &tester)
{
	i_math::aabbox3df aabb;
	if (FALSE==tester.GetAABB(aabb))
		return FALSE;

	//first remove from the blocks
	_RemoveRgnFromBlks(rgn);

	rgn->tester=tester;

	_bufTemp.clear();
	_bufZonee2.clear();

	//now break the link with the zonees that no longer contact with the rgn
	std::deque<Zonee *>::iterator it;
	for (it=rgn->zonees.begin();it!=rgn->zonees.end();it++)
	{
		Zonee *zonee=(*it);
		zonee->SetFlag(ZoneeFlag_Enum);
		_bufTemp.push_back(zonee);

		if (_TestRgnAgainstZonee(rgn,zonee))
		{//这个zonee仍然在rgn范围内,我们保留它
			_bufZonee2.push_back(zonee);
			continue;
		}
		//不在rgn范围内了,丢弃它
		rgn->SetFlag(RgnFlag_Enum);
		_FlushZoneeRgn(zonee,1);//flush the only enumed rgn
		assert(zonee->refcount>1);
		SAFE_RELEASE(zonee);
	}
	rgn->zonees.swap(_bufZonee2);

	//加入到blocks
	_AddRgnToBlks(rgn,aabb);
	
	//clear the enum flag for all the enumed zonees
	for (int i=0;i<_bufTemp.size();i++)
		((Zonee*)_bufTemp[i])->ClearFlag(ZoneeFlag_Enum);

	rgn->ClearFlag(RgnFlag_Enum);

	return TRUE;	
}




void CZoner::_FlushDead(BOOL bFlushAll)
{
	DWORD sz=_rcMap.getArea();
	
	if (bFlushAll)
	{
		for (int i=0;i<sz;i++)
			_blkmap[i].FlushDead(bFlushAll);
	}
	else
	{
		if (sz>0)
		{
			const int step=50;
			for (int i=0;i<step;i++)
				_blkmap[(i+_lastgc)%sz].FlushDead(FALSE);
			_lastgc=(step+_lastgc)%sz;
		}
	}

	std::map<__int64,_Block *>::iterator it=_blksOutter.begin(),itNext;

	while(it!=_blksOutter.end())
	{
		(*it).second->FlushDead(TRUE);
		if ((*it).second->IsEmpty())
		{
			_blkpool.Free((*it).second);
			itNext=it;
			itNext++;
			_blksOutter.erase(it);
			it=itNext;
			continue;
		}
		it++;
	}
}

void CZoner::_UpdateQTree()
{
	//first update all the leaf node
	DWORD sz=_rcMap.getArea();
	for (int i=0;i<sz;i++)
	{
		_Block *blk=&_blkmap[i];
//		blk->FlushDead(FALSE);
		blk->RecalcVR();
		_Node *node=_GetBlockNode(blk->pt.x-_rcMap.Left(),blk->pt.y-_rcMap.Top());
		node->vr=blk->zqs[0].vr;
		node->vr.merge(blk->zqs[1].vr);
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


//Set the map center,in meter
BOOL CZoner::Locate(float x,float z)
{
	//add an offset to make the cyclemap updating rythm different from others
	x-=2.0f;
	z-=2.0f;

	i_math::pos2di pt=_rcMap.getCenter();
	if (UpdateBlockMapCenter(pt,x,z,_blocklen,_blocklen))
	{
		CCycleMap2::SetCenter(pt.x,pt.y);
		_rcMap=CCycleMap2::GetMapRect();
		_UpdateQTree();
	}

	return TRUE;

}

#define ADD_ENUM_ZONEE(buf,zonee)											\
{																											\
	BOOL bEnum=FALSE;																	\
	if (!((Zonee *)zonee)->TestFlag(ZoneeFlag_Enum))						\
	{																										\
		if (aabb)																						\
		{																									\
			if (aabb->intersectsWithBox(zonee->aabb))						\
				bEnum=TRUE;																	\
		}																									\
		else																								\
			bEnum=TRUE;																		\
	}																										\
	if (bEnum)																						\
	{																										\
		((Zonee *)zonee)->SetFlag(ZoneeFlag_Enum);						\
		buf.push_back((Zonee *)zonee);												\
	}																										\
}


//返回这个blk是否有东西被enum到
void CZoner::_AddEnumBlock(CZoner::_Block *blk,i_math::aabbox3df *aabb,ZoneeFlag flagFilter)
{

	blk->FlushDead(FALSE);
	if (_zeCur&ZEnum_Zonee)
	{
		for (int i=0;i<ARRAY_SIZE(blk->zqs);i++)
		{
			std::deque<Zonee*> &zonees=blk->zqs[i].zonees;
			DWORD sz=zonees.size();

			std::deque<Zonee*>::iterator it; 
			for (it=zonees.begin();it!=zonees.end();it++)
			{
				Zonee*p=(Zonee*)(*it);
				if (!p->IsAlive())
				{
					blk->IncDead(i);
					continue;
				}

				if (!(p->type&(ZoneeType)_typesCur))
					continue;//不关心的类型

				if (flagFilter)
				if (!(p->flags&flagFilter))
					continue;//

				ADD_ENUM_ZONEE(_bufZonee,p);
			}
		}
	}

	if (_zeCur&ZEnum_Rgn)
	{
		for (int i=0;i<blk->rgns.size();i++)
		{
			ZoneRgn *p=blk->rgns[i];
			if (!(p->type&(RgnType)_typesCur))
				continue;
			if (!(p->flag&RgnFlag_Enum))
			{
				_bufRgn.push_back(p);
				p->flag|=RgnFlag_Enum;
			}
		}
	}

}

#define ENUMNODE_DEPTH (_depth-1)		//only the nodes that has a depth less-equal than 
																		//ENUMNODE_DEPTH will be check against the tester
void CZoner::_EnumNode(SpacialTester &tester,_Node *node,ZoneeFlag flagFilter)
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

// 	ProfilerStart(TestAABB);
	SpacialTester::Result r=tester.Test(aabbBlock);
// 	ProfilerEnd();

	if (r==SpacialTester::NoTouch)
		return;

	if ((r==SpacialTester::Contain)||((int)node->depth>=ENUMNODE_DEPTH))
	{
		DWORD nRC;
		i_math::recti *rcs=CCycleMap2::CalcLocalRect(rcNode,nRC);

		DWORD w=_rcMap.getWidth();
		DWORD h=_rcMap.getHeight();

		for (int i=0;i<nRC;i++)
		{
			_Block *blk=&_blkmap[rcs[i].Top()*w+rcs[i].Left()];

			DWORD ww=rcs[i].getWidth();
			DWORD hh=rcs[i].getHeight();

			for (int jj=0;jj<hh;jj++)
			{
				for (int ii=0;ii<ww;ii++)
					_AddEnumBlock(&blk[ii],NULL,flagFilter);
				blk+=w;
			}
		}
		return;
	}

	//test the children
	for (int i=0;i<ARRAY_SIZE(node->childs);i++)
	{
		if (node->childs[i])
			_EnumNode(tester,node->childs[i],flagFilter);
	}
}

void CZoner::_FilterZonee_And(SpacialTester *testers,DWORD nTester,DWORD types)
{
	DWORD c=0;
	for (int i=0;i<_bufZonee.size();i++)
	{
		Zonee*p=_bufZonee[i];

		if (!(p->type&types))
			continue;

		int k;
		for (k=0;k<nTester;k++)
		{
			if (testers[k].Test(p->aabb)!=SpacialTester::NoTouch)
				break;
		}
		if (k>=nTester)
			continue;

		_bufZonee[c]=p;
		c++;
	}
	_bufZonee.resize(c);

}

void CZoner::_EnumCore(SpacialTester &tester,ZoneeFlag flagFilter)
{
//	ProfilerStart(_EnumCore);
	switch(tester.type)
	{
		case SpacialTester::Frustum:
		{
			_EnumNode(tester,_qtree.data(),flagFilter);
			break;
		}
		case SpacialTester::Rect:
		{
			i_math::recti rc;
			rc.Left()=(int)floor(tester.rc.Left()/_blocklen);
			rc.Top()=(int)floor(tester.rc.Top()/_blocklen);
			rc.Right()=(int)floor(tester.rc.Right()/_blocklen)+1;
			rc.Bottom()=(int)floor(tester.rc.Bottom()/_blocklen)+1;

			rc.clipAgainst(_rcMap);

			i_math::aabbox3df aabb;
			aabb.reset(tester.rc.Left(),-10000,tester.rc.Top());
			aabb.addInternalPoint(tester.rc.Right(),10000,tester.rc.Bottom());

			for (int j=rc.Top();j<rc.Bottom();j++)
			for (int i=rc.Left();i<rc.Right();i++)
			{
				i_math::pos2di pt(i,j);
				CCycleMap2::_core.ToLocal(pt);

				_Block *blk=_GetInnerBlock(pt.x,pt.y);

				_AddEnumBlock(blk,&aabb,flagFilter);
			}
			break;
		}

		case SpacialTester::Box:
		{
			i_math::recti rc;
			rc.Left()=(int)floor(tester.aabb.MinEdge.x/_blocklen);
			rc.Top()=(int)floor(tester.aabb.MinEdge.z/_blocklen);
			rc.Right()=(int)floor(tester.aabb.MaxEdge.x/_blocklen)+1;
			rc.Bottom()=(int)floor(tester.aabb.MaxEdge.z/_blocklen)+1;

			rc.clipAgainst(_rcMap);

			i_math::aabbox3df aabbBlock;
			for (int j=rc.Top();j<rc.Bottom();j++)
			for (int i=rc.Left();i<rc.Right();i++)
			{
				i_math::pos2di pt(i,j);
				CCycleMap2::_core.ToLocal(pt);

				_Block *blk=_GetInnerBlock(pt.x,pt.y);

				//calculate the aabb for this block
				if (!blk->CalcAABB(aabbBlock))
					continue;

				SpacialTester::Result r=tester.Test(aabbBlock);
				if (r!=SpacialTester::NoTouch)
					_AddEnumBlock(blk,&tester.aabb,flagFilter);
			}
			break;
		}

		case SpacialTester::Line:
		{
			std::vector<i_math::pos2di>qu;
			TileByLine(	tester.line.start.x,	tester.line.start.z,
								tester.line.end.x,		tester.line.end.z,
								_blocklen,qu);

			for (int i=0;i<qu.size();i++)
			{
				if (!_rcMap.isPointInside(qu[i]))
					continue;
				CCycleMap2::_core.ToLocal(qu[i]);
				_Block *blk=_GetInnerBlock(qu[i].x,qu[i].y);
				_AddEnumBlock(blk,NULL,flagFilter);
			}

			break;
		}


		default:
			assert(FALSE);
	}

//	ProfilerEnd();

}


void CZoner::_Enum(SpacialTester *testers,DWORD nTester,ZonerEnum ze,DWORD types)
{
	_bufZonee.clear();
	_bufRgn.clear();

	_zeCur=ze;
	_typesCur=types;


	for (int k=0;k<nTester;k++)
		_EnumCore(testers[k]);

	for (int i=0;i<_bufZonee.size();i++)
		_bufZonee[i]->ClearFlag(ZoneeFlag_Enum);
	for (int i=0;i<_bufRgn.size();i++)
		_bufRgn[i]->flag&=~RgnFlag_Enum;



	_FilterZonee_And(testers,nTester,types);

	if (TRUE)//do further check for rgn
	{
		if (TRUE)
		{
			DWORD c=0;
			for (int i=0;i<_bufRgn.size();i++)
			{
				ZoneRgn *p=_bufRgn[i];
				int k;
				for (k=0;k<nTester;k++)
				{
					if (testers[k].Test(p->tester)!=SpacialTester::NoTouch)
						break;
				}
				if (k>=nTester)
					continue;
				_bufRgn[c]=p;
				c++;
			}
			_bufRgn.resize(c);
		}
	}
}

Zonee *CZoner::CastRayZonee(i_math::line3df &line,ZoneeType types,CastRayCallBack callback,i_math::vector3df *pos)
{
	if (!callback)
		return NULL;

	_bufZonee.clear();
	std::vector<i_math::pos2di>qu;
	TileByLine(	line.start.x,	line.start.z,line.end.x,line.end.z,_blocklen,qu);

	Zonee *zoneeHit=NULL;
	float dist2Min=1000000.0f;
	float dist2;

	for (int i=0;i<qu.size();i++)
	{
		if (!_rcMap.isPointInside(qu[i]))
			continue;
		CCycleMap2::_core.ToLocal(qu[i]);
		_Block *blk=_GetInnerBlock(qu[i].x,qu[i].y);

		blk->FlushDead(FALSE);
		for (int i=0;i<ARRAY_SIZE(blk->zqs);i++)
		{
			std::deque<Zonee*> &zonees=blk->zqs[i].zonees;
			DWORD sz=zonees.size();

			std::deque<Zonee*>::iterator it; 
			for (it=zonees.begin();it!=zonees.end();it++)
			{
				Zonee*p=(Zonee*)(*it);
				if (!p->IsAlive())
				{
					blk->IncDead(i);
					continue;
				}

				if (!(p->type&types))
					continue;//不关心的类型

				if (p->TestFlag(ZoneeFlag_Enum))
					continue;

				p->SetFlag(ZoneeFlag_Enum);  


				_bufZonee.push_back(p);

				if (callback(line,p,dist2))
				{
					if (dist2<dist2Min)
					{
						zoneeHit=p;
						dist2Min=dist2;
					}
				}
			}
		}

		if (zoneeHit)
			break;
	}
	for (int i=0;i<_bufZonee.size();i++)
		_bufZonee[i]->ClearFlag(ZoneeFlag_Enum);

	if (zoneeHit)
	{
		if (pos)
		{
			i_math::vector3df v=line.getVector();
			v.normalize();
			*pos=line.start+v*sqrtf(dist2Min);
		}
	}

	return zoneeHit;
}


//call this immediately after Enum(..) is called
ZoneRgn**CZoner::GetEnumRgn(DWORD &count)
{
	count=_bufRgn.size();
	if (count<=0)
		return NULL;
	return (ZoneRgn**)_bufRgn.data();
}

//call this immediately after Enum(..) is called
Zonee **CZoner::GetEnumZonee(DWORD &count)
{
	count=_bufZonee.size();
	if (count<=0)
		return NULL;
	return (Zonee **)_bufZonee.data();
}

void CZoner::_FlushGlobalDead()
{
	if (!_bGlobalDirty)
		return;
	DWORD c=0;
	for (int i=0;i<_globals.size();i++)
	{
		if (_globals[i]->IsAlive())
		{
			_globals[c]=_globals[i];
			c++;
		}
		else
		{
			SAFE_RELEASE(_globals[i]);
		}
	}

	_globals.resize(c);
}

void CZoner::EnumZoneeEx(SpacialTester &tester,DWORD types,SpacialTester *includers,DWORD nIncluders,SpacialTester *excluders,DWORD nExcluders,DWORD *cullInc,DWORD *cullExc)
{
	if (cullInc)
		(*cullInc)=0;
	if (cullExc)
		(*cullExc)=0;

	_bufZonee.clear();
	_bufZonee3.clear();

	_zeCur=ZEnum_Zonee;
	_typesCur=types;

	//枚举到所有在tester范围内的zonees
//	ProfilerStart(_EnumCore);
	_EnumCore(tester);
//	ProfilerEnd();

	//清除枚举标志
	for (int i=0;i<_bufZonee.size();i++)
		_bufZonee[i]->ClearFlag(ZoneeFlag_Enum);

	//如果没有includers,我们要精确的测试一下
//	if (nIncluders<=0)
	_FilterZonee_And(&tester,1,types);

	if (FALSE)
	{
		//将枚举到的zonee打上enum3标志,并保存到_bufZonee3中
		for (int i=0;i<_bufZonee.size();i++)
			_bufZonee[i]->SetFlag(ZoneeFlag_Enum3);
		_bufZonee.swap(_bufZonee3);

		if (nIncluders>0)
		{
			for (int i=0;i<nIncluders;i++)
			{
				_bufZonee.clear();
				_EnumCore(includers[i],ZoneeFlag_Enum3);//只枚举那些有ZoneeFlag_Enum3的zonee

				for (int j=0;j<_bufZonee.size();j++)
					_bufZonee[j]->ClearFlag(ZoneeFlag_Enum);

				//对于所有被这个includer枚举到的zonee,做精确的测试
				for (int j=0;j<_bufZonee.size();j++)
				{
					Zonee *zonee=_bufZonee[j];
					if (zonee->TestFlag(ZoneeFlag_Enum2))
						continue;//已经被includers枚举到了,不用测试
					if (includers[i].Test(zonee->aabb)!=SpacialTester::NoTouch)
						zonee->SetFlag(ZoneeFlag_Enum2);//标记为被includers枚举到了
				}
			}

			DWORD c=0;
			for (int i=0;i<_bufZonee3.size();i++)
			{
				Zonee *zonee=_bufZonee3[i];
				if (zonee->TestFlag(ZoneeFlag_Enum2))
				{
					zonee->ClearFlag(ZoneeFlag_Enum2);
					_bufZonee3[c]=zonee;
					c++;
				}
				else
					zonee->ClearFlag(ZoneeFlag_Enum3);//不在_bufZonee3里了,所以要把enum3标志清掉
			}
			_bufZonee3.resize(c);
			
		}

		if (nExcluders>0)
		{
			for (int i=0;i<nExcluders;i++)
			{
				_bufZonee.clear();
				_EnumCore(excluders[i],ZoneeFlag_Enum3);//只枚举那些有ZoneeFlag_Enum3的zonee

				for (int j=0;j<_bufZonee.size();j++)
					_bufZonee[j]->ClearFlag(ZoneeFlag_Enum);

				for (int j=0;j<_bufZonee.size();j++)
				{
					Zonee *zonee=_bufZonee[j];
					if (zonee->TestFlag(ZoneeFlag_Enum2))
						continue;//已经被excluder枚举到了,不用测试
					if (excluders[i].Test(zonee->aabb)==SpacialTester::Contain)
						zonee->SetFlag(ZoneeFlag_Enum2);//标记为被excluders枚举到了
				}
			}

			DWORD c=0;
			for (int i=0;i<_bufZonee3.size();i++)
			{
				Zonee *zonee=_bufZonee3[i];
				if (!zonee->TestFlag(ZoneeFlag_Enum2))
				{
					//保留没有被枚举到的
					_bufZonee3[c]=zonee;
					c++;
				}
				else
					zonee->ClearFlag(ZoneeFlag_Enum2|ZoneeFlag_Enum3);
			}
			_bufZonee3.resize(c);

		}

		//清除enum3标记,并保存回_bufZonee
		for (int i=0;i<_bufZonee3.size();i++)
			_bufZonee3[i]->ClearFlag(ZoneeFlag_Enum3);
		_bufZonee.swap(_bufZonee3);
	}
	else
	{
		if ((nIncluders>0)||(nExcluders>0))
		{
			DWORD c=0;
			Zonee *zonee;
			__int64 tmax=0;//调试用
			int imax=-1;//调试用
			for (int i=0;i<_bufZonee.size();i++)
			{
				zonee=_bufZonee[i];

				int k;
				if (nIncluders>0)
				{	
					for (k=0;k<nIncluders;k++)
					{
						if (includers[k].Test(zonee->aabb)!=SpacialTester::NoTouch)
							break;
					}

					if (k>=nIncluders)
					{
						if (cullInc)
							(*cullInc)++;
						continue;//没有被include
					}
				}

				if (nExcluders>0)
				{	

					__int64 t1=GetTSC();//调试用

					for (k=0;k<nExcluders;k++)
					{
						if (excluders[k].Test(zonee->aabb)==SpacialTester::Contain)
							break;
					}

					//调试用
					t1=GetTSC()-t1;
					if (t1>tmax)
					{
						tmax=t1;
						imax=i;
					}
					//

					if (k<nExcluders)
					{
						if (cullExc)
							(*cullExc)++;
						continue;//被exclude了
					}
				}

				_bufZonee[c]=zonee;
				c++;
			}

			_bufZonee.resize(c);
		}

	}

}



void CZoner::GarbageCollect()
{
	_FlushDead(FALSE);
	_FlushGlobalDead();
}

Zonee **CZoner::GetGlobalZonee(DWORD &count)
{
	_FlushGlobalDead();
	count=_globals.size();
	return _globals.data();
}

