KBUILD_CFLAGS += -I/home/kml/include

obj-m += readahead.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

CFLAGS_readahead.o := -DKML_KERNEL
CFLAGS_REMOVE_readahead.o += -mno-sse2
CFLAGS_REMOVE_readahead.o += -mno-sse
CFLAGS_REMOVE_readahead.o += -mno-mmx
