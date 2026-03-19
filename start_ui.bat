@echo off
echo Starting Hospital Management System UI...
start "HMS API Server" python server.py
timeout /t 2 /nobreak > nul
start http://127.0.0.1:5000/
echo Done! The UI should open in your default browser.
