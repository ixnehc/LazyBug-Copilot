
#include "stdh.h"
#include "Level.h"

#include "LevelRtnuCircum.h"

#include "LevelObjMove.h"

#include "datapacket/BitPacket.h"

#include "LevelRecordUnit.h"

#include "Log/LogDump.h"

#include "rvo2/RvoUnit.h"


extern void UnitCollide_SetAlly(UnitCollide &collide,DWORD ally);
extern void UnitCollide_SetPlayer(UnitCollide &collide,BOOL bPlayer);
extern void UnitCollide_SetLayor(UnitCollide &collide,DWORD layor);



//////////////////////////////////////////////////////////////////////////
//CLevelObjMove
LevelTick CLevelObjMove::_GetT()	
{		
	return _owner?_owner->GetT():0;	
}

void CLevelObjMove::Create(CLevelObj *owner,LevelObjMoveSetting &setting)
{
	_owner=owner;
	_setting=setting;
}

CUnitGesture *CLevelObjMove::_FetchGesture()
{
	if (_unitGround)
		return _unitGround->FetchGesture();
	if (_unitFlying)
		return _unitFlying->FetchGesture();
	return NULL;
}

void CLevelObjMove::_SwitchGroundUnit(BOOL bFloating,LevelPos &pos,LevelFace face,LevelTeleportID idTeleport)
{
	if ((_method!=LevelMoveMethod_Ground)||(_method!=LevelMoveMethod_Floating)||IsRtnuMode())
	{
		CUnitGesture *ges=_FetchGesture();
		_ClearUnit();

		//Unit
		CUnitMgrNavMesh *unitmgr=_owner->GetLevel()->GetUnitMgr();
		_unitGround=unitmgr->CreateUnit(pos,face,_setting.radius,_setting.speed,_setting.pace);
		if (_unitGround)
			_unitGround->SetPathFindType(_setting.bAdvWalkable?UnitFindPath_AdvWalkable:UnitFindPath_Walkable);

		//UnitCollide
		if (TRUE)
		{
			UnitCollide collide=UnitCollide_Empty;

			extern DWORD LevelUtil_PlayerIDToUnitCollideAlly(LevelPlayerID idPlayer);
			UnitCollide_SetAlly(collide,LevelUtil_PlayerIDToUnitCollideAlly(_owner->GetPlayerID()));
			UnitCollide_SetPlayer(collide,_owner->IsPlayer());
			UnitCollide_SetLayor(collide,_setting.layorCollide);

			_unitGround->SetCollide(collide);
		}
		_unitGround->SetData((void*)_owner);
		if (ges)
			_unitGround->SetGesture(ges);
		SAFE_RELEASE(ges);
	}
	else
	{
		if (_unitGround)
			_unitGround->Reset(pos,face);
	}

	_pos=pos;
	_face=face;
	_ht=0.0f;
	_tPos=_GetT();
	_vel.set(0,0);

	_bReaching=1;
	if (idTeleport!=LevelTeleportID_Invalid)
		_idTeleport=idTeleport;
}


void CLevelObjMove::SwitchGround(LevelPos &pos,LevelFace face,LevelTeleportID idTeleport)
{
	if (IsRtnuMode())
		return;

	_SwitchGroundUnit(FALSE,pos,face,idTeleport);
	_method=LevelMoveMethod_Ground;

	_UpdateRtnuPlayerID();
}

void CLevelObjMove::SwitchFloating(LevelPos &pos,LevelFace face,float htFloating,LevelTeleportID idTeleport)
{
	if (IsRtnuMode())
		return;

	_SwitchGroundUnit(FALSE,pos,face,idTeleport);
	if (_unitGround)
		_unitGround->ResetFloatingHeight(htFloating);
	_ht=htFloating;
	_method=LevelMoveMethod_Floating;

	_UpdateRtnuPlayerID();
}


void CLevelObjMove::SwitchResided(LevelPos3D &pos3D,LevelTeleportID idTeleport)
{
	if (IsRtnuMode())
		return;

	if (_method!=LevelMoveMethod_Resided_)
	{
		_ClearUnit();
		_unitResided_=Class_New2(ResidedUnit);
	}
	_unitResided_->pos=pos3D;

	_pos.set(pos3D.x,pos3D.z);
	_ht=pos3D.y;
	_tPos=_GetT();
	_vel.set(0,0);

	_bReaching=1;
	if (idTeleport!=LevelTeleportID_Invalid)
		_idTeleport=idTeleport;

	_method=LevelMoveMethod_Resided_;
}

void CLevelObjMove::SwitchRtnuMode(BOOL bRtnuMode,LevelRtnuRank rank)
{
	if (_method!=LevelMoveMethod_Ground)
		return;

	LevelPos pos=_GetUnitGroundPos();
	LevelFace face=_GetUnitGroundFace();
	if (IsRtnuMode()&&(!bRtnuMode))
		_SwitchGroundUnit(FALSE,pos,face,LevelTeleportID_Invalid);
	else
	{
		if (!IsRtnuMode()&&bRtnuMode)
		{
			if (_owner)
			{
				extern CLevelPlayer *LevelUtil_GetOwnerPlayer(CLevelObj *lo);
				CLevelPlayer *player=LevelUtil_GetOwnerPlayer(_owner);
				if (player)
				{
					_ClearUnit();
					if (player->GetRtnuCircum())
					{
						_unitGroundRtnu=player->GetRtnuCircum()->Register(rank,_owner,pos,face);
						_unitGroundRtnu->AddRef();
					}
				}
			}
		}
	}
	_UpdateRtnuPlayerID();
}

void CLevelObjMove::SwitchFlying(LevelPos3D &pos3D,LevelFace face,LevelTeleportID idTeleport)
{
	if (IsRtnuMode())
		return;

	assert(_setting.fly);
	if (!_setting.fly)
	{
		assert(false);
		return;//不支持
	}

	if (_method!=LevelMoveMethod_Flying)
	{
		CUnitGesture *ges=_FetchGesture();
		_ClearUnit();

		//Unit
		CUnit3DMgr*unitmgr=_owner->GetLevel()->GetUnit3DMgr();
		_unitFlying=unitmgr->CreateUnit3D(pos3D,face,_setting.fly);

		_unitFlying->SetData((void*)_owner);

		_unitFlying->SetGesture(ges);
		SAFE_RELEASE(ges);
	}
	else
		_unitFlying->Reset(pos3D,face);
	_pos.set(pos3D.x,pos3D.z);
	_face=face;
	_ht=pos3D.y;
	_tPos=_GetT();
	_vel.set(0,0);

	_bReaching=1;
	if (idTeleport!=LevelTeleportID_Invalid)
		_idTeleport=idTeleport;

	_method=LevelMoveMethod_Flying;

}

void CLevelObjMove::SwitchMount(LevelObjID idMountTarget,float ht,LevelTeleportID idTeleport)
{
	if (IsRtnuMode())
		return;

	if (_method!=LevelMoveMethod_Mount)
	{
		_ClearUnit();
		_unitMount=Class_New2(MountUnit);
	}
	_unitMount->idMountTarget=idMountTarget;

	_bMountPosDirty=1;
	_ht=ht;

	_bReaching=1;
	if (idTeleport!=LevelTeleportID_Invalid)
		_idTeleport=idTeleport;

	_method=LevelMoveMethod_Mount;
}


void CLevelObjMove::_ClearUnit()
{
	Safe_Class_Delete(_unitResided_);
	Safe_Class_Delete(_unitMount);
	SAFE_DESTROY(_unitFlying);
	SAFE_DESTROY(_unitGround);
	if (_unitGroundRtnu)
	{
		extern CLevelPlayer *LevelUtil_GetOwnerPlayer(CLevelObj *lo);
		CLevelPlayer *player=LevelUtil_GetOwnerPlayer(_owner);
		if (player)
		{
			if (player->GetRtnuCircum())
				player->GetRtnuCircum()->Unregister(_unitGroundRtnu);
		}
		SAFE_RELEASE(_unitGroundRtnu);
	}
}

void CLevelObjMove::_ClearSpeedMod()
{
	if (_modSpeed)
	{
		_modSpeed->Clear();
		Safe_Class_Delete(_modSpeed);
	}
}

void CLevelObjMove::Destroy()
{
	_ClearUnit();
	_ClearSpeedMod();
	Zero();
}


BOOL CLevelObjMove::GetFramePos3D(LevelPos3D &pos3D)
{
	if (_method==LevelMoveMethod_Floating)
	{
		i_math::vector2df &pos=_unitGround->GetPos();
		float htFloating=_unitGround->GetFloatingHeight();

		extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
		pos3D=LevelUtil_GetGroundHeight(_owner->GetLevel(),pos.x,pos.y,FALSE);

		return TRUE;
	}

	if (_method==LevelMoveMethod_Resided_)
	{
		pos3D.set(_pos.x,_ht,_pos.y);
		return TRUE;
	}
	if (_method==LevelMoveMethod_Flying)
	{
		pos3D=_unitFlying->GetPos();
		return TRUE;
	}
	if (_method==LevelMoveMethod_Mount)
	{
		LevelPos pos=GetFramePos();
		pos3D.set(pos.x,_ht,pos.y);
		return TRUE;
	}

	return FALSE;
}

LevelMoveStage LevelMoveStageFromUnitStage(UnitStage stage)
{
	static LevelMoveStage remap[40];//Big enough
	static BOOL bInit=FALSE;
	if (!bInit)
	{
		bInit=TRUE;
		memset((BYTE*)&remap[0],0,sizeof(remap));

		remap[UnitStage_NotMove]=LevelMoveStage_NotMove;
		remap[UnitStage_RotateOnSpot]=LevelMoveStage_RotateOnSpot;
		remap[UnitStage_Faced]=LevelMoveStage_Faced;
		remap[UnitStage_Reached]=LevelMoveStage_Reached;
		remap[UnitStage_Abort]=LevelMoveStage_Abort;
		remap[UnitStage_StartFw]=LevelMoveStage_StartFw;
		remap[UnitStage_StartRot]=LevelMoveStage_StartRot;
		remap[UnitStage_Move]=LevelMoveStage_Move;
		remap[UnitStage_Stop]=LevelMoveStage_Stop;
		//XXXXX:More Move Stage
	}
	return remap[stage];
}


LevelMoveStage CLevelObjMove::GetStage()
{
	if (_owner->IsPlayer())
	{
		if (_bReaching)
			return LevelMoveStage_NotMove;
		return LevelMoveStage_Move;
	}
	else
	{
		switch(_method)
		{
			case LevelMoveMethod_Floating:
			case LevelMoveMethod_Ground:
			{
				if (_unitGround)
					return LevelMoveStageFromUnitStage(_unitGround->GetStage());
				if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
					return LevelMoveStageFromUnitStage(_unitGroundRtnu->GetStage());
				break;
			}
			case LevelMoveMethod_Resided_:
				return LevelMoveStage_NotMove;
			case LevelMoveMethod_Flying:
				if ((_unitFlying->GetState()==CUnit3D::Idle)||(_unitFlying->GetState()==CUnit3D::PostFollow))
					return LevelMoveStage_NotMove;
				else
					return LevelMoveStage_Move;
			case LevelMoveMethod_Mount:
				return LevelMoveStage_NotMove;
		}
	}

	return LevelMoveStage_None;
}

LevelPos &CLevelObjMove::_GetUnitGroundPos()
{
	if (_unitGround)
		return _unitGround->GetPos();
	assert(_unitGroundRtnu);
	if (_unitGroundRtnu->IsAlive())
		return _unitGroundRtnu->GetPos();
	return _pos;
}

LevelFace CLevelObjMove::_GetUnitGroundFace()
{
	if (_unitGround)
		return _unitGround->GetFace();
	assert(_unitGroundRtnu);
	if (_unitGroundRtnu->IsAlive())
		return _unitGroundRtnu->GetFace();
	return _face;
}



void CLevelObjMove::_WritePos(CBitPacket *bp)
{
	bp->Bits_Write(_method,3);

	switch(_method)
	{
		case LevelMoveMethod_Floating:
		{
			//这一帧的位置
			bp->Data_WriteSimpleR(_pos);
			bp->Data_WriteSimple(_face);
			bp->Data_WriteSimple(_ht);

			//下一帧的位置
			bp->Data_WriteSimpleR(_unitGround->GetPos());
			bp->Data_WriteSimple(_unitGround->GetFace());
			bp->Data_WriteSimple(_unitGround->GetFloatingHeight());
			break;
		}

		case LevelMoveMethod_Ground:
		{
			//这一帧的位置
			LevelPosInt pos=LevelPosToInt(_pos);
			bp->Data_WriteSimpleR(pos);
			bp->Data_WriteSimple(_face);
			//下一帧的位置
			bp->Data_WriteSimpleR(LevelPosToInt(_GetUnitGroundPos()));
			bp->Data_WriteSimple(_GetUnitGroundFace());
			break;
		}
		case LevelMoveMethod_Resided_:
		{
			//这一帧的位置及下一帧的位置
			bp->Data_WriteSimpleR(_unitResided_->pos);
			break;
		}
		case LevelMoveMethod_Flying:
		{
			//这一帧的位置
			bp->Data_WriteSimpleR(_pos);
			bp->Data_WriteSimple(_ht);
			bp->Data_WriteSimple(_face);

			//下一帧的位置
			bp->Data_WriteSimpleR(_unitFlying->GetPos());
			bp->Data_WriteSimple(_unitFlying->GetFace());

			break;
		}
		case LevelMoveMethod_Mount:
		{
			bp->Data_WriteSimpleR(_unitMount->idMountTarget);
			break;
		}
	}

}

void _GetGestureInfo(CLevelObjMove *move,UnitGestureUID &uidGesture,float &tGestureAge)
{
	uidGesture=UnitGestureUID_Invalid;
	tGestureAge=0.0f;
	CUnit3D *unitFlying=move->GetFlyingUnit();
	if (unitFlying)
	{
		uidGesture=unitFlying->GetGestureUID();
		tGestureAge=unitFlying->GetGestureAge();
	}
}


void CLevelObjMove::WriteFirstSync(CBitPacket *bp)
{
	//位置信息
	_WritePos(bp);

	//和移动位置相关的一些信息 
	bp->Bits_Write(GetStage(),LevelMoveStage_BitCount);
	bp->Data_WriteSimple(_idTeleport);
	bp->Data_WriteSimple(_idSkill);
	bp->Data_WriteSimple(_GetUnitSpeed());

	//Gesture
	if (TRUE)
	{
		UnitGestureUID uidGesture;
		float tGestureAge;
		_GetGestureInfo(this,uidGesture,tGestureAge);

		if (uidGesture!=UnitGestureUID_Invalid)
		{
			bp->Bit_Write_1();
			bp->Data_WriteSimple(uidGesture);
			bp->Data_WriteSimple(tGestureAge);
		}
		else
			bp->Bit_Write_0();
	}

}

//注意,如果填写了任何需要被传送的信息,一定要给bContent赋值,否则有可能会导致这些信息无法发送出去
//切记切记
void CLevelObjMove::WriteSyncH(CBitPacket *bp,BOOL &bContent)
{
	if (_method==_methodLast)
	{
		bp->Bit_Write_0();//method没有更新

		switch(_method)
		{
			case LevelMoveMethod_Ground:
			{
				LevelPosInt pos=LevelPosToInt(_GetUnitGroundPos());
				if (pos==_posNextLast)
					bp->Bits_Write(0,2);//没有位置更新
				else
				{
					bContent=TRUE;

					LevelPosInt posOff=pos-_posNextLast;

					if ((posOff.x>=-16)&&(posOff.x<16)&&
						(posOff.y>=-16)&&(posOff.y<16))
					{
						DWORD offx,offy;
						offx=posOff.x+16;
						offy=posOff.y+16;

						bp->Bits_Write(1,2);//极小范围更新(+/- 50厘米范围内),我们更新offset(2x5bit)
						bp->Bits_Write(offx,5);
						bp->Bits_Write(offy,5);
					}
					else
					{
						if ((posOff.x>=-64)&&(posOff.x<64)&&
							(posOff.y>=-64)&&(posOff.y<64))
						{
							DWORD offx,offy;
							offx=posOff.x+64;
							offy=posOff.y+64;

							bp->Bits_Write(2,2);//小范围更新(+/- 200厘米范围内),我们更新offset(2x7bit)
							bp->Bits_Write(offx,7);
							bp->Bits_Write(offy,7);
						}
						else
						{
							bp->Bits_Write(3,2);//大范围更新(+/- 200厘米范围内),我们更新绝对值
							bp->Data_WriteSimpleR(posOff);
						}
					}
				}

				LevelFaceInt face=LevelFaceToInt(_GetUnitGroundFace());
				if (face==_faceNextLast)
					bp->Bit_Write(FALSE);//没有face更新
				else
				{
					bContent=TRUE;
					bp->Bit_Write(TRUE);//有face更新
					bp->Data_WriteSimple(face);
				}

				break;	
			}
			case LevelMoveMethod_Resided_:
			{
				if (!(_posNext3DLast==_unitResided_->pos))
				{
					bContent=TRUE;
					bp->Bit_Write_1();//有位置更新
					bp->Data_WriteSimpleR(_unitResided_->pos);
				}
				else
					bp->Bit_Write_0();//没有位置更新
				break;
			}
			case LevelMoveMethod_Flying:
			{
				LevelPos3D &pos3D=_unitFlying->GetPos();
				if (!(_posNext3DLast==pos3D))
				{
					bContent=TRUE;
					bp->Bit_Write_1();//有更新
					bp->Data_WriteSimpleR(pos3D);
				}
				else
					bp->Bit_Write_0();//没有位置更新

				LevelFaceInt face=LevelFaceToInt(_unitFlying->GetFace());
				if (face==_faceNextLast)
					bp->Bit_Write(FALSE);//没有face更新
				else
				{
					bContent=TRUE;
					bp->Bit_Write(TRUE);//有face更新
					bp->Data_WriteSimple(face);
				}

				break;
			}
			case LevelMoveMethod_Floating:
			{
				LevelPos &pos=_unitGround->GetPos();
				float ht=_unitGround->GetFloatingHeight();
				if (!((_posNext3DLast.x==pos.x)&&
						(_posNext3DLast.z==pos.y)&&
						(_posNext3DLast.y==ht)))
				{
					bContent=TRUE;
					bp->Bit_Write_1();//有更新
					bp->Data_WriteSimpleR(pos);
					bp->Data_WriteSimpleR(ht);
				}
				else
					bp->Bit_Write_0();//没有位置更新

				LevelFaceInt face=LevelFaceToInt(_unitGround->GetFace());
				if (face==_faceNextLast)
					bp->Bit_Write(FALSE);//没有face更新
				else
				{
					bContent=TRUE;
					bp->Bit_Write(TRUE);//有face更新
					bp->Data_WriteSimple(face);
				}

				break;
			}

			case LevelMoveMethod_Mount:
			{
				if (!(_idMountTarget==_unitMount->idMountTarget))
				{
					bContent=TRUE;
					bp->Bit_Write_1();//有内容更新
					bp->Data_WriteSimpleR(_unitMount->idMountTarget);
				}
				else
					bp->Bit_Write_0();//没有位置更新
				break;
			}
		}
	}
	else
	{//Method有更新
		bContent=TRUE;
		bp->Bit_Write_1();//method有更新
		_WritePos(bp);//重新传一下位置
	}

	//移动速度
	if (TRUE)
	{
		float speed=_GetUnitSpeed();
		if (speed==_speedLast)
			bp->Bit_Write_0();//没有变化
		else
		{
			bContent=TRUE;
			bp->Bit_Write_1();//有变化
			bp->Data_WriteSimple(speed);
		}
	}

	if (TRUE)
	{
		LevelMoveStage stage=GetStage();
		if (_stageLast==stage)
			bp->Bit_Write_0();//没有变化
		else
		{
			bContent=TRUE;
			bp->Bit_Write_1();//有变化
			bp->Bits_Write(stage,LevelMoveStage_BitCount);
			if ((stage==LevelMoveStage_StartRot)||(stage==LevelMoveStage_RotateOnSpot))
			{
				BOOL bLeftRot=TRUE;
				float gap=0;

				if (_unitGround)
				{
					UnitRot *rot=_unitGround->GetRot();
					if (rot)
					{
						bLeftRot=i_math::judge_rotate_dir(rot->from,rot->to);
						gap=i_math::get_radian_dist(rot->from,rot->to);
					}
				}

				bp->Bit_Write(bLeftRot);
				LevelFaceInt gap2=LevelFaceToInt(gap);
				bp->Data_WriteSimple(gap2);
			}
		}
	}

	if (TRUE)
	{
		if (_idTeleportLast==_idTeleport)
			bp->Bit_Write_0();//没有变化
		else
		{
			bContent=TRUE;
			bp->Bit_Write_1();//有变化
			bp->Data_WriteSimple(_idTeleport);
		}
	}

	if (TRUE)
	{
		_idSkill=LevelSkillID_Invalid;
		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
		CLevelSkill *skill=LevelUtil_GetCastingSkill(_owner);
		if (skill)
		{
			if (skill->GetCastMoving()==CLevelSkill::CastMoving_Control)
				_idSkill=skill->GetID();
		}

		if (_idSkillLast==_idSkill)
			bp->Bit_Write_0();//没有变化
		else
		{
			bContent=TRUE;
			bp->Bit_Write_1();//有变化
			bp->Data_WriteSimple(_idSkill);
		}
	}

	if (TRUE)
	{
		UnitGestureUID uidGesture;
		float tGestureAge;
		_GetGestureInfo(this,uidGesture,tGestureAge);

		BOOL bNewGesture=FALSE;
		if (uidGesture!=_uidGestureLast)
			bNewGesture=TRUE;
		else
		{
			if (tGestureAge<_tGestureAgeLast)
				bNewGesture=TRUE;//同一个类型的Gesture,但age小了,说明是新的Gesture
		}

		if (bNewGesture)
		{
			bContent=TRUE;
			bp->Bit_Write_1();
			bp->Data_WriteSimple(uidGesture);
			bp->Data_WriteSimple(tGestureAge);
		}
		else
			bp->Bit_Write_0();
	}
}

void CLevelObjMove::PostWriteSync()
{
	_methodLast=_method;
	switch(_method)
	{

		case LevelMoveMethod_Floating:
		{
			LevelPos &pos=_unitGround->GetPos();
			_posNext3DLast.set(pos.x,_unitGround->GetFloatingHeight(),pos.y);
			_faceNextLast=LevelFaceToInt(_unitGround->GetFace());
			break;
		}
		case LevelMoveMethod_Ground:
			_posNextLast=LevelPosToInt(_GetUnitGroundPos());
			_faceNextLast=LevelFaceToInt(_GetUnitGroundFace());
			break;
		case LevelMoveMethod_Resided_:
			_posNext3DLast=_unitResided_->pos;
			break;
		case LevelMoveMethod_Flying:
		{
			_posNext3DLast=_unitFlying->GetPos();
			_faceNextLast=LevelFaceToInt(_unitFlying->GetFace());
			break;
		}
		case LevelMoveMethod_Mount:
			_idMountTarget=_unitMount->idMountTarget;
			break;
	}

	_stageLast=GetStage();
	_idTeleportLast=_idTeleport;
	_idSkillLast=_idSkill;
	_GetGestureInfo(this,_uidGestureLast,_tGestureAgeLast);
	_speedLast=_GetUnitSpeed();

}


void CLevelObjMove::SetMove(LevelPos &pos,LevelFace face,BOOL bReaching)
{
	if (_method==LevelMoveMethod_Ground)
	{
		if (_unitGround||_unitGroundRtnu)
		{
			float dt=ANIMTICK_TO_SECOND(_GetT()-_tPos);
			_vel=(pos-_pos)/ANIMTICK_TO_SECOND(LEVEL_FRAME_TICK);
			_pos=pos;
			_face=face;
			_tPos=_GetT();

			if (_unitGround)
				_unitGround->Reset(pos,face);
			if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
				_unitGroundRtnu->Reset(pos,face);
			_bReaching=bReaching?1:0;
		}
	}
}

void CLevelObjMove::Teleport(LevelTeleportID idTeleport,LevelPos &pos,LevelFace face)
{
	if (_method==LevelMoveMethod_Ground)
	{
		_idTeleport=idTeleport;
		_pos=pos;
		_face=face;
		_tPos=_GetT();
		_vel.set(0,0);
		if (_unitGround)
			_unitGround->Reset(pos,face);
		if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
			_unitGroundRtnu->Reset(pos,face);
	}
}


float CLevelObjMove::_GetUnitSpeed()
{
	float speed=0.0f;
	if (_unitGround)
		speed=_unitGround->GetSpeed();
	else
	{
		if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
			speed=_unitGroundRtnu->GetSpeed();
		else
		{
			if (_unitFlying)
				speed=_unitFlying->GetSpeed();
		}
	}
	return speed;
}

float CLevelObjMove::CalcGroundSpeed()
{
	CLevelBuffs *buffs=_owner->GetBuffs();
	float ims=1.0f;
	if (buffs)
		ims=buffs->GetIMS();
	float speed=_setting.speed;
	if (_modSpeed)
	{
		float speedT;
		if (_modSpeed->speedFinal.GetValue(speedT))
			speed=speedT;
		else
		{
			if (_modSpeed->speed.GetValue(speedT))
				speed=speedT;
			speed*=ims;
		}
	}
	else
		speed*=ims;

	return speed;
}


void CLevelObjMove::_UpdateUnitSpeed()
{
	CLevelBuffs *buffs=_owner->GetBuffs();
	if (_unitGround)
		_unitGround->SetSpeed(CalcGroundSpeed());
	if (_unitFlying)
	{
		assert(_setting.fly);
		float speed=_setting.fly->speed;
		if (_modSpeed)
		{
			float speedT;
			if (_modSpeed->speedFlyingFinal.GetValue(speedT))
				speed=speedT;
			else
			{
				if (_modSpeed->speedFlying.GetValue(speedT))
					speed=speedT;
				float ims=1.0f;
				if (buffs)
					ims=buffs->GetIMS();
				speed*=ims;
			}
		}
		else
		{
			float ims=1.0f;
			if (buffs)
				ims=buffs->GetIMS();
			speed*=ims;
		}
		_unitFlying->SetSpeed(speed);
	}
}


void CLevelObjMove::Update()
{
	LevelTick t=_GetT();
	if (_tPos>=t)
		return;
	float dt=ANIMTICK_TO_SECOND(t-_tPos);
	_tPos=t;
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Flying))
	{
		//从Unit里得到当前位置
		if (_unitGround)
		{
			_vel=(_unitGround->GetPos()-_pos)/dt;
			_pos=_unitGround->GetPos();
			_face=_unitGround->GetFace();
		}
		if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
		{
			_vel=(_unitGroundRtnu->GetPos()-_pos)/dt;
			_pos=_unitGroundRtnu->GetPos();
			_face=_unitGroundRtnu->GetFace();
		}
		if (_unitFlying)
		{
			LevelPos3D &pos3D=_unitFlying->GetPos();
			LevelPos posCur(pos3D.x,pos3D.z);
			_vel=(posCur-_pos)/dt;
			_pos=posCur;
			_face=_unitFlying->GetFace();
			_ht=pos3D.y;
		}

		//更新Unit的移动速度
		_UpdateUnitSpeed();
	}

	if (_method==LevelMoveMethod_Floating)
	{
		//从Unit里得到当前位置
		if (_unitGround)
		{
			_pos=_unitGround->GetPos();
			_face=_unitGround->GetFace();
			_vel=(_unitGround->GetPos()-_pos)/dt;
			_ht=_unitGround->GetFloatingHeight();
		}

		//更新Unit的移动速度
		_UpdateUnitSpeed();
	}


	if (_method==LevelMoveMethod_Mount)
	{
		LevelPos posCur=_GetMountPos();
		_vel=(posCur-_pos)/dt;
		_pos=posCur;
	}

	//更新Ghost Collide
	if (_unitGround||_unitGroundRtnu)
	{
		BOOL bGhostCollide=FALSE;
		CLevelBuffs *buffs=_owner->GetBuffs();
		if(buffs)
		{
			if (buffs->TestFlag(BuffFlag_GhostCollide))
				bGhostCollide=TRUE;
		}

		SetCollide_Ghost(bGhostCollide);
	}

	if (_pacePending)
	{
		if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Flying))
		{
			if (_unitGround)
			{
				if (_unitGround->SetPace(_pacePending->bEnable?_pacePending:NULL))
					_pacePending=NULL;
			}
		}
	}

}

LevelMoveSession CLevelObjMove::RequestTarget(LevelPos &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange)
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->RequestTargetPos(pos,range,bClosestFollow,bNoStopMoveWhenInRange);
	}

	if (_method==LevelMoveMethod_Flying)
	{
		_unitFlying->SetTarget_Pos(pos,range);
		return LevelMoveSession_Invalid;
	}
	return LevelMoveSession_Invalid;
}

LevelMoveSession CLevelObjMove::RequestTarget(LevelPos3D &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange)
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
		{
			LevelPos pos2D;
			pos2D.set(pos.x,pos.z);
			return _unitGround->RequestTargetPos(pos2D,range,bClosestFollow,bNoStopMoveWhenInRange);
		}
	}
	if (_method==LevelMoveMethod_Flying)
	{
		_unitFlying->SetTarget_Pos3D(pos,range);
		return LevelMoveSession_Invalid;
	}
	return LevelMoveSession_Invalid;
}


LevelMoveSession CLevelObjMove::RequestTarget(CLevelObj *lo,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange,BOOL b3DFollow)
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
		{
			CLevelObjMove *move=lo->GetMove();
			if (move)
			{
				if (TRUE)
				{
					CUnit *unit=move->GetGroundUnit();
					if (unit)
						return _unitGround->RequestTarget(unit,range,bClosestFollow,bNoStopMoveWhenInRange);
				}
				if (TRUE)
				{
					CRtnuUnit *unitRtnu=move->GetGroundRtnuUnit();
					if (unitRtnu)
					{
						CUnit *unitMirror=unitRtnu->GetMirrorUnit();
						if (unitMirror)
							return _unitGround->RequestTarget(unitMirror,range,bClosestFollow,bNoStopMoveWhenInRange);
					}
				}
				if(TRUE)
				{
					CUnit3D *unit3D=move->GetFlyingUnit();
					if (unit3D)
						return _unitGround->RequestTarget3D(unit3D,range);
				}
			}
			if (TRUE)
			{
				CUnit *unit=lo->GetUnit();
				if (unit)
				{
					return _unitGround->RequestTarget(unit,range,bClosestFollow,bNoStopMoveWhenInRange);
				}
			}
			//直接设位置
			LevelPos pos=lo->GetFramePos();
			if (!(pos==LevelPos_Invalid))
				return RequestTarget(pos,range+lo->GetRadius_(),bClosestFollow,bNoStopMoveWhenInRange);
		}
	}

	if (_method==LevelMoveMethod_Flying)
	{
		CLevelObjMove *move=lo->GetMove();
		if (move)
		{
			CUnit *unit=move->GetGroundUnit();
			if (unit)
			{
				if (b3DFollow)
					_unitFlying->SetTarget_GroundUnit(unit,lo->GetAimHeight(),range);
				else
					_unitFlying->SetTarget_Unit(unit,range);

				return LevelMoveSession_Invalid;
			}
			CUnit3D *unit3D=move->GetFlyingUnit();
			if (unit3D)
			{
				_unitFlying->SetTarget_Unit3D(unit3D,range);
				return LevelMoveSession_Invalid;
			}
		}
		//直接设位置
		LevelPos pos=lo->GetFramePos();
		if (!(pos==LevelPos_Invalid))
			RequestTarget(pos,range,bClosestFollow,bNoStopMoveWhenInRange);
		return LevelMoveSession_Invalid;
	}
	return LevelMoveSession_Invalid;
}

LevelMoveSession CLevelObjMove::RequestFacing(float range,float rad)
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->RequestFacing(range,rad);
	}
	return LevelMoveSession_Invalid;
}



void CLevelObjMove::ResetIdle()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
		{
			//_pos=_unitGround->GetPos();//要不要更新_pos?
			_unitGround->Reset();
		}
	}
	if (_method==LevelMoveMethod_Flying)
		_unitFlying->ResetIdle();
}

LevelMoveSession CLevelObjMove::RequestNoTarget()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->RequestNoTarget();
	}
	if (_method==LevelMoveMethod_Flying)
	{
		_unitFlying->ResetIdle();
		return LevelMoveSession_Invalid;
	}
	return LevelMoveSession_Invalid;
}


BOOL CLevelObjMove::IsMoving_()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->IsMoving();
		if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
			return _unitGroundRtnu->IsMoving();
	}
	if (_method==LevelMoveMethod_Flying)
	{
		if ((_unitFlying->GetState()==CUnit3D::Idle)||(_unitFlying->GetState()==CUnit3D::PostFollow))
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

BOOL CLevelObjMove::IsMovingFailure()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->IsFailure();
	}

	return FALSE;
}

BOOL CLevelObjMove::IsMovingEnd()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->IsEnd();
	}

	return FALSE;
}




BOOL CLevelObjMove::IsMovingOrRotating()
{
	if ((_method==LevelMoveMethod_Ground)||(_method==LevelMoveMethod_Floating))
	{
		if (_unitGround)
			return _unitGround->IsMovingOrRotating();
	}
	if (_method==LevelMoveMethod_Flying)
	{
		if ((_unitFlying->GetState()==CUnit3D::Idle)||(_unitFlying->GetState()==CUnit3D::PostFollow))
			return FALSE;
		return TRUE;
	}
	return FALSE;
}


void CLevelObjMove::SetCollide_Ghost(BOOL bGhost)
{
	if (_method==LevelMoveMethod_Ground)
	{
		extern void UnitCollide_SetGhost(CUnit *unit,BOOL bGhost);
		if (_unitGround)
			UnitCollide_SetGhost(_unitGround,bGhost);
		if (_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
		{
			CUnit *unitMirror=_unitGroundRtnu->GetMirrorUnit();
			if (unitMirror)
				UnitCollide_SetGhost(unitMirror,bGhost);
		}
	}
}


LevelPos CLevelObjMove::_GetMountPos()
{
	CLevelObj *loMountTarget=_owner->GetLevel()->GetIDs()->LoFromID(_unitMount->idMountTarget);
	if (loMountTarget)
		return loMountTarget->GetFramePos();

	return LevelPos_Invalid;
}

void CLevelObjMove::_UpdateRtnuPlayerID()
{
	if (!_owner)
		return;
	if(_unitGroundRtnu&&_unitGroundRtnu->IsAlive())
	{
		RvoUnit *unitRvo=_unitGroundRtnu->GetRvoUnit();
		if (unitRvo)
			unitRvo->setPlayerID(_owner->GetPlayerID());
	}
	if (_unitGround)
	{
		RvoUnit *unitRvo=_unitGround->GetMirror();
		if (unitRvo)
			unitRvo->setPlayerID(_owner->GetPlayerID());
	}
}
