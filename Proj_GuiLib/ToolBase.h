#pragma once

#include "ToolTypes.h"
#include "GuiEditor.h"
#include <set>
#include "class/class.h"
#include "gds/GObj.h"
#include "datapacket/DataPacket.h"

class CToolBase
{
public:
	struct Mode
	{
		std::string name;
		int mode;
		CToolBase * owner;
	};

	struct _ToolPanel:public CXTPDialog
	{
	protected:
		virtual void OnOK(){}
		virtual void OnCancel(){}
	};
	virtual CClass*GetClass()=0;

	CToolBase(void);
	virtual ~CToolBase(void);

	virtual BOOL DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode);
	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView,int priority = AGENTPRIORITY_STANDARD);
	virtual void EndParam(int mode);
	virtual BOOL InitDlg(CWnd * pParent);
	virtual void OnInitDlg(CGeActor * actor);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void RegisterAgent();
	virtual void DisableAll();
	virtual void OnUpdateUI(CGeActor * actor){}

	virtual void RegisterMode(){};
	virtual const char * GetTypeName() = 0;
	virtual DWORD GetType() = 0;
	virtual GObjBase *GetGObj(){	return NULL;}
	
	virtual void InitGObj();
	virtual void DestructGObj();

	virtual void Load(CDataPacket &dp);
	virtual void Save(CDataPacket &dp);

	virtual void Load();
	virtual void Save();

	int NumOfModes();
	Mode * GetMode(int idx);
	int GetActiveMode(){return m_mode;}

	CGuiView * GetView();

	friend class ToolContainer;
	friend LRESULT ToolBase_UserWndProc(HWND hWnd,UINT mesasge,WPARAM wParam,LPARAM lParam);


protected:
	CGeActor * GetActor(){return m_actor;}
	void AddAgent(CGuiAgent & agent,AgentPriority priority=-1);
	void AddMode(const char * name,int mode);
	BOOL DefDialog(CWnd * pParent,DWORD ID);
	void UpdateUI(CGeActor *actor);
	const char * _GetPathName();

	std::vector<Mode> _modes;
	HWND m_hwnd;
	WNDPROC m_oldWndProc;
	int m_mode;
	CGeActor * m_actor;
	_ToolPanel m_panel;
	std::set<CGuiAgent *> _agents;
	int m_level;
	int m_priority;
	std::string m_nameView;
};

class CClassT : public CClass
{
public:
	unsigned long typetl;
};

class ToolFactory
{
public:
	void RegisterClass(CClassT * cls);
	CClassT * GetClass(const char * name);
	CToolBase * CreateInstance(const char * name);
	int GetNumberOfClass(){return _class.size();}
	CClassT * GetClass(int idx){ if(idx<0||idx>=_class.size()) return NULL ; else return _class[idx];}
private:
	std::vector<CClassT *> _class;
};

extern ToolFactory * GetToolFactory();


#define _DECLARE_TOOLCLASS_BEGIN(__c,clss,typeTool,baseclss)				\
		_DEFINE_CLASS_BEGIN(__c,CClassPool,clss,baseclss)			\
		 instance.typetl = typeTool;										\

#define _DECLARE_TOOLCLASS_END(theclass)	\
	return &instance;															\
		}																		\
		static CClass_##theclass *_clss;											\
		virtual CClass *GetClass()												\
	{																			\
		return _clss;															\
	}\
	class dummyclass_##theclass													\
	{																				\
	public:																		\
		dummyclass_##theclass(CClassT * cls)									\
		{																		\
			ToolFactory * factory  = GetToolFactory();						\
			factory->RegisterClass(cls);									\
		}																	\
	};																		\

#define BEGIN_DECLARE_TOOL_CLASS(theclass,typeTool)\
	_DECLARE_TOOLCLASS_BEGIN(CClassT,theclass,typeTool,void)	\
	_DECLARE_TOOLCLASS_END(theclass)											\
	virtual void InitGObj(){GConstructor();}				\
	virtual void DestructGObj(){GDestructor();}			\
	BEGIN_GOBJ(theclass,0)														\

	
#define END_DECLARE_TOOL_CLASS()\
	END_GOBJ();	\

#define DECLARE_TOOL_CLASS(theclass,typeTool)		\
	_DECLARE_TOOLCLASS_BEGIN(CClassT,theclass,typeTool,void)	\
	_DECLARE_TOOLCLASS_END(theclass)\			


#define IMPLEMENT_TOOL_CLASS(theclass)\
		IMPLEMENT_CLASS(theclass)	\
		static theclass::dummyclass_##theclass dummyclass(theclass::_instantiate());	\

									



