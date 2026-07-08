@echo off
chcp 65001 >nul
title Edge Cognitive Agent PC Server

echo ==========================================
echo   Edge Cognitive Agent PC Server Start
echo ==========================================

cd /d D:\deepseek_proxy

REM 这里填你的 DeepSeek API Key
set DEEPSEEK_API_KEY="YOUR_API_KEY_HERE"

echo [1] Current folder:
cd

echo [2] Start edge_agent.py
echo Dashboard will be available at:
echo http://127.0.0.1:5000/dashboard
echo.

start "" "http://127.0.0.1:5000/dashboard"

python edge_agent.py

echo.
echo edge_agent.py stopped.
pause