# Start the build image
FROM alpine:3.18 as build

# The configure argument is passed to the ./configure script
ARG configure

# Update package list and upgrade, and install build dependencies
RUN apk update && apk upgrade --no-cache && \
    apk add autoconf automake c-ares-dev curl-dev g++ libtool make zlib

# Create and switch to a normal user for the build
RUN adduser -S builder
USER builder
WORKDIR /home/builder

# Copy the local source directory into the docker environment
COPY --chown=builder:nogroup . bzflag/
WORKDIR /home/builder/bzflag

# Run the build process
RUN ./autogen.sh && \
    ./configure \
        --disable-bzadmin \
        --disable-client \
        $configure && \
    make -j$(nproc)

# Switch to root to run the install step
USER root
RUN make install-strip

# Start the final image
FROM alpine:3.18

# Update package list and upgrade, and install runtime dependencies. Add a user
# and create a data directory owned by said user.
RUN apk update && apk upgrade --no-cache && \
    apk add c-ares libcurl libgcc libstdc++ zlib && \
    adduser -S bzfsd && \
    mkdir /data && chown bzfsd:nogroup /data

# Copy in files from the build
COPY --from=build /usr/local/bin/bzfs /usr/local/bin/bzfs
COPY --from=build /usr/local/lib/bzflag /usr/local/lib/bzflag

# Switch to the user and work out of the /data directory, which can be passed
# in as a volume.
USER bzfsd
WORKDIR /data
VOLUME /data

ENTRYPOINT [ "/usr/local/bin/bzfs" ]

