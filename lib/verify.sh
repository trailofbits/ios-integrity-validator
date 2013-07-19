source 'lib/common.sh'

function verify {
  version=$1

  log 'Mounting filesystem'
  mountpoint='/mnt1'
  device mount_hfs -o ro /dev/disk0s1s1 $mountpoint

  log 'Verifying filesystem'
  spec="specs/${version}.spec"
  output=$(device /usr/local/bin/mtree \
    -k $MTREE_KEYWORDS \
    -X /etc/mtree_exclude \
    -p /mnt1 \
    <${spec})

  if (( $? == 2 ))
  then
    warn 'Device filesystem has been modified'
  elif (( $? == 1 ))
  then
    err 'Error running mtree'
    halt
  fi

  evidence="evidence/$(date '+%F-%H%M%S')"
  log 'Copying added/modified files to' $evidence
  for changed in $(echo $output | awk '/\S+ (changed|extra)$/ { print $1 }')
  do
    dest="$evidence/$(dirname $changed)"
    mkdir -p $dest
    scp -pr -P 2222 root@localhost:"$mountpoint/$changed" $dest
  done

  halt
}

function halt {
  log 'Shutting down device'
  device /sbin/halt
  kill $USBMUX
  exit
}

