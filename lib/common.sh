# Print line to terminal.
# Includes a colored prefix to differentiate iiv output from other output.
#
# Arguments:
#
#   1... - Arguments to print
#
function log {
  print "\x1b[34m==> \x1b[37m$*\x1b[0m"
}

# Print an error to terminal.
# Includes a colored prefix to differentiate iiv output from other output.
#
# Arguments:
#
#   1... - Arguments to print
#
function err {
  print "\x1b[31;4mError\x1b[0m: $*"
}

# Print a warning to terminal.
# Includes a colored prefix to differentiate iiv output from other output.
#
# Arguments:
#
#   1... - Arguments to print
#
function warn {
  print "\x1b[31;4mWarning\x1b[0m: $*"
}

# Run a command, recording the output in a log file, and exit if the command
# fails. If the command succeeds, the log is deleted. If it fails, the log is
# printed, an error is printed, and the program terminates.
#
# Arguments:
#
#   1... - Command to run
#
# Examples:
#
#   check make install
#
function check {
  local logdir="$HOME/Library/Logs/com.trailofbits.iiv"
  mkdir -p $logdir

  local timestamp
  timestamp=$(date '+%F-%H%M%S')
  local log_path="${logdir}/$(basename $1)-${timestamp}.log"

  $* >$log_path 2>&1

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

# Run a command on the connected iOS device.
#
# Arguments:
#
#   1... - Command to run
#
# Examples:
#
#   device /sbin/halt
#
function device {
  ssh -p 2222 -oStrictHostKeyChecking=no root@localhost $*
}

# mtree keywords are used in multiple places, so we define them here.
MTREE_KEYWORDS='uid,gid,mode,sha1digest'

