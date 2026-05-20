#pragma once

#include "AssetCtrlPrerequisites.h"

#include "../../IAssetRendererDefines.h"

struct AssetSystemState;
class IAssetShell;
class CMouseCursor 
{
public:
	/*!
	\brief
		Constructor for MouseCursor objects
	*/
	CMouseCursor(AssetSystemState *ss, IAssetShell* shell);

	/*!
	\brief
		Destructor for MouseCursor objects
	*/
	~CMouseCursor(void);

public:
	/*!
	\brief
		Set the current mouse cursor image
	*/
	void SetImage(const char* image_name);

	/*!
	\brief
		Get the current mouse cursor image
	\return
		The current image used to draw mouse cursor.
	*/
	const char* GetImage(void) const	{return _cursorImage.c_str();}

	/*!
	\brief
		Makes the cursor draw itself

	\return
		Nothing
	*/
	void Draw(void);

	/*!
	\brief
		Set the current mouse cursor position

	\param position
		Point object describing the new location for the mouse.  This will be clipped to within the renderer screen area.
	*/
	void SetPosition(const ShellPos& position);

	/*!
	\brief
		Offset the mouse cursor position by the deltas specified in \a offset.

	\param offset
		Point object which describes the amount to move the cursor in each axis.

	\return
		Nothing.
	*/
	void OffsetPosition(const ShellPos& offset);

	/*!
	\brief
		Set the area that the mouse cursor is constrained to.

	\param area
		Pointer to a Rect object that describes the area of the display that the mouse is allowed to occupy.  The given area will be clipped to
		the current Renderer screen area - it is never possible for the mouse to leave this area.  If this parameter is NULL, the
		constraint is set to the size of the current Renderer screen area.

	\return
		Nothing.
	*/
	void SetConstraintArea(const ShellRect& area);

	/*!
	\brief
		Hides the mouse cursor.

	\return
		Nothing.
	*/
	void Hide(void)	{_visible = FALSE;}

	/*!
	\brief
		Shows the mouse cursor.

	\return
		Nothing.
	*/
	void Show(void)	{_visible = TRUE;}

	/*!
	\brief
		Set the visibility of the mouse cursor.

	\param visible
		'TRUE' to show the mouse cursor, 'FALSE' to hide it.

	\return
		Nothing.
	*/
	void SetVisible(BOOL visible) {_visible = visible;}

	/*!
	\brief
		return whether the mouse cursor is visible.

	\return
		TRUE if the mouse cursor is visible, FALSE if the mouse cursor is hidden.
	*/
	BOOL IsVisible(void) const {return _visible;}

	/*!
	\brief
		Return the current mouse cursor position as a pixel offset from the top-left corner of the display.

	\return
		ShellPos object describing the mouse cursor position in screen pixels.
	*/
	ShellPos GetPosition(void) const	{return _position;}

	/*!
	\brief
		return the current constraint area of the mouse cursor.

	\return
		ShellRect object describing the active area that the mouse cursor is constrained to.
	*/
	ShellRect GetConstraintArea(void) const;

private:
	/*************************************************************************
		Implementation Methods
	*************************************************************************/
	/*!
	\brief
		Checks the mouse cursor position is within the current 'constrain' Rect and adjusts as required.
	*/
	void	ConstrainPosition(void);

private:
	std::string	 _cursorImage;	//!< Image that is currently set as the mouse cursor.
	ShellPos	 _position;		//!< Current location of the cursor
	BOOL	 _visible;		//!< TRUE if the cursor will be drawn, else FALSE.
	ShellRect	 _constraints;	//!< Specifies the area (in screen pixels) that the mouse can move around in.

private:
	/*!
	\brief
		Update the RatomID location & clipped rect.
	*/
	void _Update();

	AssetSystemState* _ss;
	RatomID	_image;
};
