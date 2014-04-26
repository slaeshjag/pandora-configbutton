CFLAGS += `pkg-config --cflags gtk+-2.0` -Wall -O0 -g
LDFLAGS += `pkg-config --libs gtk+-2.0` -ldl

default:
	$(CC)  configbutton.c load_plugins.c config.c plugin_settings.c -o configbutton $(CFLAGS) $(LDFLAGS)
	cd plugin && make

clean:
	rm -f configbutton
	cd plugin && make clean
