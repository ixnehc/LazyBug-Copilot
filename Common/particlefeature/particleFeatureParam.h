

#ifndef _ParticleFeatureParam_H_
#define _ParticleFeatureParam_H_

#include "../class/class.h"
#include "gds/GObj.h"

enum ParticleFeatureType
{
	PARTICLE_UNKONW,
	PARTICLE_BROKENLINE,				// brokenline 可以spawn可以update
	PARTICLE_BROKENLINE_UPDATE,			// brokenline 只是在update时使用 编辑器里使用的参数
	PARTICLE_BROKENLINE_UPDATE_OF_TIME,	// 更新时依据时间
	PARTICLE_BROKENLINE_UPDATE_OF_lIFE,	// 更新时依据生命的百分比
	PARTICLE_FORMULA,					// 用公式描述的feature 比如sina函数描述的一个波 
};

struct FeatureParameter 
{	
	virtual CClass *GetClass() = 0;
	virtual GObjBase *GetGObj() = 0;

	ParticleFeatureType eType;
};

struct FeatureParameterBrokenLine : public FeatureParameter
{	
	std::vector<float>	vControlPoints;
	std::vector<float>	vValue;
	bool				bConst;
	DECLARE_CLASS( FeatureParameterBrokenLine );
	
	BEGIN_GOBJ_PURE( FeatureParameterBrokenLine, 1 ); 
		GELEM_VARVECTOR( float, vControlPoints )
		GELEM_VARVECTOR( float, vValue )
		GELEM_VAR( bool, bConst )
	END_GOBJ();
};

#endif