#pragma once

#include "GuiLib.h"

#include "WorldSystem/IBaffle.h"

#include "editor/editor.h"

#include "class/class.h"

#include <list>

//节点信息
struct BaffleNCache{
	DEFINE_CLASS(BaffleNCache);
	BaffleNCache(){bUpdate = false;}
	std::vector<i_math::vector3df> lines;	//线段
	std::vector<i_math::vector3df> tris;    //面带
	std::vector<i_math::vector3df> keyPos;  //关键点位置
	HMapObj hObj;							
	float d2Cam;							//到眼睛的距离
	bool bUpdate;
	bool bInView;							//用于更新
	BaffleInfo info;						//Baffle 数据
	DWORD updateRef;
};

struct GuiLib_Api GuiData_Baffle:public GeData
{
	virtual const char *GetName()	{		return "baffle";	}

	GuiData_Baffle()
	{
		Zero();
	}
	~GuiData_Baffle()
	{
		Clear();
	}
	void Zero()
	{
		pES = NULL;
		bOnCreate = FALSE;
		bCreateSub = FALSE;
	}
	
	void Clear()
	{
		Zero();
		ClearBuffer();
	}
	
	void ClearBuffer()
	{
		std::list<BaffleNCache*>::iterator it;
		for(it=nodeCache.begin();it!=nodeCache.end();it++){
			BaffleNCache * n = (*it);
			Class_Delete(n);
		}
		nodeCache.clear();
	}

	IBaffleEditor * GetEditor();
	IEntitySystem *pES;
	
	//选中状态
	HMapObj hObjSel;
	std::vector<__int64> selKeys; //选中对象上的节点信息

	BOOL bOnCreate;
	BOOL bCreateSub;			//是否为增补创建
	DWORD ver;

	std::list<BaffleNCache*> nodeCache;			//

//	std::vector<HMapObj> hObjsMod;				// 修改过的对象列表
};





