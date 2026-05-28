@echo off
chcp 65001 >nul

set COMPILER=D:\mingw64\bin\g++.exe
set PROJECT_DIR=%~dp0
set OUTPUT=%PROJECT_DIR%main.exe

echo --------------------------------------
echo 正在收集超级马里奥项目的源文件...

set SOURCES=
for /r "%PROJECT_DIR%" %%f in (*.cpp) do (
    call set SOURCES=%%SOURCES%% "%%f"
)

echo 编译文件列表: %SOURCES%
echo 编译中...
%COMPILER% -fdiagnostics-color=always -g -I"%PROJECT_DIR%" -I"%PROJECT_DIR%include" %SOURCES% -o "%OUTPUT%" -leasyx

if %errorlevel% equ 0 (
    echo --------------------------------------
    echo 编译成功！生成 main.exe
) else (
    echo --------------------------------------
    echo 编译失败！请检查代码错误。
)