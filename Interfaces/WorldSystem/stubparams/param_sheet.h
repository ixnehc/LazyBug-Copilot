
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"

#include "sheet/sheet.h"
#include "RenderSystem/ISheet.h"
#include "WorldSystem/ILuaMachine.h"



class ISheet;
struct PropSheet:public GProperty
{
	PropSheet()
	{
		GConstructor();
	}
	~PropSheet()
	{
		SAFE_RELEASE(sheet);
		GDestructor();
	}
	DEFINE_CLASS(PropSheet);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ(PropSheet,1);
		GELEM_VAR_INIT(CSheet *,sheet,NULL);
	END_GOBJ();    

	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_Sheet;	}
	virtual GProperty *Clone()
	{
		PropSheet *ret=Class_New2(PropSheet);
		SAFE_REPLACE(ret->sheet,sheet);
		return ret;
	}

	ISheet *sheet;

};

struct PropSheetRow:public GProperty
{
	PropSheetRow()
	{
		GConstructor();
	}
	~PropSheetRow()
	{
		SAFE_RELEASE(row);
		GDestructor();
	}
	DEFINE_CLASS(PropSheetRow);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ(PropSheetRow,1);
		GELEM_VAR_INIT(CSheetRow*,row,NULL);
	END_GOBJ();    

	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_SheetRow;	}
	virtual GProperty *Clone()
	{
		PropSheetRow *ret=Class_New2(PropSheetRow);
		SAFE_REPLACE(ret->row,row);
		return ret;
	}

	CSheetRow *row;

};