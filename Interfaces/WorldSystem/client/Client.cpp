/********************************************************************
	created:	2011/6/29   10:33
	file path:	e:\IxEngine\Proj_Client
	author:		chenxi
	
	purpose:	Client Main Define
*********************************************************************/
#include "stdh.h"

#include "Client.h"

#include "stringparser/stringparser.h"


std::vector<ClientEntityDesc>&GetClientDescBuf()
{
	static std::vector<ClientEntityDesc>buf;
	return buf;
}


int AddClientEntityDesc(const char *path,CClass *clss)
{
//	StringLower((char*)path);

	std::vector<ClientEntityDesc>&buf=GetClientDescBuf();

	ClientEntityDesc t;
	t.clss=clss;
	t.pathProto=path;
	buf.push_back(t);
	return buf.size();
}





CClient::CClient()
{
}



DWORD CClient::GetClientEntityDescs(ClientEntityDesc *&descs)
{
	std::vector<ClientEntityDesc>&buf=GetClientDescBuf();
	descs=buf.data();
	return buf.size();
}
