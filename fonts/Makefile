all: fontconvert

CC     = gcc
CFLAGS = -Wall -I/usr/local/include/freetype2 -I/usr/include/freetype2 -I/usr/include
LIBS   = -lfreetype

fontconvert: fontconvert.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
	strip $@

fontconvert_ada: fontconvert_ada.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
	strip $@

clean:
	rm -f fontconvert
