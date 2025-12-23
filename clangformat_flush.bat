@echo off
echo Formatting all source files in src...
for /r src %%f in (*.cpp *.h *.hpp) do (
    echo Formatting: %%f
    clang-format -i --style=file "%%f"
)
echo Done!
pause