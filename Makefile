# Makefile for reip

LWIPDIR = src
LWIPARCH = reip
NEWLIBDIR = newlib
HTTPDDIR = httpd_raw
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS := $(CFLAGS) -mcmodel=large
CFLAGS := $(CFLAGS) -nostdlib -nostartfiles -nodefaultlibs
CFLAGS := $(CFLAGS) -fomit-frame-pointer -fno-stack-protector -mno-red-zone
CFLAGS := $(CFLAGS) -Os
CFLAGS := $(CFLAGS) -I$(LWIPDIR)/include
CFLAGS := $(CFLAGS) -I$(LWIPARCH)/include
CFLAGS := $(CFLAGS) -I$(LWIPDIR)/include/ipv4
CFLAGS := $(CFLAGS) -I$(LWIPDIR)/include/ipv6
CFLAGS := $(CFLAGS) -I$(LWIPDIR)
CFLAGS := $(CFLAGS) -I$(HTTPDDIR)
CFLAGS := $(CFLAGS) -I$(NEWLIBDIR)/include
CFLAGS := $(CFLAGS) -I.
LDFLAGS := $(LDFLAGS) -T app.ld
LDFLAGS := $(LDFLAGS) --as-needed

ifdef NEWLIB_PATH
CFLAGS := $(CFLAGS) -I "$(NEWLIB_PATH)/include"
endif

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES = $(LWIPDIR)/core/def.c $(LWIPDIR)/core/dhcp.c $(LWIPDIR)/core/dns.c \
	$(LWIPDIR)/core/ipv4/inet_chksum.c $(LWIPDIR)/core/init.c $(LWIPDIR)/core/mem.c \
	$(LWIPDIR)/core/memp.c $(LWIPDIR)/core/netif.c $(LWIPDIR)/core/pbuf.c \
	$(LWIPDIR)/core/raw.c $(LWIPDIR)/core/stats.c $(LWIPDIR)/core/sys.c \
	$(LWIPDIR)/core/tcp.c $(LWIPDIR)/core/tcp_in.c $(LWIPDIR)/core/tcp_in.c \
	$(LWIPDIR)/core/tcp_out.c $(LWIPDIR)/core/timers.c $(LWIPDIR)/core/udp.c
CORE4FILES = $(LWIPDIR)/core/ipv4/autoip.c $(LWIPDIR)/core/ipv4/icmp.c \
	$(LWIPDIR)/core/ipv4/igmp.c $(LWIPDIR)/core/ipv4/ip_frag.c \
	$(LWIPDIR)/core/ipv4/ip.c $(LWIPDIR)/core/ipv4/ip_addr.c

# NETIFFILES: Files implementing various generic network interface functions.
NETIFFILES = $(LWIPDIR)/netif/etharp.c

# ARCHFILES: Architecture specific files.
ARCHFILES =

# LWIPFILES: All the above.
LWIPFILES = $(COREFILES) $(CORE4FILES) $(NETIFFILES) $(ARCHFILES)
LWIPFILESW = $(wildcard $(LWIPFILES))
LWIPOBJS = $(notdir $(LWIPFILESW:.c=.o))

# REIPFILES
REIPFILES := reip.o renetif.o fs.o httpd.o libBareMetal.o
REIPFILES := $(REIPFILES) liblwip.a
ifdef NEWLIB_PATH
REIPFILES := $(REIPFILES) $(NEWLIB_PATH)/lib/crt0.o
REIPFILES := $(REIPFILES) $(NEWLIB_PATH)/lib/libnosys.a
REIPFILES := $(REIPFILES) $(NEWLIB_PATH)/lib/libc.a
endif

TARGET = liblwip.a

.PHONY: all
all:
	$(CC) $(CFLAGS) -c $(LWIPFILES)
	ar -cvq $(TARGET) $(LWIPOBJS)
	$(CC) $(CFLAGS) -c reip/reip.c reip/renetif.c reip/libBareMetal.c
	$(CC) $(CFLAGS) -c httpd_raw/fs.c httpd_raw/httpd.c
	$(LD) $(LDFLAGS) $(REIPFILES) -o reip/reip.app
	mv reip/reip.app httpd.app

.PHONY: clean
clean:
	rm -f *.o $(TARGET) *.s httpd.app reip/reip.app
