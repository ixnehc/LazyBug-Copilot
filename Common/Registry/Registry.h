// WUIDGen.h: interface for the CWUIDGen class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <tchar.h> 
#include <string>
#include <vector>
class CCurrentUserRegistry
{
public:
	CCurrentUserRegistry(const char* nameCompany,const char* nameApp);
	CCurrentUserRegistry()	{	}

	void Init(const char* nameCompany,const char* nameApp);

	BOOL WriteInt(const char* sSection,const char* sEntry,int v);
	int ReadInt(const char* sSection,const char* sEntry,int def=0);
	BOOL WriteString(const char* sSection,const char* sEntry,const char* v);
	std::string ReadString(const char* sSection,const char* sEntry,const char* sDef="");

	// 宽字符串读写函数
	BOOL WriteWString(const char* sSection,const char* sEntry,const wchar_t* v);
	BOOL WriteWString(const char* sSection,const char* sEntry,const std::wstring& v);
	std::wstring ReadWString(const char* sSection,const char* sEntry,const wchar_t* sDef=L"");

	BOOL WriteVar(const char* sSection,const char* sEntry,void *var,DWORD szVar);
	BOOL ReadVar(const char* sSection,const char* sEntry,void *var,DWORD szVar);

	BOOL WriteData(const char* sSection,const char* sEntry,void *data,DWORD szData);
	BOOL ReadData(const char* sSection,const char* sEntry,void *&data,DWORD &szData);//return a temply ptr

	void SendEvent(const char* nmEvent);
	BOOL FetchEvent(const char* nmEvent);
	BOOL PeekEvent(const char* nmEvent);

	template<typename T>
	BOOL WriteVar(const char* sSection,const char* sEntry,T &v)
	{
		return WriteVar(sSection,sEntry,&v,sizeof(v));
	}

	template<typename T>
	BOOL ReadVar(const char* sSection,const char* sEntry,T &v)
	{
		return ReadVar(sSection,sEntry,&v,sizeof(v));
	}


private:
	std::string m_sRegistryKey;
	std::string m_sProfileName;

	HKEY GetAppRegistryKey();
	HKEY GetSectionKey(const char* lpszSection);

	std::vector<BYTE>m_buf;//temp for ReadData


};
