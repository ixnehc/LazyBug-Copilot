
#pragma once
#include "GuiLib.h"

#include <vector>


#include "RichGrid.h"

#include "gds/GDefines.h"

class IRenderSystem;
class IWorldSystem;
class IEntitySystem;

struct GObjBase;
struct GElemBase;
struct DebugVarDesc;
class GuiLib_Api CDebugVarGrid:public CXTPReportControl
{
public:
	CDebugVarGrid()
	{
		_bColumnAdded=FALSE;
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void Add(DebugVarDesc &var);
	virtual void AddInvalid(const char *varname);

	const char *GetSelectVarPath();



protected:
	DECLARE_MESSAGE_MAP()

	CXTPReportRecord *_FindChildByName(CXTPReportRecord *record,const char *name);


	void _InsertRecord(const char *path,const char *str);
	CXTPReportRecord *_InsertRecord(CXTPReportRecord *recordParent,const char *key);

	BOOL _bColumnAdded;
};


struct DebugValue;
GuiLib_Api void DebugValueToString(DebugValue *v,std::string &s);
