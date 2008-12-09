# CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

HOME_DIR = $(CURDIR)/../../

#OBJS = mp3.o test.o

# db_backend path
DB_BACKEND = $(HOME_DIR)/src/include

# Boost libraries path
BOOST = $(HOME_DIR)/../boost_1_37_0/

DEFS = -DDBG -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

LIBS = -ltag -lexiv2 -lpthread -lsqlite3

# where are the headers
INC	 =		$(HOME_DIR)/src/include/modules/mp3

# where is the taglib.so header files
TAG_LIB = $(HOME_DIR)/extensions

# Sqlite3 path
SQLITE = $(HOME_DIR)/sqlite

EXTRA_OPTS = -Wall -g

# to be added -O2 -finline-functions --> (c) Steph
CXXFLAGS = -I$(INC) -I$(TAG_LIB)/mp3/include -I$(TAG_LIB)/exif/include -I$(SQLITE)/include -I$(DB_BACKEND) -I$(BOOST)/ $(DEFS) $(EXTRA_OPTS)

# taglib.so location
LDFLAGS = -L$(TAG_LIB)/mp3/lib -L$(TAG_LIB)/exif/lib -L$(SQLITE)/lib -L$(HOME_DIR)/extensions/exif/include

SRCS = $(shell find $(1) -type f -name '*.cpp')
OBJS = $(patsubst %.cpp,%.o,$(call SRCS,$(1)))

# default test
TARGET =	test

$(TARGET):	$(OBJS) $(INC)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) ../hybfs-core/db_backend.o ../hybfs-core/misc.o ../hybfs-core/hybfs_data.o ../hybfs-core/path_crawler.o ../hybfs-core/virtualdir.o $(LIBS)

all:	$(TARGET)

clean:
	rm $(shell find $(1) -type f -name '*.o')
	rm -f test
