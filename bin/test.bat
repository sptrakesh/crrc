set cwd=%cd%
set build=%0\..\..\test
if not exist %test% mkdir %test%
cd %test%
cmake -G"Visual Studio 15 Win64" ..
rem cmake --build . --target clean
cmake --build . --target crrcTest
tests\unit\Debug\crrcTest.exe
cd %cwd%
