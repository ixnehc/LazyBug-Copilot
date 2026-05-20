/************************************************************************/
/*  author:star
	date: 2008-01-10
	pupose: draw a sekeleton model
*/
/************************************************************************/
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"

#include "SKeletonModel.h"


#define Vector_Set(vec,value)                         \
	for(int i=0;i<vec.size();i++)					 \
		 vec[i]=value;								  \

BOOL DrawBone(IRenderPort *rp,i_math::matrix43f matParent,i_math::matrix43f matLocal,i_math::vector3df start,i_math::vector3df end,/*out*/i_math::vector3df *tri,DWORD * color,float scaleSJ=0.2f,DWORD col=0xff00ff00,BOOL bDraw=TRUE,BOOL bEnd=FALSE);
DWORD CSkeletonModel::_Color(DWORD state)
{
	if(state==State_Normal) return _colorNormal;
	if(state==State_Disable) return _colorDisable;
	if(state==State_Selected) return _colorSel;
	return  _colorNormal;
}
void CSkeletonModel::Create(SkeletonInfo &skeleton,DWORD colorSel,DWORD colorNormal,DWORD colorDisable,matrix43f *matOffset,float scaleSJ)
{	
	_matInit=matOffset;
	_skeletonInfo=skeleton;
	_buffer.resize(24*skeleton.size());
	_color.resize(24*skeleton.size());
	_state.resize(24*skeleton.size());
	_posBone.resize(_skeletonInfo.size()+1);

	_colorSel=colorSel;
	_colorNormal=colorNormal;
	_colorDisable=colorDisable;
	_scaleSJ=scaleSJ;
	Vector_Set(_state,State_Normal);
}

void CSkeletonModel::_CalculateBoneDir(int root)					
{															
		i_math::vector3df norm(0.0f,0.0f,0.0f);				
		i_math::matrix43f dir= _skeletonInfo[root].xformDef.getMatrix();
		dir.transformVect(norm,norm);					
		norm.normalize();											
		norm.setLength(0.3f);										
		_posBone[_skeletonInfo.size()]=_posBone[root]-norm;				
}																	
void  CSkeletonModel::_Init(IRenderPort *rp)
{	  
	if(!rp) return;
	extern BOOL GetBoneDefMatrix(i_math::matrix43f &matWorld,SkeletonInfo &skeleton,int idx,i_math::matrix43f * matOffset=NULL);
	memset(_posBone.data(),0,_posBone.size()*sizeof(i_math::vector3df));
	
	std::vector<i_math::matrix43f> matBones;
	matBones.resize(_skeletonInfo.size()+1);

	i_math::matrix43f * matDefs = NULL;
	_skeletonInfo.GetDefMatrix(matDefs);

	for(int i=0;i<_skeletonInfo.size();i++)
	{
		matBones[i] = matDefs[i];
		if(_matInit)
			matBones[i] *=(*_matInit);
		matBones[i].transformVect(_posBone[i],_posBone[i]);
	}
	delete []matDefs;

	matBones[_skeletonInfo.size()].makeIdentity();

	DWORD color;
	i_math::matrix43f  matLocal;
	BOOL  bEnd;
	for(int i=0;i<_skeletonInfo.size();i++)
	{	
		bEnd=FALSE;
		matLocal=_skeletonInfo[i].xformDef.getMatrix();
		int parent=_skeletonInfo[i].iParent;
		if(parent==-1) 
		{   
			_CalculateBoneDir(i);
			parent=_skeletonInfo.size();
			bEnd=TRUE;
		}
		color=_Color(_state[i]);
		DrawBone(rp,matBones[parent],matLocal,_posBone[parent],_posBone[i],&_buffer[24*i],&_color[24*i],_scaleSJ,color,FALSE,bEnd); //not draw indeed,but get the buffer
	}
}
void CSkeletonModel::Update(IRenderPort *rp)
{
	_Init(rp);
}
void CSkeletonModel::Update(IRenderPort *rp,i_math::matrix43f *matsKey)
{
	if(!rp) return;
	BOOL GetBoneWorldMatrix(i_math::matrix43f &matWorld,SkeletonInfo &skeleton,int idx,i_math::matrix43f * matsKey, i_math::matrix43f * matOffset=NULL);
	DWORD color;

	std::vector<i_math::matrix43f> matBones;
	matBones.resize(_skeletonInfo.size()+1);
	for(int i=0;i<_skeletonInfo.size();i++)
	{
		matBones[i].makeIdentity();
		_posBone[i].set(0.0f,0.0f,0.0f);
		GetBoneWorldMatrix(matBones[i],_skeletonInfo,i,matsKey,_matInit);
		matBones[i].transformVect(_posBone[i],_posBone[i]);
	}
	matBones[_skeletonInfo.size()].makeIdentity();

	BOOL  bEnd;
	i_math::matrix43f matLocal;
	for(int i=0;i<_skeletonInfo.size();i++)
	{
		bEnd=FALSE;
		int parent=_skeletonInfo[i].iParent;
		if(parent==-1) 
		{
			_CalculateBoneDir(i); 			  //the pos of every pos.
			parent=_skeletonInfo.size();
			bEnd=TRUE;
		}
		color=_Color(_state[i]);
		matLocal=_skeletonInfo[i].xformDef.getMatrix();
		DrawBone(rp,matBones[parent],matLocal,_posBone[parent],_posBone[i],&_buffer[24*i],&_color[24*i],_scaleSJ,color,FALSE,bEnd); //not draw indeed,but get the buffer
	}
}
void CSkeletonModel::Draw(IRenderPort *rp,i_math::matrix43f *matsKey/*=NULL*/)
{
	if(rp)
	{	
		if(matsKey)
			Update(rp,matsKey);
		else{
			_Init(rp);
		}
		rp->Triangles(_buffer.data(),_buffer.size()/3,_color.data());
	}
}

void CSkeletonModel::_HitTest(IRenderPort * rp,int x,int y,std::vector<DWORD> &itemSels)
{
	HitProbe  hitProc;
	rp->CalcHitProbe(x,y,hitProc);
	i_math::vector3df intersectionPoint;
	i_math::triangle3df triangle;
	itemSels.clear();
	for(int i=0;i<_buffer.size();i+=3)
	{
		triangle.set(_buffer[i],_buffer[i+1],_buffer[i+2]);
		if(triangle.getIntersectionWithLimitedLine(hitProc,intersectionPoint))
		itemSels.push_back(i/24);
	}
}
void CSkeletonModel::SetNormalColor(DWORD color)
{
	_colorNormal=color;
}
void CSkeletonModel::SetSelColor(DWORD color)
{
	_colorSel=color;
}
void CSkeletonModel::SelectByPos(IRenderPort * rp,int x,int y)
{
	std::vector<DWORD> sels;
	_HitTest(rp,x,y,sels);
	for(int i=0;i<sels.size();i++)
		_state[sels[i]]=State_Selected;
}
void CSkeletonModel::SelectByName(const char * nameBone)
{
	int idx=_FindByName(nameBone);
	if(idx<0) return ;
	_state[idx]=State_Selected;
}
void CSkeletonModel::UnSelectByName(const char * nameBone)
{
	int idx=_FindByName(nameBone);
	if(idx<0) return ;
	_state[idx]=State_Normal;
}
int CSkeletonModel::_FindByName(const char *name)
{
	for(int i=0;i<_skeletonInfo.size();i++)
		if(!strcmp(_skeletonInfo[i].name,name))
			return i;
	return -1;
}
void CSkeletonModel::UnSelectByPos(IRenderPort * rp,int x,int y)
{
	std::vector<DWORD> sels;
	_HitTest(rp,x,y,sels);
	for(int i=0;i<sels.size();i++)
		_state[sels[i]]=FALSE;	
}
void CSkeletonModel::ResetState(DWORD  state)
{
	for(int i=0;i<_state.size();i++)
		_state[i]=state;
}
void CSkeletonModel::GetSelectedBones(std::vector<DWORD> bonesSel)
{
	for(int i=0;i<_state.size();i++)
		if(_state[i]==State_Selected)  bonesSel.push_back(i);
}
int CSkeletonModel::GetBoneCount()
{
	return _skeletonInfo.size();
}
void CSkeletonModel::SetState(DWORD  idx,DWORD state)
{
	if(idx>=0&&idx<_skeletonInfo.size())
	 _state[idx]=state;
}
