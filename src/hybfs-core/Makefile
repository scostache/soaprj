# Makefile for the HybFS core

# Sqlite3 path
SQLITE = ../../sqlite

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

EXTRA_OPTS = -Wall 

INC = ../include

CXXFLAGS = -I../include -I$(SQLITE)/include $(DEFS) $(EXTRA_OPTS)
LDFLAGS += -L$(SQLITE)/lib/

LIB = -lfuse -lpthread -lsqlite3

SRCS=$(shell find $(1) -type f -name '*.cpp')
OBJS=$(patsubst %.cpp,%.o,$(call SRCS,$(1)))


hybfs: $(OBJS) $(INC)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIB)

clean:
	rm -f hybfs
	rm -f *.o
