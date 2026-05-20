/********************************************************************
	created:	2011/11/20   20:31
	file path:	d:\IxEngine\Common\bitpacket
	file base:	DataPacket
	file ext:	cpp
	author:		cxi
	
	purpose:	在datapacket的基础上增加了保存bit位的package
*********************************************************************/
#include "stdh.h"
#include "BitPacket.h"

#include <assert.h>
 
void CBitPacket::Bits_Write(DWORD v,DWORD c)
{
	assert(v<(1<<c));
	DWORD *p=(DWORD*)((_bits+(_iCurBit/8)));
	DWORD off=_iCurBit%8;

	DWORD mask=((1<<off)-1);
	(*p)&=mask;
	(*p)|=(v<<off);
	_iCurBit+=c;
}


