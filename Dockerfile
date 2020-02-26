FROM ubuntu:18.04

MAINTAINER todaylg

RUN  sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
RUN  apt-get clean

RUN apt-get -y update --fix-missing && apt-get install -y \
    p7zip-full \
    zip \
    ccache \
    cmake \
    g++ \
    libgif-dev \
    libwebp-dev \
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    libtbb-dev \
    libboost-dev libboost-filesystem-dev libboost-regex-dev libboost-system-dev libboost-thread-dev libboost-python-dev \
    libopenimageio-dev \
    openexr \
    openimageio-tools \
    python \
    wget

RUN echo "/usr/local/lib64/" >/etc/ld.so.conf.d/lib64.conf
RUN echo "/usr/local/lib/" >/etc/ld.so.conf.d/lib.conf

# envtools
RUN rm -Rf /root/envtools
RUN mkdir /root/envtools
COPY ./ /root/envtools/

RUN mkdir /root/envtools/release && cd /root/envtools/release && cmake -DCMAKE_BUILD_TYPE=Release  ../ && make -j6 install
