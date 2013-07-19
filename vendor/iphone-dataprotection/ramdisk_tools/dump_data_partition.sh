#!/bin/sh

cat /dev/rdisk0s2s1 | netcat -l -p 1234
