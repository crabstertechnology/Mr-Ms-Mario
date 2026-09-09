@echo off
title Luna FTP Server
cd /d "%~dp0"
echo ===================================================
echo Starting Luna FTP Server for File Explorer...
echo ===================================================
python start_ftp_server.py
pause
