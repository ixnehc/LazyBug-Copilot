#pragma once

#include "editor/editor.h"

#include "shaderlib2/ShaderLib2.h"

class CShaderLibGlobal;
class GraphicsGraph;
class CShaderLibGraph
{
public:
	CShaderLibGraph()
	{
		Zero();
		_ver=0;
	}
	~CShaderLibGraph()
	{
		Clear();
	}
	void Zero()
	{
		_startVS=0.0f;
		_startPS=10.0f;
		_startOut=20.0f;
	}
	void Clear();
	void Load(CShaderLibGlobal *global,const char *nmLib,GraphicsGraph *gg);
	void Draw(GraphicsGraph *gg);

protected:

	//版本信息
	DWORD _ver;
	std::string _nmLib;

	//用来绘制的数据
	float _startVS;
	float _startPS;
	float _startOut;
};


