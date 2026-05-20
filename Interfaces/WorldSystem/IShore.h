
#pragma  once

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/IObjMap.h"

#include "valueset/valueset.h"

#include "WorldSystem/ICtrlPoint.h"

#include "WorldSystem/IBrushLib.h"

#include "gds/GObj.h"


class CtrlPt_Shore :public CtrlPoint
{
public:
	DEFINE_CLASS(CtrlPt_Shore);
	
	virtual void Clone(const CtrlPoint * p)
	{
		if(p){
			pos = p->pos;
			alpha = ((CtrlPt_Shore*)p)->alpha;
		}
	}

	virtual void Lerp(CtrlPoint * p0,CtrlPoint *p1,float r)
	{
		assert(p0&&p1);
		pos = (1.0f - r)*p0->pos + r*p1->pos;
		alpha = (1.0f-r)*((CtrlPt_Shore *)p0)->alpha + r*((CtrlPt_Shore *)p1)->alpha;
	}
	
	virtual BOOL Equals(CtrlPoint * p)
	{
		return (pos==p->pos)&&(alpha==((CtrlPt_Shore *)p)->alpha);
	}

	virtual void Load(CDataPacket &dp) 
	{
		alpha = dp.Data_NextFloat();
		CtrlPoint::Load(dp);
	}

	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextFloat() = alpha;
		CtrlPoint::Save(dp);
	}

	BEGIN_GOBJ_PURE(CtrlPt_Shore,1);
		GELEM_VAR_INIT(float,alpha,1.0f);
			GELEM_EDITVAR("Alpha",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"顶点alpha值。");
		GELEM_VAR(i_math::vector3df,pos);
	END_GOBJ()

public:
	float alpha;
};

struct ShoreInfo
{
	DEFINE_CLASS(ShoreInfo);

	BEGIN_GOBJ_PURE(ShoreInfo,1)
		GELEM_VAR_INIT(float,inner_w,1.0f);
			GELEM_EDITVAR("内宽",GVT_F,GSem(GSem_Float,"0.1f,20.0f,0.01f"),"内部宽度。");
		GELEM_VAR_INIT(float,outter_w,1.0f);
			GELEM_EDITVAR("外宽",GVT_F,GSem(GSem_Float,"0.1f,20.0f,0.01f"),"外部宽度。");
		GELEM_VAR_INIT(float,max_dist,200.0f);
			GELEM_EDITVAR("可视距离",GVT_F,GSem(GSem_Float,"10.0f,500.0f,0.1f"),"最远可见距离。");
		GELEM_VAR_INIT(float,trans_dist,10.0f);
			GELEM_EDITVAR("过渡长度",GVT_F,GSem(GSem_Float,"0,100.0f,0.1f"),"过渡区域的长度，在过渡区域内慢慢的消隐,在最远距离处aplha为零。");
		GELEM_VAR_INIT(float,delay,0.0f);
			GELEM_EDITVAR("延迟",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"延迟时间。");
	END_GOBJ()
	
	float inner_w,outter_w;		//内外宽度
	float max_dist,trans_dist;	//最远可见距离，过渡区域的距离
	float delay;				//时间延迟
};

class IShoreEditor : public IObjMapEditor
{
public:
	virtual void SetWireframeVisible(BOOL bVisible) = 0;

	virtual HMapObj AddShore(const ShoreInfo &info,ICtrlPointPack * pack,const BRUID & br) = 0;
	virtual const ShoreInfo * GetShoreInfo(const HMapObj &hObj) = 0;
	virtual HMapObj SetShoreInfo(const HMapObj &hObj,const ShoreInfo & info) = 0;
	virtual BOOL GetGroundPos(i_math::vector3df& pos) = 0;
	virtual ICtrlPointPack * GetCtrlPointPack(const HMapObj &hObj) = 0;
	virtual HMapObj SetCtrlPointPack(const HMapObj &hObj,ICtrlPointPack * pack) = 0;

	//波浪刷子相关
	virtual BOOL  AttachBrush(const HMapObj & hObj,const BRUID &idBr) = 0;
	virtual BRUID GetBrush(const HMapObj & hObj) = 0;
	
	//得到波浪库
	virtual IBrushLib * GetWaveLib() = 0;

	virtual ICtrlPointPack * NewCtrlPointPack()  = 0;
};






