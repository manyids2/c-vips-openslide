PROJECT := c-vips-openslide
SRC_FILES := $(wildcard src/*.c)
HDR_FILES := $(wildcard src/*.h)
LDFLAGS := -lopenslide -lm -lvips -lglib-2.0
IFLAGS := -I/usr/include/glib-2.0
CXXFLAGS := -Wall -Wextra -pedantic -std=c99 -O3 -pg
VIPSFLAGS := -pthread -I/usr/local/include -I/usr/local/include/openjpeg-2.5 -I/usr/include/libexif -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/x86_64-linux-gnu -I/usr/include/librsvg-2.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/lib/x86_64-linux-gnu/hdf5/serial/include -I/usr/include/OpenEXR -I/usr/include/orc-0.4 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/local/lib/x86_64-linux-gnu -lvips-cpp -lvips -lgio-2.0 -lgobject-2.0 -lglib-2.0

bin/wsi-tvx: $(SRC_FILES) $(HDR_FILES)
	$(CC) $(SRC_FILES) -o bin/$(PROJECT) $(IFLAGS) $(LDFLAGS) $(CXXFLAGS) $(VIPSFLAGS) -g

clean:
	rm bin/$(PROJECT)
