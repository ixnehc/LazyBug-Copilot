#include "GuiLib.h"
#include <map>
#include "math/imath_all.h"
class GuiLib_Api CScroolWnd :public CWnd
{
	CScroolWnd()
	{

	}
	~CScroolWnd()
	{

	}
public :
	BOOL AddControl(CWnd * pControl);
	BOOL SetControlRect(CWnd * pControl ,DWORD height,DWORD width);
	BOOL Create(RECT & rc,CWnd *pParent,UINT templateId,DWORD wsStyle=WS_CHILD|WS_VISIBLE);
protected:
// 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnPaint(CDC* pDC);

	DECLARE_MESSAGE_MAP();
protected:
	std::map<CWnd *,i_math::vector2du> _controls;
	typedef  std::map<CWnd *,i_math::vector2du>::iterator  control_iterator ;
};

