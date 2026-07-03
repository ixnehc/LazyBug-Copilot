#pragma once
#include <string>
#include <memory>

// GDI+ 前向声明
namespace Gdiplus {
	class Image;
	class Bitmap;
}
using namespace Gdiplus;

// 图片提示窗口类
// 用于当用户鼠标移到 CChatInput 的 image tag 上时显示图片缩略图
class CChatInputImageTip : public CWnd
{
public:
	// 窗口尺寸限制（单位：像素）
	static constexpr int MIN_WINDOW_WIDTH = 200;   // 最小窗口宽度
	static constexpr int MAX_WINDOW_WIDTH = 600;   // 最大窗口宽度
	static constexpr int MAX_IMAGE_HEIGHT = 400;   // 最大图片高度
	static constexpr int PATH_HEIGHT = 20;         // 文件路径显示区域高度
	static constexpr int PADDING = 8;              // 内边距

public:
	CChatInputImageTip();
	virtual ~CChatInputImageTip();

	// 创建窗口
	BOOL CreateTipWindow(CWnd* pParent);

	// 显示图片提示
	// @param imagePath: 图片完整路径
	// @param x, y: 显示位置（屏幕坐标）
	void ShowImage(const std::wstring& imagePath, int x, int y);

	// 显示图片提示，自动调整位置使其显示在 avoidRect 之外
	// @param imagePath: 图片完整路径
	// @param x, y: 优先显示位置（屏幕坐标）
	// @param avoidRect: 需要避开的区域（屏幕坐标）
	// @return: 是否成功显示窗口
	BOOL ShowImage(const std::wstring& imagePath, int x, int y, const CRect& avoidRect);

	// 隐藏窗口
	void HideWindow();

	// 检查窗口是否正在显示指定路径的图片
	bool IsShowingImage(const std::wstring& imagePath) const;

	void Update();

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	// 当前显示的图片路径
	std::wstring _currentImagePath;

	// 当前图片文件大小（字节）
	LONGLONG _currentFileSize;

	// GDI+ Image 对象
	Gdiplus::Image* _pImage;

	// 图片缺失标志（图片路径不存在时显示提示）
	bool _isMissingImage;

	// 缓冲位图（用于双缓冲绘制）
	Gdiplus::Bitmap* _bufferBitmap;
	int _bufferWidth;
	int _bufferHeight;

	// 实际显示的图片区域（相对于窗口客户区）
	CRect _imageRect;

	// 前台窗口监控
	DWORD _currentProcessId;

	// 加载图片
	bool LoadImage(const std::wstring& imagePath);

	// 释放当前图片资源
	void ReleaseImage();

	// 计算窗口大小（根据图片尺寸和限制）
	CSize CalculateWindowSize(int imageWidth, int imageHeight);

	// 计算图片显示区域（保持宽高比）
	CRect CalculateImageRect(int clientWidth, int clientHeight);

	// 确保缓冲位图有效
	void EnsureBufferBitmap(int width, int height);

	// 双缓冲绘制
	void DrawDoubleBuffered(CDC* pDC);

	// 执行实际绘制
	void DoPaint(Graphics* graphics, int width, int height);

	// 检查前台窗口，如果不属于当前进程则隐藏
	void CheckForegroundWindow();

	// 检查文件扩展名是否为支持的图片格式
	static bool IsSupportedImageFormat(const std::wstring& filePath);
};
