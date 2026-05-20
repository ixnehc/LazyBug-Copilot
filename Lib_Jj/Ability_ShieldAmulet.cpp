
#include "stdh.h"

#include "Protocal.h"

#include "Ability_ShieldAmulet.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeShieldAmulet_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeShieldAmulet_Init);
BOOL CUpgradeShieldAmulet_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_ShieldAmulet *ability=(CLevelAbility_ShieldAmulet *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_ShieldAmulet

CLevelAbility_ShieldAmulet::~CLevelAbility_ShieldAmulet()
{
	_abortsDmg.clear();

	GDestructor();
}


void CLevelAbility_ShieldAmulet::_OnBuildRT()
{
	_BuildGradeRT();

	CUpgradeShieldAmulet_Init*upgradeInitial=(CUpgradeShieldAmulet_Init *)_upgradeInitial;
	_speedRot=upgradeInitial->speedRot;

}

void CLevelAbility_ShieldAmulet::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_ShieldAmulet::_SaveSync(CDataPacket &dp)
{
	dp.Data_WriteSimple(_tStart);
	dp.Data_WriteSimple(_speedRot);
}

void CLevelAbility_ShieldAmulet::_LoadSync(CDataPacket &dp,CRecords *records)
{
	dp.Data_ReadSimple(_tStart);
	dp.Data_ReadSimple(_speedRot);
}

BOOL CLevelAbility_ShieldAmulet::HitTest(i_math::line3df &line,float radius,i_math::vector3df &posHit)
{
	if (!_owner)
		return FALSE;
	if (!IsActive())
		return FALSE;
	LevelPos3D pos3D=_owner->GetFramePos3D();
	LevelTick t=_owner->GetT();

	i_math::vector2df start=line.start.getXZ();
	i_math::vector2df dir=line.end.getXZ()-start;
	float len=dir.getLength();
	if (len<0.001f)
		return FALSE;

	i_math::vector2df posMe=pos3D.getXZ();

	float dist=intersectSphereBySphere(start,radius,dir,posMe,SHIELDAMULET_SHIELDRANGE);
	if (dist<0.0f)
		return FALSE;

	float rate=dist/len;
	posHit=line.start+(line.end-line.start)*rate;

	if (posHit.y>pos3D.y+SHIELDAMULET_SHIELDHEIGHT)
		return FALSE;
	if (posHit.y<pos3D.y)
		return FALSE;

	t=ANIMTICK_SAFE_MINUS(t,_tStart);
	float theta=ANIMTICK_TO_SECOND(t)*_speedRot*i_math::GRAD_PI2+i_math::Pi/2.0f;

	i_math::vector2df dirShield,dirHit;
	dirShield.x=cosf(theta);
	dirShield.y=sinf(theta);

	dirHit.x=posHit.x-posMe.x;
	dirHit.y=posHit.z-posMe.y;
	dirHit.safe_normalize();

	static float d=cosf(SHIELDAMULET_SHIELDFOV*i_math::GRAD_PI2/2.0f);
	if (dirHit.dotProduct(dirShield)<d)
		return FALSE;//Out of fov

	return TRUE;

}


void CLevelAbility_ShieldAmulet::_OnStartDay()
{
	CUpgradeShieldAmulet_Init*upgradeInitial=(CUpgradeShieldAmulet_Init *)_upgradeInitial;

}


void CLevelAbility_ShieldAmulet::_OnUpdate(LevelTick dt)
{
	if (_tStart==ANIMTICK_INFINITE)
	{
		if (_owner)
			_tStart=_owner->GetT();
	}
}

void CLevelAbility_ShieldAmulet::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelAbility_ShieldAmulet::_OnEvent(LevelEvent &e0)
{
}


void CLevelAbility_ShieldAmulet::DepositDmgAbort(LevelDmgAbort &abort)
{
	_abortsDmg.push_back(abort);
}

BOOL CLevelAbility_ShieldAmulet::FetchDmgAbort(LevelDmgAbort &abort)
{
	if (_abortsDmg.size()>0)
	{
		abort=_abortsDmg[0];
		_abortsDmg.pop_front();
		return TRUE;
	}
	return FALSE;
}

