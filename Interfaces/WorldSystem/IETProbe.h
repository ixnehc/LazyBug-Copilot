
#pragma  once

#include "IObjMap.h"

#include "gds/GObj.h"

struct ETProbeInfo
{
	DWORD grpID;
	BOOL  bDrawWater;
	std::string  texPath;
	i_math::vector3df pos;
	int typeFmt;		//格式类型  0:DXT1  1:DXT5  2:R5G6B5  3:R8G8B8
	int wMap;			//环境贴图的宽度
	
	BEGIN_GOBJ_PURE(ETProbeInfo,1)
		GELEM_VAR_INIT(DWORD,grpID,0);
			GELEM_EDITVAR("组号",GVT_U,GSem_StringID,"生成环境贴图时，同一组号的对象不绘制");
		GELEM_VAR_INIT(BOOL,bDrawWater,FALSE);
			GELEM_EDITVAR("屏蔽水面",GVT_S,GSem_Boolean,"生成环境贴图时是否绘制水面");
		GELEM_STRING_INIT(texPath,"");
			GELEM_EDITVAR("路径",GVT_String,GSem(GSem_FilePath,"[RESROOT]:dds:*.dds|*.dds"),"环境贴图存储的路径");
		GELEM_VAR_INIT(int,typeFmt,0);
			GELEM_EDITVAR("格式",GVT_S,GSem(GSem_Interger,"DXT1:0,DXT5:1,R5G6B5:2,A8R8G8B8:3"),"环境贴图的存储格式");
		GELEM_VAR_INIT(int,wMap,128);
			GELEM_EDITVAR("大小",GVT_S,GSem(GSem_Interger,"16x16:4,32x32:32,64x64:64,128x128:128,256x256:256,512x512:512"),"环境贴图的大小");
		GELEM_VAR(i_math::vector3df,pos);
	END_GOBJ()

};

class IETProbeEditor: public IObjMapEditor
{
public:
	virtual HMapObj AddETProbe(const ETProbeInfo &info) = 0;
	virtual const ETProbeInfo * GetETProbeInfo(const HMapObj & hObj) = 0;
	virtual HMapObj SetETProbe(const HMapObj &hObj,const ETProbeInfo &info) = 0;
};


