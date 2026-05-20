#pragma  once
#include "../Interfaces/RenderSystem/IRenderSystem.h"

#include "resdata/MeshData.h"

#include "anim/KeySet.h"

class CSkeletonModel
{
public:
	enum BoneState
	{
		State_Normal,
		State_Selected,
		State_Disable,
	};
	void Create(SkeletonInfo &skeleton,DWORD colorSel=0xffff0000,DWORD colorNormal=0xff00ff00,DWORD colorDisable=0xff888888,matrix43f *matOffset=NULL,float scaleSJ=0.2f);
	void Update(IRenderPort *rp,i_math::matrix43f *matsKey);
	void Update(IRenderPort *rp);
	void Draw(IRenderPort *rp,i_math::matrix43f *matsKey=NULL);
	void SelectByPos(IRenderPort * rp,int x,int y);
	void SelectByName(const char * nameBone);
	void UnSelectByName(const char * nameBone);
	void UnSelectByPos(IRenderPort * rp,int x,int y);
	void ResetState(DWORD  state);
	void GetSelectedBones(std::vector<DWORD> bonesSel); 
	void SetNormalColor(DWORD color);
	void SetSelColor(DWORD color);
	void SetState(DWORD  idx,DWORD state);
	int GetBoneCount();
	void SetColor(DWORD color){_colorNormal = color;}
	DWORD GetColor(){return _colorNormal;}
protected:
	void _HitTest(IRenderPort * rp,int x,int y,std::vector<DWORD> &itemSels);
	int _FindByName(const char *name);
	void  _Init(IRenderPort *rp);
	void _CalculateBoneDir(int root);
	DWORD _Color(DWORD state);
private :
	DWORD _colorSel,_colorNormal,_colorDisable;
	SkeletonInfo _skeletonInfo;
	i_math::matrix43f *_matInit;
	std::vector<i_math::vector3df>  _buffer;
	std::vector<DWORD>  _color;
	std::vector<DWORD> _state;
	float  _scaleSJ;
	std::vector<i_math::vector3df>  _posBone;
};