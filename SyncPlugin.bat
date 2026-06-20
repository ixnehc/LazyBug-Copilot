:wait_for_processes
@timeout /t 1 >nul
tasklist | find /i "LazyBugService.exe" >nul 2>&1
if %errorlevel% == 0 goto wait_for_processes
tasklist | find /i "LazyBugService_RL.exe" >nul 2>&1
if %errorlevel% == 0 goto wait_for_processes

call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\x64\Retail\LazyBugPlugInControls.dll C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\x64\Retail\LazyBugPlugIn.dll C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\x64\Retail\LazyBugService.exe C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\x64\Retail\LazyBugPlugInUI.dll C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\x64\Retail\LazyBugCppParser.exe C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatCtrlHtml C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatInputHtml C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\CommonHtml C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatSettingPage.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatSkillsTree.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatMcpsTree.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatSkillTip.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\ChatSettingMenu.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\llm_setting_intro.html C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\chatrules_usingtools.txt C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
call "D:\LazyBug\CopyFileToFolder.bat" D:\LazyBug\Proj_LazyBug\llm_default.ini C:\Users\xi.chen\AppData\Local\Microsoft\VisualStudio
pause