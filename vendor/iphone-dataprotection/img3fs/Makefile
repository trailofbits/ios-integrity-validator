img3fs: img3fs.c
	gcc -o $@ $^ -Wall -lfuse_ino64 -lcrypto -I/usr/local/include/osxfuse || gcc -o $@ $^ -Wall -losxfuse_i64 -lcrypto -I/usr/local/include/osxfuse -L/usr/local/lib

clean:
	rm img3fs
