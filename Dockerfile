FROM alpine:3.18

ARG BZ_VERSION

RUN apk update && apk upgrade --no-cache
RUN apk add autoconf automake c-ares-dev curl-dev g++ git libtool make zlib

RUN touch plugins.txt
RUN git clone -b $BZ_VERSION https://github.com/BZFlag-Dev/bzflag.git
WORKDIR /bzflag

RUN ./autogen.sh
RUN ./configure \
        --disable-bzadmin \
        --disable-client \
        --enable-custom-plugins-file=../plugins.txt
RUN make -j$(nproc)

RUN adduser -S bzfsd
USER bzfsd

ENTRYPOINT [ "src/bzfs/bzfs" ]
