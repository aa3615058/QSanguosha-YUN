for %%a in (cd .) do (
set exe_dir=%%~dpaQSanguosha-YUN-EXE\
)
set qt_lib="%QT_HOME%\bin\"

copy "%qt_lib%icudt53.dll" "%exe_dir%"
copy "%qt_lib%icuin53.dll" "%exe_dir%"
copy "%qt_lib%icuuc53.dll" "%exe_dir%"
copy "%qt_lib%libgcc_s_dw2-1.dll" "%exe_dir%"
copy "%qt_lib%libstdc++-6.dll" "%exe_dir%"
copy "%qt_lib%libwinpthread-1.dll" "%exe_dir%"
copy "%qt_lib%Qt5Core.dll" "%exe_dir%"
copy "%qt_lib%Qt5Declarative.dll" "%exe_dir%"
copy "%qt_lib%Qt5Gui.dll" "%exe_dir%"
copy "%qt_lib%Qt5Network.dll" "%exe_dir%"
copy "%qt_lib%Qt5Script.dll" "%exe_dir%"
copy "%qt_lib%Qt5Sql.dll" "%exe_dir%"
copy "%qt_lib%Qt5Widgets.dll" "%exe_dir%"
copy "%qt_lib%Qt5XmlPatterns.dll" "%exe_dir%"

pause