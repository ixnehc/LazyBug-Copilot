#pragma once

#include "class/class.h"

#include "records/records.h"

#include "anim/KeySet.h"

struct LevelLoResoPath
{
	DEFINE_CLASS(LevelLoResoPath);

	KeySet ks;
};

struct LevelPathesEvent:public AnimEvent
{
	i_math::xformf xfm;
	AnimEventZone zone;
};

struct LevelPath
{
	DEFINE_CLASS(LevelPath);

	KeySet ksPos;
	KeySet ksPos3D;
	KeySet ksFace;
	AnimTick dur;
	float facePath;
	float length;//2D length

};

struct AnimPiece;
struct LevelPathes
{
	DEFINE_CLASS(LevelPathes);
	LevelPathes()
	{
		Zero();
	}
	void Zero()
	{
		def=NULL;
		defLoReso=NULL;
	}
	void Clear();
	LevelPathesEvent *FindEvent(StringID nmEvent);

	std::unordered_map<StringID,LevelPath *>pathes;
	std::unordered_map<StringID,LevelLoResoPath *>pathesLoReso;
	LevelPath *def;
	LevelLoResoPath *defLoReso;

	std::vector<LevelPathesEvent> events;

};

class CLevelRecords;
struct ResData;
class CLevelResources
{
public:
	IMPLEMENT_REFCOUNT_C

	DEFINE_CLASS(CLevelResources);
	CLevelResources()
	{
		Zero();
	}
	~CLevelResources()
	{
		Clear();
	}
	void Zero()
	{
	}

	void Clear();
	void Init(const char *pathRoot,CLevelRecords *records);

	LevelPathes *FindPathes(RecordID idRec);
	LevelPath *FindPath(RecordID idRec,StringID nmAP=StringID_Invalid);
	LevelLoResoPath *FindLoResoPath(RecordID idRec,StringID nmAP=StringID_Invalid);


protected:

	void _LoadRes(RecordID id,const char *path,float scaleTime,float facePath);

	void _CullAPKeySet(ResData *data,int idxAP,KeySet &ksAP);
	void _LocalizeKeySet(KeySet &ks);
	LevelLoResoPath * _MakeLoResoPath(KeySet &ksAP,float scaleTime);
	LevelPath * _MakePath(ResData *data,int idxAP,KeySet &ksAP,float scaleTime);

	std::string _pathRoot;
	std::unordered_map<RecordID,ResData *> _entries;
	std::unordered_map<RecordID,LevelPathes*> _pathes;

};

