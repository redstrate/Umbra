#!/bin/sh

flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo &&
flatpak-builder build --user --force-clean --install-deps-from=flathub zone.xiv.umbra.yml &&
flatpak build-export export build &&
flatpak build-bundle export umbra.flatpak zone.xiv.umbra --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
