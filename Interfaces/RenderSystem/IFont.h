
#pragma once


#define TEXTCLIP_INFINITE (i_math::rect_sh(-10000,-10000,10000,10000))

#define TEXTWIDTHLIMIT_INFINITE 4096
//#define DT_LEFT             0x00000000
//#define DT_CENTER           0x00000001
//#define DT_RIGHT            0x00000002
struct DrawFontArg
{
	DrawFontArg()
	{
		m_dtAlign=0;//DT_LEFT
		m_bSingleLine=FALSE;
		m_ptLoc.set(0,0);
		m_wLimit=TEXTWIDTHLIMIT_INFINITE;
		m_rcClip=TEXTCLIP_INFINITE;
		m_alpha=1.0f;
	}
	void SetAlign(int dtAlign)	{		m_dtAlign=dtAlign;	}
	void SetSingleLine(BOOL bSingleLine){		m_bSingleLine=bSingleLine;	}

	//w,h只在这个字体需要非左对齐的时候才有意义填
	void SetLocation(int x,int y){		m_ptLoc.set(x,y);	}
	void SetWidthLimit(DWORD wLimit)	{		m_wLimit=wLimit;	}

	void SetClipRect(int l,int t,int r,int b){		m_rcClip.set(l,t,r,b);}
	void SetClipRect(i_math::rect_sh &rc){		m_rcClip=rc;	}
	void ClearClipRect(){		m_rcClip=TEXTCLIP_INFINITE;	}

	void SetAlpha(float alpha)	{		m_alpha=alpha;	}

	int m_dtAlign;//DT_XXXX(DT_LEFT,DT_RIGHT,DT_CENTER)
	BOOL m_bSingleLine;

	i_math::pos2d_sh m_ptLoc;
	DWORD m_wLimit;
	i_math::rect_sh m_rcClip;
	float m_alpha;//0..1
};

//用在ITextPiece::HitTest(..)
struct TextHitResult
{
	TextHitResult()
	{
		nPos=-1;
		bLineEnd=FALSE;
		iLine=-1;
	}
	int nPos;//点中的字符的位置
	BOOL bLineEnd;//是否点在行末
	int iLine;//只在bLineEnd为TRUE的时候有效,指出是哪一行的行末
};

class ITexture;
struct TxtPatch;

//TextPiece 的Location的意义:
//TextPiece的Location是一个点,可以看作基准点,和普通的理解不一样,它不一定是这个text piece的左上角,而是会根据textpiece的
//对齐方式(align)不同而不同:
//如果是左对齐,location是左上角的点
//如果是右对齐,location是右上角的点
//如果是居中对齐,location是上边的中点
class ITextPiece
{
public:
	INTERFACE_REFCOUNT;

	virtual void ApplyArg(const DrawFontArg &arg)=0;
	virtual TxtPatch*ObtainPatches(DWORD &c)=0;//注意:返回的Patches里的tex没有加引用计数

	virtual void SetDefaultFormat(const char *strFmt)=0;//注意,改变缺省的format不会影响现有的文字的format,只会影响新加入的文字
	virtual void SetFormatText(const char* pchNewText)=0;
	virtual void SetWidthLimit(DWORD wLimit)=0;
	virtual DWORD GetWidthLimit()=0;
	virtual void SetLocation(const i_math::pos2d_sh &ptLoc)=0;
	virtual void SetClip(const i_math::rect_sh &clip)=0;
	virtual i_math::size2d_sh GetActualSize()=0;//得到文字块的实际大小(包围所有文字的最小区域的大小)
	virtual void SetAlign(DWORD dt)=0;//DT_XXXX
	virtual DWORD GetAlign()=0;//DT_XXXX

	virtual void SetLineSpan(WORD nSpan)=0;
	virtual WORD GetLineSpan()=0;//行间隔
	virtual void SetSingleLine(BOOL bSingleLine)=0;
	virtual BOOL IsSingleLine()=0;

	virtual int GetWhichLine(int nPos)=0;


	virtual int GetLineHeight(int iLine=0)=0;

	virtual int GetLineYBase(int iLine=0)=0;
	virtual int GetLineAscender(int iLine=0)=0;
	virtual int GetLineDescender(int iLine=0)=0;

	virtual int GetLineNumber()=0;
	virtual int GetLineEndPos(int nLine)=0;
	virtual int GetLineBeginPos(int nLine)=0;

	virtual BOOL GetCharCoordinate(int nIndex,i_math::pos2d_sh &pt)=0;
	virtual BOOL GetLineEndCoordinate(int nLine,i_math::pos2d_sh&pt)=0;	

	virtual int DeleteCharW(int nIndex,int nDeleteLen)=0;
	virtual int InsertChar(int nIndex,char* pChar, int nInsertLen)=0;
	virtual int InsertCharW(int nIndex,WORD* pCodes, int nInsertLen)=0;

	virtual void SetShowPassword(char cPassword= '\0')=0;//if char=='\0', no password	
	virtual WORD GetPasswordCode()=0;

	virtual void SetSelection( int nStart, int nEnd)=0;//nStart < nEnd
	virtual void ClearSelection()=0;

	virtual TextHitResult LineHitTest(int nLine,int x)=0;//return position in content
	virtual TextHitResult HitTest(i_math::pos2d_sh &pt)=0;//return position in content

	virtual int GetTextLen()=0;
	virtual int GetTextLenMB()=0;
	virtual WORD *GetText()=0;//return unicode bytes for text
	virtual char *GetTextMB()=0;//return Multibytes bytes for text

	virtual int GetLineCount()=0;

	virtual void SetAlpha(DWORD vAlpha)=0;
	virtual void SetBgAlpha(DWORD vAlpha)=0;

	virtual void SetSelColor(DWORD colSel,DWORD colSelBg)=0;
	virtual void ClearSelColor()=0;

	virtual void ShowLink(DWORD colHilightLink,BOOL bShowLink=TRUE)=0;
	virtual void SetHilightLinkID(WORD idHilightLink)=0;
	virtual WORD GetHilightLinkID()=0;
	virtual WORD LinkIDHitTest(i_math::pos2d_sh &pt)=0;
	virtual BOOL GetLinkString(WORD idLink,std::string &strLink)=0;

	virtual void SetPixelRate(float rate)=0;

};


class IFontMgr
{
public:
	virtual void Flush()=0;

	virtual ITextPiece *MakeText(const char *str)=0;
	virtual void ClearCache()=0;//清除所有字体的贴图cache

	//this is for test
	virtual ITexture *GetFontTexture(DWORD idx)=0;
};
