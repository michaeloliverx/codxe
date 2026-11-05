@echo off
setlocal
pushd "%~dp0.."
if not defined VS100COMNTOOLS set "VS100COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\"
call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86 >nul
if errorlevel 1 goto failed
if not exist "build\validation\iw3-demo" mkdir "build\validation\iw3-demo"
cl /nologo /EHsc /W4 /WX tests\iw3_demo_stream_tests.cpp /Fobuild\validation\iw3-demo\stream-tests.obj /Febuild\validation\iw3-demo\stream-tests.exe
if errorlevel 1 goto failed
build\validation\iw3-demo\stream-tests.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
