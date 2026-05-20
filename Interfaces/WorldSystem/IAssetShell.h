/********************************************************************
	created:	1:3:2009   13:45
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	defines & interfaces for Asset Shell
*********************************************************************/
#pragma once



typedef i_math::rect_sh ShellRect;
typedef i_math::pos2d_sh ShellPos;
typedef i_math::size2d_sh ShellSize;




//////////////////////////////////////////////////////////////////////////
//for Ragent Shell

//ShellImage string format:
//texturepath,left,top,right,bottom			-- use "," as seperator

#define ParseShellImageStr(str,pathTex,rc)													\
{																														\
	(rc).set(0,0,0,0);																							\
	(pathTex)="";																								\
	std::vector<std::string>__buf;																	\
	std::string __s=(str);																					\
	SplitStringBy(",",__s,&__buf);																		\
	if (__buf.size()>0)																						\
	{																													\
		(pathTex)=__buf[0];																				\
		short *p=(short*)&(rc);																			\
		for (int i=1;i<__buf.size();i++)																\
		p[i-1]=(short)IntFromString(__buf[i].c_str());											\
	}																													\
}

#define ComposeShellImageStr(str,pathTex,rc)											\
	FormatString(str,"%s,%d,%d,%d,%d",(pathTex).c_str(),							\
	(rc).Left(),(rc).Top(),(rc).Right(),(rc).Bottom());

#define SHELL_STANDARD_WIDTH 1600
#define SHELL_STANDARD_HEIGHT 1024


class CAssetCtrl;
class CMouseCursor;

struct CtrlOp;
class IAsset;
class IAssetShell
{
public:
	// Render the GUI
	virtual void CollectDraw(void) = 0;

	// Return a pointer to the active GUI sheet (root) control.
	virtual CAssetCtrl* GetGUISheet(void) const = 0;

	virtual BOOL TransToScreenPos(i_math::vector3df &pos,ShellPos &posScrn) =0;

	virtual IAsset * HitTest(ShellPos &pos) =0;

	virtual const char *GetCursorName(void) = 0;
	virtual ShellPos GetCursorPos(void) = 0;

	virtual void SetCursorName(const char *name)=0;

	// Return the currently set default mouse cursor image
	virtual const char* GetDefaultMouseCursor(void) const = 0;

	// Set the image to be used as the default mouse cursor.
	virtual void SetDefaultMouseCursor(const char* image) = 0;

	// Method to directly set the current modal target.
	virtual void SetModalTarget(CAssetCtrl* target) = 0;

	// Return a pointer to the Window that is currently the modal target.
	virtual CAssetCtrl* GetModalTarget(void) const = 0;

	//Mouse Capture Ïà¹Ø
	virtual void SetMouseCapture(CAssetCtrl* target) = 0;
	virtual CAssetCtrl* GetMouseCapture(void) const = 0;
	virtual void SetMouseCaptureByBelow()=0;
	virtual BOOL IsMouseCaptureByBelow()=0;

	virtual void SetDragCapture(CAssetCtrl* target)=0;
	virtual CAssetCtrl *GetDragCapture()=0;

	virtual void ClearTip(CAssetCtrl *ctrl)=0;

	// Set the screen size, actually set the active sheet size
	virtual void SetScreenSize(const i_math::size2di& size) = 0;

	// Get the screen size, actually get the active sheet size
	virtual i_math::size2di GetScreenSize(void) const = 0;

	virtual void SetRenderPortSize(i_math::size2di& size)= 0;
	virtual i_math::size2di GetRenderPortSize()=0;

	virtual BOOL IsKeyDown(unsigned char vk)=0;

	// Method that injects all input events into the system
	virtual BOOL Respond(const CtrlOp& co) = 0;

	//whenever a window is destroyed, so that System can perform any required housekeeping.
	virtual void NotifyWindowDestroyed(const CAssetCtrl* window) = 0;

};
