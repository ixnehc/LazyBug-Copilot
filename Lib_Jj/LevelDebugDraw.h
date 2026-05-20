#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

class CLevel;
class CLevelDebugDraw
{
public:
	struct Entry
	{
		Entry()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}

		enum Type
		{
			None=0,
			Circle,
			Line
		};

		Type tp;

		LevelPos pos1;
		LevelPos pos2;
		float radius;
		DWORD col;

		AnimTick tStart;
		AnimTick dur;
	};

	CLevelDebugDraw()
	{
		Zero();
	}

	void Zero()
	{
		_level=NULL;
	}

	void Init(CLevel *level);
	void Clear();

	void Update();

	void DrawCircle(LevelPos &pos,float radius,DWORD col,float dur=0);
	void DrawLine(LevelPos &from,LevelPos &to,DWORD col,float dur=0);
	void DrawFan(DWORD uid,AnimEventZone::KeyFan &fan,DWORD col,float dur=0);
	void DrawSphere(DWORD uid,LevelPos3D pos,float radius,DWORD col,float dur=0);

public://take it as protected

	std::vector<Entry> _entries;

	CLevel *_level;

};