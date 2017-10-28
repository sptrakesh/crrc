#!/bin/sh
cd `dirname $0`/..

if [ ! -d var ]
then
  mkdir var
fi

docker run -it \
  --name crrc --rm=true \
  -v $PWD/root:/opt/crrc/root \
  -v $PWD/var:/opt/crrc/var \
  -p 8080:80 \
  crrc updateStart
