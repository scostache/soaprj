CFLAGS += -Wall
CPPFLAGS += -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26
LDFLAGS +=

LIB = -lfuse -lpthread -lm

all:
	make -C hybfs-core/
	cp hybfs-core/hybfs ../bin/

clean:
	make -C hybfs-core/ clean
	rm ../bin/hybfs
