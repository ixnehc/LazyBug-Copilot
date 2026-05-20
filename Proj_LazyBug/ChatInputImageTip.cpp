#include "stdh.h"
#include "ChatInputImageTip.h"

#include <algorithm>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

using namespace Gdiplus;

//====================== CChatInputImageTip 实现 ======================

BEGIN_MESSAGE_MAP(CChatInputImageTip, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CChatInputImageTip::CChatInputImageTip()
	: _pImage(nullptr)
	, _bufferBitmap(nullptr)
	, _bufferWidth(0)
	, _bufferHeight(0)
	, _currentProcessId(0)
	, _currentFileSize(0)
	, _isMissingImage(false)
{
	// 获取当前进程ID
	_currentProcessId = GetCurrentProcessId();
}

CChatInputImageTip::~CChatInputImageTip()
{
	// 清理资源
	ReleaseImage();

	if (_bufferBitmap)
	{
		delete _bufferBitmap;
		_bufferBitmap = nullptr;
	}

	if (m_hWnd)
		DestroyWindow();
}

BOOL CChatInputImageTip::CreateTipWindow(CWnd* pParent)
{
	// 注册窗口类
	static CString className = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
		::LoadCursor(NULL, IDC_ARROW),
		NULL, // 设置背景画刷为NULL，防止系统擦除背景
		NULL);

	// 创建窗口
	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW|WS_EX_TRANSPARENT,
		className, _T("ImageTip"),
		WS_POPUP | WS_BORDER,
		0, 0, MIN_WINDOW_WIDTH, MIN_WINDOW_WIDTH + PATH_HEIGHT,
		pParent->GetSafeHwnd(), NULL);
}

bool CChatInputImageTip::IsSupportedImageFormat(const std::wstring& filePath)
{
	// 获取文件扩展名（小写）
	std::wstring ext = filePath;
	size_t dotPos = ext.find_last_of(L'.');
	if (dotPos == std::wstring::npos)
		return false;

	ext = ext.substr(dotPos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

	// 支持 png, jpg, jpeg, webp
	return (ext == L"png" || ext == L"jpg" || ext == L"jpeg" || ext == L"webp" || ext == L"bmp" || ext == L"gif" || ext == L"ico");
}

namespace {
	Gdiplus::Image* LoadImageWithWIC(const std::wstring& imagePath) {
		IWICImagingFactory* pFactory = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) return nullptr;

		IWICBitmapDecoder* pDecoder = nullptr;
		hr = pFactory->CreateDecoderFromFilename(imagePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
		if (FAILED(hr)) {
			pFactory->Release();
			return nullptr;
		}

		IWICBitmapFrameDecode* pFrame = nullptr;
		hr = pDecoder->GetFrame(0, &pFrame);
		if (FAILED(hr)) {
			pDecoder->Release();
			pFactory->Release();
			return nullptr;
		}

		IWICFormatConverter* pConverter = nullptr;
		hr = pFactory->CreateFormatConverter(&pConverter);
		Gdiplus::Bitmap* pBitmap = nullptr;
		if (SUCCEEDED(hr)) {
			hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
			if (SUCCEEDED(hr)) {
				UINT width = 0, height = 0;
				pConverter->GetSize(&width, &height);
				pBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppPARGB);
				Gdiplus::BitmapData bitmapData;
				Gdiplus::Rect rect(0, 0, width, height);
				if (pBitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppPARGB, &bitmapData) == Gdiplus::Ok) {
					pConverter->CopyPixels(nullptr, bitmapData.Stride, bitmapData.Stride * height, (BYTE*)bitmapData.Scan0);
					pBitmap->UnlockBits(&bitmapData);
				}
				else {
					delete pBitmap;
					pBitmap = nullptr;
				}
			}
			pConverter->Release();
		}

		pFrame->Release();
		pDecoder->Release();
		pFactory->Release();

		return pBitmap;
	}
}

bool CChatInputImageTip::LoadImage(const std::wstring& imagePath)
{
	// 先释放之前的图片
	ReleaseImage();

	// 保存当前图片路径
	_currentImagePath = imagePath;

	// 检查文件是否存在并获取文件大小
	WIN32_FILE_ATTRIBUTE_DATA fileAttrs;
	if (!GetFileAttributesExW(imagePath.c_str(), GetFileExInfoStandard, &fileAttrs))
	{
		// 文件不存在，设置为图片缺失状态
		_isMissingImage = true;
		_currentFileSize = 0;
		return true;
	}

	// 检查文件格式
	if (!IsSupportedImageFormat(imagePath))
	{
		_isMissingImage = true;
		_currentFileSize = 0;
		return true;
	}

	// 计算文件大小（字节）
	LARGE_INTEGER fileSize;
	fileSize.LowPart = fileAttrs.nFileSizeLow;
	fileSize.HighPart = fileAttrs.nFileSizeHigh;
	_currentFileSize = fileSize.QuadPart;

	// 使用 GDI+ 加载图片，如果失败（如 WebP），尝试使用 WIC 回退加载
	_pImage = Image::FromFile(imagePath.c_str());

	if (_pImage == nullptr || _pImage->GetLastStatus() != Ok)
	{
		if (_pImage)
		{
			delete _pImage;
			_pImage = nullptr;
		}
		
		// 尝试用 WIC 加载 (支持 WebP 等 GDI+ 不原生支持的格式，依赖系统是否安装 WebP 扩展)
		_pImage = LoadImageWithWIC(imagePath);
		
		if (_pImage == nullptr || _pImage->GetLastStatus() != Ok)
		{
			ReleaseImage();
			_isMissingImage = true;
			return true;
		}
	}

	_isMissingImage = false;
	return true;
}

void CChatInputImageTip::ReleaseImage()
{
	if (_pImage)
	{
		delete _pImage;
		_pImage = nullptr;
	}
	_currentImagePath.clear();
	_currentFileSize = 0;
	_isMissingImage = false;
}

CSize CChatInputImageTip::CalculateWindowSize(int imageWidth, int imageHeight)
{
	// 窗口高度上限：图片最大高度 + 路径区域 + 上下内边距
	const int maxWindowHeight = MAX_IMAGE_HEIGHT + PATH_HEIGHT + PADDING * 2;
	// 窗口宽度上限
	const int maxWindowWidth = MAX_WINDOW_WIDTH;
	// 图片可用区域的最大尺寸
	const int maxImageWidth = maxWindowWidth - PADDING * 2;
	const int maxImageHeight = MAX_IMAGE_HEIGHT;

	if (imageWidth <= 0 || imageHeight <= 0)
		return CSize(MIN_WINDOW_WIDTH, MIN_WINDOW_WIDTH + PATH_HEIGHT);

	// 计算图片在保持宽高比情况下，能放入最大可用区域的尺寸
	double aspectRatio = static_cast<double>(imageWidth) / static_cast<double>(imageHeight);

	int displayWidth = imageWidth;
	int displayHeight = imageHeight;

	// 先按宽度限制缩放
	if (displayWidth > maxImageWidth)
	{
		displayWidth = maxImageWidth;
		displayHeight = static_cast<int>(displayWidth / aspectRatio);
	}

	// 再按高度限制缩放
	if (displayHeight > maxImageHeight)
	{
		displayHeight = maxImageHeight;
		displayWidth = static_cast<int>(displayHeight * aspectRatio);
	}

	// 窗口宽度：图片宽度 + 两侧内边距，但不小于最小宽度
	int windowWidth = displayWidth + PADDING * 2;
	if (windowWidth < MIN_WINDOW_WIDTH)
		windowWidth = MIN_WINDOW_WIDTH;

	// 窗口高度：图片高度 + 路径区域 + 上下内边距，但不超过最大高度
	int windowHeight = displayHeight + PATH_HEIGHT + PADDING * 2;
	if (windowHeight > maxWindowHeight)
		windowHeight = maxWindowHeight;

	return CSize(windowWidth, windowHeight);
}

CRect CChatInputImageTip::CalculateImageRect(int clientWidth, int clientHeight)
{
	// 图片显示区域起点（路径区域下方）
	const int startY = PATH_HEIGHT + PADDING;
	// 可用的图片显示区域
	const int availWidth = clientWidth - PADDING * 2;
	const int availHeight = clientHeight - PATH_HEIGHT - PADDING * 2;

	if (_pImage == nullptr)
		return CRect(PADDING, startY, PADDING + availWidth, startY + availHeight);

	int imageWidth = _pImage->GetWidth();
	int imageHeight = _pImage->GetHeight();

	if (imageWidth <= 0 || imageHeight <= 0)
		return CRect(PADDING, startY, PADDING + availWidth, startY + availHeight);

	// 计算保持宽高比的显示尺寸（不超出可用区域，允许留空）
	double aspectRatio = static_cast<double>(imageWidth) / static_cast<double>(imageHeight);

	int displayWidth = imageWidth;
	int displayHeight = imageHeight;

	// 先按可用宽度限制
	if (displayWidth > availWidth)
	{
		displayWidth = availWidth;
		displayHeight = static_cast<int>(displayWidth / aspectRatio);
	}

	// 再按可用高度限制
	if (displayHeight > availHeight)
	{
		displayHeight = availHeight;
		displayWidth = static_cast<int>(displayHeight * aspectRatio);
	}

	// 居中显示
	int x = PADDING + (availWidth - displayWidth) / 2;
	int y = startY + (availHeight - displayHeight) / 2;

	return CRect(x, y, x + displayWidth, y + displayHeight);
}

void CChatInputImageTip::ShowImage(const std::wstring& imagePath, int x, int y)
{
	// 加载新图片（如果需要）
	if (!IsShowingImage(imagePath) || !IsWindowVisible())
	{
		if (!LoadImage(imagePath))
		{
			HideWindow();
			return;
		}
	}

	// 计算窗口大小
	int imgWidth = _isMissingImage ? 0 : _pImage->GetWidth();
	int imgHeight = _isMissingImage ? 0 : _pImage->GetHeight();
	CSize windowSize = CalculateWindowSize(imgWidth, imgHeight);

	// 获取屏幕工作区
	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	// 调整位置以避免超出屏幕右边界和底部
	if (x + windowSize.cx > workArea.right)
		x = workArea.right - windowSize.cx;
	if (y + windowSize.cy > workArea.bottom)
		y = workArea.bottom - windowSize.cy;
	if (x < workArea.left)
		x = workArea.left;
	if (y < workArea.top)
		y = workArea.top;

	// 设置窗口位置和大小
	SetWindowPos(&CWnd::wndTopMost, x, y, windowSize.cx, windowSize.cy,
		SWP_SHOWWINDOW | SWP_NOACTIVATE);

	Invalidate();
}

BOOL CChatInputImageTip::ShowImage(const std::wstring& imagePath, int x, int y, const CRect& avoidRect)
{
	// 加载新图片（如果需要）
	if (!IsShowingImage(imagePath) || !IsWindowVisible())
	{
		if (!LoadImage(imagePath))
		{
			HideWindow();
			return FALSE;
		}
	}

	// 计算窗口大小
	int imgWidth = _isMissingImage ? 0 : _pImage->GetWidth();
	int imgHeight = _isMissingImage ? 0 : _pImage->GetHeight();
	CSize windowSize = CalculateWindowSize(imgWidth, imgHeight);

	// 获取屏幕工作区（使用 avoidRect 所在监视器的工作区，支持多显示器）
	HMONITOR hMon = MonitorFromRect(&avoidRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMon, &mi);
	CRect workArea(mi.rcWork);

	// -----------------------------------------------------------------------
	// 辅助 lambda
	// -----------------------------------------------------------------------

	// 检查矩形是否完全在工作区内
	auto isInWorkArea = [&workArea](const CRect& r) -> bool {
		return r.left   >= workArea.left  &&
			   r.top    >= workArea.top   &&
			   r.right  <= workArea.right &&
			   r.bottom <= workArea.bottom;
	};

	// 检查两个矩形是否相交（含边相切视为不相交，不遮挡即可）
	auto rectsIntersect = [](const CRect& a, const CRect& b) -> bool {
		return a.left < b.right && a.right > b.left &&
			   a.top  < b.bottom && a.bottom > b.top;
	};

	// 将值夹紧到 [lo, hi]
	auto clamp = [](int v, int lo, int hi) -> int {
		return v < lo ? lo : (v > hi ? hi : v);
	};

	// -----------------------------------------------------------------------
	// 核心：对给定的主轴坐标，将副轴坐标 clamp 到工作区内后检验是否合法。
	//
	// 调用约定：
	//   tryPlace(left, top) —— 直接以 (left,top) 为左上角，
	//                          内部对坐标做 clamp 保证不超出工作区，
	//                          然后检查是否与 avoidRect 不相交。
	//   返回 true 表示该位置合法，同时更新 windowRect。
	// -----------------------------------------------------------------------
	CRect windowRect;

	auto tryPlace = [&](int left, int top) -> bool {
		// 先 clamp，确保窗口完整地落在工作区内
		int L = clamp(left, workArea.left,  workArea.right  - windowSize.cx);
		int T = clamp(top,  workArea.top,   workArea.bottom - windowSize.cy);
		CRect r(L, T, L + windowSize.cx, T + windowSize.cy);
		// 工作区可能本身就放不下（太小），额外校验
		if (!isInWorkArea(r))
			return false;
		if (rectsIntersect(r, avoidRect))
			return false;
		windowRect = r;
		return true;
	};

	// -----------------------------------------------------------------------
	// 按优先级逐一尝试 8 个方向：
	//   优先级：左 -> 右 -> 上 -> 下
	//   每个方向先尝试与 avoidRect 对齐，再尝试与传入的 (x,y) 对齐。
	// -----------------------------------------------------------------------
	bool foundPosition = false;

	// ---------- 1. 左侧（最优先） ----------
	// 1.1 左方，Y 与 avoidRect.top 对齐（副轴 clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.left - windowSize.cx, avoidRect.top);

	// 1.2 左方，Y 使用参考 y（clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.left - windowSize.cx, y);

	// ---------- 2. 右侧 ----------
	// 2.1 右方，Y 与 avoidRect.top 对齐（副轴 clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.right, avoidRect.top);

	// 2.2 右方，Y 使用参考 y（clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.right, y);

	// ---------- 3. 上方 ----------
	// 3.1 上方，X 与 avoidRect.left 对齐（副轴 clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.left, avoidRect.top - windowSize.cy);

	// 3.2 上方，X 使用参考 x（clamp）
	if (!foundPosition)
		foundPosition = tryPlace(x, avoidRect.top - windowSize.cy);

	// ---------- 4. 下方（最后） ----------
	// 4.1 下方，X 与 avoidRect.left 对齐（副轴 clamp）
	if (!foundPosition)
		foundPosition = tryPlace(avoidRect.left, avoidRect.bottom);

	// 4.2 下方，X 使用参考 x（clamp）
	if (!foundPosition)
		foundPosition = tryPlace(x, avoidRect.bottom);

	// ---------- 兜底：工作区内找任意不重叠角落 ----------
	// 按「左上 → 右上 → 左下 → 右下」顺序尝试工作区四个角
	if (!foundPosition)
		foundPosition = tryPlace(workArea.left, workArea.top);
	if (!foundPosition)
		foundPosition = tryPlace(workArea.right - windowSize.cx, workArea.top);
	if (!foundPosition)
		foundPosition = tryPlace(workArea.left, workArea.bottom - windowSize.cy);
	if (!foundPosition)
		foundPosition = tryPlace(workArea.right - windowSize.cx, workArea.bottom - windowSize.cy);

	// ---------- 最终兜底：强制夹紧显示，不再隐藏窗口 ----------
	// 若工作区实在放不下（窗口比工作区还大），仍然尽量显示，
	// 而不是直接 return FALSE 让用户什么都看不到。
	if (!foundPosition)
	{
		return FALSE;
// 		int L = clamp(x, workArea.left, workArea.right  - windowSize.cx);
// 		int T = clamp(y, workArea.top,  workArea.bottom - windowSize.cy);
// 		windowRect.SetRect(L, T, L + windowSize.cx, T + windowSize.cy);
	}

	// 设置窗口位置和大小
	SetWindowPos(&CWnd::wndTopMost, windowRect.left, windowRect.top,
		windowSize.cx, windowSize.cy,
		SWP_SHOWWINDOW | SWP_NOACTIVATE);

	Invalidate();
	return TRUE;
}

void CChatInputImageTip::HideWindow()
{
	if (IsWindowVisible())
	{
		CWnd::ShowWindow(SW_HIDE);
	}
}

bool CChatInputImageTip::IsShowingImage(const std::wstring& imagePath) const
{
	// 标准化路径比较（大小写不敏感）
	if (imagePath.length() != _currentImagePath.length())
		return false;

	return _wcsicmp(imagePath.c_str(), _currentImagePath.c_str()) == 0;
}

void CChatInputImageTip::Update()
{
	CheckForegroundWindow();
}

void CChatInputImageTip::CheckForegroundWindow()
{
	// 获取前台窗口
	HWND foregroundWnd = ::GetForegroundWindow();
	if (foregroundWnd == NULL)
		return;

	// 获取前台窗口的进程ID
	DWORD foregroundProcessId = 0;
	GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);

	// 如果前台窗口不属于当前进程，隐藏窗口
	if (foregroundProcessId != _currentProcessId)
	{
		HideWindow();
	}
}

void CChatInputImageTip::OnPaint()
{
	CPaintDC dc(this);
	DrawDoubleBuffered(&dc);
}

void CChatInputImageTip::DrawDoubleBuffered(CDC* pDC)
{
	CRect clientRect;
	GetClientRect(&clientRect);
	int width = clientRect.Width();
	int height = clientRect.Height();

	// 确保缓冲位图有效
	EnsureBufferBitmap(width, height);

	if (!_bufferBitmap)
		return;

	// 在缓冲位图上绘制
	Graphics bufferGraphics(_bufferBitmap);
	bufferGraphics.SetSmoothingMode(SmoothingModeAntiAlias);
	bufferGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	DoPaint(&bufferGraphics, width, height);

	// 将缓冲位图绘制到屏幕
	Graphics screenGraphics(pDC->GetSafeHdc());
	screenGraphics.DrawImage(_bufferBitmap, 0, 0);
}

// 格式化文件大小为英文单位字符串（B, k, M）
static std::wstring FormatFileSize(LONGLONG size)
{
	const LONGLONG KB = 1024;
	const LONGLONG MB = 1024 * KB;
	const LONGLONG GB = 1024 * MB;

	wchar_t buffer[64];
	if (size >= GB)
	{
		swprintf_s(buffer, L"%.2f G", size / (double)GB);
	}
	else if (size >= MB)
	{
		swprintf_s(buffer, L"%.2f M", size / (double)MB);
	}
	else if (size >= KB)
	{
		swprintf_s(buffer, L"%.1f k", size / (double)KB);
	}
	else
	{
		swprintf_s(buffer, L"%lld B", size);
	}
	return std::wstring(buffer);
}

void CChatInputImageTip::DoPaint(Graphics* graphics, int width, int height)
{
	// 背景色
	SolidBrush backgroundBrush(Color(255, 13, 17, 23)); // GitHub深色主题
	graphics->FillRectangle(&backgroundBrush, 0, 0, width, height);

	// 边框
	Pen borderPen(Color(255, 48, 54, 61), 1.0f);
	graphics->DrawRectangle(&borderPen, 0, 0, width - 1, height - 1);

	// 绘制文件路径（顶部区域）
	if (!_currentImagePath.empty())
	{
		// 路径背景
		SolidBrush pathBgBrush(Color(255, 22, 27, 34));
		graphics->FillRectangle(&pathBgBrush, 1, 1, width - 2, PATH_HEIGHT);

		// 路径文本 - 使用更亮的颜色
		FontFamily fontFamily(L"Segoe UI");
		Gdiplus::Font font(&fontFamily, 9, FontStyleRegular, UnitPoint);
		SolidBrush textBrush(Color(255, 240, 246, 252)); // 更亮的白色

		// 计算文件名（取最后一部分）
		std::wstring displayPath = _currentImagePath;
		size_t lastSlash = displayPath.find_last_of(L"\\/");
		if (lastSlash != std::wstring::npos)
		{
			displayPath = displayPath.substr(lastSlash + 1);
		}

		// 格式化图片尺寸和大小（宽x高(大小)）
		std::wstring infoStr;
		if (_pImage && !_isMissingImage)
		{
			int imgW = _pImage->GetWidth();
			int imgH = _pImage->GetHeight();
			if (imgW > 0 && imgH > 0)
			{
				std::wstring sizeStr = FormatFileSize(_currentFileSize);
				wchar_t infoBuf[128];
				swprintf_s(infoBuf, L"%d x %d (%s)", imgW, imgH, sizeStr.c_str());
				infoStr = infoBuf;
			}
		}
		else
		{
			// 图片缺失时只显示文件大小
			infoStr = FormatFileSize(_currentFileSize);
		}

		// 测量文本尺寸
		StringFormat measureFormat;
		measureFormat.SetFormatFlags(StringFormatFlagsNoWrap);

		RectF nameRect;
		graphics->MeasureString(displayPath.c_str(), -1, &font, PointF(0, 0), &measureFormat, &nameRect);

		RectF infoRect;
		float infoWidth = 0;
		if (!infoStr.empty())
		{
			graphics->MeasureString(infoStr.c_str(), -1, &font, PointF(0, 0), &measureFormat, &infoRect);
			infoWidth = infoRect.Width + 12; // 信息文本宽度 + 左边距
		}

		// 计算可用宽度
		const float availableWidth = static_cast<float>(width - PADDING * 2);

		// 文件名宽度（完整显示）
		float nameWidth = nameRect.Width;

		// 绘制文件名（左侧，完整显示）
		StringFormat nameFormat;
		nameFormat.SetFormatFlags(StringFormatFlagsNoWrap);
		nameFormat.SetAlignment(StringAlignmentNear);
		nameFormat.SetLineAlignment(StringAlignmentCenter);

		RectF nameDrawRect(
			static_cast<REAL>(PADDING),
			1.0f,
			nameWidth,
			static_cast<REAL>(PATH_HEIGHT - 2));

		graphics->DrawString(displayPath.c_str(), -1, &font, nameDrawRect, &nameFormat, &textBrush);

		// 计算剩余空间，决定是否显示信息
		float remainingWidth = availableWidth - nameWidth;

		// 信息：剩余空间足够才显示
		bool showInfo = !infoStr.empty() && (remainingWidth >= infoWidth + 8);

		// 绘制信息（右侧）
		if (showInfo)
		{
			StringFormat infoFormat;
			infoFormat.SetFormatFlags(StringFormatFlagsNoWrap);
			infoFormat.SetAlignment(StringAlignmentFar);
			infoFormat.SetLineAlignment(StringAlignmentCenter);

			RectF infoDrawRect(
				static_cast<REAL>(width - PADDING - infoWidth),
				1.0f,
				infoWidth,
				static_cast<REAL>(PATH_HEIGHT - 2));

			SolidBrush infoBrush(Color(255, 139, 148, 158)); // 淡灰色
			graphics->DrawString(infoStr.c_str(), -1, &font, infoDrawRect, &infoFormat, &infoBrush);
		}

		// 分隔线
		Pen separatorPen(Color(255, 48, 54, 61), 1.0f);
		graphics->DrawLine(&separatorPen, PADDING, PATH_HEIGHT, width - PADDING, PATH_HEIGHT);
	}

	// 图片显示区域背景
	if (_isMissingImage)
	{
		// 图片缺失状态：显示深灰色背景和提示文字
		SolidBrush missingBgBrush(Color(255, 64, 64, 64)); // 深灰色背景
		CRect imageRect = CalculateImageRect(width, height);
		graphics->FillRectangle(&missingBgBrush, 
			imageRect.left, imageRect.top, 
			imageRect.Width(), imageRect.Height());

		// 在图片区域中央显示 "image is missing"
		FontFamily fontFamily(L"Segoe UI");
		Gdiplus::Font font(&fontFamily, 12, FontStyleRegular, UnitPoint);
		SolidBrush textBrush(Color(255, 200, 200, 200)); // 浅灰色文字

		const wchar_t* missingText = L"image is missing";
		
		StringFormat stringFormat;
		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);
		
		RectF drawRect(
			static_cast<REAL>(imageRect.left),
			static_cast<REAL>(imageRect.top),
			static_cast<REAL>(imageRect.Width()),
			static_cast<REAL>(imageRect.Height()));
		graphics->DrawString(missingText, -1, &font, drawRect, &stringFormat, &textBrush);
	}
	else if (_pImage)
	{
		// 计算图片显示区域
		_imageRect = CalculateImageRect(width, height);

		// 用灰色填充整个图片可用区域（覆盖上下/左右留空部分）
		const int areaTop = PATH_HEIGHT + PADDING;
		SolidBrush letterboxBrush(Color(255, 45, 45, 45));
		graphics->FillRectangle(&letterboxBrush,
			PADDING, areaTop,
			width - PADDING * 2, height - areaTop - PADDING);

		// 在图片实际占据的矩形内绘制灰白格子背景（用于透明区域显示）
		{
			const int cellSize = 16; // 每个格子的边长（像素）
			const Color colorLight(255, 204, 204, 204);
			const Color colorDark (255, 153, 153, 153);
			SolidBrush lightBrush(colorLight);
			SolidBrush darkBrush (colorDark);

			int imgLeft   = _imageRect.left;
			int imgTop    = _imageRect.top;
			int imgRight  = _imageRect.right;
			int imgBottom = _imageRect.bottom;

			for (int row = 0; imgTop + row * cellSize < imgBottom; ++row)
			{
				for (int col = 0; imgLeft + col * cellSize < imgRight; ++col)
				{
					int cellX = imgLeft + col * cellSize;
					int cellY = imgTop  + row * cellSize;
					int cellW = min(cellSize, imgRight  - cellX);
					int cellH = min(cellSize, imgBottom - cellY);

					bool isLight = ((row + col) % 2 == 0);
					graphics->FillRectangle(
						isLight ? &lightBrush : &darkBrush,
						cellX, cellY, cellW, cellH);
				}
			}
		}

		// 使用高质量插值模式绘制图片
		graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);

		// 绘制图片（透明部分会透出下方的格子背景）
		graphics->DrawImage(_pImage,
			static_cast<REAL>(_imageRect.left),
			static_cast<REAL>(_imageRect.top),
			static_cast<REAL>(_imageRect.Width()),
			static_cast<REAL>(_imageRect.Height()));
	}
}

void CChatInputImageTip::EnsureBufferBitmap(int width, int height)
{
	// 检查是否需要重新创建缓冲位图
	if (!_bufferBitmap || _bufferWidth != width || _bufferHeight != height)
	{
		// 清理旧的缓冲位图
		if (_bufferBitmap)
		{
			delete _bufferBitmap;
			_bufferBitmap = nullptr;
		}

		// 创建新的缓冲位图
		if (width > 0 && height > 0)
		{
			_bufferBitmap = new Bitmap(width, height, PixelFormat32bppARGB);
			_bufferWidth = width;
			_bufferHeight = height;
		}
	}
}

BOOL CChatInputImageTip::OnEraseBkgnd(CDC* pDC)
{
	// 在OnPaint中绘制，避免闪烁
	return TRUE;
}

LRESULT CChatInputImageTip::OnNcHitTest(CPoint point)
{
	// 返回 HTTRANSPARENT 使窗口对鼠标事件透明，
	// 从而不遮挡下方窗口的操作
	return HTTRANSPARENT;
}

void CChatInputImageTip::OnMouseMove(UINT nFlags, CPoint point)
{
	// 检查是否需要隐藏窗口（鼠标移出窗口区域）
	CRect clientRect;
	GetClientRect(&clientRect);

	if (!clientRect.PtInRect(point))
	{
		// 可选：鼠标移出时隐藏窗口
		// HideWindow();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CChatInputImageTip::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 点击图片预览窗口时隐藏
	HideWindow();
}

void CChatInputImageTip::OnCaptureChanged(CWnd* pWnd)
{
	CWnd::OnCaptureChanged(pWnd);
}
