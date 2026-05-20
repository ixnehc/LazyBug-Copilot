
#include "stdh.h"

#include "ShaderLibGlobal.h"
#include "ShaderLibGraph.h"

#include "graphicsgraph.h"

//////////////////////////////////////////////////////////////////////////
//CShaderLibGraph

void CShaderLibGraph::Clear()
{
	Zero();
}

void CShaderLibGraph::Load(CShaderLibGlobal *global,const char *nmLib,GraphicsGraph *gg)
{
	BOOL bNeedReload=FALSE;
	if (global->_ver!=_ver)
		bNeedReload=TRUE;
	if (_nmLib!=nmLib)
		bNeedReload=TRUE;

	if (!bNeedReload)
		return;

	_ver=global->_ver;
	_nmLib=nmLib;

	Clear();

	CShaderLib2 *lib=global->FindLib(_nmLib.c_str());
	if (lib)
	{
		_startVS=0.0f;
		_startPS=lib->_heightVS;
		_startOut=_startPS+lib->_heightPS;
	}
	
}

void CShaderLibGraph::Draw(GraphicsGraph *gg)
{
	i_math::pos2df ptFrom,ptTo;
	ptFrom.x=-4000.0f;
	ptTo.x=4000.0f;

	ptFrom.y=ptTo.y=_startVS;
	gg->DrawConnectLine(ptFrom,ptTo,ColorAlpha(0x000000,0x7f),2.0f,DRAWCONNECT_DASH|DRAWCONNECT_NOCAP);

	ptFrom.y=ptTo.y=_startPS;
	gg->DrawConnectLine(ptFrom,ptTo,ColorAlpha(0x000000,0x7f),2.0f,DRAWCONNECT_DASH|DRAWCONNECT_NOCAP);

	ptFrom.y=ptTo.y=_startOut;
	gg->DrawConnectLine(ptFrom,ptTo,ColorAlpha(0x000000,0x7f),2.0f,DRAWCONNECT_DASH|DRAWCONNECT_NOCAP);


}
