#pragma once

#include "ResEditPanel.h"
#include "ResAnchor.h"
#include "SpeedWindList.h"
#include "SpeedTreeLodCtrl.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/ISpt.h"

struct SpeedTreePanelSate :public ResEditPanelState
{
	SpeedTreePanelSate(){
		iSelWind = -1;
	}
	virtual void Copy(ResEditPanelState &src)
	{
		ResEditPanelState::Copy(src);
		SpeedTreePanelSate * pSrc = (SpeedTreePanelSate *)&src;
		iSelWind = pSrc->iSelWind;
	}
	int iSelWind;
};

class ISpt;
class IVertexBuffer;
class IIndexBuffer;
class GuiLib_Api CSpeedTreePanel :public CResEditPanel 
{
public:
	CSpeedTreePanel(void);
	~CSpeedTreePanel(void);
public:
	virtual UINT GetIDD()	{		return IDD_PANEL_SPEEDTREE;	}
	virtual void OnResDataChange(ResData *data);
	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();
	virtual void Draw(IRenderPort *rp);
	
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state

protected:
	virtual ResEditPanelState *_NewState();

	DECLARE_MESSAGE_MAP()
	afx_msg virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 
	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnCheckShowCol();
	afx_msg void OnGenLightUV();
	afx_msg void OnSegChange();
	afx_msg void OnShowChange();
	BOOL TestCollision(ISpt * pSpt,HitProbe & prob);

private:
	CSpeedWindList _windLists;
	CSpeedTreeLodCtrl _lodPropGrid;
	ISpt * _pSpt;
	IVertexBuffer * _pVB;
	BOOL _bShowCol;

//////////////////////////////////////////////////////////////////////////
	void _TouchSegBuffer(IRenderSystem * pRS);
	void _DrawSegInfo(IRenderPort * rp);
	void _InitColPlate();
	
	void _BuildUV2DTri(SptData * data);
	void _BuildTriFlat(	std::vector<i_math::vector2df> & uvs,
						std::vector<WORD> & indices,
						std::vector<i_math::pos2di> & triLines,
						int ox,int oy,int w,int h);
	void _DrawUV2DTri(IRenderPort * rp);
	
	void _FetchIndex(SptData * data,DWORD s,DWORD e,i_math::vector3df * pos,std::vector<int> grpIDs,std::vector<WORD>&indices);
	void _FetchVtxBr(SptData *data);
	void _FetchVtxFr(SptData *data);
	float _AreaTri(i_math::vector3df * pos);	//计算三角形的面积
	
	void _ReBuild(SptData * data);
	
	void _BuildSamples(ISpt * pSpt);
	void _DrawSamples(IRenderPort *rp);
	
	void _DrawAuxObj(IRenderPort * rp);
	void _DrawSpt(IRenderPort * rp);

	void _DrawLeafSphere(IRenderPort * rp);

	int _segID;
	DWORD _colPlate[96];
	IVertexBuffer * _pBranchSegVB;
	IIndexBuffer * _pBranchSegIB;
	IMtrl * _pSegDrawMtrl;


	int _nPixel;
	int _nValidPixel;
	int _gapPixel;

	//假定不同的顶点含有不同的位置
	CSptLMUVGen _uvGen;

	//uv 
	std::vector<i_math::pos2di> _triLinesBr;
	std::vector<i_math::pos2di> _triLinesFr;
	
	std::vector<i_math::vector3df> _posVtxsBr;
	std::vector<i_math::vector3df> _posVtxsFr;
	
	std::vector<i_math::vector2df> _uvVtxsBr;
	std::vector<i_math::vector2df> _uvVtxsFr;

	std::vector<i_math::vector3df> _samplesPos;
	IMesh * _meshSample;
	IMtrl * _mtrlSample;

	std::vector<int> _grpIDsBr;
	std::vector<int> _grpIDsFr;
 
	std::vector<WORD> _indicesBr;
	std::vector<WORD> _indicesFr;

	BOOL _cbShowUV;
	BOOL _cbShowSeg;
	BOOL _cbShowSample;
	BOOL _cbShowBr;
	BOOL _cbShowFr;
	BOOL _cbShowLeaves;
	BOOL _cbShowLeafHook;
	i_math::size2di _szBr,_szFr;

	DWORD _nGrps;
	ISptDrawer * _drawer;
};






