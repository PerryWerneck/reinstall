#!/bin/bash
TEMPFILE=$(mktemp)
sudo kpartx -av /tmp/test.iso | tee ${TEMPFILE}

PART1=$(awk '{if(NR==1) print $0}' ${TEMPFILE} | awk '{print $3}')
PART2=$(awk '{if(NR==2) print $0}' ${TEMPFILE} | awk '{print $3}')

echo "PART1=${PART1}	PART2=${PART2}"

mkdir -p /tmp/reinstall/iso/p1
mkdir -p /tmp/reinstall/iso/p2

cd /tmp/reinstall/iso
sudo mount "/dev/mapper/${PART1}" "/tmp/reinstall/iso/p1"
sudo mount "/dev/mapper/${PART2}" "/tmp/reinstall/iso/p2"

echo "Press exit to release"
/bin/bash

sudo umount "/tmp/reinstall/iso/p1"
sudo umount "/tmp/reinstall/iso/p2"

sudo kpartx -d /tmp/test.iso
