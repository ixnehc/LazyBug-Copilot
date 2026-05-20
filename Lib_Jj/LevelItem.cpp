
#include "stdh.h"

#include "LevelItem.h"

#include "Level.h"


//////////////////////////////////////////////////////////////////////////
//CLevelItem
void CLevelItem::OnRelease()
{
	_Destroy();
	Class_Delete(this);
}

BOOL CLevelItem::Create()
{
	if (FALSE==OnCreate())
		return FALSE;
	_bAlive=1;
	_level->GetIDs()->Register(this);

	AddRef();
	return TRUE;
}

void CLevelItem::_Destroy()
{
	if (!IsAlive())
		return;

	OnDestroy();

	_bAlive=0;
	_level->GetIDs()->UnRegister(this);
}



void CLevelItem::Destroy()
{
	_Destroy();
	Release();
}
