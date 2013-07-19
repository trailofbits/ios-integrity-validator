#!/usr/bin/env zsh

# Logs what directory is being entered and then calls builtin `cd`.
#
# $1 - Directory to change to
#
function cd {
  builtin cd $1
  log 'Entering' $(pwd)
}

# Print line to terminal.
# Includes a colored prefix to differentiate iVerify output from other output.
#
# $1... - Arguments to print
#
function log {
  print "\x1b[34m==> \x1b[37m$*\x1b[0m"
}

# Print an error to terminal, then exit.
# Includes a colored prefix to differentiate iVerify output from other output.
#
# $1... - Arguments to print
#
function err {
  print "\x1b[31;4mError\x1b[0m: $*"
}

function warn {
  print "\x1b[31;4mWarning\x1b[0m: $*"
}

function check {
  logdir="$HOME/Library/Logs/com.trailofbits.iverify"
  mkdir -p $logdir

  timestamp=$(date '+%F-%H%M%S')
  log_path="${logdir}/$(basename $1)-${timestamp}.log"
  >$log_path 2>&1 $*
  if (( $? ))
  then
    err $*
    cat $log_path
    log 'Error log saved to' $log_path
    exit
  else
    rm $log_path
  fi
}

function device {
  ssh -p 2222 -oStrictHostKeyChecking=no root@localhost $*
}

MTREE_KEYWORDS='uid,gid,mode,sha1digest'

