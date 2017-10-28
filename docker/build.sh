#!/bin/sh
cd `dirname $0`
docker build --compress --force-rm -f Dockerfile -t crrc .
