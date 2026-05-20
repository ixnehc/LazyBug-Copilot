#pragma once

#include "gds/GStub.h"
#include "gds/GProp.h"

#include "WorldSystem/stubparams/param_ael.h"
#include "WorldSystem/stubparams/stubparams.h"

#include "editor/ctrlop.h"

#include "NotifyArgs.h"
#include "Event.h"

#include "align/rectalign.h"



class CAssetCtrlRenderer;

struct PropAEL;

struct AssetSystemState;
struct ImageCombo;
struct PropImageCombo;
struct PropRef;
class CAssetCtrl
{
public:
	enum eEvents
	{
		EventBegin = 1,

		//! Window size has changed
		EventSized,
		//! Window position has changed
		EventMoved,
		//! Text string for the Window has changed
		EventTextChanged,
		//! Window has been activated (has input focus)
		EventActivated,
		//! Window has been deactivated (loses input focus)
		EventDeactivated,
		// generated externally (inputs)
		//! Mouse cursor was moved within the area of the Window.
		EventMouseMove,
		//! Mouse wheel was scrolled within the Window.
		EventMouseWheel,
		//! A mouse button was pressed down within the Window.
		EventMouseButtonDown,
		//! A mouse button was released within the Window.
		EventMouseButtonUp,
		//! A mouse button was clicked (down then up) within the Window.
		EventMouseClick,
		//! A mouse button was double-clicked within the Window.
		EventMouseDoubleClick,
		//! A key on the keyboard was pressed.
		EventKeyDown,
		//! A key on the keyboard was released.
		EventKeyUp,
		//! A text character was typed on the keyboard.
		EventCharacterKey,
		//! A DragContainer has been dragged over this window.
		EventDragDropItemEnters, 
		//! A DragContainer has left this window.
		EventDragDropItemLeaves, 
		//! A DragContainer was dropped on this Window.
		EventDragDropItemDropped,  

		EventEnd,
	};

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
	virtual CClass *GetClass()=0;
	/*!
	\brief
		Constructor for Window base class
	*/
	CAssetCtrl(void);

	/*!
	\brief
		Destructor for Window base class
	*/
	virtual ~CAssetCtrl(void);

public:
	BOOL IsAlwaysOnTop(void) const  {return _alwaysOnTop;}

	BOOL IsDisabled(void) const;
	BOOL IsVisible(void) const;

	BOOL IsActive(void) const;

	BOOL IsClippedByParent(void) const {return _clippedByParent;}

	BOOL    IsDestroyedByParent(void) const    {return _destroyedByParent;}

	/*!
	\brief
		Return whether this window is set to restore old input capture when it
		loses input capture.

		This is only really useful for certain sub-components for widget
		writers.

	\return
		- true if the window will restore the previous capture window when it
		loses input capture.
		- false if the window will set the capture window to NULL when it loses
		input capture (this is the default behaviour).
	*/
	BOOL RestoresOldCapture(void) const     {return _restoreOldCapture;}

	/*!
	\brief
		Return whether the window wants inputs passed to its attached
		child windows when the window has inputs captured.

	\return
		- true if System should pass captured input events to child windows.
		- false if System should pass captured input events to this window only.
	*/
	BOOL DistributesCapturedInputs(void) const {return _distCapturedInputs;}

	RectAlign GetAlign() const  {return _align;}


	size_t GetChildCount(void) const  {return _children.size();}

	BOOL IsChild(const CAssetCtrl* window) const;

	CAssetCtrl* GetChildAtIdx(size_t idx) const {return _children[idx];}

	CAssetCtrl* GetActiveChild(void);
	const CAssetCtrl* GetActiveChild(void) const;

	//a window is not an ancestor of this window.
	BOOL IsAncestor(const CAssetCtrl* window) const;

	BYTE GetAlpha(void) const  {return _alpha;}

	virtual const ShellRect& GetScreenRect(void);

	virtual const ShellRect& GetClippedScreenRect(void);

	virtual ShellRect GetInnerRect(void);

	virtual ShellRect GetClippedInnerRect(void);

	BOOL IsCapturingMouse(void) const;
	BOOL IsCapturingDrag(void) const;

	virtual BOOL IsHit(const ShellPos& position,FindCtrlReason reason) const;

	//如果after不为NULL,从after后面开始找
	virtual CAssetCtrl* FindChild(const ShellPos& position,FindCtrlReason reason,CAssetCtrl *after=NULL) const;

	CAssetCtrl* GetParent(void) const   {return _parent;}
	IAsset *GetOwnerAsset()	{		return _assetOwner;	}

	BOOL TestClassName(const char* class_name);

	BOOL GetModalState(void) const;

	/*!
	\brief
		Returns the active sibling window.

		This searches the immediate children of this window's parent, and
		returns a pointer to the active window.  The method will return this if
		we are the immediate child of our parent that is active.  If our parent
		is not active, or if no immediate child of our parent is active then 0
		is returned.  If this window has no parent, and this window is not
		active then 0 is returned, else this is returned.

	\return
		A pointer to the immediate child window attached to our parent that is
		currently active, or 0 if no immediate child of our parent is active.
	*/
	CAssetCtrl* GetActiveSibling();

	ShellSize GetParentSize(void) const;

	void SetDestroyedByParent(BOOL setting);

	void SetAlwaysOnTop(BOOL setting);
	void SetEnabled(BOOL setting);
	void Enable(void)   {SetEnabled(TRUE);}
	void Disable(void)  {SetEnabled(FALSE);}
	virtual void SetVisible(BOOL setting);
	void Show(void) {SetVisible(TRUE);}
	void Hide(void) {SetVisible(FALSE);}
	void Activate(void);
	void Deactivate(void);

	void SetClippedByParent(BOOL setting);

	void SetAlignment(const RectAlign alignment);
	RectAlign GetAlignment()	{		return _align;	}

	void SetText(const char* text);

	void AddChildWindow(CAssetCtrl* window);

	void RemoveChildWindow(CAssetCtrl* window);

	void MoveToFront();

	void MoveToBack();

	BOOL CaptureMouse(void);
	void ReleaseMouse(void);
	BOOL CaptureDrag();
	void ReleaseDrag();

	virtual BOOL ShowTip();//返回是否成功的显示了tip
	virtual void HideTip();
	virtual BOOL NeedChangeTip()	{		return FALSE;	}

	/*!
	\brief
		Set whether this window will remember and restore the previous window
		that had inputs captured.

	\param setting
		- true: The window will remember and restore the previous capture
		window.  The CaptureLost event is not fired on the previous window
		when this window steals input capture.  When this window releases
		capture, the old capture window is silently restored.

		- false: Input capture works as normal, each window losing capture is
		signalled via CaptureLost, and upon the final release of capture, no
		previous setting is restored (this is the default behaviour).

	\return
		Nothing
	*/
	void SetRestoreCapture(bool setting);

	/*!
	Set the current alpha value for this window.
	*/
	void SetAlpha(BYTE alpha);

	void SetMouseCursor(const char* image);

	virtual BOOL Create(AssetSystemState *ss, IAsset *owner);

	virtual void Destroy(void);

	void SetModalState(BOOL state);

	void SetRect(const ShellRect& area);

	virtual void SetRect(const ShellPos& pos, const ShellSize& size);

	void SetPosition(const ShellPos& pos);

	void SetLeft(int left);
	void SetTop(int top);
	void SetSize(const ShellSize& size);
	void SetWidth(int width);
	void SetHeight(int height);
	void SetMaxSize(const ShellSize& size);
	void SetMinSize(const ShellSize& size);

	const ShellRect& GetRect() /*const*/;

	const ShellPos& GetPosition()/* const*/;

	int GetLeft(void) /*const*/;
	int GetTop(void) /*const*/;
	int GetWidth() const { return _rect.getWidth(); }
	int GetHeight() const { return _rect.getHeight(); }
	ShellSize GetSize() /*const*/;
	const ShellSize& GetMaxSize() const;
	const ShellSize& GetMinSize() const;

	virtual void Render(void);
	CAssetCtrlRenderer* GetWindowRenderer(void) const;

	void SetParent(CAssetCtrl* parent);	
	void SetWindowRenderer(CAssetCtrlRenderer* renderer);
	virtual void OnClock()	{	}
	virtual void OnCreate(void) {}
	virtual void OnPostCreate(){}
	virtual void OnDestroy(void) {}

public:
	/*!
	\brief
		Coordinate transform between screen and window.
	*/
	ShellPos ScreenToWindow(const ShellPos& pt) /*const*/;
	ShellRect  ScreenToWindow(const ShellRect& rc) /*const*/;
	ShellPos WindowToScreen(const ShellPos& pt) /*const*/;
	ShellRect  WindowToScreen(const ShellRect& rc) /*const*/;

	ShellPos GetScreenPosition(void) /*const*/ {return WindowToScreen(ShellPos(0, 0));}

public:
	void SubscribeEvent(int id, CEvent::Subscriber subscriber);

protected:
	/*************************************************************************
	System object can trigger events directly
	*************************************************************************/
	friend class CAssetShell;
	friend class CAssetCtrlRenderer;

	/*************************************************************************
	Event trigger methods
	*************************************************************************/
	virtual void OnRectChange();
	virtual void OnSized();
	virtual void OnParentSized();
	virtual void OnEnabled();
	virtual void OnDisabled();

	virtual void OnMouseCaptureLost(WindowEventArgs& e);
	virtual void OnDragCaptureLost(WindowEventArgs& e)	{	}//基类里啥都不做
	virtual void OnTipCaptureLost(WindowEventArgs& e);

	virtual void OnDestructionStarted(WindowEventArgs& e);

	virtual void OnActivated(WindowEventArgs& e);
	virtual void OnDeactivated(WindowEventArgs& e);


	virtual void OnChildAdded(WindowEventArgs& e);

	virtual void OnChildRemoved(WindowEventArgs& e);

	virtual void OnMouseEnters(MouseEventArgs& e);

	virtual void OnMouseLeaves(MouseEventArgs& e);

	virtual void OnMouseMove(MouseEventArgs& e);

	virtual void OnMouseWheel(MouseEventArgs& e);

	virtual void OnMouseButtonDown(MouseEventArgs& e);

	virtual void OnMouseButtonUp(MouseEventArgs& e);

	virtual void OnMouseClicked(MouseEventArgs& e);
	virtual void OnMouseDoubleClicked(MouseEventArgs& e);
	virtual void OnKeyDown(KeyEventArgs& e);
	virtual void OnKeyUp(KeyEventArgs& e);
	virtual void OnCharacter(KeyEventArgs& e);


	virtual void DrawSelf(void);

protected:
	virtual BOOL CreateWindowRenderer(IRatomsShell* ratoms);


	virtual void RecalcLayout() {}

	virtual void CleanupChildren(void);

	virtual void AddChild_impl(CAssetCtrl* wnd);

	virtual void RemoveChild_impl(CAssetCtrl* wnd);

	virtual BOOL MoveToFront_impl(BOOL wasClicked);

	void AddWindowToDrawList(CAssetCtrl& wnd, BOOL at_back = FALSE);

	BOOL RemoveWindowFromDrawList(const CAssetCtrl& wnd);

	BOOL IsTopOfZOrder() const;

	/*************************************************************************
		May not copy or assign Window objects
	*************************************************************************/
	CAssetCtrl(const CAssetCtrl& wnd) {}
	CAssetCtrl& operator=(const CAssetCtrl& wnd) {return *this;}

public:
	void	AddEvent(int id);

	void	RemoveEvent(int id);

	void	RemoveAllEvents(void);

	BOOL	IsEventPresent(int id);

	void FireEvent(int id, WindowEventArgs& args);

	CEvent* GetEventObject(int id, BOOL autoAdd = FALSE);

	void SetAel(NodeAEL&ael);

	void LuaDebugOutput(const char *type,const char *content,...);

	void RegisterClock();
	void UnRegisterClock();

	BOOL IsPostCreated();

	void NotifyRectChanged(BOOL bSized);
	void EnsureLocalValid();
protected:

	BOOL _DockAndShowTip(ShellRect &rc);//rc是屏幕空间的rect


	// child stuff
	typedef std::vector<CAssetCtrl*> ChildList;
	//! The list of child Window objects attached to this.
	ChildList _children;

	//! Child window objects arranged in rendering order.
	ChildList _drawList;

	CAssetCtrl* _parent;

	//! Holds the render image for this Window.
	ImageCombo *_comboImage;

	//! Alpha transparency setting for the Window

//	//! This Window objects area as defined by a Rect.
	ShellRect _rect;

	ShellSize _minSize;
	ShellSize _maxSize;

	// positional alignments
	//! Specifies the base for horizontal alignment.
	RectAlign _align:8;
	RectAlign _alignTip:8;

	DWORD _enabled:1;
	DWORD  _visible:1;
	//! TRUE when Window is the active Window (receiving inputs).
	DWORD _active:1;
	//! TRUE when Window will be clipped by parent Window area Rect.
	DWORD _clippedByParent:1;
	//! TRUE when Window will be auto-destroyed by parent.
	DWORD _destroyedByParent:1;
	//! TRUE if Window will be drawn on top of all other Windows
	DWORD _alwaysOnTop:1;
	// true if the Window restores capture to the previous window when it
	DWORD _restoreOldCapture:1;
	// true if unhandled captured inputs should be distributed to child
	DWORD _distCapturedInputs:1;
	//! TRUE when this window is being destroyed.
	DWORD _destructionStarted:1;
	DWORD _screenClippedRectValid:1;
	DWORD _screenRectValid:1;

	BYTE _alpha;

	//! The Window that previously had capture (used for restoreOldCapture mode)
	CAssetCtrl* _oldCapture;

	//! Possible custom Tooltip for this window.
	//CTooltip* _customTip;

	//// rendering
	////! TRUE if window image cache needs to be regenerated.
	//BOOL _needsRedraw;

	//! The WindowRenderer module that implements the Look'N'Feel specification
	CAssetCtrlRenderer* _windowRenderer;


	//! current unclipped screen rect in pixels
	mutable ShellRect _screenClippedRect;
	//! current fully clipped screen rect in pixels
	mutable ShellRect _screenRect;

	typedef std::map<int, CEvent*>	EventMap;
	EventMap	_events;

	NodeAEL _aelTip;

public:
	IAsset* _assetOwner;	//! it's owner
	AssetSystemState* _ss;
	IAssetShell* _assetShell;
	NodeAEL _ael;			
	
public:
	virtual BOOL GetPos(i_math::vector3df &pos)	{		return FALSE;	}
	virtual BOOL GetXForm(i_math::matrix43f&mat)	{		return FALSE;	}
	virtual BOOL SetPos(i_math::vector3df &pos){		return FALSE;	}
	virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f *matLink){		return FALSE;	}
	virtual BOOL GetBaseXform(AnimTick t,i_math::matrix43f &mat)	{		return FALSE;	}

	GStubBegin(CAssetCtrl)

		// Properties
		GPropInt(Enabled, 1, GSem_Boolean, "Enabled")
		GStubInt(Visible, 1, GSem_Boolean, "Visible")	
		GPropInt(Align, 0, GSem(GSem_Interger,"LeftUp,Up,RightUp,Left,Center,Right,LeftDown,Down,RightDown"), "Alignment")
		GPropFloat(Alpha, 1.0f, GSem_Alpha, "Alpha")
		GPropInt(TopMost, 0, GSem_Boolean, "AlwaysOnTop")
		GPropInt4(Rect, i_math::vector4di(0, 0, 80, 24), GSem(GSem_Rect,"ShowSize"), "Rect")
		GPropInt(ParentClip, 1, GSem_Boolean, "是否可以被父控件裁剪")

		//signal
		GSignalDefine(ShowTip,StbParams);
			GPropSetDesc("显示Tool tip")

		// Methods
		GStubInt(SetModalState, 0, GSem_Interger, "Do modal")
			GStubSetType(GStub_Slot)
		GStubDefine( Enable,StbVoid);
			GPropSetDesc( "Enable" );
			GStubSetType( GStub_Slot );
		GStubDefine( Disable,StbVoid);
			GPropSetDesc( "Disable" );
			GStubSetType( GStub_Slot );
		GCallDefine(GetScreenRect, Prop_Void,Prop_Sx4)
			GPropSetDesc("得到控件在屏幕上的绝对位置")
		GCallDefine(SetTip, StbParams,Prop_Void)
			GPropSetDesc("设置Tool tip")

	GStubEnd()



	BOOL prop_Align(BOOL bSet, int& align);
	BOOL prop_Alpha(BOOL bSet, float& alpha);
	BOOL prop_TopMost(BOOL bSet, int& setting);
	BOOL prop_Enabled(BOOL bSet, int& setting);
	//BOOL prop_MouseCursorImage(BOOL bSet, const char*& image);	
	BOOL prop_Image(BOOL bSet, PropRef*&combo);
	virtual BOOL prop_Rect(BOOL bSet, i_math::vector4di& area);
	BOOL prop_Visible(BOOL bSet, int& setting);
	BOOL prop_ParentClip(BOOL bSet, int& setting);

	BOOL prop_SetModalState(BOOL bSet, int& setting);
	BOOL prop_Enable( BOOL bSet ,StbVoid*&prop);
	BOOL prop_Disable( BOOL bSet ,StbVoid*&prop);
	BOOL call_GetScreenRect(Prop_Void *, Prop_Sx4* &rcAbs);
	BOOL call_SetTip(StbParams*param, Prop_Void *&);

};



#define CtrlStubTrigger(name)																								\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubTrigger(name);																											\
	GStackPop();																														\
}

#define CtrlStubFire(name,data)																							\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFire(name,data);																										\
	GStackPop();																														\
}


#define  CtrlStubFireVoid(name)																									\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireVoid(name);																								\
	GStackPop();																														\
}

#define CtrlStubFireString(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireString(name,value);																							\
	GStackPop();																														\
}

#define CtrlStubFireInt(name,value)																						\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireInt(name,value);																									\
	GStackPop();																														\
}

#define CtrlStubFireDword(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireDword(name,value);																							\
	GStackPop();																														\
}

#define CtrlStubFireFloat(name,value)																					\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireFloat(name,value);																								\
	GStackPop();																														\
}

#define CtrlStubFireVector2(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireVector2(name,value);																						\
	GStackPop();																														\
}

#define CtrlStubFireVector3(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireVector3(name,value);																						\
	GStackPop();																														\
}

#define CtrlStubFireVector4(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireVector4(name,value);																						\
	GStackPop();																														\
}


#define CtrlStubFireMat43(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireMat43(name,value);																							\
	GStackPop();																														\
}

#define CtrlStubFireMat44(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireMat44(name,value);																							\
	GStackPop();																														\
}

#define CtrlStubFireInt4(name,value)																					\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireInt4(name,value);																								\
	GStackPop();																														\
}

#define CtrlStubFireInt2(name,value)																					\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireInt2(name,value);																								\
	GStackPop();																														\
}

#define CtrlStubFireByte4(name,value)																				\
{																																				\
	GStackPush_General(#name,_assetOwner,_assetOwner?_assetOwner->GetClass():NULL);																	\
	GStubFireByte4(name,value);																							\
	GStackPop();																														\
}
