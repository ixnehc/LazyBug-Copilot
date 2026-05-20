
#pragma once

#include "IResource.h"

struct DummyInfo;
struct DummiesData;
class IMatrice43;
class ISkeleton;
class IDummies :public IResource
{
public: 
	//additional function,special for Dummies
	virtual DWORD  GetCount()=0;   //the number of dummy contains
	virtual const char * GetDummyName(DWORD idx)=0;  // temp string ,can't be maintained.
	virtual DummyInfo * GetDummyInfo(DWORD &count)=0;
	virtual int FindDummy(const char *name)=0;//如果找不到返回-1

	virtual BOOL CalcMat(DWORD iDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat)=0;
	virtual BOOL CalcMat(const char *nameDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat)=0;

	virtual BOOL CalcMatAtSkeleton(DWORD iDummy,IMatrice43 *sklmats,i_math::matrix43f &mat)=0;
	virtual BOOL CalcMatAtSkeleton(const char *nameDummy,IMatrice43 *sklmats,i_math::matrix43f &mat)=0;

	//注意:xfms/nXfmx的xforms在parent的局部空间里
	virtual BOOL CalcMatAtBones(DWORD iDummy,i_math::xformf *xfms,DWORD nXfms,i_math::matrix43f &mat)=0;
	virtual BOOL CalcMatAtBones(const char *nameDummy,i_math::xformf *xfms,DWORD nXfms,i_math::matrix43f &mat)=0;

	virtual i_math::aabbox3df &GetDefAabb()=0;//得到缺省的aabb
	virtual BOOL CalcAabb(i_math::aabbox3df &aabb,IMatrice43 *sklmats,i_math::matrix43f *mat)=0;
	virtual BOOL HitTest(i_math::vector3df *posRet,i_math::line3df &line,float radius,IMatrice43 *sklmats,i_math::matrix43f *matBase)=0;

};

class IDummiesMgr :public IResourceMgr
{
};
class IDummies;
struct DummiesData;
class IDynDummiesMgr:public IResourceMgr
{
public:
	virtual IDummies * Create(const DummiesData * dummies) = 0;
};

