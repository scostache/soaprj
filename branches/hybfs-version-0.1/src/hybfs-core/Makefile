# Makefile for the HybFS core

# Berkley DB instalation path
BDB_PATH = /usr/local/BerkeleyDB.4.7

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

EXTRA_OPTS = -Wall 

INC = ../include

CFLAGS = -I../include -I$(BDB_PATH)/include $(DEFS) $(EXTRA_OPTS)
LDFLAGS += -L$(BDB_PATH)/lib

LIB = -lfuse -lpthread -ldb

SRCS=$(shell find $(1) -type f -name '*.c')
OBJS=$(patsubst %.c,%.o,$(call SRCS,$(1)))


hybfs: $(OBJS) $(INC)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIB)

clean:
	rm -f hybfs
	rm -f *.o
