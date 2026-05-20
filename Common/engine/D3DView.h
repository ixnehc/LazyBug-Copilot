
#pragma once

#include "RenderSystem/IFont.h"



template <class BASE_CLASS>
class CD3DView:public BASE_CLASS
{
public:
	CD3DView()
	{
		_pRS=NULL;
		_bOwnDevice=TRUE;
		_rp=NULL;
		_bInit=FALSE;
		_bShowProfile=FALSE;
	}
	void SetRS(IRenderSystem *pRS,BOOL bOwnDevice=FALSE)
	{
		_pRS=pRS;	
		_bOwnDevice=bOwnDevice;
	}

	void ShowProfile(BOOL bShow)
	{
		_bShowProfile=bShow;
	}
	void SetProfileInfo(const char *s)
	{
		_profileinfo=s;
	}

	virtual void Draw()
	{
		if (_pRS)
		{
			IRenderPort *rp=_GetRP();
			if (!rp)
				return;

			_pRS->BeginFrame();

			OnDrawBg(rp);
			OnDraw(rp);

			if (_bShowProfile)
				_DrawProfile(rp);

			_pRS->EndFrame();

			HWND hwndOverride=NULL;
			if (!_bOwnDevice)
				hwndOverride=m_hWnd;
			i_math::recti rc;
			GetPresentRect(*(CRect*)&rc);
			_pRS->PresentAsyn(&rc,&rc,hwndOverride);
		}
	}

	virtual void OnInit3D()	{	}
	virtual void OnClear3D()	{	}

	virtual void OnDraw(IRenderPort *rp)	{	}

	virtual void OnDrawBg(IRenderPort *rp)
	{
		rp->ClearBuffer(ClearBuffer_All,ColorAlpha(0x3f3f3f,0xff));
	}

	virtual void PreResetConfig(DeviceConfig &cfg)	{	}
	virtual BOOL GetPresentRect(CRect &rc)	
	{		
		GetClientRect(&rc);
		return TRUE;	
	}

	virtual void OnDraw(CDC *)	{	}//overriding CView's pure function



protected:

	virtual IRenderPort *_GetRP()
	{
		if (!_rp)
		{
			if (_pRS)
			{
				if (_pRS->IsDeviceReset())
				{
					if (!_bInit)
					{
						OnInit3D();
						_bInit=TRUE;
					}
				}

				if (_bInit)
					_rp=_pRS->CreateRenderPort();
			}
		}

		if (_rp&&(!_bOwnDevice))
		{//Not fully owning the device,we should provide the renderport rect
			CRect rc;
			GetClientRect(&rc);
			_rp->SetRect(0,0,rc.Width(),rc.Height());
		}

		//		if (_rp)
		//			_UpdateCamera(_rp);
		return _rp;
	}


	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_SIZE:
			{
				BASE_CLASS::WindowProc(message, wParam, lParam);
				if (_bOwnDevice)
				{
					if ((LOWORD(lParam)>0)&&(HIWORD(lParam)>0))
						if (_pRS)
						{
							DeviceConfig cfg(m_hWnd);
							PreResetConfig(cfg);
							if (_pRS)
								_pRS->ResetDevice(cfg);

						}
				}
				return 0;
			}

		case WM_PAINT:
			{
				CPaintDC dc(this);
				Draw();
				return 0;
			}

		case WM_DESTROY:
			{
				OnClear3D();
				SAFE_RELEASE(_rp);

				if (_bOwnDevice&&_pRS)
					_pRS->CleanDevice();

				_bInit=FALSE;

				break;
			}
		case WM_ERASEBKGND:
			return 1;


		}

		return BASE_CLASS::WindowProc(message, wParam, lParam);
	}

	void _DrawProfile(IRenderPort *rp)
	{
		i_math::pos2di pt(4,4);
		DrawFontArg arg;
		i_math::size2di sz;
		rp->CalcDrawText(_profileinfo.c_str(),arg,sz);
		i_math::recti rc;
		rc.set(pt,sz);
		rc.inflate(0,0,16,16);

		rp->FillRect(rc,ColorAlpha(0x0,0x7f));
		rp->FrameRect(rc,ColorAlpha(0xffffff,0xff));

		arg.SetLocation(pt.x+8,pt.y+8);
		rp->DrawText(_profileinfo.c_str(),arg);
	}

	IRenderSystem *_pRS;
	IRenderPort *_rp;
	BOOL _bInit;
	BOOL _bOwnDevice;//whether this view has full control of the rendersystem's device,
										//if TRUE,this view should manage the device by itself.
										//if FALSE,this view just uses the device,and need not care about its
										//management

	BOOL _bShowProfile;
	std::string _profileinfo;

};

