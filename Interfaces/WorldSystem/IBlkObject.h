
#pragma  once


typedef __int64 HBlkObj;
#define INVALID_HBLKOBJ	0xffffffffffffffff

class IBlkMapEditor
{
public:
	//editor type
	virtual DWORD GetType() = 0;
	//get block position
	virtual BOOL GetMapFileBlk(const HBlkObj & hObj, i_math::pos2di & ptBlk) = 0; 
	// check handle is valid
	virtual BOOL IsValid(const HBlkObj & hObj) = 0;	
	//delete a object by handle
	virtual BOOL Delete(const HBlkObj & hObj) = 0;
	//save 
	virtual void Save() = 0;
	// enum object in given range; rc: in world coord.
	virtual HBlkObj * Enum(i_math::pos2di pt0,i_math::pos2di pt1,DWORD &count) = 0; 
};

enum BLKOBJ_TYPE
{
	BLKOBJ_TYPE_PROBECUBE = 1, 
};

//Obj Type

