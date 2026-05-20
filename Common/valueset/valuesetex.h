#ifndef _BrokenLineEx_H_
#define _BrokenLineEx_H_

#include "brokenLine/brokenLine.h"
#include "datapacket/DataPacket.h"
#include "gds/GObj.h"
#include "class/class.h"


// brokenline gobj版本
class BrokenLineEx : public BrokenLine
{
public:
	DEFINE_CLASS( BrokenLineEx );
	IMPLEMENT_REFCOUNT_C;

	BEGIN_GOBJ_PURE( BrokenLineEx, 1 );
		GELEM_VARVECTOR( i_math::pos2df, m_vecPoint );
		GELEM_VAR_INIT( int, m_nLoopIndex, -1 );
		GELEM_VAR_INIT( float, m_fLoopStep, 0.0f );
		GELEM_VAR_INIT( i_math::pos2df, m_translate, i_math::pos2df( 0.0f, 0.0f ) );
		GELEM_VAR_INIT( float, m_fScale, 1.0f );
	END_GOBJ();

	// 保存
	void Save( CDataPacket &dp )
	{
		GSave( dp );
	}
	// 读取
	BOOL Load( CDataPacket &dp, BOOL *bRepaired )
	{
		return GetGObj()->Load( dp, bRepaired );
	}

	inline void	SetScale( float fScale )
	{
		m_fScale = fScale;
	}
	
	float GetScale()
	{
		return m_fScale;
	}

	void SetTranslate( const i_math::pos2df &pos )
	{
		m_translate = pos;
	}

	i_math::pos2df GetTranslate()
	{
		return m_translate;
	}
protected:
	// 编辑时的gg使用的信息
	float			m_fScale;
	i_math::pos2df	m_translate;
};


#endif