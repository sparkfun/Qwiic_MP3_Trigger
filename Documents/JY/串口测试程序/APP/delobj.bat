

::@echo off
rem 正在搜索...
rem 删除文件
for /f "delims=" %%i in ('dir /b /a-d /s "*.obj"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.lst"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.__i"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.bak"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.SBR"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.MAP"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.pch"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.orc"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.ls1"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.i"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.src"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.c.orig"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.h.orig"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.c.pre"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.*.bak"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.uvgui.*"') do del %%i
for /f "delims=" %%i in ('dir /b /a-d /s "*.uvopt"') do del %%i
rem 删除完毕

