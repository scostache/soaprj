# dummy makefile for all the sub-modules

all:
	make -C hybfs-core/
	cp hybfs-core/hybfs ../bin/

clean:
	make -C hybfs-core/ clean
	rm ../bin/hybfs
