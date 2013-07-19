#!/usr/bin/env zsh

typeset -A IPSW_URLS
IPHONE4_CDMA=(
  '4.2.10'  'http://appldnld.apple.com/iPhone4/041-1959.20110721.jvP29/iPhone3,3_4.2.10_8E600_Restore.ipsw'
  '5.0'     'http://appldnld.apple.com/iPhone4/041-9743.20111012.vjhfp/iPhone3,3_5.0_9A334_Restore.ipsw'
  '5.1.1'   'http://appldnld.apple.com/iOS5.1.1/041-4291.20120427.Zs8F0/iPhone3,3_5.1.1_9B206_Restore.ipsw'
  '6.0'     'http://appldnld.apple.com/iOS6/Restore/041-7177.20120919.xqoqs/iPhone3,2_6.0_10A403_Restore.ipsw'
  '6.0.1'   'http://appldnld.apple.com/iOS6/041-7952.20121101.aClm5/iPhone3,3_6.0.1_10A523_Restore.ipsw'
  '6.1.3'   'http://appldnld.apple.com/iOS6.1/091-2444.20130319.exFqm/iPhone3,2_6.1.3_10B329_Restore.ipsw'
)

typeset -A IPSW_SHA1
IPSW_SHA1=()

