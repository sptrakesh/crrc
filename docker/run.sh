#!/bin/sh
cd `dirname $0`/..

if [ ! -d /var/tmp/crrc ]
then
  mkdir /var/tmp/crrc
fi

docker run -it \
  -v /private/var/tmp/crrc:/var/lib/postgresql/9.6/data \
  --name crrc --rm=true \
  -p 3000:3000 -p 5432:5432 \
  crrc
