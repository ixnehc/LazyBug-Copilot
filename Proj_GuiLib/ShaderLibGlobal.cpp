
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "FileSystem/IFileSystem.h"

#include "ShaderLibGlobal.h"

#include "stringparser/stringparser.h"

//////////////////////////////////////////////////////////////////////////
//CShaderLibGlobal

void CShaderLibGlobal::Clear()
{
	for (int i=0;i<_libs.size();i++)
		Safe_Class_Delete(_libs[i]);
	_libs.clear();

	for (int i=0;i<_features.size();i++)
		Safe_Class_Delete(_features[i]);
	_features.clear();

	_parampool.Clear();

	_pathRoot="";
	Zero();
}

void CShaderLibGlobal::_SortLibs()
{
	std::map<std::string,CShaderLib2 *>mp;
	for (int i=0;i<_libs.size();i++)
	{
		CShaderLib2 *lib=_libs[i];

		mp[lib->_nm]=lib;
	}

	_libs.clear();

	std::map<std::string,CShaderLib2 *>::iterator it;
	for (it=mp.begin();it!=mp.end();it++)
		_libs.push_back((*it).second);
}

void CShaderLibGlobal::_SortFeatures()
{
	std::map<std::string,CShaderFeature*>mp;
	for (int i=0;i<_features.size();i++)
	{
		CShaderFeature*feature=_features[i];

		mp[feature->_nm]=feature;
	}

	_features.clear();

	std::map<std::string,CShaderFeature*>::iterator it;
	for (it=mp.begin();it!=mp.end();it++)
		_features.push_back((*it).second);
}


void CShaderLibGlobal::SetContent(IFileSystem *pFS,const char *root)
{
	Clear();

	_pathRoot=root;

	std::vector<BYTE>buf;
	CDataPacket dp;

	//枚举并载入所有的lib
	if (TRUE)
	{
		std::string rootLib=_pathRoot+"\\libs";


		std::vector<std::string>files;
		IFileSystem_EnumFiles(pFS,rootLib.c_str(),files);

		for (int i=0;i<files.size();i++)
		{
			if (!CheckFileSuffix(files[i].c_str(),"sls"))
				continue;

			std::string s=rootLib+"\\"+files[i];

			IFile *fl=pFS->OpenFileAbs(s.c_str(),FileAccessMode_Read);
			if (fl)
			{

				IFile_ReadVector(fl,buf);
				fl->Close();

				CShaderLib2 *lib=Class_New2(CShaderLib2);
				dp.SetDataBufferPointer(buf.data());
				lib->Load(dp);

				lib->_nm=GetFileTitle(s);//根据文件名修补lib的名称
				_libs.push_back(lib);
			}
		}

	}

	//枚举并载入所有的features
	if (TRUE)
	{
		std::string root=_pathRoot+"\\features";

		std::vector<std::string>files;
		IFileSystem_EnumFiles(pFS,root.c_str(),files);

		for (int i=0;i<files.size();i++)
		{
			if (!CheckFileSuffix(files[i].c_str(),"fea"))
				continue;

			std::string s=root+"\\"+files[i];

			IFile *fl=pFS->OpenFileAbs(s.c_str(),FileAccessMode_Read);
			if (fl)
			{
				IFile_ReadVector(fl,buf);
				fl->Close();

				CShaderFeature*feature=Class_New2(CShaderFeature);
				dp.SetDataBufferPointer(buf.data());
				feature->Load(dp);

				feature->_nm=GetFileTitle(s);//根据文件名修补feature的名称
				_features.push_back(feature);
			}
		}
	}

	_SortLibs();
	_SortFeatures();

	_ver++;

}

CShaderLib2 *CShaderLibGlobal::_FindLib(const char *nm)
{
	for (int i=0;i<_libs.size();i++)
	{
		CShaderLib2 *lib=_libs[i];
		if (StringEqualNoCase(lib->_nm.c_str(),nm))
			return lib;
	}
	return NULL;
}

CShaderFeature*CShaderLibGlobal::_FindFeature(const char *nm)
{
	for (int i=0;i<_features.size();i++)
	{
		CShaderFeature *feature=_features[i];
		if (StringEqualNoCase(feature->_nm.c_str(),nm))
			return feature;
	}
	return NULL;
}



BOOL CShaderLibGlobal::AddLib(const char *nm)
{
	if (_FindLib(nm))
		return FALSE;

	CShaderLib2 *lib=Class_New2(CShaderLib2);

	lib->_nm=nm;
	_libs.push_back(lib);
	_SortLibs();

	_ver++;

	return TRUE;
}

BOOL CShaderLibGlobal::AddFeature(const char *nm)
{
	if (_FindFeature(nm))
		return FALSE;

	CShaderFeature*feature=Class_New2(CShaderFeature);

	feature->_nm=nm;
	_features.push_back(feature);
	_SortFeatures();

	_ver++;

	return TRUE;

}

BOOL CShaderLibGlobal::RenameLib(const char *nm,const char *nmNew)
{
	CShaderLib2 *lib=_FindLib(nm);
	if (!lib)
		return FALSE;

	CShaderLib2 *lib2=_FindLib(nmNew);
	if (lib2)
		return FALSE;

	lib->_nm=nmNew;

	_SortLibs();
	_ver++;

	return TRUE;
}

BOOL CShaderLibGlobal::RenameFeature(const char *nm,const char *nmNew)
{
	CShaderFeature*feature=_FindFeature(nm);
	if (!feature)
		return FALSE;

	CShaderFeature*feature2=_FindFeature(nmNew);
	if (feature2)
		return FALSE;

	feature->_nm=nmNew;

	_SortFeatures();
	_ver++;

	return TRUE;
}

BOOL CShaderLibGlobal::DeleteLib(const char *nm)
{
	CShaderLib2 *lib=_FindLib(nm);
	if (!lib)
		return FALSE;

	VEC_REMOVE(_libs,lib);
	lib->Clear();
	Safe_Class_Delete(lib);

	_SortLibs();
	_ver++;

	return TRUE;
}

BOOL CShaderLibGlobal::DeleteFeature(const char *nm)
{
	CShaderFeature*feature=_FindFeature(nm);
	if (!feature)
		return FALSE;

	VEC_REMOVE(_features,feature);
	feature->Clear();
	Safe_Class_Delete(feature);

	_SortFeatures();
	_ver++;
	return TRUE;
}
