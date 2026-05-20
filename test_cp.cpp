#include <windows.h>
#include <iostream>
#include <string>

int main() {
    std::string cmdLine = "cmd.exe /c python -c \"\nprint('hello')\n\"";
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (CreateProcessA(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "Process created successfully.\n";
    } else {
        std::cout << "CreateProcessA failed.\n";
    }
    return 0;
}