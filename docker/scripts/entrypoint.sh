#!/bin/sh

PID_FILE=/opt/crrc/build/crrc.pid
cd /opt/crrc

checkRunning()
{
  STATUS=0

  if [ -f $PID_FILE ]
  then
    pgrep -F $PID_FILE > /dev/null || echo "Stale PID file found"
    pgrep -F $PID_FILE > /dev/null && STATUS=1
  fi

  return $STATUS
}

update()
{
  git pull
  (cd build; cmake ..; cmake --build .)
  echo "Successfully build crrc application"
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

  if [ ! -f var/crrc.db ]
  then
    sqlite3 var/crrc.db < /tmp/crrc/sqlite/crrc.sql
  fi

  /opt/local/bin/cutelyst -r --server -p 80 --app-file $PWD/build/src/libcrrc.so -- > crrc.log 2>&1 &
  echo $! > $PID_FILE
  echo "Started CRRC application with PID: `cat $PID_FILE`"
}

loop()
{
  tail -f crrc.log
}

stop()
{
  if [ -f $PID_FILE ]
  then
    pkill -P `cat $PID_FILE`
    rm -f $PID_FILE
  fi
}

cleanup()
{
  echo "Container stopped, performing cleanup..."
  stop
}

trap 'true' SIGTERM

case "$1" in
  'start')
    crrc
    loop
    ;;
  'updateStart')
    update
    crrc
    loop
    ;;
  'stop')
    stop
    ;;
  *)
    echo "Usage: $0 <start|updateStart|stop>"
    exit 1
    ;;
esac

cleanup
