#pragma once

#include "WorldSystem/Client/IClient.h"


struct EntitySystemState;

class CClient:public IClient
{
public:
	CClient();

	virtual DWORD GetClientEntityDescs(ClientEntityDesc *&descs);


};




