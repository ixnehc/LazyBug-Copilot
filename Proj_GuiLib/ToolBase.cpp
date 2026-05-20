#include "stdh.h"
#include ".\toolbase.h"
#include "Registry/Registry.h"
#include "stringparser/stringparser.h"

LRESULT ToolBase_UserWndProc(HWND hWnd,UINT mesasge,WPARAM wParam,LPARAM lParam)
{
#pragma warning(disable:4312)
	CToolBase * pUtil = (CToolBase *)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	CGeActor * actor = NULL;
	if(pUtil)
		actor = pUtil->GetActor();

	if(actor)
	{
		if(pUtil->DlgProc(mesasge,wParam,lParam,actor,pUtil->m_mode))
			return TRUE;

		switch(mesasge)
		{
		case WM_COMMAND:
			{
				DWORD ctrlID = LOWORD(wParam);
				DWORD code = HIWORD(wParam);
				if(TRUE == pUtil->OnCommand(ctrlID,code,lParam,actor))
					return TRUE;
				break;
			}
		case WM_DESTROY:
			{
				pUtil->Save();
				pUtil->DestructGObj();
				break;
			}
		}
	}
	
	return pUtil->m_oldWndProc(hWnd,mesasge,wParam,lParam);
}
//////////////////////////////////////////////////////////////////////////
CToolBase::CToolBase(void)
{
	m_hwnd = NULL;
}

CToolBase::~CToolBase(void)
{
}
void CToolBase::InitGObj()
{
}
void CToolBase::DestructGObj()
{
}
BOOL CToolBase::DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode)
{
	return FALSE;
}
void CToolBase::OnInitDlg(CGeActor * actor)
{
}
BOOL CToolBase::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	return TRUE;
}
void CToolBase::DisableAll()
{
	for(int i = 0;i<_modes.size();i++)
		EndParam(_modes[i].mode);
}
BOOL CToolBase::BeginParam(CWnd * pParent,int mode,CGeActor * pActor,int level,const char * nameView,int priority)
{
	assert(pParent&&pActor);
	assert(mode>=0);
#pragma warning(disable:4244)
#pragma warning(disable:4312)

	m_mode = mode;
	m_actor = pActor;

	m_level = level;
	m_nameView = nameView;
	m_priority = priority;

	if(!m_hwnd)
	{
		if(FALSE == InitDlg(pParent))
			return FALSE;
		if(m_hwnd)
		{
			m_oldWndProc = WNDPROC(GetWindowLongPtr(m_hwnd,GWLP_WNDPROC));
			SetWindowLongPtr(m_hwnd,GWLP_WNDPROC,LONG_PTR(ToolBase_UserWndProc));
			SetWindowLongPtr(m_hwnd,GWLP_USERDATA,LONG_PTR(this));
			
			InitGObj();
			
			Load();

			OnInitDlg(pActor);
		}
	}

	RegisterAgent();

	if(m_hwnd)
	{
		ShowWindow(m_hwnd,SW_SHOW);
		 // clear the dirty areas
		HWND hParent = GetParent(m_hwnd);
		assert(hParent);
		hParent = GetParent(hParent);
		if(hParent)
		{
			InvalidateRect(hParent,NULL,TRUE);
			UpdateWindow(hParent);
		}		
	}

	return TRUE;
}

void CToolBase::AddAgent(CGuiAgent & agent,AgentPriority priority)
{

	std::pair<std::set<CGuiAgent *>::iterator,bool> p;
	p = _agents.insert(&agent);

	CGeView * viewTouch = agent.GetView();
	if(!viewTouch)
	{
		CGuiView * view = GetView();
		assert(view);
		if (priority<0)
			view->AddAgent(m_level,&agent,m_priority);
		else
			view->AddAgent(m_level,&agent,priority);
	}

	agent.AddRef();
	agent.Enable(TRUE);
}
void CToolBase::AddMode(const char * name,int mode)
{
	CToolBase::Mode brmode;
	brmode.name = name;
	brmode.mode = mode;
	brmode.owner = this;
	
	_modes.push_back(brmode);
}
CGuiView * CToolBase::GetView()
{
	CGeActor * actor = GetActor();
	if(actor)
	{
		CGuiView * view=(CGuiView *)actor->FindView(m_nameView.c_str());
		return view;
	}
	return NULL;
}
BOOL CToolBase::InitDlg(CWnd * pParent)
{
	return TRUE;
}
BOOL CToolBase::DefDialog(CWnd * pParent,DWORD ID)
{
	if(FALSE == m_panel.Create(ID,pParent))
		return FALSE;

	m_hwnd = m_panel.GetSafeHwnd();

	return TRUE;
}
void CToolBase::RegisterAgent()
{
}
void CToolBase::EndParam(int mode)
{
	assert(mode>=0);
	if(m_mode==mode)
	{
		m_mode = -1;
		std::set<CGuiAgent *>::iterator it;
		for(it = _agents.begin();it!=_agents.end();it++)
		{
			CGuiAgent * agent = (*it);
			agent->Enable(FALSE);
		}
		if(m_hwnd)
		{
			ShowWindow(m_hwnd,SW_HIDE);
		}
	}
}
int CToolBase::NumOfModes()
{
	return int(_modes.size());
}
CToolBase::Mode * CToolBase::GetMode(int idx)
{
	if(0==_modes.size()||idx<0||idx>=_modes.size())
		return NULL;
	else
		return &_modes[idx];
}
void CToolBase::Load(CDataPacket &dp)
{
	LoadGObj(dp,GetGObj(),NULL);
}
void CToolBase::Save(CDataPacket &dp)
{
	SaveGObj(dp,GetGObj());
}

void CToolBase::Load()
{
	GObjBase * pObj = GetGObj();
	
	std::string name = _GetPathName();
	
	CCurrentUserRegistry reg("IxSoftware","IxEngine");
	
	void * pBuf = NULL;
	DWORD sz = 0;

	if(FALSE == reg.ReadData("GuiToolCfg",name.c_str(),pBuf,sz))
		return ;

	std::vector<unsigned char> buf(sz);
	if(pBuf)
	{
		memcpy(buf.data(),pBuf,sz);
		CDataPacket dp;
		dp.SetDataBufferPointer(buf.data());

		Load(dp);
	}
}
void CToolBase::Save()
{
	CDataPacket dp;
	Save(dp);

	int sz = dp.GetDataSize();
	std::vector<unsigned char> buf(sz);
	dp.SetDataBufferPointer(buf.data());

	Save(dp);

	CCurrentUserRegistry reg("IxSoftware","IxEngine");
	std::string name = _GetPathName();
	reg.WriteData("GuiToolCfg",name.c_str(),buf.data(),sz);

}
const char * CToolBase::_GetPathName()
{
	CClass  * cls = GetClass();
	static std::string namePath;

	namePath = "";
	namePath.append(cls->GetName());
	namePath.append(".tool");

	return namePath.c_str();
}

void CToolBase::UpdateUI(CGeActor *actor)
{
	if(m_hwnd)
		OnUpdateUI(actor);
}
//////////////////////////////////////////////////////////////////////////

ToolFactory *  GetToolFactory()
{
	static ToolFactory  gtoolfactory;
	return &gtoolfactory;
}

void ToolFactory::RegisterClass(CClassT * cls)
{
	_class.push_back(cls);	
}
CClassT * ToolFactory::GetClass(const char * name)
{
	for(int i = 0;i<_class.size();i++)
	{
		CClassT * cls = _class[i];
		assert(cls);
		if(cls->_classname.compare(name)==0)
			return cls;
	}
	return NULL;
}
CToolBase * ToolFactory::CreateInstance(const char * nameClass)
{
	CClassT * cls = GetClass(nameClass);
	if(cls)
		return (CToolBase *)cls->_new();
	return NULL;
}




