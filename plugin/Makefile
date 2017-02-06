CFLAGS += `pkg-config --cflags gtk+-2.0`
CFLAGS += -O3 -Wall -fPIC -shared

C_FILES := $(wildcard *.c)
OBJ_FILES := $(addprefix bin/,$(notdir $(C_FILES:.c=.so)))

all: $(OBJ_FILES)

install: $(OBJ_FILES)
	cp $^ /usr/share/configbutton

bin/%.so: %.c
	mkdir -p bin
	$(CC) -o bin/$(<:.c=.so) $< $(CFLAGS) -Wl,-soname,$(<:.c=.so) $(LDFLAGS)

clean:
	rm -Rf bin
