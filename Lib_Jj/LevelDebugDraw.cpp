
#include "stdh.h"
#include "LevelDebugDraw.h"
#include "Level.h"


extern i_math::vector3df GameUtil_CalcGroundPos(LevelPos &pos);


void CLevelDebugDraw::Init(CLevel *level)
{
	_level=level;
}

void CLevelDebugDraw::Clear()
{
	_entries.clear();
	Zero();
}

void CLevelDebugDraw::Update()
{
	DWORD c=0;
	for (int i=0;i<_entries.size();i++)
	{
		Entry &e=_entries[i];
		if(e.tStart+e.dur<=_level->GetT_())
		{
			e.Zero();
			continue;
		}
		_entries[c]=e;
		c++;
	}
	_entries.resize(c);

}


void CLevelDebugDraw::DrawCircle(LevelPos &pos,float radius,DWORD col,float dur)
{
	Entry e;
	e.tp=Entry::Circle;

	e.pos1=pos;
	e.radius=radius;
	e.col=col;
	e.dur=ANIMTICK_FROM_SECOND(dur);
	e.tStart=_level->GetT_();

	_entries.push_back(e);

	BCDebugDraw msg;
	msg.tp=BCDebugDraw::Circle;
	msg.GetCircle().set(pos,radius);
	msg.col=col;
	msg.dur=dur;
	msg.uid=0;
	_level->SendNetMsg(&msg);

}


void CLevelDebugDraw::DrawLine(i_math::vector2df &from,i_math::vector2df &to,DWORD col,float dur)
{
	Entry e;
	e.tp=Entry::Line;

	e.pos1=from;
	e.pos2=to;
	e.col=col;
	e.dur=ANIMTICK_FROM_SECOND(dur);
	e.tStart=_level->GetT_();

	_entries.push_back(e);

	BCDebugDraw msg;
	msg.tp=BCDebugDraw::Line;
	msg.GetLine().setLine(from,to);
	msg.col=col;
	msg.dur=dur;
	msg.uid=0;
	_level->SendNetMsg(&msg);

}

void CLevelDebugDraw::DrawFan(DWORD uid,AnimEventZone::KeyFan &fan,DWORD col,float dur)
{
	BCDebugDraw msg;
	msg.tp=BCDebugDraw::Fan;
	msg.GetFan()=fan;
	msg.col=col;
	msg.dur=dur;
	msg.uid=uid;
	_level->SendNetMsg(&msg);
}

void CLevelDebugDraw::DrawSphere(DWORD uid,LevelPos3D pos,float radius,DWORD col,float dur)
{
	BCDebugDraw msg;
	msg.tp=BCDebugDraw::Sphere;
	msg.GetSphere().radius=radius;
	msg.GetSphere().center=pos;
	msg.col=col;
	msg.dur=dur;
	msg.uid=uid;
	_level->SendNetMsg(&msg);
}
