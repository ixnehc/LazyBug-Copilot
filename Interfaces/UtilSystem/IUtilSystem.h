#pragma once

#include "UtilSystemDefines.h"

#include <string>
#include <vector>

class IRenderSystem;
class IFileSystem;

struct TexInfo;

class IUtilSystem
{
public:
	//working environment
	virtual void SetRS(IRenderSystem *pRS)=0;
	virtual void SetFS(IFileSystem *pFS)=0;
	virtual IRenderSystem *GetRS()=0;
	virtual IFileSystem *GetFS()=0;


};

