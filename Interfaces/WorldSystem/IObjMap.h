
#pragma  once


typedef __int64 HMapObj;
#define INVALID_HMAPOBJ	0xffffffffffffffff

class IObjMapEditor
{
public:
	//editor type
	virtual DWORD GetType() = 0;
	//get block position
	virtual BOOL GetMapFileBlk(const HMapObj & hObj, i_math::pos2di & ptBlk) = 0; 
	virtual i_math::pos2di GetMapFileBlk(const i_math::vector3df& pos) = 0;
	// check handle is valid
	virtual BOOL IsValid(const HMapObj & hObj) = 0;	
	//delete a object by handle
	virtual BOOL Delete(const HMapObj & hObj) = 0;
	//save 
	virtual void Save() = 0;
	// enum object in given range; rc: in world coord.单位为米
	virtual HMapObj * Enum(const i_math::recti &rc,DWORD &count) = 0;
	virtual HMapObj * Enum(const i_math::vector3df & center,float radius,DWORD &count) = 0;

	virtual HMapObj  HitTest(const i_math::line3df & rayHit,i_math::vector3df *pos = NULL) = 0;
	virtual float GetMapBlockLen() = 0;
	virtual void ReLoadMap() = 0;

	virtual BOOL GetGroundPos(const i_math::line3df &ray,i_math::vector3df &posHi) = 0;
	virtual BOOL GetGroundPos(float x,float z,i_math::vector3df &pos) = 0;
};

enum OBJMAP_TYPE
{
	OBJMAP_TYPE_PROBECUBE = 1, 
	OBJMAP_TYPE_TREE = 2,
	OBJMAP_TYPE_BAFFLE = 3,
	OBJMAP_TYPE_VEGETABLE = 4,
	OBJMAP_TYPE_ETPROBE = 5,
	OBJMAP_TYPE_SHORE = 6,
	OBJMAP_TYPE_WATER = 7,
	OBJMAP_TYPE_RIDGE = 8,
	OBJMAP_TYPE_ROAD  = 9,
	OBJMAP_TYPE_NAVMESH = 10,
	OBJMAP_TYPE_GAMERGN = 11,
};

//Obj Type

