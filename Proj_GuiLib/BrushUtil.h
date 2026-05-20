#pragma once

#include "GuiEditor.h"

#include "WorldSystem/ITrrn.h"
#include "ToolBase.h"
#include <set>
#include "pin/pin.h"

// 一个地表上Brush ,代理编辑的界面交互，及对地表数据的处理。 
// 为单例模式
class CBrushUtil :public CToolBase
{
public:
	virtual const char * GetTypeName();
	virtual DWORD GetType();

	friend class CGuiAgent_TerrainRLOp;
	friend class CGuiAgent_TerrainPaint;
protected:
	TrrnSeedMap _seedMap;
	TrrnSeedMapArg _arg;

	TrrnSeedMapArg::Purpose _purpose;

	CPinboard * _lnk_radius0;
	CPinboard * _lnk_radius1;

	float _speed;
	float _hardness;
	float _height;
	float _height2;
	BOOL  _bAccurate;
	unsigned char _idBr;
};
