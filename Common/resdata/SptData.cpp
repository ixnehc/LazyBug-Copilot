/********************************************************************
	created:	2008/6/25   11:21
	filename: 	e:\IxEngine\Common\resdata\sptdata.cpp
	author:		star
	
	purpose:	speed tree data
*********************************************************************/
#include "stdh.h"

#include "SptData.h"

#include "stringparser/stringparser.h"

#include "datapacket/DataPacket.h"

#include "fvfex/fvfex.h"

#include "algorithm"

#include <assert.h>

IMPLEMENT_CLASS(SptData);


void SptData::Save(CDataPacket &dp)
{
	dp.Data_NextDword() = Res_Ver;

	SaveHeader(dp);
	DP_WriteVector(dp,branchVB);		// 0
	DP_WriteVector(dp,frondVB);
	DP_WriteVector(dp,leafCardVB);
	DP_WriteVector(dp,leafMeshVB);
	DP_WriteVector(dp,sptIB);
	
	DP_WriteArray(dp,branchLODs);		// 1
	DP_WriteArray(dp,frondLODs);
	DP_WriteArray(dp,leafCardLODs);
	DP_WriteArray(dp,leafMeshLODs);

	dp.Data_WriteData(&fvfBranch,sizeof(fvfBranch));	// 2
	dp.Data_WriteData(&fvfFrond,sizeof(fvfFrond));
	dp.Data_WriteData(&fvfLeafCard,sizeof(fvfLeafCard));
	dp.Data_WriteData(&fvfLeafMesh,sizeof(fvfLeafMesh));

	dp.Data_NextFloat() = fLeafRockScale;				//3
	dp.Data_NextFloat() = fLeafRusltScale;

	dp.Data_NextByte() = numLeafCardLods;				//4
	dp.Data_NextByte() = numBranchLods;
	dp.Data_NextByte() = numFrondLods;
	dp.Data_NextByte() = numLeafMeshLods;

	dp.Data_WriteString(mapCompisiteDif);				//5
	dp.Data_WriteString(mapCompisiteNor);
	dp.Data_WriteString(mapBranchDif);
	dp.Data_WriteString(mapBranchNor);
	
	dp.Data_NextDword() = numLods;
	DP_WriteArray(dp,transitionDists);
	DP_WriteArray(dp,transitionPrecent);

	DP_WriteVector(dp,cfgwinds);						//6
	DWORD size = namewinds.size();
	dp.Data_NextDword() = size;
	for(int i = 0;i<size;i++)
		dp.Data_WriteString(namewinds[i]);

//	dp.Data_NextFloat() = fwVertBB;						//7
//	dp.Data_NextFloat() = fhVertBB;
//	dp.Data_NextFloat() = fwHorizBB;
//	dp.Data_NextFloat() = fhHorizBB;
//	dp.Data_NextFloat() = fhHorizHeight;
//	dp.Data_NextInt() =  nImages;
	
	dp.Data_WriteData(&aabb,sizeof(i_math::aabbox3df));		//8 

	DP_WriteVector(dp,capus);							//9
	DP_WriteVector(dp,sphs);
	DP_WriteVector(dp,obbs);

	dp.Data_WriteData(&leafboundSph,sizeof(leafboundSph)); //10

	DP_WriteVector(dp,uvBranch);	//11
	DP_WriteVector(dp,uvFrond);
	
	dp.Data_WriteData(&szBr,sizeof(szBr)); //12
	dp.Data_WriteData(&szFr,sizeof(szFr));
	dp.Data_NextFloat() = nPixel;
}

void SptData::Load(CDataPacket &dp)
{
	ver = dp.Data_NextDword();
	
	LoadHeader(dp);
	DP_ReadVector(dp,branchVB);							//	0
	DP_ReadVector(dp,frondVB);
	DP_ReadVector(dp,leafCardVB);
	DP_ReadVector(dp,leafMeshVB);
	DP_ReadVector(dp,sptIB);
	
	DP_ReadArray(dp,branchLODs);						// 1
	DP_ReadArray(dp,frondLODs);
	DP_ReadArray(dp,leafCardLODs);
	DP_ReadArray(dp,leafMeshLODs);

	dp.Data_ReadData(&fvfBranch,sizeof(fvfBranch));		//2
	dp.Data_ReadData(&fvfFrond,sizeof(fvfFrond));
	dp.Data_ReadData(&fvfLeafCard,sizeof(fvfLeafCard));
	dp.Data_ReadData(&fvfLeafMesh,sizeof(fvfLeafMesh));

	fLeafRockScale = dp.Data_NextFloat();				// 3
	fLeafRusltScale = dp.Data_NextFloat();

	numLeafCardLods = dp.Data_NextByte();				//4
	numBranchLods = dp.Data_NextByte();
	numFrondLods = dp.Data_NextByte();
	numLeafMeshLods = dp.Data_NextByte();

	dp.Data_ReadString(mapCompisiteDif);				//5
	dp.Data_ReadString(mapCompisiteNor);
	dp.Data_ReadString(mapBranchDif);
	dp.Data_ReadString(mapBranchNor);


	numLods = dp.Data_NextDword();
	DP_ReadArray(dp,transitionDists);
	DP_ReadArray(dp,transitionPrecent);

	DP_ReadVector(dp,cfgwinds);							//6
	DWORD size = dp.Data_NextDword();
	for(int i = 0;i<size;i++)
	{
		std::string s;
		dp.Data_ReadString(s);
		namewinds.push_back(s);
	}
	
	if(ver==1||ver==2){
		float fwVertBB, fhVertBB;
		float fwHorizBB, fhHorizBB;
		float fhHorizHeight;
		int nImages;
		fwVertBB = dp.Data_NextFloat();						//7
		fhVertBB = dp.Data_NextFloat();
		fwHorizBB = dp.Data_NextFloat();
		fhHorizBB = dp.Data_NextFloat();
		fhHorizHeight = dp.Data_NextFloat();
		nImages = dp.Data_NextInt();
	}

	
	if(ver==1){
		float texVertMap[4*MAX_BB_IMAGES];
		float texHorizMap[8];
		DP_ReadArray(dp,texVertMap);
		DP_ReadArray(dp,texHorizMap);
	}

	dp.Data_ReadData(&aabb,sizeof(i_math::aabbox3df));	//8

	DP_ReadVector(dp,capus);							//9
	DP_ReadVector(dp,sphs);
	DP_ReadVector(dp,obbs);

	dp.Data_ReadData(&leafboundSph,sizeof(leafboundSph));//10

	DP_ReadVector(dp,uvBranch);			//11
	DP_ReadVector(dp,uvFrond);

	dp.Data_ReadData(&szBr,sizeof(szBr));	//12
	dp.Data_ReadData(&szFr,sizeof(szFr));
	nPixel = dp.Data_NextFloat();
	
	if(ver<4)
		CalcBoundBox(this);
}

void CalcBoundBox(SptData * resData)
{
	i_math::aabbox3df abbTight;

	//branch
	DWORD stride = fvfSize(resData->fvfBranch);
	BYTE * pXYZ = &(resData->branchVB[0]) + fvfOffset(resData->fvfBranch,FVFEX_XYZW0);
	DWORD nVtx = resData->branchVB.size()/stride;
	for(int i = 0;i<nVtx;i++){
		abbTight.addInternalPoint(*((i_math::vector3df*)(pXYZ)));
		pXYZ += stride;
	}

	//frond
	stride = fvfSize(resData->fvfFrond);
	pXYZ = &(resData->frondVB[0]) + fvfOffset(resData->fvfFrond,FVFEX_XYZW0);
	nVtx = resData->frondVB.size()/stride;
	for(int i = 0;i<nVtx;i++){
		abbTight.addInternalPoint(*((i_math::vector3df*)(pXYZ)));
		pXYZ += stride;
	}

	//leafmesh
	stride = fvfSize(resData->fvfLeafMesh);
	pXYZ = &(resData->leafMeshVB[0]) + fvfOffset(resData->fvfLeafMesh,FVFEX_XYZW0);
	BYTE * pCZ  = &(resData->leafMeshVB[0]) + fvfOffset(resData->fvfLeafMesh,FVFEX_FLAG_QUX2) + 3*sizeof(float);
	BYTE * pCXY = &(resData->leafMeshVB[0]) + fvfOffset(resData->fvfLeafMesh,FVFEX_FLAG_TEX3);
	nVtx = resData->leafMeshVB.size()/stride;
	for(int i = 0;i<nVtx;i++){
		i_math::vector3df center;
		center.x = ((float*)pCXY)[0];
		center.y = ((float*)pCXY)[1];
		center.z = ((float*)pCZ)[0];
		i_math::vector3df pos = ((i_math::vector3df*)pXYZ)[0];
		abbTight.addInternalPoint(center+pos);
		pXYZ += stride;
		pCXY += stride;
		pCZ  += stride;
	}

	//leafcard
	stride = 4*fvfSize(resData->fvfLeafCard);
	pXYZ = &(resData->leafCardVB[0]) + fvfOffset(resData->fvfLeafCard,FVFEX_XYZW0);
	BYTE * pSZ = &(resData->leafCardVB[0]) + fvfOffset(resData->fvfLeafCard,FVFEX_FLAG_QUX1);
	nVtx = resData->leafCardVB.size()/stride;
	for(int i = 0;i<nVtx;i++){
		i_math::vector3df pos = ((i_math::vector3df *)pXYZ)[0];
		i_math::vector2df wh = ((i_math::vector2df*)pSZ)[0];
		float t = max(wh.x,wh.y);
		i_math::vector3df diag(t,t,t);
		i_math::aabbox3df abb;
		abb.MinEdge = pos - diag;
		abb.MaxEdge = pos + diag;
		abbTight.addInternalBox(abb);
	}

	resData->aabb = abbTight;
}

void SptData::SaveHeader(CDataPacket &dp)
{
}

void SptData::LoadHeader(CDataPacket &dp)
{
}

DWORD SptData::GetVtxNumberBr()
{
	int stride = fvfSize(fvfBranch);
	DWORD nVtx = (stride)?branchVB.size()/stride:0;
	return nVtx;
}

DWORD SptData::GetVtxNumberFr()
{
	int stride = fvfSize(fvfFrond);
	DWORD nVtx = (stride)?frondVB.size()/stride:0;
	return nVtx;
}

void SptData::CollectRefs(std::vector<std::string>&buf)
{
	if(!mapCompisiteDif.empty())
		buf.push_back(mapCompisiteDif);
	
	if(!mapCompisiteNor.empty())
		buf.push_back(mapCompisiteNor);
	
	if(!mapBranchDif.empty())
		buf.push_back(mapBranchDif);
	
	if(!mapBranchNor.empty())
		buf.push_back(mapBranchNor);
}


//////////////////////////////////////////////////////////////////////////
CSptLMUVGen::CSptLMUVGen()
{
	_gap = 0.01f;
	_szBr.set(0,0);
	_szFr.set(0,0);
}
void CSptLMUVGen::Gen(SptData * data,int nPixel,int validPixel,int gapPixel)
{
	_FetchVtxBr(data);
	_FetchVtxFr(data);
	
	assert(nPixel>0);

	//设置参数
	{
		_nPixel = float(nPixel);
		_validPixel = float(validPixel);
		_gap = float(gapPixel/_nPixel);
	}

	if(data->numBranchLods){
		DWORD s = data->branchLODs[0];
		DWORD e = data->branchLODs[1];
		_FetchIndex(data,s,e,_posVtxsBr.data(),_indicesBr);
	}

	if(data->numFrondLods){
		DWORD s = data->frondLODs[0];
		DWORD e = data->frondLODs[1];
		_FetchIndex(data,s,e,_posVtxsFr.data(),_indicesFr);
	}
	
	//划分 Branch 的连通组
	if(_posVtxsBr.size()) {
		_nGrpsBr = _ConstructSeg(_posVtxsBr,_indicesBr,_facesBr,_grpsBr,_vtxsBr);
		_CalcGeomLenProjUV(_grpsBr,_posVtxsBr,_norVtxsBr,_tanVtxsBr,_posUVsBr);
		_szBr = _GenerateUV(data,_posVtxsBr,_posUVsBr,_grpsBr,_vtxsBr,_indicesBr);
	}
	
	//划分Frond 的连通性
	if(_posVtxsFr.size()){
		_nGrpsFr = _ConstructSeg(_posVtxsFr,_indicesFr,_facesFr,_grpsFr,_vtxsFr);
		_CalcGeomLenProjUV(_grpsFr,_posVtxsFr,_norVtxsFr,_tanVtxsFr,_posUVsFr);
	 	_szFr = _GenerateUV(data,_posVtxsFr,_posUVsFr,_grpsFr,_vtxsFr,_indicesFr);
	}

	_DoChange(data);
}
float CSptLMUVGen::GetBranchArea()
{
	return _szBr.getArea();
}
float CSptLMUVGen::GetFrondArea()
{
	return _szFr.getArea();
}
void CSptLMUVGen::GetNumberOfSegments(DWORD & nGrpBr,DWORD & nGrpFr)
{
	nGrpBr = _nGrpsBr;
	nGrpFr = _nGrpsFr;
}
BOOL CSptLMUVGen::_GetGrpInfo(int * grp,int nVtx,std::vector<_Vtx> & vtxs)
{
	DWORD n = min(nVtx,vtxs.size());
	
	for(int i = 0;i<vtxs.size();i++){
		int idxGrp = _GrpID(vtxs,i);
		assert(idxGrp>=0);
		grp[i] = idxGrp;
	}

	return nVtx==vtxs.size();
}
BOOL CSptLMUVGen::GetGrpInfoBr(int * grp,int nVtx)
{	
	return _GetGrpInfo(grp,nVtx,_vtxsBr);
}
BOOL CSptLMUVGen::GetGrpInfoFr(int * grp,int nVtx)
{
	return _GetGrpInfo(grp,nVtx,_vtxsFr);
}
void CSptLMUVGen::_DoChange(SptData * data)
{
	DWORD stride = 0;
	BYTE * pUvs = NULL;

	//branch
	if(TRUE){
		stride = fvfSize(data->fvfBranch);
		pUvs = &(data->branchVB[0]) + fvfOffset(data->fvfBranch,FVFEX_FLAG_QUX0);
		for(int i = 0;i<_posUVsBr.size();i++){
			float * uv = (float *)(pUvs + 2*sizeof(float)); 
			uv[0] = _posUVsBr[i].x;
			uv[1] = _posUVsBr[i].y;
			pUvs += stride;
		}
	}

	if(TRUE){
		stride = fvfSize(data->fvfFrond);
		pUvs = &(data->frondVB[0]) + fvfOffset(data->fvfFrond,FVFEX_FLAG_QUX0);
		for(int i = 0;i<_posUVsFr.size();i++){
			float * uv = (float *)(pUvs + 2*sizeof(float)); 
			uv[0] = _posUVsFr[i].x;
			uv[1] = _posUVsFr[i].y;
			pUvs += stride;
		}
	}
	
	data->szBr = _szBr;
	data->szFr = _szFr;
	data->nPixel = float(_nPixel);
}

void CSptLMUVGen::_FetchIndex(SptData * data,DWORD s,DWORD e,i_math::vector3df * posVtxs,std::vector<WORD>&indices)
{
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

		indices.push_back(pSrc[0]);
		indices.push_back(pSrc[1]);
		indices.push_back(pSrc[2]);
		
		pSrc++;
	}
}

void CSptLMUVGen::_FetchVtxBr(SptData *data)
{
	int stride = fvfSize(data->fvfBranch);
	int nVtx = data->branchVB.size()/stride;

	int offVtx = fvfOffset(data->fvfBranch,FVFEX_XYZW0);
	int offNor = fvfOffset(data->fvfBranch,FVFEX_NORMAL0);
	int offTan = fvfOffset(data->fvfBranch,FVFEX_TANGENT);

	//得到顶点buffer的索引区
	BYTE * pVtx = (&(data->branchVB[offVtx]));
	BYTE * pNor = (&(data->branchVB[offNor]));
	BYTE * pTan = (&(data->branchVB[offTan]));

	_posVtxsBr.resize(nVtx);
	_posUVsBr.resize(nVtx);
	_norVtxsBr.resize(nVtx);
	_tanVtxsBr.resize(nVtx);

	assert(nVtx==data->uvBranch.size()) ;
	for(int i = 0;i<nVtx;i++)
		_posUVsBr[i] = data->uvBranch[i];

	//FVFEX_FLAG_QUX0  .zw  light map Flat uv
	for(int i = 0;i<nVtx;i++){
		_posVtxsBr[i] = *((i_math::vector3df *)pVtx);
		_norVtxsBr[i] = *((i_math::vector3df *)pNor);
		_tanVtxsBr[i] = *((i_math::vector3df *)pTan);
		pVtx += stride;
		pNor += stride;
		pTan += stride;
	}
}
void CSptLMUVGen::_FetchVtxFr(SptData *data)
{
	//没有展开的UV坐标
	if(!data->uvFrond.size())
		return;

	int stride = fvfSize(data->fvfFrond);
	int nVtx = data->frondVB.size()/stride;

	int offVtx = fvfOffset(data->fvfFrond,FVFEX_XYZW0);
	int offNor = fvfOffset(data->fvfFrond,FVFEX_NORMAL0);
	int offTan = fvfOffset(data->fvfFrond,FVFEX_TANGENT);

	//得到顶点buffer的索引区
	BYTE * pVtx = (&(data->frondVB[offVtx]));
	BYTE * pNor = (&(data->frondVB[offNor]));
	BYTE * pTan = (&(data->frondVB[offTan]));

	_posVtxsFr.resize(nVtx);
	_posUVsFr.resize(nVtx);
	_norVtxsFr.resize(nVtx);
	_tanVtxsFr.resize(nVtx);

	assert(nVtx==data->uvFrond.size()) ;
	for(int i = 0;i<nVtx;i++)
		_posUVsFr[i] = data->uvFrond[i];

	//FVFEX_FLAG_QUX0  .zw  light map Flat uv
	for(int i = 0;i<nVtx;i++){
		_posVtxsFr[i] = *((i_math::vector3df *)pVtx);
		_norVtxsFr[i] = *((i_math::vector3df *)pNor);
		_tanVtxsFr[i] = *((i_math::vector3df *)pTan);
		pVtx += stride;
		pNor += stride;
		pTan += stride;
	}
}

struct _ElemUv
{
	_ElemUv(float v,float s,int i){
		uv = v;
		idx = i;
		sec = s;
	}
	bool operator <(const _ElemUv &oth) const {
		return (uv<oth.uv||(uv==oth.uv&&sec>oth.sec));
	}
	float uv,sec;  // 该点的uv值
	int   idx; // 对应的顶点索引
};

void CSptLMUVGen::_CalcGeomLenProjUV( std::vector<_GrpUV>&grps,
						std::vector<i_math::vector3df> &posVtxs,
						std::vector<i_math::vector3df> &norVtxs,
						std::vector<i_math::vector3df> &tanVtxs,
						std::vector<i_math::vector2df> &uvVtxs)
{
	
	std::vector<_ElemUv> vVals;
	std::vector<_ElemUv> uVals;

	for(int i = 0;i<grps.size();i++){

		_GrpUV &grp = grps[i];
		
		uVals.clear();
		vVals.clear();

		for(int j = 0;j<grp.idxs.size();j++){
			int idx = grp.idxs[j];
			i_math::vector2df & uv = uvVtxs[idx];
			uVals.push_back(_ElemUv(uv.u,uv.v,idx));
			vVals.push_back(_ElemUv(uv.v,uv.u,idx));
		}

		std::sort(uVals.begin(),uVals.end());
		std::sort(vVals.begin(),vVals.end());
		
		float uLen = 0;
		float vLen = 0;

		//得到u方向的长度
		float r = -1.0f;
		for(int k = 1;k<vVals.size();k++){
			_ElemUv & v0 = vVals[k-1];
			_ElemUv & v1 = vVals[k];
			i_math::vector3df grad = posVtxs[v1.idx] - posVtxs[v0.idx];
			uLen += abs(grad.dotProduct(tanVtxs[v0.idx]));
			r = v0.uv;
		}
		
		//得到u方向的长度
		r = -1.0f;
		for(int k = 1;k<uVals.size();k++){
			_ElemUv & v0 = uVals[k-1];
			_ElemUv & v1 = uVals[k];
			i_math::vector3df biNor = norVtxs[v0.idx].crossProduct(tanVtxs[v0.idx]);
			biNor.normalize();
			i_math::vector3df grad = posVtxs[v1.idx] - posVtxs[v0.idx];
			vLen += abs(grad.dotProduct(biNor));
			r = v0.uv;
		}
		
		r = vLen/uLen;

		grp.ulen = uLen;
		grp.vlen = vLen;
	}
}

BOOL CSptLMUVGen::_CheckFace(int * indices,int r)
{
	if(indices[0]==indices[1]&&(abs(indices[1]-indices[2])>r||abs(indices[0]-indices[2])>r))
		return FALSE;

	if(indices[1]==indices[2]&&(abs(indices[1]-indices[0])>r||abs(indices[2]-indices[0])>r))
		return FALSE;

	if(indices[0]==indices[2]&&(abs(indices[1]-indices[0])>r||abs(indices[1]-indices[2])>r))
		return FALSE;

	return TRUE;
}

i_math::size2df CSptLMUVGen::_GenerateUV(SptData * data,std::vector<i_math::vector3df>&posVtxs,
							  std::vector<i_math::vector2df>&uvVtxs,std::vector<_GrpUV> &grps,
							  std::vector<_Vtx> & vtxs,std::vector<WORD>& indices)
{
	DWORD nIndexs = indices.size();
	WORD * pIndices = indices.data();
	
	i_math::size2df szMap(0,0);

	if(nIndexs<3)
		return szMap;

	if(!data||(0==vtxs.size()))
		return szMap;
	
//	对每个分组进行计算,得到面积的因子
	for(int i = 0;i<nIndexs;i+=3){
		int indices[] = {pIndices[i+0],pIndices[i+1],pIndices[i+2]};
		if(!_CheckFace(indices,1))
			continue;
		i_math::vector3df tri[] = { posVtxs[indices[0]],
									posVtxs[indices[1]],
									posVtxs[indices[2]]};
		int idxGrp = _GrpID(vtxs,indices[0]);
		float area = _AreaTri(tri);
		grps[idxGrp].w += area;
	}

	float scale = 0.1f;

	for(int i = 0;i<grps.size();i++){
		_GrpUV &grp = grps[i];
		grp.w = grp.ulen*grp.vlen*scale*scale;
	}
		
	//计算原UV的分布
	i_math::vector2df uv;
	for(int i = 0;i<vtxs.size();i++){
		_Vtx & v = vtxs[i];
		int grpID = _GrpID(vtxs,i);
		//确保已经分组
		assert(grpID>=0);
		_GrpUV &grp = grps[grpID];
		uv = uvVtxs[i];

		//加入uv
		grp.rcSrc.UpperLeftCorner.x  = min(uv.x,grp.rcSrc.UpperLeftCorner.x);
		grp.rcSrc.LowerRightCorner.x = max(uv.x,grp.rcSrc.LowerRightCorner.x);
		grp.rcSrc.UpperLeftCorner.y  = min(uv.y,grp.rcSrc.UpperLeftCorner.y);
		grp.rcSrc.LowerRightCorner.y = max(uv.y,grp.rcSrc.LowerRightCorner.y);
	}
	
	//得到权重值
	float wTotal = 0;
	for(int i = 0;i<grps.size();i++)
		wTotal += grps[i].w;

	//根据面积因子预算变换后的UV分布
	for(int i = 0;i<grps.size();i++)
	{
		_GrpUV &grp = grps[i]; 
		float a0 = grp.rcSrc.getArea();
		if(a0==0){
			grp.w = -1.0f;
			grp.rcDst.set(0,0,0,0);
			continue;
		}

		grp.rcDst.set(0,0,0,0);
		if(grp.vlen<=0||grp.ulen<=0)
			continue;
		
		float w = grp.ulen*scale;
		float h = grp.vlen*scale;

		//检测宽度是否太小
		if(w*_nPixel<_validPixel)
			w = _validPixel/_nPixel;
		
		//检测高度是否太小
		if(h*_nPixel<_validPixel)
			h = _validPixel/_nPixel;

		grp.rcDst.UpperLeftCorner.set(0,0);
		grp.rcDst.LowerRightCorner.x =	w;
		grp.rcDst.LowerRightCorner.y = h;
	}
	
	//将uv分布平铺在矩形区域 得到最终的矩形区域
	i_math::rectf rcTotal = _PackageQuad(grps);

	//计算变换矩阵 rcSrc --> rcDst
	float ku,kv,bu,bv;
	for (int i = 0;i<grps.size();i++){
		_GrpUV & v = grps[i];

		//该段面积为零
		if(v.rcSrc.getArea()==0){
			v.kb[0].set(0,0.0f);
			v.kb[1].set(0,0.0f);
			continue;
		}

		ku = v.rcDst.getWidth()/v.rcSrc.getWidth();
		kv = v.rcDst.getHeight()/v.rcSrc.getHeight();
		bu = v.rcDst.UpperLeftCorner.x - ku*v.rcSrc.UpperLeftCorner.x;
		bv = v.rcDst.UpperLeftCorner.y - kv*v.rcSrc.UpperLeftCorner.y;
		// y = k(x - b) kx ,ky 分别为u v 方向的系数
		v.kb[0].set(ku,bu);
		v.kb[1].set(kv,bv);
	}

	//更新顶点的UV 
	for(int i = 0;i<grps.size();i++){
		_GrpUV &grp = grps[i];
		//调整改组内所有的顶点
		for(int j = 0;j<grp.idxs.size();j++){
			int idx = grp.idxs[j];
			assert(idx<posVtxs.size());
			i_math::vector2df & uv = uvVtxs[idx];

			//将顶点的UV 变换到 调整后的区域
			uv.x = grp.kb[0].x * uv.x + grp.kb[0].y;
			uv.y = grp.kb[1].x * uv.y + grp.kb[1].y;
		}
	}
	
	szMap.w = rcTotal.getWidth();
	szMap.h = rcTotal.getHeight();
	
	szMap.w *= _nPixel;
	szMap.h *= _nPixel;

	if(szMap.w<1.0f)
		szMap.w = 1.0f;
	
	if(szMap.h<1.0f)
		szMap.h = 1.0f;

	return szMap;
}

i_math::rectf CSptLMUVGen::_PackageQuad(std::vector<_GrpUV> &grps)
{
	//按照面积进行排序
	std::sort(grps.begin(),grps.end());

	//找到最大面积的一个
	int iP = 0;
	for(;iP<grps.size()&&grps[iP].w==0;iP++);

	i_math::pos2df org(0,0);
	i_math::rectf rcTotal; //合并后的矩形
	i_math::rectf rcCut;   //两个矩形拼接后余下的区域

	_GrpUV & v = grps[iP];
	v.rcDst.UpperLeftCorner.x  += _gap;
	v.rcDst.UpperLeftCorner.y  += _gap;
	v.rcDst.LowerRightCorner.x += _gap;
	v.rcDst.LowerRightCorner.y += _gap;
	
	rcTotal = v.rcDst;

	//种子矩形，已被合并标记为(w=-1.0f)
	v.w = -1.0f;

	_TotalEdge edges;
	edges.leftEdges.push_back(iP);
	edges.bottomEdges.push_back(iP);

	//合并
	while(true){

		//矩形碎片列表，试图在碎边中放入矩形
		std::vector<_Segment> rcSegments;

		int k = _FindBestQuad(grps,rcTotal,edges,rcSegments);
		//不能找到一个
		if(k<0)
			break;

		while(!rcSegments.empty()){	
			_Segment  rcSeg = rcSegments.back();
			rcSegments.pop_back(); //处理完一个碎片

			//放入一个能装下的矩形,如果不能被任何的矩形填充则采用延伸 原有的矩形把它消耗掉
			if(_FindQuadIn(grps,rcSeg.rcSeg,rcSegments)<0){
				int extType = rcSeg.extType;
				int idx = -1;
				for(int i = 0;i<rcSeg.idxExts.size();i++){
					idx = rcSeg.idxExts[i];
					assert(idx<grps.size());
					_GrpUV & grp = grps[idx];
					if(0==extType){// 向左延伸
						grp.rcDst.LowerRightCorner.x = rcSeg.rcSeg.LowerRightCorner.x;
					}
					if(1==extType){ //向下延伸
						grp.rcDst.LowerRightCorner.y = rcSeg.rcSeg.LowerRightCorner.y;
					}
				}
			}
			// for next segment
		}
	}

	if(rcTotal.getArea()<=0)
		return rcTotal;
	
	//充满间隙
	rcTotal.UpperLeftCorner.set(0,0);
	
	float w = rcTotal.getWidth();
	float h = rcTotal.getHeight();
	//调整长宽比过大
	if(h/w>2.0f){
		float r = 2.0f*w/h;
		rcTotal.LowerRightCorner.y = rcTotal.UpperLeftCorner.y + 2.0f*w;
		for (int i = 0;i<grps.size();i++) {
			_GrpUV & v = grps[i];
			v.rcDst.UpperLeftCorner.y  *= r;
			v.rcDst.LowerRightCorner.y *= r;
		}
	}

	float sw = 1.0f/rcTotal.getWidth();
	float sh = 1.0f/rcTotal.getHeight();
	//将所有的矩形缩放到边长为一的区域
	for (int i = 0;i<grps.size();i++) {
		_GrpUV & v = grps[i];

		v.rcDst.UpperLeftCorner.x  *= sw;
		v.rcDst.LowerRightCorner.x *= sw;
		v.rcDst.UpperLeftCorner.y  *= sh;
		v.rcDst.LowerRightCorner.y *= sh;
	}
	
	return rcTotal;
}

int CSptLMUVGen::_FindQuadIn(std::vector<_GrpUV> &grps,i_math::rectf &rcCut,
							 std::vector<_Segment> &rcSegments)
{
	float w = rcCut.getWidth();
	float h = rcCut.getHeight();

	int iK = -1;
	float areaCut = rcCut.getArea();
	float areaMin = areaCut;

	for(int i = 0;i<grps.size();i++){
		_GrpUV & v = grps[i];
		//将无效的或已经合并了矩形忽略掉
		if(v.w<=0)
			continue;

		//当前矩形太大
		if(v.rcDst.getWidth()>w||v.rcDst.getHeight()>h)
			continue;

		//找到一个余下的面积最小的那一个
		float a = areaCut - v.rcDst.getArea();
		if(a<areaMin){
			areaMin = a;
			iK = i;
		}
	}

	if(iK<0)
		return -1;

	_GrpUV &kv = grps[iK];
	float w0 = kv.rcDst.getWidth();
	float h0 = kv.rcDst.getHeight();

	//停靠矩形
	if(TRUE){
		kv.rcDst.UpperLeftCorner.x  = rcCut.UpperLeftCorner.x;
		kv.rcDst.UpperLeftCorner.y  = rcCut.UpperLeftCorner.y;
		kv.rcDst.LowerRightCorner.x = rcCut.UpperLeftCorner.x + w0;
		kv.rcDst.LowerRightCorner.y = rcCut.UpperLeftCorner.y + h0;
	}

	//标记该矩形区域已经被停靠
	kv.w = -1.0f;

	float gap = _gap;

	//将余下的碎片加入到列表中
	i_math::rectf rcSeg;
	if(TRUE){
		if(rcCut.getWidth()>w0+gap){
			rcSeg.UpperLeftCorner.x  = rcCut.UpperLeftCorner.x + w0 + gap;
			rcSeg.UpperLeftCorner.y  = rcCut.UpperLeftCorner.y;
			rcSeg.LowerRightCorner.x = rcCut.LowerRightCorner.x;
			rcSeg.LowerRightCorner.y = rcCut.UpperLeftCorner.y + h0;
			rcSegments.push_back(_Segment(rcSeg,0)); //向左延伸
			rcSegments.back().idxExts.push_back(iK);
		}
		if(rcCut.getHeight()>h0+gap){
			rcSeg.UpperLeftCorner.x  = rcCut.UpperLeftCorner.x;
			rcSeg.UpperLeftCorner.y  = rcCut.UpperLeftCorner.y + h0 + gap;
			rcSeg.LowerRightCorner.x = rcCut.LowerRightCorner.x;
			rcSeg.LowerRightCorner.y = rcCut.LowerRightCorner.y;
			rcSegments.push_back(_Segment(rcSeg,1)); //向下延伸
			rcSegments.back().idxExts.push_back(iK);
		}
	}

	return iK;
}
int CSptLMUVGen::_FindBestQuad(std::vector<_GrpUV> &grps,i_math::rectf & rcFix,_TotalEdge & edges,std::vector<_Segment> &rcSegments)
{
	int bLeft = false;
	int iK = -1;
	float minBais = 9999999.9f; 
	
	//已经添加的矩形区域
	float h0 = rcFix.getHeight();
	float w0 = rcFix.getWidth();

	float h1 = 0,w1 = 0;
	//被截后剩下的部分信息
	i_math::rectf rcCut;
	std::vector<int> idxExts;
	int extType = -1;
	
	//得到最好的矩形和停靠方法
	for(int i = 0;i<grps.size();i++){
		_GrpUV & v = grps[i];
		//已经被合并
		if(v.w<=0)
			continue;

		h1 = v.rcDst.getHeight();
		w1 = v.rcDst.getWidth();

		//停靠在左侧
		float a0 = (h0>h1)?(h0-h1)*w1:(h1-h0)*w0;
		float rw = (h0>h1)?((w0+w1)/h0):((w0+w1)/h1);  //形状代价
		a0 = a0*rw;

		//停靠在下侧
		float a1 = (w0>w1)?(w0-w1)*h1:(w1-w0)*h0;
		float rh = (w0>w1)?((h0+h1))/w0:((h0+h1)/w1);  //形状代价
		a1 = a1*rh;

		assert(a0<9999999.9f&&a1<9999999.9f);

		//停靠在左侧最小
		if(a0<minBais){
			bLeft = true;
			minBais = a0;
			iK = i;
		}

		//停靠在右侧最小
		if(a1<minBais){
			bLeft = false;
			minBais =   a1;
			iK = i;
		}
	}

	//没有了
	if(iK<0)
		return -1;

	_GrpUV & kv = grps[iK];

	w1 = kv.rcDst.getWidth();
	h1 = kv.rcDst.getHeight();

	float gap = _gap;
	
	BOOL bAlmostFix = FALSE;
	//得到余下的矩形区域
	if(bLeft){
		bAlmostFix = (abs(h0-h1)<=_gap);
		if(!bAlmostFix){
			if(h0>h1){
				rcCut.UpperLeftCorner.x = rcFix.LowerRightCorner.x + gap;
				rcCut.UpperLeftCorner.y = rcFix.UpperLeftCorner.y + h1 + gap;
				rcCut.LowerRightCorner.x = rcFix.LowerRightCorner.x + w1 + gap;
				rcCut.LowerRightCorner.y = rcFix.LowerRightCorner.y;

				idxExts.push_back(iK);
				extType = 1; //向下延伸
			}
			else
			{
				rcCut.UpperLeftCorner.x  = rcFix.UpperLeftCorner.x;
				rcCut.UpperLeftCorner.y  = rcFix.LowerRightCorner.y + gap;
				rcCut.LowerRightCorner.x = rcFix.LowerRightCorner.x;
				rcCut.LowerRightCorner.y = rcFix.UpperLeftCorner.y + h1;

				for(int i = 0;i<edges.bottomEdges.size();i++)
					idxExts.push_back(edges.bottomEdges[i]);
				extType = 1; //向下延伸
			}
		}
		else{
			rcCut.set(0,0,0,0);
		}
		
		//更新左边沿 rcFix
		edges.leftEdges.clear();
		edges.leftEdges.push_back(iK);
	}
	else{
		bAlmostFix = (abs(w0-w1)<=_gap);
		if(!bAlmostFix){
			if(w0>w1){
				rcCut.UpperLeftCorner.x  = rcFix.UpperLeftCorner.x + w1 + gap;
				rcCut.UpperLeftCorner.y  = rcFix.LowerRightCorner.y + gap;
				rcCut.LowerRightCorner.x = rcFix.LowerRightCorner.x;
				rcCut.LowerRightCorner.y = rcFix.LowerRightCorner.y + h1 + gap;

				idxExts.push_back(iK);
				extType = 0; //向左延伸
			}
			else{
				rcCut.UpperLeftCorner.x  = rcFix.LowerRightCorner.x + gap;
				rcCut.UpperLeftCorner.y  = rcFix.UpperLeftCorner.y;
				rcCut.LowerRightCorner.x = rcFix.UpperLeftCorner.x + w1 + gap;
				rcCut.LowerRightCorner.y = rcFix.LowerRightCorner.y;

				/************************************************************************/
				/*	取出左边缘的所有矩形区域，当拼凑后余下的矩形碎片不能容纳任何的矩形时，
				/*  将所有的边缘矩形向左拉长使其将碎片区域填满  
				/************************************************************************/
				for(int i = 0;i<edges.leftEdges.size();i++) 
					idxExts.push_back(edges.leftEdges[i]);

				extType = 0;  //向左延伸
			}
		}
		else{
			rcCut.set(0,0,0,0);
		}
		//更新下边沿
		edges.bottomEdges.clear();
		edges.bottomEdges.push_back(iK);
	}

	//将选中的那个矩形停靠在 大的矩形上
	if(bLeft){ //停靠在左侧
		kv.rcDst.UpperLeftCorner.x  = rcFix.LowerRightCorner.x + gap;
		kv.rcDst.UpperLeftCorner.y  = rcFix.UpperLeftCorner.y;
		kv.rcDst.LowerRightCorner.x = kv.rcDst.UpperLeftCorner.x + w1;
		kv.rcDst.LowerRightCorner.y = kv.rcDst.UpperLeftCorner.y + h1;
	}
	else{
		kv.rcDst.UpperLeftCorner.x  = rcFix.UpperLeftCorner.x;
		kv.rcDst.UpperLeftCorner.y  = rcFix.LowerRightCorner.y + gap;
		kv.rcDst.LowerRightCorner.x = kv.rcDst.UpperLeftCorner.x + w1;
		kv.rcDst.LowerRightCorner.y = kv.rcDst.UpperLeftCorner.y + h1;
	}

	//标记该矩形已经被合并
	kv.w = -1.0f;

	//扩充大矩形使其将新的停靠矩形包括在其中
	if(bLeft){
		rcFix.LowerRightCorner.x = rcFix.LowerRightCorner.x + w1 +gap;
		if(h0<=h1)
			rcFix.LowerRightCorner.y = rcFix.UpperLeftCorner.y + h1;
	}
	else{
		rcFix.LowerRightCorner.y = rcFix.LowerRightCorner.y + h1 + gap;
		if(w0<=w1)
			rcFix.LowerRightCorner.x = rcFix.UpperLeftCorner.x + w1;
	}

	//不正常的区域在此诞生
	if(FALSE){
		assert(rcFix.isRectInside(rcCut));
		assert(rcFix.isRectInside(kv.rcDst));
	}

	/************************************************************************/
	/* 余下的部分加入到碎片列表
	/* 记录碎片不能再加入任何矩形时如何去延伸
	/************************************************************************/
	if(!bAlmostFix){
		rcSegments.push_back(_Segment(rcCut,extType));
		idxExts.swap(rcSegments.back().idxExts); 
	}

	return  iK;
}
int CSptLMUVGen::_GrpID(std::vector<_Vtx> & vtxs,int idx)
{
	int idxGrp = -1;

	_Vtx & v = vtxs[idx];

	if(v.idxRef<0)
		idxGrp = v.grp;
	else{
		_Vtx & vRef = vtxs[v.idxRef];
		assert(vRef.idxRef<0);
		idxGrp = vRef.grp;
	}

	return idxGrp;
}
float CSptLMUVGen::_AreaTri(i_math::vector3df * pos)
{
	float a = (float)pos[0].getDistanceFrom(pos[1]);
	float b = (float)pos[0].getDistanceFrom(pos[2]);
	float c = (float)pos[1].getDistanceFrom(pos[2]);
	float p = (a+b+c)/2;
	float area = sqrtf(p*(p-a)*(p-b)*(p-c));
	return area;
}

int CSptLMUVGen::_ConstructSeg(std::vector<i_math::vector3df> & posVtxs,
								std::vector<WORD> & indices,std::vector<_Face> &faces,
								std::vector<_GrpUV> &grps,std::vector<_Vtx> & vtxs)
{
	DWORD nIndexs = indices.size();
	DWORD nVtxs = posVtxs.size();

	if(nIndexs<3||nVtxs<1)
		return 0;
	
	//每个顶点有唯一的位置
	std::map<_UniVtx,int>  uniVtxs; 
	
	faces.clear();
	vtxs.clear();

	faces.resize(nIndexs/3);
	vtxs.resize(nVtxs);

	WORD * pIndices = &(indices[0]);
	i_math::vector3df * pVtx = &(posVtxs[0]);

	//初始化数据
	int iFace = 0;
	std::map<_UniVtx,int>::iterator itMap;
	for(int i = 0;i<nIndexs;i+=3){

		int indices[] = {pIndices[i+0],pIndices[i+1],pIndices[i+2]};
		int idxUni[3] = {-1};

		if(!_CheckFace(indices,1))
			continue;

		//找到位置相同点的位置
		for(int j = 0;j<3;j++){

			int idx = indices[j];
			i_math::vector3df pos = pVtx[idx];
			itMap = uniVtxs.find(_UniVtx(pos));

			if(itMap==uniVtxs.end()){
				//新的位置
				uniVtxs[_UniVtx(pos)] = idx;
				vtxs[idx].idxRef = -1;
			}
			else{
				//引用已有的顶点
				int refIdx = itMap->second;
				if(idx!=refIdx)
					vtxs[idx].idxRef = refIdx;

				assert(vtxs[refIdx].idxRef<0);
				idx = refIdx;
			}

			idxUni[j] = idx;
		}

		//添加该信息
		for(int j = 0;j<3;j++)
		{
			int idx = idxUni[j];
			assert(vtxs[idx].idxRef<0);
			faces[iFace].vtx[j] = idx;

			if(idx==200){
				int c = 0;
				c++;
				if(idxUni[(j+1)%3]==426||idxUni[(j+2)%3]==426)
				{
					int c = 0;
					c++;
				}
			}
			vtxs[idx].addFace(iFace,idxUni[(j+1)%3],idxUni[(j+2)%3]);
		}

		iFace++;
	}

	int grp = 0;
	int iP = 0;
	std::vector<int> stack;

	while(iP<vtxs.size()){	

		//找到第一个边界点开始连通性测试
		vtxs[iP].grp = grp;
		stack.push_back(iP);

		while(!stack.empty()){
			//从栈中取出一个元素开始归类
			int iCur = stack.back();
			_Vtx & curVtx = vtxs[iCur];
			stack.pop_back();
			int grpParent = curVtx.grp;

			assert(curVtx.idxRef<0);

			for(int i = 0;i<curVtx.nearVtxs.size();i++){
				int idxNear = curVtx.nearVtxs[i].idx;
				_Vtx & nearVtx = vtxs[idxNear];
				assert(nearVtx.idxRef<0);
				//该节点已经被规类
				if(nearVtx.grp>=0){
					assert(nearVtx.grp==grp);
					continue; 
				}
				nearVtx.grp = grpParent;
				stack.push_back(idxNear);
			}
		}

		grp++;
		//一个连通体完成开始新的一个连通体,找到第一个没有被规类的顶点
		iP++;
		for(;iP<vtxs.size();iP++){
			//未归类 且不为 引用点
			if(vtxs[iP].grp<0&&vtxs[iP].idxRef<0)
				break;
		}
	}
	
	grps.resize(grp);

	//按grp值分组
	for(int i = 0;i<vtxs.size();i++){
		int grpID = _GrpID(vtxs,i);
		assert(grpID>=0&&grpID<grp);
		grps[grpID].idxs.push_back(i);
	}

	return grp;
}


