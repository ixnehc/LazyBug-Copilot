
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "CircumSites.h"

#include "Circum.h"
#include "timer/profiler.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CCircumSlots

CircumSlot*&CCircumSlots::_LookUp(i_math::vector2df &pos)
{
	float xoff,yoff;
	xoff=pos.x-_rcLookUp.Left();
	yoff=pos.y-_rcLookUp.Top();
	if (xoff<_rcLookUp.Left())
		xoff=_rcLookUp.Left();
	if (yoff<_rcLookUp.Top())
		yoff=_rcLookUp.Top();

	DWORD x,y;
	x=(DWORD)(xoff/_stepLookUp);
	y=(DWORD)(yoff/_stepLookUp);

	if (x>=_lenLookUp)
		x=_lenLookUp-1;
	if (y>=_lenLookUp)
		y=_lenLookUp-1;

	return _lookup[y*_lenLookUp+x];
}

struct CircumSlotCompareInfo
{
	CircumSlot *slots;
	i_math::vector2df pos;
};
int CircumSlotCompare(void *ctxt,const void *l0,const void *r0)
{
	CircumSlotIdx *l,*r;
	l=(CircumSlotIdx*)l0;
	r=(CircumSlotIdx*)r0;

	CircumSlotCompareInfo *info=(CircumSlotCompareInfo *)ctxt;

	float dist1=info->slots[*l].pos.getDistanceFrom(info->pos);
	float dist2=info->slots[*r].pos.getDistanceFrom(info->pos);

	if (dist1<dist2)
		return -1;
	if (dist1>dist2)
		return 1;
	return 0;
}

void CCircumSlots::Build(float gap)
{
	CCircumSites csites;
	csites.Build(20,2.0f,gap);

	i_math::rectf rc;
	if (TRUE)
	{

		DWORD nPos;
		i_math::vector2df *poses=csites.GetSites(nPos);

		_slots.resize(nPos);

		for (int i=0;i<nPos;i++)
		{
			_slots[i].pos=poses[i];
			_slots[i].idxMe=i;
			rc.merge(poses[i].x,poses[i].y);
		}
	}

	//寻找links
	if (TRUE)
	{
		DWORD nPos;
		i_math::vector2df *start=csites.GetSites(nPos);

		for (int i=0;i<_slots.size();i++)
		{
			i_math::vector2df &pos=_slots[i].pos;
			CircumSlot *slot=&_slots[i];

			int iRing=csites.RingFromSite(i);

			slot->iRing=iRing;
			slot->inner=CircumSlotIdx_Invalid;
			slot->outter=CircumSlotIdx_Invalid;
			slot->prev=slot->next=CircumSlotIdx_Invalid;

			//内圈
			if (iRing>0)
			{
				i_math::vector2df *closest=NULL;
				float dist2=1000000000.0f;
				DWORD c;
				i_math::vector2df *sites=csites.GetRingSites(iRing-1,c);
				for (int i=0;i<c;i++)
				{
					if (sites[i].getDistanceSQFrom(pos)<dist2)
					{
						dist2=sites[i].getDistanceSQFrom(pos);
						closest=&sites[i];
					}
				}

				if (closest)
					slot->inner=(CircumSlotIdx)(closest-start);
			}

			//外圈
			if (iRing+1<csites.GetRingCount())
			{
				i_math::vector2df *closest=NULL;
				float dist2=1000000000.0f;
				DWORD c;
				i_math::vector2df *sites=csites.GetRingSites(iRing+1,c);
				for (int i=0;i<c;i++)
				{
					if (sites[i].getDistanceSQFrom(pos)<dist2)
					{
						dist2=sites[i].getDistanceSQFrom(pos);
						closest=&sites[i];
					}
				}

				if (closest)
					slot->outter=(CircumSlotIdx)(closest-start);
			}

			if (TRUE)
			{
				DWORD c;
				i_math::vector2df *sites=csites.GetRingSites(iRing,c);

				if (c>1)
				{
					CircumSlotIdx iStart=(CircumSlotIdx)(sites-start);
					CircumSlotIdx iEnd=(CircumSlotIdx)(iStart+c-1);
					CircumSlotIdx iMe=i;

					if(iMe==iStart)
						slot->prev=iEnd;
					else
						slot->prev=iMe-1;

					if(iMe==iEnd)
						slot->next=iStart;
					else
						slot->next=iMe+1;
				}

				slot->nSearchRange=(WORD)(c/12);//两侧各30度范围内

			}
		}
	}

	//寻找neighbours,以及esacpes
	float distNb=gap*1.5f;
	for (int i=0;i<_slots.size();i++)
	{
		i_math::vector2df &pos=_slots[i].pos;
		CircumSlot *entry=&_slots[i];
		float radianThis=atan2(0.0f-pos.y,0.0f-pos.x);//中心点到这个slot的angle
		float radius=pos.getLength();//到中心点的距离
		for (int j=0;j<_slots.size();j++)
		{
			if (i==j)
				continue;

			float radian=atan2(_slots[j].pos.y-pos.y,_slots[j].pos.x-pos.x);

			if (pos.getDistanceSQFrom(_slots[j].pos)<distNb*distNb)
			{
				if (entry->nNBs<ARRAY_SIZE(entry->nbs))
				{
					CircumSlot::NbIdx ni;
					ni.idx=j;
					ni.radian=radian;

					entry->nbs[entry->nNBs]=ni;
					entry->nNBs++;
					continue;
				}
			}

			//是否是Escapes
			float r=fabs(i_math::normalize_radian(radian-radianThis));
			if (r<i_math::Pi/4.0f)
			{//+/-45度之内
				float dist=pos.getDistanceFrom(_slots[j].pos);
				if (dist<radius+4.0f)
				{
					if (entry->nEscapes<ARRAY_SIZE(entry->escapes))
					{
						CircumSlotIdx idx=j;

						entry->escapes[entry->nEscapes]=idx;
						entry->nEscapes++;
						continue;
					}
				}
			}
		}

		//将escapes排序
		if (TRUE)
		{
			CircumSlotCompareInfo info;
			info.slots=&_slots[0];
			info.pos=pos;
			qsort_s(entry->escapes,entry->nEscapes,sizeof(CircumSlotIdx),CircumSlotCompare,&info);
		}
	}


	//创建lookup
	if (TRUE)
	{
		float w=rc.getWidth();
		if (rc.getHeight()>w)
			w=rc.getHeight();

		_rcLookUp.inflate(w/2.0f,w/2.0f,w/2.0f,w/2.0f);

		float step=gap/2.0f;
		_lenLookUp=1+(DWORD)(w/step);

		_lookup.resize(_lenLookUp*_lenLookUp);
		VEC_SET(_lookup,0);

		for (int i=0;i<_slots.size();i++)
		{
			i_math::vector2df &pos=_slots[i].pos;

			CircumSlot *&slot=_LookUp(pos);
			slot=&_slots[i];
		}
	}

	_gap=gap;
}

void CCircumSlots::Clear()
{
	_slots.clear();
	_lookup.clear();
}


CCircumSlots *CCircumSlots::GetSingleton()
{
	static CCircumSlots slots;
	return &slots;
}

CircumSlot *CCircumSlots::GetSlots(DWORD &c)
{
	c=_slots.size();
	return &_slots[0];
}



////////////////////////////////////////////////////////////////////////
//CLevelCircum2


void CLevelCircum2::Init(i_math::vector2df &center)
{
	_center=center;
	CCircumSlots *slots=CCircumSlots::GetSingleton();

	if (TRUE)
	{
		DWORD c;
		slots->GetSlots(c);
		_dithers.resize(c);
		VEC_SET(_dithers,0);
	}

	_gap=slots->GetGap();
	_stepDither=_gap/4.0f/100.0f;
}

void CLevelCircum2::Clear()
{
	std::hash_map<CircumSlotIdx,CircumNode*>::iterator it;
	for (it=_nodes.begin();it!=_nodes.end();it++)
		Safe_Class_Delete((*it).second);
	_nodes.clear();

	for (int i=0;i<_requests.size();i++)
		Safe_Class_Delete(_requests[i]);
	_requests.clear();

	_dithers.clear();
}

LevelCircumNodeHandle CLevelCircum2::Request(i_math::vector2df &pos)
{
	CircumNode *node=Class_New2(CircumNode);
	node->bRequest=1;
	node->posRequest=pos;
	_requests.push_back(node);

	return (LevelCircumNodeHandle)node;
}


void CLevelCircum2::_ToLS(i_math::vector2df &pos)
{
	pos-=_center;

	float dist=pos.getLength();
	if (dist<0.01f)
	{
		pos.set(0,0);
		return;
	}

	float rad=atan2(pos.y,pos.x);
	rad-=_rad;

	rad=i_math::wrap_radian(rad);

	pos.x=cos(rad)*dist;
	pos.y=sin(rad)*dist;
}

void CLevelCircum2::_ToWS(CircumSlot *slots,CircumSlotIdx idx,i_math::vector2df &pos)
{
	pos=slots[idx].pos;
	float rad=atan2(pos.y,pos.x);
	float dist=pos.getLength();
	rad+=_rad;

	pos.x=cos(rad)*dist;
	pos.y=sin(rad)*dist;

	pos+=_center;
	pos.x+=_stepDither*(float)_dithers[idx].x;
	pos.y+=_stepDither*(float)_dithers[idx].y;
}

void CLevelCircum2::_RemoveNode(CircumNode *node)
{
	std::hash_map<CircumSlotIdx,CircumNode*>::iterator it=_nodes.find(node->idxSlot);
	if (it!=_nodes.end())
		_nodes.erase(it);
}

void CLevelCircum2::_AddNode(CircumNode *node)
{
	assert(_nodes.find(node->idxSlot)==_nodes.end());
	_nodes[node->idxSlot]=node;
}

BOOL CLevelCircum2::_EscapeInnerMove(CircumNode *node,CircumSlot *slots,DWORD depth)
{
	// 	if (depth>6)
	// 		return FALSE;
	CircumSlot *slot=&slots[node->idxSlot];
	DWORD flags=0;

	BYTE mode=0;//0:倾向于向后,1:任意,2:倾向于向前
	if (node->bInnerBackward)
	{//这个Node要求往后靠
		if (_CheckInnerForward(slots,node->idxSlot))
		{//在向后的方向上靠前了
			mode=0;
		}
	}
	if (node->bInnerForward)
	{//这个Node要求往前靠
		if (_CheckInnerBackward(slots,node->idxSlot))
		{//在向前的方向上靠后了
			mode=2;
		}
	}

	CircumSlotIdx tries[32];
	DWORD nTries=0;

	if (mode==1)
	{//任意模式
		for (int i=0;i<slot->nNBs;i++)
		{
			CircumSlotIdx idxTry=slot->nbs[i].idx;

			if (!_CheckReachable(slots,idxTry))
				continue;

			if (_CheckObstacle(slots,idxTry))
				continue;//不能Escape到一个和center有障碍的slot

			if (_nodes.find(idxTry)==_nodes.end())
			{//空位置
				//占领它
				_OccupySlot(node,idxTry);
				return TRUE;
			}

			tries[nTries]=idxTry;
			nTries++;
		}
	}
	else
	{//倾向向前或向后模式
		DWORD flags=0;

		//先考虑倾向方向上的NB
		for (int i=0;i<slot->nNBs;i++)
		{
			CircumSlotIdx idxTry=slot->nbs[i].idx;

			float rad=slot->nbs[i].radian+_rad;
			if (mode==2)
			{//倾向于向前模式

				if (i_math::get_radian_dist(rad,_radForward)>=(i_math::Pi/2.0f*0.95f))
				{
					flags|=(1<<i);//标记为反方向的NB
					continue;//不是center移动方向
				}
			}
			else
			{//倾向于向后模式
				if (i_math::get_radian_dist(rad,_radBackward)>=(i_math::Pi/2.0f*0.95f))
				{
					flags|=(1<<i);//标记为反方向的NB
					continue;//不是center移动方向
				}
			}

			if (!_CheckReachable(slots,idxTry))
				continue;

			if (_CheckObstacle(slots,idxTry))
				continue;//不能Escape到一个和center有障碍的slot

			if (_nodes.find(idxTry)==_nodes.end())
			{//空位置
				//占领它
				_OccupySlot(node,idxTry);
				return TRUE;
			}

			tries[nTries]=idxTry;
			nTries++;
		}

		//再考虑反方向的NB
		for (int i=0;i<slot->nNBs;i++)
		{
			CircumSlotIdx idxTry=slot->nbs[i].idx;

			if (!(flags&(1<<i)))
				continue;

			if (!_CheckReachable(slots,idxTry))
				continue;

			if (_CheckObstacle(slots,idxTry))
				continue;//不能Escape到一个和center有障碍的slot

			if (_nodes.find(idxTry)==_nodes.end())
			{//空位置
				//占领它
				_OccupySlot(node,idxTry);
				return TRUE;
			}

			tries[nTries]=idxTry;
			nTries++;
		}
	}

	if (nTries>0)
	{
		//neighbours里找不到空位置,要递归的Escape
		for (int i=0;i<nTries;i++)
		{
			CircumSlotIdx idxTry=tries[i];
			std::hash_map<CircumSlotIdx,CircumNode*>::iterator it=_nodes.find(idxTry);
			CircumNode *nodeTry=(*it).second;

			_AddBlock(idxTry);//锁定idxTry
			if (_EscapeInnerMove(nodeTry,slots,depth+1))//将nodeTry移到另一个Reachable的slot去
			{
				//idxTry已经被让出,node占领这个slot
				_OccupySlot(node,idxTry);
				return TRUE;
			}
			else
			{//nodeTry无法移到其它地方
				nodeTry->bProcessed=TRUE;//nodeTry位于一个Reachable的slot上,并且无法移到另一个Reachable的slot上去
			}
		}
	}

	return FALSE;
}



BOOL CLevelCircum2::_Escape(CircumNode *node,CircumSlot *slots,DWORD depth)
{
// 	if (depth>6)
// 		return FALSE;
	CircumSlot *slot=&slots[node->idxSlot];
	DWORD flags=0;

	for (int i=0;i<slot->nNBs;i++)
	{
		CircumSlotIdx idxTry=slot->nbs[i].idx;

		if (!_CheckReachable(slots,idxTry))
			continue;

		if (_CheckObstacle(slots,idxTry))
			continue;//不能Escape到一个和center有障碍的slot

		if (_nodes.find(idxTry)==_nodes.end())
		{//空位置
			//占领它
			_OccupySlot(node,idxTry);
			return TRUE;

		}

		flags|=(1<<i);//标记为可以再次尝试的
	}

	if (flags!=0)
	{
		//neighbours里找不到空位置,要递归的Escape
		for (int i=0;i<slot->nNBs;i++)
		{
			if (!(flags&(1<<i)))
				continue;
			CircumSlotIdx idxTry=slot->nbs[i].idx;
			std::hash_map<CircumSlotIdx,CircumNode*>::iterator it=_nodes.find(idxTry);
			CircumNode *nodeTry=(*it).second;

			_AddBlock(idxTry);//锁定idxTry
			if (_Escape(nodeTry,slots,depth+1))//将nodeTry移到另一个Reachable的slot去
			{
				//idxTry已经被让出,node占领这个slot
				_OccupySlot(node,idxTry);
				return TRUE;
			}
			else
			{//nodeTry无法移到其它地方
				nodeTry->bProcessed=TRUE;//nodeTry位于一个Reachable的slot上,并且无法移到另一个Reachable的slot上去
			}
		}
	}

	for (int i=0;i<slot->nEscapes;i++)
	{
		CircumSlotIdx idxTry=slot->escapes[i];

		if (!_CheckReachable(slots,idxTry))
			continue;

		if (_CheckObstacle(slots,idxTry))
			continue;//不能Escape到一个和center有障碍的slot

		std::hash_map<CircumSlotIdx,CircumNode*>::iterator it=_nodes.find(idxTry);
		if (it==_nodes.end())
		{//空位置
			//占领它
			_OccupySlot(node,idxTry);
			return TRUE;
		}

		_AddBlock(idxTry);
		CircumNode *nodeTry=(*it).second;
		if (_Escape(nodeTry,slots,depth+1))
		{
			//idxTry已经被让出,node占领这个slot
			_OccupySlot(node,idxTry);

			return TRUE;
		}
		else
		{//nodeTry无法移到其它地方
			nodeTry->bProcessed=TRUE;//nodeTry位于一个Reachable的slot上,并且无法移到另一个Reachable的slot上去
		}
	}


	return FALSE;
}

BOOL CLevelCircum2::_Escape2(CircumNode *node)
{
	DWORD c;
	CircumSlot *slots=CCircumSlots::GetSingleton()->GetSlots(c);
	for (int i=0;i<c;i++)
	{
		CircumSlotIdx idxTry=(CircumSlotIdx)i;
		CircumSlot *slot=&slots[idxTry];
		if (slot->iRing>_iCloseRing)
			return FALSE;

		if (!_CheckObstacle(slots,idxTry))
		{//这个slot和center没有阻碍,表示这个slot所在的Ring不是封闭的ring

			//把close ring设为这个slot的外圈
			if (slot->iRing+1>_iCloseRing)
				_iCloseRing=slot->iRing+1;
		}
		if (!_CheckReachable(slots,idxTry))
			continue;
		if (_nodes.find(idxTry)!=_nodes.end())
			continue;

		_OccupySlot(node,idxTry);

		return TRUE;
	}

	return FALSE;
}

BOOL CLevelCircum2::_CheckObstacle(CircumSlot *slots,CircumSlotIdx idx)
{
	if (_IsObstacle(idx))
		return TRUE;
	if (_IsUnobstacle(idx))
		return FALSE;

	i_math::vector2df pos;
	_ToWS(slots,idx,pos);
	if (_IsObstacle(pos))
	{
		_AddObstacle(idx);
		return TRUE;
	}
	else
	{
		_AddUnobstacle(idx);
		return FALSE;
	}

}


BOOL CLevelCircum2::_CheckReachable(CircumSlot *slots,CircumSlotIdx idx)
{
	if (_IsBlock(idx))
		return FALSE;
	if (_IsReachable(idx))
		return TRUE;

	//这个slot尚未检测过
	i_math::vector2df pos;
	_ToWS(slots,idx,pos);
	if (!_IsWalkable(pos))
	{//不可走
		_AddBlock(idx);
		return FALSE;
	}
	if (!_IsObstacle(pos))
	{//可走且与中心点无障碍
		_AddReachable(idx);
		return TRUE;
	}

	//可走且与中心点有障碍,我们测试周围临近的几个slot
	CircumSlot *slot=&slots[idx];

	BOOL bReachable=FALSE;

	//沿一个方向寻找
	CircumSlotIdx idxTry=slot->next;
	for (int i=0;i<slot->nSearchRange;i++)
	{
		if (!_CheckObstacle(slots,idxTry))
		{//找到一个与center没有障碍的slot
			i_math::vector2df pos2;
			_ToWS(slots,idxTry,pos2);

			if (_IsObstacle(pos,pos2))
				break;
			bReachable=TRUE;
			break;
		}

		idxTry=slots[idxTry].next;
	}

	if (!bReachable)
	{
		//沿另一个方向寻找
		idxTry=slot->prev;
		for (int i=0;i<slot->nSearchRange;i++)
		{
			if (!_CheckObstacle(slots,idxTry))
			{//找到一个与center没有障碍的slot
				i_math::vector2df pos2;
				_ToWS(slots,idxTry,pos2);

				if (_IsObstacle(pos,pos2))
					break;
				bReachable=TRUE;
				break;
			}

			idxTry=slots[idxTry].prev;
		}
	}

	if (!bReachable)
	{
		_AddBlock(idx);
		return FALSE;
	}
	else
	{
		_AddReachable(idx);
		return TRUE;
	}
}

void CLevelCircum2::_UpdateInnerInfo(CircumSlot *slots,CircumSlotIdx idx)
{
	if (!_bInnerMove)
	{
		_SetInnerInfo(idx,FALSE,FALSE);
		return;
	}
	i_math::vector2df pos;
	_ToWS(slots,idx,pos);

	i_math::vector2df dir=pos-_center;
	dir.normalize();

	float d;
	d=dir.dotProduct(_dirForward);
	BOOL bInnerBackward=FALSE;
	if (d<0.15f)
		bInnerBackward=TRUE;//在向前的方向上靠后了

	d=dir.dotProduct(_dirBackward);
	BOOL bInnerForward=FALSE;
	if (d<0.15f)
		bInnerForward=TRUE;//在向后的方向上靠前了

	_SetInnerInfo(idx,bInnerForward,bInnerBackward);
}


BOOL CLevelCircum2::_CheckInnerBackward(CircumSlot *slots,CircumSlotIdx idx)
{
	if (!_cacheSlots[idx].bInnerInfo)
		_UpdateInnerInfo(slots,idx);
	return _cacheSlots[idx].bInnerBackward;
}

BOOL CLevelCircum2::_CheckInnerForward(CircumSlot *slots,CircumSlotIdx idx)
{
	if (!_cacheSlots[idx].bInnerInfo)
		_UpdateInnerInfo(slots,idx);
	return _cacheSlots[idx].bInnerForward;
}

void CLevelCircum2::UpdateCenter(i_math::vector2df &center)
{
	CCircumSlots *slots=CCircumSlots::GetSingleton();
	BOOL bCenterMod=FALSE;
	float distMove=0.0f;
	//更新_center和_rad
	if (TRUE)
	{
		i_math::vector2df dir=center-_center;
		distMove=dir.getLength();

		if (!(center==_center))
			bCenterMod=TRUE;

		if (distMove>0.1f)
		{
			float rad=atan2(dir.y,dir.x);

			if (_radLast>100.0f)
				_radLast=rad;

			if (fabs(i_math::normalize_radian(rad-_radLast))<i_math::Pi/12.0f)//夹角小于15度
				_rad+=rad-_radLast;
			_radLast=rad;
		}

		_center=center;
	}

	//Flush _requests
	if (TRUE)
	{
		DWORD c;
		CircumSlot *buf=slots->GetSlots(c);

		for (int i=0;i<c;i++)
		{
			if (_requests.size()<=0)
				break;
			CircumSlot *slot=&buf[i];
			CircumSlotIdx idx=(CircumSlotIdx)i;

			if (_nodes.find(idx)!=_nodes.end())
				continue;

			_dithers[idx].x=CSysRandom::RandRangeInt(-100,100);
			_dithers[idx].y=CSysRandom::RandRangeInt(-100,100);

			i_math::vector2df pos;
			_ToWS(buf,idx,pos);

			if (!_IsWalkable(pos))
				continue;

			//一个可用的slot,找最近的request
			float dist2Min=100000000.0f;
			int iClosest=-1;
			for (int k=0;k<_requests.size();k++)
			{
				i_math::vector2df posRequest=_requests[k]->posRequest;
				float dist2=posRequest.getDistanceSQFrom(pos);
				if (dist2<dist2Min)
				{
					dist2Min=dist2;
					iClosest=k;
				}
			}

			_requests[iClosest]->bRequest=FALSE;
			_requests[iClosest]->idxSlot=idx;
			_nodes[idx]=_requests[iClosest];
			_requests[iClosest]=_requests[_requests.size()-1];
			_requests.pop_back();

		}
	}

	//随机选择几个slot,更新它们的dither
//	if (FALSE)
	if (bCenterMod)
	{
		if (_nLastSlots>0)
		{
			DWORD nDither=_nLastSlots/8;
			if (nDither<=0)
				nDither=1;

			DWORD idx=CSysRandom::RandRangeInt<DWORD>(0,_nLastSlots);
			extern int GenPrimeStep();
			DWORD step=(DWORD)GenPrimeStep();

			int varyDither=(int)(distMove/4.0f/_stepDither);

			for (int i=0;i<nDither;i++)
			{
				if (idx<_dithers.size())
				{
					int varyX=CSysRandom::RandVaryUInt(0,varyDither);
					int varyY=CSysRandom::RandVaryUInt(0,varyDither);

					int v=((int)_dithers[idx].x)+varyX;
					i_math::clamp_i(v,-100,100);
					_dithers[idx].x=(char)v;

					v=((int)_dithers[idx].y)+varyY;
					i_math::clamp_i(v,-100,100);
					_dithers[idx].y=(char)v;
				}

				idx=(idx+step)%_nLastSlots;
			}

		}
	}

	//重新调整队列,以使所有的node都处于可走位置上
//	if (bCenterMod)
	if (TRUE)
	{
		DWORD c;
		CircumSlot *buf=slots->GetSlots(c);

		_temp.clear();
		_temp.reserve(_nodes.size());
		std::hash_map<CircumSlotIdx,CircumNode*>::iterator it;
		for (it=_nodes.begin();it!=_nodes.end();it++)
			_temp.push_back((*it).second);

		BOOL bFailEscape2=FALSE;
		_iCloseRing=2;

		for (int i=0;i<_temp.size();i++)
		{
			CircumNode *node=_temp[i];

			if (node->bProcessed)//bProcess表示,这个node已经找到(或者本来就位于)一个Reachable的slot了.
				continue;

			if (_CheckReachable(buf,node->idxSlot))
			{
				if (_bInnerMove)
				{
					if (node->bInnerForward)
					{//倾向靠前
						if (_CheckInnerBackward(buf,node->idxSlot))
						{//在靠后的位置
							_EscapeInnerMove(node,buf,0);
							node->bProcessed=TRUE;
							continue;
						}
					}
					if (node->bInnerBackward)
					{//倾向靠后
						if (_CheckInnerForward(buf,node->idxSlot))
						{//在靠前的位置
							_EscapeInnerMove(node,buf,0);
							node->bProcessed=TRUE;
							continue;
						}
					}
				}

				CircumSlot *slot=&buf[node->idxSlot];
				//尝试向内圈移动
				if (slot->iRing>0)
				{
					CircumSlotIdx idx=slot->inner;
					if (slot->iRing>1)
					{
						CircumSlot *slotInner=&buf[slot->inner];
						int iChoose=CSysRandom::RandRangeInt(0,3);
						switch(iChoose)
						{
							case 0:
								idx=slotInner->prev;
								break;
							case 2:
								idx=slotInner->next;
								break;
						}
					}

					if (idx!=CircumSlotIdx_Invalid)
					{
						if (_nodes.find(idx)==_nodes.end())
						{
							if (_CheckReachable(buf,idx))
								_OccupySlot(node,idx);
						}
					}
				}

				node->bProcessed=TRUE;
				continue;
			}

			if (!_Escape(node,buf,0))//将node移到另一个Reachable的slot上去
			{
				ProfilerStart_Recent(_Escape2);
				if (!bFailEscape2)
				{
					if (!_Escape2(node))
						bFailEscape2=TRUE;//Escape2失败一次,以后应该都会失败了
				}
				ProfilerEnd();
			}
			if (!node->bProcessed)
			{
				int v=0;
				v++;
			}

		}

		//清除Process标志
		for (int i=0;i<_temp.size();i++)
		{
			CircumNode *node=_temp[i];
			node->bProcessed=FALSE;
		}

		//更新_nLastSlots;
		_nLastSlots=_nSlotCache;

		_ClearSlotCache();
	}

}

void CLevelCircum2::Discard(LevelCircumNodeHandle h)
{
	CircumNode*node=(CircumNode*)h;
	if (node->bRequest)
	{
		VEC_REMOVE_SWAP(_requests,node);
		Safe_Class_Delete(node);
		return;
	}

	std::hash_map<CircumSlotIdx,CircumNode*>::iterator it=_nodes.find(node->idxSlot);
	if (it!=_nodes.end())
	{
		assert(node==(*it).second);
		Safe_Class_Delete(node);
		_nodes.erase(it);
	}
	else
	{
		assert(FALSE);
	}
}

BOOL CLevelCircum2::IsRequesting(LevelCircumNodeHandle h)
{
	CircumNode*node=(CircumNode*)h;
	if (node->bRequest)
		return TRUE;
	return FALSE;
}

void CLevelCircum2::SetInnerForward(LevelCircumNodeHandle h,BOOL bForward)
{
	CircumNode*node=(CircumNode*)h;
	node->bInnerForward=bForward?1:0;
	if (bForward)
		node->bInnerBackward=0;
}

void CLevelCircum2::SetInnerBackward(LevelCircumNodeHandle h,BOOL bBackward)
{
	CircumNode*node=(CircumNode*)h;
	node->bInnerBackward=bBackward?1:0;
	if (bBackward)
		node->bInnerForward=0;
}

void CLevelCircum2::SetInnerMove(float radForward,float radBackward)
{
	_bInnerMove=TRUE;
	_radForward=radForward;
	_dirForward.set(cos(radForward),sin(radForward));
	_radBackward=radBackward;
	_dirBackward.set(cos(radBackward),sin(radBackward));
}

void CLevelCircum2::ClearInnerMove()
{
	_bInnerMove=FALSE;
}



BOOL CLevelCircum2::GetPos(LevelCircumNodeHandle h,i_math::vector2df &pos)
{
	CircumNode*node=(CircumNode*)h;
	if (node->bRequest)
		pos=node->posRequest;
	else
	{
		CCircumSlots *slots=CCircumSlots::GetSingleton();
		CircumSlot *slot=slots->GetSlot(node->idxSlot);
		if (!slot)
			return FALSE;

		DWORD c;
		CircumSlot *buf=slots->GetSlots(c);
		_ToWS(buf,node->idxSlot,pos);
	}
	return TRUE;

}
