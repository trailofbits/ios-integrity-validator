#!/usr/bin/env zsh

source 'lib/common.sh'
CACHE="${HOME}/Library/Caches/com.trailofbits.iverify"

# Download an IPSW from Apple, or load it from ~/Library/Caches.
#
# $1 - the version of iOS to download
#
# Examples
#
#   source 'lib/versions/ipod4g.sh'
#   download '5.1.1'
#
# Note that you must load a version file first, as shown in the example!
#
function download_ipsw {
  url=${IPSW_URLS[$1]}
  package="${CACHE}/$(basename ${url})"
  dest='ipsw'
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

function download_redsn0w {
  url='https://sites.google.com/a/iphone-dev.com/files/home/redsn0w_mac_0.9.15b3.zip'
  package="${CACHE}/$(basename ${url})"
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
# $1 - the file to hash
#
function sha {
  shasum ${1} | awk '{print $1}'
}

