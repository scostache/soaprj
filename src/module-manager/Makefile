# CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

HOME_DIR = $(CURDIR)/../../

#OBJS = mp3.o test.o

# db_backend path
DB_BACKEND = $(HOME_DIR)/src/include

# hybfs-core directory
HYB_DIR = $(HOME_DIR)/src/hybfs-core

# Boost libraries path
BOOST = $(HOME_DIR)/../boost_1_37_0/

DEFS = -DDBG -D_FILE_OFFSET_BITS=64

LIBS = -ltag -lexiv2 -lpthread -lsqlite3 -lhybfs

# where are the headers
INC	 =		$(HOME_DIR)/src/include/modules/data

# where is the taglib.so header files
LIB_DIR = $(HOME_DIR)/extensions/lib

MP3_INC = $(HOME_DIR)/extensions/include/mp3

HYB_OPS_INC = $(HOME_DIR)/src/include/modules/ops

EXIF_INC = $(HOME_DIR)/extensions/include/exif

SQLITE_INC = $(HOME_DIR)/sqlite/include

# Sqlite3 path
SQLITE = $(HOME_DIR)/sqlite

EXTRA_OPTS = -Wall -g

# to be added -O2 -finline-functions --> (c) Steph
CXXFLAGS = -I$(INC) -I$(HYB_OPS_INC) -I$(MP3_INC) -I$(EXIF_INC) -I$(SQLITE_INC) -I$(DB_BACKEND) -I$(BOOST)/ $(DEFS) $(EXTRA_OPTS)

# taglib.so location
LDFLAGS = -L$(LIB_DIR) -L$(HOME_DIR)/lib -L$(SQLITE)/lib -Wl,-rpath $(LIB_DIR)

SRCS = $(shell find $(1) -type f -name '*.cpp') 
OBJS = $(patsubst %.cpp,%.o,$(call SRCS,$(1)))

# default test
TARGET =	module-manager

$(TARGET): $(OBJS) $(INC) $(HOME_DIR)/lib/libhybfs.*
	$(CXX) $(LDFLAGS) -o $(HOME_DIR)/bin/$(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm $(shell find $(1) -type f -name '*.o')
	rm -f test
