
#pragma once

#include "WorldSystem/IObjMap.h"

#include "ModBlockBack.h"

void CommitMapObjMod(CModManager * mgrMod,CGeView * view,std::vector<HMapObj>& hObjsMod,IObjMapEditor * editor);
void CommitMapObjMod(CModManager * mgrMod,CGeView * view,HMapObj &hObjMod,IObjMapEditor * editor);