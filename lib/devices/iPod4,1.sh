#!/usr/bin/env zsh

typeset -A IPSW_URLS
IPSW_URLS=(
  '4.1'   'http://appldnld.apple.com/iPhone4/061-9344.20100922.Urgt43/iPod4,1_4.1_8B118_Restore.ipsw'
  '4.2.1' 'http://appldnld.apple.com/iPhone4/061-9859.20101122.$erft/iPod4,1_4.2.1_8C148_Restore.ipsw'
  '4.3'   'http://appldnld.apple.com/iPhone4/061-9588.20110311.GtP7y/iPod4,1_4.3_8F190_Restore.ipsw'
  '4.3.5' 'http://appldnld.apple.com/iPhone4/041-1963.20110721.Huant/iPod4,1_4.3.5_8L1_Restore.ipsw'
  '5.0'   'http://appldnld.apple.com/iPhone4/061-9622.20111012.Evry3/iPod4,1_5.0_9A334_Restore.ipsw'
  '5.0.1' 'http://appldnld.apple.com/iPhone4/041-3313.20111109.Azxe3/iPod4,1_5.0.1_9A405_Restore.ipsw'
  '5.1'   'http://appldnld.apple.com/iOS5/041-1542.20120307.Jyknq/iPod4,1_5.1_9B176_Restore.ipsw'
  '5.1.1' 'http://appldnld.apple.com/iOS5.1.1/041-4298.20120427.Tzr8w/iPod4,1_5.1.1_9B206_Restore.ipsw'
  '6.0'   'http://appldnld.apple.com/iOS6/Restore/041-0807.20120919.soT6X/iPod4,1_6.0_10A403_Restore.ipsw'
  '6.0.1' 'http://appldnld.apple.com/iOS6/041-7439.20121101.rgt54/iPod4,1_6.0.1_10A523_Restore.ipsw'
  '6.1.3' 'http://appldnld.apple.com/iOS6.1/091-2655.20130319.Wyt54/iPod4,1_6.1.3_10B329_Restore.ipsw'
)

typeset -A IPSW_SHA1
IPSW_SHA1=(
  '5.0'   '668b3784966d829ec6007d10e38fa117d6799cf1'
  '5.0.1' 'ec4feac81de53459701e4baabb6b4534be3260ca'
  '5.1'   '65d7ee2d2fa505ce6fe91271f84f3fc9cbcc81a1'
  '5.1.1' '5facf346ad939e9f11854b05237fddc1ed59e2e3'
  '6.1.3' 'c9c8efb5663b8ab5faece095cefd03aa7d499f5e'
)

typeset -A IPSW_ROOT
IPSW_ROOT=(
  '5.0'   '038-2707-006.dmg'
  '5.0.1' '038-3698-001.dmg'
  '5.1'   '038-1765-165.dmg'
  '5.1.1' '038-4290-006.dmg'
  '6.1.3' '048-2642-005.dmg'
)

typeset -A IPSW_KEYS
IPSW_KEYS=(
  '5.0'   '575bcb4f9290a28bc00451f7e444973fd8b0afc529d2d84db4ae227bdd779563f070eaea'
  '5.0.1' '7ed37d8c051da8f8d31b0ccf0980fa5ffa54770c7e68ecb5ebf28abe683cadf21a4a99ed'
  '5.1'   '1ab3765d93aaf106b918829db85c18e724473e51106ad89f3d10e863cc3864974c2852f5'
  '5.1.1' 'f21b4cc1ae39ed3b36d33374f77e224b9e77f375f8722c551acdc7033ef16383c0b99b39'
  '6.1.3' '625003a349494ed7fc1972d30f7cf840f8bbff11b2d493593017d0606d88605b7fad5e61'
)

