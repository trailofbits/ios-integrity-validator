SDKVER?=5.1
ARCH?=armv7
MINIOS=4.0
SDK=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(SDKVER).sdk/
HGVERSION:= $(shell hg parents --template '{node|short}' || echo "unknown")
CC=clang -arch $(ARCH)
CFLAGS=-Wall -isysroot $(SDK) -DHGVERSION="\"${HGVERSION}\"" -O3 -I.
CFLAGS+= -framework CoreFoundation -framework IOKit -framework Security
CFLAGS+= -miphoneos-version-min=$(MINIOS)
CODESIGN=codesign -s - --entitlements

all: $(SDK) IOKit IOUSBDeviceControllerLib.h device_infos restored_external bruteforce ioflashstoragekit

$(SDK):
	@echo "iOS SDK not found in $(SDK)"
	@echo "=> check SDKVER/SDK in Makefile"

IOUSBDeviceControllerLib.h:
	curl -o IOUSBDeviceControllerLib.h http://www.opensource.apple.com/source/IOKitUser/IOKitUser-502/usb_device.subproj/IOUSBDeviceControllerLib.h?txt

IOKit:
	ln -s /System/Library/Frameworks/IOKit.framework/Versions/Current/Headers IOKit

device_infos: device_infos.c device_info.c IOAESAccelerator.c AppleEffaceableStorage.c AppleKeyStore.c bsdcrypto/pbkdf2.c bsdcrypto/sha1.c bsdcrypto/key_wrap.c bsdcrypto/rijndael.c util.c IOKit.c registry.c ioflash/ioflash.c kernel_patcher.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) tfp0.plist $@

restored_external: restored_external.c device_info.c remote_functions.c plist_server.c AppleKeyStore.c AppleEffaceableStorage.c IOKit.c IOAESAccelerator.c util.c registry.c AppleKeyStore_kdf.c bsdcrypto/pbkdf2.c bsdcrypto/sha1.c bsdcrypto/rijndael.c bsdcrypto/key_wrap.c ioflash/ioflash.c kernel_patcher.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) keystore_device.xml $@

bruteforce: systemkb_bruteforce.c AppleKeyStore.c AppleEffaceableStorage.c IOKit.c IOAESAccelerator.c util.c registry.c AppleKeyStore_kdf.c bsdcrypto/pbkdf2.c bsdcrypto/sha1.c bsdcrypto/rijndael.c bsdcrypto/key_wrap.c device_info.c ioflash/ioflash.c kernel_patcher.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) keystore_device.xml $@

ioflashstoragekit: ioflash/ioflash.c ioflash/ioflash_kernel.c ioflash/ioflashstoragekit.c util.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) tfp0.plist $@

kernel_patcher: kernel_patcher.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) tfp0.plist $@

shsh_dump: ioflash/ioflash.c ioflash/ioflash_kernel.c shsh_dump.c util.c
	$(CC) $(CFLAGS) -o $@ $^
	$(CODESIGN) tfp0.plist $@

clean:
	rm -f bruteforce restored_external device_infos ioflashstoragekit shsh_dump

rebuild: clean all
