#pragma once

#include "Log/LogFile.h"

#include "../progress/progress.h"

/**
 * @brief 循环地图位置结构体
 * @tparam TPrim 地图单元类型
 * 
 * 用于定位和访问循环地图中的具体位置，支持循环访问功能
 */
template <class TPrim>
struct CycleMapLoc
{
	CycleMapLoc()
	{
		p=NULL;
		x=y=w=h=0;
	}
	// 检查位置是否有效
	BOOL IsValid()	{		return p!=NULL;	}
	// 获取指定偏移量的地图单元，支持循环访问
	TPrim &Get(int xOff,int yOff)
	{
		xOff+=x;
		yOff+=y;
		CYCLE_VALUE(xOff,w);
		CYCLE_VALUE(yOff,h);
		return *(p+yOff*w+xOff);
	}
	TPrim *p;      // 地图数据指针
	int x,y;       // 当前位置坐标
	DWORD w,h;     // 地图宽度和高度（以TPrim为单位）
};

/**
 * @brief 添加脏区域（需要更新的区域）的宏
 * @param l,t,r,b 区域的左、上、右、下边界
 */
#define CMC_ADD_DIRTY_RECT(l,t,r,b)\
{\
	rcDirty[nRc].set(l,t,r,b);\
	if (rcDirty[nRc].isValid())\
		nRc++;\
}

/**
 * @brief 添加脏区域的宏（使用矩形对象）
 * @param rc 矩形对象
 */
#define CMC_ADD_DIRTY_RECT2(rc)\
{\
	if ((rc).isValid())\
		rcDirty[nRc++]=(rc);\
}

// 无效中心点坐标
#define INVALID_CENTER (-1000000)

/**
 * @brief 循环地图核心类
 * 
 * 提供循环地图的基本功能，包括坐标转换、中心点管理和区域更新
 */
class CCycleMapCore
{
public:
	CCycleMapCore()
	{
		Zero();
	}
	// 初始化地图
	void Zero()
	{
		_szUnit.set(0,0);
		_ptUnitOrg.set(0,0);
		_ptLocalOrg.set(-1,-1);
	}

	// 设置地图单元大小
	void Set(DWORD wUnit,DWORD hUnit)
	{
		_szUnit.set(wUnit,hUnit);
	}

	// 获取地图宽度（以单元为单位）
	DWORD GetWidth() const	{		return _szUnit.w;	}
	// 获取地图高度（以单元为单位）
	DWORD GetHeight() const	{		return _szUnit.h;	}
	// 获取地图矩形区域（以单元为单位，在世界坐标系中）
	i_math::recti GetMapRect() const	
	{		
		if (!IsCentered())
			return i_math::recti(INVALID_CENTER,INVALID_CENTER,INVALID_CENTER,INVALID_CENTER);
		return i_math::recti(_ptUnitOrg,_szUnit);	
	}
	// 检查指定单元是否在地图范围内
	BOOL CheckUnitIn(int xUnit,int yUnit) const
	{
		if (((DWORD)(xUnit-_ptUnitOrg.x))>=_szUnit.w)
			return FALSE;
		if (((DWORD)(yUnit-_ptUnitOrg.y))>=_szUnit.h)
			return FALSE;
		return TRUE;
	}

	// 检查地图是否已设置中心点
	BOOL IsCentered() const
	{
		return !((_ptLocalOrg.x==-1)&&(_ptLocalOrg.y==-1));
	}

	// 获取地图中心点坐标（世界坐标系，以单元为单位）
	void GetCenter(int &xc,int &yc) const
	{
		if (!IsCentered())
		{
			xc=yc=INVALID_CENTER;
			return;
		}
		xc=_ptUnitOrg.x+_szUnit.w/2;
		yc=_ptUnitOrg.y+_szUnit.h/2;
	}
	// 获取地图中心点位置
	i_math::pos2di GetCenter() const
	{		
		if (!IsCentered())
			return i_math::pos2di(INVALID_CENTER,INVALID_CENTER);
		return _ptUnitOrg+i_math::pos2di(_szUnit.w/2,_szUnit.h/2);	
	}

	// 获取地图原点位置
	i_math::pos2di GetOrg()	
	{	
		if (!IsCentered())
			return i_math::pos2di(INVALID_CENTER,INVALID_CENTER);

		return _ptUnitOrg;	
	}

	/**
	 * @brief 设置地图中心点
	 * @param xc,yc 世界坐标系中的中心点坐标（以单元为单位）
	 * @param nRc 返回脏区域数量
	 * @return 脏区域数组指针
	 */
	i_math::recti*SetCenter(int xc,int yc,DWORD &nRc)
	{
		return _SetCenter(xc,yc,nRc,FALSE);
	}

	/**
	 * @brief 预设置地图中心点
	 * @param xc,yc 世界坐标系中的中心点坐标（以单元为单位）
	 * @param nRc 返回脏区域数量
	 * @return 脏区域数组指针
	 */
	i_math::recti*PreSetCenter(int xc,int yc,DWORD &nRc)
	{
		return _SetCenter(xc,yc,nRc,TRUE);
	}

	/**
	 * @brief 将世界坐标区域转换为本地坐标区域
	 * @param rc0 世界坐标系中的区域
	 * @param nRc 返回转换后的区域数量
	 * @return 本地坐标区域数组指针
	 */
	i_math::recti*ToLocalRect(i_math::recti &rc0,DWORD &nRc)
	{
		static i_math::recti rcDirty[8];// 足够大的缓冲区
		nRc=0;

		// 转换区域左上角坐标
		i_math::pos2di pt=rc0.UpperLeftCorner;
		ToLocal(pt);
		i_math::recti rc=rc0;
		rc.zeroBase();
		rc+=pt;

		// 计算四个象限的区域
		i_math::recti rc00,rc10,rc01,rc11;
		i_math::pos2di pt00(0,0),pt10(_szUnit.w,0),pt01(0,_szUnit.h),pt11(_szUnit.w,_szUnit.h);
		rc00.set(0,0,_szUnit.w,_szUnit.h);
		rc10=rc00+pt10;
		rc01=rc00+pt01;
		rc11=rc00+pt11;

		// 裁剪区域
		rc00.clipAgainst(rc);
		rc10.clipAgainst(rc);
		rc01.clipAgainst(rc);
		rc11.clipAgainst(rc);

		// 调整坐标
		rc10-=pt10;
		rc01-=pt01;
		rc11-=pt11;

		// 添加有效区域
		CMC_ADD_DIRTY_RECT2(rc00);
		CMC_ADD_DIRTY_RECT2(rc10);
		CMC_ADD_DIRTY_RECT2(rc01);
		CMC_ADD_DIRTY_RECT2(rc11);

		return rcDirty;
	}

	/**
	 * @brief 将世界坐标转换为本地坐标
	 * @param pt 要转换的坐标点
	 */
	void ToLocal(i_math::pos2di &pt) const
	{
		pt=_ptLocalOrg+pt-_ptUnitOrg;
		CYCLE_VALUE(pt.x,_szUnit.w);
		CYCLE_VALUE(pt.y,_szUnit.h);
	}

	/**
	 * @brief 将本地坐标转换为世界坐标
	 * @param pt 要转换的坐标点
	 */
	void ToWorld(i_math::pos2di &pt) const
	{
		pt=pt-_ptLocalOrg;
		CYCLE_VALUE(pt.x,_szUnit.w);
		CYCLE_VALUE(pt.y,_szUnit.h);
		pt=_ptUnitOrg+pt;
	}

	/**
	 * @brief 将本地矩形区域转换为世界坐标区域
	 * @param rc 要转换的矩形区域
	 */
	void ToWorld(i_math::recti &rc) const
	{
		ToWorld(rc.UpperLeftCorner);
		rc.LowerRightCorner.x--;
		rc.LowerRightCorner.y--;
		ToWorld(rc.LowerRightCorner);
		rc.LowerRightCorner.x++;
		rc.LowerRightCorner.y++;
		rc.repair();
	}

protected:
	/**
	 * @brief 内部设置中心点的方法
	 * @param xc,yc 世界坐标系中的中心点坐标
	 * @param nRc 返回脏区域数量
	 * @param bPreSet 是否为预设置
	 * @return 脏区域数组指针
	 */
	i_math::recti*_SetCenter(int xc,int yc,DWORD &nRc,BOOL bPreSet)
	{
		static i_math::recti rcDirty[8];// 足够大的缓冲区
		int xOrg,yOrg;
		xOrg=xc-_szUnit.w/2;
		yOrg=yc-_szUnit.h/2;
		nRc=0;
		if (!IsCentered())
		{// 首次定位
			if (!bPreSet)
			{
				_ptUnitOrg.set(xOrg,yOrg);
				_ptLocalOrg.set(0,0);
			}
			rcDirty[0].set(0,0,_szUnit.w,_szUnit.h);
			nRc=1;
			return rcDirty;
		}

		if ((_ptUnitOrg.x==xOrg)&&(_ptUnitOrg.y==yOrg))
			return rcDirty;

		// 计算需要更新的列范围
		int scol,ecol;
		if (xOrg<_ptUnitOrg.x)
		{
			scol=i_math::clampup_i(xOrg-_ptUnitOrg.x,-_szUnit.w);
			ecol=0;
		}
		else
		{
			scol=0;
			ecol=i_math::clampdown_i(xOrg-_ptUnitOrg.x,_szUnit.w);
		}

		// 计算需要更新的行范围
		int srow,erow;
		if (yOrg<_ptUnitOrg.y)
		{
			srow=i_math::clampup_i(yOrg-_ptUnitOrg.y,-_szUnit.h);
			erow=0;
		}
		else
		{
			srow=0;
			erow=i_math::clampdown_i(yOrg-_ptUnitOrg.y,_szUnit.h);
		}

		// 转换为脏区域
		if (TRUE)
		{
			BOOL bNoCol=TRUE;
			if (scol!=ecol)
			{
				bNoCol=FALSE;
				scol+=_ptLocalOrg.x;
				ecol--;
				ecol+=_ptLocalOrg.x;
				CYCLE_VALUE(scol,_szUnit.w);
				CYCLE_VALUE(ecol,_szUnit.w);

				if (ecol>=scol)
				{
					CMC_ADD_DIRTY_RECT(scol,0,ecol+1,_szUnit.h);
				}
				else
				{
					CMC_ADD_DIRTY_RECT(scol,0,_szUnit.w,_szUnit.h);
					CMC_ADD_DIRTY_RECT(0,0,ecol+1,_szUnit.h);
				}
			}

			if (srow!=erow)
			{
				srow+=_ptLocalOrg.y;
				erow--;
				erow+=_ptLocalOrg.y;
				CYCLE_VALUE(srow,_szUnit.h);
				CYCLE_VALUE(erow,_szUnit.h);

				if (erow>=srow)
				{
					if (bNoCol)
					{
						CMC_ADD_DIRTY_RECT(		0,			srow,		_szUnit.w,	erow+1);
					}
					else
					{
						if (ecol>=scol)
						{
							CMC_ADD_DIRTY_RECT(0,			srow,		scol,				erow+1);
							CMC_ADD_DIRTY_RECT(ecol+1,	srow,		_szUnit.w,	erow+1);
						}
						else
							CMC_ADD_DIRTY_RECT(ecol+1,	srow,		scol,				erow+1);
					}
				}
				else
				{
					if (bNoCol)
					{
						CMC_ADD_DIRTY_RECT(		0,			srow,		_szUnit.w,	_szUnit.h);
						CMC_ADD_DIRTY_RECT(		0,			0,				_szUnit.w,	erow+1);
					}
					else
					{
						if (ecol>=scol)
						{
							CMC_ADD_DIRTY_RECT(0,			srow,		scol,				_szUnit.h);
							CMC_ADD_DIRTY_RECT(ecol+1,	srow,		_szUnit.w,	_szUnit.h);
							CMC_ADD_DIRTY_RECT(0,			0,				scol,				erow+1);
							CMC_ADD_DIRTY_RECT(ecol+1,	0,				_szUnit.w,	erow+1);
						}
						else
						{
							CMC_ADD_DIRTY_RECT(ecol+1,	srow,		scol,				_szUnit.h);
							CMC_ADD_DIRTY_RECT(ecol+1,	0,				scol,				erow+1);
						}
					}
				}
			}
		}
		if (!bPreSet)
		{
			_ptLocalOrg+=i_math::pos2di(xOrg,yOrg)-_ptUnitOrg;
			CYCLE_VALUE(_ptLocalOrg.x,_szUnit.w);
			CYCLE_VALUE(_ptLocalOrg.y,_szUnit.h);
			_ptUnitOrg=i_math::pos2di(xOrg,yOrg);
		}

		return rcDirty;
	}

	i_math::size2di _szUnit;
	i_math::pos2di _ptUnitOrg;
	i_math::pos2di _ptLocalOrg;//in unit

};

//Cycle Map Unit Flag
#define CMUF_Touched 1
#define CMUF_Modified 2


template <class TPrim>
class CCycleMap
{
public:
	CCycleMap()
	{
		Zero();
	}
	void Zero()
	{
		_map=NULL;
		_blockflags=NULL;
		_core.Zero();
		_szPrim.set(0,0);
		_ptOrg.set(0,0);
		_nextToTouch=0;
		_nextToSave=0;
		_bTouchModified=FALSE;
	}
	//w/h is is in TPrim
	BOOL Init(DWORD w,DWORD h)
	{
		Clear();
		if ((w==0)&&(h==0))
			return FALSE;
		DWORD wUnit,hUnit;
		wUnit=w;
		hUnit=h;
		_core.Set(wUnit,hUnit);
		_blockflags=new unsigned char[wUnit*hUnit];
		memset(_blockflags,0,wUnit*hUnit);
		_map=new TPrim[wUnit*hUnit];
		_szPrim.w=w;
		_szPrim.h=h;

		return TRUE;
	}
	void Clear()
	{
		UnTouchAll();
		SAFE_DELETE(_blockflags);
		SAFE_DELETE_ARRAY(_map);
		Zero();
	}

	DWORD GetWidth()	{		return _szPrim.w;	}
	DWORD GetHeight()	{		return _szPrim.h;	}
	DWORD GetUnitWidth()	{		return _core.GetWidth();	}
	DWORD GetUnitHeight()	{		return _core.GetHeight();	}


	//xc,yc is a world coordinate,in prim,
	void GetCenter(int &xc,int &yc)
	{	
		if (!_core.IsCentered())
		{
			xc=yc=INVALID_CENTER;
			return;
		}

		xc=_ptOrg.x+_szPrim.w/2;yc=_ptOrg.y+_szPrim.h/2;
	}

	i_math::recti GetMapRect()//in prim
	{		
		if (!_core.IsCentered())
			return i_math::recti(INVALID_CENTER,INVALID_CENTER,INVALID_CENTER,INVALID_CENTER);
		return i_math::recti(_ptOrg.x,_ptOrg.y,_ptOrg.x+_szPrim.w,_ptOrg.y+_szPrim.h);
	}


	//xc,yc is a world coordinate,in prim,
	void SetCenter(int xc,int yc)
	{
		_ptOrg.x=xc-_szPrim.w/2;
		_ptOrg.y=yc-_szPrim.h/2;
		int xcUnit,ycUnit;
		//First calc the unit position of the left-up corner
		xcUnit=_ptOrg.x;
		ycUnit=_ptOrg.y;
		//adjust to the center
		xcUnit+=_core.GetWidth()/2;
		ycUnit+=_core.GetHeight()/2;
		i_math::recti *rc;
		DWORD nRc;

		if (TRUE)//first save the going-to-be untouched units,and untouch them
		{
			i_math::recti *rcs;
			rcs=_core.PreSetCenter(xcUnit,ycUnit,nRc);
			for (int k=0;k<nRc;k++)
			{
				i_math::recti rc=rcs[k];
				BYTE *p=_blockflags+rc.Top()*_core.GetWidth()+rc.Left();
				for (int j=0;j<rc.getHeight();j++)
				{
					for (int i=0;i<rc.getWidth();i++)
					{
						i_math::pos2di ptUnitLocal,ptUnit;
						ptUnitLocal.set(i,j);
						ptUnitLocal+=rc.UpperLeftCorner;
						if (p[i]&CMUF_Modified)
						{
							ptUnit=ptUnitLocal;
							_core.ToWorld(ptUnit);
							_Save(ptUnit);
						}
						_Untouch(ptUnitLocal);
					}
					p+=_core.GetWidth();
				}
			}

		}

		rc=_core.SetCenter(xcUnit,ycUnit,nRc);

		_nextToTouch=0;
		_nextToSave=0;
	}

	//x,y is world coord,in TPrim
	TPrim *ObtainPrim(int x,int y)
	{
		if (!IsIn(x,y))
			return NULL;
		i_math::pos2di ptUnit,ptStart,ptOff;
		ptUnit.x=x;
		ptUnit.y=y;
		if (!_Touch(ptUnit,ptStart))
			return NULL;
		ptOff.x=x-ptUnit.x;
		ptOff.y=y-ptUnit.y;
		return _map+(ptStart.y+ptOff.y)*(_core.GetWidth())+
								(ptStart.x+ptOff.x);
	}

	TPrim *QueryPrim(int x,int y)
	{
		TPrim *prim=ObtainPrim(x,y);
		if (!prim)
			return NULL;

		i_math::pos2di ptLocal(x,y);
		_core.ToLocal(ptLocal);
		_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]|=CMUF_Modified;
		_nextToSave=0;
		return prim;
	}

	//x,y is world coord,in TPrim
	BOOL IsIn(int x,int y) const
	{
		if (((DWORD)(x-_ptOrg.x))>=_szPrim.w)
			return FALSE;
		if (((DWORD)(y-_ptOrg.y))>=_szPrim.h)
			return FALSE;
		return TRUE;
	}

	//touch next untouched unit,return whether there are still some untouched units
	//in bSuccess returns whether this touch is successful,will ONLY set it to FALSE
	BOOL TouchNext(BOOL *bSuccess)
	{
		DWORD total=_core.GetWidth()*_core.GetHeight();
		while(_nextToTouch<total)
		{
			if (!(_blockflags[_nextToTouch]&CMUF_Touched))
			{
				i_math::pos2di pt,ptStart;
				pt.x=_nextToTouch%_core.GetWidth();
				pt.y=_nextToTouch/_core.GetWidth();
				_core.ToWorld(pt);
				if (!_Touch(pt,ptStart))
				{
					if (bSuccess)
						*bSuccess=FALSE;
				}
				_nextToTouch++;
				return _nextToTouch<total;
			}
			_nextToTouch++;
		}
		return FALSE;//no more to touch
	}

	//save next modified unit,return whether there are still some modified units
	//in bSuccess returns whether this save is successful,will ONLY set it to FALSE
	BOOL SaveNext(BOOL *bSuccess)
	{
		DWORD total=_core.GetWidth()*_core.GetHeight();
		while(_nextToSave<total)
		{
			if (_blockflags[_nextToSave]&CMUF_Modified)
			{
				i_math::pos2di pt;
				pt.x=_nextToSave%_core.GetWidth();
				pt.y=_nextToSave/_core.GetWidth();
				_core.ToWorld(pt);
				if (!_Save(pt))
				{
					if (bSuccess)
						*bSuccess=FALSE;
				}
				_nextToSave++;
				return _nextToSave<total;
			}
			_nextToSave++;
		}
		return FALSE;//no more to save
	}

	void UnTouchAll()
	{
		for (int i=0;i<_core.GetWidth();i++)
		for (int j=0;j<_core.GetHeight();j++)
			_Untouch(i_math::pos2di(i,j));
		_nextToSave=0;
		_nextToTouch=0;
	}

	void TouchAll(CProgress *prg=NULL)
	{
		DWORD nToTouch=0;
		if (prg)
		{
			DWORD total=_core.GetWidth()*_core.GetHeight();
			DWORD t=_nextToTouch;
			while(t<total)
			{
				if (!(_blockflags[_nextToTouch]&CMUF_Touched))
					nToTouch++;
				t++;
			}
		}
		DWORD nTouched=0;
		while(TouchNext(NULL))
		{
			nTouched++;
			if (prg)
			{
				if (nTouched%4==0)
					prg->SetProgress("",nTouched,nToTouch);
			}
		}
	}

	void ReTouch(i_math::pos2di ptUnit)
	{
		i_math::pos2di pt=ptUnit;
		if (!IsIn(pt.x,pt.y))
			return;
		_core.ToLocal(pt);
		_Untouch(pt);
		_Touch(ptUnit,pt);
	}



protected:

	//ptUnit is a world coordinate,in unit,
	//in ptStart return the local coordinate(in TPrim) of it
	BOOL _Touch(i_math::pos2di &ptUnit,i_math::pos2di &ptStart)
	{
		assert(_core.CheckUnitIn(ptUnit.x,ptUnit.y));

		i_math::pos2di ptLocal=ptUnit;
		_core.ToLocal(ptLocal);

		TPrim *p=_map+ptLocal.y*_core.GetWidth()+
						ptLocal.x;
		if (!(_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]&CMUF_Touched))
		{
			_bTouchModified=FALSE;
			if (FALSE==_OnTouch(ptUnit.x,ptUnit.y,p))//this unit not touched yet,try to touch it
				return FALSE;
			_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]|=CMUF_Touched;
			if (_bTouchModified)
				_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]|=CMUF_Modified;
		}

		ptStart=ptLocal;

		return TRUE;
	}

	//ptUnitLocal is a world coordinate,in unit,
	void _Untouch(i_math::pos2di &ptUnitLocal)
	{
		assert(((DWORD)ptUnitLocal.x)<_core.GetWidth());
		assert(((DWORD)ptUnitLocal.y)<_core.GetHeight());

		if (_blockflags[ptUnitLocal.y*_core.GetWidth()+ptUnitLocal.x]&CMUF_Touched)
		{
			TPrim *p=_map+ptUnitLocal.y*_core.GetWidth()+
										ptUnitLocal.x;
			_OnUntouch(p);//this unit is touched,try to untouch it
			_blockflags[ptUnitLocal.y*_core.GetWidth()+ptUnitLocal.x]&=(~CMUF_Touched);
		}
	}

	//ptUnit is a world coordinate,in unit,
	BOOL _Save(i_math::pos2di &ptUnit)
	{
		assert(_core.CheckUnitIn(ptUnit.x,ptUnit.y));

		i_math::pos2di ptLocal=ptUnit;
		_core.ToLocal(ptLocal);

		TPrim *p=_map+ptLocal.y*_core.GetWidth()+
			ptLocal.x;
		if (_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]&CMUF_Modified)
		{
			if (FALSE==_OnSave(ptUnit.x,ptUnit.y,p))//this unit is modified,try to save it
				return FALSE;
			_blockflags[ptLocal.y*_core.GetWidth()+ptLocal.x]&=(~CMUF_Modified);
		}

		return TRUE;
	}



	virtual BOOL _OnTouch(int xUnit,int yUnit,TPrim *p)=0;//World coordinate
	virtual void _OnUntouch(TPrim *p)=0;
	virtual BOOL _OnSave(int xUnit,int yUnit,TPrim *p)=0;//World coordinate

	void _SetModifiedTouch()	{		_bTouchModified=TRUE;	}

	TPrim *_map;
	i_math::size2di _szPrim;//map size in TPrim
	i_math::pos2di _ptOrg;//map left-up world coord in TPrim

	CCycleMapCore _core;

	BOOL _bTouchModified;


	BYTE *_blockflags;//flags(CMBF_XXXX) for each block
	DWORD _nextToTouch;
	DWORD _nextToSave;
};



class CCycleMap2
{
public:
	CCycleMap2()
	{
		Zero();
	}
	void Zero()
	{
		_core.Zero();
	}
	BOOL Init(DWORD wUnit,DWORD hUnit)
	{
		Clear();
		if ((wUnit==0)&&(hUnit==0))
			return FALSE;
		_core.Set(wUnit,hUnit);
		return TRUE;
	}
	void Clear()
	{
		Zero();
	}


	DWORD GetWidth()	{		return _core.GetWidth();	}
	DWORD GetHeight()	{		return _core.GetHeight();	}
	i_math::recti GetMapRect()//in unit
			{		return _core.GetMapRect();	}

	BOOL IsCentered()	{		return _core.IsCentered();	}
	void GetCenter(int &xc,int &yc){	return _core.GetCenter(xc,yc);}
	i_math::pos2di GetCenter(){	return _core.GetCenter();}

	//xc,yc is a world coordinate,in unit,
	void SetCenter(int xc,int yc)
	{
		i_math::recti *rc;
		DWORD nRc;
		rc=_core.SetCenter(xc,yc,nRc);

		for (int k=0;k<nRc;k++)
			_TouchRect(rc[k]);
	}

	//set the left-up corner of this map
	//xo,yo is world coordinate,in unit
	void SetOrg(int xo,int yo)
	{
		SetCenter(xo+_core.GetWidth()/2,yo+_core.GetHeight()/2);
	}
	void GetOrg(int &xo,int &yo)
	{
		i_math::pos2di pt=GetOrg();
		xo=pt.x;
		yo=pt.y;
	}
	i_math::pos2di GetOrg()	{		return _core.GetOrg();	}

	i_math::recti *CalcLocalRect(i_math::recti &rc0,DWORD &nRC)
	{
		nRC=0;
		i_math::recti rcMap;
		rcMap=GetMapRect();
		i_math::recti rc=rc0;
		rc.clipAgainst(rcMap);
		if (!rc.isValid())
			return NULL;

		return _core.ToLocalRect(rc,nRC);
	}

	void ReTouch(i_math::recti &rc0)
	{
		if (!_core.IsCentered())
			return;//not validly centered yet,do nothing
		i_math::recti *rcs;
		DWORD nRc;

		rcs=CalcLocalRect(rc0,nRc);

		for (int k=0;k<nRc;k++)
			_TouchRect(rcs[k]);
	}

	void ReTouchAll()
	{
		ReTouch(GetMapRect());
	}
protected:
	virtual void _TouchRect(i_math::recti &rc)
	{
		_BeginTouchRect(rc);
		for (int j=0;j<rc.getHeight();j++)
		for (int i=0;i<rc.getWidth();i++)
		{
			i_math::pos2di ptRel,ptUnit;
			ptRel.set(i,j);
			ptUnit=ptRel;
			ptUnit+=rc.UpperLeftCorner;
			_core.ToWorld(ptUnit);
			_Touch(ptUnit,ptRel);
		}
		_EndTouchRect();
	}

	//rc is in local coordinate
	virtual void _BeginTouchRect(i_math::recti &rc)=0;

	//ptUnit is in world coordinate,while ptRelative is a pt (in unit) relative to the left-up 
	//corner of the rect passed in _BeginTouchRect(...)
	virtual void _Touch(i_math::pos2di &ptUnit,i_math::pos2di &ptRelative)=0;
	virtual void _EndTouchRect()=0;

	CCycleMapCore _core;
};



//If changed,return TRUE and ptCenter will be filled the new center.
//Note that the center is in block
//ע��:radius����Ҫ��blocklen�Ĵ�С
inline BOOL UpdateBlockMapCenter(i_math::pos2di &ptCenter,float x,float z,float radius,float blocklen)
{
	// 确保更新半径不小于块长度，避免频繁更新
	if (radius < blocklen)
		radius = blocklen;

	// 将当前中心点的块坐标转换为世界坐标
	float xc, zc;
	xc = (float)(ptCenter.x * blocklen);
	zc = (float)(ptCenter.y * blocklen);

	// 检查目标位置是否超出当前中心点的更新半径
	if ((fabsf(x-xc) > radius) || (fabsf(z-zc) > radius))
	{
		// 更新中心点位置，将世界坐标转换为块坐标
		ptCenter.x = FloatToNearestInt(x/blocklen);
		ptCenter.y = FloatToNearestInt(z/blocklen);
		return TRUE;
	}
	return FALSE;
}
