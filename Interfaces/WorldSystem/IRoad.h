
#pragma  once 

#include "WorldSystem/IObjMap.h"

#include "WorldSystem/ICtrlPoint.h"

#include "gds/GObj.h"
#include "class/class.h"

class ITrrnMapEditor;

//路面的初始信息
struct RoadProp
{
	DEFINE_CLASS(RoadProp);

	BEGIN_GOBJ_PURE(RoadProp,1);
	GELEM_STRING_INIT(pathMtrl,"_std\\road\\road.mtl");
		GELEM_EDITVAR("材质",GVT_String,GSem_MtrlPath,"材质路径");
	GELEM_STRING_INIT(pathTexDif,"_std\\road\\none.dds");
		GELEM_EDITVAR("漫反射图",GVT_String,GSem_TexturePath,"漫反射贴图路径");
	GELEM_VAR_INIT(int,nTexUnit,8);
		GELEM_EDITVAR("块数",GVT_U,GSem(GSem_Interger,"[ 3 ]:3,[ 4 ]:4,[ 5 ]:5,[ 6 ]:6,[ 7 ]:7,[ 8 ]:8"),"贴图上小块的个数。");
	GELEM_VAR_INIT(float,uvtileLen,1.0f);
		GELEM_EDITVAR("贴图块长度",GVT_F,GSem(GSem_Float,"0.1f,20.0f,0.01f"),"贴图单位块展开长度(单位米)。");
	GELEM_VAR_INIT(float,w,3.0f);
		GELEM_EDITVAR("路宽",GVT_F,GSem(GSem_Float,"0.1f,20.0f,0.01f"),"路面的宽度。");
	GELEM_VAR_INIT(int,segw,2);
		GELEM_EDITVAR("横向段数",GVT_U,GSem_Interger,"路面横向段数。");
	GELEM_VAR_INIT(int,segh,2);
		GELEM_EDITVAR("纵向段数",GVT_U,GSem_Interger,"路面纵向段数。");
	GELEM_VAR_INIT(float,ratioTex,1.0f);
		GELEM_EDITVAR("贴图的宽高比",GVT_F,GSem(GSem_Float,"0.01f,100.0f,0.01"),"贴图的宽度与高度的比值");
	GELEM_VAR_INIT(float,bias,0.05f);
		GELEM_EDITVAR("高度偏移",GVT_F,GSem(GSem_Float,"-1.0f,1.0f,0.01"),"高度偏移距离");
	GELEM_VAR_INIT(float,uvoff,0.0f);
		GELEM_EDITVAR("uv偏移",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01"),"uv偏移距离");
	GELEM_VAR_INIT(DWORD,lvlRender,0);
		GELEM_EDITVAR("绘制层级",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"层级越高的Road显示在越上面");
	GELEM_VAR_INIT(int,mode,2);
		GELEM_EDITVAR("模式",GVT_U,GSem(GSem_Interger,"[平铺]:1,[拉伸]:2"),"贴图在路面上铺设模式。");
	END_GOBJ();

	float w;				//路的宽度
	int segw,segh;			//路的横向和纵向分段数
	std::string pathMtrl;	//材质路径
	int nTexUnit;			//贴图上具有意义的块数
	float uvtileLen;		//纹理贴图展开长度
	std::string pathTexDif;	
	float ratioTex;
	int mode;
	float bias;
	float uvoff;
	DWORD lvlRender;
};

#define ROAD_POSH2GROUD	0.05f

class IRoadEditor :public IObjMapEditor
{
public:
	virtual HMapObj AddRoad(ITrrnMapEditor * trrn,ICtrlPointPack * pack,const RoadProp &prop) = 0;
	virtual ICtrlPointPack * GetRoad(const HMapObj & hObj) = 0;

	virtual RoadProp * GetRoadProp(const HMapObj &hObj) = 0;
	virtual BOOL SetRoadProp(const HMapObj &hObj,const RoadProp &prop) = 0;//iSeg = -1 表示重置所有的

    virtual void BeginModOpacity(const HMapObj &hObj) = 0;
    virtual void ModOpacity(const HMapObj &hObj, i_math::vector3df &posCenter, float delta, float radiusInner, float radiusOutter) = 0;
    virtual void EndModOpacity(const HMapObj &hObj) = 0;

	virtual HMapObj SetCtrlPointPack(const HMapObj &hObj,ITrrnMapEditor * trrn,ICtrlPointPack * pack) = 0;
	virtual ICtrlPointPack * NewCtrlPointPack() = 0;
};




