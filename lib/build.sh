source 'lib/common.sh'

# Build a kernel and ramdisk for a given iOS version.
#
# Arguments:
#
#   1 - version of iOS for which to build
#
# Environment:
#
#   IPSW - path to iOS firmware, relative to project root
#
# Exports:
#
#   IPSW          - path to iOS firmware, relative to iphone-dataprotection
#   RAMDISKNAME   - path to extracted ramdisk
#   KEY           - key to decrypt ramdisk
#   IV            - IV to decrypt ramdisk
#   CUSTOMRAMDISK - path to write custom ramdisk
#   OUTKERNEL     - path to patched kernel
#
function build_ramdisk {
  local version=$1
  if (( ! ${+IPSW_URLS[$version]} ))
  then
    err "No available firmware for '$version'"
    exit
  fi

  pushd 'vendor/iphone-dataprotection'
  log 'make img3fs'
  check make -C img3fs/

  log 'Patching kernel'
  typeset -a args
  args=($(python python_scripts/kernel_patcher.py "../../${IPSW}"))

  export IPSW=${args[1]}
  export RAMDISKNAME=${args[2]}
  export KEY=${args[3]}
  export IV=${args[4]}
  export CUSTOMRAMDISK=${args[5]}
  export OUTKERNEL=${args[6]}

  log 'Building ramdisk_tools'
  SDKVER='6.1' check make -C ramdisk_tools

  log 'Building ramdisk'
  check ./build_ramdisk.sh ${IPSW} ${RAMDISKNAME} ${KEY} ${IV} ${CUSTOMRAMDISK}

  popd
}


