CC = clang
LIB_DIR = ./lib
INCLUDE_DIR = include/
CFLAGS = -Wall -Werror -I $(INCLUDE_DIR) -arch i386 -arch x86_64 \
		-mmacosx-version-min=10.7

libs: libdir libhostoverride.dylib libconnectoverride.dylib

libdir:
	mkdir $(LIB_DIR)

src/%.c :  $(INCLUDE_DIR)/override_lookup.h

%.dylib : src/%.c
	$(CC) $(CFLAGS) -o $(LIB_DIR)/$@ -dynamiclib $^
	strip -S -no_uuid $(LIB_DIR)/$@
	install_name_tool -id $@ $(LIB_DIR)/$@
