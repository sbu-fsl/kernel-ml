KBUILD_CFLAGS += -I/home/kml/include

obj-m += kml-nfs.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

CFLAGS_kml-nfs.o := -DKML_KERNEL
CFLAGS_REMOVE_kml-nfs.o += -mno-sse2
CFLAGS_REMOVE_kml-nfs.o += -mno-sse
CFLAGS_REMOVE_kml-nfs.o += -mno-mmx
