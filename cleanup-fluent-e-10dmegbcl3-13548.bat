echo off
set LOCALHOST=%COMPUTERNAME%
set KILL_CMD="C:\PROGRA~1\ANSYSI~1\v195\fluent/ntbin/win64/winkill.exe"

"C:\PROGRA~1\ANSYSI~1\v195\fluent\ntbin\win64\tell.exe" e-10dmegbcl3 55813 CLEANUP_EXITING
if /i "%LOCALHOST%"=="e-10dmegbcl3" (%KILL_CMD% 16712) 
if /i "%LOCALHOST%"=="e-10dmegbcl3" (%KILL_CMD% 13548) 
if /i "%LOCALHOST%"=="e-10dmegbcl3" (%KILL_CMD% 18932)
del "H:\PPO_Fluent\CSF\1\cleanup-fluent-e-10dmegbcl3-13548.bat"
