set cwd=%cd%
set build=%0\..\..\test
if not exist %build% mkdir %build%
cd %build%
cmake -G"Visual Studio 15 Win64" ..
cmake --build . --target clean
cmake --build . --target crrcTest
tests\service\Debug\crrcTest.exe
cd %cwd%
