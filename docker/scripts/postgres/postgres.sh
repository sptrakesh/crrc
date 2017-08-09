#!/bin/sh
if [ -f $PGDATA/postgresql.conf ]
then
  echo "Database already initialised.  No actions necessary..."
  exec su postgres -c "pg_ctl start -D $PGDATA -l /tmp/psql.log"
  exit 0
fi
chown -R postgres $PGDATA
echo "Initialising PostgreSQL ..."
su postgres -c initdb
echo "Starting PostgreSQL ..."
su postgres -c "pg_ctl start -D $PGDATA -l /tmp/psql.log"
sleep 10
echo "Setting up database ..."
psql -U postgres -c "create user crrc with password 'crrc'"
createdb -U postgres -O crrc crrc
psql -U crrc crrc -f crrc.sql
