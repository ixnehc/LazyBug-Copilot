
#include "stdh.h"

#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IMesh.h"


#include "GuiAgent_ProbeMove.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IEnvLight.h"

#include "GuiData_El.h"

#include "GuiAgent_MatrixEdit.h"

#include "GuiActor_El.h"

#include "log/LogDump.h"

#define SAMPLE_POINT_SZ 10.0f
extern BOOL ProjectScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf);

CGuiAgent_ProbeMove::CGuiAgent_ProbeMove(Matrix_EditMode mode)
:CGuiAgent_3DNodeMatEdit(mode)
{
	_mtrl = NULL;
	_mesh = NULL;
	_light = NULL;
	_iCorSel=-1;
}

CGuiAgent_ProbeMove::~CGuiAgent_ProbeMove(void)
{
	SAFE_RELEASE(_mtrl);
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_light);
}

BOOL CGuiAgent_ProbeMove::OnDraw()
{	
	IProbeCubeMapEditor * editor = NULL;
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();
	
	if(!editor||0==data->hObjSels.size())	
		return TRUE;
	
	HMapObj & hObj = data->hObjSels.back();
	ProbeCubeInfo info;
	if(!editor->GetInfo(hObj,info))
		return TRUE;

	_DrawAbbCorner(info.aabb);
	
	return CGuiAgent_3DNodeMatEdit::OnDraw();
}

BOOL CGuiAgent_ProbeMove::_TouchRes()
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

BOOL CGuiAgent_ProbeMove::OnLButtonDown(int x,int y,DWORD flag)
{
	BOOL bOk_ActiveP = TRUE;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(0==data->hObjSels.size()){ //不存在测试的对象
		_iCorSel = -1;	
		bOk_ActiveP = FALSE;
	}

	i_math::matrix43f matSample;
	if(FALSE==_TouchRes()){	
		bOk_ActiveP = FALSE;
	}
	
	IRenderPort * rp = GetRP();
	assert(rp);
	HitProbe lineHit;
	rp->CalcHitProbe(x,y,lineHit);

	_iCorSel = -1;
	float distMin = 0;
	for(int i = 0;i<8;i++){
		matSample.setScale(SAMPLE_POINT_SZ,SAMPLE_POINT_SZ,SAMPLE_POINT_SZ);
		matSample.addTranslation(_corners[i]);
		ProjectScaleMask(rp,matSample);
		
		MeshSnapshotArg arg;
		IMeshSnapshot * _msSnap = _mesh->ObtainSnapshot();
		_msSnap->TakeSnapshot(matSample,arg);

		float dist = 0;
		if(_msSnap->HitTest(lineHit,dist)){
			if(_iCorSel<0){
				_iCorSel = i;
				distMin = dist;
			}
			else{
				if(dist<distMin){
					_iCorSel = i;
					distMin = dist;
				}
			}
		}

		SAFE_RELEASE(_msSnap);
	}
		
	if(_iCorSel<0)
		bOk_ActiveP = FALSE;

	
	if(!bOk_ActiveP)
		return CGuiAgent_3DNodeMatEdit::OnLButtonDown(x,y,flag); 

	
	//选中激活点所在的节点，其他节点均置为未选中状态
	if(bOk_ActiveP){
		assert(data->hObjSels.size());
		HMapObj hObj = data->hObjSels.back();
		data->hObjSels.clear();
		data->hObjSels.push_back(hObj);
	}

	return FALSE; //存在激活点被选中
}

void CGuiAgent_ProbeMove::_DrawAbbCorner(i_math::aabbox3df &abb)
{
	IRenderPort * rp = GetRP();
	assert(rp);
	
	//所见即所得
	abb.getCorners(_corners);
	
	if(FALSE==_TouchRes())	
		return;
	
	i_math::matrix43f matSample;
	for(int i = 0;i<8;i++){
		matSample.setScale(SAMPLE_POINT_SZ,SAMPLE_POINT_SZ,SAMPLE_POINT_SZ);
		matSample.addTranslation(_corners[i]);
		ProjectScaleMask(rp,matSample);

		DWORD col = (i==_iCorSel)?0xffffff00:0xff00ffff;

		i_math::vector3df eyeDir;
		ICamera * cam = rp->GetCamera();
		cam->GetEyeDir(eyeDir);
		_light->SetDirLight(eyeDir,0,0xffffffff,0xff555555);
		rp->SimpleDrawMesh(_mesh,matSample,col,FALSE,_mtrl,_light);
	}
}

//////////////////////////////////////////////////////////////////////////
void * CGuiAgent_ProbeMove::_GetSelBuf()
{
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return NULL;
	
	return &(data->hObjSels);
}
i_math::matrix43f * CGuiAgent_ProbeMove::_GetMat(H3DNode node)
{
	IProbeCubeMapEditor  * editor = _GetEditor();
	if(!editor)
		return NULL;

	HMapObj hObj = HMapObj(node);
	ProbeCubeInfo info;
	if(!editor->GetInfo(hObj,info))
		return NULL;

	if((_iCorSel>=0)&&(_iCorSel<ARRAY_SIZE(_corners)))
	{ //移动角点
		_matTemp.setTranslation(_corners[_iCorSel]);
	}
	else//移动节点
	{
		i_math::vector3df vec = info.aabb.getCenter();
		_matTemp.setTranslation(vec);
	}

	return &_matTemp;
}
i_math::pos2di * CGuiAgent_ProbeMove:: _GetBlock(H3DNode node)
{
	IProbeCubeMapEditor * editor = _GetEditor();
	if(!editor)
		return NULL;

	HMapObj hObj = HMapObj(node);

	if(_iCorSel>=0){
		GuiData_El * data = (GuiData_El *)FindData("envlight");
		if(0==data->hObjSels.size()>0)
			return NULL;

		hObj = data->hObjSels.back(); //编辑角点
	}

	if(editor->GetMapFileBlk(HMapObj(node),_ptBlkTemp))
		return &_ptBlkTemp;

	return NULL;	
}
float CGuiAgent_ProbeMove::_Clamp(float v,float ov)
{
	float rv = v;

	if(v*ov<0){ //穿过
		rv = (ov>0)?LIMIT_LEN_MIN:-LIMIT_LEN_MIN;
	}
	else{
		float as = abs(v);
		if(as>LIMIT_LEN_MAX)
			as = LIMIT_LEN_MAX;

		if(as<LIMIT_LEN_MIN)
			as = LIMIT_LEN_MIN;
		
		rv = (v>0)?as:-as;
	}

	return rv;
}
void CGuiAgent_ProbeMove::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	IProbeCubeMapEditor * editor = _GetEditor();
	if(!editor)
		return;
	
	HMapObj hObj = HMapObj(node);
	if(_iCorSel>=0){
		GuiData_El * data = (GuiData_El *)FindData("envlight");
		assert(data);
		assert(data->hObjSels.size()>0);
		hObj = data->hObjSels.back();
	}
	
	ProbeCubeInfo info;
	if(!editor->GetInfo(hObj,info))
		return;

	if(_iCorSel>=0){
		i_math::vector3df vec = mat.getTranslation();

		//加入大小限制
		i_math::vector3df diag = vec - _corners[7 - _iCorSel];
		i_math::vector3df diagOrg = _corners[_iCorSel] - _corners[7-_iCorSel];

		i_math::vector3df clamp_v = diag;
		
		//判断是否改变了的位置关系
		clamp_v.x = _Clamp(diag.x,diagOrg.x);			
		clamp_v.y = _Clamp(diag.y,diagOrg.y);
		clamp_v.z = _Clamp(diag.z,diagOrg.z);
		
		//改变后的位置
		vec = _corners[7-_iCorSel] + clamp_v;
		
		//未发生改变
		if(vec==_corners[_iCorSel])
			return;
		
		_corners[_iCorSel] = vec;
		info.aabb.reset(_corners[7 - _iCorSel]);//加入对角点	
		info.aabb.addInternalPoint(_corners[_iCorSel]); 
	}
	else
	{
		i_math::vector3df vec0 = info.aabb.getCenter();
		i_math::vector3df vec1 = mat.getTranslation();
		i_math::vector3df vecOff = vec1 - vec0;
		info.aabb.MinEdge += vecOff;
		info.aabb.MaxEdge += vecOff;
	}
	
	if(editor->SetInfo(hObj,info))
		info.aabb.getCorners(_corners);

	node = H3DNode(hObj);
}
void CGuiAgent_ProbeMove::_EndMatrixEdit(i_math::matrix43f *mat)
{	
	CGuiAgent_3DNodeMatEdit::_EndMatrixEdit(mat);

	CGuiPanel_El * pActor = (CGuiPanel_El *)_GetActor();
	assert(pActor);
	pActor->UpdateSel(TRUE); //强制更新界面 因为参数发生了参数
}
IProbeCubeMapEditor * CGuiAgent_ProbeMove::_GetEditor()
{	
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return NULL;

	return data->GetEditor();
}
DWORD * CGuiAgent_ProbeMove::_GetVer()
{
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		return &(data->ver);
	return NULL;
}


