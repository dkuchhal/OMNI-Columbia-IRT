#!/bin/sh

# Unified Heterogeneous Networking Middleware
# Android Cross Compile - Compile Middleware
# Lingyuan He - 04/2015

# for cross compile
INSTALL_PATH=$(pwd)/install
TOOLCHAIN=arm-linux-gnueabi-
BUILD=x86_64-unknown-linux-gnu
HOST=arm-none-linux

# software version
ZLIB_VER=1.2.8
CURL_VER=7.41.0
WRAPPER_VER=7.6

# work folder
cd work

# zlib
if [ ! -f zlib-$ZLIB_VER.tar.gz ]; then
	wget http://zlib.net/zlib-$ZLIB_VER.tar.gz
fi
tar zxvf zlib-$ZLIB_VER.tar.gz
cd zlib-$ZLIB_VER
CC=${TOOLCHAIN}gcc AR=${TOOLCHAIN}ar RANLIB=${TOOLCHAIN}ranlib ./configure --prefix=$INSTALL_PATH
make
make install
cd ../

# tcp wrapper
if [ ! -f tcp_wrappers_$WRAPPER_VER.tar.gz ]; then
	wget ftp://ftp.porcupine.org/pub/security/tcp_wrappers_$WRAPPER_VER.tar.gz
fi
tar zxvf tcp_wrappers_$WRAPPER_VER.tar.gz
cd tcp_wrappers_$WRAPPER_VER
# apply patches
patch -p0 < ../../tcp_wrapper.patch
CC=${TOOLCHAIN}gcc RANLIB=${TOOLCHAIN}ranlib make REAL_DAEMON_DIR=./ android
cp libwrap.a ../../install/lib
cd ../

# curl
if [ ! -f curl-$CURL_VER.tar.gz ]; then
	wget http://curl.haxx.se/download/curl-$CURL_VER.tar.gz
fi
tar zxvf curl-$CURL_VER.tar.gz
cd curl-$CURL_VER
CC=${TOOLCHAIN}gcc AR=${TOOLCHAIN}ar RANLIB=${TOOLCHAIN}ranlib LD=${TOOLCHAIN}ld NM=${TOOLCHAIN}nm CPPFLAGS=-I$INSTALL_PATH/include LDFLAGS=-L$INSTALL_PATH/lib LIBS="-lz -lssl -lcrypto" ./configure --prefix=$INSTALL_PATH --host=$HOST
make
make install
cd ../

# middleware
cp -a ../../middleware ./
cd middleware
make clean
cd ./srelay/srelay-0.4.8b5
./configure
cd ../../
# compile
TOOLSET=${TOOLCHAIN} LDFLAGS="-L${INSTALL_PATH}/lib -L/usr/arm-linux-gnueabi/lib"  ANDROID="-I${INSTALL_PATH}/include" make
# copy output
cp sined ../../install/sined
cp kill_sined ../../install/sined
cp locsw ../../install/sined

echo ""
echo "Cross compilation for middleware is done"
echo ""

