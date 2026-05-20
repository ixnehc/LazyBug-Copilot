
#pragma once

#include "class/class.h"

extern void InitLevelClasses();

class CLevelObjSrc;
class CLevelObjParam;
class CLevelObj;
class CLevelItem;
class CLevelOp;
class CLevelSkill;
class CLevelBuff;

//注意:以下函数的返回值都不带引用计数
extern CLevelObjSrc*NewLevelObjSrc(ClassUID uid);
extern CLevelObjParam*NewLevelObjParam(ClassUID uid);
extern CLevelObj*NewLevelObj(ClassUID uid);
extern CLevelItem*NewLevelItem(ClassUID uid);
extern CLevelOp*NewLevelOp(ClassUID uid);
extern CLevelSkill*NewLevelSkill(ClassUID uid);
extern CLevelBuff*NewLevelBuff(ClassUID uid);