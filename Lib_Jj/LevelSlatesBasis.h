#pragma once

#include "LevelSlateDefines.h"


#include <unordered_map>

struct LosSlate;
struct LopSlate;

struct LevelSlateInfo
{
	LevelSlateInfo()
	{
		uidOwner=LevelGUID_Invalid;
		tpDef=LevelSlateType_None;
		src=NULL;
		param=NULL;
		nLinks=0;
		idxMe=-1;
		pt.set(-1,-1);
	}

	LevelGUID GetUID();


	void AddLink(LevelSlateIdx idx)
	{
		for (int i=0;i<nLinks;i++)
		{
			if (links[i]==idx)
				return;//已经连接了
		}
		if (nLinks>=ARRAY_SIZE(links))
			return;//无法连接
		links[nLinks]=idx;
		nLinks++;
	}
	LosSlate *src;
	LopSlate *param;
	LevelGUID uidOwner;//Slates which owns me
	LevelSlateType tpDef;
	LevelSlateIdx idxMe;
	int nLinks;
	LevelSlateIdx links[LevelSlate_MaxLink];
	i_math::pos2di pt;
};

struct LosSlates;
struct LopSlates;
struct LevelSlatesInfo
{
	LevelGUID GetUID();
	LosSlates *src;
	LopSlates *param;

	//包含哪些Slates
	LevelSlateIdx iStartSlate;
	DWORD countSlates;

	//Group
	struct Grp
	{
		Grp()
		{
			iStart=count=0;
		}
		short iStart;
		short count;
	};
	std::deque<LevelSlateIdx> indicesGrp;
	std::unordered_map<StringID,Grp> grps;

	i_math::vector2df dirH;
	i_math::vector2df dirV;

	//二维阵列
	LevelSlateIdx GetSlateAt(int x,int y)
	{
		if ((x>=0)&&(x<wMatrix)&&(y>=0)&&(y<hMatrix))
			return matrix[y*wMatrix+x];
		return LevelSlateIdx_Invalid;
	}
	int wMatrix,hMatrix;
	std::deque<LevelSlateIdx> matrix;


};

typedef LevelSlatesInfo::Grp LevelSlatesGrpHandle;
#define LevelSlatesGrpHandle_IsValid(h) ((h).count>0)
#define LevelSlatesGrpHandle_GetCount(h) ((h).count)

class CLevelSources;
class CLevelSlatesBasis
{
public:
	void Build(CLevelSources *srces);

	void Clear();

	LevelSlatesInfo *FindSlatesInfo(LevelGUID uid);

	LevelSlateInfo *GetSlateInfo(LevelSlateIdx idx);
	LevelSlateInfo *FindSlateInfo(LevelGUID uid);
	LevelSlateIdx FindSlateIdx(LevelGUID uid);

protected:
	void _BuildMatrix(LevelSlatesInfo &infoSlates);

	std::unordered_map<LevelGUID,LevelSlatesInfo> _bufSlates;

	std::deque<LevelSlateInfo> _bufSlate;//用LevelSlateIdx索引
	std::unordered_map<LevelGUID,LevelSlateIdx> _lookupSlate;

};