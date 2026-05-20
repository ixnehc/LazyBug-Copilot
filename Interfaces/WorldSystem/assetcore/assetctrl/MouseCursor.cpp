#include "stdh.h"
#include "MouseCursor.h"
#include "AssetCtrl.h"

#include "../../IAssetShell.h"
#include "../../IAssetRenderer.h"


CMouseCursor::CMouseCursor(AssetSystemState *ss, IAssetShell* shell) : 
	_ss(ss), 
	_position(0, 0), 
	_visible(TRUE), 
	_image(0),
	_constraints(0, 0, 1, 1)
{
}

CMouseCursor::~CMouseCursor(void)
{
	// For renderer:
	if ((_ss)&&(_image))
	{
		_ss->ratomsShell->UnRegister(_image);
	}
}

void CMouseCursor::SetImage(const char* image_name)
{
	_cursorImage = image_name;

	// For renderer:
	if (_ss)
		_ss->ratomsShell->UpdateImage(_image, image_name);
}

void CMouseCursor::Draw(void)
{
	if (_visible)
	{
		// For renderer: draw the cursor
		if (_ss)
		{
			// If not exist
			if (_image == 0)
				_image = _ss->ratomsShell->Register(ShellRatom_Image);

			// Draw
			//_ss->ratomsShell->DrawRatom(_image);
		}
	}
}

void CMouseCursor::SetPosition(const ShellPos& position)
{
	_position = position;
//	ConstrainPosition();

	_Update();
}

void CMouseCursor::OffsetPosition(const ShellPos& offset)
{
	_position += offset;

	_Update();
}

void CMouseCursor::ConstrainPosition(void)
{
	ShellRect absarea(GetConstraintArea());

	if (_position.x >= absarea.Right())
		_position.x = absarea.Right() -1;

	if (_position.y >= absarea.Bottom())
		_position.y = absarea.Bottom() -1;

	if (_position.y < absarea.Top())
		_position.y = absarea.Top();

	if (_position.x < absarea.Left())
		_position.x = absarea.Left();
}

void CMouseCursor::SetConstraintArea(const ShellRect& area)
{
	if (_ss)
	{
		const ShellRect& renderer_area = _ss->shell->GetGUISheet()->GetRect();
		_constraints = area;
		_constraints.clipAgainst(area);

		ConstrainPosition();
	}
}

/*************************************************************************
Set the area that the mouse cursor is constrained to.
*************************************************************************/
ShellRect CMouseCursor::GetConstraintArea(void) const
{
	return _constraints;
}

void CMouseCursor::_Update()
{
	// For renderer:
	if (_ss)
	{
		CAssetCtrl* root = _ss->shell->GetGUISheet();
		ShellRect rcScr = root->GetScreenRect();

		_ss->ratomsShell->UpdateLoc(_image, (ShellRect&)rcScr);
		_ss->ratomsShell->UpdateClip(_image, (ShellRect&)_constraints);
	}
}