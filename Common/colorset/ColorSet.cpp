#include "stdh.h"
#include "ColorSet/ColorSet.h"
#include <float.h>


DWORD ColorSet::GetVal( const float t )
{
	if (IsEmpty())
		return 0xffffffff;
	
	Key_col k;
	CalcKey<Key_col>(ANIMTICK_FROM_SECOND(t),&k);
	return k.color;
}

void ColorSet::Reset()
{
	Clean();
}

void	ColorSet::Reset(DWORD col)
{
	Clean();
	SetKeyCount(1);
	Key_col *k=((Key_col*)GetKey(0));
	k->t=0;
	k->color=col;
}
