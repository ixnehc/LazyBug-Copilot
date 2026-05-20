/********************************************************************
	created:	2013/1/1 
	author:		cxi
	
	purpose:	Net Msg
*********************************************************************/
#include "stdh.h"
#include "../class/class.h"
#include "../gds/GObj.h"

#include "netmsg.h"

NetMsg *NetMsg::Clone()
{
	NetMsg *msgNew=(NetMsg *)GetClass()->New();

	DWORD sz;
	BYTE *data=GetData(sz);
	if (data)
	{
		DWORD sz2;
		memcpy(msgNew->GetData(sz2),data,sz);
	}
	else
		msgNew->GetGObj()->Copy(GetGObj());

	return msgNew;
}
