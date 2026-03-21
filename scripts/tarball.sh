#!/bin/sh

# ensure the submodules are up to date
git submodule init &&
git submodule update &&

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='umbra-source.tar.gz' --exclude='.flatpak-builder' --exclude='export' --exclude='build' --exclude='umbra.flatpak' --exclude-vcs -zcvf ../umbra-source.tar.gz .
