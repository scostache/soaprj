# dummy makefile for all the sub-modules

all:
	make -C src/hybfs-core/
	cp src/hybfs-core/hybfs bin/

clean:
	make -C src/hybfs-core/ clean
	rm bin/hybfs
