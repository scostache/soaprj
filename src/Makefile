# Makefile for the HybFS core
HOME_DIR = $(CURDIR)/../

# Boost libraries path
BOOST = $(HOME_DIR)/../boost_1_37_0/

# Sqlite3 path
SQLITE = $(HOME_DIR)/sqlite

# this tells where my head(er)s are
INC = $(HOME_DIR)/src/include

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

# when finish debugging add -O2 -finline-functions
EXTRA_OPTS = -Wall -g

CXXFLAGS = -I$(INC) -I$(SQLITE)/include -I$(BOOST)/ $(DEFS) $(EXTRA_OPTS)

LDFLAGS += -L$(SQLITE)/lib/ -L$(CURDIR)/hybfs-core/

HYBFS_LIB = -lpthread -lsqlite3 -lfuse -lhybfs


SRCS=$(shell find $(1) -type f -name '*.cpp')
OBJS=$(patsubst %.cpp,%.o,$(call SRCS,$(1)))


LIB_SRCS = $(call OBJS,hybfs-core)

HYBFS_SRCS = $(call OBJS,hybfs-fuse) 

MM_SRCS = $(call OBJS,module-manager)

#PARSER_SRCS = $(call OBJS,parser)


all: lib hybfs mmanager

lib: $(LIB_SRCS)
	ar r hybfs-core/libhybfs.a $(LIB_SRCS)
	ranlib hybfs-core/libhybfs.a


hybfs: $(HYBFS_SRCS) $(INC)/*.h hybfs-core/libhybfs.a
	$(CXX) $(CCFLAGS) $(LDFLAGS) -o hybfs-fuse/hybfs $(HYBFS_SRCS) $(HYBFS_LIB)

mmanager: $(MM_SRCS)
	$(MAKE) -C module-manager
	

hybfs-core/%.o: hybfs-core/%.cpp $(INC)/core/*.hpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<


clean:
	rm -f hybfs-core/libhybfs.a hybfs-fuse/hybfs
	rm -f hybfs-core/*.o hybfs-fuse/*.o
