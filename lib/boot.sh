source 'lib/common.sh'

function boot {

  pushd 'vendor/iphone-dataprotection'
  log 'Running redsn0w'
  ../redsn0w_mac_0.9.15b3/redsn0w.app/Contents/MacOS/redsn0w \
    --ipsw=${IPSW} \
    --ramdisk=${CUSTOMRAMDISK} \
    --kernelcache=${OUTKERNEL} \
    --tetheredBootLogo=../../share/TOB_Blue.png \
    >/dev/null 2>&1 &
  redsn0w_pid=$!

  log 'Forwarding SSH ports'
  python usbmuxd-python-client/tcprelay.py \
      -t 22:2222 1999:1999 \
      >/dev/null 2>&1 &
  export USBMUX=$!

  log 'Waiting for device'
  while { ! (device uname -a >/dev/null 2>&1) }
  do
    sleep 2
  done

  log 'Booted!'
  kill $redsn0w_pid
  popd
}


