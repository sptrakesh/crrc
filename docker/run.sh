#!/bin/sh
cd `dirname $0`/..

if [ ! -d var ]
then
  mkdir var
fi

docker run -d \
  --name crrc --rm=true \
  --cpus 4 \
  -v $PWD/root:/opt/crrc/root \
  -v $PWD/var:/opt/crrc/var \
  -p 80:80 \
  crrc updateStart
