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
&& cd ..

EXPOSE 80
ENTRYPOINT ["/tmp/crrc/entrypoint.sh"]
