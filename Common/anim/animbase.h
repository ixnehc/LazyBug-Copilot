#pragma once

#include "../strlib/strlibdefines.h"

#include "animdefines.h"

namespace i_math
{
	template<class T>
	class circle;
}



enum KeyType
{
	KT_None,
	KT_Float,
	KT_Int,
	KT_Floatx2,
	KT_Pos,
	KT_Quat,
	KT_XForm,
	KT_Mat43_obsolete,
	KT_Color,
	KT_MapCoord,
	KT_Refs_obsolete,//references
	KT_Bones_obsolete,
	KT_Ref,
	KT_Shortx4,
	//XXXXX:More KeyType
};

struct Key_i;
struct Key_f;
struct Key_2f;
struct Key_pos;
struct Key_quat;
struct Key_xform;
struct Key_mat43;
struct Key_col;
struct Key_mapcoord;
struct Key_ref;
struct Key_s4;

//XXXXX:More KeyType


/// @brief 动画事件，表示动画在某个时间点需要触发的命名事件
struct AnimEvent
{
	/// @brief 默认构造函数，将name初始化为无效ID，tEvent初始化为0
	AnimEvent()
	{
		name=StringID_Invalid;
		tEvent=0;
	}

	StringID name;		///< 事件名称的字符串ID，用于标识该事件的语义（如脚步声、攻击判定等）
	AnimTick tEvent;	///< 事件触发的时间点（基于0的tick值），表示在动画片段内的相对时刻
};

struct AnimEventZone
{
	AnimEventZone()
	{
		name=StringID_Invalid;
		t=0;
		tp=None;
	}

	/// @brief 检查动画事件区域是否有效
	/// @return 如果区域名称有效且类型不为None，则返回TRUE；否则返回FALSE
	BOOL IsValid()
	{
		return name!=StringID_Invalid&&tp!=None;
	}

	struct KeyFan
	{
		KeyFan()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL IsValid()
		{
			if ((radianTo!=radianFrom)||(radianFrom!=radianTo))
				return TRUE;
			return FALSE;
		}
		BOOL CheckIn(i_math::vector2df &posTarget,float radiusTarget=0.0f);
		i_math::circle<float> CalcBoundingCircle();
		BOOL CalcInfo(i_math::vector3df &pos,i_math::vector3df &dir,float &fov);
		void CalcXfm(i_math::xformf &xfm);

		AnimTick t;
		i_math::xformf xfmCenter;
		float radiusInner;
		float radiusOutter;
		float radianFrom;
		float radianTo;
		float ht;
	};


	StringID name;
	AnimTick t;
	enum Type
	{
		None,
		Fan,
	};

	Type tp;

	BOOL IsIn(AnimTick t);
	BOOL CalcKeyFan(AnimTick t,KeyFan &k);
	BOOL CalcXform(AnimTick t,i_math::xformf &xfm);
	AnimTick GetDur();
	AnimTick GetStart();
	AnimTick GetEnd();

	std::vector<KeyFan> keysFan;

};
