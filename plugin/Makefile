CFLAGS += -O3 -Wall -fPIC -shared

default:
	mkdir -p bin
	$(CC) -o cpuspeed.so cpuspeed.c $(CFLAGS) -Wl,-soname,cpuspeed.so
	$(CC) -o usbhost.so usbhost.c $(CFLAGS) -Wl,-soname,usbhost.so
	$(CC) -o wifi.so wifi.c $(CFLAGS) -Wl,-soname,wifi.so
	$(CC) -o bluetooth.so bluetooth.c $(CFLAGS) -Wl,-soname,bluetooth.so
	$(CC) -o tvout.so tvout.c $(CFLAGS) -Wl,-soname,tvout.so
	mv *.so bin/

clean:
	rm -Rf bin
