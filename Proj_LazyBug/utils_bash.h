#pragma once

#include <string>

namespace Utils
{

// 当前 bash.exe 的"风格"(决定 Windows 盘符应转换成哪种 POSIX 路径前缀)
enum class BashFlavor
{
	Unknown,   // 探测失败/未知, 兜底按 WSL 处理
	WSL,       // Windows Subsystem for Linux:  C:\ -> /mnt/c/
	GitBash,   // Git Bash / MSYS2 / MinGW:     C:\ -> /c/
	Cygwin,    // Cygwin:                       C:\ -> /cygdrive/c/
};

// 获取当前 bash.exe 的类型(带缓存, 整个进程生命周期内只真正探测一次)
BashFlavor GetBashFlavor();

// 检测当前系统是否支持 bash（WSL / Git Bash / Cygwin 均可）
bool IsBashAvailable();

// 将 Windows 路径转换为当前 bash 可识别的 POSIX 路径
// 例如(WSL):     C:\Users\foo\bar.sh -> /mnt/c/Users/foo/bar.sh
//     (GitBash): C:\Users\foo\bar.sh -> /c/Users/foo/bar.sh
//     (Cygwin):  C:\Users\foo\bar.sh -> /cygdrive/c/Users/foo/bar.sh
std::string WindowsPathToPosix(const std::string& winPath);

// 将一条 bash 命令行中出现的所有 Windows 绝对路径转换为当前 bash 可识别的 POSIX 路径
// 例如:  grep -rn "x" "D:\LazyBug\a.h" C:/Temp/b.txt /mnt/c/foo /c/bar /cygdrive/c/baz
//   ->   grep -rn "x" "/mnt/d/LazyBug/a.h" /mnt/c/Temp/b.txt /mnt/c/foo /mnt/c/bar /mnt/c/baz
// 支持识别并转换以下四种路径格式，其余内容(命令、参数)保持不变:
//   1) Windows 盘符路径:  C:\foo\bar  或  C:/foo/bar
//   2) WSL 路径:          /mnt/c/foo/bar
//   3) Git Bash 路径:     /c/foo/bar
//   4) Cygwin 路径:       /cygdrive/c/foo/bar
// 先将所有格式规范化为 "C:/xxx" 形式，再统一交给 WindowsPathToPosix 转换为目标格式。
std::string ConvertBashCommandPaths(const std::string& command);

} // namespace Utils
