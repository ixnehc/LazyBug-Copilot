
#include "stdh.h"

#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderPort.h"


#include "GuiAgent_ETProbeDraw.h"

#include "GuiData_ETProbe.h"

#define ETProbe_ShowRange	400

#include <algorithm>

CGuiAgent_ETProbeDraw::CGuiAgent_ETProbeDraw()
{
	_mesh = NULL;
	_mtrl = NULL;
	_light = NULL;
}

CGuiAgent_ETProbeDraw::~CGuiAgent_ETProbeDraw()
{
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_mtrl);
	SAFE_RELEASE(_light);
}

BOOL CGuiAgent_ETProbeDraw::OnDraw()
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(!editor)
		return TRUE;
	
	//枚举节点
	_EnumNodes();
	
	//加载绘制资源
	if(!_TouchRes())
		return TRUE;
	
	//设置光源
	IRenderPort * rp = GetRP();
	ICamera * cam = rp->GetCamera();
	i_math::vector3df eyeDir;
	cam->GetEyeDir(eyeDir);
	_light->SetDirLight(eyeDir,0,0xffffffff,0xff555555);
	
	//绘制节点
	_lines.clear();
	_linesActive.clear();
	for(int i = 0;i<data->nodes.size();i++){
		HMapObj &hObj = data->nodes[i];
		
		//是否选中被选的
		std::vector<HMapObj>::iterator it;
		it = std::find(data->hObjSels.begin(),data->hObjSels.end(),hObj);
		BOOL bSel = (it!=data->hObjSels.end());
		DWORD col = (bSel)?0xffffff00:0xff00ffff;

		const ETProbeInfo * info = editor->GetETProbeInfo(hObj);
		if(info){
			i_math::matrix43f mat;
			mat.setScale(0.5f,0.5f,0.5f);
			mat.addTranslation(info->pos);
			rp->SimpleDrawMesh(_mesh,mat,col,FALSE,_mtrl,_light);
			
			i_math::vector3df vec0(info->pos.x,info->pos.y-0.5f,info->pos.z);
			i_math::vector3df vec1(info->pos.x,-9999.0f,info->pos.z);
			if(bSel){
				_linesActive.push_back(vec0);
				_linesActive.push_back(vec1);
			}
			else{
				_lines.push_back(vec0);
				_lines.push_back(vec1);
			}
		}
	}
	
	if(!_lines.empty())
		rp->Lines(_lines.data(),_lines.size()/2,0xff00ffff);
	
	if(!_linesActive.empty())
		rp->Lines(_linesActive.data(),_linesActive.size()/2,0xffffff00);

	return TRUE;
}

void CGuiAgent_ETProbeDraw::_EnumNodes()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	IETProbeEditor * editor = NULL;
	if(data){
		data->nodes.clear();
		editor = data->GetEditor();
	}

	if(editor){
		IRenderPort * rp = GetRP();
		i_math::vector3df posEye;
		ICamera * cam = rp->GetCamera();
		cam->GetEyePos(posEye);
		DWORD count = 0;
		HMapObj * hObjs = editor->Enum(posEye,ETProbe_ShowRange,count);
		data->nodes.resize(count);
		memcpy(&data->nodes[0],hObjs,count*sizeof(HMapObj));
	}
}

BOOL CGuiAgent_ETProbeDraw::_TouchRes()
{
	IRenderPort * rp  = GetRP();
	assert(rp);

	IRenderSystem * pRS = rp->GetRS();

	if(!_mesh){
		_mesh = (IMesh *)pRS->GetMeshMgr()->ObtainRes("_editor\\sphere.msh");
	}
	if(!_mtrl){
		_mtrl = (IMtrl *)pRS->GetMtrlMgr()->ObtainRes("_editor\\sphere.mtl");
	}
	if(!_light){
		_light = pRS->CreateLight();
	}
	
	if(A_Ok != _mesh->Touch())
		return FALSE;

	return TRUE;
}




