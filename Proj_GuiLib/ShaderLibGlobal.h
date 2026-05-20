#pragma once
#include "GuiLib.h"

#include "editor/editor.h"

#include "shaderlib2/ShaderLib2.h"

class IFileSystem;
class GuiLib_Api CShaderLibGlobal
{
public:
	CShaderLibGlobal()
	{
		_ver=0;
		Zero();
	}
	~CShaderLibGlobal()
	{
		Clear();
	}
	void Zero()
	{
	}
	void Clear();
	void SetContent(IFileSystem *pFS,const char *root);

	BOOL AddLib(const char *nm);
	BOOL AddFeature(const char *nm);
	BOOL RenameLib(const char *nm,const char *nmNew);
	BOOL RenameFeature(const char *nm,const char *nmNew);
	BOOL DeleteLib(const char *nm);
	BOOL DeleteFeature(const char *nm);

	CShaderLib2 *FindLib(const char *nm){		return _FindLib(nm);	}

public://take it as protected

	void _SortLibs();
	void _SortFeatures();

	CShaderLib2 *_FindLib(const char *nm);
	CShaderFeature*_FindFeature(const char *nm);


	std::string _pathRoot;
	DWORD _ver;
	std::vector<CShaderLib2*>_libs;
	std::vector<CShaderFeature*>_features;
	CShaderParamPool _parampool;
};


