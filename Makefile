all: trngdev feedtrng

trngdev:
	@make -f Makefile.trngdev

feedtrng:
	@make -f Makefile.feedtrng
