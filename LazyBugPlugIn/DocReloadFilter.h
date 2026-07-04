#pragma once

#include <oleidl.h>   // For IOleCommandTarget
#include <unknwn.h>   // For IUnknown
#include <docobj.h>   // For OLECMD, OLECMDF_SUPPORTED, OLECMDF_ENABLED, OLECMDERR_E_NOTSUPPORTED etc.
#include <comdef.h>   // For _com_error (optional, for error handling)
#include <atlbase.h>
#include <vsshell.h>

// 定义一个特殊的接口用于识别我们的拦截器
// {F7A8B2C1-3D4E-5F6A-7B8C-9D0E1F2A3B4C}
static const GUID IID_IDocReloadFilterMarker = 
    { 0xf7a8b2c1, 0x3d4e, 0x5f6a, { 0x7b, 0x8c, 0x9d, 0x0e, 0x1f, 0x2a, 0x3b, 0x4c } };

interface IDocReloadFilterMarker : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetOriginalDocData(IVsPersistDocData** ppOriginal) = 0;
};

class CDocReloadFilter : public IVsPersistDocData, public IDocReloadFilterMarker
{
public:
	// 构造函数，传入原始的 DocData 对象
	CDocReloadFilter(IUnknown* pOriginalDocDataUnk) : _cRef(1)
	{
		// 保存原始 DocData 的 IVsPersistDocData 接口指针
		_spOriginalDocData = CComQIPtr<IVsPersistDocData>(pOriginalDocDataUnk);
	}

	// --- IUnknown 实现 ---
	STDMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&_cRef);
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		ULONG cRef = InterlockedDecrement(&_cRef);
		if (cRef == 0)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (ppvObject == nullptr) return E_POINTER;
		*ppvObject = nullptr;

		if (riid == IID_IUnknown)
		{
			*ppvObject = static_cast<IUnknown*>(static_cast<IVsPersistDocData*>(this));
			AddRef();
			return S_OK;
		}
		if (riid == IID_IVsPersistDocData)
		{
			*ppvObject = static_cast<IVsPersistDocData*>(this);
			AddRef();
			return S_OK;
		}
		if (riid == IID_IDocReloadFilterMarker)
		{
			*ppvObject = static_cast<IDocReloadFilterMarker*>(this);
			AddRef();
			return S_OK;
		}

		// 对于我们不处理的接口，委托给原始的 DocData 对象
		if (_spOriginalDocData)
		{
			CComPtr<IUnknown> spUnknown;
			if (SUCCEEDED(_spOriginalDocData->QueryInterface(IID_IUnknown, (void**)&spUnknown)))
			{
				return spUnknown->QueryInterface(riid, ppvObject);
			}
		}

		return E_NOINTERFACE;
	}

	// --- IDocReloadFilterMarker 实现 ---
	STDMETHODIMP GetOriginalDocData(IVsPersistDocData** ppOriginal) override
	{
		if (!ppOriginal) return E_POINTER;
		*ppOriginal = _spOriginalDocData;
		if (*ppOriginal)
		{
			(*ppOriginal)->AddRef();
			return S_OK;
		}
		return E_FAIL;
	}

	// --- IVsPersistDocData 实现 (关键部分) ---

	// 告诉 Visual Studio 这个文档不可重载
	STDMETHODIMP IsDocDataReloadable(BOOL* pfReloadable) override
	{
		if (pfReloadable == nullptr) return E_POINTER;

		// 这是核心！我们总是说它不能被重新加载。
		*pfReloadable = FALSE;

		return S_OK;
	}

	// 阻止实际的重载操作
	STDMETHODIMP ReloadDocData(VSRELOADDOCDATA grfFlags) override
	{
		// 返回 S_FALSE 表示 "不需要重载"
		// 这可以防止在某些情况下绕过 IsDocDataReloadable 的重载尝试
		return S_FALSE;
	}

	// --- 其他 IVsPersistDocData 方法 (全部委托给原始对象) ---

	STDMETHODIMP GetGuidEditorType(CLSID* pClassID) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->GetGuidEditorType(pClassID);
		return E_UNEXPECTED;
	}

	STDMETHODIMP IsDocDataDirty(BOOL* pfDirty) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->IsDocDataDirty(pfDirty);
		return E_UNEXPECTED;
	}

	STDMETHODIMP SetUntitledDocPath(LPCOLESTR pszDocDataPath) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->SetUntitledDocPath(pszDocDataPath);
		return E_UNEXPECTED;
	}

	STDMETHODIMP LoadDocData(LPCOLESTR pszDocDataPath) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->LoadDocData(pszDocDataPath);
		return E_UNEXPECTED;
	}

	STDMETHODIMP SaveDocData(VSSAVEFLAGS dwSave, BSTR* pbstrMkDocumentNew, BOOL* pfSaveCanceled) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->SaveDocData(dwSave, pbstrMkDocumentNew, pfSaveCanceled);
		return E_UNEXPECTED;
	}

	STDMETHODIMP Close() override
	{
		if (_spOriginalDocData) return _spOriginalDocData->Close();
		return E_UNEXPECTED;
	}

	STDMETHODIMP OnRegisterDocData(VSCOOKIE docCookie, IVsHierarchy* pHierNew, VSITEMID itemidNew) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->OnRegisterDocData(docCookie, pHierNew, itemidNew);
		return E_UNEXPECTED;
	}

	STDMETHODIMP RenameDocData(VSRDTATTRIB grfAttribs, IVsHierarchy* pHierNew, VSITEMID itemidNew, LPCOLESTR pszMkDocumentNew) override
	{
		if (_spOriginalDocData) return _spOriginalDocData->RenameDocData(grfAttribs, pHierNew, itemidNew, pszMkDocumentNew);
		return E_UNEXPECTED;
	}

// 	STDMETHODIMP IsDocDataReadOnly(BOOL* pfReadOnly) override
// 	{
// 		if (_spOriginalDocData) return _spOriginalDocData->IsDocDataReadOnly(pfReadOnly);
// 		return E_UNEXPECTED;
// 	}
// 
// 	STDMETHODIMP SetDocDataReadOnly(BOOL fReadOnly) override
// 	{
// 		if (_spOriginalDocData) return _spOriginalDocData->SetDocDataReadOnly(fReadOnly);
// 		return E_UNEXPECTED;
// 	}

public:
	~CDocReloadFilter() {} // 私有析构函数，强制使用 Release()

	ULONG _cRef;
	CComPtr<IVsPersistDocData> _spOriginalDocData;
};