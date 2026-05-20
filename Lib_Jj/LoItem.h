#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "LevelPlayerStates.h"

#include "LevelObj.h"

struct LevelItemState;
struct LevelRecordItem;
struct LevelOp_ItemBirth;
struct LevelOSB;

class CLoItem:public CLevelObj
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoItem,7);

	CLoItem()
	{
		Zero();
	}
	void Zero()
	{
		_rec=NULL;
		_tCreate=0;
		_opBirth=NULL;
	}
	virtual LevelObjType GetType()	{		return LevelObjType_Item;	}
	virtual LevelObjID GetRootOwnerID()	{		return GetID();	}

	void PostCreate(LevelItemState *state,LevelPos&pos,LevelOSB &osb,LevelOpLink &link);
	virtual void OnDestroy();

	virtual LevelPos GetFramePos()
	{
		return _pos;
	}

	LevelRecordItem *GetRec()	{		return _rec;	}

	LevelItemState &GetState()	{		return _state;	}

	virtual void WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	virtual void Update();



protected:

	AnimTick _tCreate;


	LevelPos _pos;

	LevelRecordItem * _rec;

	LevelItemState _state;

	LevelOp_ItemBirth *_opBirth;


};
