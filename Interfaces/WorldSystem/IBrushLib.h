
#pragma once

#include "gds/GObj.h"

#include "class/class.h"

#include "enums/enums.h"

#include "progress/progress.h"

typedef __int64  BRUID;

#define INVALID_BRUID 0xffffffffffffffff;

class IBrush
{
public:
	virtual BRUID GetUID() const = 0;
	virtual GObjBase * GetGObj() = 0;
	virtual CClass * GetClass() = 0;
	virtual const char * GetName() const = 0;
	virtual void SetName(const char * name) = 0;
};

enum BRLIB_TYPE
{
	BrushLib_ShoreWave,
	BrushLib_TreeWind,
	BrushLib_WaterWave,
	BrushLib_Grass,
	BrushLib_Tree,

	BrushLib_Max,

	BrushLib_UnKnown = 0xffffffff
};

BEGIN_ENUMS(BRLIB_TYPE,BrushLib_)

	ENUM_ENTRY(BrushLib_ShoreWave)
	ENUM_ENTRY(BrushLib_TreeWind)
	ENUM_ENTRY(BrushLib_WaterWave)
	ENUM_ENTRY(BrushLib_Grass)
	ENUM_ENTRY(BrushLib_Tree)

END_ENUMS();

typedef int BRLIB_TYPE_VALUE;


struct AssetSystemState;
class IResource;
struct NodeTreeRef;
typedef void * NodeHandle;
class IBrushLib
{
public:
	virtual const char * GetPath() = 0;
	virtual IBrush* New() = 0;
	virtual BOOL Delete(const BRUID &uid) = 0;
	virtual BOOL Set(const BRUID &uid,const IBrush * br) = 0;
	virtual const IBrush * Get(const BRUID &uid) = 0;
	virtual void Enum(BRUID *& pUid,DWORD &count) = 0;
	virtual void Save() = 0;
	virtual BOOL ReLoad() = 0;						//重新读取
	virtual void * GetInfo(const BRUID &uid) = 0;	//用于暴露笔刷 部分信息，指针不可保留使用
	virtual BRLIB_TYPE GetType() = 0;
	virtual DWORD GetMemVersion() = 0;			// >1
	
	virtual IResource * ObtainRes(const IBrush * brush) = 0;//得到笔刷引用到的资源 
	virtual NodeTreeRef * GetNodeTree() = 0;
	virtual BRUID GetBrushID(NodeHandle nodeHandle) = 0;
	virtual BOOL IsChecked(const BRUID &uid) = 0;	
	virtual IBrush * GetDefaultBrush() = 0;

	virtual BOOL Bake(AssetSystemState *ss,const BRUID &uid,CProgress *pProgress = NULL) = 0;
	virtual BOOL NeedBake(const BRUID &uid) = 0;
};


