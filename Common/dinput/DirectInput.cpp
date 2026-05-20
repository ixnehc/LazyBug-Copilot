/********************************************************************
	created:	2007/1/17   17:36
	filename: 	e:\IxEngine\Common\dinput\DirectInput.cpp
	author:		cxi
	
	purpose:	direct input wrap
*********************************************************************/

#include "stdh.h"

#include "DirectInput.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectInput::CDirectInput()
{
	OnInit();
}

CDirectInput::~CDirectInput()
{
	OnDestroy();
}

void CDirectInput::OnInit()
{
	m_lpDI = NULL;
	m_lpDIDKeyboard = NULL;
	m_lpDIDMouse = NULL;

	for(int i = 0; i < DINPUT_MAX_KEYS; i++)
	{
		m_keyState[i] = 0;
		m_keyAction[i] = DINPUT_ACTION_NONE;
	}

	for(i = 0; i < DINPUT_MAX_BUTTONS; i++)
	{
		m_buttonState[i] = 0;
		m_buttonAction[i] = DINPUT_ACTION_NONE;
	}
}

void CDirectInput::OnDestroy()
{
	if(m_lpDIDKeyboard)
	{
		m_lpDIDKeyboard->Unacquire();
		m_lpDIDKeyboard->Release();
		m_lpDIDKeyboard = NULL;
	}

	if(m_lpDIDMouse)
	{
		m_lpDIDMouse->Unacquire();
		m_lpDIDMouse->Release();
		m_lpDIDMouse = NULL;
	}

	SAFE_RELEASE(m_lpDI);
}

///////////////////////////////////////////////////////////////////////
//	DirectInput initialize
//	DirectInput初始化
BOOL CDirectInput::OnCreate(HINSTANCE hInst, HWND hWnd, LPCRECT lpRect)
{
	_ASSERT(hInst != NULL);
	_ASSERT(hWnd != NULL);
	_ASSERT(lpRect != NULL);

	HRESULT	hr;

	//	Create DirectInput object
	//	創建DirectInput物件
	hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_lpDI, NULL);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		AfxMessageBox("create");
		OnDestroy();
		return FALSE;
	}

	//	Create DirectInput mouse device
	//	創建DirectInput鼠標指針設備
	hr = m_lpDI->CreateDevice(GUID_SysKeyboard, (LPDIRECTINPUTDEVICE8*)&m_lpDIDKeyboard, NULL);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		AfxMessageBox("device");
		OnDestroy();
		return FALSE;
	}

	//	Set to default keyboard data format
	//	設置缺省鍵盤數據格式
	hr = m_lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Set keyboard cooperative level
	//	設置鍵盤操作級別
	hr = m_lpDIDKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		AfxMessageBox("keyboard");
		OnDestroy();
		return FALSE;
	}

	//	Set keyboard data buffer
	//	設置鍵盤數據緩沖區
	DIPROPDWORD	dipdw = 
	{
		{
			sizeof(DIPROPDWORD),
			sizeof(DIPROPHEADER),
			0,
			DIPH_DEVICE,
		},
		DINPUT_BUFFER_SIZE
	};

	hr = m_lpDIDKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Acquire keyboard
	//	啟用鍵盤
	hr = m_lpDIDKeyboard->Acquire();
	while(hr==DIERR_OTHERAPPHASPRIO)
	{
//		::SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetForegroundWindow(hWnd);
		hr = m_lpDIDKeyboard->Acquire();
	}
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		AfxMessageBox("keyboard_acquire");
		OnDestroy();
		return FALSE;
	}

	//	Create DirectInput mouse device
	//	創建DirectInput鼠標指針設備
	hr = m_lpDI->CreateDevice(GUID_SysMouse, (LPDIRECTINPUTDEVICE8*)&m_lpDIDMouse, NULL);
 	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Set to default mouse data format
	//	設置缺省鼠標數據格式
	hr = m_lpDIDMouse->SetDataFormat(&c_dfDIMouse);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Set mouse cooperative level
	//	設置鼠標操作級別
	hr = m_lpDIDMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Set mouse data buffer
	//	設置鼠標數據緩沖區
	hr = m_lpDIDMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Acquire mouse
	//	啟用鼠標
	hr = m_lpDIDMouse->Acquire();
	while(hr==DIERR_OTHERAPPHASPRIO)
	{
		//		::SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetForegroundWindow(hWnd);
		hr = m_lpDIDMouse->Acquire();
	}
	_ASSERT(hr == DI_OK);
	if FAILED(hr)
	{
		OnDestroy();
		return FALSE;
	}

	//	Set mouse pointer moving range
	//	設置鼠標指針移動范圍
	m_rtScreen = *lpRect;

	//	Center mouse pointer
	//	鼠標指針居中
	m_ptMouse = m_rtScreen.TopLeft();
	m_ptMouse.x += m_rtScreen.Width() / 2;
	m_ptMouse.y += m_rtScreen.Height() / 2;

	//	Get maximum double click time
	//	設置雙擊時間
	m_dwDoubleClickTime = ::GetDoubleClickTime();
	if((m_dwDoubleClickTime <= 0) || (m_dwDoubleClickTime > 1000))
	{
		m_dwDoubleClickTime = 500;
	}
	
	//	Mouse state initialize
	//	鼠標狀態初始化
	m_bLBClick = FALSE;
	m_bRBClick = FALSE;
	m_bLBDoubleClick = FALSE;
	m_bRBDoubleClick = FALSE;
	m_bLBDrag = FALSE;
	m_bRBDrag = FALSE;
//	m_bLBDragStart = FALSE;
//	m_bRBDragStart = FALSE;

	m_dwLBThisTick = GetTickCount();
	m_dwRBThisTick = GetTickCount();
	m_dwLBLastTick = GetTickCount();
	m_dwRBLastTick = GetTickCount();

	//added by Rosey
	m_dwDoublePressKeyTime = 500;
	memset(m_KeyLastPressTick,0,sizeof(DWORD)*DINPUT_MAX_KEYS);
	return TRUE;
}

void CDirectInput::Update(StateDI &state)
{
	OnUpdate(0,0);

	state.ptCursor = *GetMousePos();
	state.bLClick = IsMouseLBRelease();
	state.bRClick = IsMouseRBRelease();
	state.bLDblClick = IsMouseRBDoubleClick();
	state.bRDblClick = IsMouseRBDoubleClick();
	state.bLDown = IsMouseLBDown();
	state.bRDown = IsMouseRBDown();

	state.bShiftDown=(IsKeyDown(DIK_LSHIFT)||IsKeyDown(DIK_RSHIFT));
	state.bCtrlDown=(IsKeyDown(DIK_LCONTROL)||IsKeyDown(DIK_RCONTROL));
}


///////////////////////////////////////////////////////////////////////
//	Refresh state of keyboard and mouse
//	鼠標和鍵盤狀態刷新
//	nWorldX, nWorldY: 當前屏幕左上角在大地圖中座標
void CDirectInput::OnUpdate(int nWorldX, int nWorldY)
{
	//	Refresh state of keyboard
	//	鍵盤狀態刷新(包括讀取當前狀態和緩沖區數據處理)
	UpdateKeyboard();
	//	Refresh state of mouse
	//	鼠標狀態刷新(包括讀取當前狀態和緩沖區數據處理)
	UpdateMouse(nWorldX, nWorldY);
}

///////////////////////////////////////////////////////////////////////
//	Refresh state of keyboard
//	鍵盤狀態刷新(包括讀取當前狀態和緩沖區數據處理)
void CDirectInput::UpdateKeyboard()
{
	//	Set last saved state to default value before refresh
	//	每次刷新前先重置上次紀錄的狀態
	for(int i = 0; i < DINPUT_MAX_KEYS; i++)
	{
		m_keyState[i] = 0;
		m_keyAction[i] = DINPUT_ACTION_NONE;
	}

	HRESULT hr;

	//	Get current state of all keys
	//	得到鍵盤所有按鍵的當前狀態
	hr = m_lpDIDKeyboard->GetDeviceState(DINPUT_MAX_KEYS, m_keyState);
	if(hr == DIERR_INPUTLOST)
	{
		hr = m_lpDIDKeyboard->Acquire();
		if(hr == DI_OK)
			hr = m_lpDIDKeyboard->GetDeviceState(DINPUT_MAX_KEYS, m_keyState);
	}
	if(FAILED(hr))
	{
		hr = m_lpDIDKeyboard->Acquire();
		if(FAILED(hr))
		{
//				WriteSystemLog("keyboard faildfdsfsa!");
			return;
		}
	}

	///DWORD	dwInOut = DINPUT_BUFFER_SIZE;
	m_dwKeyboardCode = DINPUT_BUFFER_SIZE;
	//DIDEVICEOBJECTDATA	rgdodKeyboard[DINPUT_BUFFER_SIZE];

	//	Get data from keyboard data buffer from last update
	//	得到從上一次刷新到現在鍵盤緩沖區中的數據
	//hr = m_lpDIDKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdodKeyboard, &dwInOut, 0);
	hr = m_lpDIDKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_rgodKeyboard, &m_dwKeyboardCode, 0);
	if(hr == DIERR_INPUTLOST)
	{
		hr = m_lpDIDKeyboard->Acquire();
		if(hr == DI_OK)
			hr = m_lpDIDKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_rgodKeyboard, &m_dwKeyboardCode, 0);
	}
	if(FAILED(hr))
	{
		hr = m_lpDIDKeyboard->Acquire();
		return;
	}

	if(m_dwKeyboardCode == 0)
		return;

	//	Save data in data buffer
	//	紀錄緩沖區中的數據
	for(i = 0; i < m_dwKeyboardCode; i++)
	{
		if(m_rgodKeyboard[i].dwData & 0x80)
			m_keyAction[m_rgodKeyboard[i].dwOfs] = DINPUT_ACTION_PRESS;
		else
		{
			
			DWORD tick = GetTickCount();
			if (tick - m_KeyLastPressTick[m_rgodKeyboard[i].dwOfs] < m_dwDoubleClickTime)
			{
				m_keyAction[m_rgodKeyboard[i].dwOfs] = DINPUT_ACTION_DOUBLE_PRESS;				
			}
			else
			{
				m_keyAction[m_rgodKeyboard[i].dwOfs] = DINPUT_ACTION_RELEASE;
			}
			m_KeyLastPressTick[m_rgodKeyboard[i].dwOfs] = tick;
		}
	}
}

///////////////////////////////////////////////////////////////////////
//	Refresh state of mouse
//	鼠標狀態刷新(包括讀取當前狀態和緩沖區數據處理)
void CDirectInput::UpdateMouse(int nWorldX, int nWorldY)
{
	//	Set state to default value before refresh
	//	每次刷新前先重置上次紀錄的狀態
	for(int i = 0; i < DINPUT_MAX_BUTTONS; i++)
	{
		m_buttonState[i] = 0;
		m_buttonAction[i] = DINPUT_ACTION_NONE;
	}

	m_bLBDrag = FALSE;
	m_bRBDrag = FALSE;
	m_bLBDoubleClick = FALSE;
	m_bRBDoubleClick = FALSE;

	//	Add by Li Yong at 07/11/00
	//	Clear state of m_bLBClick if time from last mouse left button clicked to now more than double click time
	//	從上一次鼠標左鍵釋放狀態到現在的時間超過雙擊時間則清m_bLBClick標志
	if(m_bLBClick == TRUE)
	{
		m_dwLBThisTick = GetTickCount();
		if(m_dwLBThisTick - m_dwLBLastTick > m_dwDoubleClickTime)
		{
			m_bLBClick = FALSE;
		}
	}

	//	Clear state of m_bLBClick if time from last mouse right button clicked to now more than double click time
	//	從上一次鼠標右鍵釋放狀態到現在的時間超過雙擊時間則清m_bLBClick標志
	if(m_bRBClick == TRUE)
	{
		m_dwRBThisTick = GetTickCount();
		if(m_dwRBThisTick - m_dwRBLastTick > m_dwDoubleClickTime)
		{
			m_bRBClick = FALSE;
		}
	}
	//	End add

	HRESULT hr;
	DIMOUSESTATE	diMouseState;
	//	Get current state of mouse
	//	得到鼠標的當前狀態
	hr = m_lpDIDMouse->GetDeviceState(sizeof(diMouseState), &diMouseState);
	if(hr == DIERR_INPUTLOST)
	{
		hr = m_lpDIDMouse->Acquire();
		if(hr == DI_OK)
			hr = m_lpDIDMouse->GetDeviceState(sizeof(diMouseState), &diMouseState);
	}
	if(FAILED(hr))
	{
		hr = m_lpDIDMouse->Acquire();
		return;
	}

	m_buttonState[DINPUT_LEFT_BUTTON] = diMouseState.rgbButtons[0];
	m_buttonState[DINPUT_RIGHT_BUTTON] = diMouseState.rgbButtons[1];
	m_buttonState[DINPUT_MIDDLE_BUTTON] = diMouseState.rgbButtons[2];

	DWORD	dwInOut = DINPUT_BUFFER_SIZE;
	DIDEVICEOBJECTDATA	rgdodMouse[DINPUT_BUFFER_SIZE];
	//	Get data from mouse data buffer from last update
	//	得到從上一次刷新到現在鼠標緩沖區中的數據
	hr = m_lpDIDMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdodMouse, &dwInOut, 0);
	if(hr == DIERR_INPUTLOST)
	{
		hr = m_lpDIDMouse->Acquire();
		if(hr == DI_OK)
			hr = m_lpDIDMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdodMouse, &dwInOut, 0);
	}
	if(FAILED(hr))
	{
		hr = m_lpDIDMouse->Acquire();
		return;
	}

	if(dwInOut == 0)
		return;

	//	Save data in data buffer
	//	紀錄緩沖區中的數據
	for(i = 0; i < dwInOut; i++)
	{
		switch(rgdodMouse[i].dwOfs)
		{
		case DIMOFS_X:
			m_ptMouse.x += rgdodMouse[i].dwData;
			m_ptMouse.x = (m_ptMouse.x < m_rtScreen.left) ? m_rtScreen.left : m_ptMouse.x;
			m_ptMouse.x = (m_ptMouse.x > m_rtScreen.right) ? m_rtScreen.right : m_ptMouse.x;
			break;
		case DIMOFS_Y:
			m_ptMouse.y += rgdodMouse[i].dwData;
			m_ptMouse.y = (m_ptMouse.y < m_rtScreen.top) ? m_rtScreen.top : m_ptMouse.y;
			m_ptMouse.y = (m_ptMouse.y > m_rtScreen.bottom) ? m_rtScreen.bottom : m_ptMouse.y;
			break;
		case DIMOFS_BUTTON0:
			if(rgdodMouse[i].dwData & 0x80)
				m_buttonAction[DINPUT_LEFT_BUTTON] = DINPUT_ACTION_PRESS;
			else
				m_buttonAction[DINPUT_LEFT_BUTTON] = DINPUT_ACTION_RELEASE;
			break;
		case DIMOFS_BUTTON1:
			if(rgdodMouse[i].dwData & 0x80)
				m_buttonAction[DINPUT_RIGHT_BUTTON] = DINPUT_ACTION_PRESS;
			else
				m_buttonAction[DINPUT_RIGHT_BUTTON] = DINPUT_ACTION_RELEASE;
			break;
		case DIMOFS_BUTTON2:
			if(rgdodMouse[i].dwData & 0x80)
				m_buttonAction[DINPUT_MIDDLE_BUTTON] = DINPUT_ACTION_PRESS;
			else
				m_buttonAction[DINPUT_MIDDLE_BUTTON] = DINPUT_ACTION_RELEASE;
			break;
		}
	}

	//	Drag start
	//	鼠標左鍵開始拖曳,紀錄開始位置
	if(m_buttonAction[DINPUT_LEFT_BUTTON] == DINPUT_ACTION_PRESS)
	{
		//	Modified by Li Yong 08/08/2000
		//	Now m_rtDrag save the map coordinates of drag start and end position
//		m_rtDrag.left = m_ptMouse.x;
//		m_rtDrag.top = m_ptMouse.y;
		m_rtDrag.left = m_ptMouse.x + nWorldX;
		m_rtDrag.top = m_ptMouse.y + nWorldY;
//		m_rtDrag.right = m_ptMouse.x + nWorldX;
//		m_rtDrag.bottom = m_ptMouse.y + nWorldY;
	}

	//	Drag end
	//	鼠標左鍵結束拖曳,紀錄結束位置
	if(m_buttonAction[DINPUT_LEFT_BUTTON] == DINPUT_ACTION_RELEASE)
	{
		//	Modified by Li Yong 08/08/2000
		//	Now m_rtDrag save the map coordinates of drag start and end position
//		m_rtDrag.right = m_ptMouse.x;
//		m_rtDrag.bottom = m_ptMouse.y;
		m_rtDrag.right = m_ptMouse.x + nWorldX;
		m_rtDrag.bottom = m_ptMouse.y + nWorldY;

		if((m_rtDrag.left != m_rtDrag.right) || (m_rtDrag.top != m_rtDrag.bottom))
			m_bLBDrag = TRUE;
	}
	
	//	Drag start
	//	鼠標右鍵開始拖曳,紀錄開始位置
	if(m_buttonAction[DINPUT_RIGHT_BUTTON] == DINPUT_ACTION_PRESS)
	{
		m_rtDrag.left = m_ptMouse.x + nWorldX;
		m_rtDrag.top = m_ptMouse.y + nWorldY;
//		m_rtDrag.right = m_ptMouse.x + nWorldX;
//		m_rtDrag.bottom = m_ptMouse.y + nWorldY;
	}

	//	Drag end
	//	鼠標右鍵結束拖曳,紀錄結束位置
	if(m_buttonAction[DINPUT_RIGHT_BUTTON] == DINPUT_ACTION_RELEASE)
	{
		m_rtDrag.right = m_ptMouse.x + nWorldX;
		m_rtDrag.bottom = m_ptMouse.y + nWorldY;

		if((m_rtDrag.left != m_rtDrag.right) || (m_rtDrag.top != m_rtDrag.bottom))
			m_bLBDrag = TRUE;

	}

	//	Decide if left button been double clicked
	//	判斷是否左鍵雙擊
	if(m_buttonAction[DINPUT_LEFT_BUTTON] == DINPUT_ACTION_RELEASE)
	{
		if(m_bLBClick == TRUE)
		{
			m_dwLBThisTick = GetTickCount();
			if(m_dwLBThisTick - m_dwLBLastTick < m_dwDoubleClickTime)
			{
				if((abs(m_ptMouseLBLast.x - m_ptMouse.x - nWorldX) < 4) &&
					(abs(m_ptMouseLBLast.y - m_ptMouse.y - nWorldY) < 4))
				{
					m_bLBDoubleClick = TRUE;
					//	Add by Li Yong at 07/10/00
					//	Clear mouse left button click state if double click occur
					//	判斷鼠標左鍵發生雙擊後則清單擊狀態
					m_buttonAction[DINPUT_LEFT_BUTTON] = DINPUT_ACTION_NONE;
					//	End add
				}
			}
			m_bLBClick = FALSE;
		}
		else
		{
			m_bLBClick = TRUE;
			m_dwLBThisTick = GetTickCount();
			m_dwLBLastTick = GetTickCount();
			//	Add by Li Yong 08/31/2000
			m_ptMouseLBLast.x = m_ptMouse.x + nWorldX;
			m_ptMouseLBLast.y = m_ptMouse.y + nWorldY;
		}
	}

	//	Decide if right button been double clicked
	//	判斷是否右鍵雙擊
	if(m_buttonAction[DINPUT_RIGHT_BUTTON] == DINPUT_ACTION_RELEASE)
	{
		if(m_bRBClick == TRUE)
		{
			m_dwRBThisTick = GetTickCount();
			if(m_dwRBThisTick - m_dwRBLastTick < m_dwDoubleClickTime)
			{
				if((abs(m_ptMouseRBLast.x - m_ptMouse.x - nWorldX) < 4) &&
					(abs(m_ptMouseRBLast.y - m_ptMouse.y - nWorldY) < 4))
				{
					m_bRBDoubleClick = TRUE;
					//	Add by Li Yong at 07/10/00 
					//	Clear mouse right button click state if double click occur
					//	判斷鼠標右鍵發生雙擊後則清單擊狀態
					m_buttonAction[DINPUT_RIGHT_BUTTON] = DINPUT_ACTION_NONE;
					//	End add
				}
			}
			m_bRBClick = FALSE;
		}
		else
		{
			m_bRBClick = TRUE;
			m_dwRBThisTick = GetTickCount();
			m_dwRBLastTick = GetTickCount();
			m_ptMouseRBLast.x = m_ptMouse.x + nWorldX;
			m_ptMouseRBLast.y = m_ptMouse.y + nWorldY;
		}
	}
}

///////////////////////////////////////////////
//	Decide if iKey is in down state;
//	判斷鍵盤上指定鍵是否處於按下狀態
BOOL CDirectInput::IsKeyDown(BYTE iKey)
{
	return (m_keyState[iKey] & 0x80); 
}

///////////////////////////////////////////////////////////////////////
//	Decide if iKey is doing action of pressing
//	判斷鍵盤上指定鍵是否發生按下的動作(從彈起狀態到按下狀態的轉換)
BOOL CDirectInput::IsKeyPress(BYTE iKey,BOOL bReset/*=FALSE*/)
{
	BOOL bReturn=(m_keyAction[iKey] == DINPUT_ACTION_PRESS);
	if (bReset && bReturn)
	{
		m_keyAction[iKey]=DINPUT_ACTION_NONE;
	}
	return bReturn;
}
/////////////////////////////////////////////////////////
//adde by Rosey
// decide if iKey is doing action of double press
//判斷鍵盤上指定鍵是否發生雙擊動作
BOOL CDirectInput::IsKeyDoublePress(BYTE iKey)
{ 
	return (m_keyAction[iKey] == DINPUT_ACTION_DOUBLE_PRESS);
}
///////////////////////////////////////////////////////////////////////
//	Decide if iKey is doing action of releasing
//	判斷鍵盤上指定鍵是否發生釋放的動作(從按下狀態到彈起狀態的轉換)
BOOL CDirectInput::IsKeyRelease(BYTE iKey)
{
	return (m_keyAction[iKey] == DINPUT_ACTION_RELEASE);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse left button is in down state;
//	判斷鼠標左鍵是否處於按下狀態
BOOL CDirectInput::IsMouseLBDown(void)
{
	return (m_buttonState[DINPUT_LEFT_BUTTON] & 0x80);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse right button is in down state;
//	判斷鼠標右鍵是否處於按下狀態
BOOL CDirectInput::IsMouseRBDown(void)
{
	return (m_buttonState[DINPUT_RIGHT_BUTTON] & 0x80);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse left button is doing action of pressing
//	判斷鼠標左鍵是否發生按下的動作(從彈起狀態到按下狀態的轉換)
BOOL CDirectInput::IsMouseLBPress(void)
{
	return (m_buttonAction[DINPUT_LEFT_BUTTON] == DINPUT_ACTION_PRESS);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse right button is doing action of pressing
//	判斷鼠標右鍵是否發生按下的動作(從彈起狀態到按下狀態的轉換)
BOOL CDirectInput::IsMouseRBPress(void)
{
	return (m_buttonAction[DINPUT_RIGHT_BUTTON] == DINPUT_ACTION_PRESS);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse left button is doing action of releasing
//	判斷鼠標左鍵是否發生釋放的動作(從按下狀態到彈起狀態的轉換)
BOOL CDirectInput::IsMouseLBRelease(void)
{
	return (m_buttonAction[DINPUT_LEFT_BUTTON] == DINPUT_ACTION_RELEASE);
}

///////////////////////////////////////////////////////////////////////
//	Decide if mouse right button is doing action of releasing
//	判斷鼠標右鍵是否發生釋放的動作(從按下狀態到彈起狀態的轉換)
BOOL CDirectInput::IsMouseRBRelease(void)
{
	return (m_buttonAction[DINPUT_RIGHT_BUTTON] == DINPUT_ACTION_RELEASE);
}

//Added by cxi
void CDirectInput::SetMousePos(POINT &pt)
{
	m_ptMouse=pt;
}
/*
* Function:      CheckKey 
* --------------------------------------------------------------------- 
* Purpose:			檢查是否有鍵盤動作,包括KeyDown和KeyUp
* Return:			0:沒有; 1;KeyDown; 2:KeyUp
* External Ref.:	無 
* Summary:			無
* Flowchart:		無 
* --------------------------------------------------------------------- 
* Note:				無
* --------------------------------------------------------------------- 
*/
int CDirectInput::CheckKey(UINT &nChar, UINT &nRepCnt)
{
	if(true)
	{
		if ((m_dwKeyboardCode > 0))
		{
			nChar = m_rgodKeyboard[m_dwKeyboardCode - 1].dwOfs;
			nRepCnt = m_rgodKeyboard[m_dwKeyboardCode - 1].dwSequence;
			if(m_rgodKeyboard[m_dwKeyboardCode - 1].dwData&0x80)
				return 1;
			else
				return 2;
		}
	}
	return 0;
}