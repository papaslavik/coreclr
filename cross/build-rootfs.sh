#!/usr/bin/env bash

usage()
{
    echo "Usage: $0 [BuildArch]"
    echo "BuildArch can be: arm, arm-softfp, arm64"

    exit 1
}

__UbuntuCodeName=trusty

__CrossDir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
__InitialDir=$PWD
__BuildArch=arm
__UbuntuArch=armhf
__UbuntuRepo="http://ports.ubuntu.com/"
__UbuntuPackages="build-essential lldb-3.6-dev libunwind8-dev gettext symlinks liblttng-ust-dev libicu-dev"
__MachineTriple=arm-linux-gnueabihf
__UnprocessedBuildArgs=
for i in "$@"
    do
        lowerI="$(echo $i | awk '{print tolower($0)}')"
        case $lowerI in
        -?|-h|--help)
        usage
        exit 1
        ;;
        arm)
        __BuildArch=arm
        __UbuntuArch=armhf
        __UbuntuRepo="http://ports.ubuntu.com/"
        __UbuntuPackages="build-essential lldb-3.6-dev libunwind8-dev gettext symlinks liblttng-ust-dev libicu-dev"
        __MachineTriple=arm-linux-gnueabihf
        ;;
        arm64)
        __BuildArch=arm64
        __UbuntuArch=arm64
        __UbuntuRepo="http://ports.ubuntu.com/"
        __UbuntuPackages="build-essential libunwind8-dev gettext symlinks liblttng-ust-dev libicu-dev"
        __MachineTriple=aarch64-linux-gnu
        ;;
        arm-softfp)
        __BuildArch=arm-softfp
        __UbuntuArch=armel
        __UbuntuRepo="http://ftp.debian.org/debian/"
        __UbuntuPackages="build-essential lldb-3.6-dev libunwind8-dev gettext symlinks liblttng-ust-dev libicu-dev"
        __MachineTriple=arm-linux-gnueabi
        __UbuntuCodeName=sid
        ;;
        *)
        __UnprocessedBuildArgs="$__UnprocessedBuildArgs $i"
    esac
done

__RootfsDir="$__CrossDir/rootfs/$__BuildArch"

if [[ -n "$ROOTFS_DIR" ]]; then
    __RootfsDir=$ROOTFS_DIR
fi

umount $__RootfsDir/*
rm -rf $__RootfsDir
qemu-debootstrap --arch $__UbuntuArch $__UbuntuCodeName $__RootfsDir $__UbuntuRepo
cp $__CrossDir/$__BuildArch/sources.list $__RootfsDir/etc/apt/sources.list
chroot $__RootfsDir apt-get update
chroot $__RootfsDir apt-get -y install $__UbuntuPackages
chroot $__RootfsDir symlinks -cr /usr
umount $__RootfsDir/*

