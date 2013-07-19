source 'lib/common.sh'

# Generate an mtree specification for a given iOS firmware version.
# Specification is saved to specs/<version>.spec.
#
# Arguments:
#
#   1 - the firmware version to build a specification for
#
# Environment:
#
#   IPSW - path to downloaded iOS firmware, relative to project root
#
function spec {
  local version=$1

  mkdir -p 'specs'
  local dest="specs/${version}.spec"
  if [[ -f $dest ]]
  then
    log $dest 'exists, remove to regenerate'
    return
  fi

  log 'Building vfdecrypt'
  check make -C 'vendor/vfdecrypt' mac

  log 'Extracting root filesystem'
  check unzip -jo ${IPSW} ${IPSW_ROOT[$version]} -d 'ipsw/'

  log 'Decrypting root filesystem'
  check vendor/vfdecrypt/vfdecrypt \
      -i "ipsw/${IPSW_ROOT[$version]}" \
      -k ${IPSW_KEYS[$version]} \
      -o "decrypted.dmg"

  log 'Mounting'
  local mount_point
  mount_point=$(hdiutil attach "decrypted.dmg" | awk '/Volumes/ { print $3 }')
  log "Mounted to $mount_point"

  log 'Generating mtree spec (need root)'
  sudo mtree -c \
      -k $MTREE_KEYWORDS \
      -p $mount_point \
      -X share/mtree_exclude \
      >"specs/${version}.spec"

  log 'Ejecting'
  check hdiutil eject $mount_point

  log 'Cleaning up'
  rm "decrypted.dmg"
  rm "ipsw/${IPSW_ROOT[$version]}"
}
