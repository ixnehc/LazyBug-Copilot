#pragma once
#include "GuiLib.h"

struct GObjBase;

GuiLib_Api void GetGObjRefPath(GObjBase *obj,std::vector<std::string>*bufRes,
								std::vector<std::string>*bufTrrnBrLib,std::vector<std::string>*bufBrLib
								,std::vector<std::string>*bufMapFile,std::vector<unsigned __int64>*bufProto);
