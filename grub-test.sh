#!/bin/bash

export pkgdatadir="/usr/share/grub2"
export GRUB_DEVICE="boot"
export GRUB_DISABLE_SUBMENU="true"
#export install-version="INSTALL-VERSION"
#export install-kloading="Loading installation kernel"
#export kernel-fspath="/boot/kernel"
#export kernel-parameters="KPARMS"
#export install-iloading="Loading installation image"
#export initrd-fspath="/boot/initrd"

. ./templates/grub2

