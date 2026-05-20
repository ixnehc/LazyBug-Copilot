#include "stdh.h"

#include "RenderSystem/ISpeedTree.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IFont.h"
#include "RenderSystem/IMesh.h"


#include ".\SpeedTreePanel.h"
#include "log/LogFile.h"
#include "shaderlib/SLDefines.h"
#include "stringparser/stringparser.h"
#include "WndBase.h"
#include "fvfex/fvfex.h"
#include "WorldSystem/IStdRes.h"
#include "WorldSystem/IWorldSystem.h"
#include "trisampler/trisampler.h"
#include <algorithm>

extern void DrawSamples(IRenderPort * rp, i_math::vector3df * fPos,IMesh * mesh,IMtrl * mtrl,DWORD nCount);
extern void DrawSphere(IRenderPort * rp,i_math::vector3df &center,float radius,DWORD col,float nStep=10,int nSeg =20);

CSpeedTreePanel::CSpeedTreePanel(void)
{
	_anchor.SetResType(Res_Spt);
	_anchor.SetLabel("SpeedTree");

	_pSpt = NULL;
	_pVB = NULL;
	_drawer = NULL;

	_pBranchSegVB = NULL;
	_pBranchSegIB = NULL;
	_pSegDrawMtrl = NULL;
	_nGrps = 0;
	_segID = -1;
	_nPixel = 10;
	_nValidPixel = 3;
	_gapPixel = 2;
	
	_bShowCol = FALSE;
	_cbShowSample = FALSE;
	_cbShowSeg = FALSE;
	_cbShowUV = FALSE;
	
	_cbShowBr = TRUE;
	_cbShowFr = TRUE;
	_cbShowLeaves = TRUE;

	_mtrlSample = NULL;
	_meshSample = NULL;

	_szBr.set(0,0);
	_szFr.set(0,0);
}

#define GAP_LEN 120

CSpeedTreePanel::~CSpeedTreePanel(void)
{

}
BEGIN_MESSAGE_MAP(CSpeedTreePanel,CResEditPanel)
	ON_COMMAND(IDC_CHECK_SHOWOBB,OnCheckShowCol)
	ON_COMMAND(IDC_BT_GENLGHTUV,OnGenLightUV)
	ON_EN_CHANGE(IDC_EDIT_SEGID,OnSegChange)
	
	ON_BN_CLICKED(IDC_CHECK_SHUV,OnShowChange)
	ON_BN_CLICKED(IDC_CHECK_SAMPLES,OnShowChange)
	ON_BN_CLICKED(IDC_CHECK_SEGMENT,OnSegChange)
	
	ON_BN_CLICKED(IDC_CHECK_BRANCH,OnShowChange)
	ON_BN_CLICKED(IDC_CHECK_FROND,OnShowChange)
	ON_BN_CLICKED(IDC_CHECK_LEAVE,OnShowChange)

	ON_BN_CLICKED(IDC_CHECK_SHOWLEAFHOOK,OnShowChange)
END_MESSAGE_MAP()

void CSpeedTreePanel::OnCheckShowCol()
{
	_bShowCol = (BST_CHECKED==::SendMessage(::GetDlgItem(GetSafeHwnd(),IDC_CHECK_SHOWOBB),BM_GETCHECK,0,NULL));
}

ResEditPanelState *CSpeedTreePanel::_NewState()
{
	return new SpeedTreePanelSate;
}
void CSpeedTreePanel::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_SPEEDTREEANCHOR,_anchor);
	DDX_Control(pDX,IDC_LIST_SPEEDWIND,_windLists);
	DDX_Check(pDX,IDC_CHECK_SHUV,_cbShowUV);
	DDX_Text(pDX,IDC_EDIT_SEGID,_segID);
	DDX_Text(pDX,IDC_EDIT_GAP,_gapPixel);
	DDX_Text(pDX,IDC_EDIT_VALIDPIXEL,_nValidPixel);
	DDX_Text(pDX,IDC_EDIT_DENSITYPIXEL,_nPixel); 
	DDX_Check(pDX,IDC_CHECK_SEGMENT,_cbShowSeg);
	DDX_Check(pDX,IDC_CHECK_SAMPLES,_cbShowSample);
	DDX_Check(pDX,IDC_CHECK_BRANCH,_cbShowBr);
	DDX_Check(pDX,IDC_CHECK_FROND,_cbShowFr);
	DDX_Check(pDX,IDC_CHECK_LEAVE,_cbShowLeaves);
	DDX_Check(pDX,IDC_CHECK_SHOWLEAFHOOK,_cbShowLeafHook);
}
void CSpeedTreePanel::OnShowChange()
{
	UpdateData(TRUE);
}
void CSpeedTreePanel::OnSegChange()
{
	UpdateData(TRUE);
	if(_stateToMod){
		SptData *data = (SptData *)_stateToMod->resdata;
		if(data){
			_ReBuild(data);
			_TouchSegBuffer(g_ssGuiLib.pRS);
		}
	}
}
BOOL CSpeedTreePanel::OnInitDialog()
{
	if(FALSE == CResEditPanel::OnInitDialog())
		return FALSE;

	_windLists.Initialize();
	_windLists.SetListEditStyle(_T("Winds:"),LBS_XT_DEFAULT);
	
	RECT rc;
	GET_CONTROL_RECT(this,IDC_SPEEDWIND_PROP,rc);
	_lodPropGrid.Create(rc,this,IDC_SPEEDWIND_PROP);

	AddCtrl(dynamic_cast<CResEditCtrl *>(&_windLists));
	AddCtrl(dynamic_cast<CResEditCtrl *>(&_lodPropGrid));

	return TRUE;
}
void CSpeedTreePanel::OnGenLightUV()
{
	if(!_stateToMod)
		return;

	SptData * data = (SptData *)(_stateToMod->resdata);
	
	UpdateData(TRUE);

	int nVtxBr = data->GetVtxNumberBr();
	int nVtxFr = data->GetVtxNumberFr();
	_grpIDsBr.resize(nVtxBr);
	_grpIDsFr.resize(nVtxFr);

	CSptLMUVGen gen;
	gen.Gen(data,_nPixel,_nValidPixel,_gapPixel);
	gen.GetGrpInfoBr(_grpIDsBr.data(),_grpIDsBr.size());
	gen.GetGrpInfoFr(_grpIDsFr.data(),_grpIDsFr.size());

	_ReBuild(data);
	RefreshStateMod();

	_grpIDsBr.clear();
	_grpIDsFr.clear();
}
void CSpeedTreePanel::_BuildSamples(ISpt * pSpt)
{
	ISptTriSampleAdapter * pAdapter = pSpt->GetTriSampleAdapter();
	
	void * pVtx = NULL;
	DWORD nVtx = 0,nIBs = 0;
	WORD  * pIB = NULL;
	
	CTriSampler triSampler;
	DWORD nSamples = 0;
	TriSample * pSample = NULL;
		
	pSample = pAdapter->BuildBranchTriSamples(nSamples,1);

	_samplesPos.clear();
	//添加采样点
	for(int i = 0;i<nSamples;i++){
		i_math::vector3df pos = pSample[i].pos;
		i_math::vector3df nor = pSample[i].normal;
		nor.setLength(0.05f);
		pos += nor;
		_samplesPos.push_back(pos);
	}

	pAdapter->Release();

	_TouchSegBuffer(g_ssGuiLib.pRS);
}
void CSpeedTreePanel::_DrawSamples(IRenderPort *rp)
{
	if(NULL==rp||0==_samplesPos.size())
		return;

	if(!_meshSample){
		IRenderSystem * pRS = rp->GetRS();
		_meshSample = (IMesh *)pRS->GetMeshMgr()->ObtainRes("_std\\spt\\sample\\box.msh");
		if(!_meshSample->ForceTouch())
			return;
	}
	if(!_mtrlSample){
		IRenderSystem * pRS = rp->GetRS();
		_mtrlSample = (IMtrl *)pRS->GetMtrlMgr()->ObtainRes("_std\\spt\\sample\\box.mtl");
	}

	DrawSamples(rp,&(_samplesPos[0]),_meshSample,_mtrlSample,_samplesPos.size());
}
void CSpeedTreePanel::OnResDataChange(ResData *data)
{
	if(!data) return;
	_stateToMod->SetData(data);

	SptData * pResData = static_cast<SptData *>(data);
	std::string msg;
	
	msg.append("Branch:\r\n");
	for(int i = 0;i< pResData->numBranchLods;i++)
	{
		std::string s;
		int n = (pResData->branchLODs[i+1]-pResData->branchLODs[i]);
		FormatString(s,"lod[%d]: nPrim< %d > \r\n",i,(n==0)?0:(n-2));
		msg.append(s);
	}
	
	msg.append("Frond:\r\n");
	for(int i = 0;i< pResData->numFrondLods;i++)
	{
		std::string s;
		int n = (pResData->frondLODs[i+1]-pResData->frondLODs[i]);
		FormatString(s,"lod[%d]: nPrim< %d >\r\n",i,(n==0)?0:(n-2));
		msg.append(s);
	}
	
	msg.append("LeafCard:\r\n");
	DWORD stride = fvfSize(pResData->fvfLeafCard);
	for(int i = 0;i< pResData->numLeafCardLods;i++)
	{
		std::string s;
		int n = (pResData->leafCardLODs[i+1] - pResData->leafCardLODs[i])/(stride*2);
		FormatString(s,"lod[%d]: nPrim< %d >\r\n",i,n);
		msg.append(s);
	}
	
	msg.append("LeafMesh:\r\n");
	for(int i = 0;i< pResData->numLeafMeshLods;i++)
	{
		std::string s;
		int n = (pResData->leafMeshLODs[i+1] - pResData->leafMeshLODs[i])/3;
		FormatString(s,"lod[%d]: nPrim< %d >\r\n",i,n);
		msg.append(s);
	}
	
	//引用贴图的位置
	if(TRUE){
		std::string s;
		FormatString(s,"branch diffuse map: [%s] \r\n",pResData->mapBranchDif.c_str());
		msg.append(s);
		FormatString(s,"branch normal map: [%s] \r\n",pResData->mapBranchNor.c_str());
		msg.append(s);
		FormatString(s,"composite diffuse map: [%s] \r\n",pResData->mapCompisiteDif.c_str());
		msg.append(s);
		FormatString(s,"composite normal map: [%s] \r\n",pResData->mapCompisiteNor.c_str());
		msg.append(s);
	}

	CEdit * pEdit =(CEdit*) GetDlgItem(IDC_EDIT_SPEEDTREEINFO);
	pEdit->SetWindowText(fromMBCS(msg.c_str()));
	
	SptData * dataSpt = (SptData *)(_stateToMod->resdata);
	if(dataSpt)
	{
		//展开索引使其变为三角面序列
		_ReBuild(dataSpt);
	}
}

void CSpeedTreePanel::Init3d()
{
	_pSegDrawMtrl = (IMtrl *)g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes("_std\\spt\\sptmtrl\\segdraw.mtl");
	_pVB = g_ssGuiLib.pRS->GetVertexMgr()->CreateVB(6,FVFEX_FLAG_QUX0|FVFEX_XYZW0,1,VBFlag_Dynamic);
	_drawer = g_ssGuiLib.pWS->CreateSptDrawer();

	//初始化调色板
	_InitColPlate();
}
void CSpeedTreePanel::Clear3d()
{
	SAFE_RELEASE(_pSpt);
	SAFE_RELEASE(_pVB);
	SAFE_RELEASE(_pBranchSegVB);
	SAFE_RELEASE(_pBranchSegIB);
	SAFE_RELEASE(_pSegDrawMtrl);
	SAFE_RELEASE(_meshSample);
	SAFE_RELEASE(_mtrlSample);
	SAFE_RELEASE(_drawer);
}

void CSpeedTreePanel::_BuildUV2DTri(SptData * data)
{
	i_math::size2di szBr,szFr;
	if(_pSpt){
		_pSpt->GetMapSize(szBr,szFr,1);
		if(szBr.getArea()>0)
			_BuildTriFlat(_uvVtxsBr,_indicesBr,_triLinesBr,0,0,szBr.w,szBr.h);
		if(szFr.getArea())
			_BuildTriFlat(_uvVtxsFr,_indicesFr,_triLinesFr,szBr.w+GAP_LEN,0,szFr.w,szFr.h);
	}
}

void CSpeedTreePanel::_BuildTriFlat(std::vector<i_math::vector2df> & uvs,
									std::vector<WORD> & indices,
									std::vector<i_math::pos2di> & triLines,
									int ox,int oy,int w,int h)
{
	int nFace = indices.size()/3;
	WORD * pIB = &(indices[0]);

	triLines.clear();
	for(int i = 0;i<nFace;i++){
		i_math::pos2di tri[3];
		int idx = 0;

		if(pIB[0]==pIB[1]||pIB[0]==pIB[2]||pIB[2]==pIB[1]){
			pIB += 3;
			continue;
		}

		for(int j = 0;j<3;j++){
			idx = pIB[j];
			assert(idx<uvs.size());
			i_math::vector2df uv = uvs[idx];
			if(uv.x>1.0f||uv.y>1.0f||uv.x<0||uv.y<0){
				int c = 0;
				c++;
			}
			tri[j].x = ox + int(w*uv.x);
			tri[j].y = oy + int(h*uv.y);
		}

		pIB += 3;
		triLines.push_back(i_math::pos2di(tri[0].x,tri[0].y));
		triLines.push_back(i_math::pos2di(tri[1].x,tri[1].y));
		triLines.push_back(i_math::pos2di(tri[0].x,tri[0].y));
		triLines.push_back(i_math::pos2di(tri[2].x,tri[2].y));
		triLines.push_back(i_math::pos2di(tri[2].x,tri[2].y));
		triLines.push_back(i_math::pos2di(tri[1].x,tri[1].y));
	}	
}
void CSpeedTreePanel::_DrawUV2DTri(IRenderPort * rp)
{
	if(!_triLinesBr.empty()){
		rp->Lines(_triLinesBr.data(),_triLinesBr.size()/2,0xffff0000);
	}

	if(!_triLinesFr.empty())
		rp->Lines(_triLinesFr.data(),_triLinesFr.size()/2,0xffff0000);

	char temp[256];
	DrawFontArg argFont;
	
	argFont.m_ptLoc.set(_szBr.w+3,10);
	sprintf(temp,"[ %d X %d ]",_szBr.w,_szBr.h);
	rp->DrawText(temp,argFont);
	
	argFont.m_ptLoc.set(_szBr.w+_szFr.w + GAP_LEN + 3,10);
	sprintf(temp,"[ %d X %d ]",_szFr.w,_szFr.h);
	rp->DrawText(temp,argFont);
}

void CSpeedTreePanel::_InitColPlate()
{	
	for(int i = 0;i<32;i++){
		BYTE * p = (BYTE*)(_colPlate + 3*i + 0);
		p[0] = i*8;
		p[1] = i*6 + 50;
		p[2] = 0;
		p[3] = 255;
	}

	for(int i = 0;i<32;i++){
		BYTE * p = (BYTE*)(_colPlate + 3*i + 1);
		p[0] = 0;
		p[1] = i*8;
		p[2] = i*6 + 50;
		p[3] = 255;
	}

	for(int i = 0;i<32;i++){
		BYTE * p = (BYTE*)(_colPlate + 3*i + 2);
		p[0] = i*6 + 50;
		p[1] = 0;
		p[2] = i*8;
		p[3] = 255;
	}
}
void CSpeedTreePanel::_FetchIndex(SptData * data,DWORD s,DWORD e,i_math::vector3df * posVtxs,std::vector<int> grpIDs,std::vector<WORD>&indices)
{
	if(!posVtxs)
		return;

	int nIndex = e - s;
	indices.clear();

	WORD * pSrc = &(data->sptIB[0]) + s;

	DWORD nIBs = 0;
	for(int i = 0;i<nIndex-2;i++){
		i_math::vector3df pos[3];

		pos[0] = posVtxs[pSrc[0]];
		pos[1] = posVtxs[pSrc[1]];
		pos[2] = posVtxs[pSrc[2]];

		if((pSrc[0]==pSrc[1]||pSrc[0]==pSrc[1]||pSrc[1]==pSrc[2])){
			pSrc++;
			continue;
		}

		if(_AreaTri(pos)<0.001f){
			pSrc++;
			continue;
		}
		
		//
		if(_grpIDsBr.size()>0){
			int r0 = grpIDs[pSrc[0]];
			int r1 = grpIDs[pSrc[1]];
			int r2 = grpIDs[pSrc[2]];
			if(r0==_segID){
				pSrc++;
				continue;
			}
		}

		indices.push_back(pSrc[0]);
		indices.push_back(pSrc[1]);
		indices.push_back(pSrc[2]);

		pSrc++;
	}
}

struct _UV
{
	float u;
	float v;
};
void CSpeedTreePanel::_FetchVtxBr(SptData *data)
{
	int stride = fvfSize(data->fvfBranch);
	int nVtx = data->branchVB.size()/stride;

	int offVtx = fvfOffset(data->fvfBranch,FVFEX_XYZW0);
	int offUv = fvfOffset(data->fvfBranch,FVFEX_FLAG_QUX0) + 2*sizeof(float);

	//得到顶点buffer的索引区
	BYTE * pVtx = (&(data->branchVB[offVtx]));
	BYTE * pUV = (&(data->branchVB[offUv]));

	_posVtxsBr.resize(nVtx);
	_uvVtxsBr.resize(nVtx);
	
	//FVFEX_FLAG_QUX0  .zw  light map Flat uv
	for(int i = 0;i<nVtx;i++){
		_posVtxsBr[i] = *((i_math::vector3df *)pVtx);
		_uvVtxsBr[i] = *((i_math::vector2df * )pUV);

		pVtx += stride;
		pUV += stride;
	}
	
	// Check uv [0,1]
	if(TRUE){
		BOOL bFailed = FALSE;
		for(int i = 0;i<_uvVtxsBr.size();i++){
			float u = _uvVtxsBr[i].x;
			float v = _uvVtxsBr[i].y;
			if(u<0||u>1.0f||v<0||v>1.0f)
				bFailed = TRUE;
		}
		if(bFailed)
			AfxMessageBox(_T("uv is overflow!"));
	}
}

void CSpeedTreePanel::_ReBuild(SptData * data)
{
	const char * respath = _anchor.GetRelativePath();

	SAFE_RELEASE(_pSpt);
	_pSpt = (ISpt *)g_ssGuiLib.pRS->GetDynSptMgr()->Create((SptData *)data,respath);
	_pSpt->ForceTouch();
	
	_pSpt->GetMapSize(_szBr,_szFr,1);

	_FetchVtxBr(data);
	_FetchVtxFr(data);

	if(data->numBranchLods){
		DWORD s = data->branchLODs[0];
		DWORD e = data->branchLODs[1];
		_FetchIndex(data,s,e,&(_posVtxsBr[0]),_grpIDsBr,_indicesBr);
	}

	if(data->numFrondLods){
		DWORD s = data->frondLODs[0];
		DWORD e = data->frondLODs[1];
		_FetchIndex(data,s,e,&(_posVtxsFr[0]),_grpIDsFr,_indicesFr);
	}

	_BuildUV2DTri(data);
	_BuildSamples(_pSpt);
}
float CSpeedTreePanel::_AreaTri(i_math::vector3df * pos)
{
	float a = (float)pos[0].getDistanceFrom(pos[1]);
	float b = (float)pos[0].getDistanceFrom(pos[2]);
	float c = (float)pos[1].getDistanceFrom(pos[2]);
	float p = (a+b+c)/2;
	float area = sqrtf(p*(p-a)*(p-b)*(p-c));
	return area;
}

void CSpeedTreePanel::_FetchVtxFr(SptData *data)
{
	//没有展开的UV坐标
	if(!data->uvFrond.size())
		return;

	int stride = fvfSize(data->fvfFrond);
	int nVtx = data->frondVB.size()/stride;

	int offVtx = fvfOffset(data->fvfFrond,FVFEX_XYZW0);
	int offUv = fvfOffset(data->fvfFrond,FVFEX_FLAG_QUX0) + 2*sizeof(float);

	//得到顶点buffer的索引区
	BYTE * pVtx = (&(data->frondVB[offVtx]));
	BYTE * pUV = (&(data->frondVB[offUv]));

	_posVtxsFr.resize(nVtx);
	_uvVtxsFr.resize(nVtx);
	assert(nVtx==data->uvFrond.size()) ;

	//FVFEX_FLAG_QUX0  .zw  light map Flat uv
	for(int i = 0;i<nVtx;i++){
		_posVtxsFr[i] = *((i_math::vector3df *)pVtx);
		_uvVtxsFr[i] = *((i_math::vector2df * )pUV);
		pVtx += stride;
		pUV += stride;
	}
}
void CSpeedTreePanel::_TouchSegBuffer(IRenderSystem * pRS)
{
	//已经创建好资源，不再创建
	SAFE_RELEASE(_pBranchSegVB);
	SAFE_RELEASE(_pBranchSegIB);

	//不存在segment信息时
	if(0==_posVtxsBr.size()||!pRS)
		return;
	
	SptData * data = (SptData *)(_stateToMod->resdata);
	if(!data)
		return;

	FVFEx fvfVB = FVFEX_XYZ0|FVFEX_DIFFUSE;
	_pBranchSegVB = pRS->GetVertexMgr()->CreateVB(_posVtxsBr.size(),fvfVB,1);
	_pBranchSegIB = pRS->GetVertexMgr()->CreateIB(_indicesBr.size());

	DWORD vbStride = _pBranchSegVB->GetStride();
	BYTE * pVB = (BYTE *)_pBranchSegVB->Lock(TRUE);
	BYTE * pPos = pVB + fvfOffset(fvfVB,FVFEX_XYZ0);
	BYTE * pDif = pVB + fvfOffset(fvfVB,FVFEX_DIFFUSE);

	if(pVB){
		//得到顶点buffer的索引区
		int nVtx = data->branchVB.size()/fvfSize(data->fvfBranch);
		for(int i = 0;i<nVtx;i++){
			*((i_math::vector3df*)pPos) = _posVtxsBr[i];
			int idxCol = (_grpIDsBr.size()>0)?_grpIDsBr[i]:0;
			*((DWORD*)pDif) = _colPlate[idxCol%30];
			pPos += vbStride;
			pDif += vbStride;
		}

		_pBranchSegVB->Unlock();
	}

	if(_pBranchSegIB){
		void * p = _pBranchSegIB->Lock(TRUE);
		if(p)
			memcpy(p,_indicesBr.data(),_indicesBr.size()*sizeof(WORD));
		_pBranchSegIB->Unlock();
	}
}
void CSpeedTreePanel::_DrawSegInfo(IRenderPort * rp)
{
	if(_pSpt){
		SptLod & lod = _pSpt->GetLod(0);
		IRenderer * render = rp->ObtainRenderer();
		IShader * shader = render->BeginRaw(_pSegDrawMtrl);
		if(shader){
			_pSegDrawMtrl->BindEP(shader,0);
			_pSegDrawMtrl->BindState(shader,0);
			VBBindArg arg;
			i_math::matrix43f mat;
			shader->SetEP_World(&mat,1);
			shader->BindVB(_pBranchSegVB,_pBranchSegIB,&(arg));
			shader->DoShadeRaw();
			render->EndRaw(shader);
		}
	}
}
BOOL CSpeedTreePanel::TestCollision(ISpt * pSpt,HitProbe & prob)
{
	i_math::vector3df outIntersec;
	BOOL ret = pSpt->TestCollision(prob,i_math::vector3df(0,0,0),1.0f,0,outIntersec);
	return ret;
}
void CSpeedTreePanel::_DrawAuxObj(IRenderPort * rp)
{
	if(TRUE)
	{
		extern  BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df&aabb,DWORD col);
		i_math::matrix43f mat;
		i_math::aabbox3df aabb  = _pSpt->GetBoundingBox();
		SptData * data = (SptData *)(_stateToMod->resdata);
		CalcBoundBox(data);
		DrawOBB(rp,mat,aabb,0xaa22aa22);
	}

	if(TRUE)
	{
		int nCollisionObjects = _pSpt->GetNumberOfCollisionObjects();
		CollisionObjectType coType;
		for(int i = 0;i<nCollisionObjects;i++)
		{
			void * pOjb = _pSpt->GetCollisionObject(i,coType);
			switch(coType)
			{
			case CO_CAPSULE:
				{
					i_math::capsulef * cap = static_cast<i_math::capsulef *>(pOjb);
					extern BOOL DrawCapsule(IRenderPort * rp,i_math::capsulef & cap);
					DrawCapsule(rp,*cap);

					break;
				}
			case CO_SPHERE:
				{
					i_math::spheref  sph = *(static_cast<i_math::spheref *>(pOjb));
					extern void DrawSphere(IRenderPort * rp,i_math::vector3df &center,float radius,DWORD col,float nStep=10,int nSeg =20);
					DrawSphere(rp,sph.center,sph.radius,0x8800aaaa);

					break;
				}
			default:
				break;
			}	
		}
	}

	//
	i_math::spheref sph = _pSpt->GetLeafBoundSphere();
	if(sph.radius>0)
	{
		extern void DrawSphere(IRenderPort * rp,i_math::vector3df &center,float radius,DWORD col,float nStep=10,int nSeg =20);
		DrawSphere(rp,sph.center,sph.radius,0xff006600);
	}
}
void CSpeedTreePanel::_DrawSpt(IRenderPort * rp)
{
	if(!_pSpt)
		return;	
	_pSpt->Touch();

	i_math::vector3df eyePos,cameraDir;

	ICamera * camera = rp->GetCamera();
	camera->GetEyePos(eyePos);
	float dist = float(eyePos.getDistanceFrom(i_math::vector3df(0,0,0))); 
	camera->GetEyeDir(cameraDir);

	cameraDir.toAngle();
	std::string s;
	FormatString(s,"Azimuth: %f",cameraDir.x);
	
	if(_drawer){
		SptDrawParam param;
		param.bShowFrond = _cbShowFr;
		param.bShowLeafCard = _cbShowLeaves;
		param.bShowLeafMesh  = _cbShowLeaves;
		param.bShowTrunk = _cbShowBr;
		param.dist = dist;
		_drawer->SetRP(rp);
		_drawer->Draw(_pSpt,&param);
	}	
}

void CSpeedTreePanel::Draw(IRenderPort *rp)
{
	if(_cbShowSeg)
		_DrawSegInfo(rp);
	else
		_DrawSpt(rp);

	//绘制连通区域的信息
	if(_cbShowUV)
		_DrawUV2DTri(rp);
	
	if(_cbShowSample)
		_DrawSamples(rp);
	
	if(_bShowCol)
		_DrawAuxObj(rp);

	if(_cbShowLeafHook)
		_DrawLeafSphere(rp);
}
	
BOOL CSpeedTreePanel::StateToControl(ResEditPanelState *state0)
{
	if (FALSE==CResEditPanel::StateToControl(state0))
		return FALSE;
	
	SptData * data = (SptData *)state0->resdata;

	SAFE_RELEASE(_pSpt);
	_pSpt = (ISpt *)g_ssGuiLib.pRS->GetDynSptMgr()->Create((SptData *)data,_anchor.GetRelativePath());
	_pSpt->ForceTouch();

	return TRUE;
}
void CSpeedTreePanel::_DrawLeafSphere(IRenderPort * rp)
{
	i_math::vector3df * pos = NULL;
	float radius = 0;
	DWORD count = 0;
	if(_pSpt){
		_pSpt->GetLeafHookPoint(pos,radius,count);
		for(int i = 0;i<count;i++)
			DrawSphere(rp,pos[i],radius,0xffff0000,6,6);
	}
}



void CSpeedTreePanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}
