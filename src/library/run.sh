#!/bin/bash

make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

sudo mount -o loop ~/Público/iso/openSUSE-Leap-15.3-NET-x86_64-Media.iso ~/Público/openSUSE-Leap-15.3-NET-x86_64/

sudo make run
if [ "$?" != "0" ]; then
	exit -1
fi

. ./kvm/libvirt.conf

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


