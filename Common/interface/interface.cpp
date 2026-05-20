/********************************************************************
	created:	2006/05/13
	created:	13:5:2006   15:09
	filename: 	D:\IxEngine\Interfaces\interface.cpp
	author:		cxi
	
	purpose:	interface implement
*********************************************************************/
#include "stdh.h"
#include "interface.h"
#include <string.h>
#include <stdlib.h> 

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;


InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) :
m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}



// ------------------------------------------------------------------------------------ //
// CreateInterface.
// ------------------------------------------------------------------------------------ //

extern "C" __declspec( dllexport )  void* CreateInterface( const char *pName)
{
	InterfaceReg *pCur;

	for(pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_pName, pName) == 0)
			return pCur->m_CreateFn();
	}
	return NULL;	
}


void *CreateInterfaceInModule( HMODULE hDll,const char *nameInterface)
{
	CreateInterfaceFn fn;
	fn=static_cast<CreateInterfaceFn>((void*)GetProcAddress( hDll, CREATEINTERFACE_PROCNAME));
	if (fn)
		return fn(nameInterface);
	return NULL;
}



