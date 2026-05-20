
#pragma  once

#include "GuiEditor.h"

#include "WorldSystem/IEnvLight.h"


class IVertexBuffer;
class IIndexBuffer;
class IMesh;
class IMeshSnapshot;
class IMtrl;

class CGuiAgent_ElDraw:public CGuiAgent
{
public:

	CGuiAgent_ElDraw();
	virtual ~CGuiAgent_ElDraw();
	virtual BOOL OnDraw();
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);

protected:

	void _EnumProbeCube();
	void _AddAbbToDraw(i_math::aabbox3df &abb);
	void _DrawSel();
	void _FillDrawSamp();
	void _DrawSamp(int nPrim);
	void _DrawGrid();
	void _DrawSHMap();
private:
	std::vector<HMapObj> _hObjs;
	IProbeCubeMapEditor * _editor;
	std::vector<i_math::vector3df> _lines;	  // the all wireframe box in the secene.
	IVertexBuffer * _pVb;
	IIndexBuffer * _pIb;
	IMesh * _meshSample;
	IMeshSnapshot * _snapMsh;
	IMtrl * _mtrlSample;
	std::vector<i_math::vector3df> _lineGridFore;
	std::vector<i_math::vector3df> _lineGridBack;
};

