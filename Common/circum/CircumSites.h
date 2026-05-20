#pragma once

class CCircumSites
{
public:
	CCircumSites()
	{
		Zero();
	}
	~CCircumSites()
	{
		Clear();
	}
	void Zero()
	{
		_rateChoose=0.0f;
		_iRing=0;
		_iPos=0;
		_nGen=0;
	}
	void Build(DWORD nRing,float radiusStart,float gap);
	void AddRing(i_math::vector2df *pos,DWORD nPos);
	void AddRing(i_math::matrix43f *mats,DWORD nMats);
	void Clear();

	DWORD GetCapacity()	{		return _sites.size();	}//返回最多可以产生多少位点
	i_math::vector2df *GetSites(DWORD &c)
	{
		c=_sites.size();
		return _sites.data();
	}
	DWORD GetRingCount()	{		return _rings.size();	}
	i_math::vector2df *GetRingSites(DWORD iRing,DWORD &c);
	int RingFromSite(DWORD iSite);

	void BeginGen(float rateChoose=0.75f);//rateChoose,表示在所有的位点中选出多少位点
	i_math::vector2df *Gen();//返回NULL,表示已经没有可以被选择的点了

protected:
	WORD _GetRingSitesCount(DWORD iRing);
	std::vector<i_math::vector2df>_sites;
	std::vector<WORD> _rings;//记录每一圈的起始Site

	float _rateChoose;
	WORD _iRing;
	WORD _iPos;
	WORD _nGen;//当前的Ring上已经产生了几个了
	WORD _step;

};
