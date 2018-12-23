::
:: Created on:	Jan. 03 2018
:: Autor:	Davit Kalantaryan
::
:: Purpose:	This script sets the correct platfor for window compilation
::
:: Argumet list:This script expects 1 argumets, that shows which server to start
::

@ECHO off

::SETLOCAL enableextensions

::set lsbRelease=win64

:: calculating current directory
SET  scriptDirectory=%~dp0
::set  currentDirectory=%cd%

set MATLABPATH=%scriptDirectory%..\..\sys\win64\mbin;%scriptDirectory%..\src\matlabscripts;%MATLABPATH%
set Path=%scriptDirectory%..\..\sys\win64\dll;%scriptDirectory%..\..\sys\win64\bin;%Path%

::cd %currentDirectory%

::ENDLOCAL
