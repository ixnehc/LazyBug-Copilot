#pragma once

#include "WorldSystem/stubparams/stubparams.h"

#include "gds/GStub.h"





#define BEGIN_CE_SIGNAL_HANDLER(_ownerclss) GStubBegin(_ownerclss)

#define ON_SIGNAL(__name,__handler)																					\
			{																																		\
				static GSlot<StbParams,OwnerClassType>org;														\
				last=&org;																													\
				int idx=_stubs().stubs2.size();																					\
				_stubs().stubs[std::string(__name)]=idx;																	\
				_stubs().stubs2.push_back(&org);																				\
				org.funcSet=&OwnerClassType::__handler;																\
				org.type=GStub_Slot;																									\
				org.name=__name;																									\
				org.ownername=ownername;																					\
				org.idx=idx;																												\
				org.idxConn=_stubs().nConn;																					\
				_stubs().nConn++;																										\
			}


#define END_CE_SIGNAL_HANDLER() GStubEnd()

#define DECLARE_CLIENT_ENTITY(clss)																					\
DECLARE_CLASS(clss);																												\
static int __idxCE;

#define IMPLEMENT_CLIENT_ENTITY(clss,pth)																		\
IMPLEMENT_CLASS(clss);																											\
extern int AddClientEntityDesc(const char *path,CClass *clss);												\
int clss::__idxCE=AddClientEntityDesc(pth,Class_Ptr(clss));



