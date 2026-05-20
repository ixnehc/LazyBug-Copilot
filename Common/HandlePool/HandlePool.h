#pragma once

#include <vector>
#include <string>


class CHandlePool
{
public:
	class CHandle
	{
	public:
		CHandle()
		{
			m_RefCount=0;
			m_hHandle=NULL;
		}
		std::string m_path;
		HANDLE m_hHandle;
		int m_RefCount;
		virtual CHandle& operator=(const CHandle &src)
		{
			m_path=src.m_path;
			m_hHandle=src.m_hHandle;
			m_RefCount=src.m_RefCount;

			return *this;
		}
	};


	std::vector<CHandle> m_aHandles;

	HANDLE OpenHandle(const char *buffer);
	void CloseHandle(HANDLE hHandle);
	HANDLE QueryHandle(const char *buffer);//check whether existing a handle opened using buffer

protected:
	virtual HANDLE Open(const char *buffer)=0;
	virtual void Close(HANDLE)=0;

};
