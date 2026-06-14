using System;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.VisualStudio.Shell;
using Task = System.Threading.Tasks.Task;
using Microsoft.VisualStudio.Text.Tagging;
using System.ComponentModel.Composition;
using System.Windows.Media;
using Microsoft.VisualStudio.Text.Classification; // For EditorFormatDefinition
using Microsoft.VisualStudio.Text.Editor;         // For MarkerFormatDefinition
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.Text;
using System.Collections.Generic;
using Microsoft.VisualStudio.Text.Formatting;
using System.Windows.Controls;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text.Differencing;

namespace LazyBugPlugInCSharp
{

    /// <summary>
    /// 装饰层定义。这个类导出一个 AdornmentLayerDefinition，
    /// 它允许我们定义一个自定义的装饰层，并控制其 Z 顺序。
    /// </summary>
    internal static class AdornmentLayorDefinations
    {
        /// <summary>
        /// 装饰层定义。这个属性导出一个 AdornmentLayerDefinition，
        /// 它允许我们定义一个自定义的装饰层，并控制其 Z 顺序。
        /// MEF 会发现这个导出并实例化 AdornmentLayerDefinition
        [Export(typeof(AdornmentLayerDefinition))]
        [Name("LazyBugDiffLineBackgroundLayer")] // 装饰层的唯一名称
        [Order(Before = PredefinedAdornmentLayers.Text,// 将我们的装饰层绘制在文本之下
                    After = PredefinedAdornmentLayers.Selection)] 
        // 你也可以选择其他层，例如：
        // [Order(After = PredefinedAdornmentLayers.Selection)] // 绘制在选区之上
        // [Order(Before = PredefinedAdornmentLayers.Caret)] // 绘制在插入符号之下
        public static AdornmentLayerDefinition LazyBugDiffAdornmentLayerDefinition { get; set; }
    }

    /// <summary>
    /// IWpfTextViewCreationListener 的实现。
    /// 当 IWpfTextView (编辑器视图) 被创建时，这个工厂会被调用。
    /// </summary>
    [Export(typeof(IWpfTextViewCreationListener))]
    [ContentType("text")] // 指定此装饰适用于所有基于 "text" 的内容类型，你可以根据需要修改为更具体的内容类型，如 "code"
    [TextViewRole(PredefinedTextViewRoles.Document)] // 指定此装饰适用于文档视图 (即主要的文本编辑窗口)
    internal class LazyBugDiffAdornmentFactory : IWpfTextViewCreationListener
    {
        public void TextViewCreated(IWpfTextView textView)
        {
            // 为每个新创建的视图实例化我们的装饰管理器。
            // LazyBugDiffAdornment 类将负责实际的绘制和管理。
            new LazyBugDiffAdornment(textView);
        }
    }

        /// <summary>
        /// 管理和绘制整行背景装饰的类。
        /// </summary>
    internal class LazyBugDiffAdornment : IDisposable
    {
        private readonly IWpfTextView _view;
        private readonly IAdornmentLayer _layer;
        private readonly ITextBuffer _buffer;
        private bool _isDisposed = false;
        private bool _showFileChangeBorder = false;
        private Dictionary<int, int> _lineTypes = null;
        private bool _isLineTypesBuilt = false;

        private Dictionary<int, int> BuildLineTypes(IVsUserData vsUserData)
        {
            // (在此处实现缓存逻辑，如果需要)
            // if (_cachedMapData != null && !IsCacheStale()) return _cachedMapData;


            Guid guid = new Guid("257A16BC-9B4E-4C67-8528-EE23A740BAD4");

            var resultMap = new Dictionary<int, int>();

            if (vsUserData != null)
            {
                object dataValue;
                Guid key = guid;
                int hr = vsUserData.GetData(ref key, out dataValue);

                if (ErrorHandler.Succeeded(hr) && dataValue != null)
                {
                    _isLineTypesBuilt = true;

                    // SAFEARRAY of VARIANTs 通常会封送为 object[] in C#
                    if (dataValue is object[] outerArray)
                    {
                        foreach (object pairItem in outerArray)
                        {
                            // 每个 pairItem 应该是代表 [lineNumber, value] 的 object[]
                            if (pairItem is object[] innerArray && innerArray.Length == 2)
                            {
                                int lineNumber = Convert.ToInt32(innerArray[0]);
                                int value = Convert.ToInt32(innerArray[1]);
                                resultMap[lineNumber] = value;
                            }
                        }
                    }
                }
            }
            _lineTypes = resultMap;

            return resultMap;
        }

        private static IVsUserData GetVsUserDataFromTextView(IWpfTextView textView)
        {
            if (textView == null)
            {
                return null;
            }

            ITextBuffer textBuffer = textView.TextBuffer;

            // ITextBuffer.Properties is a good place to find the underlying IVsTextBuffer.
            // The key is typeof(IVsTextBuffer).
            if (textBuffer.Properties.TryGetProperty(typeof(IVsTextBuffer), out IVsTextBuffer vsTextBuffer))
            {
                // IVsTextBuffer directly implements IVsUserData, so a cast is sufficient.
                return vsTextBuffer as IVsUserData;
            }

            return null;
        }

        public LazyBugDiffAdornment(IWpfTextView view)
        {
            _view = view ?? throw new ArgumentNullException(nameof(view));
            _buffer = view.TextBuffer;

            // 获取我们之前定义的装饰层
            _layer = view.GetAdornmentLayer("LazyBugDiffLineBackgroundLayer");

            // 订阅视图布局变化事件，当滚动、文本编辑等导致视图重绘时，我们需要更新装饰
            _view.LayoutChanged += OnLayoutChanged;
            // （可选）如果高亮逻辑依赖于文本内容且需要即时更新，可以订阅文本缓冲区的 Changed 事件
             _buffer.Changed += OnBufferChanged;

            // （可选）订阅视图关闭事件，以便进行清理
            _view.Closed += OnViewClosed;

            // 首次加载时可能需要绘制一次
            // DrawAdornments(); // 或者等待第一次 LayoutChanged 事件
        }

        /// <summary>
        /// 当视图布局发生变化时调用此方法。
        /// 这是重新绘制或更新我们装饰的主要时机。
        /// </summary>
        private void OnLayoutChanged(object sender, TextViewLayoutChangedEventArgs e)
        {
            if (_isDisposed) return;
            DrawAdornments();
        }

        /// <summary>
        /// （可选）当文本缓冲区内容发生变化时调用。
        /// </summary>
        private void OnBufferChanged(object sender, TextContentChangedEventArgs e)
        {
            if (_isDisposed) return;
            // 如果高亮逻辑依赖于文本内容，这里可能需要重新绘制。
            // 注意：频繁的文本更改可能导致此事件频繁触发，需要考虑性能。
            // 一种常见的策略是标记需要重绘，然后在下一次 LayoutChanged 时执行，或者使用某种形式的防抖/节流。
            _isLineTypesBuilt = false;
            _lineTypes = null;
            DrawAdornments();
        }


        /// <summary>
        /// 核心的绘制逻辑。
        /// </summary>
        private void DrawAdornments()
        {
            if (_isDisposed || _view.InLayout) // 防止在布局过程中重复进入
                return;

            if (!_isLineTypesBuilt)
                BuildLineTypes(GetVsUserDataFromTextView(_view));

            // 清除当前层上的所有旧装饰。
            // 对于更复杂的场景或为了极致性能，你可能希望只移除或更新那些实际发生变化的装饰。
            // 但对于大多数情况，全部移除并重新添加是可接受的。
            _layer.RemoveAllAdornments();

            // 检查是否显示 FileChange 边框
            _showFileChangeBorder = CheckFileChangeBorderFlag();
            DrawFileChangeBorder();

            // 获取当前视口中可见的文本行集合。
            // TextViewLines 包含了已经过布局和可能的几何变换（如行高变化）的行。
            foreach (ITextViewLine viewLine in _view.TextViewLines)
            {
                // 有时 TextViewLines 会包含部分可见的行，或者在快速滚动时状态可能不一致。
                // 确保我们处理的是有效的、完全可见的行。
                if (viewLine.VisibilityState != VisibilityState.FullyVisible || viewLine.Start == viewLine.End) // 忽略空行或不可见的行
                {
                    // 你也可以选择处理部分可见的行 (viewLine.VisibilityState == VisibilityState.PartiallyVisible)
                    // continue; // 根据你的需求决定是否跳过
                }

                // 从视图行获取其在文本缓冲区快照中的对应范围。
                // 我们需要 ITextSnapshotLine 来应用我们的高亮逻辑。
                // 注意：一个 ITextViewLine 可能对应多个 ITextSnapshotLine (例如在双向文本或特殊情况下)，
                // 但通常情况下是一对一的。GetBufferPositionFromXCoordinate 和 GetTextElementSpan 提供了更精确的映射。
                // 对于整行高亮，通常我们关心的是 ITextViewLine 覆盖的主要 ITextSnapshotLine。
                // 这里我们简单地取 ITextViewLine 起始点所在的 ITextSnapshotLine。
                SnapshotPoint startPoint = viewLine.Start;
                if (startPoint.Position >= startPoint.Snapshot.Length && startPoint.Snapshot.Length > 0)
                {
                    // 如果起始点超出了快照的末尾 (可能发生在空文件或特殊情况)
                    // 尝试获取快照的最后一行
                    startPoint = startPoint.Snapshot.GetLineFromLineNumber(startPoint.Snapshot.LineCount - 1).Start;
                }
                else if (startPoint.Snapshot.Length == 0)
                {
                    continue; // 空快照，没有行可以处理
                }


                ITextSnapshotLine snapshotLine = startPoint.GetContainingLine();

                int lineType = GetLineType(snapshotLine);

                // 调用我们的自定义逻辑来判断这一行是否应该被高亮。
                if (lineType > 0)
                {
                    // 创建一个矩形作为背景。
                    SolidColorBrush br;
                    if (lineType == 1)
                        br = new SolidColorBrush(Colors.Green) { Opacity = 0.3 };
                    else
                        br = new SolidColorBrush(Colors.Red) { Opacity = 0.3 };

                    var backgroundRectangle = new Rectangle
                    {
                        Fill = br,
                        Width = _view.ViewportWidth,
                        Height = viewLine.Height
                    };

                    // 设置矩形在 Canvas 面板上的位置。
                    // Left 设置为视口的左边缘。
                    Canvas.SetLeft(backgroundRectangle, _view.ViewportLeft);
                    // Top 设置为当前视图行的顶部位置。
                    Canvas.SetTop(backgroundRectangle, viewLine.Top);

                    // 将矩形装饰添加到层上。
                    // 第一个参数 (AdornmentPositioningBehavior.TextRelative) 指定装饰如何随文本移动。
                    //   对于整行背景，如果希望它严格跟随视口而不是文本的水平滚动，可能需要更复杂的处理或不同的行为。
                    //   但通常 TextRelative 配合手动设置宽度为 ViewportWidth 可以工作。
                    // 第二个参数 (viewLine.Extent) 是与此装饰关联的文本范围。当此范围内的文本发生变化或移动时，
                    //   装饰层可能会尝试自动管理或移除装饰。对于整行背景，使用 viewLine.Extent 是合适的。
                    // 第三个参数 (tag) 是一个可选的标记，可以用于以后查找或移除特定的装饰。
                    // 第四个参数 (adornment) 是要添加的 UIElement。
                    // 第五个参数 (removedCallback) 是当装饰被移除时调用的回调。
                    _layer.AddAdornment(AdornmentPositioningBehavior.TextRelative, viewLine.Extent, null, backgroundRectangle, null);
                }
            }
        }

        /// <summary>
        /// 自定义逻辑，判断给定的文本行是否应该被高亮。
        /// </summary>
        /// <param name="snapshotLine">要检查的 ITextSnapshotLine。</param>
        /// <returns>如果行应该高亮，则为 true；否则为 false。</returns>
        private int GetLineType(ITextSnapshotLine snapshotLine)
        {
            if (_lineTypes.TryGetValue(snapshotLine.LineNumber, out int propertyValue))
            {
                return propertyValue;
            }
            return 0; // 默认不高亮
        }

        /// <summary>
        /// 检查是否显示 FileChange 边框
        /// </summary>
        private bool CheckFileChangeBorderFlag()
        {
            var vsUserData = GetVsUserDataFromTextView(_view);
            if (vsUserData == null)
                return false;

            // {A1B2C3D4-E5F6-7890-ABCD-EF1234567890} - 与 C++ 端相同的 GUID
            Guid guidFileChangeBorder = new Guid("A1B2C3D4-E5F6-7890-ABCD-EF1234567890");
            Guid key = guidFileChangeBorder;

            int hr = vsUserData.GetData(ref key, out object dataValue);
            if (ErrorHandler.Succeeded(hr) && dataValue != null)
            {
                if (dataValue is bool boolValue)
                    return boolValue;
                // VARIANT_BOOL (short) 的情况
                if (dataValue is short shortValue)
                    return shortValue != 0;
                if (dataValue is int intValue)
                    return intValue != 0;
            }
            return false;
        }

        /// <summary>
        /// 绘制 FileChange 边框（橘黄色）：通知左侧 margin 更新可见性
        /// </summary>
        private void DrawFileChangeBorder()
        {
            FileChangeBorderMargin.SetVisible(_view, _showFileChangeBorder);
        }

        /// <summary>
        /// 当视图关闭时调用，用于清理资源。
        /// </summary>
        private void OnViewClosed(object sender, EventArgs e)
        {
            Dispose();
        }

        /// <summary>
        /// 实现 IDisposable 接口，用于清理资源。
        /// </summary>
        public void Dispose()
        {
            if (!_isDisposed)
            {
                // 取消事件订阅，防止内存泄漏
                _view.LayoutChanged -= OnLayoutChanged;
                // if (_buffer != null) _buffer.Changed -= OnBufferChanged; // 如果订阅了
                _view.Closed -= OnViewClosed;

                // 清理 margin 注册
                FileChangeBorderMargin.Unregister(_view);

                // （可选）移除所有添加的装饰，尽管当层或视图销毁时它们通常也会被清理
                if (_layer != null)
                {
                    _layer.RemoveAllAdornments();
                }

                _isDisposed = true;
                // GC.SuppressFinalize(this); // 如果有终结器 (finalizer)，取消其执行
            }
        }

        // （可选）如果你需要一个终结器 (finalizer) 以确保在忘记调用 Dispose 时也能清理非托管资源，
        // 但对于这个纯托管代码的场景，正确实现 IDisposable 通常就足够了。
        // ~LazyBugDiffAdornment()
        // {
        //     Dispose();
        // }
    }

    // -------------------------------------------------------------------------
    // FileChange 左侧 Margin 实现
    // -------------------------------------------------------------------------

    /// <summary>
    /// Margin 工厂：MEF 导出，负责为每个文本视图创建 FileChangeBorderMargin 实例。
    /// MarginContainer 指定为 LeftSelection，紧贴 viewport 左边缘（所有 margin 最右侧）。
    /// </summary>
    [Export(typeof(IWpfTextViewMarginProvider))]
    [Name("FileChangeBorderMargin")]
    [MarginContainer(PredefinedMarginNames.LeftSelection)]
    [Order(Before = PredefinedMarginNames.Spacer)]
    [ContentType("text")]
    [TextViewRole(PredefinedTextViewRoles.Document)]
    internal class FileChangeBorderMarginProvider : IWpfTextViewMarginProvider
    {
        public IWpfTextViewMargin CreateMargin(IWpfTextViewHost wpfTextViewHost, IWpfTextViewMargin marginContainer)
        {
            return new FileChangeBorderMargin(wpfTextViewHost.TextView);
        }
    }

    /// <summary>
    /// 左侧 Margin 实体：宽度固定 10px，显示橘黄色条带。
    /// 自身不继承任何 WPF 类型，通过 VisualElement 属性返回一个 Border 作为可视元素。
    /// 通过静态字典与 LazyBugDiffAdornment 通信，由后者调用 SetVisible() 驱动显示/隐藏。
    /// </summary>
    internal class FileChangeBorderMargin : IWpfTextViewMargin
    {
        public const double MarginWidth = 5.0;
        private static readonly Color OrangeColor = Color.FromArgb(64, 255, 165, 0);

        // 静态字典：view -> margin，供 LazyBugDiffAdornment 查找并通知
        private static readonly Dictionary<IWpfTextView, FileChangeBorderMargin> s_instances
            = new Dictionary<IWpfTextView, FileChangeBorderMargin>();

        private readonly IWpfTextView _view;
        private bool _isDisposed = false;

        // 作为 VisualElement 的 Border，背景色即橘黄色条带
        private readonly System.Windows.Controls.Border _border;

        public FileChangeBorderMargin(IWpfTextView view)
        {
            _view = view;

            _border = new System.Windows.Controls.Border
            {
                Background = new SolidColorBrush(OrangeColor),
                Width = 0,          // 默认隐藏
                ClipToBounds = true,
            };

            // 注册到静态字典
            lock (s_instances)
                s_instances[view] = this;

            view.Closed += OnViewClosed;
        }

        // ---- 显示/隐藏 ----
        private void SetVisibleInternal(bool visible)
        {
            _border.Width = visible ? MarginWidth : 0;
        }

        /// <summary>
        /// 由 LazyBugDiffAdornment.DrawFileChangeBorder() 调用，驱动 margin 显示/隐藏。
        /// </summary>
        public static void SetVisible(IWpfTextView view, bool visible)
        {
            FileChangeBorderMargin margin;
            lock (s_instances)
                s_instances.TryGetValue(view, out margin);
            margin?.SetVisibleInternal(visible);
        }

        /// <summary>
        /// 由 LazyBugDiffAdornment.Dispose() 调用，清理静态字典中的注册。
        /// </summary>
        public static void Unregister(IWpfTextView view)
        {
            lock (s_instances)
                s_instances.Remove(view);
        }

        private void OnViewClosed(object sender, EventArgs e)
        {
            Dispose();
        }

        // ---- IWpfTextViewMargin ----
        public System.Windows.FrameworkElement VisualElement => _border;
        public double MarginSize => _border.Width;
        public bool Enabled => true;

        public ITextViewMargin GetTextViewMargin(string marginName)
            => marginName == "FileChangeBorderMargin" ? this : null;

        public void Dispose()
        {
            if (!_isDisposed)
            {
                _view.Closed -= OnViewClosed;
                lock (s_instances)
                    s_instances.Remove(_view);
                _isDisposed = true;
            }
        }
    }

    /// <summary>
    /// ISymbolQueryService: C++ 插件查询符号定义的 COM 接口
    /// </summary>
    [Guid("1D68CCF8-1F74-480A-B3B4-A5C6FD4DE5D6")]
    [ComVisible(true)]
    public interface ISymbolQueryService
    {
        bool FindFunctionDefinition(string fullyQualifiedName, out string filePath, out int line);
    }

    /// <summary>
    /// 服务的接口标识 (SID)，VS 内部用于 QueryService
    /// </summary>
    [Guid("6F241565-9F09-4DC2-83F0-90A13C61EA47")]
    public interface SSymbolQueryService { }

    /// <summary>
    /// SymbolQueryService: 查询符号定义的实际逻辑实现
    /// </summary>
    [Guid("F41D00B4-91BE-4523-83D5-2A838725C390")]
    [ComVisible(true)]
    public class SymbolQueryService : ISymbolQueryService, SSymbolQueryService
    {
        private readonly EnvDTE80.DTE2 _dte;

        public SymbolQueryService(EnvDTE80.DTE2 dte)
        {
            _dte = dte;
        }

        public bool FindFunctionDefinition(string fullyQualifiedName, out string filePath, out int line)
        {
            filePath = string.Empty;
            line = -1;

            ThreadHelper.ThrowIfNotOnUIThread();

            try
            {
                if (_dte == null || _dte.Solution == null)
                    return false;

                foreach (EnvDTE.Project project in _dte.Solution.Projects)
                {
                    try
                    {
                        // 使用 dynamic 调用 VCCodeModel 特有方法，避免引入特定的 C++ 互操作库依赖
                        dynamic vcCodeModel = project.CodeModel;
                        if (vcCodeModel != null)
                        {
                            EnvDTE.CodeElement element = vcCodeModel.CodeElementFromFullName(fullyQualifiedName);

                            // EnvDTE.vsCMElement.vsCMElementFunction 为 2
                            if (element != null && element.Kind == (EnvDTE.vsCMElement)2)
                            {
                                EnvDTE.TextPoint startPoint = element.StartPoint;
                                filePath = element.ProjectItem.FileNames[1]; // 索引从1开始
                                line = startPoint.Line;
                                return true;
                            }
                        }
                    }
                    catch
                    {
                        // 忽略不支持或者找不到的项目，继续查找下一个
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error finding function definition: {ex.Message}");
            }

            return false;
        }
    }

    /// <summary>
    /// IBreakpointBookmarkService: C++ 插件保存/恢复断点和书签的 COM 接口
    /// </summary>
    [Guid("A7B2C3D4-E5F6-4A7B-8C9D-0E1F2A3B4C5D")]
    [ComVisible(true)]
    [InterfaceType(ComInterfaceType.InterfaceIsDual)]
    public interface IBreakpointBookmarkService
    {
        /// <summary>
        /// 保存指定文件的断点信息，返回 "line|condition|enabled\tline|condition|enabled\t..."
        /// line 是 1-based，不包含文件路径
        /// </summary>
        string SaveBreakpoints(string filePath);

        /// <summary>
        /// 恢复断点，bpData 格式同 SaveBreakpoints 返回值，lineMap 格式: "oldLine:newLine;oldLine:newLine;..."
        /// </summary>
        void RestoreBreakpoints(string filePath, string bpData, string lineMapData);

        /// <summary>
        /// 保存指定文件的书签信息，返回格式: "line;type;line;type;..."
        /// line 是 1-based, type: 3=Bookmark, 4=Shortcut
        /// </summary>
        string SaveBookmarks(string filePath);

        /// <summary>
        /// 恢复书签，bmData 格式同 SaveBookmarks 返回值，lineMapData 同 RestoreBreakpoints
        /// </summary>
        void RestoreBookmarks(string filePath, string bmData, string lineMapData);
    }

    /// <summary>
    /// 服务的接口标识 (SID)
    /// </summary>
    [Guid("B8C9D0E1-F2A3-4B4C-5D6E-7F8A9B0C1D2E")]
    public interface SBreakpointBookmarkService { }

    /// <summary>
    /// BreakpointBookmarkService: 保存/恢复断点和书签的实际逻辑实现
    /// </summary>
    [Guid("C9D0E1F2-A3B4-4C5D-6E7F-8A9B0C1D2E3F")]
    [ComVisible(true)]
    public class BreakpointBookmarkService : IBreakpointBookmarkService, SBreakpointBookmarkService
    {
        private readonly EnvDTE80.DTE2 _dte;

        public BreakpointBookmarkService(EnvDTE80.DTE2 dte)
        {
            _dte = dte;
        }

        /// <summary>
        /// 保存指定文件的断点，返回 "line|condition|enabled\tline|condition|enabled\t..."
        /// line 是 1-based，不包含文件路径
        /// </summary>
        public string SaveBreakpoints(string filePath)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_dte == null || string.IsNullOrEmpty(filePath))
                return string.Empty;

            try
            {
                var result = new System.Text.StringBuilder();
                string normalizedPath = System.IO.Path.GetFullPath(filePath);

                foreach (EnvDTE.Breakpoint bp in _dte.Debugger.Breakpoints)
                {
                    try
                    {
                        if (string.IsNullOrEmpty(bp.File))
                            continue;

                        string bpPath = System.IO.Path.GetFullPath(bp.File);
                        if (!string.Equals(bpPath, normalizedPath, StringComparison.OrdinalIgnoreCase))
                            continue;

                        if (result.Length > 0)
                            result.Append('\t');

                        // line|condition|enabled
                        result.Append(bp.FileLine); // 1-based
                        result.Append('|');
                        result.Append(bp.Condition ?? string.Empty);
                        result.Append('|');
                        result.Append(bp.Enabled ? "1" : "0");
                    }
                    catch { }
                }

                return result.ToString();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"SaveBreakpoints error: {ex.Message}");
                return string.Empty;
            }
        }

        /// <summary>
        /// 恢复断点。bpData 格式同 SaveBreakpoints 返回值。
        /// lineMapData 格式: "oldLine:newLine;oldLine:newLine;..." (均为 0-based)
        /// </summary>
        public void RestoreBreakpoints(string filePath, string bpData, string lineMapData)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_dte == null || string.IsNullOrEmpty(bpData))
                return;

            try
            {
                // 解析行号映射
                var lineMap = new Dictionary<int, int>();
                if (!string.IsNullOrEmpty(lineMapData))
                {
                    foreach (string pair in lineMapData.Split(';'))
                    {
                        string[] parts = pair.Split(':');
                        if (parts.Length == 2 && int.TryParse(parts[0], out int oldLine) && int.TryParse(parts[1], out int newLine))
                        {
                            lineMap[oldLine] = newLine;
                        }
                    }
                }

                // 解析并恢复断点
                string[] entries = bpData.Split('\t');
                foreach (string entry in entries)
                {
                    if (string.IsNullOrEmpty(entry))
                        continue;

                    string[] fields = entry.Split('|');
                    if (fields.Length < 3)
                        continue;

                    if (!int.TryParse(fields[0], out int oldLine1)) // 1-based
                        continue;
                    string condition = fields[1];
                    bool enabled = fields[2] == "1";

                    // 映射行号: oldLine1(1-based) -> oldLine0(0-based) -> newLine0(0-based) -> newLine1(1-based)
                    int oldLine0 = oldLine1 - 1;
                    int newLine0 = oldLine0;
                    if (lineMap.TryGetValue(oldLine0, out int mappedLine0))
                        newLine0 = mappedLine0;

                    int newLine1 = newLine0 + 1;

                    try
                    {
                        // Breakpoints.Add(Function, File, Line, Column, Condition, ConditionType,
                        //                  Language, Data, DataCount, Address, HitCount, HitCountType)
                        EnvDTE.Breakpoints bps = _dte.Debugger.Breakpoints.Add(
                            string.Empty,                                   // Function
                            filePath,                                       // File
                            newLine1,                                       // Line (1-based)
                            1,                                              // Column
                            condition ?? string.Empty,                      // Condition
                            EnvDTE.dbgBreakpointConditionType.dbgBreakpointConditionTypeWhenTrue, // ConditionType
                            string.Empty,                                   // Language
                            string.Empty,                                   // Data
                            0,                                              // DataCount
                            string.Empty,                                   // Address
                            0,                                              // HitCount
                            EnvDTE.dbgHitCountType.dbgHitCountTypeNone       // HitCountType
                        );

                        // 如果原断点是禁用状态，设置新断点为禁用
                        if (!enabled && bps != null && bps.Count > 0)
                        {
                            foreach (EnvDTE.Breakpoint newBp in bps)
                            {
                                newBp.Enabled = false;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        System.Diagnostics.Debug.WriteLine($"RestoreBreakpoint failed for {filePath}:{newLine1}: {ex.Message}");
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"RestoreBreakpoints error: {ex.Message}");
            }
        }

        /// <summary>
        /// 保存指定文件的书签，返回 "line;line;..." (1-based)
        /// 使用 EnvDTE.EditPoint.NextBookmark() 枚举现代 VS 书签
        /// </summary>
        public string SaveBookmarks(string filePath)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_dte == null || string.IsNullOrEmpty(filePath))
                return string.Empty;

            try
            {
                // 找到对应的 Document
                EnvDTE.Document doc = null;
                string normalizedPath = System.IO.Path.GetFullPath(filePath);
                foreach (EnvDTE.Document d in _dte.Documents)
                {
                    try
                    {
                        if (string.Equals(System.IO.Path.GetFullPath(d.FullName), normalizedPath, StringComparison.OrdinalIgnoreCase))
                        {
                            doc = d;
                            break;
                        }
                    }
                    catch { }
                }

                if (doc == null)
                    return string.Empty;

                var textDoc = doc.Object("TextDocument") as EnvDTE.TextDocument;
                if (textDoc == null)
                    return string.Empty;

                var result = new System.Text.StringBuilder();

                // 从文档开头创建 EditPoint，用 NextBookmark() 遍历所有书签
                // NextBookmark() 到达末尾后会回绕到开头，需用已访问行号集合防止死循环
                EnvDTE.EditPoint ep = textDoc.StartPoint.CreateEditPoint();
                var visitedLines = new HashSet<int>();
                while (ep.NextBookmark())
                {
                    int line = ep.Line;
                    if (visitedLines.Contains(line))
                        break; // 已回绕，退出
                    visitedLines.Add(line);

                    if (result.Length > 0)
                        result.Append(';');
                    result.Append(line); // 1-based
                }

                System.Diagnostics.Debug.WriteLine($"[SaveBookmarks] result={result}");
                return result.ToString();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"SaveBookmarks error: {ex.Message}");
                return string.Empty;
            }
        }

        /// <summary>
        /// 恢复书签。bmData 格式: "line;type;line;type;..." (1-based line)
        /// lineMapData 格式同 RestoreBreakpoints
        /// </summary>
        public void RestoreBookmarks(string filePath, string bmData, string lineMapData)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (string.IsNullOrEmpty(bmData))
                return;

            try
            {
                // 解析行号映射 (0-based old -> 0-based new)
                var lineMap = new Dictionary<int, int>();
                if (!string.IsNullOrEmpty(lineMapData))
                {
                    foreach (string pair in lineMapData.Split(';'))
                    {
                        string[] parts = pair.Split(':');
                        if (parts.Length == 2 && int.TryParse(parts[0], out int oldLine) && int.TryParse(parts[1], out int newLine))
                            lineMap[oldLine] = newLine;
                    }
                }

                // 找到对应的 Document，若未打开则打开它
                EnvDTE.Document doc = null;
                string normalizedPath = System.IO.Path.GetFullPath(filePath);
                foreach (EnvDTE.Document d in _dte.Documents)
                {
                    try
                    {
                        if (string.Equals(System.IO.Path.GetFullPath(d.FullName), normalizedPath, StringComparison.OrdinalIgnoreCase))
                        {
                            doc = d;
                            break;
                        }
                    }
                    catch { }
                }

                if (doc == null)
                {
                    // 文档未打开，用 OpenFile 打开（使用 ViewKind:=EnvDTE.Constants.vsViewKindTextView 避免打开设计器）
                    try
                    {
                        EnvDTE.Window win = _dte.ItemOperations.OpenFile(filePath, EnvDTE.Constants.vsViewKindTextView);
                        if (win != null)
                            doc = win.Document;
                    }
                    catch (Exception ex)
                    {
                        System.Diagnostics.Debug.WriteLine($"[RestoreBookmarks] OpenFile failed: {ex.Message}");
                        return;
                    }
                }

                if (doc == null)
                    return;

                var textDoc = doc.Object("TextDocument") as EnvDTE.TextDocument;
                if (textDoc == null)
                    return;

                // 解析书签行号并用 EditPoint.SetBookmark() 恢复
                // bmData 格式: "line;line;..." (1-based)
                foreach (string token in bmData.Split(';'))
                {
                    if (!int.TryParse(token, out int oldLine1)) // 1-based
                        continue;

                    // 映射行号: 1-based -> 0-based -> map -> 1-based
                    int oldLine0 = oldLine1 - 1;
                    int newLine0 = lineMap.TryGetValue(oldLine0, out int mapped) ? mapped : oldLine0;
                    int newLine1 = newLine0 + 1;

                    try
                    {
                        int totalLines = textDoc.EndPoint.Line;
                        if (newLine1 < 1 || newLine1 > totalLines)
                            continue;

                        EnvDTE.EditPoint ep = textDoc.StartPoint.CreateEditPoint();
                        ep.MoveToLineAndOffset(newLine1, 1);
                        ep.SetBookmark();
                        System.Diagnostics.Debug.WriteLine($"[RestoreBookmarks] set bookmark at line={newLine1}");
                    }
                    catch (Exception ex)
                    {
                        System.Diagnostics.Debug.WriteLine($"[RestoreBookmarks] failed at line={newLine1}: {ex.Message}");
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"RestoreBookmarks error: {ex.Message}");
            }
        }
    }

    /// <summary>
    /// LazyBugCSharpBridgePackage: 用于将我们的服务注册到 Visual Studio 全局环境中
    /// </summary>
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [Guid(PackageGuidString)]
    [ProvideService(typeof(SSymbolQueryService), IsAsyncQueryable = true)]
    [ProvideService(typeof(SBreakpointBookmarkService), IsAsyncQueryable = true)]
    [ProvideAutoLoad(VSConstants.UICONTEXT.SolutionExistsAndFullyLoaded_string, PackageAutoLoadFlags.BackgroundLoad)]
    public sealed class LazyBugCSharpBridgePackage : AsyncPackage
    {
        public const string PackageGuidString = "B66902EB-87C8-4F8C-98AE-FA3787D4F4EE";

        public static LazyBugCSharpBridgePackage Instance { get; private set; }

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
        {
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);

            Instance = this;

            // 获取 DTE，以便传给 Service
            EnvDTE80.DTE2 dte = await GetServiceAsync(typeof(EnvDTE.DTE)) as EnvDTE80.DTE2;

            // 注册我们的自定义服务
            this.AddService(typeof(SSymbolQueryService), (container, cancellation, type) =>
            {
                return Task.FromResult<object>(new SymbolQueryService(dte));
            }, true);

            // 注册断点/书签服务
            this.AddService(typeof(SBreakpointBookmarkService), (container, cancellation, type) =>
            {
                return Task.FromResult<object>(new BreakpointBookmarkService(dte));
            }, true);
        }
    }
}
