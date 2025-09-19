@echo off
color b
title TS file Format
REM -------------------------------
REM Qt TS files format batch script
REM -------------------------------

REM Path to lconvert
set LCONVERT="C:\Qt\6.9.1\mingw_64\bin\lconvert.exe"

REM TS dosyaları listesi
set TSFILES=en_US.ts tr_TR.ts de_DE.ts fr_FR.ts it_IT.ts es_ES.ts ru_RU.ts

REM Döngüyle her dosyayı formatla
for %%f in (%TSFILES%) do (
    if exist "%%f" (
        echo Formatting %%f ...
        %LCONVERT% -i "%%f" -o "%%f"
        if errorlevel 1 (
            echo Failed to format %%f
        ) else (
            echo Successfully formatted %%f
        )
    ) else (
        echo File %%f not found!
    )
)

echo All done.
pause
