/********************************************************************
	created:	2007/7/2   15:50
	filename: 	e:\IxEngine\Proj_WorldSystem\assert.cpp
	author:		cxi
	
	purpose:	Asset Base Class
*********************************************************************/
#include "stdh.h"

#include "class/class.h"

#include <assert.h>
#include "asset.h"




//////////////////////////////////////////////////////////////////////////
//CAsset

void CAsset::Test()
{
}

void CAsset::LuaDebugOutput(const char *type,const char *content,...)
{
	if (_ss->dlgtDebugOutput)
	{
		static char buf[2048];

		va_list args;   
		va_start(args,content);
		_vsnprintf(buf,sizeof(buf),content,args);
		va_end(args);

		_ss->dlgtDebugOutput(type,buf);
	}
}

const char *CAsset::GetDebugLocation()
{
	if (this==NULL)
		return "";
	if (!_ss)
		return "";
	if (_ss->dlgtGetDebugLocation)
		return _ss->dlgtGetDebugLocation(this);
	return GetShowName();
}




void CAsset::OnRelease()
{
	_Destroy();
	Class_Delete(this);
}


const char *CAsset::GetClassName()
{		
	return GetClass()->GetName();	
}



BOOL CAsset::Save(CDataPacket &dp)
{
	_SaveComponent(dp);

	return TRUE;
}

BOOL CAsset::Load(CDataPacket &dp)
{
	_LoadComponent(dp);

	return TRUE;
}

void CAsset::_Destroy()
{	
	if (!IsAlive())
		return;
	OnDestroy();
	_ClearComonent();
	DisableStub();

	ClearBit(AssetBit_Alive);
}

void CAsset::Destroy()
{
	_Destroy();
	Release();
}

void CAsset::DeferredDestroy()
{
	_ss->dc.Add(this);
}


BOOL CAsset::_Create(IAsset *parent)	
{	
	_CreateComponent(parent);
	if (FALSE==OnCreate())
		return FALSE;
	SetBit(AssetBit_Alive);


	return TRUE;
}


BOOL CAsset::Create(AssetCreateArg &arg,AssetSystemState *ss)
{
	_bits=0;
	_ss=ss;
	if (FALSE==_Create(arg.parent))
		return FALSE;
	AddRef();
	return TRUE;
}

