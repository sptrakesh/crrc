cd %0\..

if not exist var mkdir var

docker run -d --name crrc --rm=true -v root:/opt/crrc/root -v var:/opt/crrc/var -p 80:80 crrc updateStart
