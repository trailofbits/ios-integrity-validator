linux: 
	gcc -o vfdecrypt vfdecrypt.c -lcrypto
mac:
	gcc -o vfdecrypt vfdecrypt.c -lcrypto -DMAC_OSX
install: 
	cp ./vfdecrypt /usr/local/bin
	ldconfig
uninstall:
	rm -rf /usr/local/bin/vfdecrypt
clean:
	rm -rf ./vfdecrypt
