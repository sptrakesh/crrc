set cwd=%cd%
set build=%0\..\..\test
if not exist %build% mkdir %build%
cd %build%
cmake -G"Visual Studio 15 2017 Win64" ..
rem cmake --build . --target clean
cmake --build . --target crrcTest
tests\service\Debug\crrcTest.exe
cd %cwd%
