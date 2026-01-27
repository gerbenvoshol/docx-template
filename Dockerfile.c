# Dockerfiles for building images with LibreOffice and docx-template-render (C version)

FROM instructure/libreoffice:5.2
MAINTAINER Serhii Moroz <frost.asm@gmail.com>

ENV DEBIAN_FRONTEND noninteractive

USER root

# Install build dependencies and runtime dependencies
RUN locale-gen en_US.UTF-8 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get update \
    && echo "Y" | apt-get install -y \
        mime-support \
        build-essential \
        gcc \
        make \
        git \
        cmake \
        libzip-dev \
        libxml2-dev \
        ca-certificates \
        --no-install-recommends

# Clone and build libtct
RUN cd /tmp \
    && git clone https://github.com/gerbenvoshol/libtct.git \
    && cd libtct \
    && make \
    && make install \
    && ldconfig

# Clone and build doctxt
RUN cd /tmp \
    && git clone https://github.com/gerbenvoshol/doctxt.git \
    && cd doctxt \
    && make \
    && make install \
    && ldconfig

# Copy and build the C program
COPY Makefile /tmp/docx-template-render/Makefile
COPY docx-template-render.c /tmp/docx-template-render/docx-template-render.c

RUN cd /tmp/docx-template-render \
    && make \
    && make install

# Cleanup
RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

USER docker
