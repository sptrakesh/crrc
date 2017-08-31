cd %0\..
docker build --compress --squash --force-rm -f Dockerfile -t crrc .
