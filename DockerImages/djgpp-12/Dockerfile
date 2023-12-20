# syntax=docker/dockerfile:1

FROM debian:11.5-slim AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    bison \
    flex \
    curl \
    wget \
    git \
    gcc \
    g++ \
    make \
    texinfo \
    zlib1g-dev \
    unzip \
    cmake \
    ninja-build \
    file

RUN git clone https://github.com/andrewwutw/build-djgpp.git && \
    cd build-djgpp && \
    ./build-djgpp.sh 12.1.0


FROM debian:11.5-slim

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y \
    cmake \
    gdb && \
    apt-get clean

COPY --from=build /usr/local/djgpp /usr/local/djgpp/

ENV PATH /usr/local/djgpp/i586-pc-msdosdjgpp/bin/:$PATH
ENV GCC_EXEC_PREFIX /usr/local/djgpp/lib/gcc/
