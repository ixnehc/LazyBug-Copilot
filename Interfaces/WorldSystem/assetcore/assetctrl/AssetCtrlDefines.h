#pragma once


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


//这个枚举专用于CAssetCtrl::FindChild(..),表示以什么原因寻找Child
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
