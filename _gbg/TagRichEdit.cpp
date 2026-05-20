#include "stdh.h"
#include "TagRichEdit.h"

#include "stringparser/stringparser.h"

#include <algorithm>

extern CStringW GetWindowTextAsUnicode(const CRichEditCtrl* pWnd);


IMPLEMENT_DYNAMIC(CTagRichEdit, CRichEditCtrl)

BEGIN_MESSAGE_MAP(CTagRichEdit, CRichEditCtrl)
	ON_WM_ERASEBKGND()
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	ON_NOTIFY_REFLECT(EN_PROTECTED, OnProtected)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_CHAR()
	ON_WM_PASTE()
	ON_MESSAGE(WM_NOTIFY + 1, OnSelectionChanged) // 自定义消息，用于捕获选择变化
END_MESSAGE_MAP()

CTagRichEdit::CTagRichEdit()
	: m_nNextTagID(1), m_bUpdating(false)
{
	// 初始化标签格式模板
	m_strTagFormat = _T("#$@");

}

CTagRichEdit::~CTagRichEdit()
{
}

// 在创建后设置通知消息，确保能收到EN_CHANGE
BOOL CTagRichEdit::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CRichEditCtrl::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

BOOL CTagRichEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CRichEditCtrl::Create(dwStyle, rect, pParentWnd, nID))
		return FALSE;
	
	// 设置接收EN_CHANGE通知
	SetEventMask(GetEventMask() | ENM_CHANGE);

	// 设置默认文本颜色为白色
	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_COLOR|CFM_BACKCOLOR; // 要设置的属性
	_tcscpy_s(cf.szFaceName, LF_FACESIZE, _T("微软雅黑"));   // 字体名
	cf.yHeight = 280;    // 字号（20pt = 400，10pt = 200，单位为1/20pt）
	cf.crTextColor = RGB(255, 255, 255);
	cf.crBackColor = RGB(0, 0, 0);
	cf.wWeight = FW_BOLD;
	SetDefaultCharFormat(cf);

	SetSel(0, -1);
	SetSelectionCharFormat(cf);

	m_defCF = cf;

	// 设置背景色为黑色
	SetBackgroundColor(FALSE, RGB(0, 0, 0));

	
	return TRUE;
}

void CTagRichEdit::GenerateTagMarkers(TagInfo& tagInfo)
{
	// 生成唯一的开始和结束标记
	CStringW strID;
	strID.Format(L"%03d", tagInfo.id);

	// 使用ASCII控制字符组合，如垂直制表符(VT, 0x0B)和文件分隔符(FS, 0x1C)
	// 这些字符通常不会显示，但可以存储在RichEdit中
	// tagInfo.startMarker = std::wstring(L"\x0B") + m_strTagFormat + L"_S_" + strID + std::wstring(L"\x1C");
	// tagInfo.endMarker = std::wstring(L"\x0B") + m_strTagFormat + L"_E_" + strID + std::wstring(L"\x1C");
	// tagInfo.startMarker = m_strTagFormat + L"S" + strID;
	tagInfo.startMarker = L"";
	tagInfo.endMarker = m_strTagFormat + L"E" + strID;
}

int CTagRichEdit::ReplaceSelWithTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription, COLORREF bgColor, COLORREF textColor)
{
	m_bUpdating = true;

	ReplaceSel("");

	// 获取当前插入位置
	long nStartPos = GetCurrentPosition();

	// 创建标签信息
	TagInfo tagInfo;
	tagInfo.id = m_nNextTagID++;
	tagInfo.text = lpszTagText;
	tagInfo.description = lpszDescription ? lpszDescription : _T("");
	tagInfo.bgColor = bgColor;
	tagInfo.textColor = textColor;

	// 生成唯一标记
	GenerateTagMarkers(tagInfo);

	// 插入格式化的标签文本，包括开始和结束标记
	SetSel(nStartPos, nStartPos);

	// 插入开始标记（设为不可见）
	CHARFORMAT2 cfHidden;
	ZeroMemory(&cfHidden, sizeof(cfHidden));
	cfHidden.cbSize = sizeof(cfHidden);
	cfHidden.dwMask = CFM_COLOR | CFM_HIDDEN;
	cfHidden.dwEffects = CFE_HIDDEN;
	SetSelectionCharFormat(cfHidden);

	CHARRANGE crStartMarker;
	crStartMarker.cpMin = nStartPos;
	ReplaceSel(CString(tagInfo.startMarker));
	crStartMarker.cpMax = nStartPos + tagInfo.startMarker.GetLength();

	// 插入标签文本（设置为可见，带格式）
	SetSel(crStartMarker.cpMax, crStartMarker.cpMax);
	CHARFORMAT2 cfVisible;
	ZeroMemory(&cfVisible, sizeof(cfVisible));
	cfVisible.cbSize = sizeof(cfVisible);
	cfVisible.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_BOLD;
	cfVisible.crTextColor = textColor;
	cfVisible.crBackColor = bgColor;
	cfVisible.dwEffects = CFE_BOLD;
	SetSelectionCharFormat(cfVisible);

	long nTagStart = crStartMarker.cpMax;
	ReplaceSel(lpszTagText);
	long nTagEnd = nTagStart + _tcslen(lpszTagText);

	// 插入结束标记（设为不可见）
	SetSel(nTagEnd, nTagEnd);
	SetSelectionCharFormat(cfHidden);
	ReplaceSel(CString(tagInfo.endMarker));

	// 设置标签位置
	tagInfo.position.cpMin = nTagStart;
	tagInfo.position.cpMax = nTagEnd;

	// 存储标签信息
	m_mapTags[tagInfo.id] = tagInfo;

	// 设置到末尾
	if (TRUE)
	{
		CHARRANGE cr;
		cr.cpMin = cr.cpMax = nTagEnd + tagInfo.endMarker.GetLength();
		SetSel(cr);
	}

	UpdateTagPositions();

	m_bUpdating = false;

	return tagInfo.id;
}

int CTagRichEdit::AddTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription, COLORREF bgColor, COLORREF textColor)
{
	if (!lpszTagText || _tcslen(lpszTagText) == 0)
		return -1;

	// 保存当前选择和位置
	int nLength = GetWindowTextLength();
	SetSel(nLength, nLength);

	return ReplaceSelWithTag(lpszTagText, lpszDescription, bgColor, textColor);
}

BOOL CTagRichEdit::RemoveTag(int nTagID)
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;

	m_bUpdating = true;

	// 查找标签的实际位置（通过标记）
	CHARRANGE crTag;
	if (FindTagPositionByMarkers(it->second.startMarker, it->second.endMarker, it->second.text.GetLength(), crTag))
	{
		// 删除标签文本和标记
		SetSel(crTag.cpMin, crTag.cpMax);
		ReplaceSel(_T(""));
	}
	else
	{
		// 使用存储的位置信息尝试删除
		SetSel(it->second.position);
		ReplaceSel(_T(""));
	}

	// 移除标签信息
	m_mapTags.erase(it);

	// 更新其他标签位置
	UpdateTagPositions();

	m_bUpdating = false;

	return TRUE;
}

BOOL CTagRichEdit::RemoveTagAtPosition(long nPosition)
{
	int nTagID = 0;
	if (IsCursorInTag(nPosition, nTagID))
	{
		return RemoveTag(nTagID);
	}

	return FALSE;
}

BOOL CTagRichEdit::GetTagInfo(int nTagID, TagInfo& tagInfo) const
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;

	tagInfo = it->second;
	return TRUE;
}

BOOL CTagRichEdit::SetTagDescription(int nTagID, LPCTSTR lpszDescription)
{
	auto it = m_mapTags.find(nTagID);
	if (it == m_mapTags.end())
		return FALSE;

	it->second.description = lpszDescription ? lpszDescription : _T("");
	return TRUE;
}

int CTagRichEdit::GetTagCount() const
{
	return static_cast<int>(m_mapTags.size());
}

void CTagRichEdit::GetAllTagIDs(std::vector<int>& tagIDs) const
{
	tagIDs.clear();
	tagIDs.reserve(m_mapTags.size());

	for (const auto& pair : m_mapTags)
	{
		tagIDs.push_back(pair.first);
	}
}

void CTagRichEdit::ClearAllTags()
{
	m_bUpdating = true;

	for (auto& pair : m_mapTags)
	{
		SetSel(pair.second.position);
		ReplaceSel(_T(""));
	}

	m_mapTags.clear();

	m_bUpdating = false;
}

int CTagRichEdit::GetTagIDAtPosition(long nPosition) const
{
	int nTagID = 0;
	if (IsCursorInTag(nPosition, nTagID))
	{
		return nTagID;
	}

	return -1;
}

void CTagRichEdit::OnEnChange()
{
	// 调试输出，检查是否被调用
	TRACE(_T("TagRichEdit::OnEnChange() called\n"));
	
	if (m_bUpdating)
		return;

	// 更新标签位置
	UpdateTagPositions();

	// 保护标签不被分割
	ProtectTags();
	
	// 重置所有非标签部分的格式
	ResetNonTagFormat();
}

void CTagRichEdit::OnProtected(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENPROTECTED* pEP = (ENPROTECTED*)pNMHDR;

	// 如果试图编辑标签内部，阻止操作
	if (pEP->msg == WM_KEYDOWN || pEP->msg == EM_REPLACESEL)
	{
		int nTagID = 0;
		// 检查操作位置是否在标签内
		if (IsCursorInTag(pEP->chrg.cpMin, nTagID) || IsCursorInTag(pEP->chrg.cpMax, nTagID))
		{
			*pResult = 1; // 阻止操作
			return;
		}

		// 检查操作是否影响了标记
		CStringW strText;
		if (TRUE)
		{
			strText = GetWindowTextAsUnicode(this);
			strText.Replace(L"\r\n", L"\n");
		}

		// 检查每个标签的标记是否受到影响
		for (const auto& pair : m_mapTags)
		{
			const TagInfo& tagInfo = pair.second;
			
			// 查找标记位置
			int nStartMarkerPos = strText.Find(tagInfo.startMarker);
			if (nStartMarkerPos != -1)
			{
				int nEndMarkerPos = strText.Find(tagInfo.endMarker, nStartMarkerPos);
				if (nEndMarkerPos != -1)
				{
					// 检查操作是否会影响标记本身
					if ((pEP->chrg.cpMin <= nStartMarkerPos && pEP->chrg.cpMax > nStartMarkerPos) ||
						(pEP->chrg.cpMin < nEndMarkerPos + tagInfo.endMarker.GetLength() && 
						 pEP->chrg.cpMax >= nEndMarkerPos + tagInfo.endMarker.GetLength()))
					{
						*pResult = 1; // 阻止操作
						return;
					}
				}
			}
		}
	}

	*pResult = 0; // 允许操作
}

void CTagRichEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// 处理删除键
	if (nChar == VK_DELETE || nChar == VK_BACK)
	{
		CHARRANGE cr;
		GetSel(cr);

		// 如果是选中状态（有选择范围）
		if (cr.cpMin != cr.cpMax)
		{
			// 扩展的选择范围，将包含与选择范围相交的所有标签
			CHARRANGE crExtended = cr;
			
			// 记录所有受影响的标签
			std::vector<int> tagsToDelete;
			
			// 检查所有标签，如果与选中范围相交，则合并范围
			for (const auto& pair : m_mapTags)
			{
				const TagInfo& tagInfo = pair.second;
				
				// 获取包含标记的完整标签范围
				CHARRANGE tagRange = tagInfo.GetRangeWithMarkers();
				
				// 检查标签范围是否与选中范围相交
				// 两个范围相交的条件：范围A的开始在范围B内，或范围A的结束在范围B内，或范围B完全包含在范围A内
				if ((tagRange.cpMin >= crExtended.cpMin && tagRange.cpMin < crExtended.cpMax) ||  // 标签开始在选择范围内
					(tagRange.cpMax > crExtended.cpMin && tagRange.cpMax <= crExtended.cpMax) ||  // 标签结束在选择范围内
					(tagRange.cpMin <= crExtended.cpMin && tagRange.cpMax >= crExtended.cpMax))   // 标签完全包含选择范围
				{
					// 扩展选择范围，确保包含整个标签和标记
					crExtended.cpMin = min(crExtended.cpMin, tagRange.cpMin);
					crExtended.cpMax = max(crExtended.cpMax, tagRange.cpMax);
					
					// 标记需要删除的标签
					tagsToDelete.push_back(pair.first);
				}
			}
			
			m_bUpdating = true;
			
			// 删除扩展后的范围
			SetSel(crExtended.cpMin, crExtended.cpMax);
			ReplaceSel(_T(""));
			
			// 从数据结构中删除相关标签
			for (int tagId : tagsToDelete)
			{
				auto it = m_mapTags.find(tagId);
				if (it != m_mapTags.end())
				{
					m_mapTags.erase(it);
				}
			}
			
			// 更新其他标签位置
			UpdateTagPositions();
			
			m_bUpdating = false;
			return;
		}
		else
		{
			// 单个位置的删除
			long pos = cr.cpMin;
			if (nChar == VK_BACK && pos > 0)
				pos--;
			if (nChar == VK_DELETE)
				pos++;

			int nTagID = 0;
			if (IsCursorInTag(pos, nTagID))
			{
				RemoveTag(nTagID);
				return;
			}
		}
	}
	// 处理方向键，确保光标不停留在标签内部
	else if (nChar == VK_LEFT || nChar == VK_RIGHT || nChar == VK_UP || nChar == VK_DOWN || 
              nChar == VK_HOME || nChar == VK_END)
	{
		// 先让默认处理完成
		CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
        
        // 获取处理后的光标位置
        CHARRANGE cr;
        GetSel(cr);
        
        // 确保是单光标而不是选择状态
        if (cr.cpMin == cr.cpMax)
        {
            int nTagID = 0;
            // 检查光标是否在标签内部
            if (IsCursorInTag(cr.cpMin, nTagID))
            {
                auto it = m_mapTags.find(nTagID);
                if (it != m_mapTags.end())
                {
                    // 根据光标位置和方向键决定将光标放在标签前面还是后面
                    long newPos;
                    
                    // 对于左、上、Home键通常向前移动，右、下、End键通常向后移动
                    if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_HOME)
                    {
                        // 将光标放在标签开始前
                        newPos = it->second.position.cpMin- it->second.startMarker.GetLength();
                    }
                    else // VK_RIGHT, VK_DOWN, VK_END
                    {
                        // 将光标放在标签结束后
                        newPos = it->second.position.cpMax+ it->second.endMarker.GetLength();
                    }
                    
                    // 设置新的光标位置
                    SetSel(newPos, newPos);
					SetSelectionCharFormat(m_defCF);
                    return;
                }
            }
			SetSelectionCharFormat(m_defCF);
        }
		else
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
				SetSel(cr.cpMin, cr.cpMax);
			return;
		}
        
        return;
	}

	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTagRichEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRichEditCtrl::OnLButtonDown(nFlags, point);

	// 获取点击位置
	CPoint ptl = { point.x, point.y };
	long nCharIndex = CharFromPos(ptl);

	// 检查点击位置是否在标签上
	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		// 选中整个标签
		auto it = m_mapTags.find(nTagID);
		if (it != m_mapTags.end())
		{
			CHARRANGE cr= it->second.position;
			cr.cpMin -= it->second.startMarker.GetLength();
			cr.cpMax+= it->second.endMarker.GetLength();

			SetSel(cr);
		}
	}
}

void CTagRichEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CRichEditCtrl::OnLButtonDblClk(nFlags, point);

	// 获取双击位置
	CPoint ptl = { point.x, point.y };
	long nCharIndex = CharFromPos(ptl);

	// 检查点击位置是否在标签上
	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		// 可以在此处理双击标签的行为
		// 例如显示标签描述等
		TagInfo tagInfo;
		if (GetTagInfo(nTagID, tagInfo))
		{
//			MessageBox(tagInfo.description, _T("标签信息"), MB_OK | MB_ICONINFORMATION);
		}
	}
}

void CTagRichEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// 获取鼠标位置对应的文本位置
	CPoint ptl;
	if (point.x == -1 && point.y == -1)
	{
		// 键盘触发的上下文菜单
		CHARRANGE cr;
		GetSel(cr);
		CPoint pt = PosFromChar(cr.cpMin);
		ClientToScreen((LPPOINT)&pt);
		point.x = pt.x;
		point.y = pt.y;
	}

	ScreenToClient(&point);
	ptl.x = point.x;
	ptl.y = point.y;
	long nCharIndex = CharFromPos(ptl);
	ClientToScreen(&point);

	// 检查位置是否在标签上
	int nTagID = 0;
	if (IsCursorInTag(nCharIndex, nTagID))
	{
		// 创建标签上下文菜单
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, 1, _T("删除标签"));
		menu.AppendMenu(MF_STRING, 2, _T("标签属性"));

		int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			point.x, point.y, this);

		switch (nCmd)
		{
		case 1: // 删除标签
			RemoveTag(nTagID);
			break;

		case 2: // 标签属性
		{
			TagInfo tagInfo;
			if (GetTagInfo(nTagID, tagInfo))
			{
// 				CStringW strInfo;
// 				strInfo.Format(_T("标签ID: %d\n标签文本: %s\n标签描述: %s"),
// 					tagInfo.id, tagInfo.text, tagInfo.description);
// 				MessageBox(strInfo, _T("标签属性"), MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		}
	}
	else
	{
		// 使用默认的上下文菜单
		CRichEditCtrl::OnContextMenu(pWnd, point);
	}
}

LRESULT CTagRichEdit::OnSelectionChanged(WPARAM wParam, LPARAM lParam)
{
	// 处理选择变化
	return 0;
}


void CTagRichEdit::DrawTag(const TagInfo& tagInfo)
{
	m_bUpdating = true;

	// 保存当前选择和格式
	CHARRANGE crOldSelection;
	GetSel(crOldSelection);

	// 设置选择范围为标签位置
	if (true)
	{
//		CHARRANGE pos = tagInfo.position;
		CHARRANGE pos = tagInfo.GetRangeWithMarkers();
		SetSel(pos);
	}

	// 设置字体格式
	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_BOLD;
	cf.crTextColor = tagInfo.textColor;
	cf.crBackColor = tagInfo.bgColor;
	cf.dwEffects = CFE_BOLD;
	SetSelectionCharFormat(cf);

	// 恢复选择
	SetSel(crOldSelection);

	m_bUpdating = false;
}

void CTagRichEdit::UpdateTagPositions()
{
	if (m_mapTags.empty())
		return;

	m_bUpdating = true;

	// 使用标记查找并更新每个标签的位置
	std::vector<int> tagsToRemove;
	for (auto& pair : m_mapTags)
	{
		TagInfo& tagInfo = pair.second;
		
		// 使用标记查找标签位置
		CHARRANGE crFullTag; // 包括标记的完整范围
		if (FindTagPositionByMarkers(tagInfo.startMarker, tagInfo.endMarker, tagInfo.text.GetLength(),crFullTag))
		{
			// 计算标签文本的实际位置（不包括标记）
			tagInfo.position.cpMin = crFullTag.cpMin + tagInfo.startMarker.GetLength();
			tagInfo.position.cpMax = crFullTag.cpMax - tagInfo.endMarker.GetLength();
			
			// 重新应用标签格式
			DrawTag(tagInfo);
		}
		else
		{
			// 查找失败，标记此标签将被删除
			tagsToRemove.push_back(pair.first);
		}
	}

	// 删除标记为待删除的标签
	for (int tagId : tagsToRemove)
	{
		m_mapTags.erase(tagId);
	}

	m_bUpdating = false;
}

BOOL CTagRichEdit::IsCursorInTag(long nPosition, int& nTagID) const
{
	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
		if (nPosition >= tagInfo.position.cpMin- tagInfo.startMarker.GetLength()+1 && nPosition < tagInfo.position.cpMax+ tagInfo.endMarker.GetLength())
		{
			nTagID = tagInfo.id;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CTagRichEdit::IsCharInTag(long nPosition, int& nTagID) const
{
	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
		if (nPosition >= tagInfo.position.cpMin - tagInfo.startMarker.GetLength() && nPosition < tagInfo.position.cpMax + tagInfo.endMarker.GetLength())
		{
			nTagID = tagInfo.id;
			return TRUE;
		}
	}

	return FALSE;
}

void CTagRichEdit::ProtectTags()
{
	// 重要：确保标记本身保持隐藏
	m_bUpdating = true;

	CHARRANGE crOldSelection;
	GetSel(crOldSelection);

	// 获取整个文本内容
	CStringW strText;
	if (TRUE)
	{
		strText = GetWindowTextAsUnicode(this);
		strText.Replace(L"\r\n", L"\n");
	}

	// 检查并保护所有标签的标记
	for (const auto& pair : m_mapTags)
	{
		const TagInfo& tagInfo = pair.second;
		
		// 查找标记位置
		int nStartMarkerPos = strText.Find(tagInfo.startMarker);
		if (nStartMarkerPos != -1)
		{
			// 保护开始标记
			CHARRANGE crMark;
			crMark.cpMin = nStartMarkerPos;
			crMark.cpMax = nStartMarkerPos + tagInfo.startMarker.GetLength();
			SetSel(crMark);
			
			// 设置为隐藏
			CHARFORMAT2 cf;
			ZeroMemory(&cf, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_HIDDEN;
			cf.dwEffects = CFE_HIDDEN;
			SetSelectionCharFormat(cf);
		}
		
		// 查找结束标记
		int nEndMarkerPos = strText.Find(tagInfo.endMarker, nStartMarkerPos + 1);
		if (nEndMarkerPos != -1)
		{
			// 保护结束标记
			CHARRANGE crMark;
			crMark.cpMin = nEndMarkerPos;
			crMark.cpMax = nEndMarkerPos + tagInfo.endMarker.GetLength();
			SetSel(crMark);
			
			// 设置为隐藏
			CHARFORMAT2 cf;
			ZeroMemory(&cf, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_HIDDEN;
			cf.dwEffects = CFE_HIDDEN;
			SetSelectionCharFormat(cf);
		}
	}
	
	// 恢复当前选择
	SetSel(crOldSelection);
	
	m_bUpdating = false;
}

long CTagRichEdit::GetCurrentPosition() const
{
	CHARRANGE cr;
	((CTagRichEdit*)this)->GetSel(cr);
	return cr.cpMin;
}


BOOL CTagRichEdit::FindTagPositionByMarkers(const CStringW& startMarker, const CStringW& endMarker, int tagLength,CHARRANGE& position)
{
	// 获取整个文本内容
	CStringW strText;
	if (TRUE)
	{
		strText = GetWindowTextAsUnicode(this);
		strText.Replace(L"\r\n", L"\n");
	}

	if (false)
	{
		// 查找开始标记
		int nStartPos = strText.Find(startMarker);
		if (nStartPos == -1)
			return FALSE;

		// 计算标签实际开始位置（跳过开始标记）
		int nTagStart = nStartPos + startMarker.GetLength();

		// 查找结束标记
		int nEndMarkerPos = strText.Find(endMarker, nTagStart);
		if (nEndMarkerPos == -1)
			return FALSE;

		// 设置位置信息
		position.cpMin = nStartPos;
		position.cpMax = nEndMarkerPos + endMarker.GetLength();
	}
	else
	{
		int nEndMarkerPos = strText.Find(endMarker);
		if (nEndMarkerPos == -1)
			return FALSE;

		position.cpMin = nEndMarkerPos-tagLength-startMarker.GetLength();
		position.cpMax = nEndMarkerPos + endMarker.GetLength();
	}
	
	return TRUE;
}

// 获取不含标记的内容
CStringW CTagRichEdit::GetContentString() const
{
	// 获取完整文本内容
	CStringW strContent;
	if (TRUE)
	{
		strContent = GetWindowTextAsUnicode(this);
		strContent.Replace(L"\r\n", L"\n");
	}

	// 创建一个标签信息的副本，并按照位置排序
	std::vector<TagInfo> sortedTags;
	for (const auto& pair : m_mapTags)
	{
		sortedTags.push_back(pair.second);
	}
	
	// 按位置逆序排序（从后向前处理，以免位置变化）
	std::sort(sortedTags.begin(), sortedTags.end(), 
		[](const TagInfo& a, const TagInfo& b) {
			return a.position.cpMin > b.position.cpMin;
		});
	
	// 从后向前删除所有标记
	for (const auto& tag : sortedTags)
	{
		// 删除结束标记
		if (!tag.endMarker.IsEmpty())
		{
			int endPos = tag.position.cpMax;
			int endLength = tag.endMarker.GetLength();
			if (endPos + endLength <= strContent.GetLength())
			{
				strContent.Delete(endPos, endLength);
			}
		}
		
		// 删除开始标记
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
	
// 	// 还原换行符为Windows格式
 	strContent.Replace(L"\n", L"\r\n");
	
	return strContent;
}

void CTagRichEdit::ResetNonTagFormat()
{
	if (m_mapTags.empty())
	{
		CHARRANGE crOldSelection;
		GetSel(crOldSelection);

		// 如果没有标签，直接设置整个文档格式
		SetSel(0, -1);
		SetSelectionCharFormat(m_defCF);

		SetSel(crOldSelection);

		return;
	}

	m_bUpdating = true;

	// 保存当前选择
	CHARRANGE crOldSelection;
	GetSel(crOldSelection);

	// 获取文档长度
	long nLength = 0;
	if (TRUE)
	{
		CStringW strText;
		strText = GetWindowTextAsUnicode(this);
		nLength= strText.GetLength();
	}

	// 收集所有标签的范围
	std::vector<CHARRANGE> tagRanges;
	for (const auto& pair : m_mapTags)
	{
		CHARRANGE range = pair.second.GetRangeWithMarkers();
		tagRanges.push_back(range);
	}

	// 按起始位置排序标签范围
	// 使用函数指针替代lambda表达式
	struct CompareRanges 
	{
		static bool Compare(const CHARRANGE& a, const CHARRANGE& b)
		{
			return a.cpMin < b.cpMin;
		}
	};
	std::sort(tagRanges.begin(), tagRanges.end(), CompareRanges::Compare);

	// 处理文档开始到第一个标签之前的部分
	if (!tagRanges.empty() && tagRanges[0].cpMin > 0)
	{
		SetSel(0, tagRanges[0].cpMin);
		SetSelectionCharFormat(m_defCF);
	}

	// 处理标签之间的部分
	for (size_t i = 0; i < tagRanges.size() - 1; i++)
	{
		if (tagRanges[i].cpMax < tagRanges[i + 1].cpMin)
		{
			SetSel(tagRanges[i].cpMax, tagRanges[i + 1].cpMin);
			SetSelectionCharFormat(m_defCF);
		}
	}

	// 处理最后一个标签之后到文档结束的部分
	if (!tagRanges.empty() && tagRanges.back().cpMax < nLength)
	{
		SetSel(tagRanges.back().cpMax, nLength);
		SetSelectionCharFormat(m_defCF);
	}

	// 恢复原始选择
	SetSel(crOldSelection);

	m_bUpdating = false;
}

void CTagRichEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
// 	if (nChar!=VK_RETURN)
// 		SetSelectionCharFormat(m_defCF);
// 	SetDefaultCharFormat(m_defCF);

	// 调用基类的处理方法
	CRichEditCtrl::OnChar(nChar, nRepCnt, nFlags);
	
// 	// 重置所有非标签部分的格式
// 	ResetNonTagFormat();
}

void CTagRichEdit::OnPaste()
{
	// 阻止默认的粘贴行为
	// 自己实现纯文本粘贴

	m_bUpdating = true;

	// 保存当前选择
	CHARRANGE crOldSelection;
	GetSel(crOldSelection);

	// 打开剪贴板
	if (!OpenClipboard())
	{
		m_bUpdating = false;
		return;
	}

	CString strText;
	bool bHaveText = false;

	// 优先获取Unicode文本
	HANDLE hClipboardData = GetClipboardData(CF_TEXT);
	if (hClipboardData)
	{
		// 锁定内存，获取指针
		LPSTR lpszClipboardText = (LPSTR)GlobalLock(hClipboardData);
		if (lpszClipboardText)
		{
			// 将ANSI文本转换为UNICODE
			strText = lpszClipboardText;
			bHaveText = true;
			// 解锁内存
			GlobalUnlock(hClipboardData);
		}
	}
	
	// 关闭剪贴板
	CloseClipboard();

	if (bHaveText)
	{
		// 在当前位置插入文本
		ReplaceSel(strText);
		
// 		// 应用默认格式
// 		CHARRANGE crNewSelection;
// 		crNewSelection.cpMin = crOldSelection.cpMin;
// 		crNewSelection.cpMax = crOldSelection.cpMin + strText.GetLength();
// 		SetSel(crNewSelection);
// 		SetSelectionCharFormat(m_defCF);
// 		
// 		// 将光标移到文本末尾
// 		SetSel(crNewSelection.cpMax, crNewSelection.cpMax);
// 		
// 		// 更新标签位置
// 		UpdateTagPositions();
// 		
// 		// 重置所有非标签部分的格式
// 		ResetNonTagFormat();
	}
	
	m_bUpdating = false;
}

void CTagRichEdit::GetContent(TagRichEditContent& content)
{
	CStringW text= GetWindowTextAsUnicode(this);

	extern std::string widechar_to_utf8(const wchar_t* str);
	content.m_text = widechar_to_utf8((LPCWSTR)text).c_str();

	content.m_mapTags = m_mapTags;
	content.m_nNextTagID = m_nNextTagID;
}

void CTagRichEdit::SetContent(TagRichEditContent& content)
{
	m_mapTags = content.m_mapTags;
	m_nNextTagID = content.m_nNextTagID;

	std::string s = (LPCSTR)content.m_text;
	extern std::wstring utf8_to_widechar(const std::string & utf8_str);
	std::wstring text = utf8_to_widechar(s);
	extern void SetUnicodeTextToRichEdit(CRichEditCtrl & edit, const wchar_t* str, bool replaceSel);
	SetUnicodeTextToRichEdit(*this, text.c_str(),false);
}
