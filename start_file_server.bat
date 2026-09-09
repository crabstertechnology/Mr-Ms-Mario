@echo off
title Luna File Sharing Server
cd /d "%~dp0"
echo ===================================================
echo Starting Luna File Sharing Server...
echo ===================================================
python file_server.py
pause
