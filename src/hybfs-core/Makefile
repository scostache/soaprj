CFLAGS += -Wall
CPPFLAGS += -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26
LDFLAGS +=

LIB = -lfuse -lpthread -lm

HYBFS_OBJ = fusexmp.o

hybfs: $(HYBFS_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(HYBFS_OBJ) $(LIB)

clean:
	rm -f hybfs
	rm -f *.o
