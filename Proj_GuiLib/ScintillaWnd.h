/* @doc
 * @module ScintillaWnd.h | Definition of a Scintilla syntax coloring edit control for MFC<nl>
 * This program is an example how to use the excellent scintilla edit control of Neil Hodgson.<nl>
 * See www.scintilla.org for details<nl>
 * Author: Horst Br�ckner, hb@ec-logic.com<nl>
 * Environment: VisualC++ Version 6, static Build of Scintilla, SciLexer.dll as Lexer<nl>
 */

#pragma once

#include "GuiLib.h"

#define SCN_CUSTOM_POST_KEYDOWN 3000

////////////////////////////////////
// @class CScintillaWnd | Class of a GCL Scintilla syntax coloring edit control for MFC
// @base public | CWnd
//
class GuiLib_Api CScintillaWnd : public CWnd  
{
public:
// @access public constructor - destructor
// @cmember empty Constructor
	CScintillaWnd();
// @cmember destructor
	virtual ~CScintillaWnd();

public:
// @access public macro members
// @cmember return linenumber display flag
   BOOL GetDisplayLinenumbers (){return m_bLinenumbers;};
// @cmember return selection display flag
   BOOL GetDisplaySelection (){return m_bSelection;};
// @cmember return folding margin display flag
   BOOL GetDisplayFolding (){return m_bFolding;};
// @cmember set search flags
   virtual void SetSearchflags (int nSearchflags){m_nSearchflags = nSearchflags;};
   virtual int GetSearchflags ()   {	   return m_nSearchflags;   }
   

public:
// @access public function members
// @cmember register a window class to use scintilla in a dialog
   static BOOL Register(CWinApp *app, UINT id);
// @cmember try to load Lexer DLL
   static HMODULE LoadScintillaDll (LPCSTR szLexerDll = NULL);
// @cmember create window
   virtual BOOL Create (LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
// @cmember Set Text to the Scintilla control
   virtual void SetText (LPCSTR szText,BOOL bSetFocus=TRUE);
// @cmember Get Text from the Scintilla control
   virtual void GetText (std::string &strText);
// @cmember Get Text from the Scintilla control
   virtual LPSTR GetText();
// @cmember try to find lexer format from extension
   virtual int GetFormatFromExtension (LPCSTR szExtension);
// @cmember calcluate number of chars needed for linenumberdisplay
   virtual int GetLinenumberChars ();
// @cmember calcluate number of pixels for linenumber display
   virtual int GetLinenumberWidth ();
   virtual int SetMarginWidth(int width);
// @cmember set display of linenumbers on/off
   virtual void SetDisplayLinenumbers(BOOL bFlag = TRUE);
// @cmember set display of selection/bookmark margin on/off
   virtual void SetDisplaySelection(BOOL bFlag = TRUE);
// @cmember set display of source folding margin on/off
   virtual void SetDisplayFolding(BOOL bFlag = TRUE);
// @cmember cut selection to clipboard
   virtual void Cut();
// @cmember copy selection to clipboard
   virtual void Copy();
// @cmember paste from clipboard
   virtual void Paste();
// @cmember clear selection
   virtual void Clear();
// @cmember undo last change
   virtual void Undo();
// @cmember redo last change
   virtual void Redo();
// @cmember check if we can undo
   virtual BOOL CanUndo();
// @cmember check if we can redo
   virtual BOOL CanRedo();
// @cmember undo last change
   virtual void EmptyUndoBuffer();
// @cmember check if we have something to paste from clipboard
   virtual BOOL CanPaste();
// @cmember select complete text
   virtual void SelectAll();
// @cmember return the current line number
   virtual long GetCurrentLine();
// @cmember set the current line number
   virtual void SetCurrentLine(long line);
// @cmember return the current column number
   virtual long GetCurrentColumn();
// @cmember return the current character position within the file
   virtual long GetCurrentPosition();
   virtual void SetCurrentPosition(int pos);
// @cmember return the current style number at the current character position
   virtual int GetCurrentStyle();
// @cmember return the current folding level at the current character position
   virtual int GetCurrentFoldinglevel();
// @cmember set the fontname for a style number
   virtual void SetFontname(int nStyle, LPCSTR szFontname);
// @cmember set the fontname height in points for a style number
   virtual void SetFontheight(int nStyle, int nHeight);
// @cmember set the foregroundcolor for a style number
   virtual void SetForeground(int nStyle, COLORREF crForeground);
// @cmember set the backgroundcolor for a style number
   virtual void SetBackground(int nStyle, COLORREF crBackground);
// @cmember set given style to bold
   virtual void SetBold(int nStyle, BOOL bFlag);
// @cmember set given style to bold
   virtual void SetItalic(int nStyle, BOOL bFlag);
// @cmember set given style to underlind
   virtual void SetUnderline(int nStyle, BOOL bFlag);
   virtual void SetHotSpot(int nStyle, BOOL bFlag);

   long GetFirstVisibleLine();
   void SetFirstVisibleLine(int line);

   // @cmember set style attributes
   void SetStyle(int style, COLORREF fore, COLORREF back, int size=0, const char *face=NULL); 
   void SetSelColor(COLORREF fore, COLORREF back); 

   // @cmember get flag if we are in overstrike mode
   virtual BOOL GetReadOnly();
   // @cmember set flag for overstrike mode
   virtual void SetReadOnly(BOOL bReadOnly);

// @cmember get flag if we are in overstrike mode
   virtual BOOL GetOverstrike();
// @cmember set flag for overstrike mode
   virtual void SetOverstrike(BOOL bOverstrike);
// @cmember init the view with reasonable defaults
   virtual void Init();
// @cmember called whenever the text is changed - we can update any indicators and do brace matching
   virtual void UpdateUI();
// @cmember do default folding 
   virtual void DoDefaultFolding(int nMargin, long lPosition);
// @cmember refresh lexer and display after new settings
   virtual void Refresh();
   virtual void DefineBookmark(int marker,DWORD colFg,DWORD colBg,int shape);
// @cmember add a bookmark = marker type 0
   virtual void AddBookmark(long lLine,int marker=0);
// @cmember delete a bookmark = marker type 0
   virtual void DeleteBookmark(long lLine,int marker=0);
   virtual void DeleteAllBookmark(int marker=0);
// @cmember check for bookmark = marker type 0
   virtual BOOL HasBookmark(long lLine,int marker=0);
// @cmember Find next bookmark
   virtual int FindNextBookmark(DWORD markers,int iStart=-1,BOOL bGoThere=TRUE);
// @cmember Find previous bookmark
   virtual int FindPreviousBookmark(DWORD markers,int iStart=-1,BOOL bGoThere=TRUE);

// @cmember goto line
   virtual void GotoLine(long lLine);
// @cmember goto position
   virtual void GotoPosition(long lPos);
// @cmember search forward for a given text,return the pos of the searched text,or return -1 if failed
   virtual int SearchForward(LPSTR szText,long lPosStart=-1,BOOL bGoThere=TRUE);
// @cmember search backward for a given text
   virtual BOOL SearchBackward(LPSTR szText);
   // @cmember search forward for the text at current position all the current selection
   virtual BOOL SearchForwardAuto();


// @cmember replace a text found by SearchForward or SearchBackward
   virtual void ReplaceSearchedText(LPCSTR szText);
// @cmember Set your own lexer
   virtual void SetLexer(int nLexer);
// @cmember return start position of selection
   virtual long GetSelectionStart();
// @cmember return end position of selection
   virtual long GetSelectionEnd();
// @cmember set selection range
   virtual void SetSelection(long startPos, long endPos);
// @cmember set selection range using line and column
   virtual void SetSelection(int startLine, int startColumn, int endLine, int endColumn);
// @cmember get selected text
   virtual const char *GetSelectedText();
// @cmember get the word at the current position
   virtual const char *GetCurWord(int pos=-1,const char *wordchars="");
   // @cmember get the word at the current position
   virtual const char *GetCurText();
// @cmember replace all in buffer or selection
   virtual int ReplaceAll(LPCSTR szFind, LPCSTR szReplace, BOOL bSelection = TRUE);

// @cmember set key words list
   virtual void SetKeywords(int set,LPCSTR keywords);

   virtual void SetWordChar(const char *chars);

   // @cmember set caret fore-color
   virtual void SetCaretFore(DWORD col);

   virtual void SetTipFore(DWORD col);
   virtual void SetTipBack(DWORD col);
   virtual void ShowTip(int pos,const char *content);
   virtual void HideTip();

   virtual void SetDwellTime(DWORD tick=1000);

   virtual int GetCharAt(int pos);

   virtual void EnablePopupMenu(BOOL bEnable);

   virtual int PositionFromPoint(int x,int y);
   virtual int LineFromPosition(int pos);
   virtual int PositionFromLine(int iLine);
   virtual int EndPosFromLine(int iLine);
   virtual const char *GetLineText(int iLine);


   virtual void AddText(const char *str);
   virtual void InsertText(const char *str,int pos=-1);
   virtual void AppendText(const char *str);

   virtual int GetLineCount();

   virtual void AC_Show(int lenEntered,const char *list);
   virtual void AC_Cancel();
   virtual BOOL AC_IsActive();
   virtual void AC_Complete();
   virtual void AC_SetStopChars(const char *chars);
   virtual void AC_SetMaxHeight(int row);

   // @cmember set the code page (encoding) for the document
   virtual void SetCodePage(int codePage);

protected:
// @access protected data members
// @cmember flag if we should display line numbers
   BOOL m_bLinenumbers;
// @cmember flag if we should display selection and bookmark margin
   BOOL m_bSelection;
// @cmember flag if we should display folding margin
   BOOL m_bFolding;
// @cmember search flags
   int  m_nSearchflags;

   DECLARE_MESSAGE_MAP()
   afx_msg void OnKillFocus(CWnd* );
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);



};
#define STR_SCINTILLAWND _T("Scintilla")
#define STR_LEXERDLL     _T("scilexer.dll")

