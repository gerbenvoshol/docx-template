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
        ca-certificates \
        --no-install-recommends

# Copy source files for the C program
COPY Makefile /tmp/docx-template-render/Makefile
COPY docx-template-render.c /tmp/docx-template-render/docx-template-render.c
COPY mjson.c /tmp/docx-template-render/mjson.c
COPY mjson.h /tmp/docx-template-render/mjson.h
COPY miniz.c /tmp/docx-template-render/miniz.c
COPY miniz.h /tmp/docx-template-render/miniz.h
COPY txml.h /tmp/docx-template-render/txml.h
COPY libtct.c /tmp/docx-template-render/libtct.c
COPY libtct.h /tmp/docx-template-render/libtct.h

# Build and install the C program
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
