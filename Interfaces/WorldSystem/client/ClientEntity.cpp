/********************************************************************
	created:	2011/6/28   16:36
	file path:	e:\IxEngine\Proj_Client
	author:		chenxi
	
	purpose:	Client Entity Base
*********************************************************************/
#include "stdh.h"

#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IEntitySystem.h"

#include "ClientEntity.h"

//////////////////////////////////////////////////////////////////////////
//CClientEntity

CClientEntity::CClientEntity()
{
	_core=NULL;
}

INael*CClientEntity::ObtainSubNael(const char *nm)
{
	if (_core)
		return _core->ObtainNael(nm);
	return NULL;
}
