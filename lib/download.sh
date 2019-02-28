source 'lib/common.sh'
CACHE="${HOME}/Library/Caches/com.trailofbits.iiv"

# Download an IPSW from Apple, or load it from ~/Library/Caches.
#
# Arguments:
#
#   1 - the version of iOS to download
#
# Exports:
#
#   IPSW - path to downloaded firmware relative to the current directory
#
# Examples:
#
#   source 'lib/versions/ipod4g.sh'
#   download '5.1.1'
#
# Note that you must load a version file first, as shown in the example!
#
function download_ipsw {
  local url=${IPSW_URLS[$1]}
  local package="${CACHE}/$(basename ${url})"
  local dest='ipsw'
  mkdir -p $dest

  if [[ -f $package &&
        ${IPSW_SHA1[$1]} == $(sha $package) ]]
  then
    log 'Using cached' $package
  else
    log 'Downloading' $package
    curl --progress-bar --location --output ${package} ${url}
  fi

  export IPSW="${dest}/$(basename ${package})"
  cp ${package} "${IPSW}"
}

# Download redsn0w and install it to vendor/, install Keys.plist into
# vendor/iphone-dataprotection.
# This function also caches to ~/Library/Caches.
#
# Exports:
#
#   REDSN0W - path to redsn0w installation
#
function download_redsn0w {
  local url='https://sites.google.com/a/iphone-dev.com/files/home/redsn0w_mac_0.9.15b3.zip'
  local package="${CACHE}/$(basename ${url})"
  local basename
  basename=$(basename -s '.zip' ${package})

  if [[ -f ${package} &&
        'd32a6dd5a2a5ff7c7f05bebaba24992abe291e08' == $(sha ${package}) ]]
  then
    log 'Using cached' $package
  else
    log 'Downloading' ${package}
    curl --progress-bar --location --output ${package} ${url}
  fi

  export REDSN0W=$basename
  check unzip -o ${package} -d 'vendor'
  check cp "vendor/${basename}/redsn0w.app/Contents/MacOS/Keys.plist" 'vendor/iphone-dataprotection'
}

# Return the sha1 hash of a file.
#
# Arguments:
#
#   $1 - the file to hash
#
function sha {
  shasum $1 | awk '{print $1}'
}

