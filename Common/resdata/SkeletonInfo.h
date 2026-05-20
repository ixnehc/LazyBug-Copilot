#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/matrix43.h"
#include "../math/vector3d.h"
#include "../math/vector2d.h"
#include "../math/xform.h"


struct BoneInfo
{
	enum
	{
		Flag_NotAffecting=1,//this bone does not affect any vertex(but may play a role in controlling the bone transform animation)
		ForceDword=0xffffffff,
	};
	enum
	{
		MaxBoneName=48,
	};

	char name[MaxBoneName];
	int iParent;
	i_math::matrix43f matOff;//offset matrix
	i_math::xformf xformDef;//default matrix
	DWORD flag;
};

class SkeletonInfo :public std::vector<BoneInfo>
{
public:
	DWORD GetBranchCount();
	BOOL IsMatch(SkeletonInfo &src,BOOL bCase);//bCase indicates whether case senstive
	int FindBone(const char *name);//not case sensitive,return -1 if not found
	int GetParent(int iBone)
	{
		if ((iBone>=0)&&(iBone<size()))
			return (*this)[iBone].iParent;
		return -1;
	}

	void GetDefMatrix(i_math::matrix43f *& mat/* must delete after*/)
	{
		mat = new i_math::matrix43f[size()];
		BOOL * bCal = new BOOL[size()];
		memset(bCal,0,size()*sizeof(BOOL));

		for(int i =0;i<size();i++)
		{
			if(bCal[i]) continue; 

			std::stack<int> s;
			s.push(i);
			while(!s.empty())
			{
				BoneInfo & bone = (*this)[s.top()];

				if(bone.iParent==-1)
				{
					bCal[i] = TRUE;
					mat[i] = bone.xformDef.getMatrix();
					s.pop();
					continue;
				}

				if(!bCal[bone.iParent])
					s.push(bone.iParent);
				else
				{
					mat[i] = bone.xformDef.getMatrix()*mat[bone.iParent];
					bCal[i]	= TRUE;
					s.pop();
				}   
			}
		}

		delete bCal;
	}

	void CalcGlobalXfms(i_math::xformf *xfmsLocal,i_math::xformf &xfmBase,i_math::xformf *xfmsGlobal)
	{
		i_math::xformf *xfmParent;

		for(int i =0;i<size();i++)
		{
			int iParent=(*this)[i].iParent;
			if (iParent==-1)
				xfmParent=&xfmBase;
			else
				xfmParent=&xfmsGlobal[iParent];

			xfmsGlobal[i]=xfmsLocal[i]*(*xfmParent);
		}
	}

	void CalcLocalXfms(i_math::xformf *xfmsLocal,i_math::xformf &xfmBase,i_math::xformf *xfmsGlobal)
	{
		i_math::xformf *xfmParent;

		for(int i =0;i<size();i++)
		{
			int iParent=(*this)[i].iParent;
			if (iParent==-1)
				xfmParent=&xfmBase;
			else
				xfmParent=&xfmsGlobal[iParent];

			xfmsLocal[i]=xfmsGlobal[i];
			xfmsLocal[i].makeLocal(*xfmParent);
		}
	}

};
