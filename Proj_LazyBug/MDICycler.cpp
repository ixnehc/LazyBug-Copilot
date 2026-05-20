/********************************************************************
	created:	2010/7/5   10:17
	file path:	d:\IxEngine\Proj_ProtoEditor\protolibframe
	author:		chenxi
	
	purpose:	mdi 子窗口切换
*********************************************************************/
#include "stdh.h"

#include "MDICycler.h"

#include "commondefines/general_stl.h"
#include "assert.h"


void CMDICycler::_EnumChilds(CWnd *wnd)
{
	CWnd *child=wnd->GetWindow(GW_CHILD);
	while(child)
	{
		if (child->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
			_childs.push_back(child->GetSafeHwnd());
		else
			_EnumChilds(child);
		child=child->GetWindow(GW_HWNDNEXT);
	}
}


void CMDICycler::_UpdateQueue()
{
	if (!_wnd)
		return;

	_childs.clear();
	_EnumChilds(_wnd);

	int i=0;
	while(i<_queue.size())
	{
		HWND h=_queue[i];
		int idx;
		VEC_FIND(_childs,h,idx);
		if (idx==-1)
		{
			_queue.erase(_queue.begin()+i);//这个窗口不存在了
			continue;
		}
		_childs[idx]=_childs.back();
		_childs.pop_back();
		i++;
	}

	VEC_APPEND(_queue,_childs);

	CWnd *wnd=_wnd->MDIGetActive();
	if (wnd)
	{
		int idx;
		VEC_FIND(_queue,wnd->GetSafeHwnd(),idx);
		assert(idx>=0);
		if (idx!=0)
			Swap(_queue[0],_queue[idx]);
	}

// 	if (_swap>=_queue.size())
// 		_swap=_queue.size()-1;

}

void CMDICycler::Next()
{
	_UpdateQueue();
	if (_queue.size()<=1)
		return;
	if (_swap<1)
		return;
	if (_swap>=_queue.size())
	{
		_queue.push_back(_queue[0]);
		_queue.erase(_queue.begin());
		_swap=1;
	}
	else
	{
		Swap(_queue[0],_queue[_swap]);
		_swap++;
	}
	::SendMessage(_wnd->m_hWndMDIClient, WM_MDIACTIVATE,(WPARAM)_queue[0], 0);
}

void CMDICycler::Rest()
{
	_swap=1;
}
