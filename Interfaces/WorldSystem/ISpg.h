
#pragma  once

#include "IObjMap.h"

#include "gds/GObj.h"

#include "ref/sharedPtr.h"

#include "WorldSystem/IBrushLib.h"

struct SpgWindCfg
{
	SpgWindCfg(){strength = 0.1f; waveLen = 2.0f; speed = 20.0f; direction.set(1.0f,0.0f,0.0f);}
	float strength;
	float speed;
	float waveLen;
	i_math::vector3df direction;
};

typedef RefConPtr<SpgWindCfg> SpgWindCfgPtr;

struct SpgInfo
{
	float scale; 
	float  rot;
	i_math::vector3df pos;
	float fLod;
};

struct SpgBrushInfo
{
	float radius;
	float weight;
	BOOL bBB;
};

class ISpgEditor :public IObjMapEditor
{
public:
	virtual HMapObj	  Add(const BRUID &uid,const SpgInfo &info) = 0;
	virtual HMapObj * Remove(i_math::pos2df &center,float r,DWORD& count) = 0;
	virtual IBrushLib * GetSpgLib() = 0;

	virtual void SetWindCfg(const SpgWindCfg *cfg) = 0;	
	virtual SpgWindCfgPtr GetWindCfg() = 0;	

	virtual BOOL CheckVisible(const HMapObj &hObj) = 0;	//清除 无效不可见对象
};










