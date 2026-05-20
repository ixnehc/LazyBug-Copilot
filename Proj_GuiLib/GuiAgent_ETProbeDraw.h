
#pragma  once

#include "GuiEditor.h"

#include "RenderSystem/IRenderSystem.h"

class CGuiAgent_ETProbeDraw :public CGuiAgent
{
public:
	CGuiAgent_ETProbeDraw();
	~CGuiAgent_ETProbeDraw();
	virtual BOOL OnDraw();

protected:
	void _EnumNodes();
	BOOL _TouchRes();
	IMesh * _mesh;
	IMtrl * _mtrl;
	ILight *_light;
	std::vector<i_math::vector3df> _lines;
	std::vector<i_math::vector3df> _linesActive;
};