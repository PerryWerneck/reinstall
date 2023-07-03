#!/bin/bash

if [ "${UID}" != "0" ]; then
	sudo ${0}
	exit 0
fi

kpartx -av /tmp/test.iso
for dev in $(kpartx -l /tmp/test.iso | cut -d: -f1)
do
	mkdir -p /tmp/test/${dev}
	mount /dev/mapper/${dev} /tmp/test/${dev}
done

cd /tmp/test
ls -l

/bin/bash

for dev in $(kpartx -l /tmp/test.iso | cut -d: -f1)
do
	mkdir -p /tmp/test/${dev}
	umount /dev/mapper/${dev}
done

kpartx -d /tmp/test.iso

