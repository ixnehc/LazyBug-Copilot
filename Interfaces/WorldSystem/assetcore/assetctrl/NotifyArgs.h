#pragma once
#include "AssetCtrlPrerequisites.h"

struct MouseNotifyArgs : public GProperty
{
public:
	i_math::pos2di		position;		//!< holds current mouse position.
	int			button;			//!< describing the mouse button causing the event (for button inputs only)
	int			sysKeys;		//!< current state of the system keys and mouse buttons.

public:
	DECLARE_CLASS(MouseNotifyArgs)

	BEGIN_GOBJ(MouseNotifyArgs, 1)
		GELEM_VAR(i_math::pos2di, position)
		GELEM_VAR(int, button)
		GELEM_VAR(int, sysKeys)

	END_GOBJ();
};

struct KeyNotifyArgs : public GProperty
{
public:
	int			scanCode;
	int			sysKeys;

public:
	DECLARE_CLASS(KeyNotifyArgs)

	BEGIN_GOBJ(KeyNotifyArgs, 1)
		GELEM_VAR(int, scanCode)
		GELEM_VAR(int, sysKeys)

	END_GOBJ();
};
