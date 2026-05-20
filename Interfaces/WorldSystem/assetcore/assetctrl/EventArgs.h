
#pragma once
#include "WorldSystem/IAssetShell.h"

#include "AssetCtrlDefines.h"

class CAssetCtrl;

struct WindowEventArgs
{
public:
	WindowEventArgs(CAssetCtrl* wnd) : handled(FALSE), hasWindow(FALSE),window(wnd)
	{ 
		this->hasWindow = TRUE; 
	}
	
	BOOL	handled;	//!< handlers should set this to TRUE if they handled the event, or FALSE otherwise.
	BOOL	hasWindow; //!< Indicates if this event set has a parent window.

	CAssetCtrl*	window;			//!< pointer to a Window object of relevance to the event.
};

struct MouseEventArgs : public WindowEventArgs
{
public:
	MouseEventArgs(CAssetCtrl* wnd) : WindowEventArgs(wnd) {}

	ShellPos		position;		//!< holds current mouse position.
	ShellPos	moveDelta;		//!< holds variation of mouse position from last mouse input
	MouseButton	button;			//!< one of the MouseButton enumerated values describing the mouse button causing the event (for button inputs only)
	int			sysKeys;		//!< current state of the system keys and mouse buttons.
	int		wheelChange;	//!< Holds the amount the scroll wheel has changed.
	int			clickCount;     //!< Holds number of mouse button down events currently counted in a multi-click sequence (for button inputs only).
};

struct KeyEventArgs : public WindowEventArgs
{
public:
	KeyEventArgs(CAssetCtrl* wnd) : WindowEventArgs(wnd) {}

	int			scanCode;
	int			sysKeys;
};

/*!
\brief
	WindowEventArgs based class that is used for Activated and Deactivated window events
*/
struct ActivationEventArgs : public WindowEventArgs
{
public:
	ActivationEventArgs(CAssetCtrl* wnd) : WindowEventArgs(wnd) {}

	CAssetCtrl*	otherWindow;	//!< Pointer to the other window involved in the activation change.
};
