MSBUILD := "C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"

.PHONY: build
build:
	${MSBUILD} "codxe.sln"

.PHONY: clean
clean:
	${MSBUILD} "codxe.sln" /t:Clean

.PHONY: format
format:
	powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp,*.hpp,*.h,*.c | ForEach-Object { clang-format --style=file -i $$_.FullName }"

.PHONY: format-check
format-check:
	powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp,*.hpp,*.h,*.c | ForEach-Object { clang-format --dry-run --Werror $$_.FullName }"
