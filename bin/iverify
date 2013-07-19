#!/usr/bin/env zsh

source 'lib/common.sh'
source 'lib/build.sh'
source 'lib/boot.sh'
source 'lib/verify.sh'
source 'lib/download.sh'
source 'lib/spec.sh'

if (( $# != 1 ))
then
  err 'Invalid arguments'
  cat <<EOF
  Usage: $0 VERSION
EOF
  exit
fi

version=$1

download_redsn0w
download_ipsw $version
spec $version

download_ipsw '5.1.1'
build_ramdisk '5.1.1'
boot

verify $version


