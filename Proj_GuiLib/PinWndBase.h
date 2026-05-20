#pragma once

#include "GuiLib.h"

#include "WMGuiLib.h"

#include "pin/pin.h"


struct NMHDR_PB :public NMHDR
{
	CPinboard * pinboard;
};

template <class T_wnd>
class CPinboardWndBase : public T_wnd ,public CPinboard
{
public:
	CPinboardWndBase(){_state = EndChange;}
protected:
	virtual void OnNotifyChange(ChangeState state) //数据改变时 被DataLink框架调用
	{
		switch(state){
			case PreChange:
				{
					NotifyMessage(PBN_PRECHANGE);
					_state = PreChange;
					break;
				}
			case OnChange:
				{
					if(_state==EndChange)
						NotifyMessage(PBN_PRECHANGE);
					NotifyMessage(PBN_ONCHANGE);
					_state = OnChange;
					break;
				}
			case EndChange:
				{
					if(_state==EndChange){
						NotifyMessage(PBN_PRECHANGE);
						NotifyMessage(PBN_ONCHANGE);
					}
					else if(state==PreChange){
						NotifyMessage(PBN_ONCHANGE);
					}
					NotifyMessage(PBN_ENDCHANGE);
					_state = EndChange;
					break;
				}
			default:
				assert(false);
				break;
		}
	}
	void  NotifyMessage(DWORD notifyCode)
	{
		CWnd * pOwner = GetOwner();
		if(!pOwner)
			pOwner = GetParent();
		HWND hOwner = pOwner->GetSafeHwnd();

		assert(hOwner);

		DWORD ctrlID = GetDlgCtrlID();
		DWORD wparam = ctrlID;

		static NMHDR_PB infoItem;
		infoItem.hwndFrom = GetSafeHwnd();
		infoItem.idFrom = ctrlID;
		infoItem.pinboard = dynamic_cast<CPinboard *>(this);
		infoItem.code = notifyCode;

		::SendMessage(hOwner,WM_NOTIFY,wparam,LPARAM(&infoItem));	
	}
private:
	ChangeState _state;
};


template <class T_Wnd>
class CPinWndBase : public T_Wnd ,public CPin
{
	
};





