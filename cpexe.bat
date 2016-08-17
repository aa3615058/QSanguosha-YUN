for %%a in (cd .) do (
set release_dir=%%~dpaQSanguosha-YUN-Build\
)
for %%a in (cd .) do (
set exe_dir=%%~dpaQSanguosha-YUN-EXE\
)

copy "%release_dir%release\QSanguosha.exe" "%exe_dir%QSanguosha.exe"