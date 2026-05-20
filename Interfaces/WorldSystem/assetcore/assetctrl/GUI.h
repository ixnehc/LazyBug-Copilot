/***************************************************************************
*   Copyright (C) 2004 - 2006 Paul D Turner & The CEGUI Development Team
*
*   Permission is hereby granted, free of charge, to any person obtaining
*   a copy of this software and associated documentation files (the
*   "Software"), to deal in the Software without restriction, including
*   without limitation the rights to use, copy, modify, merge, publish,
*   distribute, sublicense, and/or sell copies of the Software, and to
*   permit persons to whom the Software is furnished to do so, subject to
*   the following conditions:
*
*   The above copyright notice and this permission notice shall be
*   included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
*   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
*   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
*   OTHER DEALINGS IN THE SOFTWARE.
***************************************************************************/

/***************************************************************************
	created: 14:4:2008   15:33
	author:	 szg
	brief:	 
		90% logic source codes are from CEGUI(www.cegui.org.uk). I remove 
		some functions, and rewrite the controls's renderers.
***************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "math/imath_all.h"




#include "gds/GStub.h"
#include "gds/GProp.h"

#include "WorldSystem/stubparams/stubparams.h"
#include "WorldSystem/stubparams/param_ael.h"
#include "WorldSystem/stubparams/param_general.h"

#include "editor/ctrlop.h"

#pragma warning(disable:4244)


/*********************** consts ****************************/
const int SZ_MINSIZE			= 1;
const int SZ_MAXSIZE			= 16384;
const int SZ_BORDER				= 2;	// Border size
const int SZ_TITLEBAR			= 18;	// Title bar height
const int SZ_MENUBAR			= 16;	// MenuBar
const int SZ_MENUITEM			= 12;	// MenuItem
const int SZ_SLIDER				= 8;	// Slider
const int SZ_LISTITEM			= 20;	// ListBoxItem
const int SZ_LISTCOLUMN			= 22;	// ListColumn

const long CARAT_FLASHRATE		= 1000;	// 1s
const long TIP_DELAY =240;//1s

/*************************** end ****************************/


enum MouseButton
{
	LeftButton,
	RightButton,
	MiddleButton,
	MouseButtonCount,
	NoButton
};

// Event macro (From skyClient)
#define EVENT_BEGIN(Parent)		\
	enum eEvents				\
{								\
	EventBegin = Parent::EventEnd,

#define EVENT_END				\
	EventEnd,					\
};


//这个枚举用来表示表示以什么原因寻找Ctrl
enum FindCtrlReason
{
	FindCtrl_General,
	FindCtrl_MouseMove,
	FindCtrl_MouseDown,
	FindCtrl_MouseUp,
	FindCtrl_MouseWheel,
	FindCtrl_MouseDblClick,
	FindCtrl_MouseClick,
	FindCtrl_Tooltip,
};
