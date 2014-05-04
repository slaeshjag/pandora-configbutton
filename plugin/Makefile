CFLAGS += `pkg-config --cflags gtk+-2.0`
CFLAGS += -O3 -Wall -fPIC -shared

default:
	mkdir -p bin
	$(CC) -o cpuspeed.so cpuspeed.c $(CFLAGS) -Wl,-soname,cpuspeed.so $(LDFLAGS)
	$(CC) -o usbhost.so usbhost.c $(CFLAGS) -Wl,-soname,usbhost.so $(LDFLAGS)
	$(CC) -o wifi.so wifi.c $(CFLAGS) -Wl,-soname,wifi.so $(LDFLAGS)
	$(CC) -o bluetooth.so bluetooth.c $(CFLAGS) -Wl,-soname,bluetooth.so $(LDFLAGS)
	$(CC) -o tvout.so tvout.c $(CFLAGS) -Wl,-soname,tvout.so $(LDFLAGS)
	$(CC) -o usbmass.so usbmass.c $(CFLAGS) -Wl,-soname,usbmass.so $(LDFLAGS)
	$(CC) -o command.so command.c $(CFLAGS) -Wl,-soname,command.so $(LDFLAGS)
	mv *.so bin/

clean:
	rm -Rf bin
