#pragma once

#include "../class/class.h"

#include <hash_map>

#include "../bitset/bitset.h"

typedef void *LevelCircumNodeHandle;
#define LevelCircumNodeHandle_Null (0)

#define LevelCircumNodeGap (1.0f)//每个Node之间的间隔


struct LevelCircumNode
{
	DEFINE_CLASS(LevelCircumNode);

	enum State
	{
		Empty=0,
		Requesting=1,
		Commit=2,
		Discard=3,
	};

	State state;
	i_math::vector2df posLocal;
	float radius;
};

class CLevelCircum2
{
public:
	CLevelCircum2()
	{
		_player=NULL;
		_ver=0;
		memset(_nodes,0,sizeof(_nodes));
	}

	void Init(CLevelPlayer *player,i_math::vector2df &center);
	void Clear();

	LevelCircumNodeHandle Request(i_math::vector2df &pos,LevelObjID id);
	void Discard(LevelCircumNodeHandle h);
	BOOL GetPos(LevelCircumNodeHandle h,i_math::vector2df &pos);
	BOOL IsRequesting(LevelCircumNodeHandle h);

	void UpdateCenter(i_math::vector2df &center);

	void FlushNetMsg();

protected:

	CLevelPlayer *_player;


	std::vector<LevelCircumNodeHandle> _requests;
	std::vector<LevelCircumNodeHandle> _discards;

	DWORD _ver;
	LevelCircumNode _nodes[256];

	i_math::vector2df _center;
};

