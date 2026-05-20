#pragma once
#include <vector>
#include <string>


enum ResourceSelectMode
{
	RSM_ENTIRE, 
	RSM_PARTIAL
};

class CResourceSelector;
class CResourceList
{
public:
	CResourceList() : _pSelector(NULL)
	{
	}
	virtual ~CResourceList() {}

public:
	void SetSelector(CResourceSelector* selector)
	{
		_pSelector = selector;
	}

public:
	virtual void SetRootDir(const char* dir) = 0;
	virtual void SetFilter(const char* filter) = 0;
	virtual const char* GetSelResource() const = 0;

protected:
	CResourceSelector* _pSelector;
};

class CResourceViewer
{
public:
	virtual ~CResourceViewer() {}

public:
	virtual void SetSelectMode(int mode) = 0;
	virtual void SetResource(const char* res) = 0;

	virtual BOOL IsCanView(const char* res) const = 0;

	virtual const RECT& GetSelectedRect() const = 0;

	virtual void Update() = 0;
};

class CResourceViewerManager
{
public:

public:
	BOOL IsRegisterred(CResourceViewer* viewer) const
	{
		BOOL bRegisterred = FALSE;
		std::vector<CResourceViewer*>::const_iterator it = _viewers.begin();
		for(; it != _viewers.end(); it++)
		{
			if ((*it) == viewer)
			{
				bRegisterred = TRUE;
				break;
			}
		}
		return bRegisterred;
	}
	BOOL RegisterViewer(CResourceViewer* viewer)
	{
		BOOL bRet = FALSE;
		if (!IsRegisterred(viewer))
		{
			_viewers.push_back(viewer);
			bRet = TRUE;
		}
		return bRet;
	}
	void UnregisterViewer(CResourceViewer* viewer)
	{
		std::vector<CResourceViewer*>::iterator it = _viewers.begin();
		for(; it != _viewers.end(); it++)
		{
			if ((*it) == viewer)
			{
				_viewers.erase(it);
				break;
			}
		}
	}
	CResourceViewer* GetAvailableViewer(const char* res) const
	{
		CResourceViewer* pViewer = NULL;
		std::vector<CResourceViewer*>::const_iterator it = _viewers.begin();
		for(; it != _viewers.end(); it++)
		{
			if ((*it)->IsCanView(res))
			{
				pViewer = *it;
				break;
			}
		}
		return pViewer;
	}

private:
	std::vector<CResourceViewer*> _viewers;
};

class CResourceSelector
{
public:
	CResourceSelector() : _pCurViewer(NULL)//, _pResList(NULL)
	{
	}

	virtual ~CResourceSelector() {}

public:
	virtual void SetSelectedMode(int mode) = 0;
	virtual void SetRootDir(const char* dir) = 0;
	virtual const char* GetRootDir() const = 0;
	virtual void SetFilter(const char* filter) = 0;
	virtual const char* GetSelResource() const = 0;
	virtual const RECT& GetSelectedRect() const = 0;

public:
	virtual void OnResourceChanged(const char* res)
	{
		if (_pCurViewer)
		{
			if (_pCurViewer->IsCanView(res))
			{
				_pCurViewer->SetResource(res);
				return;
			}
			else
				_pCurViewer->SetResource(NULL);
		}

		CResourceViewer* viewer = _viewerMgr.GetAvailableViewer(res);
		if (viewer)
		{
			viewer->SetResource(res);
			_pCurViewer = viewer;
		}
	}

protected:
	//CResourceList* _pResList;
	CResourceViewer* _pCurViewer;
	CResourceViewerManager _viewerMgr;
};
