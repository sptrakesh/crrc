FROM sptrakesh/cutelyst:1.10

ENV PS1='$PWD \$ ' EDITOR=vi
COPY scripts /tmp/crrc
WORKDIR /opt

RUN git clone https://github.com/sptrakesh/crrc.git \
&& cd crrc \
&& mkdir build \
&& cd build \
&& cmake .. \
&& cmake --build . -- -j4 \
&& cd ..

EXPOSE 80
ENTRYPOINT ["/tmp/crrc/entrypoint.sh"]
