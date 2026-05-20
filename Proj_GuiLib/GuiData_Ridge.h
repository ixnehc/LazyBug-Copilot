#pragma once

#include "GuiLib.h"

#include "WorldSystem/IRidge.h"

#include "editor/editor.h"

#include "class/class.h"

#include <list>

//节点信息
struct RidgeNCache
{
	DEFINE_CLASS(RidgeNCache);
	RidgeNCache(){ bUpdate = false; }
	std::vector<i_math::vector3df> lines;		 //线段
	std::vector<i_math::vector3df> tris;		 //面带
	std::vector<i_math::vector3df> trisRidge;    //面带

	std::vector<i_math::vector3df> keyPos;  //关键点位置
	std::vector<i_math::vector3df> ridge;	//山脊关键点位置
	HMapObj hObj;							
	float d2Cam;							//到眼睛的距离
	bool bUpdate;
	bool bInView;							//用于更新
	
	struct _Data
	{
		_Data(){pack = NULL;}
		ICtrlPointPack * pack;  //关键点数据
		BOOL equals(ICtrlPointPack * pack_)
		{
			return  pack->Equals(pack_);
		}
	};

	~RidgeNCache()
	{
		assert(!data.pack);
	}

	void Clean()
	{
		if(data.pack)
		{
			data.pack->DeleteMe();
			data.pack = NULL;
		}
	}
	

	_Data data;						//Baffle 数据
	DWORD updateRef;
};

struct GuiLib_Api GuiData_Ridge:public GeData
{
	enum State
	{
		Creating,
		Idle,
	};

	virtual const char *GetName()	{		return "ridge";	}

	GuiData_Ridge()
	{
		Zero();
	}
	~GuiData_Ridge()
	{
	}
	void Zero()
	{
		pES = NULL;
		hObjSel = INVALID_HMAPOBJ;
	}
	
	void Clear()
	{
		ClearBuffer();
		stateCreate =  Idle;
		Zero();
	}
	
	void ClearBuffer()
	{
		std::list<RidgeNCache*>::iterator it;
		for(it=nodeCache.begin();it!=nodeCache.end();it++)
		{
			RidgeNCache * n = (*it);
			n->Clean();
			Class_Delete(n);
		}
		nodeCache.clear();
	}

	IRidgeEditor * GetEditor();
	IEntitySystem *pES;
	
	State stateCreate;

	//选中状态
	HMapObj hObjSel;
	std::vector<DWORD> selKeys; //选中对象上的节点信息
	DWORD ver;

	std::list<RidgeNCache*> nodeCache;			//
};





