# Start the build image
FROM alpine:3.18 as build

# Build arguments
ARG BZ_VERSION=2.4

# Update package list and upgrade, and install build dependencies
RUN apk update && apk upgrade --no-cache && \
    apk add autoconf automake c-ares-dev curl-dev g++ git libtool make zlib

# Create and switch to a normal user for the build
RUN adduser -S builder
USER builder
WORKDIR /home/builder

# Copying README.Linux in case there are no custom plugins, since we need at
# least one valid source.
COPY --chown=builder:nogroup README.Linux custom_plugin[s] custom_plugins/
WORKDIR custom_plugins
RUN find . -mindepth 1 -maxdepth 1 -type d | sed 's|\./||' > ../plugins.txt

# Fetch the code from git and copy in any custom plugins
WORKDIR ..
RUN git clone -b $BZ_VERSION https://github.com/BZFlag-Dev/bzflag.git bzflag-source && \
    mv custom_plugins/* bzflag-source/plugins/ && \
    rm -r custom_plugins/ bzflag-source/plugins/README.Linux
WORKDIR bzflag-source

# Run the build process
RUN ./autogen.sh && \
    ./configure \
        --disable-bzadmin \
        --disable-client \
        --enable-custom-plugins-file=../plugins.txt && \
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
