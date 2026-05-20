#pragma once

#include "gds/GObj.h"
#include "class/class.h"
#include "anim/KeySet.h"

struct ColorSet:public KeySet
{
	DEFINE_CLASS( ColorSet );
//	IMPLEMENT_REFCOUNT_C;

	BEGIN_GOBJ_PURE( ColorSet, 2 );
		GELEM_VARVECTOR(BYTE,_buf);
		GELEM_VAR_INIT(KeySetHead,_hd,KeySetHead(KT_Color,sizeof(Key_col)));
	END_GOBJ();

	// ±£´æ
	DWORD	GetVal( const float time );
	void	Reset();
	void	Reset(DWORD col);

};