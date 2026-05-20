
#pragma once

#include "WorldSystem/stubparams/param_sys.h"

#include "WorldSystem/IAssetShell.h"

#include "strlib/strlib.h"

struct TextStyle:public PropRefTarget
{
	DEFINE_CLASS(TextStyle);
	IMPLEMENT_REFCOUNT_C

	BEGIN_GOBJ_PURE(TextStyle,1);
		GELEM_VAR_INIT(StringID,fmt,StringID_Invalid);
			GELEM_EDITVAR("格式字符串",GVT_U,GSem(GSem_StringID,"字符格式"),"用来指定额外的字体格式的字符串");
		GELEM_VAR_INIT(int,align,0);
			GELEM_EDITVAR("对齐方式",GVT_S,GSem(GSem_Interger,"左对齐,居中对齐,右对齐"),"对齐方式");
		GELEM_VAR_INIT(BOOL,bWordWrap,FALSE);
			GELEM_EDITVAR("自动换行",GVT_S,GSem_Boolean,"是否自动换行");
		GELEM_VAR_INIT(i_math::recti,margin,i_math::recti(1,1,1,1));
			GELEM_EDITVAR("边界间隔",GVT_Sx4,GSem(GSem_Rect,"NoRepair"),"边界间隔");
		GELEM_VAR_INIT(i_math::vector4db,colSelFg,i_math::vector4db(0,0,0,255));
			GELEM_EDITVAR("选择前景色",GVT_Bx4,GSem_ColorAlphaU,"文字被选中时的前景色");
		GELEM_VAR_INIT(i_math::vector4db,colSelBg,i_math::vector4db(255,255,0,128));
			GELEM_EDITVAR("选择背景色",GVT_Bx4,GSem_ColorAlphaU,"文字被选中时的背景色");
		GELEM_VAR_INIT(i_math::vector4db,colLink,i_math::vector4db(255,0,0,255));
			GELEM_EDITVAR("链接选中时颜色",GVT_Bx4,GSem_ColorAlphaU,"链接文字被选中时的的颜色");

	END_GOBJ();

	i_math::vector4db colSelFg;
	i_math::vector4db colSelBg;
	i_math::vector4db colLink;
	StringID fmt;
	int align;
	i_math::recti margin;
	BOOL bWordWrap;
};


struct ShellTextArg
{
	DEFINE_CLASS(ShellTextArg);
	ShellTextArg()
	{
		idStr=StringID_Invalid;
		style=NULL;
		bSingleLine=TRUE;
		bShowPassword=0;
	}
	~ShellTextArg()
	{
		SAFE_RELEASE(style);
	}

	StringID idStr;//如果为StringID_Invalid,则str有效,否则str无效
	std::string str;
	TextStyle *style;
	DWORD bSingleLine:1;
	DWORD bShowPassword:1;
	DWORD bShowLink:1;
};

class ITextPiece;
class CShellText
{
public:
	CShellText()
	{
		_tp=NULL;
		_style=NULL;
		_owner=NULL;
	}
	void Init(CAssetCtrl *owner,ShellTextArg &arg);
	BOOL IsEmpty()
	{
		return _tp?FALSE:TRUE;
	};
	void Clear();
	void Set(StringID id);
	void Set(const char *s);
	const char *GetStr();
	ITextPiece *GetTextPiece()	{		return _tp;	}
	TextStyle *GetStyle()	{		return _style;	}
	DWORD GetAlign()	{		return _style?_style->align:0;	}
	BOOL IsWordWrap()	{		return _style?_style->bWordWrap:FALSE;	}
	BOOL IsSingleLine();
	void SetWidthLimit(DWORD wLimit);//如果不是word wrap的text,这个函数不做任何事情

	ShellSize GetActualSize();
protected:
	void _NotifyTextChange();
	ITextPiece *_tp;
	TextStyle *_style;
	CAssetCtrl *_owner;
};
