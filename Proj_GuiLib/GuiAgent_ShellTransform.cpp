#include "stdh.h"
#include ".\guiagent_shelltransform.h"
#include "GuiData_proto.h"

CGuiAgent_ShellTransform::CGuiAgent_ShellTransform(void)
{
}
CGuiAgent_ShellTransform::~CGuiAgent_ShellTransform(void)
{
}
BOOL CGuiAgent_ShellTransform::OnLButtonClick(int x,int y,DWORD flag)
{
	return TRUE;
}
void CGuiAgent_ShellTransform::OnAttachView(CGeView *view,DWORD iLevel)
{
	_UpdateTransform(_mat);
}
void CGuiAgent_ShellTransform::_UpdateTransform(i_math::matrix43f &mat)
{
}	
