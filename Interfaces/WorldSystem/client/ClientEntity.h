#pragma once

#include "WorldSystem/Client/IClient.h"
#include "WorldSystem/Client/INael.h"
#include "WorldSystem/IEntitySystemDefines.h"

#include "ClientDefines.h"

class IAsset;
class IEntity;
class ILuaObj;

class CAssetCtrl;

class INael;
class CClientEntity:public IClientEntity
{
public:
	CClientEntity();

	INael*ObtainSubNael(const char *nm);

	//这个函数是为了可以在派生类中方便的使用StbParam_XXXX(StbParam_XXXX要调用这个函数)
	void LuaDebugOutput(const char *type,const char *content,...){}


};



