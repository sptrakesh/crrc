set cwd=%cd%
set build=%0\..\..\build
if not exist %build% mkdir %build%
cd %build%
cmake -G"Visual Studio 15 2017 Win64" ..
cmake --build . --target clean
cmake --build . --target crrc
cd %cwd%
