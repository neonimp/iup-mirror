@echo off
REM This builds all the libraries for 1 uname
                                  
FOR %%u IN (src srccd srccontrols srcpplot srcgl srcim srcole srclua3 srclua5 srcconsole srcledc srcview) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9                                  