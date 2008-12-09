# dummy makefile for all the sub-modules

SUBDIRS = src/

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

install:
	cp src/hybfs-fuse/hybfs bin/
	cp src/module-manager/module_manager bin/
	cp src/hybfs-core/libhybfs.a lib/
	cp src/parser/libparser.a lib/
	cd doc && doxygen hybfs_cpp.doxyfile

clean:
	$(MAKE) -C src/ clean
	rm -rf doc/html doc/latex doc/man
	rm -rf bin/*

.PHONY: $(SUBDIRS)
