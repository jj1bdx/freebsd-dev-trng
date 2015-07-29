all: trngdev feedtrng

clean:
	make -f Makefile.trngdev clean
	make -f Makefile.feedtrng clean

trngdev:
	make -f Makefile.trngdev

feedtrng:
	make -f Makefile.feedtrng
