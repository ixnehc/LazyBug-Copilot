using System;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;

namespace LazyBugPlugInLoader
{
    /// <summary>
    /// This is the class that implements the package exposed by this assembly.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The minimum requirement for a class to be considered a valid package for Visual Studio
    /// is to implement the IVsPackage interface and register itself with the shell.
    /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
    /// to do it: it derives from the Package class that provides the implementation of the
    /// IVsPackage interface and uses the registration attributes defined in the framework to
    /// register itself and its components with the shell. These attributes tell the pkgdef creation
    /// utility what data to put into .pkgdef file.
    /// </para>
    /// <para>
    /// To get loaded into VS, the package must be referred by &lt;Asset Type="Microsoft.VisualStudio.VsPackage" ...&gt; in .vsixmanifest file.
    /// </para>
    /// </remarks>
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [Guid("a194f8e7-6b7e-491c-8699-8a6a05a862e5")]
    [ProvideAutoLoad(UIContextGuids80.NoSolution, PackageAutoLoadFlags.BackgroundLoad)]
    public sealed class LazyBugPlugInLoaderPackage : AsyncPackage
    {
        /// <summary>
        /// LazyBugPlugInLoaderPackage GUID string.
        /// </summary>
        public const string PackageGuidString = "e1d03d51-8ea5-43e2-b2fd-0972466db18d";
        private const string CppPackageGuidString = "6d2723b6-3b30-480e-8537-65b39d8b99c5";

        #region Package Members

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initialization code that rely on services provided by VisualStudio.
        /// </summary>
        /// <param name="cancellationToken">A cancellation token to monitor for initialization cancellation, which can occur when VS is shutting down.</param>
        /// <param name="progress">A provider for progress updates.</param>
        /// <returns>A task representing the async work of package initialization, or an already completed task if there is none. Do not return null from this method.</returns>
        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
        {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);

            // 获取IVsShell服务
            IVsShell vsShell = await this.GetServiceAsync(typeof(SVsShell)) as IVsShell;
            if (vsShell != null)
            {
                // 加载你的C++包
                Guid cppPackageGuid = new Guid(CppPackageGuidString);
                IVsPackage package = null;
                int hr = vsShell.LoadPackage(ref cppPackageGuid, out package);
                if (hr == VSConstants.S_OK && package != null)
                {
                    // 包加载成功
                    // 你可以在这里添加日志或进行其他操作
                }
                else
                {
                    // 包加载失败
                }
            }
        }

        #endregion
    }
}
