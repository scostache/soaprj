CFLAGS = -I../include -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -Wall

LIB = -lfuse -lpthread

SRCS=$(shell find $(1) -type f -name '*.c')
OBJS=$(patsubst %.c,%.o,$(call SRCS,$(1)))


hybfs: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIB)

clean:
	rm -f hybfs
	rm -f *.o
