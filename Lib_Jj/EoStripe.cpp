
#include "stdh.h"

#include "Level.h"
#include "LevelUtil.h"

#include "EoStripe.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//CTimedCircles
BOOL CTimedCircles::Add(i_math::vector2df &pos,AnimTick t)
{
	AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_tStart);
	if (_circles.size()>0)
	{
		TimedCircle *last=&_circles[_circles.size()-1];

		if (tLocal<last->tStart)
			return FALSE;

		float dist2=pos.getDistanceSQFrom(last->circle.center);
		if (dist2>_gapMax*_gapMax)
		{
			//间隔太大,需要额外插入一些点
			float dist=sqrtf(dist2);
			int nStep=(int)(dist/_gapMax)-1;
			if (nStep<0)
				nStep=0;

			i_math::vector2df dir;
			dir=pos-last->circle.center;
			dir.normalize();
			dir*=_gapMax;

			AnimTick dt=(AnimTick)(_gapMax/dist*(float)(tLocal-last->tStart));

			for (int i=0;i<nStep;i++)
				_Add(last->circle.center+dir*(float)(i+1),last->tStart+dt*(i+1));
		}
	}

	_Add(pos,tLocal);
	return TRUE;
}

BOOL CTimedCircles::IsIn(i_math::vector2df &pos,AnimTick t)
{
	AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_tStart);

	for (int i=0;i<_circles.size();i++)
	{
		TimedCircle *circle=&_circles[i];

		if (circle->tCache!=tLocal)
		{
			circle->circle.radius=_radius*_vsRadiusScale->GetFloat(tLocal-circle->tStart);
			circle->tCache=tLocal;
		}

		if (circle->circle.isPointIn(pos))
			return TRUE;
	}

	return FALSE;
}


void CTimedCircles::Discard(AnimTick t)
{
	AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_tStart);
	AnimTick dur=_vsRadiusScale->GetDur();

	while(_circles.size()>0)
	{
		if (_circles[0].tStart+dur<tLocal)
			_circles.pop_front();
		else
			break;
	}
}

i_math::rectf CTimedCircles::GetBoundRect(AnimTick t)
{
	Discard(t);

	i_math::rectf rc;
	for (int i=0;i<_circles.size();i++)
		rc.merge(_circles[i].circle.center.x,_circles[i].circle.center.y);
	rc.inflate(_radiusMax,_radiusMax,_radiusMax,_radiusMax);
	return rc;
}





//////////////////////////////////////////////////////////////////////////
//EoStripe

BIND_EOPARAM(EoStripe,EoParamStripe);

void EoStripe::_OnPostCreate()
{
	EoParamStripe *param=GetParam<EoParamStripe>();
	if (!param)
		return;

	_pos=_GetInitialPos();
	_dir=_GetInitialDir();
	if (_dir.equalsZero())
	{
		float rad=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		_dir.set(cosf(rad),sinf(rad));
	}


	_circles.Init(param->radius,&param->scaleRadius,1.0f);
	_circles.Reset(GetT());
	_circles.Add(_pos,GetT());

	_tLast=GetT();

	_remain=0;

	//Build the KeySet
	if (TRUE)
	{
		KeySet_Define(&_ks,KT_Floatx2);
		_ks.SetKeyCount(2);

		Key_2f *k=(Key_2f *)_ks.GetKey(0);
		k->t=0;
		k->v=_pos;

		k=(Key_2f *)_ks.GetKey(1);

		AnimTick dur=param->dur;
		LevelPos posTarget=_pos+_dir*param->speed*ANIMTICK_TO_SECOND(dur);

		CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
		LevelPos posHit;

		if (unitmgr->StaticRayCast(UnitFindPath_Walkable,_pos,posTarget,posHit))
		{
			posTarget=posHit;
			dur=ANIMTICK_FROM_SECOND(_pos.getDistanceFrom(posHit)/param->speed);
		}

		k->t=dur;
		k->v=posTarget;
	}
}


void EoStripe::_OnUpdate()
{
	EoParamStripe *param=GetParam<EoParamStripe>();
	if (!param)
		return;

	AnimTick t=GetT();
	AnimTick dt=ANIMTICK_SAFE_MINUS(t,_tLast);
	_tLast=t;

	if (!_bEnd)
	{
		AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_tCreate);

		Key_2f k;
		_ks.CalcKey(tLocal,&k);

		_circles.Add(k.v,t);
		_pos=k.v;

		if (tLocal>=_ks.GetEndTick())
			_bEnd=TRUE;
	}

	_remain+=dt;

	_circles.Discard(t);
	if (_circles.IsEmpty())
		DeferDestroy();
	else
	{
		if (_remain>=param->cycle)
		{
			_remain-=param->cycle;

			i_math::rectf rc;
			rc=_circles.GetBoundRect(t);

			LevelUtilDetectParam paramDetect;
			paramDetect.loSrc=this;
			paramDetect.pos.set(rc.getCenter().x,rc.getCenter().y);
			paramDetect.rc=rc;
			paramDetect.flags=&param->flagsDetect[0];
			paramDetect.nFlags=param->flagsDetect.size();
			paramDetect.requires=&param->requires[0];
			paramDetect.nRequires=param->requires.size();

			DWORD c;
			CLevelObj **los=LevelUtil_Detect(paramDetect,NULL,c);

			for (int i=0;i<c;i++)
			{
				CLevelObj *loTarget=los[i];

				if (!_circles.IsIn(loTarget->GetFramePos(),t))
					continue;

				DealArg arg;
				arg.link.id=GetLevel()->GenOpLinkID();
				arg.grd=1;

				_MakeDeals(loTarget,arg);
			}
		}
	}
}

void EoStripe::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_ks.Save(*bp->GetDP());
	bp->Data_WriteSimpleR(_dir);
	bContent=TRUE;
}
