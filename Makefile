MSBUILD := "C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"

.PHONY: build
build:
	${MSBUILD} "codxe.sln" /m /p:Configuration=Release /p:Platform="Xbox 360"

.PHONY: clean
clean:
	${MSBUILD} "codxe.sln" /t:Clean /p:Configuration=Release /p:Platform="Xbox 360"

.PHONY: format
format:
	powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp,*.hpp,*.h,*.c | ForEach-Object { clang-format --style=file -i $$_.FullName }"

.PHONY: format-check
format-check:
	powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp,*.hpp,*.h,*.c | ForEach-Object { clang-format --dry-run --Werror $$_.FullName }"

.PHONY: package
package:
	python build.py

XENIA_ROOT ?= C:\Users\Michael\xenia_canary_windows

.PHONY: deploy-xenia
deploy-xenia: build
	python build.py --package-only
	xcopy /E /I /Y "build\staging\xenia\plugins" "${XENIA_ROOT}\plugins"
	xcopy /E /I /Y "build\staging\xenia\patches" "${XENIA_ROOT}\patches"
