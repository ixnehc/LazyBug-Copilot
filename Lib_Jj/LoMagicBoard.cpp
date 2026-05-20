
#include "stdh.h"

#include "commondefines/general_stl.h"
 
#include "Level.h"
#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LoMagicBoard.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"
#include "LoEffectObj.h"

#include "Random/Random.h"

#include "LevelRecordMagicTile.h"

#include "Buff_Birth.h"


#include "Protocal.h"
#include "Log/LogDump.h"

/////////////////////////////////////////////////////////////////////////////
//MagicTileDistrib_Single
void MagicTileDistrib_Single::Distrib(BOOL bEnemy,CLoMagicBoard *lo)
{
	std::vector<i_math::pos2di>&freetiles=lo->_GetFreeTiles(bEnemy,rgns);

	int n=CSysRandom::RandRangeInt(cMin,cMax+1);
	if (n>freetiles.size())
		n=freetiles.size();
	extern int GenPrimeStep();
	int step=GenPrimeStep();
	int start=CSysRandom::RandRangeInt<int>(0,freetiles.size());
	int w=lo->_w;
	for (int i=0;i<n;i++)
	{
		i_math::pos2di pos=freetiles[start];
		MagicTileInfo *info=&lo->_tiles[pos.y*w+pos.x];
		start=(start+step)%freetiles.size();

		info->candi=&candi;
		info->state=MagicTileState_Sealed;
	}
}

/////////////////////////////////////////////////////////////////////////////
//MagicTileDistrib_Multiple
void MagicTileDistrib_Multiple::Distrib(BOOL bEnemy,CLoMagicBoard *lo)
{
	std::vector<i_math::pos2di>&freetiles=lo->_GetFreeTiles(bEnemy,rgns);

	int n=cMax;
	if (n==0)
		n=freetiles.size();
	if (n>freetiles.size())
		n=freetiles.size();

	if (candi.size()<=0)
		return;

	KeySet ks;
	KeySet_Define(&ks,KT_Float);
	ks.SetKeyCount(candi.size()+1);

	Key_f k;
	float wt=0.0f;
	for (int i=0;i<=candi.size();i++)
	{
		Key_f k;
		k.t=ANIMTICK_FROM_SECOND(wt);
		ks.SetKey(i,k);
		if (i<candi.size())
			wt+=candi[i].wt;
	}

	extern int GenPrimeStep();
	int step=GenPrimeStep();
	int start=CSysRandom::RandRangeInt<int>(0,freetiles.size());
	int w=lo->_w;
	for (int i=0;i<n;i++)
	{
		float v=CSysRandom::RandRange(0.0f,wt);
		DWORD iKey=0,iKey2;
		float r;
		ks.FindKeys(ANIMTICK_FROM_SECOND(v),iKey,iKey2,r);

		if (iKey>=candi.size())
			iKey=candi.size()-1;

		i_math::pos2di pos=freetiles[start];
		MagicTileInfo *info=&lo->_tiles[pos.y*w+pos.x];
		start=(start+step)%freetiles.size();
		info->candi=&candi[iKey];
		info->state=MagicTileState_Sealed;
	}
}


/////////////////////////////////////////////////////////////////////////////
//CLoMagicBoard

void CLoMagicBoard::_BuildRgns()
{
	LosMagicBoard *los=(LosMagicBoard *)_src;

	CLevelRecords *records=_level->GetRecords();
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);

	MagicBoardLayout *layout=&board->layout;

	int w=layout->wTile;
	int half=(layout->nGrd1+layout->nGrd2+layout->nGrd3+layout->nGrd4);
	int h=half*2;

	int iLine=0;
	for (int j=0;j<layout->nGrd1;j++)
	{
		for (int i=0;i<w;i++)
		{
			_rgns[MagicTileRegion_Grade1].tiles.push_back(i_math::pos2di(i,iLine));
			_rgnsEnemy[MagicTileRegion_Grade1].tiles.push_back(i_math::pos2di(i,h-1-iLine));
		}
		iLine++;
	}

	for (int j=0;j<layout->nGrd2;j++)
	{
		for (int i=0;i<w;i++)
		{
			_rgns[MagicTileRegion_Grade2].tiles.push_back(i_math::pos2di(i,iLine));
			_rgnsEnemy[MagicTileRegion_Grade2].tiles.push_back(i_math::pos2di(i,h-1-iLine));
		}
		iLine++;
	}

	for (int j=0;j<layout->nGrd3;j++)
	{
		for (int i=0;i<w;i++)
		{
			_rgns[MagicTileRegion_Grade3].tiles.push_back(i_math::pos2di(i,iLine));
			_rgnsEnemy[MagicTileRegion_Grade3].tiles.push_back(i_math::pos2di(i,h-1-iLine));
		}
		iLine++;
	}

	for (int j=0;j<layout->nGrd4;j++)
	{
		for (int i=0;i<w;i++)
		{
			_rgns[MagicTileRegion_Grade4].tiles.push_back(i_math::pos2di(i,iLine));
			_rgnsEnemy[MagicTileRegion_Grade4].tiles.push_back(i_math::pos2di(i,h-1-iLine));
		}
		iLine++;
	}

	//更新每个tile里面的RegionType
	for (int i=0;i<MagicTileRegion_Max;i++)
	{
		MagicTileRegion *rgn=&_rgns[i];
		for (int j=0;j<rgn->tiles.size();j++)
		{
			i_math::pos2di pos=rgn->tiles[j];
			_tiles[pos.y*_w+pos.x].tpRgn=(MagicTileRegionType)i;
		}
		rgn=&_rgnsEnemy[i];
		for (int j=0;j<rgn->tiles.size();j++)
		{
			i_math::pos2di pos=rgn->tiles[j];
			_tiles[pos.y*_w+pos.x].tpRgn=(MagicTileRegionType)i;
		}
	}



}

void CLoMagicBoard::_Distrib()
{
	LosMagicBoard *los=(LosMagicBoard *)_src;

	CLevelRecords *records=_level->GetRecords();
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);


	for (int i=0;i<board->distribs.size();i++)
	{
		MagicTileDistrib *distrib=board->distribs[i].distrib;
		if (distrib)
		{
			distrib->Distrib(FALSE,this);
			distrib->Distrib(TRUE,this);
		}
	}
}

std::vector<i_math::pos2di>&CLoMagicBoard::_GetFreeTiles(BOOL bEnemy,std::vector<MagicTileRegionType> &tpsRgn)
{
	_temp.clear();
	MagicTileRegion *rgns=bEnemy?&_rgnsEnemy[0]:&_rgns[0];

	for (int i=0;i<tpsRgn.size();i++)
	{
		MagicTileRegionType tp=tpsRgn[i];
		if (tp>=MagicTileRegion_Max)
			continue;

		MagicTileRegion *rgn=&rgns[tp];

		for (int j=0;j<rgn->tiles.size();j++)
		{
			i_math::pos2di posTile=rgn->tiles[j];

			MagicTileInfo *tile=&_tiles[posTile.y*_w+posTile.x];
			if (tile->state!=MagicTileState_None)
				continue;

			_temp.push_back(posTile);
		}
	}

	return _temp;
}

void CLoMagicBoard::_CreateMainTower(float x,float y,LevelPlayerID idPlayer)
{
	if (!_level)
		return;
	CLevelRecords *records=_level->GetRecords();
	if (!records)
		return;

	LosMagicBoard *los=(LosMagicBoard *)_src;
	if (!los)
		return;
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);
	if (!board)
		return;
	i_math::matrix43f mat=los->GetMat();
	if (_level->GetRecords()->GetAgent(board->idTower))
	{
		if (TRUE)
		{
			CLoGeneralAgent* lo=(CLoGeneralAgent*)_level->CreateObj(Class_Ptr2(CLoGeneralAgent));

			LevelPos pos;
			pos.x=x;
			pos.y=y;
			pos.x+=mat.getTranslationP()->x;
			pos.y+=mat.getTranslationP()->z;
			float rad=0.0f;

			lo->PostCreate(pos,rad,board->idTower,idPlayer);

			_level->AddToActives(lo);

			SAFE_RELEASE(lo);
		}
	}

}


BOOL CLoMagicBoard::OnActivate()
{
	_level->RegisterUniqueObj(LevelUniqueObj_MagicBoard,this);

	_ai.Init(this);

	CSysRandom rand;

	LosMagicBoard *los=(LosMagicBoard *)_src;
	LopMagicBoard *lop=(LopMagicBoard *)_param;

	CLevelRecords *records=_level->GetRecords();
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);

	MagicBoardLayout *layout=&board->layout;


	_w=layout->wTile;
	_h=(layout->nGrd1+layout->nGrd2+layout->nGrd3+layout->nGrd4)*2;
	_lenTile=layout->lenTile;

	_tiles.resize(_w*_h);
	for (int i=0;i<_tiles.size();i++)
	{
		_tiles[i].idx=(WORD)i;
		_tiles[i].y=i/_w;
		_tiles[i].x=i%_w;
	}
	if (TRUE)
	{
		i_math::matrix43f mat=los->GetMat();

		MagicTileInfo *ti=_tiles.data();
		for (int j=0;j<_h;j++)
		for (int i=0;i<_w;i++)
		{
			i_math::matrix43f matTile;
			if (TRUE)
			{
				float xTile,yTile;
				int xStart=-_w/2;
				int yStart=-_h/2;

				xTile=(xStart+i)*_lenTile+_lenTile/2.0f;
				yTile=(yStart+j)*_lenTile+_lenTile/2.0f;

				matTile.setTranslation(xTile,0.0f,yTile);
				matTile=matTile*(mat);
			}
			ti->mat=matTile;
			ti++;
		}
	}

	_BuildRgns();
	_Distrib();

	//创建主塔
	if (TRUE)
	{
		_CreateMainTower(0.0f,-_lenTile*(float)(_h/2)-1.5f,0);
		_CreateMainTower(0.0f,_lenTile*(float)(_h/2)+1.5f,LevelPlayerID_Wild);
	}


	//设初始的Reach
	if (TRUE)
	{
		int x=_w/2;
		int y=0;

		MagicTileInfo *ti=&_tiles[y*_w+x];
		ti->reachesPlayer|=1<<((LevelPlayerID)0);

		y=_h-1;
		ti=&_tiles[y*_w+x];
		ti->reachesPlayer|=1<<((LevelPlayerID)LevelPlayerID_Wild);
		GetMagicBoardAIContext().seals.push_back(y*_w+x);
	}


 	return TRUE;
}

void CLoMagicBoard::OnDestroy()
{
	__super::OnDestroy();
	_ai.Clear();
}


void WriteMagicTileInfo(CBitPacket *bp,MagicTileInfo *ti,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(ti->tpRgn);
	MagicTileState state;
	state=ti->state;
	switch(state)
	{
		case MagicTileState_UnSealed:
		{
			if (idPlayer!=ti->idOwnerPlayer)
				state=MagicTileState_OtherUnSealed;
			break;
		}
		case MagicTileState_Commit:
		{
			if (idPlayer!=ti->idOwnerPlayer)
				state=MagicTileState_OtherCommit;
			break;
		}
	}

	bp->Data_WriteSimple(state);
	bp->Data_WriteSimple(ti->reachesPlayer);

	if (state==MagicTileState_UnSealed)
	{
		assert(ti->candi);
		bp->Data_WriteSimple(ti->candi->idTile);
	}
}


void CLoMagicBoard::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LosMagicBoard *los=(LosMagicBoard *)_src;
	CLevelRecords *records=_level->GetRecords();
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);

	MagicBoardLayout *layout=&board->layout;

	bContent=TRUE;
	bp->Data_WriteSimple(_w);
	bp->Data_WriteSimple(_h);
	bp->Data_WriteSimple(layout->lenTile);
	for (int i=0;i<_tiles.size();i++)
		WriteMagicTileInfo(bp,&_tiles[i],idPlayer);
}

void CLoMagicBoard::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_dirties.size()<=0)
		return;
	bContent=TRUE;
	DP_WriteVector(*bp,_dirties);

	for (int i=0;i<_dirties.size();i++)
		WriteMagicTileInfo(bp,&_tiles[_dirties[i]],idPlayer);
}

void CLoMagicBoard::_OnPostWriteSync()
{
	_dirties.clear();
}

BOOL CLoMagicBoard::_CheckMBResCost(LevelPlayerID idPlayer,MBResCost &cost)
{
	LevelAttr_MagicBoard *attr=MBUtil_GetAttr(_level,idPlayer);
	if (!attr)
		return FALSE;

	for (int i=1;i<MBRes_ActualMax;i++)
	{
		if (attr->res[i].GetCur_Int()<cost.costs[i])
			return FALSE;
	}

	return TRUE;
}

void CLoMagicBoard::_ApplyMBResCost(LevelPlayerID idPlayer,MBResCost &cost)
{
	for (int i=1;i<MBRes_ActualMax;i++)
	{
		if (cost.costs[i]>0)
		{
			MBUtil_MakeResMod(_level,idPlayer,(MBResourceType)i,-(int)cost.costs[i],TRUE,LevelOSB(),LevelOpLink());
		}
	}
}


BOOL CLoMagicBoard::_CheckReach(MagicTileInfo *ti,LevelPlayerID idPlayer)
{
	if (ti->reachesPlayer&(1<<idPlayer))
		return TRUE;
	return FALSE;
}


BOOL CLoMagicBoard::_UnsealTile(MagicTileInfo *tile,LevelPlayerID idPlayer)
{
	LosMagicBoard *los=(LosMagicBoard *)_src;
	LopMagicBoard *lop=(LopMagicBoard *)_param;

	CLevelRecords *records=_level->GetRecords();
	LevelRecordMagicBoard *board=records->GetMagicBoard(los->idBoard);

	if (tile->state==MagicTileState_Sealed)
	{
		if (tile->tpRgn<MagicTileRegion_Max)
		{
			if (_CheckReach(tile,idPlayer))
			{
				if (_CheckMBResCost(idPlayer,board->costRgns[tile->tpRgn]))
				{
					_ApplyMBResCost(idPlayer,board->costRgns[tile->tpRgn]);
					tile->state=MagicTileState_UnSealed;
					tile->idOwnerPlayer=idPlayer;

					extern BOOL MBUtil_IsAIPlayer(LevelPlayerID idPlayer);
					if (MBUtil_IsAIPlayer(idPlayer))
					{
						VEC_REMOVE_SWAP(GetMagicBoardAIContext().seals,tile->idx);
						UNIQUE_VEC_ADD(GetMagicBoardAIContext().unseals,tile->idx);
					}
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CLoMagicBoard::_CommitTile(MagicTileInfo *tile)
{
	if (tile->state==MagicTileState_UnSealed)
	{
		if (tile->candi)
		{
			RecordID idTile=tile->candi->idTile;
			LevelRecordMagicTile *recTile=_level->GetRecords()->GetMagicTile(idTile);
			if (recTile)
			{
				if (_CheckMBResCost(tile->idOwnerPlayer,recTile->costCommit))
				{
					_ApplyMBResCost(tile->idOwnerPlayer,recTile->costCommit);
					if (_level->GetRecords()->GetUnit(recTile->idUnit))
					{
						CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

						LevelPos pos;
						pos.set(tile->mat.getTranslationP()->x,tile->mat.getTranslationP()->z);

						lo->PostCreate(tile->idOwnerPlayer,NULL,recTile->idUnit,1,NULL,EquipSetPick_None,pos);

						_level->AddToActives(lo);

						if (recTile->idBirthBuff!=RecordID_Invalid)
						{
							BuffArg_Birth param;

							i_math::xformf xfm;
							xfm.fromMatrix(tile->mat);
							i_math::vector3df euler;
							xfm.rot.toEuler(euler);
							param.eulerX=euler.x;
							_level->GetDecider()->MakeBuff(lo,recTile->idBirthBuff,
								ANIMTICK_FROM_SECOND(1.0f),&param,FALSE);//1.0为随便填写的值,这个Buff持续的时间应根据record里决定
						}

						SAFE_RELEASE(lo);
					}
					else
					{
						if (_level->GetRecords()->GetAgent(recTile->idAgent))
						{
							CLoGeneralAgent* lo=(CLoGeneralAgent*)_level->CreateObj(Class_Ptr2(CLoGeneralAgent));

							LevelPos pos;
							pos.set(tile->mat.getTranslationP()->x,tile->mat.getTranslationP()->z);
							float rad=0.0f;

							lo->PostCreate(pos,rad,recTile->idAgent,tile->idOwnerPlayer);

							_level->AddToActives(lo);

							SAFE_RELEASE(lo);
						}
						else
						{
							if (_level->GetRecords()->GetEo(recTile->idEO))
							{
								LevelRecordEo *rec=_level->GetRecords()->GetEo(recTile->idEO);
								CLoEffectObj *eo=NULL;
								if (rec)
								{
									eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
									if (eo)
									{
										LevelPos pos;
										pos.set(tile->mat.getTranslationP()->x,tile->mat.getTranslationP()->z);
										eo->PostCreate(tile->idOwnerPlayer,recTile->idEO,pos,LevelPos(),1,LevelOSB(),LevelOpLink());
										_level->AddToActives(eo);
									}
								}

								if (!eo)
								{
									LOG_DUMP_1P("MagicTile",Log_Error,"无法创建特效对象(Eo):%s!",rec->Name.c_str());
								}
								SAFE_RELEASE(eo);

							}

						}
					}
					tile->state=MagicTileState_Commit;

					extern BOOL MBUtil_IsAIPlayer(LevelPlayerID idPlayer);
					if (MBUtil_IsAIPlayer(tile->idOwnerPlayer))
					{
						VEC_REMOVE_SWAP(GetMagicBoardAIContext().unseals,tile->idx);
						UNIQUE_VEC_ADD(GetMagicBoardAIContext().commits,tile->idx);
					}

					return TRUE;
				}
			}
		}
	}

	return FALSE;

}

void CLoMagicBoard::_AddDirties(int x,int y)
{
	WORD idx=(WORD)(y*_w+x);
	UNIQUE_VEC_ADD(_dirties,idx);
}


void CLoMagicBoard::_UpdateTileReach(int x,int y,LevelPlayerID idPlayer)
{
	LevelPlayerMask mask=1<<idPlayer;

	i_math::pos2di offs[]={i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,-1),i_math::pos2di(0,1)};

	for (int i=0;i<ARRAY_SIZE(offs);i++)
	{
		int xx,yy;
		xx=x+offs[i].x;
		yy=y+offs[i].y;

		if ((xx<0)||(xx>=_w)||(yy<0)||(yy>=_h))
			continue;

		MagicTileInfo *tile=&_tiles[yy*_w+xx];

		LevelPlayerMask maskOld=tile->reachesPlayer;
		tile->reachesPlayer|=mask;
		if (maskOld!=tile->reachesPlayer)
			_AddDirties(xx,yy);

		if (tile->state==MagicTileState_Sealed)
		{
			extern BOOL MBUtil_IsAIPlayer(LevelPlayerID idPlayer);
			if (MBUtil_IsAIPlayer(idPlayer))
			{
				UNIQUE_VEC_ADD(GetMagicBoardAIContext().seals,tile->idx);
			}
		}
	}
}

void CLoMagicBoard::Invoke(LevelPlayerID idPlayer,MagicBoardInvoke &invoke)
{
	_level->AddAffect(this);
	if ((invoke.x>=0)&&(invoke.y>=0)&&(invoke.x<_w)&&(invoke.y<_h))
	{
		MagicTileInfo *tile=&_tiles[invoke.y*_w+invoke.x];

		switch(tile->state)
		{
			case MagicTileState_Sealed:
			{
				if (_UnsealTile(tile,idPlayer))
					_UpdateTileReach(invoke.x,invoke.y,idPlayer);
				break;
			}
			case MagicTileState_UnSealed:
			{
				if (_CommitTile(tile))
					_UpdateTileReach(invoke.x,invoke.y,idPlayer);
				break;
			}
		}

		_AddDirties(invoke.x,invoke.y);
	}
}


void CLoMagicBoard::Update()
{
	_UpdateManaRecover();

	_ai.Update();
}

void CLoMagicBoard::_UpdateManaRecover()
{
	DWORD nIDs;
	LevelPlayerID *ids=_level->GetPlayerIDs(nIDs);

	extern void MBUtil_MakeResMod(CLevel *level,LevelPlayerID idPlayer,MBResourceType tp,int mod,BOOL bInstant,LevelOSB &osb,LevelOpLink &link);

	for (int i=0;i<nIDs;i++)
	{
		extern void MBUtil_MakeResMod(CLevel *level,LevelPlayerID idPlayer,MBResourceType tp,int mod,BOOL bInstant,LevelOSB &osb,LevelOpLink &link);
		MBUtil_MakeResMod(_level,ids[i],MBRes_Mana,1,FALSE,LevelOSB(),LevelOpLink());
	}

	MBUtil_MakeResMod(_level,LevelPlayerID_Wild,MBRes_Mana,1,FALSE,LevelOSB(),LevelOpLink());
}


