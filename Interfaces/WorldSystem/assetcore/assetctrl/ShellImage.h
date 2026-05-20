
#pragma once

#include "WorldSystem/stubparams/param_sys.h"


struct ImageCombo:public PropRefTarget
{

	DEFINE_CLASS(ImageCombo);
	IMPLEMENT_REFCOUNT_C

	BEGIN_GOBJ_PURE(ImageCombo,1);
		GELEM_STRING(path);
			GELEM_EDITVAR("贴图路径",GVT_String,GSem_TexturePartPath,"贴图路径");
		GELEM_VAR_INIT(int,combo,0);//ImgCombo_Normal
			GELEM_EDITVAR("组合方式",GVT_S,GSem(GSem_Interger,"一整张,三行,九格"),"组合方式");
		GELEM_VAR_INIT(i_math::size2di,szEdge,i_math::size2di(16,16));
			GELEM_EDITVAR("边界部分的大小",GVT_Sx2,GSem_Size,"边界部分的大小");
	END_GOBJ();

	std::string path;
	int combo;//a ShellImageCombo value
	i_math::size2di szEdge;
};

