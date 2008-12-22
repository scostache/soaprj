# Makefile for the HybFS core
HOME_DIR = $(CURDIR)/../

LIB_DIR = $(HOME_DIR)/lib/

BIN_DIR = $(HOME_DIR)/bin/

# Boost libraries path
BOOST = $(HOME_DIR)/../boost_1_37_0/

# Sqlite3 path
SQLITE = $(HOME_DIR)/sqlite

# this tells where my head(er)s are
INC = $(HOME_DIR)/src/include

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

# when finish debugging add -O2 -finline-functions
EXTRA_OPTS = -fPIC -Wall -g

CXXFLAGS = -I$(INC) -I$(SQLITE)/include -I$(BOOST)/ $(DEFS) $(EXTRA_OPTS)

LDFLAGS += -L$(SQLITE)/lib/ -L$(LIB_DIR)

HYBFS_LIB = -lpthread -lsqlite3 -lfuse -lhybfs


SRCS=$(shell find $(1) -type f -name '*.cpp')
OBJS=$(patsubst %.cpp,%.o,$(call SRCS,$(1)))


LIB_SRCS = $(call OBJS,hybfs-core)

HYBFS_SRCS = $(call OBJS,hybfs-fuse) 

MM_SRCS = $(call SRCS,module-manager)

#PARSER_SRCS = $(call OBJS,parser)


all: lib libso hybfs mmanager

lib: $(LIB_SRCS)
	ar r $(LIB_DIR)/libhybfs.a $(LIB_SRCS)
	ranlib $(LIB_DIR)/libhybfs.a

libso: $(LIB_SRCS)
	$(CXX) -shared -Wl,-soname,libhybfs.so.0.1 -o $(LIB_DIR)/libhybfs.so.0.1 $(LIB_SRCS)


hybfs: libso $(HYBFS_SRCS) $(INC)/*.h
	$(CXX) $(CCFLAGS) $(LDFLAGS) -o $(BIN_DIR)/hybfs $(HYBFS_SRCS) $(HYBFS_LIB)


mmanager: $(MM_SRCS)
	$(MAKE) -C module-manager


hybfs-core/%.o: hybfs-core/%.cpp $(INC)/core/*.hpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

clean:
	rm -f hybfs-core/*.o hybfs-fuse/*.o module-manager/*.o

clean-all: clean
	rm -f $(LIB_DIR)/libhybfs.* $(BIN_DIR)/*
