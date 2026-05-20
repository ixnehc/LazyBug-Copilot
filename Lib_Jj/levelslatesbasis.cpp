
#include "stdh.h"

#include <unordered_map>

#include "LevelBasis.h"
#include "LevelSlatesBasis.h"

#include "LevelRecords.h"

#include "Log/LogDump.h"

#include "LoSlates.h"
#include "LoSlate.h"

//////////////////////////////////////////////////////////////////////////
//LevelSlateInfo
LevelGUID LevelSlateInfo::GetUID()
{
	return src?src->GetGUID():LevelGUID_Invalid;
}


//////////////////////////////////////////////////////////////////////////
//LevelSlatesInfo
LevelGUID LevelSlatesInfo::GetUID()
{
	return src?src->GetGUID():LevelGUID_Invalid;
}


//////////////////////////////////////////////////////////////////////////
//CLevelSlatesBasis

void CLevelSlatesBasis::Clear()
{
	_bufSlates.clear();
	_bufSlate.clear();
	_lookupSlate.clear();
}

void CLevelSlatesBasis::_BuildMatrix(LevelSlatesInfo &infoSlates)
{
	std::deque<LevelSlateInfo *> pool;

	i_math::recti rcRange;

	if (TRUE)
	{
		LevelSlateInfo*infoSlate=GetSlateInfo(infoSlates.iStartSlate);
		infoSlate->pt.set(10000,10000);

		pool.push_back(infoSlate);
		rcRange.merge(infoSlate->pt.x,infoSlate->pt.y);
	}

	while(pool.size()>0)
	{
		LevelSlateInfo *info=pool[0];
		pool.pop_front();

		LevelPos pos=info->src->GetPos();

		for (int i=0;i<info->nLinks;i++)
		{
			LevelSlateIdx idxNb=info->links[i];
			if (idxNb==LevelSlateIdx_Invalid)
				continue;
			LevelSlateInfo *infoNb=GetSlateInfo(idxNb);
			if (!infoNb)
				continue;
			if (infoNb->pt.x>=0)
				continue;

			LevelPos posNb=infoNb->src->GetPos();

			LevelPos dir=posNb-pos;

			float dx=dir.dotProduct(infoSlates.dirH);
			float dy=dir.dotProduct(infoSlates.dirV);

			infoNb->pt=info->pt;

			if (dx>0.4f)
				infoNb->pt.x++;
			if (dx<-0.4f)
				infoNb->pt.x--;
			if (dy>0.4f)
				infoNb->pt.y++;
			if (dy<-0.4f)
				infoNb->pt.y--;

			pool.push_back(infoNb);
			rcRange.merge(infoNb->pt.x,infoNb->pt.y);
		}
	}

	int xBase,yBase;
	xBase=rcRange.Left();
	yBase=rcRange.Top();
	int w=rcRange.getWidth();

	infoSlates.matrix.resize(rcRange.getArea());
	for (int i=0;i<infoSlates.matrix.size();i++)
		infoSlates.matrix[i]=LevelSlateIdx_Invalid;

	for (int i=0;i<infoSlates.countSlates;i++)
	{
		LevelSlateInfo*infoSlate=GetSlateInfo(infoSlates.iStartSlate+i);
		if (infoSlate)
		{
			if (infoSlate->pt.x<0)
				continue;

			infoSlate->pt.x-=xBase;
			infoSlate->pt.y-=yBase;

			infoSlates.matrix[infoSlate->pt.y*w+infoSlate->pt.x]=infoSlates.iStartSlate+i;
		}
	}

	infoSlates.wMatrix=rcRange.getWidth();
	infoSlates.hMatrix=rcRange.getHeight();
}


void CLevelSlatesBasis::Build(CLevelSources *srces)
{
	int sz=srces->_buf.size();

	_bufSlates.clear();
	_bufSlate.clear();
	for (int i=0;i<sz;i++)
	{
		CLevelObjSrc *src=srces->_buf[i].src;
		CLevelObjParam *param=srces->_buf[i].param;

		if (src)
		{
			if (src->IsDisable())
				continue;
		}

		if (param)
		{
			if (param->IsDisable())
				continue;
		}

		if (src)
		{
			if (src->GetClass()->CheckName("LosSlatesA"))
			{
				LevelSlatesInfo *infoSlates=&_bufSlates[src->GetGUID()];

				infoSlates->src=(LosSlates*)src;
				assert(param->GetClass()->CheckName("LopSlatesA"));
				infoSlates->param=(LopSlates*)param;
			}
			if (src->GetClass()->CheckName("LosSlatesB"))
			{
				LevelSlatesInfo *infoSlates=&_bufSlates[src->GetGUID()];

				infoSlates->src=(LosSlates*)src;
				assert(param->GetClass()->CheckName("LopSlatesB"));
				infoSlates->param=(LopSlates*)param;
			}
			if (src->GetClass()->CheckName("LosSlate"))
			{
				assert(param->GetClass()->CheckName("LopSlate"));
				_bufSlate.resize(_bufSlate.size()+1);
				LevelSlateInfo *info=&_bufSlate[_bufSlate.size()-1];

				info->src=(LosSlate*)src;
				info->param=(LopSlate*)param;
				info->tpDef=LevelSlateType_None;
				info->idxMe=_bufSlate.size()-1;

				_lookupSlate[info->src->GetGUID()]=_bufSlate.size()-1;
			}
		}
	}

	//建立slate之间的连接
	const float maxSlateRadius=10.0f;
	const float maxLinkDist=1.0f;
	float radius2Slate=(maxSlateRadius+maxLinkDist)*(maxSlateRadius+maxLinkDist);
	for (int i=0;i<_bufSlate.size();i++)
	{
		LevelSlateInfo *info=&_bufSlate[i];

		i_math::matrix43f &mat=info->src->GetMat();

		for (int j=0;j<info->src->links.size();j++)
		{
			i_math::vector3df pos=info->src->links[j].getTranslation();
			mat.transformVect(pos,pos);

			float minLinkDist2=10000000000.0f;
			LevelSlateInfo *infoClosestSlate=NULL;

			for (int m=0;m<_bufSlate.size();m++)
			{
				if (m==i)
					continue;
				LevelSlateInfo *infoCheck=&_bufSlate[m];
				if (infoCheck->src->GetMat().getTranslationP()->getDistanceXZFromSQ(pos)>radius2Slate)
					continue;//足够远,

				i_math::matrix43f &matCheck=infoCheck->src->GetMat();

				for (int n=0;n<infoCheck->src->links.size();n++)
				{
					i_math::vector3df posCheck=infoCheck->src->links[n].getTranslation();
					matCheck.transformVect(posCheck,posCheck);
					float dist2=pos.getDistanceXZFromSQ(posCheck);
					if (dist2>minLinkDist2)
						continue;

					minLinkDist2=dist2;
					infoClosestSlate=infoCheck;
				}
			}

			if (infoClosestSlate)
			{
				if (minLinkDist2<maxLinkDist*maxLinkDist)
				{
					//这两个slate可以连接,连接它们
					info->AddLink(infoClosestSlate->idxMe);
					infoClosestSlate->AddLink(info->idxMe);
				}
			}

		}

	}

	//建立Slates
	if (TRUE)
	{
		std::deque<LevelSlateInfo*> q;
		std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it;
		for (it=_bufSlates.begin();it!=_bufSlates.end();it++)
		{
			LevelSlatesInfo *info=&(*it).second;

			for (int i=0;i<info->param->includes.size();i++)
			{
				LevelSlateInfo *infoSlate=FindSlateInfo(info->param->includes[i]);
				if (infoSlate)
					q.push_back(infoSlate);
			}

			while(!q.empty())
			{
				LevelSlateInfo *infoSlate=q[0];
				q.pop_front();

				if (infoSlate->uidOwner!=LevelGUID_Invalid)
					continue;

				infoSlate->uidOwner=info->src->GetGUID();

				for (int i=0;i<infoSlate->nLinks;i++)
				{
					LevelSlateIdx idx=infoSlate->links[i];
					if (((DWORD)idx)<_bufSlate.size())
					{
						LevelSlateInfo *info2=&_bufSlate[idx];
						if (info2->uidOwner==LevelGUID_Invalid)
							q.push_back(info2);
					}
				}
			}
		}
	}

	//建立Slates的坐标系
	if (TRUE)
	{
		std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it;
		for (it=_bufSlates.begin();it!=_bufSlates.end();it++)
		{
			LevelSlatesInfo *info=&(*it).second;

			SlateSpaceDefine &space=info->param->space;

			if ((space.base.size()>0)&&(space.axisX.size()>0)&&(space.axisY.size()>0))
			{
				LevelSlateInfo *base=FindSlateInfo(space.base[0]);
				LevelSlateInfo *axisX=FindSlateInfo(space.axisX[0]);
				LevelSlateInfo *axisY=FindSlateInfo(space.axisY[0]);

				if (base&&axisX&&axisY)
				{
					LevelPos o=base->src->GetPos();
					LevelPos x=axisX->src->GetPos();
					LevelPos y=axisY->src->GetPos();

					info->dirH=x-o;
					info->dirV=y-o;

					info->dirH.safe_normalize();
					info->dirV.safe_normalize();
				}
			}
		}

	}

	//重新调整_bufSlate,使他们按照Slates排列
	if (TRUE)
	{
		std::vector<LevelSlateIdx> remap;
		remap.resize(_bufSlate.size());
		std::deque<LevelSlateInfo> bufSlateNew;
		
		std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it;
		for (it=_bufSlates.begin();it!=_bufSlates.end();it++)
		{
			LevelSlatesInfo *info=&(*it).second;
			LevelGUID uid=info->GetUID();

			info->iStartSlate=bufSlateNew.size();
			info->countSlates=0;

			for (int i=0;i<_bufSlate.size();i++)
			{
				if (_bufSlate[i].uidOwner==uid)
				{
					bufSlateNew.push_back(_bufSlate[i]);
					remap[i]=bufSlateNew.size()-1;
					info->countSlates++;
				}
			}
		}
		for (int i=0;i<_bufSlate.size();i++)
		{
			if (_bufSlate[i].uidOwner==LevelGUID_Invalid)
			{
				bufSlateNew.push_back(_bufSlate[i]);
				remap[i]=bufSlateNew.size()-1;
			}
		}

		assert(_bufSlate.size()==bufSlateNew.size());

		for (int i=0;i<bufSlateNew.size();i++)
		{
			LevelSlateInfo *info=&bufSlateNew[i];
			info->idxMe=remap[info->idxMe];
			for (int j=0;j<info->nLinks;j++)
				info->links[j]=remap[info->links[j]];
		}

		std::unordered_map<LevelGUID,LevelSlateIdx>::iterator it2;
		for (it2=_lookupSlate.begin();it2!=_lookupSlate.end();it2++)
			(*it2).second=remap[(*it2).second];
		
		_bufSlate.swap(bufSlateNew);
	}

	//Grp信息
	if (TRUE)
	{
		std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it;
		for (it=_bufSlates.begin();it!=_bufSlates.end();it++)
		{
			LevelSlatesInfo *info=&(*it).second;
			LevelGUID uid=info->GetUID();

			for (int i=0;i<info->param->grps.size();i++)
			{
				SlateGrpEntry *e=&info->param->grps[i];
				LevelSlatesInfo::Grp grp;
				grp.iStart=info->indicesGrp.size();
				grp.count=0;

				for (int j=0;j<e->refs.size();j++)
				{
					LevelSlateIdx idx=FindSlateIdx(e->refs[j]);
					if (idx!=LevelSlateIdx_Invalid)
					{
						info->indicesGrp.push_back(idx);
						grp.count++;
					}
				}

				info->grps[e->nm]=grp;
			}
		}
	}

	//二维阵列
	if (TRUE)
	{
		std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it;
		for (it=_bufSlates.begin();it!=_bufSlates.end();it++)
			_BuildMatrix((*it).second);
	}

}


LevelSlateInfo *CLevelSlatesBasis::FindSlateInfo(LevelGUID uid)
{
	std::unordered_map<LevelGUID,LevelSlateIdx>::iterator it=_lookupSlate.find(uid);
	if (it==_lookupSlate.end())
		return NULL;

	LevelSlateIdx idx=(*it).second;
	if (((DWORD)idx)>=_bufSlate.size())
		return NULL;

	return &_bufSlate[idx];
}


LevelSlateIdx CLevelSlatesBasis::FindSlateIdx(LevelGUID uid)
{
	std::unordered_map<LevelGUID,LevelSlateIdx>::iterator it=_lookupSlate.find(uid);
	if (it==_lookupSlate.end())
		return LevelSlateIdx_Invalid;

	return (*it).second;
}

LevelSlatesInfo *CLevelSlatesBasis::FindSlatesInfo(LevelGUID uid)
{
	std::unordered_map<LevelGUID,LevelSlatesInfo>::iterator it=_bufSlates.find(uid);
	if (it!=_bufSlates.end())
		return &(*it).second;
	return NULL;
}

LevelSlateInfo *CLevelSlatesBasis::GetSlateInfo(LevelSlateIdx idx)
{
	if ((DWORD)idx<_bufSlate.size())
		return &_bufSlate[idx];
	return NULL;
}

