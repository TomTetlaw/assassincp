@echo off

subst w: "E:\stuff\code\assassincp"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set path = w:\misc;%path%

w: