#include "stdh.h"
#include "ChatInputACItemTip.h"
#include <algorithm>
#include <cctype>
#include <sstream>

#include "stringparser/stringparser.h"

#include "Utils.h"

// GDI+相关
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//================== Tab 支持辅助函数 ==================

// Tab stop 列数（每个tab展开为几个空格宽度）
static const int kTabStopColumns = 4;

// 压缩文件路径以适应最大宽度
// 不断使用 OmitFullPath 对路径进行压缩，直到宽度满足要求或无法再压缩
// 返回最终需要的宽度
static int CompressFilePathToFit(
    std::wstring& pathW,           // 输入/输出：路径（宽字符）
    Gdiplus::Font* font,           // 字体
    int maxWidth,                  // 最大允许宽度
    int extraWidth,                // 额外宽度（边距、图标等）
    Graphics* g)                   // GDI+ 图形对象
{
    StringFormat fmt;
    fmt.SetFormatFlags(StringFormatFlagsNoWrap);
    RectF bound(0, 0, 2000, 1000);

    // 转换为 UTF-8 进行处理
    std::string pathUtf8 = widechar_to_utf8(pathW.c_str());
    
    // 循环压缩路径，直到宽度满足要求或无法再压缩
    while (!pathUtf8.empty())
    {
        pathW = utf8_to_widechar(pathUtf8.c_str());
        
        RectF size;
        g->MeasureString(pathW.c_str(), -1, font, bound, &fmt, &size);

        // 计算所需总宽度：文本宽度 + 额外宽度
        int requiredWidth = (int)std::ceil(size.Width) + extraWidth;
        
        // 如果宽度满足要求，返回所需宽度
        if (requiredWidth <= maxWidth)
        {
            return requiredWidth;
        }
        
        // 尝试进一步压缩路径
        if (!OmitFullPath(pathUtf8))
        {
            // 无法再压缩，返回当前所需宽度（可能会超出最大宽度）
            return requiredWidth;
        }
        // 继续循环，使用压缩后的路径重新测量
    }
    
    return extraWidth; // 空路径时返回额外宽度
}

// 获取单个空格的宽度（像素），用于tab展开计算
static REAL GetSpaceWidth(Graphics* g, Gdiplus::Font* font)
{
	StringFormat fmt;
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	RectF bound(0, 0, 2000, 1000);
	RectF measured;
	// 用两个字符做差来避免DrawString对空格的特殊处理
	g->MeasureString(L"x x", -1, font, bound, &fmt, &measured);
	RectF measuredX;
	g->MeasureString(L"xx", -1, font, bound, &fmt, &measuredX);
	REAL spaceW = measured.Width - measuredX.Width;
	if (spaceW <= 0.1f) spaceW = measured.Width / 3.0f; // 备用值
	return spaceW;
}

// 将含tab的字符串展开：返回各段文字及其x起始偏移（相对于originX=0）
// currentX: 当前绘制列x偏移（像素，相对于行起始），用于计算tab对齐
// tabWidth: 单个tab展开的像素宽（= spaceW * kTabStopColumns）
// 注意：输出 segments 中每个元素是 (子字符串, 该段起始x偏移)
static void ExpandTabsToSegments(
	const std::wstring& line,
	int startChar,   // 从第几个字符开始
	int charCount,   // 处理几个字符（-1表示到结尾）
	REAL startX,     // 该段在行中的初始X偏移（像素）
	REAL spaceW,     // 单个空格宽度
	Graphics* g,
	Gdiplus::Font* font,
	std::vector<std::pair<std::wstring, REAL>>& segments, // 输出：(文本段, 段起始X)
	REAL& endX       // 输出：该段结束后的X位置
)
{
	StringFormat fmt;
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	RectF bound(0, 0, 10000, 1000);

	REAL tabW = spaceW * kTabStopColumns;
	int total = (charCount < 0) ? (int)line.size() - startChar : charCount;
	int end = startChar + total;
	if (end > (int)line.size()) end = (int)line.size();

	REAL curX = startX;
	std::wstring buf;
	REAL bufStartX = curX;

	auto flushBuf = [&]() {
		if (!buf.empty()) {
			segments.emplace_back(buf, bufStartX);
			// 测量buf宽度，更新curX
			RectF measured;
			g->MeasureString(buf.c_str(), -1, font, bound, &fmt, &measured);
			curX += measured.Width;
			buf.clear();
		}
		bufStartX = curX;
	};

	for (int i = startChar; i < end; ++i)
	{
		wchar_t ch = line[i];
		if (ch == L'\t')
		{
			flushBuf();
			// 对齐到下一个tab stop
			// tab stop以tabW为单位对齐
			REAL nextTab = (std::floor(curX / tabW) + 1.0f) * tabW;
			curX = nextTab;
			bufStartX = curX;
		}
		else
		{
			buf += ch;
		}
	}
	flushBuf();

	endX = curX;
}

// 绘制含tab的字符串（单段，无高亮），返回结束X位置
static REAL DrawStringWithTabs(
	Graphics* g,
	const std::wstring& text,
	int startChar,
	int charCount,
	Gdiplus::Font* font,
	REAL x, REAL y,
	REAL spaceW,
	SolidBrush* brush
)
{
	StringFormat fmt;
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	std::vector<std::pair<std::wstring, REAL>> segments;
	REAL endX = x;
	ExpandTabsToSegments(text, startChar, charCount, x, spaceW, g, font, segments, endX);

	for (auto& seg : segments)
	{
		if (!seg.first.empty())
		{
			RectF layout(seg.second, y, 10000.0f, 1000.0f);
			g->DrawString(seg.first.c_str(), -1, font, layout, &fmt, brush);
		}
	}
	return endX;
}

// 测量含tab的字符串宽度（从startX开始），返回结束X位置（即宽度 = 返回值 - startX）
static REAL MeasureStringWithTabsEndX(
	Graphics* g,
	const std::wstring& text,
	int startChar,
	int charCount,
	Gdiplus::Font* font,
	REAL startX,
	REAL spaceW
)
{
	std::vector<std::pair<std::wstring, REAL>> segments;
	REAL endX = startX;
	ExpandTabsToSegments(text, startChar, charCount, startX, spaceW, g, font, segments, endX);
	return endX;
}

//================== 实现 ==================

BEGIN_MESSAGE_MAP(CChatInputACItemTip, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CChatInputACItemTip::CChatInputACItemTip()
	: _mode(Mode::None)
	, _textFont(nullptr)
	, _codeFont(nullptr)
	, _lineHeight(16.0f)
	, _lineSpacing(2)
	, _bufferBitmap(nullptr)
	, _bufferWidth(0)
	, _bufferHeight(0)
	, _paddingX(10)
	, _paddingY(8)
	, _maxVisibleLines(40)
	, _maxWidth(560)
{
	// 创建字体
	// 路径和代码都使用相同的字体，使用更窄的字体
	FontFamily monoFamily(L"Consolas");
	_textFont = new Gdiplus::Font(&monoFamily, 10.0f, FontStyleRegular, UnitPoint);
	_codeFont = new Gdiplus::Font(&monoFamily, 10.0f, FontStyleRegular, UnitPoint);
}

CChatInputACItemTip::~CChatInputACItemTip()
{
	if (_bufferBitmap) { delete _bufferBitmap; _bufferBitmap = nullptr; }
	if (_textFont) { delete _textFont; _textFont = nullptr; }
	if (_codeFont) { delete _codeFont; _codeFont = nullptr; }

	if (m_hWnd)
		DestroyWindow();
}

BOOL CChatInputACItemTip::CreateTipWindow(CWnd* pParent)
{
	static CString className = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
		::LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL);

	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		className, _T("ACItemTip"),
		WS_POPUP | WS_BORDER,
		0, 0, 200, 100,
		pParent ? pParent->GetSafeHwnd() : NULL, NULL);
}

void CChatInputACItemTip::ShowTip(const ChatInputACItem& item, int x, int y)
{
	_item = item;
	PrepareContent(item);

	// 计算窗口大小（使用临时DC创建Graphics）
	CClientDC dc(this);
	Graphics g(dc.GetSafeHdc());
	CSize size = CalculateWindowSize(&g);

	// 调整位置以不超出工作区
	RECT workArea; SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	if (x + size.cx > workArea.right) x = workArea.right - size.cx;
	if (x < workArea.left) x = workArea.left;
	if (y + size.cy > workArea.bottom) y = workArea.bottom - size.cy;
	if (y < workArea.top) y = workArea.top;

	SetWindowPos(&CWnd::wndTopMost, x, y, size.cx, size.cy,
		SWP_SHOWWINDOW | SWP_NOACTIVATE);

	Invalidate();
}

void CChatInputACItemTip::HideTip()
{
	if (IsWindowVisible())
		CWnd::ShowWindow(SW_HIDE);
}

void CChatInputACItemTip::PrepareContent(const ChatInputACItem& item)
{
	_displayLines.clear();
	_highlightRanges.clear();

	if (item.type == "file")
	{
		_mode = Mode::File;
		// 直接显示完整路径
		std::wstring w = utf8_to_widechar(item.fullPath.c_str());
		if (w.empty()) w = utf8_to_widechar(item.text.c_str());
		_displayLines.push_back(w);
		_highlightRanges.emplace_back(-1, 0); // 文件模式不需要高亮
		return;
	}
	else if (item.type == "symbol")
	{
		_mode = Mode::Symbol;

		// 首先添加文件路径行
		std::wstring pathW = utf8_to_widechar(item.fullPath.c_str());
		if (pathW.empty()) pathW = local_to_widechar(item.fullPath.c_str());
		_displayLines.push_back(pathW);
		_highlightRanges.emplace_back(-1, 0); // 路径行不需要高亮

		// 加载文件内容
		std::string contentUtf8;
		Utils::FileContentCodingFormat codingFmt;
		if (!Utils::GetFileContentIntoUTF8(item.fullPath.c_str(), contentUtf8, codingFmt))
		{
			std::wstring w = L"[Cannot read file]";
			_displayLines.push_back(w);
			_highlightRanges.emplace_back(-1, 0);
			return;
		}

		// 按行拆分
		std::vector<std::string> lines;
		{
			std::istringstream iss(contentUtf8);
			std::string line;
			while (std::getline(iss, line))
			{
				if (!line.empty() && line.back() == '\r') line.pop_back();
				lines.push_back(line);
			}
		}

		// 确定位置（使用 SingleLineLoc）
		int centerLine = 0; // 0-based index
		int symCol = -1;    // 0-based index of symbol start
		int symLen = 0;     // symbol length

		if (item.loc.line > 0 && item.loc.line <= (int)lines.size())
		{
			centerLine = item.loc.line;
			if (item.loc.startColumn > 0 && item.loc.endColumn > item.loc.startColumn)
			{
				symCol = item.loc.startColumn; // 转换为0-based
				symLen = item.loc.endColumn - item.loc.startColumn;
			}
		}

		// 上下文范围
		const int before = 10;
		const int after = 10;
		int start = max(0, centerLine - before);
		int end = min((int)lines.size() - 1, centerLine + after);

		_displayLines.reserve(end - start + 1);
		_highlightRanges.reserve(end - start + 1);

		for (int i = start; i <= end; ++i)
		{
			std::wstring wline = utf8_to_widechar(lines[i].c_str());
			_displayLines.push_back(wline);

			// 只在目标行添加高亮
			if (i == centerLine && symCol >= 0 && symLen > 0)
			{
				// 将UTF-8字节位置转换为UTF-16字符位置
				std::string utf8Line = lines[i];
				if (symCol < (int)utf8Line.size())
				{
					// 计算UTF-8字节位置对应的UTF-16字符位置
					std::string beforeSymbol = utf8Line.substr(0, symCol);
					std::wstring beforeSymbolW = utf8_to_widechar(beforeSymbol.c_str());
					int utf16Start = (int)beforeSymbolW.size();

					// 计算symbol的UTF-16长度
					int utf8End = min(symCol + symLen, (int)utf8Line.size());
					std::string symbolPart = utf8Line.substr(symCol, utf8End - symCol);
					std::wstring symbolPartW = utf8_to_widechar(symbolPart.c_str());
					int utf16Len = (int)symbolPartW.size();

					if (utf16Len > 0)
					{
						_highlightRanges.emplace_back(utf16Start, utf16Len);
					}
					else
					{
						_highlightRanges.emplace_back(-1, 0); // 无效高亮
					}
				}
				else
				{
					_highlightRanges.emplace_back(-1, 0); // 无效高亮
				}
			}
			else
			{
				_highlightRanges.emplace_back(-1, 0); // 非目标行，无高亮
			}
		}
		return;
	}

	_mode = Mode::None;
}

CSize CChatInputACItemTip::CalculateWindowSize(Graphics* g)
{
	// 基础边距
	int width = _paddingX * 2;
	int height = _paddingY * 2;

	StringFormat fmt; fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if (_mode == Mode::File)
	{
		// 文件模式：需要为图标预留空间
		int iconSpace = 20; // 图标 + 间距
		int fileMaxWidth = 800; // 文件模式最大宽度限制

		// 获取路径并尝试压缩直到满足宽度要求
		if (!_displayLines.empty())
		{
			// 额外宽度 = 图标空间 + 左右边距 + 10像素缓冲
			int extraWidth = iconSpace + _paddingX * 2 + 10;
			int requiredWidth = CompressFilePathToFit(_displayLines[0], _textFont, fileMaxWidth, extraWidth, g);
			width = min(fileMaxWidth, max(width, requiredWidth));
			
			// 文件模式使用固定行高 18
			height += 18;
		}
	}
	else if (_mode == Mode::Symbol)
	{
		REAL spaceW = GetSpaceWidth(g, _codeFont);

		for (size_t i = 0; i < _displayLines.size(); ++i)
		{
			const auto& w = _displayLines[i];
			RectF size;

			// 第一行是路径，使用 _textFont；其他行是代码，使用 _codeFont
			Gdiplus::Font* font = (i == 0) ? _textFont : _codeFont;

			if (i == 0)
			{
				// 文件路径行：尝试压缩路径以适应最大宽度
				// 额外宽度 = 左右边距 + 10像素缓冲
				int extraWidth = _paddingX * 2 + 10;
				int requiredWidth = CompressFilePathToFit(_displayLines[0], font, _maxWidth, extraWidth, g);
				width = min(_maxWidth, max(width, requiredWidth));
				height += 28; // 标题栏高度(24) + 间距(4)
			}
			else
			{
				// 代码行：用 tab 展开后测量真实宽度
				REAL lineEndX = MeasureStringWithTabsEndX(g, w, 0, -1, font, (REAL)_paddingX, spaceW);
				int lineWidth = (int)std::ceil(lineEndX); // 已包含 paddingX 起始偏移

				// 仅需要加右侧 padding
				int requiredWidth = lineWidth + _paddingX;
				width = min(_maxWidth, max(width, requiredWidth));

				// 使用固定行高 18.0f，与 DrawSymbolMode 绘制时保持一致
				height += 18;
			}
		}
		height += 4; // 额外空间
	}

	// 限制最大高度（可见行数）
	int maxH = _paddingY * 2 + (int)((_lineHeight + _lineSpacing) * _maxVisibleLines);
	height = min(height, maxH);

	if (width < 200) width = 200;

	return CSize(width, height);
}

void CChatInputACItemTip::EnsureBufferBitmap(int width, int height)
{
	if (!_bufferBitmap || _bufferWidth != width || _bufferHeight != height)
	{
		if (_bufferBitmap)
		{
			delete _bufferBitmap; _bufferBitmap = nullptr;
		}
		if (width > 0 && height > 0)
		{
			_bufferBitmap = new Bitmap(width, height, PixelFormat32bppARGB);
			_bufferWidth = width; _bufferHeight = height;
		}
	}
}

void CChatInputACItemTip::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; GetClientRect(&rc);

	Graphics screen(dc.GetSafeHdc());
	EnsureBufferBitmap(rc.Width(), rc.Height());
	if (!_bufferBitmap) return;

	Graphics g(_bufferBitmap);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	// 背景与边框（与ACWindow一致风格）
	SolidBrush bg(Color(255, 13, 17, 23));
	g.FillRectangle(&bg, 0, 0, rc.Width(), rc.Height());
	Pen border(Color(255, 48, 54, 61), 1.0f);
	g.DrawRectangle(&border, 0, 0, rc.Width() - 1, rc.Height() - 1);

	if (_mode == Mode::File)
		DrawFileMode(&g, rc);
	else if (_mode == Mode::Symbol)
		DrawSymbolMode(&g, rc);

	screen.DrawImage(_bufferBitmap, 0, 0);
}

BOOL CChatInputACItemTip::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CChatInputACItemTip::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (_bufferBitmap)
	{
		delete _bufferBitmap; _bufferBitmap = nullptr; _bufferWidth = 0; _bufferHeight = 0;
	}
}

void CChatInputACItemTip::DrawFileMode(Graphics* g, const CRect& rc)
{
	// 图标 + 路径文字
	// 小圆球（与ACWindow一致样式，白色）
	int left = _paddingX;
	int top = _paddingY;

	int ballSize = 12;
	int ballX = left;
	int ballY = top;
	GraphicsPath ballPath; ballPath.AddEllipse(ballX, ballY, ballSize, ballSize);
	PathGradientBrush gradient(&ballPath);
	gradient.SetCenterColor(Color(255, 255, 255, 255));
	Color surround[] = { Color(255, 200, 200, 200) }; int cnt = 1;
	gradient.SetSurroundColors(surround, &cnt);
	gradient.SetCenterPoint(PointF((REAL)(ballX + ballSize * 0.3f), (REAL)(ballY + ballSize * 0.3f)));
	g->FillEllipse(&gradient, ballX, ballY, ballSize, ballSize);
	SolidBrush highlight(Color(100, 255, 255, 255));
	g->FillEllipse(&highlight, ballX + ballSize / 4, ballY + ballSize / 4, ballSize / 3, ballSize / 3);

	int textLeft = left + ballSize + 8;

	// 路径文本 - 不使用省略号截断
	SolidBrush textBrush(Color(255, 240, 246, 252));
	StringFormat fmt; fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	RectF layout((REAL)textLeft, (REAL)top - 1, (REAL)(rc.right - _paddingX - textLeft), (REAL)(rc.bottom - _paddingY - top));
	for (size_t i = 0; i < _displayLines.size(); ++i)
	{
		g->DrawString(_displayLines[i].c_str(), -1, _textFont, layout, &fmt, &textBrush);
		layout.Y += 18.0f; // 路径文本行高
	}
}

void CChatInputACItemTip::DrawSymbolMode(Graphics* g, const CRect& rc)
{
	// 代码块渲染，使用前景色高亮
	SolidBrush codeBrush(Color(255, 240, 246, 252));
	SolidBrush pathBrush(Color(255, 255, 255, 255)); // 标题栏文本白色
	SolidBrush hlBrush(Color(255, 135, 206, 250)); // 天蓝色前景高亮
	SolidBrush titleBg(Color(255, 25, 45, 85)); // 标题栏背景色，深蓝色
	Pen titleBorder(Color(255, 70, 76, 83), 1.0f); // 标题栏底部边框

	StringFormat fmt; fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	StringFormat titleFmt;
	titleFmt.SetFormatFlags(StringFormatFlagsNoWrap);
	titleFmt.SetAlignment(StringAlignmentNear);
	titleFmt.SetLineAlignment(StringAlignmentCenter);

	REAL y = 0; // 标题栏从顶部开始
	REAL x = (REAL)_paddingX;
	REAL maxW = (REAL)(rc.Width() - _paddingX * 2);

	// 预先计算空格宽度，用于 tab 展开
	REAL spaceW = GetSpaceWidth(g, _codeFont);

	for (size_t i = 0; i < _displayLines.size(); ++i)
	{
		const std::wstring& wline = _displayLines[i];

		// 第一行是文件路径，使用标题栏样式（路径不含tab，直接绘制）
		if (i == 0)
		{
			REAL titleHeight = 24.0f; // 标题栏高度

			// 绘制标题栏背景（覆盖整个宽度）
			g->FillRectangle(&titleBg, 0.0f, y, (REAL)rc.Width(), titleHeight);

			// 绘制标题栏底部边框
			g->DrawLine(&titleBorder, 0.0f, y + titleHeight - 1, (REAL)rc.Width(), y + titleHeight - 1);

			// 绘制文件路径文本，垂直居中
			RectF titleLayout(x, y, maxW, titleHeight);
			g->DrawString(wline.c_str(), -1, _textFont, titleLayout, &titleFmt, &pathBrush);

			y += titleHeight + 4.0f; // 标题栏后增加一些间距
			continue;
		}

		// 代码行处理（需要支持 tab）
		auto& hlRange = _highlightRanges[i];

		// 检查是否有有效的高亮
		if (hlRange.first >= 0 && hlRange.second > 0)
		{
			int hlStart = hlRange.first;
			int hlLen = hlRange.second;
			int hlEnd = hlStart + hlLen;
			int lineLen = (int)wline.size();

			// 确保范围有效
			hlStart = max(0, min(hlStart, lineLen));
			hlEnd = max(hlStart, min(hlEnd, lineLen));
			hlLen = hlEnd - hlStart;

			// 绘制高亮前的部分（含tab展开）
			REAL curX = (REAL)_paddingX;
			if (hlStart > 0)
			{
				curX = DrawStringWithTabs(g, wline, 0, hlStart, _codeFont, curX, y, spaceW, &codeBrush);
			}

			// 绘制高亮部分（含tab展开）
			if (hlLen > 0)
			{
				curX = DrawStringWithTabs(g, wline, hlStart, hlLen, _codeFont, curX, y, spaceW, &hlBrush);
			}

			// 绘制高亮后的部分（含tab展开）
			if (hlEnd < lineLen)
			{
				DrawStringWithTabs(g, wline, hlEnd, lineLen - hlEnd, _codeFont, curX, y, spaceW, &codeBrush);
			}
		}
		else
		{
			// 无高亮，直接用 tab 展开绘制整行
			DrawStringWithTabs(g, wline, 0, -1, _codeFont, (REAL)_paddingX, y, spaceW, &codeBrush);
		}

		// 下一行
		y += 18.0f; // 行高
		x = (REAL)_paddingX;

		if (y > rc.bottom - _paddingY - 18) break; // 超出可见区域
	}
}


