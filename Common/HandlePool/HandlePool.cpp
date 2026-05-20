#include "stdh.h"
#include "HandlePool.h"

//check whether existing a handle opened using buffer
HANDLE CHandlePool::QueryHandle(const char *buffer)
{
	std::string s;
	s=buffer;
	strupr((char *)s.c_str());

	int i,sz;

	sz=(int)m_aHandles.size();

	for (i=0;i<sz;i++)
	{
		if (m_aHandles[i].m_path==s)
			return m_aHandles[i].m_hHandle;
	}

	return NULL;
}

HANDLE CHandlePool::OpenHandle(const char *buffer)
{
	std::string s;
	s=buffer;
	strupr((char *)s.c_str());

	int i,sz;

	sz=(int)m_aHandles.size();

	for (i=0;i<sz;i++)
	{
		if (m_aHandles[i].m_path==s)
		{
			m_aHandles[i].m_RefCount++;
			return m_aHandles[i].m_hHandle;
		}
	}

	HANDLE hHandle;
	hHandle=Open(buffer);

	if (!hHandle)
		return NULL;

	CHandle v;
	v.m_path=s;
	v.m_hHandle=hHandle;
	v.m_RefCount=1;

	m_aHandles.push_back(v);

	return hHandle;
}
void CHandlePool::CloseHandle(HANDLE hHandle)
{
	int i,sz;

	sz=(int)m_aHandles.size();

	for (i=0;i<sz;i++)
	{
		if (m_aHandles[i].m_hHandle==hHandle)
		{
			m_aHandles[i].m_RefCount--;
			if (m_aHandles[i].m_RefCount<=0)
			{
				Close(hHandle);
				m_aHandles.erase(m_aHandles.begin()+i);
				return;
			}
		}
	}
}
