#include "stdh.h"
#include "TagRichEditAtl.h"
#include <algorithm>

CTagRichEditAtl::CTagRichEditAtl()
	: m_nNextTagID(1), m_bUpdating(false)
{
	// 初始化标签格式模板
	m_strTagFormat = _T("#$@");
	
}

CTagRichEditAtl::~CTagRichEditAtl()
{
}

HWND CTagRichEditAtl::Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName, 
                             DWORD dwStyle, DWORD dwExStyle, UINT_PTR nID, LPVOID lpCreateParam)
{
    // Default styles for RichEdit if not provided
    if (dwStyle == 0)
        dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL;

	// CRichEditWindow (base for CWindowImpl) does not automatically load the RichEdit library.
	// Ensure RichEdit library is loaded. Modern versions of Windows usually load it automatically.
	// For older systems, or to be safe: AtlInitCommonControls(ICC_RICHEDIT_CLASSES); or LoadLibrary(TEXT("Riched20.dll")) or Msftedit.dll
	// This is typically done once in the application's startup code (e.g. WinMain or equivalent).
	// We assume it's loaded for now.

	HWND hWnd = CWindowImpl<CTagRichEditAtl, CWindow>::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	if (hWnd == NULL)
		return NULL;

	// 设置接收EN_CHANGE通知 (EM_SETEVENTMASK)
//	SetEventMask(GetEventMask() | ENM_CHANGE | ENM_SELCHANGE | ENM_PROTECTED); // Added ENM_SELCHANGE and ENM_PROTECTED for handlers

	// 设置默认文本颜色为白色 (EM_SETCHARFORMAT SCF_DEFAULT)
	ZeroMemory(&m_defCF, sizeof(m_defCF));
	m_defCF.cbSize = sizeof(m_defCF);
	m_defCF.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_COLOR | CFM_BACKCOLOR;
	//_tcscpy_s(m_defCF.szFaceName, LF_FACESIZE, _T("微软雅黑")); // Use lstrcpy or wcscpy_s for wide char
    lstrcpy(m_defCF.szFaceName, _T("微软雅黑"));
	m_defCF.yHeight = 280;    // 字号（20pt = 400，10pt = 200，单位为1/20pt）
	m_defCF.crTextColor = RGB(255, 255, 255);
	m_defCF.crBackColor = RGB(0, 0, 0);
	m_defCF.wWeight = FW_BOLD;
	SetDefaultCharFormat(m_defCF); // Sends EM_SETCHARFORMAT with SCF_DEFAULT

	SetSel(0, -1);
	SetSelectionCharFormat(m_defCF); // Sends EM_SETCHARFORMAT with SCF_SELECTION

	// 设置背景色为黑色 (EM_SETBKGNDCOLOR)
	SetBackgroundColor(FALSE, RGB(0, 0, 0));
	
	return hWnd;
}

void CTagRichEditAtl::GenerateTagMarkers(TagInfo& tagInfo)
{
	// 生成唯一的开始和结束标记
	ATL::CStringW strID;
	strID.Format(L"%03d", tagInfo.id);
	
	tagInfo.startMarker = L""; // As per original logic
	tagInfo.endMarker = m_strTagFormat + L"E" + strID;
}

long CTagRichEditAtl::GetCurrentPosition() const
{
	CHARRANGE cr;
	// ((CTagRichEditAtl*)this)->GetSel(cr); // MFC way
    ((CTagRichEditAtl*)this)->SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr); // const_cast if needed or make GetSel non-const
	return cr.cpMin;
}

int CTagRichEditAtl::ReplaceSelWithTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription, COLORREF bgColor, COLORREF textColor)
{
	m_bUpdating = true;

	SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L""); // ReplaceSel(L""); Corrected Linter Error

	long nStartPos = GetCurrentPosition();

	TagInfo tagInfo;
	tagInfo.id = m_nNextTagID++;
	tagInfo.text = lpszTagText;
	tagInfo.description = lpszDescription ? lpszDescription : _T("");
	tagInfo.bgColor = bgColor;
	tagInfo.textColor = textColor;

	GenerateTagMarkers(tagInfo);

	SendMessage(EM_EXSETSEL, 0, (LPARAM)&nStartPos); // SetSel(nStartPos, nStartPos);

	CHARFORMAT2 cfHidden;
	ZeroMemory(&cfHidden, sizeof(cfHidden));
	cfHidden.cbSize = sizeof(cfHidden);
	cfHidden.dwMask = CFM_COLOR | CFM_HIDDEN;
	cfHidden.dwEffects = CFE_HIDDEN;
	SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfHidden); // SetSelectionCharFormat(cfHidden);

	CHARRANGE crStartMarker;
	crStartMarker.cpMin = nStartPos;
	SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)tagInfo.startMarker); // ReplaceSel(CString(tagInfo.startMarker));
    CHARRANGE crCurrentSel;
    SendMessage(EM_EXGETSEL, 0, (LPARAM)&crCurrentSel);
	crStartMarker.cpMax = crCurrentSel.cpMin; // Position after replacing selection

	CHARRANGE crSetTagTextPos = { crStartMarker.cpMax, crStartMarker.cpMax };
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&crSetTagTextPos); // SetSel(crStartMarker.cpMax, crStartMarker.cpMax);
	CHARFORMAT2 cfVisible;
	ZeroMemory(&cfVisible, sizeof(cfVisible));
	cfVisible.cbSize = sizeof(cfVisible);
	cfVisible.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_BOLD;
	cfVisible.crTextColor = textColor;
	cfVisible.crBackColor = bgColor;
	cfVisible.dwEffects = CFE_BOLD;
	SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfVisible); // SetSelectionCharFormat(cfVisible);

	long nTagStart = crStartMarker.cpMax;
	SendMessage(EM_REPLACESEL, TRUE, (LPARAM)lpszTagText); // ReplaceSel(lpszTagText);
    SendMessage(EM_EXGETSEL, 0, (LPARAM)&crCurrentSel);
	long nTagEnd = crCurrentSel.cpMin;

	CHARRANGE crSetEndMarkerPos = { nTagEnd, nTagEnd };
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&crSetEndMarkerPos); // SetSel(nTagEnd, nTagEnd);
	SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfHidden); // SetSelectionCharFormat(cfHidden);
	SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)tagInfo.endMarker); // ReplaceSel(CString(tagInfo.endMarker));

	tagInfo.position.cpMin = nTagStart;
	tagInfo.position.cpMax = nTagEnd;

	m_mapTags[tagInfo.id] = tagInfo;

	if (TRUE) // Original logic
	{
        SendMessage(EM_EXGETSEL, 0, (LPARAM)&crCurrentSel);
		CHARRANGE cr;
		cr.cpMin = cr.cpMax = crCurrentSel.cpMin; // End of last replacement
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(cr);
	}

	UpdateTagPositions();
	m_bUpdating = false;
	return tagInfo.id;
}

int CTagRichEditAtl::AddTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription, COLORREF bgColor, COLORREF textColor)
{
	if (!lpszTagText || lstrlen(lpszTagText) == 0) // _tcslen -> lstrlen
		return -1;

	int nLength = GetWindowTextLength(); // WinAPI GetWindowTextLengthW(m_hWnd)
	CHARRANGE cr = { nLength, nLength };
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(nLength, nLength);

	return ReplaceSelWithTag(lpszTagText, lpszDescription, bgColor, textColor);
}

BOOL CTagRichEditAtl::RemoveTag(int nTagID)
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;

	m_bUpdating = true;

	CHARRANGE crTag;
	if (FindTagPositionByMarkers(it->second.startMarker, it->second.endMarker, it->second.text.GetLength(), crTag))
	{
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&crTag); // SetSel(crTag.cpMin, crTag.cpMax);
		SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L""); // ReplaceSel(_T(""));
	}
	else
	{
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&(it->second.position)); // SetSel(it->second.position);
		SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L""); // ReplaceSel(_T(""));
	}

	m_mapTags.erase(it);
	UpdateTagPositions();
	m_bUpdating = false;
	return TRUE;
}

BOOL CTagRichEditAtl::RemoveTagAtPosition(long nPosition)
{
	int nTagID = 0;
	if (IsCursorInTag(nPosition, nTagID))
	{
		return RemoveTag(nTagID);
	}
	return FALSE;
}

BOOL CTagRichEditAtl::GetTagInfo(int nTagID, TagInfo& tagInfo) const
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;
	tagInfo = it->second;
	return TRUE;
}

BOOL CTagRichEditAtl::SetTagDescription(int nTagID, LPCTSTR lpszDescription)
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;
	it->second.description = lpszDescription ? lpszDescription : _T("");
	return TRUE;
}

int CTagRichEditAtl::GetTagCount() const
{
	return static_cast<int>(m_mapTags.size());
}

void CTagRichEditAtl::GetAllTagIDs(std::vector<int>& tagIDs) const
{
	tagIDs.clear();
	tagIDs.reserve(m_mapTags.size());
	for (const auto& pair : m_mapTags)
	{
		tagIDs.push_back(pair.first);
	}
}

void CTagRichEditAtl::ClearAllTags()
{
	m_bUpdating = true;
	for (auto& pair : m_mapTags)
	{
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&(pair.second.position)); // SetSel(pair.second.position);
		SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L""); // ReplaceSel(_T(""));
	}
	m_mapTags.clear();
	m_bUpdating = false;
}

int CTagRichEditAtl::GetTagIDAtPosition(long nPosition) const
{
	int nTagID = 0;
	if (IsCursorInTag(nPosition, nTagID))
	{
		return nTagID;
	}
	return -1;
}

LRESULT CTagRichEditAtl::OnEnChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
	// TRACE(_T("TagRichEdit::OnEnChange() called\n")); // TRACE is MFC. Use ATLTRACE or OutputDebugString
	ATLTRACE(_T("TagRichEdit::OnEnChange() called\n"));

	if (m_bUpdating)
    {
        bHandled = TRUE; // Or FALSE if other handlers might need it
		return 0;
    }

	UpdateTagPositions();
	ProtectTags();
	ResetNonTagFormat();

	bHandled = FALSE; // Allow further processing if needed
	return 0;
}

LRESULT CTagRichEditAtl::OnProtected(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	ENPROTECTED* pEP = (ENPROTECTED*)pnmh;
	if (pEP->msg == WM_KEYDOWN || pEP->msg == EM_REPLACESEL)
	{
		int nTagID = 0;
		if (IsCursorInTag(pEP->chrg.cpMin, nTagID) || IsCursorInTag(pEP->chrg.cpMax, nTagID))
		{
			// *pResult = 1; // In ATL, returning non-zero from handler can mean handled.
            bHandled = TRUE; // Setting bHandled to TRUE stops further processing.
			return 1; // To signify protected
		}

		ATL::CStringW strText;
		int len = GetWindowTextLength();
        wchar_t* buffer = strText.GetBufferSetLength(len + 1);
        GetWindowText(buffer, len + 1);
        strText.ReleaseBuffer();
		strText.Replace(L"\r\n", L"\n"); // Corrected Linter Error

		for (const auto& pair : m_mapTags)
		{
			const TagInfo& tagInfo = pair.second;
			int nStartMarkerPos = strText.Find(tagInfo.startMarker);
			if (nStartMarkerPos != -1)
			{
				int nEndMarkerPos = strText.Find(tagInfo.endMarker, nStartMarkerPos);
				if (nEndMarkerPos != -1)
				{
					if ((pEP->chrg.cpMin <= nStartMarkerPos && pEP->chrg.cpMax > nStartMarkerPos) ||
						(pEP->chrg.cpMin < nEndMarkerPos + tagInfo.endMarker.GetLength() && 
						 pEP->chrg.cpMax >= nEndMarkerPos + tagInfo.endMarker.GetLength()))
					{
                        bHandled = TRUE;
						return 1; // Protected
					}
				}
			}
		}
	}

    bHandled = FALSE; // Allow operation
	return 0; // Allow operation
}

LRESULT CTagRichEditAtl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT nChar = (UINT)wParam;
	// UINT nRepCnt = LOWORD(lParam);
	// UINT nFlags = HIWORD(lParam);

	if (nChar == VK_DELETE || nChar == VK_BACK)
	{
		CHARRANGE cr;
		SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr); // GetSel(cr);

		if (cr.cpMin != cr.cpMax)
		{
			CHARRANGE crExtended = cr;
			std::vector<int> tagsToDelete;
			for (const auto& pair : m_mapTags)
			{
				const TagInfo& tagInfo = pair.second;
				CHARRANGE tagRange = tagInfo.GetRangeWithMarkers();
				if ((tagRange.cpMin >= crExtended.cpMin && tagRange.cpMin < crExtended.cpMax) ||
					(tagRange.cpMax > crExtended.cpMin && tagRange.cpMax <= crExtended.cpMax) ||
					(tagRange.cpMin <= crExtended.cpMin && tagRange.cpMax >= crExtended.cpMax))
				{
					crExtended.cpMin = std::min(crExtended.cpMin, tagRange.cpMin);
					crExtended.cpMax = std::max(crExtended.cpMax, tagRange.cpMax);
					tagsToDelete.push_back(pair.first);
				}
			}
			
			m_bUpdating = true;
			SendMessage(EM_EXSETSEL, 0, (LPARAM)&crExtended); // SetSel(crExtended.cpMin, crExtended.cpMax);
			SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L""); // ReplaceSel(_T(""));
			
			for (int tagId : tagsToDelete)
			{
				auto it = m_mapTags.find(tagId);
				if (it != m_mapTags.end())
				{
					m_mapTags.erase(it);
				}
			}
			UpdateTagPositions();
			m_bUpdating = false;
            bHandled = TRUE;
			return 0;
		}
		else
		{
			long pos = cr.cpMin;
			if (nChar == VK_BACK && pos > 0)
				pos--;
			if (nChar == VK_DELETE) // Position to check is current cursor for delete (character AT cursor)
            {
                // For VK_DELETE, the character to be deleted is at cr.cpMin.
                // IsCursorInTag checks if cr.cpMin is within a tag.
                // If we want to check the character *after* current pos (which is what VK_DELETE acts on)
                // we might need pos++ logic similar to original, but care for end of doc.
                // The original logic for VK_DELETE used pos++ *before* IsCursorInTag, which is unusual. Let's stick to current pos for now for IsCursorInTag.
            }

			int nTagID = 0;
            // For backspace, check character at pos-1 (which is stored in 'pos' if VK_BACK)
            // For delete, check character at pos (current cursor position)
			if (IsCursorInTag(pos, nTagID)) 
			{
				RemoveTag(nTagID);
                bHandled = TRUE;
				return 0;
			}
		}
	}
	else if (nChar == VK_LEFT || nChar == VK_RIGHT || nChar == VK_UP || nChar == VK_DOWN || 
              nChar == VK_HOME || nChar == VK_END)
	{
		bHandled = FALSE; // Allow default processing first
        // CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags); // Call DefWindowProc or set bHandled=FALSE
        // After default processing (which happens if bHandled is FALSE before returning), then check cursor.
        // This needs to be handled carefully. Post-processing cursor is tricky with bHandled.
        // A common ATL pattern is to set bHandled = FALSE, return 0, then in idle or a timer, check position.
        // Or, call DefWindowProc explicitly, then do the logic.

        DefWindowProc(uMsg, wParam, lParam); // Explicitly call default handler
        
        CHARRANGE cr;
        SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr); // GetSel(cr);
        
        if (cr.cpMin == cr.cpMax)
        {
            int nTagID = 0;
            if (IsCursorInTag(cr.cpMin, nTagID))
            {
                auto it = m_mapTags.find(nTagID);
                if (it != m_mapTags.end())
                {
                    long newPos;
                    if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_HOME)
                    {
                        newPos = it->second.position.cpMin - it->second.startMarker.GetLength();
                    }
                    else 
                    {
                        newPos = it->second.position.cpMax + it->second.endMarker.GetLength();
                    }
                    CHARRANGE crNewPos = {newPos, newPos};
                    SendMessage(EM_EXSETSEL, 0, (LPARAM)&crNewPos); // SetSel(newPos, newPos);
					SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);
                    bHandled = TRUE;
                    return 0;
                }
            }
			SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);
        }
		else // Selection exists
		{
			CHARRANGE oldCr = cr;
			int nTagID = 0;
			if (IsCursorInTag(cr.cpMin, nTagID))
			{
				auto it = m_mapTags.find(nTagID);
				if (it != m_mapTags.end())
				{
					cr.cpMin = it->second.position.cpMin - it->second.startMarker.GetLength();
				}
			}
			if (IsCursorInTag(cr.cpMax, nTagID))
			{
				auto it = m_mapTags.find(nTagID);
				if (it != m_mapTags.end())
				{
					cr.cpMax = it->second.position.cpMax + it->second.endMarker.GetLength();
				}
			}
			if (!((cr.cpMin==oldCr.cpMin)&& (cr.cpMax == oldCr.cpMax)))
            {
				SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(cr.cpMin, cr.cpMax);
            }
            bHandled = TRUE; // We handled it or adjusted selection
			return 0;
		}
        bHandled = TRUE; // Indicate we've handled arrow keys fully here
        return 0;
	}

	bHandled = FALSE; // Default processing for other keys
	return 0; // CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CTagRichEditAtl::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// UINT nFlags = (UINT)wParam;
	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	bHandled = FALSE; // Allow default processing first to set cursor/selection
    // CRichEditCtrl::OnLButtonDown(nFlags, point); // This will be called if bHandled is FALSE
    DefWindowProc(uMsg, wParam, lParam); // Call default handler first

	long nCharIndex = (long)SendMessage(EM_CHARFROMPOS, 0, (LPARAM)&point); // CharFromPos(ptl);

	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		auto it = m_mapTags.find(nTagID);
		if (it != m_mapTags.end())
		{
			CHARRANGE cr = it->second.position;
			cr.cpMin -= it->second.startMarker.GetLength();
			cr.cpMax += it->second.endMarker.GetLength();
			SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(cr);
            bHandled = TRUE; // We handled this to select the tag
		}
	}
	return 0;
}

LRESULT CTagRichEditAtl::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // UINT nFlags = (UINT)wParam;
	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	bHandled = FALSE; // Allow default processing
    // CRichEditCtrl::OnLButtonDblClk(nFlags, point);
    DefWindowProc(uMsg, wParam, lParam);

	long nCharIndex = (long)SendMessage(EM_CHARFROMPOS, 0, (LPARAM)&point); // CharFromPos(ptl);

	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		TagInfo tagInfo;
		if (GetTagInfo(nTagID, tagInfo))
		{
			// MessageBox(tagInfo.description, _T("标签信息"), MB_OK | MB_ICONINFORMATION);
            // Use ::MessageBox or AtlMessageBox
            ::MessageBox(m_hWnd, tagInfo.description, _T("标签信息"), MB_OK | MB_ICONINFORMATION);
            bHandled = TRUE; // We handled this
		}
	}
	return 0;
}

LRESULT CTagRichEditAtl::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// HWND hWndCtrl = (HWND)wParam; // Window where the click occurred (should be us)
	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	CPoint clientPoint = point;
	if (point.x == -1 && point.y == -1) // Keyboard context menu
	{
		CHARRANGE cr;
		SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr); // GetSel(cr);
        POINT ptScreen;
		SendMessage(EM_POSFROMCHAR, (WPARAM)&ptScreen, (LPARAM)cr.cpMin); // pt = PosFromChar(cr.cpMin);
        // EM_POSFROMCHAR returns client coordinates
        ClientToScreen(&ptScreen); // ClientToScreen((LPPOINT)&pt);
		point.x = ptScreen.x;
		point.y = ptScreen.y;
        clientPoint = ptScreen; // Keep clientPoint relative if possible
        ScreenToClient(&clientPoint);
	}
    else // Mouse context menu
    {
        ScreenToClient(&clientPoint);
    }

	long nCharIndex = (long)SendMessage(EM_CHARFROMPOS, 0, (LPARAM)&clientPoint); // CharFromPos(ptl);

	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		HMENU hMenu = CreatePopupMenu(); // CMenu menu; menu.CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _T("删除标签"));
        AppendMenu(hMenu, MF_STRING, 2, _T("标签属性"));

		int nCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                                point.x, point.y, 0, m_hWnd, NULL);
        DestroyMenu(hMenu);

		switch (nCmd)
		{
		case 1: RemoveTag(nTagID); break;
		case 2: 
			{
				TagInfo tagInfo;
				if (GetTagInfo(nTagID, tagInfo))
				{
					ATL::CStringW strInfo;
					strInfo.Format(_T("标签ID: %d\n标签文本: %s\n标签描述: %s"),
						tagInfo.id, (LPCTSTR)tagInfo.text, (LPCTSTR)tagInfo.description);
					::MessageBox(m_hWnd, strInfo, _T("标签属性"), MB_OK | MB_ICONINFORMATION);
				}
			}
			break;
		}
        bHandled = TRUE;
	}
	else
	{
		bHandled = FALSE; // Use default context menu CRichEditCtrl::OnContextMenu(pWnd, point);
	}
	return 0;
}

LRESULT CTagRichEditAtl::OnSelectionChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
    // SELCHANGE* pSelChange = (SELCHANGE*)pnmh;
    // Process selection change information from pSelChange->chrg
	// TRACE("Selection Changed: %d - %d\n", pSelChange->chrg.cpMin, pSelChange->chrg.cpMax);
    ATLTRACE(_T("Selection Changed\n")); // Basic trace for now
	bHandled = FALSE; // Allow further processing
	return 0;
}

void CTagRichEditAtl::DrawTag(const TagInfo& tagInfo)
{
	m_bUpdating = true;
	CHARRANGE crOldSelection;
	SendMessage(EM_EXGETSEL, 0, (LPARAM)&crOldSelection); // GetSel(crOldSelection);

	CHARRANGE pos = tagInfo.GetRangeWithMarkers();
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&pos); // SetSel(pos);

	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_BOLD;
	cf.crTextColor = tagInfo.textColor;
	cf.crBackColor = tagInfo.bgColor;
	cf.dwEffects = CFE_BOLD;
	SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf); // SetSelectionCharFormat(cf);

	SendMessage(EM_EXSETSEL, 0, (LPARAM)&crOldSelection); // SetSel(crOldSelection);
	m_bUpdating = false;
}

void CTagRichEditAtl::UpdateTagPositions()
{
	if (m_mapTags.empty())
		return;
	m_bUpdating = true;

	std::vector<int> tagsToRemove;
	for (auto& pair : m_mapTags)
	{
		TagInfo& tagInfo = pair.second;
		CHARRANGE crFullTag;
		if (FindTagPositionByMarkers(tagInfo.startMarker, tagInfo.endMarker, tagInfo.text.GetLength(), crFullTag))
		{
			tagInfo.position.cpMin = crFullTag.cpMin + tagInfo.startMarker.GetLength();
			tagInfo.position.cpMax = crFullTag.cpMax - tagInfo.endMarker.GetLength();
			DrawTag(tagInfo);
		}
		else
		{
			tagsToRemove.push_back(pair.first);
		}
	}

	for (int tagId : tagsToRemove)
	{
		m_mapTags.erase(tagId);
	}
	m_bUpdating = false;
}

BOOL CTagRichEditAtl::IsCursorInTag(long nPosition, int& nTagID) const
{
	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
        // Original logic for IsCursorInTag was:
        // nPosition >= tagInfo.position.cpMin - tagInfo.startMarker.GetLength() + 1 
        // && nPosition < tagInfo.position.cpMax + tagInfo.endMarker.GetLength()
        // This means cursor AT start marker is IN, cursor AT end marker is OUT.
        // Let's keep this logic if it was intentional for cursor behavior.
		if (nPosition >= (tagInfo.position.cpMin - tagInfo.startMarker.GetLength() + 1) && 
            nPosition < (tagInfo.position.cpMax + tagInfo.endMarker.GetLength()))
		{
			nTagID = tagInfo.id;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTagRichEditAtl::IsCharInTag(long nPosition, int& nTagID) const
{
	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
        // Original: nPosition >= tagInfo.position.cpMin - tagInfo.startMarker.GetLength() && nPosition < tagInfo.position.cpMax + tagInfo.endMarker.GetLength()
        // This means character AT start marker is IN, character AT end marker is OUT.
		if (nPosition >= (tagInfo.position.cpMin - tagInfo.startMarker.GetLength()) && 
            nPosition < (tagInfo.position.cpMax + tagInfo.endMarker.GetLength()))
		{
			nTagID = tagInfo.id;
			return TRUE;
		}
	}
	return FALSE;
}

void CTagRichEditAtl::ProtectTags()
{
	m_bUpdating = true;
	CHARRANGE crOldSelection;
	SendMessage(EM_EXGETSEL, 0, (LPARAM)&crOldSelection); // GetSel(crOldSelection);

	ATL::CStringW strText;
	int len = GetWindowTextLength();
    wchar_t* buffer = strText.GetBufferSetLength(len + 1);
    GetWindowText(buffer, len + 1);
    strText.ReleaseBuffer();
	strText.Replace(L"\r\n", L"\n"); // Corrected Linter Error

	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
		int nStartMarkerPos = strText.Find(tagInfo.startMarker);
		if (nStartMarkerPos != -1)
		{
			CHARRANGE crMark = {nStartMarkerPos, nStartMarkerPos + tagInfo.startMarker.GetLength() };
			SendMessage(EM_EXSETSEL, 0, (LPARAM)&crMark); // SetSel(crMark);
			CHARFORMAT2 cf;
			ZeroMemory(&cf, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_HIDDEN;
			cf.dwEffects = CFE_HIDDEN;
			SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf); // SetSelectionCharFormat(cf);
		}
		
		int nEndMarkerPos = strText.Find(tagInfo.endMarker, nStartMarkerPos != -1 ? nStartMarkerPos + 1 : 0);
		if (nEndMarkerPos != -1)
		{
			CHARRANGE crMark = { nEndMarkerPos, nEndMarkerPos + tagInfo.endMarker.GetLength() };
			SendMessage(EM_EXSETSEL, 0, (LPARAM)&crMark); // SetSel(crMark);
			CHARFORMAT2 cf;
			ZeroMemory(&cf, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_HIDDEN;
			cf.dwEffects = CFE_HIDDEN;
			SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf); // SetSelectionCharFormat(cf);
		}
	}
	
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&crOldSelection); // SetSel(crOldSelection);
	m_bUpdating = false;
}

BOOL CTagRichEditAtl::FindTagPositionByMarkers(const ATL::CString& startMarker, const ATL::CString& endMarker, int tagLength, CHARRANGE& position)
{
	ATL::CStringW strText;
	int len = GetWindowTextLength();
    wchar_t* buffer = strText.GetBufferSetLength(len + 1);
    GetWindowText(buffer, len + 1);
    strText.ReleaseBuffer();
	strText.Replace(L"\r\n", L"\n"); // Corrected Linter Error

	if (false) // Original if(false) block, kept for reference, but will use the else branch
	{
		// int nStartPos = strText.Find(startMarker);
		// if (nStartPos == -1) return FALSE;
		// int nTagStart = nStartPos + startMarker.GetLength();
		// int nEndMarkerPos = strText.Find(endMarker, nTagStart);
		// if (nEndMarkerPos == -1) return FALSE;
		// position.cpMin = nStartPos;
		// position.cpMax = nEndMarkerPos + endMarker.GetLength();
	}
	else // This was the active branch in original code
	{
		int nEndMarkerPos = strText.Find(endMarker);
		if (nEndMarkerPos == -1)
			return FALSE;

		position.cpMin = nEndMarkerPos - tagLength - startMarker.GetLength();
		position.cpMax = nEndMarkerPos + endMarker.GetLength();
        // Add a check to ensure cpMin is not negative
        if (position.cpMin < 0) return FALSE; 
	}
	return TRUE;
}

ATL::CString CTagRichEditAtl::GetContentString() const
{
	ATL::CStringW strContent;
	int len = ((CTagRichEditAtl*)this)->GetWindowTextLength(); // const_cast or make GetWindowTextLength non-const on m_hWnd
    wchar_t* buffer = strContent.GetBufferSetLength(len + 1);
    ((CTagRichEditAtl*)this)->GetWindowText(buffer, len + 1); // const_cast
    strContent.ReleaseBuffer();
	strContent.Replace(L"\r\n", L"\n"); // Corrected Linter Error

	std::vector<TagInfo> sortedTags;
	for (const auto& pair : m_mapTags)
	{
		sortedTags.push_back(pair.second);
	}
	
	std::sort(sortedTags.begin(), sortedTags.end(), 
		[](const TagInfo& a, const TagInfo& b) {
			return a.position.cpMin > b.position.cpMin;
		});
	
	for (const auto& tag : sortedTags)
	{
		if (!tag.endMarker.IsEmpty())
		{
			int endPos = tag.position.cpMax;
			int endLength = tag.endMarker.GetLength();
			if (endPos >= 0 && endPos + endLength <= strContent.GetLength())
			{
				strContent.Delete(endPos, endLength);
			}
		}
		
		if (!tag.startMarker.IsEmpty())
		{
			int startPos = tag.position.cpMin - tag.startMarker.GetLength();
			int startLength = tag.startMarker.GetLength();
			if (startPos >= 0 && startPos + startLength <= strContent.GetLength())
			{
				strContent.Delete(startPos, startLength);
			}
		}
	}
	
 	strContent.Replace(L"\n", L"\r\n");
	return ATL::CString(strContent); // Ensure final CString is of correct type if strContent is CStringW
}

void CTagRichEditAtl::ResetNonTagFormat()
{
	if (m_mapTags.empty())
	{
		CHARRANGE crOldSelection;
		SendMessage(EM_EXGETSEL, 0, (LPARAM)&crOldSelection); // GetSel(crOldSelection);

		CHARRANGE crAll = {0, -1};
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&crAll); // SetSel(0, -1);
		SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);

		SendMessage(EM_EXSETSEL, 0, (LPARAM)&crOldSelection); // SetSel(crOldSelection);
		return;
	}

	m_bUpdating = true;
	CHARRANGE crOldSelection;
	SendMessage(EM_EXGETSEL, 0, (LPARAM)&crOldSelection); // GetSel(crOldSelection);

	long nLength = GetWindowTextLength();

	std::vector<CHARRANGE> tagRanges;
	for (const auto& pair : m_mapTags)
	{
		CHARRANGE range = pair.second.GetRangeWithMarkers();
		tagRanges.push_back(range);
	}

	struct CompareRanges 
	{
		static bool Compare(const CHARRANGE& a, const CHARRANGE& b)
		{
			return a.cpMin < b.cpMin;
		}
	};
	std::sort(tagRanges.begin(), tagRanges.end(), CompareRanges::Compare);

	if (!tagRanges.empty() && tagRanges[0].cpMin > 0)
	{
		CHARRANGE cr = {0, tagRanges[0].cpMin};
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(0, tagRanges[0].cpMin);
		SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);
	}

	for (size_t i = 0; i < tagRanges.size() - 1; i++)
	{
		if (tagRanges[i].cpMax < tagRanges[i + 1].cpMin)
		{
			CHARRANGE cr = {tagRanges[i].cpMax, tagRanges[i + 1].cpMin};
			SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(tagRanges[i].cpMax, tagRanges[i + 1].cpMin);
			SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);
		}
	}

	if (!tagRanges.empty() && tagRanges.back().cpMax < nLength)
	{
		CHARRANGE cr = {tagRanges.back().cpMax, nLength};
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr); // SetSel(tagRanges.back().cpMax, nLength);
		SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_defCF); // SetSelectionCharFormat(m_defCF);
	}

	SendMessage(EM_EXSETSEL, 0, (LPARAM)&crOldSelection); // SetSel(crOldSelection);
	m_bUpdating = false;
}

LRESULT CTagRichEditAtl::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // UINT nChar = (UINT)wParam;
	// SetSelectionCharFormat(m_defCF); // This was original commented out logic
	// SetDefaultCharFormat(m_defCF);

	bHandled = FALSE; // Call base class processing // CRichEditCtrl::OnChar(nChar, nRepCnt, nFlags);
	// ResetNonTagFormat(); // Original commented out
	return 0;
}

LRESULT CTagRichEditAtl::OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bUpdating = true;

	// CHARRANGE crOldSelection;
	// SendMessage(EM_EXGETSEL, 0, (LPARAM)&crOldSelection); // GetSel(crOldSelection);

	if (!::OpenClipboard(m_hWnd)) // Use ::OpenClipboard with window handle
	{
		m_bUpdating = false;
        bHandled = TRUE; // We tried to handle it but failed
		return 0;
	}

	ATL::CString strText;
	bool bHaveText = false;

    HANDLE hClipboardData = ::GetClipboardData(CF_TEXT); // Use CF_UNICODETEXT if available and preferred
                                                        // CF_TEXT gives ANSI, CF_UNICODETEXT gives UTF-16
                                                        // Original code used CF_TEXT, so we stick to it.
	if (hClipboardData)
	{
		LPSTR lpszClipboardText = (LPSTR)::GlobalLock(hClipboardData);
		if (lpszClipboardText)
		{
			strText = lpszClipboardText; // ATL::CString will convert from ANSI to TCHAR (WCHAR if UNICODE)
			bHaveText = true;
			::GlobalUnlock(hClipboardData);
		}
	}
	
	::CloseClipboard();

	if (bHaveText)
	{
		SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)strText); // ReplaceSel(strText);
		// Original commented code for formatting pasted text not ported directly, focus on ATL conversion.
	}
	
m_bUpdating = false;
    bHandled = TRUE; // We handled the paste (or attempted to)
	return 0;
}

void CTagRichEditAtl::GetContent(TagRichEditContent& content)
{
    int len = GetWindowTextLength();
    wchar_t* buffer = content.m_text.GetBufferSetLength(len + 1);
	GetWindowText(buffer, len + 1); // GetWindowText(content.m_text);
    content.m_text.ReleaseBuffer();
	content.m_mapTags = m_mapTags;
	content.m_nNextTagID = m_nNextTagID;
}

void CTagRichEditAtl::SetContent(TagRichEditContent& content)
{
	m_mapTags = content.m_mapTags;
	m_nNextTagID = content.m_nNextTagID;
	SetWindowText(content.m_text); // SetWindowText(content.m_text);
    UpdateTagPositions(); // Refresh display based on new content and tags
    ProtectTags();
    ResetNonTagFormat();
}
