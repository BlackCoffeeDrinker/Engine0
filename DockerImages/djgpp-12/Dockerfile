# syntax=docker/dockerfile:1
FROM debian:13-slim AS build

WORKDIR /build

ENV CC=gcc
ENV CXX=g++
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -yq \
      bison \
      flex \
      curl \
      gcc \
      g++ \
      make \
      texinfo \
      zlib1g-dev \
      g++ \
      unzip \
      git bzip2 xz-utils
RUN git clone https://github.com/andrewwutw/build-djgpp.git
RUN cd build-djgpp &&  \
    ./build-djgpp.sh 12.2.0

FROM debian:13-slim
ENV DEBIAN_FRONTEND=noninteractive
ENV CC=i586-pc-msdosdjgpp-gcc
ENV CXX=i586-pc-msdosdjgpp-g++
RUN apt-get update && \
    apt-get install -yq \
        cmake \
        ninja-build \
        git
COPY --from=build /usr/local/djgpp /usr/local/djgpp
ENV PATH="/usr/local/djgpp/bin:/usr/local/djgpp/i586-pc-msdosdjgpp/bin/:/usr/local/djgpp/libexec/gcc/i586-pc-msdosdjgpp/12.2.0/:$PATH"
ENV GCC_EXEC_PREFIX="/usr/local/djgpp/lib/gcc/"
ENV DJDIR="/usr/local/djgpp/i586-pc-msdosdjgpp"
