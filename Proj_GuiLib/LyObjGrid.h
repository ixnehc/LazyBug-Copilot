#pragma once

#include "GObjGrid.h"

#include "WndBase.h"

#include "class/class.h"

#include "WorldSystem/IBrushLib.h"

template<class T>
class  CLyObjGrid :public CGObjGrid
{
public:
	CLyObjGrid(void);

	virtual ~CLyObjGrid(void);

	void Create(CWnd *pParent,DWORD idCtrl);

	void BindData(const T *pObj);

	T * GetData(){return _inst;}
	
	virtual void OnBeginItemChange(CXTPPropertyGridItem *item){_bLocked = TRUE; CGObjGrid::OnBeginItemChange(item);}

	virtual void OnEndItemChange(CXTPPropertyGridItem *item){CGObjGrid::OnEndItemChange(item); _bLocked = FALSE;}

protected:
	BOOL _bLocked;
	const T  * _ptr;
	CClass * _cls;
	T * _inst;		//编辑对象
};


#include "LyObjGrid.cpp"

