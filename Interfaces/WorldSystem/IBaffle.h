
#pragma  once 

#include "WorldSystem/IObjMap.h"

#include "WorldSystem/ICtrlPoint.h"

class IBafflesEditor :public IObjMapEditor
{
public:
	virtual HMapObj AddBaffles(ICtrlPointPack * pack) = 0;

	virtual ICtrlPointPack * GetCtrlPointPack(const HMapObj & hObj) = 0;
	virtual HMapObj SetCtrlPointPack(const HMapObj &hObj,ICtrlPointPack * pack) = 0;
	virtual ICtrlPointPack * NewCtrlPointPack() = 0;
	
	virtual i_math::vector3df * GetGroundPos(const HMapObj &hObj,DWORD &count) = 0;
	virtual BOOL GetGroundPos(const i_math::line3df &ray,i_math::vector3df &posHi) = 0;
	virtual BOOL GetGroundPos(float x,float z,i_math::vector3df &pos) = 0;
};






