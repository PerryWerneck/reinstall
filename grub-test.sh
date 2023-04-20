#!/bin/bash

export pkgdatadir="/usr/share/grub2"
export GRUB_DEVICE="boot"
export GRUB_DISABLE_SUBMENU="true"

. ./templates/grub2

