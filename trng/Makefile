# Note: It is important to make sure you include the <bsd.kmod.mk> makefile after declaring the KMOD and SRCS variables.

KMOD    =  trng
SRCS    =  trng.c
SRCS    += device_if.h bus_if.h
KMODDIR	=	/boot/modules

#CFLAGS  += -DDEBUG -g
#CFLAGS	+= -DRNDTEST

.include <bsd.kmod.mk>
