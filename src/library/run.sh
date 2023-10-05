#!/bin/bash

VM_URI="qemu:///system"
VM_NAME="reinstall"
VM_TYPE="efi"

if [ -e ./kvm/libvirt.conf ]; then
	. ./kvm/libvirt.conf
fi

isolinux() {

	virsh \
		-c "${VM_URI}" \
		undefine \
		"${VM_NAME}"

	sudo qemu-img \
		create \
		-f qcow2 \
		/tmp/reinstall.qcow2 \
		20G
	if [ "$?" != "0" ]; then
		exit -1
	fi
	
	virsh \
		-c "${VM_URI}" \
		define \
		kvm/isolinux-usb.xml
	if [ "$?" != "0" ]; then
		exit -1
	fi

}

efi() {

	virsh \
		-c "${VM_URI}" \
		undefine \
		"${VM_NAME}"

	sudo qemu-img \
		create \
		-f qcow2 \
		/tmp/reinstall.qcow2 \
		20G
	if [ "$?" != "0" ]; then
		exit -1
	fi

	cp kvm/vars.fd /tmp/vars.fd
	if [ "$?" != "0" ]; then
		exit -1
	fi

	virsh \
			-c "${VM_URI}" \
			define \
			kvm/efi.xml

}

make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

mkdir -p ~/Público/openSUSE-Leap-15.4-NET-x86_64/
if [ "$(grep -c Público/openSUSE-Leap-15.4-NET-x86_64 /proc/mounts)" == "0" ]; then
	sudo mount -o loop ~/Público/iso/openSUSE-Leap-15.4-NET-x86_64-Build243.2-Media.iso ~/Público/openSUSE-Leap-15.4-NET-x86_64/
fi

#sudo setcap cap_dac_override,cap_setuid,cap_setgid,cap_chown,cap_sys_admin+ep .bin/Debug/reinstall 

.bin/Debug/reinstall
if [ "$?" != "0" ]; then
	exit -1
fi

if [ "$1" == "--efi" ]; then
	efi
fi

if [ "$1" == "--isolinux" ]; then
	isolinux
fi

if [ "$1" == "--shell" ]; then
	sudo mount -o loop /tmp/test.iso /mnt
	cd /mnt
	echo "exit to umount"
	/bin/bash
	cd $(dirname $(readlink -f ${0}))
	sudo umount /mnt
fi


