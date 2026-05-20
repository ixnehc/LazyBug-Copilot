/********************************************************************
	created:	2008/04/14
	created:	14:4:2008   15:33
	filename: 	f:\project test\ui\src\window.cpp
	file path:	f:\project test\ui\src
	file base:	window
	file ext:	cpp
	author:		szg
	
	purpose:	base class, source code from CEGUI(www.cegui.org.uk)
*********************************************************************/
#include "stdh.h"

#include "../../IAssetShell.h"
#include "../../IAssetEventer.h"
#include "AssetCtrl.h"
#include "AssetCtrlRenderer.h"
#include "MouseCursor.h"
#include <algorithm>

#include "editor/ctrlop.h"

#include "../../stubparams/param_ael.h"
#include "../../stubparams/param_sys.h"

#include "timer/profiler.h"

#include "align/rectalign.h"

#include "ShellImage.h"

//LogFile g_logAssetCtrl("assetctrl");


/************************************************************************
	Constructor
*************************************************************************/
CAssetCtrl::CAssetCtrl(void) : 
	_minSize(SZ_MINSIZE, SZ_MINSIZE), 
	_maxSize(SZ_MAXSIZE, SZ_MAXSIZE), 
	_rect(0, 0, 80, 24),
	_assetOwner(0),
	_assetShell(0), 
	_screenClippedRect(0, 0, 0, 0), 
	_screenRect(0, 0, 0, 0)
{
	// basic set-up
	_parent		= 0;
	_alpha		= 255;
	//_needsRedraw   = TRUE;

	// basic settings
	_enabled			= TRUE;
	_visible			= TRUE;
	_active				= FALSE;
	_clippedByParent	= TRUE;
	_destroyedByParent	= FALSE;
	_alwaysOnTop		= FALSE;
	_restoreOldCapture	= FALSE;
	_oldCapture			= NULL;
	_distCapturedInputs	= FALSE;

	_comboImage=NULL;

	// initialisation flags
	//_initialising = FALSE;
	_destructionStarted = FALSE;

	// set initial alignments
	_align= ALIGN_LEFTUP;

	_screenClippedRectValid = FALSE;
	_screenRectValid = FALSE;

	// WindowRenderer
	_windowRenderer = 0;

	_alignTip=ALIGN_DOWN;

}

/*************************************************************************
	Destructor
*************************************************************************/
CAssetCtrl::~CAssetCtrl(void)
{
	// cleanup events actually happened earlier.
	SAFE_RELEASE(_comboImage);
}

/*************************************************************************
	return TRUE if the Window is currently disabled.
*************************************************************************/
BOOL CAssetCtrl::IsDisabled(void) const
{
	BOOL parDisabled = (_parent == 0) ? FALSE : _parent->IsDisabled();

	return (!_enabled) || parDisabled;
}

/*************************************************************************
	return TRUE if the Window is currently visible.
*************************************************************************/
BOOL CAssetCtrl::IsVisible(void) const
{
	BOOL parVisible = (_parent == 0) ? TRUE : _parent->IsVisible();

	return _visible && parVisible;
}

/*************************************************************************
	return TRUE if this is the active Window
	(the window that receives inputs)
*************************************************************************/
BOOL CAssetCtrl::IsActive(void) const
{
	BOOL parActive = (_parent == 0) ? TRUE : _parent->IsActive();

	return _active && parActive;
}

BOOL CAssetCtrl::IsChild(const CAssetCtrl* window) const
{
	size_t child_count = GetChildCount();

	for (size_t i = 0; i < child_count; ++i)
	{
		if (_children[i] == window)
			return TRUE;
	}

	return FALSE;
}

CAssetCtrl* CAssetCtrl::GetActiveChild(void)
{
	return const_cast<CAssetCtrl*>(static_cast<const CAssetCtrl*>(this)->GetActiveChild());
}

const CAssetCtrl* CAssetCtrl::GetActiveChild(void) const
{
	// are children can't be active if we are not
	if (!IsActive())
	{
		return 0;
	}

	size_t pos = _drawList.size();

	while (pos-- > 0)
	{
		// don't need full backward scan for activeness as we already know 'this' is active
		// NB: This uses the draw-ordered child list, as that should be quicker in most cases.
		if (_drawList[pos]->_active)
			return _drawList[pos]->GetActiveChild();
	}

	// no child was active, therefore we are the topmost active window
	return this;
}

BOOL CAssetCtrl::IsAncestor(const CAssetCtrl* window) const
{
	// if we have no parent, then return FALSE
	if (!_parent)
		return FALSE;

	// check our immediate parent
	if (_parent == window)
		return TRUE;

	// not our parent, check back up the family line
    return _parent->IsAncestor(window);
}

const ShellRect& CAssetCtrl::GetScreenRect(void)
{
	// The screen rect is lastest
	if (_screenRectValid)
		return _screenRect;

	_screenRectValid = TRUE;
	if (_parent == NULL)
		return (_screenRect = _rect);

	// Calculate the screen rect
	ShellRect &rcParent= (ShellRect &)_parent->GetScreenRect();

	ShellRect rc=RelativeAreaToLocalRect<short>(rcParent,_rect,_align);
	_screenRect = rc+ rcParent.UpperLeftCorner;

	return _screenRect;
}

ShellRect CAssetCtrl::GetInnerRect(void)
{
	return GetScreenRect();
}

const ShellRect& CAssetCtrl::GetClippedScreenRect()
{
	if (!_screenClippedRectValid)
	{
		_screenClippedRect = GetScreenRect();
		ShellRect clipped_area = _screenClippedRect;
		if (_parent && _clippedByParent)
			clipped_area = _parent->GetClippedScreenRect();
		_screenClippedRect.clipAgainst(clipped_area);
		_screenClippedRectValid = TRUE;
	}	
	return _screenClippedRect;
}

ShellRect CAssetCtrl::GetClippedInnerRect()
{
	return GetClippedScreenRect();
}

BOOL CAssetCtrl::IsCapturingMouse(void) const
{
	return (_assetShell->GetMouseCapture() == this);
}

BOOL CAssetCtrl::IsCapturingDrag(void) const
{
	return (_assetShell->GetDragCapture()==this);
}



BOOL CAssetCtrl::IsHit(const ShellPos& position,FindCtrlReason reason) const
{
	ShellRect clipped_area = ((CAssetCtrl*)this)->GetClippedScreenRect();

	return clipped_area.isPointInside(position);
}


CAssetCtrl* CAssetCtrl::FindChild(const ShellPos& position,FindCtrlReason reason,CAssetCtrl *after) const
{
	ChildList::const_reverse_iterator   child, end;

	end = _drawList.rend();

	BOOL bAfter=FALSE;
	if (!after)
		bAfter=TRUE;
	for (child = _drawList.rbegin(); child != end; ++child)
	{
		if (after)
		{
			if ((*child)==after)
			{
				bAfter=TRUE;
				continue;
			}
		}
		if (!bAfter)
			continue;
		if ((*child)->IsVisible())
		{
			// recursively scan children of this child windows...
			CAssetCtrl* wnd = (*child)->FindChild(position,reason);

			// return window pointer if we found a 'hit' down the chain somewhere
			if (wnd)
				return wnd;
			// see if this child is hit and return it's pointer if it is
			else if ((*child)->IsHit(position,reason))
				return (*child);
		}
	}

	// nothing hit
	return 0;
}

void CAssetCtrl::SetAlwaysOnTop(BOOL setting)
{
	// only react to an actual change
	if (IsAlwaysOnTop() != setting)
	{
		_alwaysOnTop = setting;

		// move us in front of sibling windows with the same 'always-on-top' setting as we have.
		if (_parent)
		{
			CAssetCtrl* org_parent = _parent;

			org_parent->RemoveChild_impl(this);
			org_parent->AddChild_impl(this);
		}

		// we no longer want a total redraw here, instead we just get each window
		// to resubmit it's imagery to the Renderer.
	}

}

void CAssetCtrl::SetEnabled(BOOL setting)
{
	// only react if setting has changed
	if (_enabled != setting)
	{
		_enabled = setting;

		if (_enabled)
		{
			// check to see if the window is actually enabled (which depends upon all ancestor windows being enabled)
			// we do this so that events we fire give an accurate indication of the state of a window.
			if ((_parent && !_parent->IsDisabled()) || !_parent)
				OnEnabled();
		}
		else
		{
			OnDisabled();
		}
	}
}

void CAssetCtrl::SetVisible(BOOL setting)
{
	// only react if setting has changed
	if (_visible != setting)
	{
		_visible = setting;

	}
}

void CAssetCtrl::Activate(void)
{
	// force complete release of input capture.
	// NB: This is not done via releaseCapture() because that has
	// different behaviour depending on the restoreOldCapture setting.
	CAssetCtrl* captureWindow = _assetShell->GetMouseCapture();
	if ((captureWindow != 0) && (captureWindow != this))
	{
		CAssetCtrl* tmpCapture = captureWindow;
		_assetShell->SetMouseCapture(NULL);

		WindowEventArgs args(0);
		tmpCapture->OnMouseCaptureLost(args);
	}

	MoveToFront();
}

void CAssetCtrl::Deactivate(void)
{
	WindowEventArgs args(this);
	OnDeactivated(args);
}

/*************************************************************************
	Set whether this Window will be clipped by its parent window(s).
*************************************************************************/
void CAssetCtrl::SetClippedByParent(BOOL setting)
{
	// only react if setting has changed
	if (_clippedByParent != setting)
	{
		_clippedByParent = setting;

		OnRectChange();
	}

}

/*************************************************************************
	Set whether or not this Window will automatically be destroyed when
	its parent Window is destroyed.
*************************************************************************/
void CAssetCtrl::SetDestroyedByParent(BOOL setting)
{
	if (_destroyedByParent != setting)
	{
		_destroyedByParent = setting;

		//WindowEventArgs args(this);
		//OnParentDestroyChanged(args);
	}

}

void CAssetCtrl::SetAlignment(const RectAlign alignment)
{
	if (_align!= alignment)
	{
		_align= alignment;

		OnRectChange();
	}
}


void CAssetCtrl::AddChildWindow(CAssetCtrl* window)
{
	// dont add ourselves as a child
	// and dont add null windows
	if (window == this || window == 0 || IsChild(window))
	{
		return;
	}
	
	AddChild_impl(window);
	WindowEventArgs args(window);
	OnChildAdded(args);
}

void CAssetCtrl::RemoveChildWindow(CAssetCtrl* window)
{
	RemoveChild_impl(window);
	WindowEventArgs args(window);
	OnChildRemoved(args);
}

void CAssetCtrl::MoveToFront()
{
	MoveToFront_impl(FALSE);
}

/*************************************************************************
	Add given window to child list at an appropriate position
*************************************************************************/
void CAssetCtrl::AddChild_impl(CAssetCtrl* wnd)
{
	// if window is already attached, detach it first (will fire normal events)
	if (wnd->GetParent())
		wnd->GetParent()->RemoveChildWindow(wnd);

	AddWindowToDrawList(*wnd);

	// add window to child list
	_children.push_back(wnd);

	// set the parent window
	wnd->_parent = this;

	wnd->OnRectChange();
}


/*************************************************************************
	Remove given window from child list
*************************************************************************/
void CAssetCtrl::RemoveChild_impl(CAssetCtrl* wnd)
{
	// remove from draw list
	RemoveWindowFromDrawList(*wnd);

	// if window has children
	if (!_children.empty())
	{
		// find this window in the child list
		ChildList::iterator	position = std::find(_children.begin(), _children.end(), wnd);

		// if the window was found in the child list
		if (position != _children.end())
		{
			// remove window from child list
			_children.erase(position);
			// reset windows parent so it's no longer this window.
			wnd->_parent = 0;
		}
	}
}

BOOL CAssetCtrl::MoveToFront_impl(BOOL wasClicked)
{
	BOOL took_action = FALSE;

	// if the window has no parent then we can have no siblings
	if (!_parent)
	{
		// perform initial activation if required.
		if (!IsActive())
		{
			took_action = TRUE;
			WindowEventArgs args(this);
			OnActivated(args);
		}

		return took_action;
	}

	// bring parent window to front of it's siblings
	took_action = _parent->MoveToFront_impl(wasClicked);

	// get immediate child of parent that is currently active (if any)
	CAssetCtrl* activeWnd = GetActiveSibling();

	// if a change in active window has occurred
	if (activeWnd != this)
	{
		took_action = TRUE;

		// notify ourselves that we have become active
		WindowEventArgs args(this);
		OnActivated(args);

		// notify any previously active window that it is no longer active
		if (activeWnd)
		{
			activeWnd->OnDeactivated(args);
		}
	}

	// bring us to the front of our siblings
	if (!IsTopOfZOrder())
	{
		took_action = TRUE;

		// remove us from our parent's draw list
		if (_parent->RemoveWindowFromDrawList(*this))
		{
			// re-attach ourselves to our parent's draw list which will move us in front of
			// sibling windows with the same 'always-on-top' setting as we have.
			_parent->AddWindowToDrawList(*this);
		}

		// we no longer want a total redraw here, instead we just get each window
		// to resubmit it's imagery to the Renderer.
	}

	return took_action;
}

void CAssetCtrl::MoveToBack()
{
	// if the window is active, de-activate it.
	if (IsActive())
	{
		WindowEventArgs args(this);
		OnDeactivated(args);
	}

	// we only need to proceed if we have a parent (otherwise we have no siblings)
	if (_parent)
	{
		// remove us from our parent's draw list
		_parent->RemoveWindowFromDrawList(*this);
		// re-attach ourselves to our parent's draw list which will move us in behind
		// sibling windows with the same 'always-on-top' setting as we have.
		_parent->AddWindowToDrawList(*this, TRUE);
		_parent->MoveToBack();
	}
}

BOOL CAssetCtrl::CaptureMouse(void)
{
	// we can only capture if we are the active window (LEAVE THIS ALONE!)
	//Why!!!??? --cxi
// 	if (!IsActive())+
// 		return FALSE;

	if (_assetShell->GetMouseCapture() != this)
	{
		CAssetCtrl* current_capture = _assetShell->GetMouseCapture();
		_assetShell->SetMouseCapture(this);
		WindowEventArgs args(this);

		// inform any window which previously had capture that it doesn't anymore!
		if ((current_capture != 0) && (current_capture != this) && (!_restoreOldCapture))
			current_capture->OnMouseCaptureLost(args);

		if (_restoreOldCapture)
			_oldCapture = current_capture;
	}

	return TRUE;
}

void CAssetCtrl::ReleaseMouse(void)
{
	// if we are not the window that has capture, do nothing
	if (!IsCapturingMouse()) {
		return;
	}

	// restore old captured window if that mode is set
	if (_restoreOldCapture) {
		_assetShell->SetMouseCapture(_oldCapture);

		// check for case when there was no previously captured window
		if (_oldCapture)
		{
			_oldCapture = 0;
			_assetShell->GetMouseCapture()->MoveToFront();
		}

	}
	else
	{
		_assetShell->SetMouseCapture(NULL);
	}

	WindowEventArgs args(this);
	OnMouseCaptureLost(args);
}


BOOL CAssetCtrl::CaptureDrag()
{
	if (_assetShell->GetDragCapture() != this)
	{
		CAssetCtrl* old= _assetShell->GetDragCapture();
		_assetShell->SetDragCapture(this);
		WindowEventArgs args(this);
		if (old)
			old->OnDragCaptureLost(args);
	}

	return TRUE;

}

void CAssetCtrl::ReleaseDrag()
{
	// if we are not the window that has capture, do nothing
	if (!IsCapturingDrag())
		return;
	_assetShell->SetDragCapture(NULL);

	WindowEventArgs args(this);
	OnDragCaptureLost(args);
}

BOOL CAssetCtrl::ShowTip()
{
	if (TRUE)
	{
		PropAEL t;
		t.v.Set(_assetOwner,PN_Asset);

		StbParams params;
		params.SetOwnObj(TRUE);
		params.AddObj(&t);
		CtrlStubFire(ShowTip,params);
		t.v.Clear();
		params.Clear();
	}

	ShellRect rc=GetScreenRect();
	if (_DockAndShowTip(rc))
		return TRUE;
	return FALSE;
}

void CAssetCtrl::HideTip()
{
	CAssetCtrl *ctrl=_aelTip.GetCtrl();
	if (ctrl)
		ctrl->SetVisible(0);
	_assetShell->ClearTip(this);
}



/*************************************************************************
	Set whether this window will remember and restore the previous window
	that had inputs captured.
*************************************************************************/
void CAssetCtrl::SetRestoreCapture(bool setting)
{
	_restoreOldCapture = setting;

	size_t child_count = GetChildCount();

	for (size_t i = 0; i < child_count; ++i)
	{
		_children[i]->SetRestoreCapture(setting);
	}

}

void CAssetCtrl::SetAlpha(BYTE alpha)
{
	_alpha = alpha;
}



void CAssetCtrl::Render(void)
{
	// don't do anything if window is not visible
	if (!IsVisible())
		return;

	// signal rendering started
	// perform drawing for 'this' Window
	DrawSelf();

	// render any child windows
	size_t child_count = _drawList.size();

	for (size_t i = 0; i < child_count; ++i)
	{
		_drawList[i]->Render();
	}

	// signal rendering ended
}

void CAssetCtrl::DrawSelf()
{
	// get derived class or WindowRenderer module to re-populate cache.
	if (_windowRenderer != 0)
	{
		ShellRect clipper = GetClippedScreenRect();
		if (clipper.isValid())
			_windowRenderer->DoRender();
	}
}

void CAssetCtrl::SetParent(CAssetCtrl* parent)
{
	if (parent && (_parent != parent))
	{
		parent->AddChildWindow(this);
	}
}

void CAssetCtrl::SetWindowRenderer(CAssetCtrlRenderer* renderer)
{
	if (renderer != _windowRenderer)
	{
		// Destroy the old instance
		if (_windowRenderer)
		{
			Class_Delete(_windowRenderer);
			_windowRenderer = NULL;
		}

		// Set the new renderer
		_windowRenderer = renderer;
		if (_windowRenderer)
		{
			// Update
			_windowRenderer->OnTextChanged();
			_windowRenderer->OnRectChanged();			
			_windowRenderer->OnImageChanged();
		}
	}
}

BOOL CAssetCtrl::CreateWindowRenderer(IRatomsShell* ratoms)
{
	//if (!_windowRenderer)
	//{
	//	_windowRenderer = Class_New(CGUISheetRenderer);
	//	if (_windowRenderer)
	//	{
	//		_windowRenderer->Create(ratoms, this);
	//		return TRUE;
	//	}
	//}	
	return TRUE;
}

void CAssetCtrl::CleanupChildren(void)
{
	while(GetChildCount() != 0)
	{
		CAssetCtrl* wnd = _children[0];

		// always remove child
		RemoveChildWindow(wnd);


		// release the instance
		if (wnd->IsDestroyedByParent())
		{
			wnd->Destroy();
			Class_Delete(wnd);
		}
	}

}

BOOL CAssetCtrl::Create(AssetSystemState *ss, IAsset *owner)
{
	// If the '_assetShell' member variable is not be NULL, the function returns 'FALSE'.
	if (_assetShell || !ss)
		return FALSE;

	if (!CreateWindowRenderer(ss->ratomsShell))
		return FALSE;

	_assetOwner = owner;

	_ss = ss;
	_assetShell = ss->shell;

	OnCreate();

	// just for test.
	if (owner && (NULL == owner->GetParent()))
	{
		_assetShell->GetGUISheet()->AddChildWindow(this);
	}


	return TRUE;
}

void CAssetCtrl::Destroy(void)
{
	// remove all events
	RemoveAllEvents();

	// release the renderer instance
	if (_windowRenderer)
	{
		Class_Delete(_windowRenderer);
		_windowRenderer = NULL;
	}

	OnDestroy();


	ReleaseMouse();
	ReleaseDrag();

	_assetShell->ClearTip(this);
	_assetShell->NotifyWindowDestroyed(this);


	//// let go of the tooltip if we have it
	//Tooltip* tip = getTooltip();
	//if (tip && tip->getTargetWindow()==this)
	//	tip->setTargetWindow(0);

	// signal our imminent destruction
	WindowEventArgs args(this);
	OnDestructionStarted(args);

	// double check we are detached from parent
	if (_parent)
	{
		_parent->RemoveChildWindow(this);
	}

	CleanupChildren();

	//和Asset断开
	if (_assetOwner)
		_assetOwner->ClearCtrl();
	_assetOwner = 0;

	//如果有_ael的话,DeferedDestroy它
	if (!_ael.IsEmpty())
	{
		_ael.DeferredDestroy();
		_ael.Clear();
	}

	if (!_aelTip.IsEmpty())
	{
		_aelTip.DeferredDestroy();
		_aelTip.Clear();
	}
}

CAssetCtrl* CAssetCtrl::GetActiveSibling()
{
	// initialise with this if we are active, else 0
	CAssetCtrl* activeWnd = IsActive() ? this : 0;

	// if active window not already known, and we have a parent window
	if (!activeWnd && _parent)
	{
		// scan backwards through the draw list, as this will
		// usually result in the fastest result.
		size_t idx = _parent->_drawList.size();
		while (idx-- > 0)
		{
			// if this child is active
			if (_parent->_drawList[idx]->IsActive())
			{
				// set the return value
				activeWnd = _parent->_drawList[idx];
				// exit loop early, as we have found what we needed
				break;
			}
		}
	}

	// return whatever we discovered
	return activeWnd;
}

ShellSize CAssetCtrl::GetParentSize(void) const
{
	ShellSize sz(0, 0);
	if (_parent)
		sz = _parent->GetSize();
	return sz;
}

void CAssetCtrl::SetRect(const ShellRect& area)
{
	SetRect(area.UpperLeftCorner, ShellSize(area.getWidth(), area.getHeight()));
}

void CAssetCtrl::SetRect(const ShellPos& pos, const ShellSize& size)
{
	ShellSize sz(size);
	if (sz.w < _minSize.w)
		sz.w = _minSize.w;
	if (sz.h < _minSize.h)
		sz.h = _minSize.h;
	if (sz.w > _maxSize.w)
		sz.w = _maxSize.w;
	if (sz.h > _maxSize.h)
		sz.h = _maxSize.h;

	BOOL moved = (pos != _rect.UpperLeftCorner);
	BOOL sized = ((sz.w != _rect.getWidth()) || (sz.h != _rect.getHeight()));

	if ((!moved)&&(!sized))
		return;// no change

	_rect.UpperLeftCorner=pos;
	_rect.LowerRightCorner = _rect.UpperLeftCorner + ShellPos(sz.w, sz.h);

	NotifyRectChanged(sized);

}

void CAssetCtrl::SetPosition(const ShellPos& pos)
{
	SetRect(pos, ShellSize(_rect.getWidth(), _rect.getHeight()));
}

void CAssetCtrl::SetLeft(int left)
{
	SetRect(ShellPos(left, _rect.Top()), ShellSize(_rect.getWidth(), _rect.getHeight()));
}

void CAssetCtrl::SetTop(int top)
{
	SetRect(ShellPos(_rect.Left(), top), ShellSize(_rect.getWidth(), _rect.getHeight()));
}

void CAssetCtrl::SetSize(const ShellSize& size)
{
	SetRect(_rect.UpperLeftCorner, size);
}

void CAssetCtrl::SetWidth(int width)
{
	SetRect(_rect.UpperLeftCorner, ShellSize(width, _rect.getHeight()));
}

void CAssetCtrl::SetHeight(int height)
{
	SetRect(_rect.UpperLeftCorner, ShellSize(_rect.getWidth(), height));
}

void CAssetCtrl::SetMaxSize(const i_math::size2d_sh& size)
{
	_maxSize = size;

	// set window area back on itself to cause new maximum size to be applied if required.
	SetRect(_rect.UpperLeftCorner, ShellSize(_rect.getWidth(), _rect.getHeight()));
}

void CAssetCtrl::SetMinSize(const i_math::size2d_sh& size)
{
	_minSize = size;

	// set window area back on itself to cause new minimum size to be applied if required.
	SetRect(_rect.UpperLeftCorner, ShellSize(_rect.getWidth(), _rect.getHeight()));
}

const ShellRect& CAssetCtrl::GetRect() /*const*/
{
	return _rect;
}

const ShellPos& CAssetCtrl::GetPosition() /*const*/
{
	return _rect.UpperLeftCorner;
}

int CAssetCtrl::GetLeft() /*const*/
{
	return _rect.UpperLeftCorner.x;
}

int CAssetCtrl::GetTop() /*const*/
{
	return _rect.UpperLeftCorner.y;
}

ShellSize CAssetCtrl::GetSize() /*const*/
{
	return ShellSize(_rect.getWidth(), _rect.getHeight());
}

const i_math::size2d_sh& CAssetCtrl::GetMaxSize() const
{
	return _maxSize;
}

const i_math::size2d_sh& CAssetCtrl::GetMinSize() const
{
	return _minSize;
}

BOOL CAssetCtrl::GetModalState(void) const
{
	return (_assetShell->GetModalTarget() == this);
}

void CAssetCtrl::SetModalState(BOOL state)
{
	BOOL already_modal = GetModalState();

	// do nothing is state is'nt changing
	if (state != already_modal)
	{
		// if going modal
		if (state)
		{
			Activate();
			_assetShell->SetModalTarget(this);
		}
		// clear the modal target
		else
		{
			_assetShell->SetModalTarget(0);
		}
	}
}

void CAssetCtrl::AddWindowToDrawList(CAssetCtrl& wnd, BOOL at_back)
{
	// add behind other windows in same group
	if (at_back)
	{
		// calculate position where window should be added for drawing
		ChildList::iterator pos = _drawList.begin();
		if (wnd.IsAlwaysOnTop())
		{
			// find first topmost window
			while ((pos != _drawList.end()) && (!(*pos)->IsAlwaysOnTop()))
				++pos;
		}
		// add window to draw list
		_drawList.insert(pos, &wnd);
	}
	// add in front of other windows in group
	else
	{
		// calculate position where window should be added for drawing
		ChildList::reverse_iterator	position = _drawList.rbegin();
		if (!wnd.IsAlwaysOnTop())
		{
			// find last non-topmost window
			while ((position != _drawList.rend()) && ((*position)->IsAlwaysOnTop()))
				++position;
		}
		// add window to draw list
		_drawList.insert(position.base(), &wnd);
	}
}

BOOL CAssetCtrl::RemoveWindowFromDrawList(const CAssetCtrl& wnd)
{
	// if draw list is not empty
	if (!_drawList.empty())
	{
		// attempt to find the window in the draw list
		ChildList::iterator	position = std::find(_drawList.begin(), _drawList.end(), &wnd);

		// remove the window if it was found in the draw list
		if (position != _drawList.end())
		{
			_drawList.erase(position);
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
/*************************************************************************

Begin event triggers section

*************************************************************************/
//////////////////////////////////////////////////////////////////////////

void CAssetCtrl::OnRectChange()
{
	_screenClippedRectValid = FALSE;
	_screenRectValid = FALSE;

	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
		_children[i]->OnRectChange();

	if (_windowRenderer)
		_windowRenderer->OnRectChanged();
}


void CAssetCtrl::OnSized()
{
	// inform children their parent has been re-sized
	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
		_children[i]->OnParentSized();
	RecalcLayout();
}

void CAssetCtrl::OnParentSized()
{
	// For renderer: inform children their parent has been re-moved
	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
	{
		_children[i]->OnParentSized();	// to perform child window layout
	}
}

void CAssetCtrl::OnEnabled()
{
	// signal all non-disabled children that they are now enabled (via inherited state)
	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
	{
		if (_children[i]->_enabled)
		{
			_children[i]->OnEnabled();
		}
	}
}

void CAssetCtrl::OnDisabled()
{
	// signal all non-disabled children that they are now disabled (via inherited state)
	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
	{
		if (_children[i]->_enabled)
			_children[i]->OnDisabled();
	}

}

void CAssetCtrl::OnMouseCaptureLost(WindowEventArgs& e)
{
	// handle restore of previous capture window as required.
	if (_restoreOldCapture && (_oldCapture != 0)) {
		_oldCapture->OnMouseCaptureLost(e);
		_oldCapture = 0;
	}

	assert(_assetShell);
	if (!_assetShell)
		return;

	// handle case where mouse is now in a different window
	// (this is a bit of a hack that uses the mouse input injector to handle this for us).
	//_assetShell->InjectMouseMove(0, 0);
	ShellPos pos= _assetShell->GetCursorPos();

	CtrlOp op;
	op.op = CtrlOp::Op_Move;
	op.x = pos.x;
	op.y = pos.y;
	_assetShell->Respond(op);

}

void CAssetCtrl::OnTipCaptureLost(WindowEventArgs& e)
{

}


void CAssetCtrl::OnDestructionStarted(WindowEventArgs& e)
{
	_destructionStarted = TRUE;
}

void CAssetCtrl::OnActivated(WindowEventArgs& e)
{
	_active = TRUE;
	FireEvent(EventActivated, e);
}


void CAssetCtrl::OnDeactivated(WindowEventArgs& e)
{
	// first de-activate all children
	size_t child_count = GetChildCount();
	for (size_t i = 0; i < child_count; ++i)
	{
		if (_children[i]->IsActive())
		{
			// make sure the child gets itself as the .window member
			WindowEventArgs args(_children[i]);
			_children[i]->OnDeactivated(args);
		}

	}

	_active = FALSE;
	FireEvent(EventDeactivated, e);
}


void CAssetCtrl::OnChildAdded(WindowEventArgs& e)
{
	// we no longer want a total redraw here, instead we just get each window
	// to resubmit it's imagery to the Renderer.
	//FireEvent(EventChildAdded, e);
}

void CAssetCtrl::OnChildRemoved(WindowEventArgs& e)
{
	// we no longer want a total redraw here, instead we just get each window
	// to resubmit it's imagery to the Renderer.
	//FireEvent(EventChildRemoved, e);
}

void CAssetCtrl::OnMouseEnters(MouseEventArgs& e)
{
}


void CAssetCtrl::OnMouseLeaves(MouseEventArgs& e)
{
}


void CAssetCtrl::OnMouseMove(MouseEventArgs& e)
{

	FireEvent(EventMouseMove, e);
}


void CAssetCtrl::OnMouseWheel(MouseEventArgs& e)
{
	FireEvent(EventMouseWheel, e);
}


void CAssetCtrl::OnMouseButtonDown(MouseEventArgs& e)
{

	FireEvent(EventMouseButtonDown, e);
}


void CAssetCtrl::OnMouseButtonUp(MouseEventArgs& e)
{
	FireEvent(EventMouseButtonUp, e);
}


void CAssetCtrl::OnMouseClicked(MouseEventArgs& e)
{
	FireEvent(EventMouseClick, e);
}


void CAssetCtrl::OnMouseDoubleClicked(MouseEventArgs& e)
{
	FireEvent(EventMouseDoubleClick, e);
}

void CAssetCtrl::OnKeyDown(KeyEventArgs& e)
{
	FireEvent(EventKeyDown, e);
}

void CAssetCtrl::OnKeyUp(KeyEventArgs& e)
{
	FireEvent(EventKeyUp, e);
}

void CAssetCtrl::OnCharacter(KeyEventArgs& e)
{
	FireEvent(EventCharacterKey, e);
}


CAssetCtrlRenderer* CAssetCtrl::GetWindowRenderer(void) const
{
	return _windowRenderer;
}



BOOL CAssetCtrl::IsTopOfZOrder() const
{
	// if not attached, then always on top!
	if (!_parent)
		return TRUE;

	// get position of window at top of z-order in same group as this window
	ChildList::reverse_iterator pos = _parent->_drawList.rbegin();
	if (!_alwaysOnTop)
	{
		// find last non-topmost window
		while ((pos != _parent->_drawList.rend()) && (*pos)->IsAlwaysOnTop())
			++pos;
	}

	// return whether the window at the top of the z order is us
	return *pos == this;
}

ShellPos CAssetCtrl::ScreenToWindow(const ShellPos& pt) /*const*/
{
	return pt-GetScreenRect().UpperLeftCorner;
}

ShellRect  CAssetCtrl::ScreenToWindow(const ShellRect& rc)/* const*/
{
	return rc-GetScreenRect().UpperLeftCorner;
}

ShellPos CAssetCtrl::WindowToScreen(const ShellPos& pt) /*const*/
{
	return pt+GetScreenRect().UpperLeftCorner;
}

ShellRect  CAssetCtrl::WindowToScreen(const ShellRect& rc) /*const*/
{
	return rc+GetScreenRect().UpperLeftCorner;
}

void CAssetCtrl::AddEvent(int id)
{
	if (!IsEventPresent(id))
	{
		CEvent *e=Class_New2(CEvent);
		e->SetID(id);
		_events[id] =e;
	}

}

void CAssetCtrl::RemoveEvent(int id)
{
	EventMap::iterator pos = _events.find(id);

	if (pos != _events.end())
	{
		Class_Delete(pos->second);
		_events.erase(pos);
	}

}

void CAssetCtrl::RemoveAllEvents(void)
{
	EventMap::iterator pos = _events.begin();
	EventMap::iterator end = _events.end()	;

	for (; pos != end; ++pos)
	{
		Class_Delete(pos->second);
	}

	_events.clear();
}

BOOL CAssetCtrl::IsEventPresent(int id)
{
	return (_events.find(id) != _events.end());
}

void CAssetCtrl::SubscribeEvent(int id, CEvent::Subscriber subscriber)
{
	// do subscription & return connection
	GetEventObject(id, TRUE)->Subscribe(subscriber);
}

void CAssetCtrl::FireEvent(int id, WindowEventArgs& args)
{
	// handle local event
	// find event object
	CEvent* ev = GetEventObject(id);

	// fire the event if present and set is not muted
	if (ev != 0)
		(*ev)(args);
}

CEvent* CAssetCtrl::GetEventObject(int id, BOOL autoAdd)
{
	EventMap::iterator pos = _events.find(id);

	// if event did not exist, add it and then find it.
	if (pos == _events.end())
	{
		if (autoAdd)
		{
			AddEvent(id);
			return _events.find(id)->second;
		}
		else
		{
			return 0;
		}
	}

	return pos->second;
}

BOOL CAssetCtrl::TestClassName(const char* class_name)
{
	BOOL bPassed = FALSE;
	CClass* cls = GetClass();
	while (cls)
	{
		if (0 == strcmp(cls->GetName(), class_name))
		{
			bPassed = TRUE;
			break;
		}
		cls = cls->GetBase();
	}
	return bPassed;
}

void CAssetCtrl::SetAel(NodeAEL &ael)
{
	assert(_parent);
	assert(_assetOwner);

	if (!_ael.Equal(ael))
		_ael.CopyFrom(ael);
}


BOOL CAssetCtrl::prop_Align(BOOL bSet, int& align)
{
	if (bSet)
	{
		SetAlignment((RectAlign) align);
	}
	else
		align = _align;

	return TRUE;
}


BOOL CAssetCtrl::prop_Alpha(BOOL bSet, float& alpha)
{
	if (bSet)
		SetAlpha((BYTE)i_math::clamp_f(alpha*255.0f,0,255));
	else
		alpha = (((float)GetAlpha())/255.0f);
	return TRUE;
}

BOOL CAssetCtrl::prop_TopMost(BOOL bSet, int& setting)
{
	if (bSet)
		SetAlwaysOnTop(setting);
	else
		setting = IsAlwaysOnTop();
	return TRUE;
}

BOOL CAssetCtrl::prop_Enabled(BOOL bSet, int& setting)
{
	if (bSet)
		SetEnabled(setting);
	else
		setting = !IsDisabled();
	return TRUE;
}

BOOL CAssetCtrl::prop_Image(BOOL bSet, PropRef*&combo)
{
	if (bSet)
	{
		SAFE_REPLACE(_comboImage,combo->stuff);
		if (_windowRenderer)
			_windowRenderer->OnImageChanged();
	}
	return TRUE;
}

void CAssetCtrl::NotifyRectChanged(BOOL bSized)
{
	OnRectChange();
	if (bSized)
		OnSized();	
}

BOOL CAssetCtrl::prop_Rect(BOOL bSet, i_math::vector4di& area)
{
	if (bSet)
	{
		_rect.set(area.x,area.y,area.z,area.w);
		NotifyRectChanged(TRUE);
	}
	else
	{	
		const ShellRect& rc = GetRect();
		area.x = rc.UpperLeftCorner.x;
		area.y = rc.UpperLeftCorner.y;
		area.z = rc.LowerRightCorner.x;
		area.w = rc.LowerRightCorner.y;
	}
	return TRUE;
}

BOOL CAssetCtrl::prop_Visible(BOOL bSet, int& setting)
{
	if (bSet)
		SetVisible(setting);
	else
		setting = IsVisible();
	return TRUE;
}

BOOL CAssetCtrl::prop_ParentClip(BOOL bSet, int& setting)
{
	if (bSet)
		SetClippedByParent(setting);
	else
		setting=_clippedByParent;
	return TRUE;
}


BOOL CAssetCtrl::prop_SetModalState(BOOL bSet, int& setting)
{
	SetModalState(setting);
	return TRUE;
}

BOOL CAssetCtrl::prop_Enable( BOOL bSet ,StbVoid*&prop)
{
	SetEnabled(TRUE);
	return TRUE;
}

BOOL CAssetCtrl::prop_Disable( BOOL bSet ,StbVoid*&prop)
{
	SetEnabled(FALSE);
	return TRUE;
}



BOOL CAssetCtrl::call_GetScreenRect(Prop_Void *, Prop_Sx4* &rcAbs)
{
	ShellRect &rc=(ShellRect &)GetScreenRect();
	rcAbs->v.set(rc.Left(),rc.Top(),rc.Right(),rc.Bottom());
	return TRUE;
}

BOOL CAssetCtrl::call_SetTip(StbParams* params,Prop_Void *&)
{
	PropAEL *p=StbParams_GetObj(params,0,PropAEL);
	if (!p)
	{
		HideTip();
		_aelTip.DeferredDestroy();
		_aelTip.Clear();
	}
	else
	{
		if (_aelTip.Equal(p->v))
			return TRUE;//没有发生变化,不需要更新什么
		_aelTip.DeferredDestroy();
		_aelTip.CopyFrom(p->v);

		if (!_aelTip.IsEmpty())
		{
			CAssetCtrl *ctrl=_aelTip.GetCtrl();
			if (!ctrl)
				LuaDebugOutput("Warning","传入的Tip不是一个界面控件");
			if (ctrl)
			{
				ctrl->SetAlwaysOnTop(TRUE);
				_assetShell->GetGUISheet()->AddChildWindow(ctrl);
				ctrl->SetVisible(FALSE);
			}
		}

		const char *align=params->GetString(1);
		_alignTip=ALIGN_DOWN;
		if ((align[0]=='L')||(align[0]=='l'))
			_alignTip=ALIGN_LEFT;
		if ((align[0]=='R')||(align[0]=='r'))
			_alignTip=ALIGN_RIGHT;
		if ((align[0]=='U')||(align[0]=='u'))
			_alignTip=ALIGN_UP;
		if ((align[0]=='D')||(align[0]=='d'))
			_alignTip=ALIGN_DOWN;
	}

	return TRUE;
}


void CAssetCtrl::LuaDebugOutput(const char *type,const char *content,...)
{
	if (_ss)
	if (_ss->dlgtDebugOutput)
	{
		static char buf[2048];

		va_list args;
		va_start(args,content);
		_vsnprintf(buf,sizeof(buf),content,args);
		va_end(args);

		_ss->dlgtDebugOutput(type,buf);
	}

}

void CAssetCtrl::RegisterClock()
{
	_ss->eventer->RegisterClock(_assetOwner);
}
void CAssetCtrl::UnRegisterClock()
{
	_ss->eventer->UnRegisterClock(_assetOwner);
}

BOOL CAssetCtrl::IsPostCreated()
{
	if (_assetOwner)
		return _assetOwner->TestBit(AssetBit_PostCreated);
	return FALSE;
}


BOOL CAssetCtrl::_DockAndShowTip(ShellRect &rc)
{
	CAssetCtrl *ctrl=_aelTip.GetCtrl();
	if (ctrl)
	{
		ctrl->SetVisible(1);
		ctrl->SetAlignment(ALIGN_LEFTUP);

		ShellPos pt=_assetShell->GetCursorPos();

		i_math::size2di szScrn=_assetShell->GetScreenSize();

		ShellSize szTip=ctrl->GetSize();

		ShellRect rcScrn;
		rcScrn.set(0,0,szScrn.w,szScrn.h);
		ctrl->SetRect(DockRect(rc,szTip,pt,_alignTip,rcScrn));


		return TRUE;
	}
	return FALSE;

}
