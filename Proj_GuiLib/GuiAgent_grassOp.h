#pragma once

#include "GuiEditor.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/ISpg.h"

#include "WorldSystem/ITrrn.h"

#include <list>

#include "sparsearray/sparsearray2D.h"

#include "GuiAgent_general.h"

#include <set>

struct Spg
{
	SpgInfo info;
	BRUID   ID;
	float   radius;
};

class DistributedUtil
{
public:
	 void  SetRS(IRenderSystem * pRS){_pRS = pRS;}	
	 void  SetRP(IRenderPort *pRP){_pRP = pRP;}
	 void  AddSeed(BRUID ID,float radius,float w,float height);
	 void  SetScale(float scaleMin,float scaleMax){_scaleMax = scaleMax;_scaleMin = scaleMin;}
	 BOOL  Begin(float gridSize,float density);
	 DWORD Place(Spg *& result,i_math::pos2df &center,float radius);
	 void  End();
	 struct _Seed{
		 float radius,w,height;
		 BRUID ID;
	 };
	
	 int _RandomInt(int vmax);
	 float _RandomFloat(float vmin,float vmax);
	 int _ChooseSeed();
	 void _ConstructWeightTable();
	 BOOL _CheckAndFill(i_math::pos2df & center,float rSeed);
	 BYTE _CalWeight(int xOrg,int yOrg,i_math::pos2df & center,float r);
	 void _InitGrid(int xOrg,int yOrg,int w,i_math::pos2df &center,float radius);
	
	 void _Dump(int xOrg,int yOrg,int w);
	 
	 typedef SparseArray2D<BYTE,64,0> _Grid;

private:
	std::vector<Spg> _spgs;
	std::vector<_Seed> _seeds;
	_Grid _grid;

	float _gridSz,_density;
	float _scaleMin,_scaleMax;
	std::vector<float>  _weightTable;
	IRenderSystem * _pRS;
	IRenderPort * _pRP;
};

class CGuiAgent_GrassOp :public CGuiAgent_Dragger<TRUE,FALSE>
{
public:
	CGuiAgent_GrassOp(void);
	virtual ~CGuiAgent_GrassOp(void);
public:
	virtual BOOL OnKeyDown(char c,DWORD flag);//c is in upper case for 'a'~'z'
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

	struct Blk :public i_math::pos2di
	{
		Blk(int x,int y):i_math::pos2di(x,y){}
		Blk(const i_math::pos2di &oth):i_math::pos2di(oth.x,oth.y){}
		bool operator<(const Blk & oth) const {return (x<oth.x)||(x==oth.x&&y<oth.y);}
	};
protected:
	IBrushLib * _GetLib();
	void  _RemovOp(ISpgEditor * editor,i_math::pos2df& c,float radius);
	void _Op(int x,int y);
	void _InitOp(int x,int y);
	void _EndOp();

	std::vector<i_math::vector3df> _posbound;
	std::set<Blk>  _blkMods;
	DistributedUtil _util;
	IBrushLib * _brLib;
};

