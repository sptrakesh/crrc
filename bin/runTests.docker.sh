#!/bin/sh

DIR=`dirname $0`/..
PID_FILE=$DIR/var/crrc.pid
CUTELYST_DIR=/opt/local
cd $DIR

init()
{
  mkdir -p build/crrc
  mkdir -p build/test
}

compile()
{
  (cd build/crrc; \
    cmake -DCMAKE_PREFIX_PATH=/opt/local:$CUTELYST_DIR ../..; \
    cmake --build . --target crrc -- -j8)
  (cd build/test; \
    cmake -DCMAKE_PREFIX_PATH=/opt/local:$CUTELYST_DIR ../..; \
    cmake --build . --target crrcTest -- -j8)
}

moveDatabase()
{
  if [ -f var/crrc.db ]
  then
    mv var/crrc.db var/crrc.db.bak
  fi

  sqlite3 var/crrc.db < docker/scripts/sqlite/crrc.sql
  sqlite3 var/crrc.db < docker/scripts/sqlite/crrc1.sql
  sqlite3 var/crrc.db < docker/scripts/sqlite/crrc2.sql
}

restoreDatabase()
{
  if [ -f var/crrc.db.bak ]
  then
    mv var/crrc.db.bak var/crrc.db
  fi
}

checkRunning()
{
  STATUS=0

  if [ -f $PID_FILE ]
  then
    pgrep -P `cat $PID_FILE` > /dev/null || echo "Stale PID file found"
    pgrep -P `cat $PID_FILE` > /dev/null && STATUS=1
  fi

  return $STATUS
}

crrc()
{
  checkRunning
  if [ $? -ne 0 ]
  then
    echo "Process with PID: `cat $PID_FILE` still running..."
    exit 2
  fi

  if [ ! -d var ]
  then
    mkdir var
  fi

  moveDatabase

  $CUTELYST_DIR/bin/cutelyst --server -p 3000 --app-file $PWD/build/crrc/src/libcrrc.so -- > var/crrc.log 2>&1 &
  echo $! > $PID_FILE
  echo "Started CRRC application with PID: `cat $PID_FILE`"
}

run()
{
  echo "Sleeping 5 seconds to make sure CRRC application has started"
  sleep 5
  build/test/tests/service/crrcTest
}

stop()
{
  if [ -f $PID_FILE ]
  then
    pkill -P `cat $PID_FILE`
    kill `cat $PID_FILE`
    echo "Stopped CRRC application with PID: `cat $PID_FILE`"
    rm -f $PID_FILE
  fi
  restoreDatabase
}

init
compile
crrc
run
stop
