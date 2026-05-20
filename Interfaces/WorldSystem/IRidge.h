
#pragma  once 

#include "WorldSystem/IObjMap.h"

#include "WorldSystem/ICtrlPoint.h"


class ITrrnMapEditor;
class IRidgeEditor :public IObjMapEditor
{
public:
	virtual HMapObj AddRidge(ITrrnMapEditor * trrn,ICtrlPointPack * pack) = 0;

	virtual HMapObj SetCtrlPointPack(const HMapObj &hObj,ITrrnMapEditor * trrn,ICtrlPointPack * pack) = 0;
	virtual ICtrlPointPack * NewCtrlPointPack() = 0;

	virtual ICtrlPointPack * GetRidge(const HMapObj & hObj) = 0;
	virtual ICtrlPointPack * GetCollisionPoints(const HMapObj & hObj) = 0; //得到与地表相交的关键点

};






