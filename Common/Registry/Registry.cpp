#include "Stdh.h"
#include "Registry.h"
#include <vector>

#include "../stringparser/stringparser.h"

#include "assert.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCurrentUserRegistry::CCurrentUserRegistry(const char* nameCompany,const char* nameApp)
{
	m_sRegistryKey = nameCompany;
	m_sProfileName = nameApp;
}

void CCurrentUserRegistry::Init(const char* nameCompany,const char* nameApp)
{
	m_sRegistryKey = nameCompany;
	m_sProfileName = nameApp;
}



// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CCurrentUserRegistry::GetAppRegistryKey()
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, fromMBCS("software"), 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, fromMBCS(m_sRegistryKey.c_str()), 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, fromMBCS(m_sProfileName.c_str()), 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CCurrentUserRegistry::GetSectionKey(const char* lpszSection)
{
	assert(lpszSection != NULL);

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	RegCreateKeyEx(hAppKey, fromMBCS(lpszSection), 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

int CCurrentUserRegistry::ReadInt(const char* lpszSection, const char* lpszEntry,
							int nDefault)
{

	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return nDefault;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwCount = sizeof(DWORD);
	LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
		(LPBYTE)&dwValue, &dwCount);
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
	{
		assert(dwType == REG_DWORD);
		assert(dwCount == sizeof(dwValue));
		return (UINT)dwValue;
	}
	return nDefault;
}

std::string CCurrentUserRegistry::ReadString(const char* lpszSection, const char* lpszEntry,
								  const char* lpszDefault)
{

	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return lpszDefault;
	std::string strValue;
	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
		NULL, &dwCount); 
	if (lResult == ERROR_SUCCESS)
	{
		assert(dwType == REG_SZ);
		std::vector<BYTE>temp;
		temp.resize(dwCount+1);
		lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
			temp.data(), &dwCount);
		strValue=(char*)temp.data();
	}
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
	{
		assert(dwType == REG_SZ);
		return strValue;
	}
	return std::string(lpszDefault);
}

BOOL CCurrentUserRegistry::WriteInt(const char* lpszSection, const char* lpszEntry,
							  int nValue)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	LONG lResult = RegSetValueEx(hSecKey, fromMBCS(lpszEntry), NULL, REG_DWORD,
		(LPBYTE)&nValue, sizeof(nValue));
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

BOOL CCurrentUserRegistry::WriteString(const char* lpszSection, const char* lpszEntry,
								 const char* lpszValue)
{
	assert(lpszSection != NULL);
	LONG lResult;
	if (lpszEntry == NULL) //delete whole section
	{
		HKEY hAppKey = GetAppRegistryKey();
		if (hAppKey == NULL)
			return FALSE;
		lResult = ::RegDeleteKey(hAppKey, fromMBCS(lpszSection));
		RegCloseKey(hAppKey);
	}
	else if (lpszValue == NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		// necessary to cast away const below
		lResult = ::RegDeleteValue(hSecKey, fromMBCS(lpszEntry));
		RegCloseKey(hSecKey);
	}
	else
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		lResult = RegSetValueEx(hSecKey, fromMBCS(lpszEntry), NULL, REG_SZ,
			(LPBYTE)lpszValue, (strlen(lpszValue)+1)*sizeof(BYTE));
		RegCloseKey(hSecKey);
	}
	return lResult == ERROR_SUCCESS;
}

// 宽字符串写入函数 - const wchar_t* 版本
BOOL CCurrentUserRegistry::WriteWString(const char* lpszSection, const char* lpszEntry,
								 const wchar_t* lpszValue)
{
	assert(lpszSection != NULL);
	LONG lResult;
	if (lpszEntry == NULL) //delete whole section
	{
		HKEY hAppKey = GetAppRegistryKey();
		if (hAppKey == NULL)
			return FALSE;
		lResult = ::RegDeleteKey(hAppKey, fromMBCS(lpszSection));
		RegCloseKey(hAppKey);
	}
	else if (lpszValue == NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		lResult = ::RegDeleteValue(hSecKey, fromMBCS(lpszEntry));
		RegCloseKey(hSecKey);
	}
	else
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		lResult = RegSetValueEx(hSecKey, fromMBCS(lpszEntry), NULL, REG_BINARY,
			(LPBYTE)lpszValue, (wcslen(lpszValue)+1)*sizeof(wchar_t));
		RegCloseKey(hSecKey);
	}
	return lResult == ERROR_SUCCESS;
}

// 宽字符串写入函数 - std::wstring 版本
BOOL CCurrentUserRegistry::WriteWString(const char* lpszSection, const char* lpszEntry,
								 const std::wstring& strValue)
{
	return WriteWString(lpszSection, lpszEntry, strValue.c_str());
}

// 宽字符串读取函数
std::wstring CCurrentUserRegistry::ReadWString(const char* lpszSection, const char* lpszEntry,
								  const wchar_t* lpszDefault)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return lpszDefault;
	std::wstring strValue;
	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
		NULL, &dwCount); 
	if (lResult == ERROR_SUCCESS)
	{
		assert(dwType == REG_BINARY);
		std::vector<BYTE>temp;
		temp.resize(dwCount+sizeof(wchar_t));
		lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
			temp.data(), &dwCount);
		strValue=(wchar_t*)temp.data();
	}
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
	{
		assert(dwType == REG_BINARY);
		return strValue;
	}
	return std::wstring(lpszDefault);
}



BOOL CCurrentUserRegistry::WriteVar(const char* lpszSection, const char* lpszEntry,
																		void *var,DWORD szVar)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	LONG lResult = RegSetValueEx(hSecKey, fromMBCS(lpszEntry), NULL, REG_BINARY,
		(LPBYTE)var, szVar);
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

BOOL CCurrentUserRegistry::ReadVar(const char* lpszSection,const char* lpszEntry,
																	void *var,DWORD szVar)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	DWORD dwType;
	DWORD dwCount=0;
	LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
		(LPBYTE)NULL, &dwCount);
	if ((dwCount>szVar)||(dwType!=REG_BINARY))
		lResult=ERROR_SUCCESS+1;//a not ERROR_SUCCESS value
	if (lResult == ERROR_SUCCESS)
	{
		LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
			(LPBYTE)var, &dwCount);
	}
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
		return TRUE;
	return FALSE;
}


BOOL CCurrentUserRegistry::WriteData(const char* lpszSection,const char* lpszEntry,
																	void *data,DWORD szData)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	m_buf.resize(szData+4);
	*((DWORD*)m_buf.data())=szData;
	if (szData>0)
		memcpy(&m_buf[4],data,szData);
	LONG lResult = RegSetValueEx(hSecKey, fromMBCS(lpszEntry), NULL, REG_BINARY,
		(LPBYTE)m_buf.data(),szData+4);
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;

}
BOOL CCurrentUserRegistry::ReadData(const char* lpszSection,const char* lpszEntry,void *&data,DWORD &szData)
{
	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	DWORD dwType;
	DWORD dwCount=0;
	LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
		(LPBYTE)NULL, &dwCount);
	if (lResult == ERROR_SUCCESS)
	{
		LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
																		NULL, &szData);
		if (dwType!=REG_BINARY)
			lResult=ERROR_SUCCESS+1;
		if (lResult==ERROR_SUCCESS)
		{
			m_buf.resize(szData);
			LONG lResult = RegQueryValueEx(hSecKey, fromMBCS(lpszEntry), NULL, &dwType,
				m_buf.data(), &szData);
			if (szData>4)
			{
				data=&m_buf[4];
				szData-=4;
			}
			else
				lResult=ERROR_SUCCESS+1;//a not ERROR_SUCCESS value
		}
	}
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
		return TRUE;
	data=NULL;
	szData=0;
	return FALSE;

}


void CCurrentUserRegistry::SendEvent(const char *nmEvent)
{
	extern unsigned __int64 GetAbsTick();
	unsigned __int64 t=GetAbsTick();

	WriteData("_RegistryEvents",nmEvent,&t,sizeof(t));
}

BOOL CCurrentUserRegistry::FetchEvent(const char *nmEvent)
{
	extern unsigned __int64 GetAbsTick();
	unsigned __int64 tCur=GetAbsTick();

	void*data;
	DWORD szData;
	if (!ReadData("_RegistryEvents",nmEvent,data,szData))
		return FALSE;

	if (szData!=sizeof(unsigned __int64))
		return FALSE;

	unsigned __int64 t;
	memcpy(&t,data,szData);

	if (t+2000>tCur)
	{
		t=0;
		WriteData("_RegistryEvents",nmEvent,&t,sizeof(t));
		return TRUE;
	}
	return FALSE;
}

BOOL CCurrentUserRegistry::PeekEvent(const char *nmEvent)
{
	extern unsigned __int64 GetAbsTick();
	unsigned __int64 tCur=GetAbsTick();

	void*data;
	DWORD szData;
	if (!ReadData("_RegistryEvents",nmEvent,data,szData))
		return FALSE;

	if (szData!=sizeof(unsigned __int64))
		return FALSE;

	unsigned __int64 t;
	memcpy(&t,data,szData);

	if (t+2000>tCur)
		return TRUE;

	return FALSE;
}

