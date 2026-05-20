#pragma once
#include "GuiLib.h"

#include "editor/editor.h"
#include "GuiEditor.h"

#include "shaderlib2/ShaderLib2.h"
#include "ShaderLibGraph.h"

#include "GuiAgent_2DTransform.h"


class CShaderLibGlobal;
struct GuiLib_Api GuiData_ShaderLib:public GeData
{
	virtual const char *GetName()	{		return "shaderlib";	}


	GuiData_ShaderLib()
	{
		global=NULL;
	}

	~GuiData_ShaderLib()
	{
	}

	CShaderLibGlobal *GetGlobal()	{		return global;	}
	void SetGlobal(CShaderLibGlobal *global_)	{		global=global_;	}

	CShaderLibGlobal *global;

	CShaderLibGraph graph;

	std::string nmLib;

};

class GuiLib_Api CGuiView_ShaderLib:public CGuiView
{
public:
	virtual const char*GetName()	{		return "shaderlib";	}

protected:
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;	}

	void _OnDraw(GraphicsGraph *gg);
};

class CGuiAgent_ShaderLibGraphScroll:public CGuiAgent_2DTransform
{
public:
	virtual void OnUpdateTransform(const i_math::pos2df & pos,const i_math::pos2df &scale){}

protected:
};

class CGuiAgent_ShaderCommand:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:

	i_math::pos2di _ptStart;
};


class GuiLib_Api CGuiActor_ShaderLib: public CGuiActor
{
public:
	virtual const char*GetName()	{		return "shaderlib";	}

	virtual void Reset();

};

