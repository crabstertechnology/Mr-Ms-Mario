@echo off
title Luna Display Studio Launcher
cd /d "W:\Mr.mario\luna_ui_studio"
echo ===================================================
echo   Starting Luna Display Studio (ESP32-S3 1.69")
echo ===================================================
start "" "http://localhost:5173/"
npm run dev -- --port 5173 --host
pause
