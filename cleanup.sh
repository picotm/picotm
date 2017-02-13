#!/bin/sh

export MAKEFLAGS=-j4

make -C lib/libc/alloc clean \
&& make -C lib/libc/fd clean \
&& make -C lib/libc/fs clean \
&& make -C lib/libc/txproto clean \
&& make clean \
&& make -C include ceuta-clean \
&& make -C lib/libc ceuta-clean \
&& make -C lib/libm ceuta-clean \
&& make -C lib/libpthread ceuta-clean

