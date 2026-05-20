#pragma once

#include "animbase.h"

#include <vector>

class CDataPacket;

struct AnimPiece
{
	AnimPiece();
	AnimPiece &operator=(const AnimPiece &src);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp,DWORD ver);

	void SaveZones(CDataPacket &dp);
	void LoadZones(CDataPacket &dp);

	AnimEventZone *FindZone(AnimEvent &e);

	StringID name;

	//tStart/tEnd describes a range(inclusive) in a KeySet
	AnimTick tStart;
	AnimTick tEnd;//if ANIMTICK_INFINITE,index to the end 

	std::vector<AnimEvent> events;//the tick in events are always 0-based
	std::vector<AnimEventZone> zones;
	
	DWORD iStart;	//start index to keyset
	DWORD iEnd;		//	end index to keyset	  [iStart,iEnd)

	float params[2];
};

