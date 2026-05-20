/********************************************************************
	created:	2007/6/11   16:37
	filename: 	e:\IxEngine\Proj_GuiLib\UVAtlasMakeDlg.cpp
	author:		cxi
	
	purpose:	UVAtlas make dialog
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IVertexBuffer.h"
#include "RenderSystem/IUtilRS.h"
#include "RenderSystem/IShader.h"

#include "WndBase.h"
#include ".\uvatlasmakedlg.h"

#include "UVAtlasMakeDlg.h"

#include "stringparser/stringparser.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CAtlasView
void CAtlasView::Draw(CDC *pDC)
{
	CRect rc;
	GetClientRect(&rc);
	float w,h;
	w=(float)rc.Width();
	h=(float)rc.Height();
	pDC->FillSolidRect(0,0,rc.Width(),rc.Height(),RGB(64,64,64));


	if (!data)
		return;

	i_math::texcoordf *uvs=data->vtxframes[0].tex[channel];
	if (!uvs)
		return;

	MeshData::AtlasInfo *ai=data->FindAtlasInfo(channel);
	if (ai)
	{
		w=ai->w;
		h=ai->h;
	}

	CPen pen,*penOld;
	pen.CreatePen(PS_SOLID,1,RGB(0,114,255));

	penOld=pDC->SelectObject(&pen);

	for (int i=0;i<data->lodInfos[0].indice.size();i+=3)
	{
		POINT pts[3];

		for (int j=0;j<3;j++)
		{
			i_math::vector2df v=uvs[data->lodInfos[0].indice[i+j]];
			pts[j].x=(LONG)(v.x*w);
			pts[j].y=(LONG)(v.y*h);
		}

		pDC->MoveTo(pts[0]);
		pDC->LineTo(pts[1]);
		pDC->LineTo(pts[2]);
		pDC->LineTo(pts[0]);
	}

	pDC->SelectObject(penOld);

	if (ai)
	{
		CBrush br;
		br.CreateSolidBrush(RGB(255,99,5));

		CRect rc2(0,0,(int)w,(int)h);
		pDC->FrameRect(&rc2,&br);
	}

}



//////////////////////////////////////////////////////////////////////////
// CUVAtlasMakeDlg dialog

IMPLEMENT_DYNAMIC(CUVAtlasMakeDlg, CDialog)
CUVAtlasMakeDlg::CUVAtlasMakeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUVAtlasMakeDlg::IDD, pParent)
{
	_bModified=FALSE;
}

CUVAtlasMakeDlg::~CUVAtlasMakeDlg()
{
}

void CUVAtlasMakeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WIDTH, _width);
	DDX_Control(pDX, IDC_HEIGHT, _height);
	DDX_Control(pDX, IDC_VIEWPORT, _viewport);
	DDX_Control(pDX, IDC_DESC, _desc);
	DDX_Control(pDX, IDC_UVCHANNEL, _channelcombo);
}


BEGIN_MESSAGE_MAP(CUVAtlasMakeDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_UVCHANNEL, OnCbnSelchangeUvchannel)
	ON_BN_CLICKED(IDC_MAKEUVATLAS, OnBnClickedMakeuvatlas)
	ON_BN_CLICKED(IDC_REMOVEUVCHANNEL, OnBnClickedRemoveuvchannel)
	ON_BN_CLICKED(IDC_ASSIGNATLASPARAM, OnBnClickedAssignatlasparam)
END_MESSAGE_MAP()


// CUVAtlasMakeDlg message handlers

BOOL CUVAtlasMakeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;

	GET_CONTROL_RECT(this,IDC_WIDTH,rc);
	HIDE_CONTROL(this,IDC_WIDTH);
	_spinWidth.Create(this,rc,IDC_WIDTH);
	_spinWidth.SetRange(4,1024);
	_spinWidth.SetSpinSpeed(2);
	_spinWidth.SetValue(256);

	GET_CONTROL_RECT(this,IDC_HEIGHT,rc);
	HIDE_CONTROL(this,IDC_HEIGHT);
	_spinHeight.Create(this,rc,IDC_HEIGHT);
	_spinHeight.SetRange(4,1024);
	_spinHeight.SetSpinSpeed(2);
	_spinHeight.SetValue(256);


	GET_CONTROL_RECT(this,IDC_GUTTER,rc);
	HIDE_CONTROL(this,IDC_GUTTER);
	_spinGutter.Create(this,rc,IDC_GUTTER);
	_spinGutter.EnableFloatMode(TRUE);
	_spinGutter.SetRange(1,64);
	_spinGutter.SetSpinSpeed(0.05f);
	_spinGutter.SetValue(2);

	GET_CONTROL_RECT(this,IDC_STRETCH,rc);
	HIDE_CONTROL(this,IDC_STRETCH);
	_spinStretch.Create(this,rc,IDC_STRETCH);
	_spinStretch.EnableFloatMode(TRUE);
	_spinStretch.SetRange(0,1);
	_spinStretch.SetSpinSpeed(0.01f);
	_spinStretch.SetValue(1);

	assert(_data->vtxframes.size()==1);
	_viewport.SetData(_data);

	_iSel=0;

	_Update();

	_bModified=FALSE;

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CUVAtlasMakeDlg::SetWorkingData(MeshData *data,IRenderSystem *pRS,IUtilRS *pUtilRS)
{
	_data=data;

	_pRS=pRS;
	_pUtilRS=pUtilRS;
}



void CUVAtlasMakeDlg::OnCbnSelchangeUvchannel()
{
	// TODO: Add your control notification handler code here
	_iSel=_channelcombo.GetCurSel();

	_Update();
}

void CUVAtlasMakeDlg::_Update()
{
	if (TRUE)
	{
		_channelcombo.ResetContent();
		MeshData::VtxData &vd=_data->vtxframes[0];

		for (int i=0;i<8;i++)
		{
			std::string s;
			FormatString(s,"Channel%02d",i);
			if (!vd.tex[i])
				s+="[Empty]";
			else
			{
				if (_data->FindAtlasInfo(i))
					s+="[Atlas]";
			}
			_channelcombo.AddString(fromMBCS(s.c_str()));
		}
		_channelcombo.SetCurSel(_iSel);
	}

	_viewport.SetChannel(_iSel);
	_viewport.InvalidateRect(NULL);
	_UpdateDesc();

	//Now the title
	if (TRUE)
	{
		std::string s;
		FormatString(s,"UVAtlas Make --%d vertices,%d faces",
							_data->vtxframes.m_nVtx,_data->lodInfos[0].indice.size()/3);
		SetWindowText(fromMBCS(s.c_str()));
	}
}

void CUVAtlasMakeDlg::_UpdateDesc()
{
	std::string s;

	MeshData::AtlasInfo *ai=_data->FindAtlasInfo(_iSel);

	if (!ai)
	{
		if (_data->vtxframes[0].tex[_iSel])
			s="no atlas info in this channel";
		else
			s="empty channel";
	}
	else
		FormatString(s,"UV Channel %02d Atlas Info: %dx%d, gutter:%f",
							ai->channel,ai->w,ai->h,ai->gutter);

	_desc.SetWindowText(fromMBCS(s.c_str()));
}

void CUVAtlasMakeDlg::_CollectArg(UVAtlasArg &arg)
{
	arg.chUV=_iSel;
	arg.w=(DWORD)_spinWidth.GetValue();
	arg.h=(DWORD)_spinHeight.GetValue();
	arg.gutter=(float)_spinGutter.GetValue();
	arg.stretch=(float)_spinStretch.GetValue();
}


void AddAtlasInfo(MeshData*data,UVAtlasArg &arg)
{
	MeshData::AtlasInfo *ai=data->FindAtlasInfo(arg.chUV);
	if (!ai)
	{
		data->atlases.resize(data->atlases.size()+1);
		ai=&data->atlases[data->atlases.size()-1];
	}
	ai->channel=(WORD)arg.chUV;
	ai->gutter=arg.gutter;
	ai->w=(WORD)arg.w;
	ai->h=(WORD)arg.h;

}


template<class T>
void RemapVtxBuf(T *&buf,DWORD *remap,DWORD nVtx)
{
	if (!buf)
		return;
	T *buf2=new T[nVtx];

	for (int i=0;i<nVtx;i++)
		buf2[i]=buf[remap[i]];

	SAFE_DELETE(buf);
	buf=buf2;
}

void CUVAtlasMakeDlg::OnBnClickedMakeuvatlas()
{
	MeshData dataBackUp;
	dataBackUp.Copy(*_data);

	assert(_data->vtxframes.size()==1);
	MeshData::VtxData &vd=_data->vtxframes[0];

	//first remove the current channel
	SAFE_DELETE(vd.tex[_iSel]);

	//	//do the compression
	//	_data->CollapseDupeVtx();

	//Now make an atlas at the current channel
	if (TRUE)
	{
		VBPatch pa,paResult;
		pa.fvf=FVFEX_XYZ0;
		pa.vtx=vd.pos;
		pa.nVtx=_data->vtxframes.m_nVtx;
		pa.idx=&_data->lodInfos[0].indice[0];
		pa.nIdx=_data->lodInfos[0].indice.size();

		paResult.fvf=0;

		DWORD *remap;

		UVAtlasArg arg;
		_CollectArg(arg);

		//Do the making
// 		if (FALSE==_pUtilRS->MakeUVAtlas(pa,paResult,arg,&remap))
// 		{
// 			_data->Copy(dataBackUp);
// 			AfxMessageBox("Failed to make UVAtlas on this channel! Please re-adjust the argument.",MB_OK);
// 			return;
// 		}
		AfxMessageBox(_T("该功能已被屏蔽!"), MB_OK);
		return;
		

		_data->vtxframes.m_nVtx=paResult.nVtx;

		//Do the remapping
		RemapVtxBuf<i_math::vector3df>(vd.pos,remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector3df>(vd.posExt,remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector3df>(vd.normal,remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector3df>(vd.normalExt,remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector3df>(vd.binormal,remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector3df>(vd.tangent,remap,paResult.nVtx);
		RemapVtxBuf<DWORD>(vd.color,remap,paResult.nVtx);
		RemapVtxBuf<i_math::weight3f>(vd.weight,remap,paResult.nVtx);
		RemapVtxBuf<DWORD>(vd.boneindex0,remap,paResult.nVtx);
		RemapVtxBuf<DWORD>(vd.boneindex1,remap,paResult.nVtx);
		for (int j=0;j<ARRAY_SIZE(vd.tex);j++)
				RemapVtxBuf<i_math::texcoordf>(vd.tex[j],remap,paResult.nVtx);
		RemapVtxBuf<i_math::vector4df>(vd.colorF,remap,paResult.nVtx);
		//XXXXX:More VtxData Element

		//Copy the new channel data
		vd.tex[_iSel]=new i_math::texcoordf[paResult.nVtx];
		memcpy(vd.tex[_iSel],paResult.vtx,paResult.nVtx*sizeof(i_math::texcoordf));

		//Copy the index data
		_data->lodInfos[0].indice.resize(paResult.nIdx);
		memcpy(&(_data->lodInfos[0].indice[0]),paResult.idx,paResult.nIdx*sizeof(WORD));

		//Now update the atlasinfo
		AddAtlasInfo(_data,arg);

		//Now update the segs
		_data->MakeSegBone(MIN_BONE);

	}

	_Update();
	_bModified=TRUE;
}

void CUVAtlasMakeDlg::OnBnClickedRemoveuvchannel()
{

	for (int i=0;i<_data->vtxframes.size();i++)
		SAFE_DELETE(_data->vtxframes[i].tex[_iSel]);

	//remove the atlas info
	int i;
	for (i=0;i<_data->atlases.size();i++)
	{
		if (_data->atlases[i].channel==_iSel)
			break;
	}
	if (i<_data->atlases.size())
		_data->atlases.erase(_data->atlases.begin()+i);


	//do the collapse
	_data->CollapseDupeVtx();

	_Update();
	_bModified=TRUE;

}

void CUVAtlasMakeDlg::OnBnClickedAssignatlasparam()
{
	if (!_data->vtxframes[0].tex[_iSel])
		return;

	UVAtlasArg arg;
	_CollectArg(arg);

	AddAtlasInfo(_data,arg);

	_Update();

	_bModified=TRUE;

}
