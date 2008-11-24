# Makefile for the HybFS core
HOME_DIR = $(CURDIR)/../../

# Boost libraries path
BOOST = $(HOME_DIR)/../boost_1_37_0/

# Sqlite3 path
SQLITE = $(HOME_DIR)/sqlite

# this tells where my headers are
INC = $(HOME_DIR)/src/include

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

# when finish debugging add -O2 -finline-functions
EXTRA_OPTS = -Wall -g

CXXFLAGS = -I$(INC) -I$(SQLITE)/include -I$(BOOST)/ $(DEFS) $(EXTRA_OPTS)
LDFLAGS += -L$(SQLITE)/lib/

LIB = -lfuse -lpthread -lsqlite3

SRCS=$(shell find $(1) -type f -name '*.cpp')
OBJS=$(patsubst %.cpp,%.o,$(call SRCS,$(1)))


hybfs: $(OBJS) $(INC)
	echo $(CWD)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIB)

clean:
	rm -f hybfs
	rm -f *.o
