#include "stdh.h"
#include "RepairWndG.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <sstream>

// 外部函数声明
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

//////////////////////////////////////////////////////////////////////////
// CRepairWndG

BEGIN_MESSAGE_MAP(CRepairWndG, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// 构造函数
CRepairWndG::CRepairWndG()
    : _gdiplusToken(0)
    , _font(nullptr)
    , _textBrush(nullptr)
    , _newCodeBrush(nullptr)
    , _backgroundBrush(nullptr)
    , _newCodeBackBrush(nullptr)
    , _lineHeight(16)
    , _charWidth(8)
    , _padding(8)
    , _isPendingShow(false)
{
    memset(&_pendingFocusRect, 0, sizeof(_pendingFocusRect));
}

// 析构函数
CRepairWndG::~CRepairWndG()
{
    _CleanupGDIPlus();
}

// 创建窗口
BOOL CRepairWndG::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 不设置背景画刷，我们自己绘制
        ::LoadIcon(NULL, IDI_APPLICATION));

    // 创建窗口
    BOOL result = CWnd::CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        className, _T("Repair Window"),
        WS_POPUP | WS_BORDER, rect, pParentWnd, nID);

    if (result)
    {
        _InitializeGDIPlus();
    }

    return result;
}

// 显示差异内容
void CRepairWndG::Show(const CodeComparingChars& content, const RECT& focusRect)
{
    // 先隐藏窗口
    ShowWindow(SW_HIDE);

    // 保存内容和focusRect
    _content = content;
    _pendingFocusRect = focusRect;
    _isPendingShow = true;

    // 处理内容
    _ProcessContent();

    // 计算窗口大小
    int width, height;
    _CalculateSize(width, height);

    // 计算窗口位置
    int x, y;
    _CalculateWindowPosition(focusRect, width, height, x, y);

    // 设置窗口大小和位置
    SetWindowPos(&wndTopMost, x, y, width, height,
        SWP_SHOWWINDOW | SWP_NOACTIVATE);

    _isPendingShow = false;
}

// 隐藏窗口
void CRepairWndG::Hide()
{
    ShowWindow(SW_HIDE);
    _isPendingShow = false;
    Clear();
}

// 清空内容
void CRepairWndG::Clear()
{
    _content.content.clear();
    _content.charTypes.clear();
    _lines.clear();
    _lineCharTypes.clear();
}

// 消息处理：大小变化
void CRepairWndG::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    Invalidate();
}

// 消息处理：创建
int CRepairWndG::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

// 消息处理：销毁
void CRepairWndG::OnDestroy()
{
    CWnd::OnDestroy();
}

// 消息处理：绘制
void CRepairWndG::OnPaint()
{
    CPaintDC dc(this);
    
    // 创建GDI+图形对象
    Gdiplus::Graphics graphics(dc.m_hDC);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    // 绘制内容
    _DrawContent(graphics);
}

// 消息处理：擦除背景
BOOL CRepairWndG::OnEraseBkgnd(CDC* pDC)
{
    // 不擦除背景，我们在OnPaint中自己绘制
    return TRUE;
}

//====================== 私有方法实现 ======================

// 初始化GDI+
void CRepairWndG::_InitializeGDIPlus()
{
    // 初始化GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);

    // 创建字体（等宽字体）
    _font = new Gdiplus::Font(L"Consolas", 11, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    if (!_font || _font->GetLastStatus() != Gdiplus::Ok)
    {
        delete _font;
        _font = new Gdiplus::Font(L"Courier New", 11, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    }

    // 创建画刷
    _textBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));           // 黑色文字
    _newCodeBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 128, 0));      // 绿色新代码文字
    _backgroundBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255)); // 白色背景
    _newCodeBackBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 230, 255, 230)); // 浅绿色新代码背景

    // 计算字符尺寸
    if (_font)
    {
        HDC hdc = ::GetDC(NULL);
        Gdiplus::Graphics graphics(hdc);
        
        Gdiplus::RectF boundingBox;
        graphics.MeasureString(L"M", 1, _font, Gdiplus::PointF(0, 0), &boundingBox);
        
        _charWidth = (int)boundingBox.Width;
        _lineHeight = (int)boundingBox.Height + 2; // 添加行间距
        
        ::ReleaseDC(NULL, hdc);
    }
}

// 清理GDI+
void CRepairWndG::_CleanupGDIPlus()
{
    delete _font;
    delete _textBrush;
    delete _newCodeBrush;
    delete _backgroundBrush;
    delete _newCodeBackBrush;

    _font = nullptr;
    _textBrush = nullptr;
    _newCodeBrush = nullptr;
    _backgroundBrush = nullptr;
    _newCodeBackBrush = nullptr;

    if (_gdiplusToken != 0)
    {
        Gdiplus::GdiplusShutdown(_gdiplusToken);
        _gdiplusToken = 0;
    }
}

// 处理内容，将字符串按行分割并保存字符类型
void CRepairWndG::_ProcessContent()
{
    _lines.clear();
    _lineCharTypes.clear();

    if (_content.content.empty())
        return;

    // 转换为宽字符串
    std::wstring wideContent = utf8_to_widechar(_content.content);
    
    // 按行分割
    std::wstringstream ss(wideContent);
    std::wstring line;
    size_t charIndex = 0;
    
    while (std::getline(ss, line))
    {
        _lines.push_back(line);
        
        // 保存这一行每个字符的类型
        std::vector<CodeComparingChars::CharType> lineTypes;
        for (size_t i = 0; i < line.length() && charIndex < _content.charTypes.size(); ++i, ++charIndex)
        {
            lineTypes.push_back(_content.charTypes[charIndex]);
        }
        
        // 跳过换行符
        if (charIndex < _content.charTypes.size())
        {
            charIndex++; // 跳过 \n
        }
        
        _lineCharTypes.push_back(lineTypes);
    }
    
    // 处理最后一行（如果没有换行符结尾）
    if (charIndex < wideContent.length())
    {
        line = wideContent.substr(charIndex);
        _lines.push_back(line);
        
        std::vector<CodeComparingChars::CharType> lineTypes;
        for (size_t i = 0; i < line.length() && charIndex < _content.charTypes.size(); ++i, ++charIndex)
        {
            lineTypes.push_back(_content.charTypes[charIndex]);
        }
        _lineCharTypes.push_back(lineTypes);
    }
}

// 计算窗口大小
void CRepairWndG::_CalculateSize(int& width, int& height)
{
    if (_lines.empty())
    {
        width = 200;
        height = 50;
        return;
    }

    // 计算最大行宽度
    int maxLineWidth = 0;
    for (const auto& line : _lines)
    {
        int lineWidth = (int)line.length() * _charWidth;
        if (lineWidth > maxLineWidth)
            maxLineWidth = lineWidth;
    }

    // 计算窗口尺寸
    width = maxLineWidth + _padding * 2;
    height = (int)_lines.size() * _lineHeight + _padding * 2;

    // 限制最小和最大尺寸
    const int MIN_WIDTH = 100;
    const int MIN_HEIGHT = 25;
    const int MAX_WIDTH = 800;
    const int MAX_HEIGHT = 600;

    width = max(MIN_WIDTH, min(MAX_WIDTH, width));
    height = max(MIN_HEIGHT, min(MAX_HEIGHT, height));
}

// 计算窗口位置（避免遮挡focusRect）
void CRepairWndG::_CalculateWindowPosition(const RECT& focusRect, int windowWidth, int windowHeight, int& x, int& y)
{
    // 获取屏幕工作区域
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    const int MARGIN = 10; // 与focusRect的间距

    // 优先尝试在focusRect右侧显示
    x = focusRect.right + MARGIN;
    y = focusRect.top;

    // 检查是否超出屏幕右边界
    if (x + windowWidth > workArea.right)
    {
        // 尝试在focusRect左侧显示
        x = focusRect.left - windowWidth - MARGIN;

        // 如果左侧也放不下，则在focusRect下方显示
        if (x < workArea.left)
        {
            x = focusRect.left;
            y = focusRect.bottom + MARGIN;

            // 如果下方也放不下，则在focusRect上方显示
            if (y + windowHeight > workArea.bottom)
            {
                y = focusRect.top - windowHeight - MARGIN;

                // 如果上方也放不下，则强制在屏幕内显示
                if (y < workArea.top)
                {
                    y = workArea.top;
                }
            }
        }
    }

    // 确保窗口在屏幕范围内
    if (x < workArea.left) x = workArea.left;
    if (y < workArea.top) y = workArea.top;
    if (x + windowWidth > workArea.right) x = workArea.right - windowWidth;
    if (y + windowHeight > workArea.bottom) y = workArea.bottom - windowHeight;
}

// 绘制内容
void CRepairWndG::_DrawContent(Gdiplus::Graphics& graphics)
{
    if (!_font || !_textBrush || !_backgroundBrush)
        return;

    // 获取客户区大小
    RECT clientRect;
    GetClientRect(&clientRect);
    
    // 绘制背景
    graphics.FillRectangle(_backgroundBrush, 0, 0, clientRect.right, clientRect.bottom);

    // 绘制边框
    Gdiplus::Pen borderPen(Gdiplus::Color(255, 128, 128, 128), 1);
    graphics.DrawRectangle(&borderPen, 0, 0, clientRect.right - 1, clientRect.bottom - 1);

    if (_lines.empty())
        return;

    // 绘制文本内容
    for (size_t lineIndex = 0; lineIndex < _lines.size(); ++lineIndex)
    {
        const std::wstring& line = _lines[lineIndex];
        const auto& lineTypes = _lineCharTypes[lineIndex];
        
        int y = _padding + (int)lineIndex * _lineHeight;
        int x = _padding;

        // 逐字符绘制以支持不同颜色
        for (size_t charIndex = 0; charIndex < line.length(); ++charIndex)
        {
            wchar_t ch = line[charIndex];
            std::wstring charStr(1, ch);
            
            // 确定字符类型和颜色
            CodeComparingChars::CharType charType = CodeComparingChars::Both;
            if (charIndex < lineTypes.size())
            {
                charType = lineTypes[charIndex];
            }

            // 绘制字符背景（如果是新代码）
            if (charType == CodeComparingChars::NewCode)
            {
                Gdiplus::RectF charRect((Gdiplus::REAL)x, (Gdiplus::REAL)y, 
                                       (Gdiplus::REAL)_charWidth, (Gdiplus::REAL)_lineHeight);
                graphics.FillRectangle(_newCodeBackBrush, charRect);
            }

            // 选择文字颜色
            Gdiplus::SolidBrush* brush = _textBrush;
            if (charType == CodeComparingChars::NewCode)
            {
                brush = _newCodeBrush;
            }

            // 绘制字符
            Gdiplus::PointF point((Gdiplus::REAL)x, (Gdiplus::REAL)y);
            graphics.DrawString(charStr.c_str(), 1, _font, point, brush);

            x += _charWidth;
        }
    }
}
