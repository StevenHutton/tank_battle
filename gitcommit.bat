@echo off

set message="%1"
set message
set message=%message:"=%
set message

git add .
git commit -m "%message%"
git push