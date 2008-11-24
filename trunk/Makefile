# dummy makefile for all the sub-modules

SUBDIRS = src/parser src/hybfs-core

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

install:
	cp src/hybfs-core/hybfs bin/
	cd doc && doxygen hybfs_cpp.doxyfile && cd ..

clean:
	$(MAKE) -C src/hybfs-core/ clean
	rm -rf doc/html doc/latex doc/man  bin/hybfs 

.PHONY: $(SUBDIRS)
