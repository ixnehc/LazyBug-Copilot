#pragma once

#include <dinput.h>

//	Input buffer size
//	????
#define DINPUT_BUFFER_SIZE		256
//	Maximum keyboard character number
//	??程才?
#define DINPUT_MAX_KEYS			256
//	Maximum mouse button number
//	公?程??
#define DINPUT_MAX_BUTTONS		4
//	Left button of mouse
//	公?オ?
#define DINPUT_LEFT_BUTTON		0
//	Right button of mouse
//	公??
#define DINPUT_RIGHT_BUTTON		1
//	Middle button of mouse
//	公?い?
#define DINPUT_MIDDLE_BUTTON	2
//	No action
//	??(公??┪??)
#define DINPUT_ACTION_NONE		0
//	Action of press
//	?(公??┪??)--????
#define DINPUT_ACTION_PRESS		1
//	Action of release
//	??(公??┪??)--???癬??
#define DINPUT_ACTION_RELEASE	2
//Action of double press key
//by Rosey:??砆??
#define DINPUT_ACTION_DOUBLE_PRESS 3



struct StateDI
{
	StateDI()
	{
		memset(this,0,sizeof(*this));
	}
	POINT ptCursor;
	BOOL bLClick;
	BOOL bRClick;
	BOOL bLDblClick;
	BOOL bRDblClick;
	BOOL bLDown;
	BOOL bRDown;
	BOOL bShiftDown;
	BOOL bCtrlDown;
};


class CDirectInput  
{
public:
	CDirectInput();
	virtual ~CDirectInput();

	void OnInit();
	void OnDestroy();
	//	DirectInput initialize
	//	DirectInput﹍て
	BOOL OnCreate(HINSTANCE hInst, HWND hWnd, LPCRECT lpRect);

	void Update(StateDI &state);

	//	Refresh state of keyboard and mouse
	//	公?㎝????穝
	//	nWorldX, nWorldY: ?玡辊オà?いГ?
	void OnUpdate(int nWorldX, int nWorldY);
	//	Refresh state of keyboard
	//	????穝(珹??玡??㎝????誹?瞶)
	void UpdateKeyboard();
	//	Refresh state of mouse
	//	公???穝(珹??玡??㎝????誹?瞶)
	void UpdateMouse(int nWorldX, int nWorldY);

	//	Decide if iKey is in down state;
	//	???﹚?琌???
	BOOL IsKeyDown(BYTE iKey);
	//	Decide if iKey is doing action of pressing
	//	???﹚?琌?ネ?(??癬??????)
	BOOL IsKeyPress(BYTE iKeyZ,BOOL bReset=FALSE);
	//	Decide if iKey is doing action of releasing
	//	???﹚?琌?ネ??(????癬????)
	BOOL IsKeyRelease(BYTE iKey);
	//adde by Rosey
	// decide if iKey is doing action of double press
	//???﹚?琌?ネ???
	BOOL IsKeyDoublePress(BYTE iKey);
	//	Decide if mouse left button is in down state;
	//	?公?オ?琌???
	BOOL IsMouseLBDown(void);
	//	Decide if mouse right button is in down state;
	//	?公??琌???
	BOOL IsMouseRBDown(void);
	//	Decide if mouse left button is doing action of pressing
	//	?公?オ?琌?ネ?(??癬??????)
	BOOL IsMouseLBPress(void);
	//	Decide if mouse right button is doing action of pressing
	//	?公??琌?ネ?(??癬??????)
	BOOL IsMouseRBPress(void);
	//	Decide if mouse left button is doing action of releasing
	//	?公?オ?琌?ネ??(????癬????)
	BOOL IsMouseLBRelease(void);
	//	Decide if mouse right button is doing action of releasing
	//	?公??琌?ネ??(????癬????)
	BOOL IsMouseRBRelease(void);
	//	Decide if mouse left button is double clicked
	//	?琌?ネ公?オ???(﹚????Ω?ネ?Ω??)
	BOOL IsMouseLBDoubleClick(void) { return m_bLBDoubleClick; }
	//	Decide if mouse right button is double clicked
	//	?琌?ネ公????(﹚????Ω?ネ?Ω??)
	BOOL IsMouseRBDoubleClick(void) { return m_bRBDoubleClick; }
	//	Decide if mouse left button is dragged
	//	?琌?ネ公?オ?╈Σ
	BOOL IsMouseLBDrag(void) { return m_bLBDrag; }
	//	Decide if mouse right button is dragged
	//	?琌?ネ公??╈Σ
	BOOL IsMouseRBDrag(void) { return m_bRBDrag; }
	//	Get move range of mouse pointer in last dragging
	//	眔公?╈?簿?璖?(?Г?)
	LPRECT	GetMouseDragRect(void)	{ return &m_rtDrag; }
	//	Get current screen coordinate of mouse pointer
	//	眔?玡公??辊Г?
	LPPOINT GetMousePos(void)		{ return &m_ptMouse; }
	void SetMousePos(POINT &pt);
	int CheckKey(UINT &nChar, UINT &nRepCnt);
		
public:
	//	A pointer point to DirectInput object
	//	DirectInputン?
	LPDIRECTINPUT8			m_lpDI;
	//	A pointer point to DirectInput keyboard device
	//	DirectInput?????
	LPDIRECTINPUTDEVICE8	m_lpDIDKeyboard;
	//	A pointer point to DirectInput mouse device
	//	DirectInput公????
	LPDIRECTINPUTDEVICE8	m_lpDIDMouse;

	//	Active range of mouse pointer
	//	公???璖?(辊Г?)
	CRect					m_rtScreen;
	//	Moving range of mouse pointer in dragging
	//	公?╈?簿?璖?(?Г?)
	CRect					m_rtDrag;
	//	Current screen coordinate of mouse pointer
	//	?玡公??辊Г?
	CPoint					m_ptMouse;

	CPoint					m_ptMouseLBLast;
	CPoint					m_ptMouseRBLast;

	//	Current keyboard keys state(up of down)
	//	???玡?????	
	DWORD					m_dwDoublePressKeyTime;
	BYTE					m_keyState[DINPUT_MAX_KEYS];
	DWORD					m_KeyLastPressTick[DINPUT_MAX_KEYS];
	//	Current mouse buttons state(up of down)
	//	???玡公????
	BYTE					m_buttonState[DINPUT_MAX_BUTTONS];
	//	Keys actions from last refresh
	//	???Ω穝?????(–Ω穝?竚)
	BYTE					m_keyAction[DINPUT_MAX_KEYS];
	//	Buttons actions from last refresh
	//	???Ω穝?公???(–Ω穝?竚)
	BYTE					m_buttonAction[DINPUT_MAX_BUTTONS];

	DWORD					m_dwLBThisTick;
	DWORD					m_dwRBThisTick;
	//	Tick of last release action of mouse left button
	//	Ω公?オ??ネ????
	DWORD					m_dwLBLastTick;
	//	Tick of last release action of mouse right button
	//	Ω公???ネ????
	DWORD					m_dwRBLastTick;
	//	Maximum time between two click of mouse button
	//	公???程???筳
	DWORD					m_dwDoubleClickTime;

	//	公?オ??ネΩ??(ノ????ノ,–Ω穝?ぃ竚)
	BOOL					m_bLBClick;
	//	公???ネΩ??(ノ????ノ,–Ω穝?ぃ竚)
	BOOL					m_bRBClick;
	//	Double click of mouse left button
	//	公?オ??ネ??
	BOOL					m_bLBDoubleClick;
	//	Double click of mouse right button
	//	公???ネ??
	BOOL					m_bRBDoubleClick;

	BOOL					m_bLBDrag;
	BOOL					m_bRBDrag;
	//	Whether mouse left button drag begin
	//	公?オ?,╈Σ?﹍
//	BOOL					m_bLBDragStart;
	//	Whether mouse right button drag begin
	//	公??,╈Σ?﹍
//	BOOL					m_bRBDragStart;


	//add by glc at 2003/9/27
    DWORD m_dwKeyboardCode;		//ボ獺??,?rgodKeyboard?
    DIDEVICEOBJECTDATA m_rgodKeyboard[DINPUT_BUFFER_SIZE];	/* Receives buffered data */
	//BYTE m_abyKeysBuffer[256];		//???
	
};


