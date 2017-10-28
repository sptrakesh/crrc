if not exist var mkdir var
if not exist var\crrc.db (
  echo Creating database
  sqlite3 var\crrc.db < %0\..\..\docker\scripts\sqlite\crrc.sql
)
\opt\cutelyst\bin\cutelyst --server --app-file %0\..\..\build\src\Debug\crrc.dll -- --chdir2 ..
