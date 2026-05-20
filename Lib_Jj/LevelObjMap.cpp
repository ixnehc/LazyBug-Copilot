
#include "stdh.h"

#include "LevelRecordUnit.h"
#include "LoUnit.h"
#include "LevelObjMap.h"

#include "LevelDetectWeights.h"

#include "commondefines/general_stl.h"

#include "timer/profiler.h"

#include "Level.h"
#include "LevelObj.h"


void CLevelObjMap::Create(CLevel *lvl,i_math::recti &rcMap)
{
	_lvl=lvl;

	_xStart=(float)rcMap.Left();
	_yStart=(float)rcMap.Top();

	_wTile=(int)(((float)rcMap.getWidth())/(float)LEVELOBJMAP_TILE_LEN)+1;
	_hTile=(int)(((float)rcMap.getHeight())/(float)LEVELOBJMAP_TILE_LEN)+1;

	_w=(_wTile+LEVELOBJMAP_TILE_PER_BLOCK-1)/LEVELOBJMAP_TILE_PER_BLOCK;
	_h=(_hTile+LEVELOBJMAP_TILE_PER_BLOCK-1)/LEVELOBJMAP_TILE_PER_BLOCK;

}

void CLevelObjMap::Destroy()
{
	BOOL bEmpty=TRUE;
	for (int j=0;j<ARRAY_SIZE(_blocks);j++)
	{
		for (int i=0;i<_blocks[j].size();i++)
		{
			CLevelObjBlock *blk=_blocks[j][i];
			if (bEmpty)
			{
				if (blk)
				{
					if (blk->_nLevelObjs>0)
						bEmpty=FALSE;
					else
					{
						for (int j=0;j<ARRAY_SIZE(blk->_tiles);j++)
						{
							if (blk->_tiles[j].objs.size()>0)
							{
								bEmpty=FALSE;
								break;
							}
						}
					}
				}
			}
			Safe_Class_Delete(blk);
		}
		assert(bEmpty);
		_blocks[j].clear();
	}

	for (int i=0;i<ARRAY_SIZE(_alls);i++)
		_alls[i].clear();

}

DWORD BlockIdxFromLevelObj(CLevelObj *lo)
{
	LevelObjType tp=lo->GetType();
	if (tp==LevelObjType_Item)
		return LEVELOBJMAP_BLKIDX_ITEM;
	if (tp==LevelObjType_Agent)
		return LEVELOBJMAP_BLKIDX_AGENT;
	if (tp==LevelObjType_Unit)
	{
		LevelPlayerID idPlayer=lo->GetPlayerID();
		if (idPlayer!=LevelPlayerID_Invalid)
			return idPlayer;
	}
	return 0xffffffff;
}


void CLevelObjMap::AddLo(CLevelObj*lo)
{
	if (lo->_tile)
		return;//已经加入了

	LevelPos pos=lo->GetFramePos();
	unsigned int xTile,yTile;
	xTile=(unsigned int)((pos.x-_xStart)/LEVELOBJMAP_TILE_LEN);
	yTile=(unsigned int)((pos.y-_yStart)/LEVELOBJMAP_TILE_LEN);

	if ((xTile>=_wTile)||(yTile>=_hTile))
		return;//越界

	DWORD idx=BlockIdxFromLevelObj(lo);
	if (idx>=ARRAY_SIZE(_blocks))
		return;

	_alls[idx].insert(lo);

	unsigned int xBlk,yBlk;
	xBlk=xTile/LEVELOBJMAP_TILE_PER_BLOCK;
	yBlk=yTile/LEVELOBJMAP_TILE_PER_BLOCK;

	unsigned int xLocal,yLocal;
	xLocal=xTile%LEVELOBJMAP_TILE_PER_BLOCK;
	yLocal=yTile%LEVELOBJMAP_TILE_PER_BLOCK;

	if (_blocks[idx].size()<=0)
		_blocks[idx].resize(_w*_h);

	CLevelObjBlock *&blk=_blocks[idx][yBlk*_w+xBlk];
	if (!blk)
	{
		blk=Class_New2(CLevelObjBlock);
		blk->Init(xBlk,yBlk,idx);
	}

	LevelObjTile *tile=&blk->_tiles[yLocal*LEVELOBJMAP_TILE_PER_BLOCK+xLocal];
	tile->objs.push_back(lo);
	lo->_tile=tile;

	blk->_nLevelObjs++;
}

void CLevelObjMap::RemoveLo(CLevelObj*lo)
{
	if (!lo->_tile)
		return;//没有加入过

	CLevelObjBlock *blk=lo->_tile->blk;

	LevelObjTile *tile=lo->_tile;

	if (TRUE)
	{
		std::set<CLevelObj *>::iterator it=_alls[tile->idxBlk].find(lo);
		if (it!=_alls[tile->idxBlk].end())
			_alls[tile->idxBlk].erase(it);
	}
	

	//遍历查找,删除
	std::deque<CLevelObj*>::iterator it;
	for (it=tile->objs.begin();it!=tile->objs.end();it++)
	{
		if ((*it)==lo)
		{
			(*it)=tile->objs[tile->objs.size()-1];
			tile->objs.pop_back();
			break;
		}
	}

	lo->_tile=NULL;

	blk->_nLevelObjs--;
}

void CLevelObjMap::UpdateLo(CLevelObj *lo)
{
	LevelObjTile *tile=lo->_tile;
	if (!tile)
		return;

	LevelPos pos=lo->GetFramePos();
	unsigned int xTile,yTile;
	xTile=(unsigned int)((pos.x-_xStart)/LEVELOBJMAP_TILE_LEN);
	if (xTile==tile->pos.x)
	{
		yTile=(unsigned int)((pos.y-_yStart)/LEVELOBJMAP_TILE_LEN);
		if (yTile==tile->pos.y)
		{
			DWORD idx=BlockIdxFromLevelObj(lo);
			if (idx==tile->idxBlk)
				return;
		}
	}

	RemoveLo(lo);
	AddLo(lo);
}

#define ENUMLOOP_BEGIN()																																\
for (DWORD i=rcTile.Left();i<rcTile.Right();i++)																									\
for (DWORD j=rcTile.Top();j<rcTile.Bottom();j++)																								\
{																																												\
	DWORD xBlk,yBlk;																																				\
	xBlk=i/LEVELOBJMAP_TILE_PER_BLOCK;																											\
	yBlk=j/LEVELOBJMAP_TILE_PER_BLOCK;																											\
	for (int k=0;k<_nBlkIdx;k++)																																\
	{																																											\
		BlkIdxInfo &info=_blkidx[k];																															\
		DWORD idx=info.idx;																																		\
		LevelMoveMethodMask methods=info.methods;																						\
		DWORD maskPlayer=info.maskPlayer;																										\
		DWORD flagsPlayerOrUnit=info.flagsPlayerOrUnit;																					\
		if (_blocks[idx].empty())																																	\
			continue;																																						\
		CLevelObjBlock *blk=_blocks[idx][yBlk*_w+xBlk];																						\
		if (blk)																																								\
		{																																										\
			DWORD xLocal,yLocal;																																\
			xLocal=i%LEVELOBJMAP_TILE_PER_BLOCK;																							\
			yLocal=j%LEVELOBJMAP_TILE_PER_BLOCK;																							\
			LevelObjTile *tile=&blk->_tiles[yLocal*LEVELOBJMAP_TILE_PER_BLOCK+xLocal];								\
			std::deque<CLevelObj*>::iterator it;																											\
			for (it=tile->objs.begin();it!=tile->objs.end();it++)																					\
			{																																									\
				CLevelObj *obj=(*it);																																\
				if (_IsIgnore(obj))																																	\
					continue;																																				\
				if (!_CheckAgent(obj))																															\
					continue;																																				\
				if (methods!=LevelMoveMethodMask_All)																							\
				{																																								\
					if (!(methods&obj->GetMoveMethodMask()))																				\
						continue;																																			\
				}																																								\
				if (!(obj->GetPlayerIDMask()&maskPlayer))																						\
					continue;																																				\
				if (!(flagsPlayerOrUnit&( obj->IsPlayer()?1:2)))																					\
					continue;																																				\
				if (!_CheckRequire(obj))																															\
					continue;


#define ENUMLOOP_END()																																	\
			}																																									\
		}																																										\
	}																																											\
}

BOOL CLevelObjMap::CheckInRange(CLevelObj *obj,float dist2,float radiusMin,float radiusMax,BOOL bTouching)
{
	if (!bTouching)
	{
		if ((dist2>radiusMax*radiusMax)||(dist2<radiusMin*radiusMin))
			return FALSE;
	}
	else
	{
		float rMax2=radiusMax+obj->GetRadius_();
		rMax2=rMax2*rMax2;
		float rMin2=radiusMin-obj->GetRadius_();
		if (rMin2<0.0f)
			rMin2=0.0f;
		rMin2=rMin2*rMin2;
		if ((dist2>rMax2)||(dist2<rMin2))
			return FALSE;
	}
	return TRUE;
}

BOOL CLevelObjMap::CheckInRange(CLevelObj *lo,i_math::rectf &rcRange,BOOL bTouching)
{
	if (!bTouching)
	{
		if(!rcRange.isPointInside(lo->GetFramePos()))
			return FALSE;
	}
	else
	{
		i_math::rectf rcRange2=rcRange;
		float radius=lo->GetRadius_();
		rcRange2.inflate(radius,radius,radius,radius);
		if(!rcRange2.isPointInside(lo->GetFramePos()))
			return FALSE;
	}

	return TRUE;
}



void CLevelObjMap::_Enum(LevelObjMapEnumCallBack dlgt)
{
	_enum.clear();
	if (_rcRange.Left()<0.0f)
	{
		_rcRange.Left()=0.0f;
		if (_rcRange.Right()<0.0f)
			_rcRange.Right()=0.0f;
	}
	if (_rcRange.Top()<0.0f)
	{
		_rcRange.Top()=0.0f;
		if (_rcRange.Bottom()<0.0f)
			_rcRange.Bottom()=0.0f;
	}

	i_math::recti rcTile;
	rcTile.Left()=(int)(_rcRange.Left()/LEVELOBJMAP_TILE_LEN);
	rcTile.Top()=(int)(_rcRange.Top()/LEVELOBJMAP_TILE_LEN);
	rcTile.Right()=(int)(_rcRange.Right()/LEVELOBJMAP_TILE_LEN)+1;
	rcTile.Bottom()=(int)(_rcRange.Bottom()/LEVELOBJMAP_TILE_LEN)+1;
	if (rcTile.Right()>_wTile)
		rcTile.Right()=_wTile;
	if (rcTile.Bottom()>_hTile)
		rcTile.Bottom()=_hTile;

	if (_radiusMax>0.0f)
	{
		i_math::vector2df center=*(i_math::vector2df*)&_rcRange.getCenter();
		center.x+=_xStart;
		center.y+=_yStart;

 		ENUMLOOP_BEGIN();

// for (DWORD i=rcTile.Left();i<rcTile.Right();i++)																								
// for (DWORD j=rcTile.Top();j<rcTile.Bottom();j++)																							
// {																																											
// 	DWORD xBlk,yBlk;																																			
// 	xBlk=i/LEVELOBJMAP_TILE_PER_BLOCK;																										
// 	yBlk=j/LEVELOBJMAP_TILE_PER_BLOCK;																										
// 	for (int k=0;k<_nBlkIdx;k++)																															
// 	{																																										
// 		BlkIdxInfo &info=_blkidx[k];																														
// 		DWORD idx=info.idx;																																	
// 		LevelMoveMethodMask methods=info.methods;																					
// 		DWORD maskPlayer=info.maskPlayer;																									
// 		DWORD flagsPlayerOrUnit=info.flagsPlayerOrUnit;																				
// 		if (_blocks[idx].empty())																																
// 			continue;																																					
// 		CLevelObjBlock *blk=_blocks[idx][yBlk*_w+xBlk];																					
// 		if (blk)																																							
// 		{																																									
// 			DWORD xLocal,yLocal;																															
// 			xLocal=i%LEVELOBJMAP_TILE_PER_BLOCK;																						
// 			yLocal=j%LEVELOBJMAP_TILE_PER_BLOCK;																						
// 			LevelObjTile *tile=&blk->_tiles[yLocal*LEVELOBJMAP_TILE_PER_BLOCK+xLocal];							
// 			std::deque<CLevelObj*>::iterator it;																										
// 			for (it=tile->objs.begin();it!=tile->objs.end();it++)																				
// 			{																																								
// 				CLevelObj *obj=(*it);																															
// 				if (_IsIgnore(obj))																																
// 					continue;																																			
// 				if (!_CheckAgent(obj))																														
// 					continue;																																			
// 				if (methods!=LevelMoveMethodMask_All)																						
// 				{																																							
// 					if (!(methods&obj->GetMoveMethodMask()))																			
// 						continue;																																		
// 				}																																							
// 				if (!(obj->GetPlayerIDMask()&maskPlayer))																					
// 					continue;																																			
// 				if (!(flagsPlayerOrUnit&( obj->IsPlayer()?1:2)))																				
// 					continue;																																			
// 				if (!_CheckRequire(obj))																														
// 					continue;

		float dist2=obj->GetFramePos().getDistanceSQFrom(center);
		if (!CheckInRange(obj,dist2,_radiusMin,_radiusMax,_bTouching))
 			continue;

		if (dlgt!=NULL)
		{
			if (!dlgt(obj,dist2))
				continue;
		}
		_enum.push_back(*it);


		ENUMLOOP_END();
	}
	else
	{
		i_math::rectf rcRange=_rcRange;
		rcRange+=i_math::pos2df(_xStart,_yStart);
		i_math::vector2df center=*(i_math::vector2df*)&rcRange.getCenter();
		ENUMLOOP_BEGIN();

		if (!CheckInRange(obj,rcRange,_bTouching))
			continue;

		if (dlgt)
		{
			float dist2=obj->GetFramePos().getDistanceSQFrom(center);
			if (!dlgt(obj,dist2))
				continue;
		}
		_enum.push_back(*it);

		ENUMLOOP_END();
	}
}

void CLevelObjMap::_EnumInAll(LevelObjMapEnumCallBack dlgt)
{
	_enum.clear();

	for (int k=0;k<_nBlkIdx;k++)																																
	{																																											
		BlkIdxInfo &info=_blkidx[k];																															
		DWORD idx=info.idx;																																		
		LevelMoveMethodMask methods=info.methods;																						
		DWORD maskPlayer=info.maskPlayer;																										
		DWORD flagsPlayerOrUnit=info.flagsPlayerOrUnit;																					
		std::set<CLevelObj *>::iterator it;																											
		for (it=_alls[idx].begin();it!=_alls[idx].end();it++)																					
		{																																									
			CLevelObj *obj=(*it);																																
			if (_IsIgnore(obj))																																	
				continue;																																				
			if (!_CheckAgent(obj))																															
				continue;																																				
			if (methods!=LevelMoveMethodMask_All)																							
			{																																								
				if (!(methods&obj->GetMoveMethodMask()))																				
					continue;																																			
			}																																								
			if (!(obj->GetPlayerIDMask()&maskPlayer))																						
				continue;																																				
			if (!(flagsPlayerOrUnit&( obj->IsPlayer()?1:2)))																					
				continue;																																				
			if (!_CheckRequire(obj))																															
				continue;

			if (dlgt!=NULL)
			{
				if (!dlgt(obj,0.0f))
					continue;
			}
			_enum.push_back(*it);
		}
	}
}

CLevelObj *CLevelObjMap::EnumFirst(LevelObjMapEnumCallBack dlgt)
{
	_enum.clear();
	if (_rcRange.Left()<0.0f)
	{
		_rcRange.Left()=0.0f;
		if (_rcRange.Right()<0.0f)
			_rcRange.Right()=0.0f;
	}
	if (_rcRange.Top()<0.0f)
	{
		_rcRange.Top()=0.0f;
		if (_rcRange.Bottom()<0.0f)
			_rcRange.Bottom()=0.0f;
	}

	i_math::recti rcTile;
	rcTile.Left()=(int)(_rcRange.Left()/LEVELOBJMAP_TILE_LEN);
	rcTile.Top()=(int)(_rcRange.Top()/LEVELOBJMAP_TILE_LEN);
	rcTile.Right()=(int)(_rcRange.Right()/LEVELOBJMAP_TILE_LEN)+1;
	rcTile.Bottom()=(int)(_rcRange.Bottom()/LEVELOBJMAP_TILE_LEN)+1;
	if (rcTile.Right()>_wTile)
		rcTile.Right()=_wTile;
	if (rcTile.Bottom()>_hTile)
		rcTile.Bottom()=_hTile;

	if (_radiusMax>0.0f)
	{
		i_math::vector2df center=*(i_math::vector2df*)&_rcRange.getCenter();
		center.x+=_xStart;
		center.y+=_yStart;

		ENUMLOOP_BEGIN();

		float dist2=obj->GetFramePos().getDistanceSQFrom(center);
		if (!CheckInRange(obj,dist2,_radiusMin,_radiusMax,_bTouching))
			continue;

		if (dlgt!=NULL)
		{
			if (!dlgt(obj,dist2))
				continue;
		}
		_ClearEnumFilter();
		return obj;

		ENUMLOOP_END();
	}
	else
	{
		i_math::rectf rcRange=_rcRange;
		rcRange+=i_math::pos2df(_xStart,_yStart);
		ENUMLOOP_BEGIN();

		if (!CheckInRange(obj,rcRange,_bTouching))
			continue;

		if (dlgt!=NULL)
		{
			float dist2=obj->GetFramePos().getDistanceSQFrom(*(i_math::vector2df*)&rcRange.getCenter());

			if (!dlgt(obj,dist2))
				continue;
		}
		_ClearEnumFilter();
		return obj;

		ENUMLOOP_END();
	}

	_ClearEnumFilter();
	return NULL;
}


LevelDetectRate CalcDetectRate(CLevelObj *obj,CalcDetectRateContext *ctx)
{
	LevelDetectRate rate;
	LevelDetectWeightsBase *weights=ctx->weights;
	rate.rate=-1.0f;
	rate.dist=0.0f;

	if (weights)
	{
		float dist=obj->GetFramePos().getDistanceFrom(ctx->center);
		rate.dist=dist;

		float rMin,rMax;
		rMin=0.0f;
		rMax=weights->distRef;
		if (ctx->bTouching)
		{
			rMin-=obj->GetRadius_();
			if (rMin<0.0f)
				rMin=0.0f;
			rMax+=obj->GetRadius_();
		}
		if (dist<rMin)
			dist=rMin;
		if (dist>rMax)
			dist=rMax;

		rate.rate=0.0f;

		if (weights->flags&LevelDetectWeights_Dist)
			rate.rate+=weights->wtDist*(rMax-dist)/(rMax-rMin);

		if (weights->flags&LevelDetectWeights_Player)
		{
			if (obj->IsPlayer())
				rate.rate+=weights->wtPlayer;
		}

		if (weights->flags&LevelDetectWeights_Agent)
		{
			if(obj->GetType()==LevelObjType_Agent)
				rate.rate+=weights->wtAgent;
		}

		if(weights->flags&LevelDetectWeights_Aggressive)
		{
			if (obj->GetType()==LevelObjType_Unit)
			{
				LevelRecordUnit *rec=((CLoUnit*)obj)->GetRec();
				if (rec)
				{
					if (rec->bAggressive)
						rate.rate+=weights->wtAggressive;
				}
			}
		}

		if(weights->flags&LevelDetectWeights_RecentTarget)
		{
			if (obj==ctx->recent)
				rate.rate+=weights->wtRecentTarget;
		}

		if(weights->flags&LevelDetectWeights_FailedTarget)
		{
			if (obj==ctx->fail)
				rate.rate-=weights->wtFailedTarget;
		}

		if(weights->flags&LevelDetectWeights_VeryClose)
		{
			if (dist<weights->distVeryClose)
				rate.rate+=weights->wtVeryClose;
		}

		if (ctx->src)
		{
			if (weights->flags&LevelDetectWeights_AttackingMe)
			{
				CLevelEventSrc *dmgsrc=ctx->src->GetEventSrc();
				if (dmgsrc)
				{
					if (dmgsrc->Exist(LET_Damage,obj->GetID(),ctx->src->GetT()-ANIMTICK_FROM_SECOND(20.0f)))
						rate.rate+=weights->wtAttackingMe;
				}
			}
			if (weights->flags&LevelDetectWeights_AttackedByMe)
			{
				CLevelEventSrc *dmgsrc=obj->GetEventSrc();
				if (dmgsrc)
				{
					if (dmgsrc->Exist(LET_Damage,ctx->src->GetID(),ctx->src->GetT()-ANIMTICK_FROM_SECOND(20.0f)))
						rate.rate+=weights->wtAttackedByMe;
				}
			}
		}

		LevelAIContext *ctx=obj->ObtainAIContext();
		if (ctx)
		{
			if(weights->flags&LevelDetectWeights_ThreatingCount)
				rate.rate*=pow(weights->scalePerThreating,(float)ctx->GetThreatingCount());
			if(weights->flags&LevelDetectWeights_AlertedCount)
				rate.rate*=pow(weights->scalePerAlerted,(float)ctx->GetAlertedCount());
			if(weights->flags&LevelDetectWeights_CombatedCount)
				rate.rate*=pow(weights->scalePerCombated,(float)ctx->GetCombatedCount());
		}
	}
	return rate;
}

CLevelObj *CLevelObjMap::EnumBest(LevelObjMapEnumCallBack dlgt)
{
	_enum.clear();
	if (_rcRange.Left()<0.0f)
	{
		_rcRange.Left()=0.0f;
		if (_rcRange.Right()<0.0f)
			_rcRange.Right()=0.0f;
	}
	if (_rcRange.Top()<0.0f)
	{
		_rcRange.Top()=0.0f;
		if (_rcRange.Bottom()<0.0f)
			_rcRange.Bottom()=0.0f;
	}

	i_math::recti rcTile;
	rcTile.Left()=(int)(_rcRange.Left()/LEVELOBJMAP_TILE_LEN);
	rcTile.Top()=(int)(_rcRange.Top()/LEVELOBJMAP_TILE_LEN);
	rcTile.Right()=(int)(_rcRange.Right()/LEVELOBJMAP_TILE_LEN)+1;
	rcTile.Bottom()=(int)(_rcRange.Bottom()/LEVELOBJMAP_TILE_LEN)+1;
	if (rcTile.Right()>_wTile)
		rcTile.Right()=_wTile;
	if (rcTile.Bottom()>_hTile)
		rcTile.Bottom()=_hTile;

	float dist2Min=100000000000.0f;
	CLevelObj *found=NULL;

	CalcDetectRateContext ctx;
	if (TRUE)
	{
		if ((_radiusMax>=0.0f)&&(_radiusMin>=0.0f))
		{
			ctx.radiusMax=_radiusMax;
			ctx.radiusMin=_radiusMin;
		}
		else
		{
			ctx.radiusMin=0.0f;
			ctx.radiusMax=(_rcRange.getWidth()+_rcRange.getHeight())/2.0f;
		}

		ctx.center=*(i_math::vector2df*)&_rcRange.getCenter();
		ctx.center.x+=_xStart;
		ctx.center.y+=_yStart;

		ctx.bTouching=_bTouching;

		ctx.weights=_weightsDetect;
		ctx.recent=_recent;
		ctx.fail=_fail;
		ctx.src=_src;
	}
	i_math::vector2df center=*(i_math::vector2df*)&_rcRange.getCenter();
	center.x+=_xStart;
	center.y+=_yStart;
	i_math::rectf rcRange;
	rcRange+=i_math::pos2df(_xStart,_yStart);

	float rateMax=-1.0f;

//	ENUMLOOP_BEGIN();

	for (DWORD i=rcTile.Left();i<rcTile.Right();i++)																									
		for (DWORD j=rcTile.Top();j<rcTile.Bottom();j++)																								
		{																																												
		DWORD xBlk,yBlk;																																				
		xBlk=i/LEVELOBJMAP_TILE_PER_BLOCK;																											
		yBlk=j/LEVELOBJMAP_TILE_PER_BLOCK;																											
		for (int k=0;k<_nBlkIdx;k++)																																
		{																																											
		BlkIdxInfo &info=_blkidx[k];																															
		DWORD idx=info.idx;																																		
		LevelMoveMethodMask methods=info.methods;																						
		DWORD maskPlayer=info.maskPlayer;																										
		DWORD flagsPlayerOrUnit=info.flagsPlayerOrUnit;																					
		if (_blocks[idx].empty())																																	
			continue;																																						
			CLevelObjBlock *blk=_blocks[idx][yBlk*_w+xBlk];																						
			if (blk)																																								
			{																																										
			DWORD xLocal,yLocal;																																
			xLocal=i%LEVELOBJMAP_TILE_PER_BLOCK;																							
			yLocal=j%LEVELOBJMAP_TILE_PER_BLOCK;																							
			LevelObjTile *tile=&blk->_tiles[yLocal*LEVELOBJMAP_TILE_PER_BLOCK+xLocal];								
			std::deque<CLevelObj*>::iterator it;																											
			for (it=tile->objs.begin();it!=tile->objs.end();it++)																					
			{																																									
			CLevelObj *obj=(*it);																																
			if (_IsIgnore(obj))																																	
				continue;																																				
				if (!_CheckAgent(obj))																															
					continue;																																				
					if (methods!=LevelMoveMethodMask_All)																							
					{																																								
					if (!(methods&obj->GetMoveMethodMask()))																				
						continue;																																			
					}																																								
					if (!(obj->GetPlayerIDMask()&maskPlayer))																						
						continue;																																				
						if (!(flagsPlayerOrUnit&( obj->IsPlayer()?1:2)))																					
							continue;																																				
							if (!_CheckRequire(obj))																															
								continue;


	if (_radiusMax>=0.0f)
	{
		float dist2=obj->GetFramePos().getDistanceSQFrom(center);
		if (!CheckInRange(obj,dist2,_radiusMin,_radiusMax,_bTouching))
			continue;
	}
	else
	{
		if (!CheckInRange(obj,rcRange,_bTouching))
			continue;
	}

	LevelDetectRate rate=CalcDetectRate(obj,&ctx);
	if (rate.rate<0.0f)
		continue;

	if (dlgt!=NULL)
	{
		if (!dlgt(obj,rate.dist*rate.dist))
			continue;
	}

	if (rate.rate>rateMax)
	{
		found=obj;
		rateMax=rate.rate;
	}

	ENUMLOOP_END();

	_ClearEnumFilter();
	return found;
}


float CLevelObjMap::EnumCalcStrength(LevelObjMapEnumCallBack dlgt)
{
	return 0.0f;
}



void CLevelObjMap::SetEnumRange(i_math::vector2df &center,float radiusMin,float radiusMax)
{
	_rcRange.set(center.x-_xStart,center.y-_yStart,center.x-_xStart,center.y-_yStart);
	_rcRange.inflate(radiusMax,radiusMax,radiusMax,radiusMax);
	_radiusMax=radiusMax;
	_radiusMin=radiusMin;
}


void CLevelObjMap::SetEnumRange(CLevelObj *lo,float radiusMin,float radiusMax)
{
	SetEnumRange(lo->GetFramePos(),radiusMin,radiusMax);
}


void CLevelObjMap::GarbageCollect()
{

}
