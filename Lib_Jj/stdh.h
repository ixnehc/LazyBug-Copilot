// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "commondefines/general.h"

#include "math/imath_all.h"
#include "records/recordsdefine.h"
#include "LevelDefines.h"
#include "class/class.h"
#include "linkpad/LinkPad.h"
#include "gds/GObj.h"
#include "gds/GObjEx.h"

#include "LevelOp.h"
#include "stringparser/stringparser.h"
#include "strlib/strlibdefines.h"
#include "ref/ref.h"
#include "anim/animdefines.h"

#include "records/records.h"

#include "LevelCost.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelBehavior.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"
#include "LevelBuff.h"

#include "LevelObj.h"
#include "LevelSkillDriver.h"

#include "BuffCalc.h"
#include "LevelOps.h"

#include "LevelDeal.h"

#include "LevelUpgrade.h"

#include "LevelUpgradableValue.h"

#include "LevelTech.h"

#include <vector>

