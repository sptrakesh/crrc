FROM sptrakesh/cutelyst

ENV PS1='$PWD \$ ' EDITOR=vi
COPY scripts /tmp/crrc
WORKDIR /opt

RUN git clone https://github.com/sptrakesh/crrc.git \
&& cd crrc \
&& mkdir build \
&& cd build \
&& cmake .. \
&& cmake --build . \
&& cd .. \
&& sqlite3 crrc.db < /tmp/crrc/sqlite/crrc.sql

EXPOSE 80
ENTRYPOINT ["/tmp/crrc/entrypoint.sh"]
