::该文件辅助调试环境
@echo off
for %%a in (cd .) do (
set exe_dir=%%~dpaQSanguosha-YUN-EXE\
)
set yun_dir=%~dp0

mklink /J "%exe_dir%audio" "%yun_dir%audio"
mklink /J "%exe_dir%image" "%yun_dir%image"
mklink /J "%exe_dir%lang" "%yun_dir%lang"
mklink /J "%exe_dir%lua" "%yun_dir%lua"

pause