#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include "Utils_Bash.h"

#include <windows.h>

namespace Utils
{

// 执行 `bash.exe -c "uname -s"`, 根据输出判定当前默认 bash.exe 的类型。
// 仅在 GetBashFlavor() 中被调用一次, 结果会被缓存。
static BashFlavor DetectBashFlavor()
{
	// uname -s 各环境的典型输出:
	//   WSL:      "Linux"
	//   Git Bash: "MINGW64_NT-10.0-..." / "MSYS_NT-10.0-..."
	//   Cygwin:   "CYGWIN_NT-10.0-..."

	std::string out;

	// 使用 CreateProcess 避免弹出命令窗口
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	HANDLE hReadPipe = NULL, hWritePipe = NULL;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
		return BashFlavor::Unknown;

	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA si = { sizeof(si) };
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = {};
	char cmdLine[] = "bash.exe -c \"uname -s\"";

	BOOL success = CreateProcessA(
		NULL, cmdLine, NULL, NULL, TRUE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(hWritePipe);
	hWritePipe = NULL;

	if (success)
	{
		char buf[256];
		DWORD bytesRead;
		while (ReadFile(hReadPipe, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0)
		{
			buf[bytesRead] = '\0';
			out += buf;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	CloseHandle(hReadPipe);

	// 转为大写便于不区分大小写比较
	std::string up = out;
	for (char& c : up)
		c = (char)std::toupper((unsigned char)c);

	if (up.find("CYGWIN") != std::string::npos)
		return BashFlavor::Cygwin;
	if (up.find("MINGW") != std::string::npos || up.find("MSYS") != std::string::npos)
		return BashFlavor::GitBash;
	if (up.find("LINUX") != std::string::npos || up.find("MICROSOFT") != std::string::npos)
		return BashFlavor::WSL;

	return BashFlavor::Unknown;
}

// 获取当前 bash.exe 的类型(带缓存, 整个进程生命周期内只真正探测一次)
BashFlavor GetBashFlavor()
{
	// C++11 起, 局部 static 的初始化是线程安全的(magic statics)
	static BashFlavor s_flavor = DetectBashFlavor();
	return s_flavor;
}

// 检测当前系统是否支持 bash（WSL / Git Bash / Cygwin 均可）
bool IsBashAvailable()
{
	return GetBashFlavor() != BashFlavor::Unknown;
}

// 将 Windows 路径转换为当前 bash 可识别的 POSIX 路径
// 例如(WSL):     C:\Users\foo\bar.sh -> /mnt/c/Users/foo/bar.sh
//     (GitBash): C:\Users\foo\bar.sh -> /c/Users/foo/bar.sh
//     (Cygwin):  C:\Users\foo\bar.sh -> /cygdrive/c/Users/foo/bar.sh
std::string WindowsPathToPosix(const std::string& winPath)
{
	if (winPath.size() < 2)
		return winPath;

	std::string result = winPath;

	// 将所有反斜杠替换为正斜杠
	// 注意: bash 中的 "\ "(反斜杠+空格)是转义空格, 属于路径的一部分,
	//       不能替换为 "/ ", 否则会破坏路径语义(如 /c/Program\ Files)。
	for (size_t i = 0; i < result.size(); ++i)
	{
		if (result[i] == '\\')
		{
			// 保留转义空格 "\ "
			if (i + 1 < result.size() && result[i + 1] == ' ')
				continue;
			result[i] = '/';
		}
	}

	// 将盘符 "C:/" 转换为对应 bash 风格的前缀
	if (result.size() >= 3 && std::isalpha((unsigned char)result[0]) && result[1] == ':' && result[2] == '/')
	{
		char driveLetter = (char)std::tolower((unsigned char)result[0]);

		std::string prefix;
		switch (GetBashFlavor())
		{
		case BashFlavor::GitBash:
			prefix = std::string("/") + driveLetter;              // -> /c
			break;
		case BashFlavor::Cygwin:
			prefix = std::string("/cygdrive/") + driveLetter;     // -> /cygdrive/c
			break;
		case BashFlavor::WSL:
		case BashFlavor::Unknown:
		default:
			prefix = std::string("/mnt/") + driveLetter;          // -> /mnt/c (默认/兜底)
			break;
		}

		result = prefix + result.substr(2); // 去掉盘符 "C:"，拼接对应前缀
	}

	return result;
}

// 将一条 bash 命令行中出现的所有 Windows 绝对路径转换为当前 bash 可识别的 POSIX 路径
std::string ConvertBashCommandPaths(const std::string& command)
{
	// 将任意格式的绝对路径规范化为 "C:/xxx" 形式，供 WindowsPathToPosix 统一处理
	// 若不是可识别的路径格式则原样返回
	auto normalizeToDrivePath = [](const std::string& path) -> std::string
	{
		// 1) Windows 盘符: C:\... 或 C:/...  -> 直接替换反斜杠即可，已是目标格式
		if (path.size() >= 3
			&& std::isalpha((unsigned char)path[0])
			&& path[1] == ':'
			&& (path[2] == '/' || path[2] == '\\'))
		{
			std::string r = path;
			// 替换反斜杠为正斜杠, 但保留转义空格 "\ "
			for (size_t i = 0; i < r.size(); ++i)
			{
				if (r[i] == '\\')
				{
					if (i + 1 < r.size() && r[i + 1] == ' ')
						continue;
					r[i] = '/';
				}
			}
			return r;
		}

		// 2) WSL: /mnt/<drive>/...  (drive 为单个字母)
		if (path.size() >= 7
			&& path[0] == '/' && path[1] == 'm' && path[2] == 'n' && path[3] == 't' && path[4] == '/'
			&& std::isalpha((unsigned char)path[5])
			&& (path.size() == 6 || path[6] == '/'))
		{
			char drive = (char)std::toupper((unsigned char)path[5]);
			std::string rest = path.size() > 6 ? path.substr(6) : "/";
			return std::string(1, drive) + ":" + rest;
		}

		// 3) Git Bash: /<drive>/...  (drive 为单个字母, 后跟 '/' 或字符串结束)
		if (path.size() >= 2
			&& path[0] == '/'
			&& std::isalpha((unsigned char)path[1])
			&& (path.size() == 2 || path[2] == '/'))
		{
			char drive = (char)std::toupper((unsigned char)path[1]);
			std::string rest = path.size() > 2 ? path.substr(2) : "/";
			return std::string(1, drive) + ":" + rest;
		}

		// 4) Cygwin: /cygdrive/<drive>/...
		if (path.size() >= 10
			&& path.substr(0, 10) == "/cygdrive/"
			&& std::isalpha((unsigned char)path[10])
			&& (path.size() == 11 || path[11] == '/'))
		{
			char drive = (char)std::toupper((unsigned char)path[10]);
			std::string rest = path.size() > 11 ? path.substr(11) : "/";
			return std::string(1, drive) + ":" + rest;
		}

		return path; // 不是可识别的路径格式，原样返回
	};

	// 匹配命令行中所有可能的路径格式(含引号包裹版本)，按优先级从左到右:
	//   引号分支优先，确保含空格的路径(如 "C:\Program Files\x")能整体匹配，
	//   否则裸路径分支会在空格处截断，导致后半段反斜杠未被转换。
	//
	//   双引号包裹: "[路径内容]"
	//   单引号包裹: '[路径内容]'
	//   裸路径:     到空白/引号为止
	//
	//   路径内容的起始特征(四种格式):
	//     [A-Za-z]:[\\/]          Windows 盘符
	//     /mnt/[A-Za-z](/|$)      WSL
	//     /[A-Za-z](/|$)          Git Bash  (单字母根目录)
	//     /cygdrive/[A-Za-z](/|$) Cygwin
	static const std::regex pathRegex(
		// 双引号包裹
		R"("(?:[A-Za-z]:[\\/]|/mnt/[A-Za-z][/]|/[A-Za-z][/]|/cygdrive/[A-Za-z][/])[^"]*")"
		R"(|)"
		// 单引号包裹
		R"('(?:[A-Za-z]:[\\/]|/mnt/[A-Za-z][/]|/[A-Za-z][/]|/cygdrive/[A-Za-z][/])[^']*')"
		R"(|)"
		// 裸路径
		//   路径主体允许 "\ "(转义空格)作为路径字符的一部分, 其余遇空白/引号即止。
		R"((?:[A-Za-z]:[\\/]|/mnt/[A-Za-z][/]|/[A-Za-z][/]|/cygdrive/[A-Za-z][/])(?:\\ |[^\s"'])*)");

	std::string result;
	result.reserve(command.size() + 16);

	auto begin = std::sregex_iterator(command.begin(), command.end(), pathRegex);
	auto end   = std::sregex_iterator();

	size_t lastPos = 0;
	for (auto it = begin; it != end; ++it)
	{
		const std::smatch& m = *it;
		size_t matchPos = (size_t)m.position();
		size_t matchLen = (size_t)m.length();

		// 拷贝匹配之前的原始内容(命令名、参数、空白等)
		result.append(command, lastPos, matchPos - lastPos);

		std::string matched = m.str();
		// 若匹配是被引号包裹的，则保留首尾引号，只转换中间的路径部分
		if (matched.size() >= 2
			&& (matched.front() == '"' || matched.front() == '\'')
			&& matched.back() == matched.front())
		{
			char quote = matched.front();
			std::string inner = matched.substr(1, matched.size() - 2);
			result += quote;
			result += WindowsPathToPosix(normalizeToDrivePath(inner));
			result += quote;
		}
		else
		{
			result += WindowsPathToPosix(normalizeToDrivePath(matched));
		}

		lastPos = matchPos + matchLen;
	}

	// 拷贝最后一段剩余内容
	result.append(command, lastPos, command.size() - lastPos);

	return result;
}

} // namespace Utils
